#ifndef MUSICTT_SETTINGS_H
#define MUSICTT_SETTINGS_H

#include "common.h"
#include "i18n.h"

#define RECENT_MAX 8

typedef struct {
    int version;
    int language;              /* -1 until the user has chosen */
    int volume;                /* 0..100 */
    int animations;            /* 0/1 */
    char last_playlist[ID_MAX_LEN];
    char last_track[ID_MAX_LEN];
    char recent[RECENT_MAX][ID_MAX_LEN * 2];
    int recent_count;
} Settings;

/* Test hook: override where settings.json lives. */
void settings_set_paths(const char *config_dir, const char *file);

void settings_defaults(Settings *s);
/* 0 when settings.json was read and looked sane, -1 otherwise. */
int settings_load(Settings *s);
/* 0 on success. Creates CONFIG/ when needed, never deletes user data. */
int settings_save(const Settings *s);
void settings_push_recent(Settings *s, const char *key);

#endif
