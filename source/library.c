#include "library.h"

#include <ctype.h>
#include <dirent.h>
#include <strings.h>
#include <sys/stat.h>

#include "cJSON.h"

static char g_root[PATH_MAX_LEN] = MUSICTT_ROOT;

void library_set_root(const char *root) {
    if (!root) snprintf(g_root, sizeof(g_root), "%s", MUSICTT_ROOT);
    else snprintf(g_root, sizeof(g_root), "%s", root);
}

const char *library_root(void) { return g_root; }

static void path_join(char *dst, size_t size, const char *a, const char *b) {
    snprintf(dst, size, "%s/%s", a, b);
}

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int dir_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

/* package.json names must be plain file names: no separators, no "..". */
static int safe_name(const char *name) {
    if (!name || !*name) return 0;
    if (strchr(name, '/') || strchr(name, '\\')) return 0;
    if (strstr(name, "..")) return 0;
    return 1;
}

int library_track_supported(const char *path) {
    return has_suffix_ci(path, ".mp3") || has_suffix_ci(path, ".wav") ||
           has_suffix_ci(path, ".flac") || has_suffix_ci(path, ".ogg") ||
           has_suffix_ci(path, ".oga");
}

static char *read_file(const char *path, long *size_out) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long size = ftell(f);
    if (size <= 0 || size > 1024 * 1024) { fclose(f); return NULL; }
    rewind(f);
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (got != (size_t)size) { free(buf); return NULL; }
    buf[size] = 0;
    if (size_out) *size_out = size;
    return buf;
}

static void lowercase(char *dst, size_t size, const char *src) {
    size_t i = 0;
    for (; src[i] && i + 1 < size; i++) dst[i] = (char)tolower((unsigned char)src[i]);
    dst[i] = 0;
}

static char *logo_folder(void) {
    static char buf[PATH_MAX_LEN];
    snprintf(buf, sizeof(buf), "%s/LOGO", g_root);
    return buf;
}

static void resolve_logo(const Playlist *pl, const char *name, const char *convention, char *out, size_t out_size) {
    out[0] = 0;
    const char *candidate = NULL;
    char buf[PATH_MAX_LEN];
    if (safe_name(name) && *name) {
        snprintf(buf, sizeof(buf), "%s/%s", logo_folder(), name);
        candidate = buf;
    }
    if (candidate && file_exists(candidate)) {
        snprintf(out, out_size, "%s", candidate);
        return;
    }
    if (safe_name(convention) && *convention) {
        snprintf(buf, sizeof(buf), "%s/%s", logo_folder(), convention);
        if (file_exists(buf)) snprintf(out, out_size, "%s", buf);
    }
    (void)pl;
}

static int cmp_playlist(const void *a, const void *b) {
    const Playlist *pa = (const Playlist *)a;
    const Playlist *pb = (const Playlist *)b;
    return strcasecmp(pa->name, pb->name);
}

static int cmp_track(const void *a, const void *b) {
    const Track *ta = (const Track *)a;
    const Track *tb = (const Track *)b;
    if (ta->number != tb->number) return ta->number < tb->number ? -1 : 1;
    return strcasecmp(ta->name, tb->name);
}

static int track_id_taken(const Playlist *pl, const char *id) {
    for (int i = 0; i < pl->count; i++) {
        if (strcmp(pl->tracks[i].id, id) == 0) return 1;
    }
    return 0;
}

static int load_playlist(const char *slug, const char *dir, Playlist *out, char *err, size_t err_size) {
    memset(out, 0, sizeof(*out));
    snprintf(out->slug, sizeof(out->slug), "%s", slug);
    snprintf(out->dir, sizeof(out->dir), "%s", dir);
    snprintf(out->name, sizeof(out->name), "%s", slug);
    snprintf(out->id, sizeof(out->id), "%s", slug);

    char json_path[PATH_MAX_LEN];
    path_join(json_path, sizeof(json_path), dir, "package.json");

    long size = 0;
    char *text = read_file(json_path, &size);
    if (!text) {
        snprintf(err, err_size, "%s", slug);
        return -1;
    }
    cJSON *root = cJSON_ParseWithLength(text, (size_t)size);
    free(text);
    if (!root) {
        snprintf(err, err_size, "%s", slug);
        return -1;
    }
    if (!cJSON_IsObject(root)) {
        cJSON_Delete(root);
        snprintf(err, err_size, "%s", slug);
        return -1;
    }

    const cJSON *name = cJSON_GetObjectItem(root, "name");
    if (cJSON_IsString(name) && name->valuestring && *name->valuestring) {
        snprintf(out->name, sizeof(out->name), "%s", name->valuestring);
    }
    const cJSON *id = cJSON_GetObjectItem(root, "id");
    if (cJSON_IsString(id) && id->valuestring && *id->valuestring) {
        snprintf(out->id, sizeof(out->id), "%s", id->valuestring);
    }

    char convention[NAME_MAX_LEN];
    {
        char lower[NAME_MAX_LEN];
        lowercase(lower, sizeof(lower), slug);
        snprintf(convention, sizeof(convention), "playlist_%s.png", lower);
    }
    const cJSON *logo = cJSON_GetObjectItem(root, "playlist_logo");
    resolve_logo(out, cJSON_IsString(logo) ? logo->valuestring : "", convention, out->logo, sizeof(out->logo));

    const cJSON *songs = cJSON_GetObjectItem(root, "songs");
    if (!cJSON_IsArray(songs)) {
        cJSON_Delete(root);
        snprintf(err, err_size, "%s", slug);
        return -1;
    }

    int capacity = cJSON_GetArraySize(songs);
    if (capacity < 0) capacity = 0;
    if (capacity > 2000) capacity = 2000;
    out->tracks = capacity ? (Track *)calloc((size_t)capacity, sizeof(Track)) : NULL;
    if (capacity && !out->tracks) {
        cJSON_Delete(root);
        snprintf(err, err_size, "%s", slug);
        return -1;
    }

    int index = 0;
    const cJSON *item = NULL;
    cJSON_ArrayForEach(item, songs) {
        if (out->count >= capacity) break;
        if (!cJSON_IsObject(item)) continue;

        Track t;
        memset(&t, 0, sizeof(t));
        t.number = index + 1;

        const cJSON *n = cJSON_GetObjectItem(item, "number");
        if (cJSON_IsNumber(n)) {
            int v = (int)n->valuedouble;
            if (v > 0 && v < 100000) t.number = v;
        }

        const cJSON *tname = cJSON_GetObjectItem(item, "name");
        if (cJSON_IsString(tname) && tname->valuestring && *tname->valuestring) {
            snprintf(t.name, sizeof(t.name), "%s", tname->valuestring);
        } else {
            snprintf(t.name, sizeof(t.name), "%s %d", slug, t.number);
        }

        const cJSON *file = cJSON_GetObjectItem(item, "file");
        if (!cJSON_IsString(file) || !safe_name(file->valuestring)) {
            /* A track without a usable file is still listed, but flagged. */
            t.has_file = 0;
            snprintf(t.file, sizeof(t.file), "%s/", dir);
        } else {
            path_join(t.file, sizeof(t.file), dir, file->valuestring);
            t.has_file = file_exists(t.file) && library_track_supported(t.file);
        }

        const cJSON *tid = cJSON_GetObjectItem(item, "id");
        if (cJSON_IsString(tid) && tid->valuestring && *tid->valuestring) {
            snprintf(t.id, sizeof(t.id), "%s", tid->valuestring);
        } else {
            snprintf(t.id, sizeof(t.id), "%s.%d", out->id, t.number);
        }
        if (track_id_taken(out, t.id)) {
            char unique[ID_MAX_LEN];
            int suffix = 2;
            do {
                snprintf(unique, sizeof(unique), "%.80s.%d", t.id, suffix++);
            } while (track_id_taken(out, unique) && suffix < 100);
            snprintf(t.id, sizeof(t.id), "%s", unique);
        }

        char lower[NAME_MAX_LEN];
        lowercase(lower, sizeof(lower), slug);
        char conv[NAME_MAX_LEN];
        snprintf(conv, sizeof(conv), "music_%s_%d.png", lower, t.number);
        const cJSON *tlogo = cJSON_GetObjectItem(item, "logo");
        resolve_logo(out, cJSON_IsString(tlogo) ? tlogo->valuestring : "", conv, t.logo, sizeof(t.logo));

        out->tracks[out->count++] = t;
        index++;
    }

    cJSON_Delete(root);

    if (out->count > 1) qsort(out->tracks, (size_t)out->count, sizeof(Track), cmp_track);
    return 0;
}

int library_scan(Library *lib) {
    memset(lib, 0, sizeof(*lib));

    char playlists_dir[PATH_MAX_LEN];
    snprintf(playlists_dir, sizeof(playlists_dir), "%s/PLAYLISTS", g_root);

    DIR *d = opendir(playlists_dir);
    if (!d) {
        snprintf(lib->last_error, sizeof(lib->last_error), "%s", playlists_dir);
        return -1;
    }

    int capacity = 32;
    lib->items = (Playlist *)calloc((size_t)capacity, sizeof(Playlist));
    if (!lib->items) {
        closedir(d);
        snprintf(lib->last_error, sizeof(lib->last_error), "%s", playlists_dir);
        return -1;
    }

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        char dir[PATH_MAX_LEN];
        path_join(dir, sizeof(dir), playlists_dir, ent->d_name);
        if (!dir_exists(dir)) continue;

        if (lib->count == capacity) {
            int next = capacity * 2;
            Playlist *grown = (Playlist *)realloc(lib->items, (size_t)next * sizeof(Playlist));
            if (!grown) break;
            memset(grown + capacity, 0, (size_t)(next - capacity) * sizeof(Playlist));
            lib->items = grown;
            capacity = next;
        }

        char err[192];
        err[0] = 0;
        Playlist pl;
        if (load_playlist(ent->d_name, dir, &pl, err, sizeof(err)) != 0) {
            lib->errors++;
            snprintf(lib->last_error, sizeof(lib->last_error), "%s", err);
            continue;
        }
        lib->items[lib->count++] = pl;
    }
    closedir(d);

    if (lib->count > 1) qsort(lib->items, (size_t)lib->count, sizeof(Playlist), cmp_playlist);
    for (int i = 0; i < lib->count; i++) lib->total_tracks += lib->items[i].count;

    if (lib->count == 0 && lib->errors == 0) lib->last_error[0] = 0;
    return 0;
}

void library_free(Library *lib) {
    if (!lib) return;
    for (int i = 0; i < lib->count; i++) free(lib->items[i].tracks);
    free(lib->items);
    memset(lib, 0, sizeof(*lib));
}

int library_find_playlist(const Library *lib, const char *id) {
    if (!lib || !id) return -1;
    for (int i = 0; i < lib->count; i++) {
        if (strcmp(lib->items[i].id, id) == 0) return i;
    }
    return -1;
}

int library_find_track(const Library *lib, const char *playlist_id, const char *track_id) {
    int p = library_find_playlist(lib, playlist_id);
    if (p < 0 || !track_id) return -1;
    for (int i = 0; i < lib->items[p].count; i++) {
        if (strcmp(lib->items[p].tracks[i].id, track_id) == 0) return i;
    }
    return -1;
}
