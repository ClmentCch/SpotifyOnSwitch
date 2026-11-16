#ifndef MUSICTT_APP_H
#define MUSICTT_APP_H

#include <SDL.h>

#include "library.h"
#include "player.h"
#include "settings.h"
#include "ui.h"

#define APP_W 1280
#define APP_H 720
#define SIDEBAR_W 236
#define HEADER_H 88
#define MINI_H 78

#define C_BG      ((Col){ 10, 10, 15, 255 })
#define C_PANEL   ((Col){ 17, 17, 25, 255 })
#define C_CARD    ((Col){ 23, 23, 33, 255 })
#define C_CARD2   ((Col){ 31, 31, 44, 255 })
#define C_TEXT    ((Col){ 242, 243, 247, 255 })
#define C_MUTED   ((Col){ 152, 158, 172, 255 })
#define C_DIM     ((Col){ 100, 106, 122, 255 })
#define C_ACCENT  ((Col){ 124, 92, 255, 255 })
#define C_ACCENT2 ((Col){ 53, 214, 196, 255 })
#define C_LINE    ((Col){ 40, 40, 54, 255 })

typedef enum { SCR_LANG = 0, SCR_SD_READY, SCR_RELAUNCH, SCR_MAIN } Screen;
typedef enum { PAGE_HOME = 0, PAGE_PLAYLISTS, PAGE_PLAYLIST, PAGE_LIBRARY, PAGE_SETTINGS, PAGE_COUNT } Page;

typedef enum {
    ACT_NONE = 0,
    ACT_NAV, ACT_OPEN_PLAYLIST, ACT_PLAY_TRACK, ACT_PLAY_ALL,
    ACT_TOGGLE, ACT_NEXT, ACT_PREV, ACT_REFRESH,
    ACT_LANG, ACT_CONFIRM_LANG, ACT_OK, ACT_QUIT, ACT_RETRY,
    ACT_SET_LANG, ACT_SET_VOLUME, ACT_SET_MOTION, ACT_SET_REFRESH,
    ACT_OPEN_NOW, ACT_CLOSE_NOW, ACT_SEEK, ACT_VOL_PLAYER, ACT_PLAY_PAIR
} Action;

typedef struct {
    int action;
    int arg;
    SDL_Rect rect;
} Hit;

typedef struct {
    char text[192];
    int kind;
    double t;
} Toast;

typedef struct {
    Screen screen;
    Page page;
    Settings cfg;
    Library lib;
    int playlist_index;
    int lang_sel;
    int running;
    int quit;

    double time;
    double page_anim;
    double now_anim;
    int now_open;

    Hit hits[192];
    int hit_count;

    Toast toasts[3];
    int toast_count;

    int focus;
    int focus_visible;
    double focus_t;

    int content_top, content_bottom;
    int view_top, view_bottom;

    float scroll[PAGE_COUNT];
    float scroll_target[PAGE_COUNT];

    int touch_down;
    int touch_x, touch_y, touch_last_y, touch_start_x, touch_start_y;
    int touch_moved;
    int touch_hit;
    int dragging_scroll;
    int dragging_seek;
    int dragging_volume;
    double seek_preview;
    int vol_preview;

    char cur_key[ID_MAX_LEN * 2];
    Col theme_top, theme_bottom, theme_accent;
    int theme_ready;
    int audio_ok;
    int needs_save;
    double last_save;
    double toast_time;
    char sd_error[192];
    int scanning;
} App;

extern App g_app;
extern SDL_Renderer *g_renderer;
extern SDL_Window *g_window;

void app_toast(const char *text, int kind);
void app_hit(int action, int arg, int x, int y, int w, int h);
void app_activate(int action, int arg);
void app_save(void);
void app_refresh(void);
void app_play_track(int playlist_index, int track_index);
void app_sync_current(void);
void app_set_language(Lang lang);

/* pages.c */
void pages_draw_sidebar(void);
void pages_draw_header(void);
void pages_draw_home(int x, int y, int w, int h);
void pages_draw_playlists(int x, int y, int w, int h);
void pages_draw_playlist(int x, int y, int w, int h);
void pages_draw_library(int x, int y, int w, int h);
void pages_draw_settings(int x, int y, int w, int h);
void pages_draw_mini(void);
void pages_draw_language(void);
void pages_draw_sd_ready(void);
void pages_draw_relaunch(void);

/* nowplaying.c */
void now_draw(void);
void now_update(double dt);

int app_has_track(void);
int app_run(void);

#endif
