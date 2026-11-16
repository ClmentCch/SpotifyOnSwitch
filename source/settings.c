#include "settings.h"

#include <sys/stat.h>

#include "cJSON.h"

static char g_path[PATH_MAX_LEN] = MUSICTT_SETTINGS;
static char g_config_dir[PATH_MAX_LEN] = MUSICTT_CONFIG;

void settings_set_paths(const char *config_dir, const char *file) {
    if (config_dir) snprintf(g_config_dir, sizeof(g_config_dir), "%s", config_dir);
    if (file) snprintf(g_path, sizeof(g_path), "%s", file);
}

void settings_defaults(Settings *s) {
    memset(s, 0, sizeof(*s));
    s->version = 1;
    s->language = -1;
    s->volume = 70;
    s->animations = 1;
}

static int quiet_mkdir(const char *path) {
#ifdef _WIN32
    (void)path;
    return 0;
#else
    if (mkdir(path, 0777) == 0) return 0;
    /* Already there is fine; anything else is reported by the caller's stat. */
    return 0;
#endif
}

static int read_whole_file(const char *path, char **out, long *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    long size = ftell(f);
    if (size < 0 || size > 512 * 1024) { fclose(f); return -1; }
    rewind(f);
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) { fclose(f); return -1; }
    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (got != (size_t)size) { free(buf); return -1; }
    buf[size] = 0;
    *out = buf;
    *out_size = size;
    return 0;
}

static void copy_field(char *dst, size_t dst_size, const cJSON *item) {
    if (!cJSON_IsString(item) || !item->valuestring) return;
    snprintf(dst, dst_size, "%s", item->valuestring);
}

int settings_load(Settings *s) {
    settings_defaults(s);

    char *text = NULL;
    long size = 0;
    if (read_whole_file(g_path, &text, &size) != 0) return -1;

    cJSON *root = cJSON_ParseWithLength(text, (size_t)size);
    free(text);
    if (!root) return -1;

    int ok = 0;
    if (cJSON_IsObject(root)) {
        const cJSON *lang = cJSON_GetObjectItem(root, "language");
        const cJSON *vol = cJSON_GetObjectItem(root, "volume");
        const cJSON *anim = cJSON_GetObjectItem(root, "animations");
        if (cJSON_IsNumber(lang)) s->language = (int)lang->valuedouble;
        if (cJSON_IsNumber(vol)) s->volume = iclamp((int)vol->valuedouble, 0, 100);
        if (cJSON_IsBool(anim)) s->animations = cJSON_IsTrue(anim) ? 1 : 0;
        else if (cJSON_IsNumber(anim)) s->animations = anim->valuedouble != 0.0;
        copy_field(s->last_playlist, sizeof(s->last_playlist), cJSON_GetObjectItem(root, "last_playlist"));
        copy_field(s->last_track, sizeof(s->last_track), cJSON_GetObjectItem(root, "last_track"));

        const cJSON *recent = cJSON_GetObjectItem(root, "recent");
        if (cJSON_IsArray(recent)) {
            const cJSON *it = NULL;
            cJSON_ArrayForEach(it, recent) {
                if (!cJSON_IsString(it) || !it->valuestring) continue;
                if (s->recent_count >= RECENT_MAX) break;
                snprintf(s->recent[s->recent_count], sizeof(s->recent[0]), "%s", it->valuestring);
                s->recent_count++;
            }
        }
        if (s->language >= LANG_COUNT) s->language = LANG_EN;
        ok = 1;
    }

    cJSON_Delete(root);
    return ok ? 0 : -1;
}

int settings_save(const Settings *s) {
    quiet_mkdir(g_config_dir);

    cJSON *root = cJSON_CreateObject();
    if (!root) return -1;

    cJSON_AddNumberToObject(root, "version", 1);
    cJSON_AddNumberToObject(root, "language", s->language);
    cJSON_AddNumberToObject(root, "volume", iclamp(s->volume, 0, 100));
    cJSON_AddBoolToObject(root, "animations", s->animations != 0);
    cJSON_AddStringToObject(root, "last_playlist", s->last_playlist);
    cJSON_AddStringToObject(root, "last_track", s->last_track);

    cJSON *recent = cJSON_AddArrayToObject(root, "recent");
    if (recent) {
        for (int i = 0; i < s->recent_count && i < RECENT_MAX; i++) {
            cJSON_AddItemToArray(recent, cJSON_CreateString(s->recent[i]));
        }
    }

    char *text = cJSON_Print(root);
    cJSON_Delete(root);
    if (!text) return -1;

    /* Write to a temporary file then rename, so a power loss never leaves a
       half-written settings.json behind. */
    char tmp[PATH_MAX_LEN];
    snprintf(tmp, sizeof(tmp), "%s.tmp", g_path);
    FILE *f = fopen(tmp, "wb");
    if (!f) { free(text); return -1; }
    size_t len = strlen(text);
    size_t wrote = fwrite(text, 1, len, f);
    free(text);
    if (fclose(f) != 0 || wrote != len) {
        remove(tmp);
        return -1;
    }
    remove(g_path);
    if (rename(tmp, g_path) != 0) {
        remove(tmp);
        return -1;
    }
    return 0;
}

void settings_push_recent(Settings *s, const char *key) {
    if (!key || !*key) return;
    int found = -1;
    for (int i = 0; i < s->recent_count; i++) {
        if (strcmp(s->recent[i], key) == 0) { found = i; break; }
    }
    if (found == 0) return;
    if (found > 0) {
        for (int i = found; i > 0; i--) memcpy(s->recent[i], s->recent[i - 1], sizeof(s->recent[0]));
    } else {
        int last = s->recent_count < RECENT_MAX ? s->recent_count : RECENT_MAX - 1;
        for (int i = last; i > 0; i--) memcpy(s->recent[i], s->recent[i - 1], sizeof(s->recent[0]));
        if (s->recent_count < RECENT_MAX) s->recent_count++;
    }
    snprintf(s->recent[0], sizeof(s->recent[0]), "%s", key);
}
