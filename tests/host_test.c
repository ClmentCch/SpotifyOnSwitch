/* Host-side tests: the parts of Musictt that do not need libnx or SDL.
 *
 *   bash scripts/host_test.sh
 */
#include <sys/stat.h>
#include <unistd.h>

#include "color.h"
#include "i18n.h"
#include "library.h"
#include "settings.h"

static int failures;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); failures++; } \
    else printf("ok: %s\n", msg); \
} while (0)

static void write_file(const char *path, const char *data) {
    FILE *f = fopen(path, "wb");
    if (!f) { printf("cannot write %s\n", path); failures++; return; }
    fwrite(data, 1, strlen(data), f);
    fclose(f);
}

static void mkdirs(const char *path) {
    char buf[512];
    snprintf(buf, sizeof(buf), "%s", path);
    for (char *p = buf + 1; *p; p++) {
        if (*p == '/') { *p = 0; mkdir(buf, 0777); *p = '/'; }
    }
    mkdir(buf, 0777);
}

static const char *MANGER_JSON =
    "{\"id\":\"2.1\",\"name\":\"Manger\",\"playlist_logo\":\"playlist_manger.png\","
    "\"songs\":["
    "{\"id\":\"2.1.2\",\"number\":2,\"name\":\"Deuxieme\",\"file\":\"2.mp3\",\"logo\":\"music_manger_2.png\"},"
    "{\"id\":\"2.1.1\",\"number\":1,\"name\":\"Premiere\",\"file\":\"1.mp3\",\"logo\":\"music_manger_1.png\"},"
    "{\"number\":3,\"name\":\"Troisieme\",\"file\":\"3.mp3\"}"
    "]}";

static const char *DUP_JSON =
    "{\"id\":\"9\",\"name\":\"Dup\",\"songs\":["
    "{\"id\":\"x\",\"number\":1,\"name\":\"A\",\"file\":\"a.mp3\"},"
    "{\"id\":\"x\",\"number\":2,\"name\":\"B\",\"file\":\"b.mp3\"}"
    "]}";

static void test_library(void) {
    const char *root = "/tmp/musictt-host/MUSIC";
    system("rm -rf /tmp/musictt-host");
    mkdirs("/tmp/musictt-host/MUSIC/PLAYLISTS/MANGER");
    mkdirs("/tmp/musictt-host/MUSIC/PLAYLISTS/GAMING");
    mkdirs("/tmp/musictt-host/MUSIC/PLAYLISTS/SPORT");
    mkdirs("/tmp/musictt-host/MUSIC/PLAYLISTS/DUP");
    mkdirs("/tmp/musictt-host/MUSIC/LOGO");
    mkdirs("/tmp/musictt-host/MUSIC/CONFIG");

    write_file("/tmp/musictt-host/MUSIC/PLAYLISTS/MANGER/package.json", MANGER_JSON);
    write_file("/tmp/musictt-host/MUSIC/PLAYLISTS/MANGER/1.mp3", "not really an mp3");
    write_file("/tmp/musictt-host/MUSIC/PLAYLISTS/MANGER/2.mp3", "nor this");
    write_file("/tmp/musictt-host/MUSIC/PLAYLISTS/GAMING/package.json", "{ this is not json ");
    write_file("/tmp/musictt-host/MUSIC/PLAYLISTS/SPORT/package.json", "{\"id\":\"3\",\"name\":\"Sport\",\"songs\":[]}");
    write_file("/tmp/musictt-host/MUSIC/PLAYLISTS/DUP/package.json", DUP_JSON);
    write_file("/tmp/musictt-host/MUSIC/PLAYLISTS/DUP/a.mp3", "x");
    write_file("/tmp/musictt-host/MUSIC/PLAYLISTS/DUP/b.mp3", "x");
    write_file("/tmp/musictt-host/MUSIC/LOGO/playlist_manger.png", "png");
    write_file("/tmp/musictt-host/MUSIC/LOGO/music_manger_1.png", "png");
    write_file("/tmp/musictt-host/MUSIC/LOGO/music_manger_3.png", "png");

    library_set_root(root);
    Library lib;
    int rc = library_scan(&lib);
    CHECK(rc == 0, "scan succeeds");
    CHECK(lib.count == 3, "three valid playlists are loaded");
    CHECK(lib.errors == 1, "the broken package.json is reported");
    CHECK(strcmp(lib.last_error, "GAMING") == 0, "the broken playlist is named");
    CHECK(lib.total_tracks == 5, "track total across playlists");

    int p = library_find_playlist(&lib, "2.1");
    CHECK(p >= 0, "playlist id is read from package.json");
    CHECK(strcmp(lib.items[p].name, "Manger") == 0, "playlist name is read");
    CHECK(lib.items[p].count == 3, "all three songs are kept");
    CHECK(strstr(lib.items[p].logo, "playlist_manger.png") != NULL, "playlist logo is resolved in LOGO/");

    /* Songs must be sorted by their number even though JSON lists 2 before 1. */
    CHECK(lib.items[p].tracks[0].number == 1, "tracks are sorted by number");
    CHECK(strcmp(lib.items[p].tracks[0].name, "Premiere") == 0, "first track name");
    CHECK(lib.items[p].tracks[0].has_file == 1, "existing audio is flagged playable");
    CHECK(strstr(lib.items[p].tracks[0].logo, "music_manger_1.png") != NULL, "song logo is resolved");
    CHECK(lib.items[p].tracks[1].logo[0] == 0, "missing song logo falls back to the placeholder");
    CHECK(lib.items[p].tracks[2].has_file == 0, "missing audio is flagged");
    CHECK(strstr(lib.items[p].tracks[2].logo, "music_manger_3.png") != NULL, "song logo convention is used");
    CHECK(strcmp(lib.items[p].tracks[2].id, "2.1.3") == 0, "missing ids are generated from the playlist id");

    int dup = library_find_playlist(&lib, "9");
    CHECK(dup >= 0 && lib.items[dup].count == 2, "duplicate playlist loads");
    CHECK(strcmp(lib.items[dup].tracks[0].id, "x") == 0, "first duplicate id kept");
    CHECK(strcmp(lib.items[dup].tracks[1].id, "x.2") == 0, "duplicate ids are made unique");

    CHECK(library_track_supported("a.mp3") == 1, "mp3 accepted");
    CHECK(library_track_supported("a.MP3") == 1, "extension check is case-insensitive");
    CHECK(library_track_supported("a.ogg") == 1, "ogg accepted");
    CHECK(library_track_supported("a.mp4") == 0, "mp4 refused");

    library_free(&lib);
    CHECK(lib.count == 0, "library_free resets the library");
}

static void test_settings(void) {
    settings_set_paths("/tmp/musictt-host/MUSIC/CONFIG", "/tmp/musictt-host/MUSIC/CONFIG/settings.json");
    Settings s;
    settings_defaults(&s);
    CHECK(s.language == -1, "defaults ask for a language");
    s.language = LANG_JA;
    s.volume = 63;
    s.animations = 0;
    snprintf(s.last_playlist, sizeof(s.last_playlist), "2.1");
    snprintf(s.last_track, sizeof(s.last_track), "2.1.2");
    settings_push_recent(&s, "2.1/2.1.1");
    settings_push_recent(&s, "2.1/2.1.2");
    settings_push_recent(&s, "2.1/2.1.1");
    CHECK(strcmp(s.recent[0], "2.1/2.1.1") == 0, "recent list is de-duplicated and ordered");
    CHECK(s.recent_count == 2, "recent list holds two entries");
    CHECK(settings_save(&s) == 0, "settings save");

    Settings back;
    CHECK(settings_load(&back) == 0, "settings load");
    CHECK(back.language == LANG_JA, "language survives a round-trip");
    CHECK(back.volume == 63, "volume survives a round-trip");
    CHECK(back.animations == 0, "animation preference survives");
    CHECK(strcmp(back.last_playlist, "2.1") == 0, "last playlist survives");
    CHECK(strcmp(back.recent[0], "2.1/2.1.1") == 0, "recent list survives");

    write_file("/tmp/musictt-host/MUSIC/CONFIG/settings.json", "{broken");
    CHECK(settings_load(&back) == -1, "broken settings.json is rejected, not fatal");
}

static void test_i18n(void) {
    for (int id = 0; id < S_COUNT; id++) {
        for (int l = 0; l < LANG_COUNT; l++) {
            const char *s = i18n_get_lang((StrId)id, (Lang)l);
            if (!s || !*s) {
                printf("FAIL: string %d missing for language %d\n", id, l);
                failures++;
            }
        }
    }
    CHECK(1, "all strings exist in all 12 languages");

    for (int l = 0; l < LANG_COUNT; l++) {
        const char *tracks = i18n_get_lang(S_TRACKS_COUNT, (Lang)l);
        if (!strstr(tracks, "%d")) { printf("FAIL: %d tracks format missing for lang %d\n", 0, l); failures++; }
        const char *missing = i18n_get_lang(S_ERR_TRACK_MISSING, (Lang)l);
        if (!strstr(missing, "%s")) { printf("FAIL: %s missing format for lang %d\n", "", l); failures++; }
    }
    CHECK(1, "formatted strings keep their placeholders");

    CHECK(strcmp(i18n_native_name(LANG_FR), "Français") == 0, "native names are real");
    CHECK(strcmp(i18n_native_name(LANG_JA), "日本語") == 0, "Japanese name");
    CHECK(strcmp(i18n_native_name(LANG_KO), "한국어") == 0, "Korean name");
    CHECK(i18n_cjk_face(LANG_JA) == 0 && i18n_cjk_face(LANG_KO) == 1 && i18n_cjk_face(LANG_ZH) == 2,
          "CJK faces are selected per language");
    CHECK(i18n_uses_cjk(LANG_ZH) == 1 && i18n_uses_cjk(LANG_RU) == 0, "CJK detection");
}

static void test_color(void) {
    enum { W = 64, H = 64 };
    static uint8_t px[W * H * 4];
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            uint8_t *p = &px[(y * W + x) * 4];
            if (y < 40) { p[0] = 210; p[1] = 30; p[2] = 50; }
            else { p[0] = 20; p[1] = 30; p[2] = 200; }
            p[3] = 255;
        }
    }
    Rgb c = color_dominant(px, W, H, W * 4);
    CHECK(c.r > 150 || c.b > 150, "the dominant colour is one of the vivid areas");
    for (int i = 0; i < W * H; i++) { px[i * 4] = 200; px[i * 4 + 1] = 24; px[i * 4 + 2] = 60; }
    Rgb red = color_dominant(px, W, H, W * 4);
    CHECK(red.r > red.g && red.r > red.b, "a red cover gives a red theme");
    ColorTheme t = color_theme(c);
    CHECK(t.top.r < t.accent.r || t.top.g < t.accent.g, "background is darker than the accent");
    Rgb zero = color_dominant(NULL, 0, 0, 0);
    CHECK(zero.r == 124 && zero.b == 255, "no image falls back to the Musictt violet");
}

int main(void) {
    printf("== library ==\n");
    test_library();
    printf("== settings ==\n");
    test_settings();
    printf("== i18n ==\n");
    test_i18n();
    printf("== colour ==\n");
    test_color();
    if (failures) {
        printf("\n%d check(s) failed\n", failures);
        return 1;
    }
    printf("\nall host checks passed\n");
    return 0;
}
