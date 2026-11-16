#ifndef MUSICTT_I18N_H
#define MUSICTT_I18N_H

#include "common.h"

typedef enum {
    LANG_FR = 0, LANG_EN, LANG_ES, LANG_DE, LANG_IT, LANG_PT,
    LANG_JA, LANG_KO, LANG_ZH, LANG_RU, LANG_NL, LANG_PL,
    LANG_COUNT
} Lang;

typedef enum {
    S_TAGLINE = 0,
    S_HOME, S_PLAYLISTS, S_LIBRARY, S_SETTINGS,
    S_WELCOME_TITLE, S_WELCOME_SUB,
    S_HOME_PLAYLISTS, S_HOME_RECENT, S_HOME_INFO,
    S_RECENT_EMPTY,
    S_TRACKS_COUNT, S_PLAYLISTS_COUNT, S_LIBRARY_STATS,
    S_LIBRARY_EMPTY_TITLE, S_LIBRARY_EMPTY_BODY, S_PLAYLISTS_EMPTY, S_PLAYLIST_EMPTY,
    S_BTN_REFRESH, S_BTN_OK, S_BTN_CONFIRM, S_BTN_QUIT, S_BTN_RETRY,
    S_LANG_TITLE, S_LANG_BODY,
    S_SD_TITLE, S_SD_BODY, S_SD_HINT,
    S_RELAUNCH_TITLE, S_RELAUNCH_BODY,
    S_NOW_PLAYING, S_VOLUME,
    S_SET_LANGUAGE, S_SET_MOTION, S_SET_REFRESH, S_SET_ABOUT,
    S_REFRESHED, S_FORMATS, S_STORAGE, S_ON, S_OFF,
    S_LOADING, S_PLAYING, S_PAUSED,
    S_ERR_SD, S_ERR_SD_BODY, S_ERR_JSON, S_ERR_JSON_BODY,
    S_ERR_AUDIO, S_ERR_AUDIO_BODY,
    S_ERR_TRACK_MISSING, S_ERR_TRACK_FORMAT, S_ERR_TRACK_READ, S_ERR_TRACK_LONG,
    S_TOAST_SCANNING, S_TOAST_PLAYLIST_ERROR,
    S_ABOUT_BODY, S_SONGS, S_PLAY_ALL, S_MISSING,
    S_COUNT
} StrId;

/* The active language, set by the settings module at startup. */
extern Lang g_lang;

const char *i18n_get(StrId id);
const char *i18n_get_lang(StrId id, Lang lang);
/* printf-formatted with a single integer, rotating buffers. */
const char *i18n_num(StrId id, int value);
const char *i18n_num2(StrId id, int first, int second);
/* printf-formatted with a single string, rotating buffers. */
const char *i18n_str(StrId id, const char *value);

const char *i18n_native_name(Lang lang);
const char *i18n_code(Lang lang);
/* TrueType collection face index used for this language's main script. */
int i18n_cjk_face(Lang lang);
int i18n_uses_cjk(Lang lang);

#endif
