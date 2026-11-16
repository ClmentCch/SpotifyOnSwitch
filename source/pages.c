#include "app.h"

#include <math.h>

#define CX (SIDEBAR_W + 36)
#define CW (APP_W - SIDEBAR_W - 72)

static void fmt_time(char *buf, size_t n, double seconds) {
    if (seconds < 0) seconds = 0;
    int total = (int)seconds;
    int h = total / 3600;
    int m = (total % 3600) / 60;
    int s = total % 60;
    if (h > 0) snprintf(buf, n, "%d:%02d:%02d", h, m, s);
    else snprintf(buf, n, "%02d:%02d", m, s);
}

static int find_track_by_key(const char *key, int *pi, int *ti) {
    for (int p = 0; p < g_app.lib.count; p++) {
        for (int t = 0; t < g_app.lib.items[p].count; t++) {
            char candidate[ID_MAX_LEN * 2];
            snprintf(candidate, sizeof(candidate), "%s/%s", g_app.lib.items[p].id, g_app.lib.items[p].tracks[t].id);
            if (strcmp(candidate, key) == 0) {
                *pi = p;
                *ti = t;
                return 1;
            }
        }
    }
    return 0;
}

static void section_label(int x, int y, const char *text) {
    ui_text(F_LABEL, x, y, C_TEXT, text);
}

static void draw_button(int x, int y, int w, int h, const char *label, Col bg, Col fg) {
    ui_round(x, y, w, h, h / 2, bg);
    int tw = ui_text_w(F_BODY, label);
    ui_text(F_BODY, x + (w - tw) / 2, y + (h - ui_text_h(F_BODY)) / 2 - 1, fg, label);
}

static void draw_play_pill(int x, int y, int w, int h, const char *label) {
    ui_round(x, y, w, h, h / 2, C_ACCENT);
    int icon = 16;
    int tw = ui_text_w(F_BODY, label);
    int total = icon + 10 + tw;
    int sx = x + (w - total) / 2;
    ui_icon(IC_PLAY, sx - 2, y + (h - icon) / 2, icon, ui_rgb(255, 255, 255));
    ui_text(F_BODY, sx + icon + 10, y + (h - ui_text_h(F_BODY)) / 2 - 1, ui_rgb(255, 255, 255), label);
}

/* ------------------------------------------------------------------ sidebar */

void pages_draw_sidebar(void) {
    ui_rect(0, 0, SIDEBAR_W, APP_H, C_PANEL);
    ui_rect(SIDEBAR_W - 1, 0, 1, APP_H, C_LINE);

    ui_round(22, 22, 44, 44, 14, C_ACCENT);
    ui_note_mark(44, 44, 26, ui_rgb(255, 255, 255));
    ui_text(F_H1, 78, 26, C_TEXT, "Musictt");
    ui_text(F_TINY, 78, 60, C_MUTED, i18n_get(S_TAGLINE));

    static const StrId nav_ids[4] = { S_HOME, S_PLAYLISTS, S_LIBRARY, S_SETTINGS };
    static const int nav_icons[4] = { IC_HOME, IC_PLAYLIST, IC_LIBRARY, IC_SETTINGS };

    int y = 116;
    for (int i = 0; i < 4; i++) {
        int active = (int)g_app.page == i;
        if (active) {
            ui_round(14, y, SIDEBAR_W - 28, 48, 14, C_CARD);
            ui_round(14, y + 10, 3, 28, 2, C_ACCENT);
        }
        ui_icon(nav_icons[i], 34, y + 13, 22, active ? C_TEXT : C_MUTED);
        ui_text(F_BODY, 68, y + 14, active ? C_TEXT : C_MUTED, i18n_get(nav_ids[i]));
        app_hit(ACT_NAV, i, 14, y, SIDEBAR_W - 28, 48);
        y += 56;
    }

    ui_text(F_TINY, 24, APP_H - 66, C_DIM, "sdmc:/MUSIC");
    ui_text(F_TINY, 24, APP_H - 44, C_DIM, "Musictt " MUSICTT_VERSION);
}

/* ------------------------------------------------------------------- header */

void pages_draw_header(void) {
    const char *title = "";
    const char *sub = "";
    switch (g_app.page) {
        case PAGE_HOME: title = i18n_get(S_HOME); sub = i18n_get(S_WELCOME_SUB); break;
        case PAGE_PLAYLISTS: title = i18n_get(S_PLAYLISTS); sub = i18n_num(S_PLAYLISTS_COUNT, g_app.lib.count); break;
        case PAGE_PLAYLIST:
            if (g_app.playlist_index < g_app.lib.count) {
                title = g_app.lib.items[g_app.playlist_index].name;
                sub = i18n_num(S_TRACKS_COUNT, g_app.lib.items[g_app.playlist_index].count);
            }
            break;
        case PAGE_LIBRARY: title = i18n_get(S_LIBRARY); sub = i18n_num(S_TRACKS_COUNT, g_app.lib.total_tracks); break;
        case PAGE_SETTINGS: title = i18n_get(S_SETTINGS); sub = i18n_get(S_ABOUT_BODY); break;
        default: break;
    }
    /* Only the first line of the settings subtitle makes sense here. */
    if (g_app.page == PAGE_SETTINGS) sub = "Musictt " MUSICTT_VERSION;
    ui_text(F_H1, CX, 20, C_TEXT, title);
    ui_text(F_SMALL, CX, 56, C_MUTED, sub);

    if (g_app.page != PAGE_SETTINGS) {
        int bx = APP_W - 76, by = 26;
        ui_round(bx, by, 40, 40, 14, C_CARD);
        ui_icon(IC_REFRESH, bx + 10, by + 10, 20, C_TEXT);
        app_hit(ACT_REFRESH, 0, bx, by, 40, 40);
    }
    ui_rect(SIDEBAR_W, HEADER_H - 1, APP_W - SIDEBAR_W, 1, C_LINE);
}

/* --------------------------------------------------------------------- home */

void pages_draw_home(int x, int y, int w, int h) {
    float scroll = g_app.scroll[PAGE_HOME];
    (void)h;

    int hero_h = 148;
    int hero_y = y + (int)scroll;
    ui_round(x, hero_y, w, hero_h, 24, ui_rgb(34, 27, 64));
    ui_round(x + w - 260, hero_y + 20, 240, hero_h - 40, 20, ui_rgb(45, 35, 84));
    ui_note_mark(x + 74, hero_y + hero_h / 2, 62, ui_alpha(C_ACCENT2, 220));
    ui_text(F_HERO, x + 128, hero_y + 26, C_TEXT, i18n_get(S_WELCOME_TITLE));
    ui_text(F_BODY, x + 132, hero_y + 84, ui_alpha(C_TEXT, 210), i18n_get(S_WELCOME_SUB));
    ui_text(F_SMALL, x + 132, hero_y + 110, ui_alpha(C_TEXT, 170),
            i18n_num2(S_LIBRARY_STATS, g_app.lib.count, g_app.lib.total_tracks));
    ui_icon(IC_NOTE, x + w - 150, hero_y + hero_h / 2 - 22, 44, ui_alpha(C_TEXT, 60));

    int sy = hero_y + hero_h + 26;
    section_label(x, sy, i18n_get(S_HOME_PLAYLISTS));

    int card_y = sy + 32;
    int card_w = 296, card_h = 140, gap = 16;
    int shown = imin(3, g_app.lib.count);
    for (int i = 0; i < shown; i++) {
        int bx = x + i * (card_w + gap);
        Playlist *pl = &g_app.lib.items[i];
        ui_round(bx, card_y, card_w, card_h, 20, C_CARD);
        ui_art(bx + 18, card_y + 18, 72, 16, pl->logo, C_ACCENT);
        ui_text(F_BODY, bx + 104, card_y + 28, C_TEXT, pl->name);
        ui_text(F_SMALL, bx + 104, card_y + 52, C_MUTED, i18n_num(S_TRACKS_COUNT, pl->count));
        int sp = 44;
        ui_round(bx + 18, card_y + card_h - sp - 16, sp, sp, 14, C_CARD2);
        ui_icon(IC_PLAY, bx + 31, card_y + card_h - sp - 3, 18, C_TEXT);
        app_hit(ACT_OPEN_PLAYLIST, i, bx, card_y, card_w, card_h);
    }

    int ry = card_y + card_h + 26;
    section_label(x, ry, i18n_get(S_HOME_RECENT));
    int row_y = ry + 32;
    int rows = 0;
    for (int i = 0; i < g_app.cfg.recent_count && i < 6; i++) {
        int pi = -1, ti = -1;
        if (!find_track_by_key(g_app.cfg.recent[i], &pi, &ti)) continue;
        Track *tr = &g_app.lib.items[pi].tracks[ti];
        int by = row_y + rows * 68;
        ui_round(x, by, w, 58, 16, C_CARD);
        ui_art(x + 8, by + 5, 48, 12, tr->logo, C_ACCENT);
        ui_text(F_BODY, x + 70, by + 10, C_TEXT, tr->name);
        ui_text(F_SMALL, x + 70, by + 32, C_MUTED, g_app.lib.items[pi].name);
        ui_round(x + w - 50, by + 9, 40, 40, 13, C_CARD2);
        ui_icon(IC_PLAY, x + w - 39, by + 20, 16, C_TEXT);
        app_hit(ACT_PLAY_PAIR, (pi << 12) | ti, x, by, w, 58);
        rows++;
    }
    if (rows == 0) {
        ui_round(x, row_y, w, 58, 16, C_CARD);
        ui_text(F_SMALL, x + 20, row_y + 20, C_MUTED, i18n_get(S_RECENT_EMPTY));
        row_y += 68;
    }

    int content_h = row_y + rows * 68 - (y + (int)scroll) + 24;
    float min_scroll = (float)(h - content_h);
    if (min_scroll > 0) min_scroll = 0;
    if (g_app.scroll_target[PAGE_HOME] < min_scroll) g_app.scroll_target[PAGE_HOME] = min_scroll;
    if (g_app.scroll_target[PAGE_HOME] > 0) g_app.scroll_target[PAGE_HOME] = 0;
}

/* ---------------------------------------------------------------- playlists */

void pages_draw_playlists(int x, int y, int w, int h) {
    float scroll = g_app.scroll[PAGE_PLAYLISTS];
    int card_w = 300, card_h = 168, gap = 18;
    int per_row = imax(1, (w + gap) / (card_w + gap));
    int rows = (g_app.lib.count + per_row - 1) / per_row;
    if (rows < 1) rows = 1;

    if (g_app.lib.count == 0) {
        ui_round(x, y + 10, w, 150, 22, C_CARD);
        ui_icon(IC_PLAYLIST, x + w / 2 - 26, y + 30, 52, C_DIM);
        ui_text_c(F_BODY, x + w / 2, y + 96, C_MUTED, i18n_get(S_PLAYLISTS_EMPTY));
    }

    for (int i = 0; i < g_app.lib.count; i++) {
        int row = i / per_row, col = i % per_row;
        int bx = x + col * (card_w + gap);
        int by = y + 8 + row * (card_h + gap) + (int)scroll;
        Playlist *pl = &g_app.lib.items[i];
        ui_round(bx, by, card_w, card_h, 22, C_CARD);
        ui_art(bx + 20, by + 20, 92, 20, pl->logo, C_ACCENT);
        ui_text(F_SMALL, bx + 126, by + 34, C_MUTED, i18n_get(S_PLAYLISTS));
        ui_text(F_LABEL, bx + 126, by + 56, C_TEXT, pl->name);
        ui_text(F_SMALL, bx + 126, by + 84, C_MUTED, i18n_num(S_TRACKS_COUNT, pl->count));
        ui_round(bx + 20, by + card_h - 48, 44, 32, 12, C_CARD2);
        ui_icon(IC_PLAY, bx + 33, by + card_h - 38, 16, C_TEXT);
        app_hit(ACT_OPEN_PLAYLIST, i, bx, by, card_w, card_h);
    }

    int content_h = 8 + rows * (card_h + gap) + 16;
    float min_scroll = (float)(h - content_h);
    if (min_scroll > 0) min_scroll = 0;
    if (g_app.scroll_target[PAGE_PLAYLISTS] < min_scroll) g_app.scroll_target[PAGE_PLAYLISTS] = min_scroll;
    if (g_app.scroll_target[PAGE_PLAYLISTS] > 0) g_app.scroll_target[PAGE_PLAYLISTS] = 0;
}

/* ----------------------------------------------------------------- playlist */

void pages_draw_playlist(int x, int y, int w, int h) {
    ui_push_clip(x, y, w, h);
    g_app.view_top = y;
    g_app.view_bottom = y + h;
    if (g_app.playlist_index < 0 || g_app.playlist_index >= g_app.lib.count) {
        ui_text(F_BODY, x + 36, y + 40, C_MUTED, i18n_get(S_PLAYLISTS_EMPTY));
        ui_pop_clip();
        return;
    }
    Playlist *pl = &g_app.lib.items[g_app.playlist_index];
    float scroll = g_app.scroll[PAGE_PLAYLIST];
    int ix = x + 36;
    int iy = y + 10 + (int)scroll;

    ui_round(ix, iy, w - 72, 158, 24, C_CARD);
    ui_art(ix + 20, iy + 20, 118, 22, pl->logo, C_ACCENT);
    ui_text(F_H1, ix + 158, iy + 34, C_TEXT, pl->name);
    ui_text(F_SMALL, ix + 160, iy + 72, C_MUTED, i18n_num(S_TRACKS_COUNT, pl->count));
    if (pl->count > 0) {
        int bw = 188;
        int bx = ix + w - 72 - bw - 24;
        draw_play_pill(bx, iy + 100, bw, 44, i18n_get(S_PLAY_ALL));
        app_hit(ACT_PLAY_ALL, 0, bx, iy + 100, bw, 44);
    }

    int list_y = iy + 184;
    if (pl->count == 0) {
        ui_round(ix, list_y, w - 72, 96, 18, C_CARD);
        ui_text(F_SMALL, ix + 24, list_y + 38, C_MUTED, i18n_get(S_PLAYLIST_EMPTY));
    }
    for (int i = 0; i < pl->count; i++) {
        Track *tr = &pl->tracks[i];
        int by = list_y + i * 76;
        if (by > y + h + 80 || by + 68 < y - 80) continue;
        Col row_bg = C_CARD;
        if (strcmp(g_app.cur_key, "") != 0) {
            char key[ID_MAX_LEN * 2];
            snprintf(key, sizeof(key), "%s/%s", pl->id, tr->id);
            if (strcmp(key, g_app.cur_key) == 0) row_bg = ui_rgb(33, 29, 58);
        }
        ui_round(ix, by, w - 72, 68, 16, row_bg);
        ui_art(ix + 8, by + 8, 52, 12, tr->logo, C_ACCENT);
        ui_text(F_BODY, ix + 74, by + 12, tr->has_file ? C_TEXT : C_DIM, tr->name);
        char sub[160];
        if (tr->has_file) {
            snprintf(sub, sizeof(sub), "%02d", tr->number);
            ui_text(F_SMALL, ix + 74, by + 36, C_MUTED, sub);
        } else {
            ui_text(F_SMALL, ix + 74, by + 36, ui_rgb(255, 120, 120), i18n_get(S_MISSING));
            ui_icon(IC_WARNING, ix + 74 + 6, by + 4, 14, ui_rgb(255, 120, 120));
        }
        int bs = 44;
        int bx = ix + w - 72 - bs - 14;
        ui_round(bx, by + 12, bs, bs, 14, tr->has_file ? C_ACCENT : C_CARD2);
        ui_icon(IC_PLAY, bx + 14, by + 22, 18, tr->has_file ? ui_rgb(255, 255, 255) : C_DIM);
        app_hit(ACT_PLAY_TRACK, i, ix, by, w - 72, 68);
    }

    int content_h = 184 + pl->count * 76 + 16;
    float min_scroll = (float)(h - content_h);
    if (min_scroll > 0) min_scroll = 0;
    if (g_app.scroll_target[PAGE_PLAYLIST] < min_scroll) g_app.scroll_target[PAGE_PLAYLIST] = min_scroll;
    if (g_app.scroll_target[PAGE_PLAYLIST] > 0) g_app.scroll_target[PAGE_PLAYLIST] = 0;
    ui_pop_clip();
}

/* ------------------------------------------------------------------ library */

void pages_draw_library(int x, int y, int w, int h) {
    float scroll = g_app.scroll[PAGE_LIBRARY];
    int sy = y + (int)scroll;

    ui_round(x, sy, w, 96, 22, C_CARD);
    ui_text(F_SMALL, x + 24, sy + 18, C_MUTED, i18n_get(S_LIBRARY));
    ui_text(F_LABEL, x + 24, sy + 44, C_TEXT,
            i18n_num2(S_LIBRARY_STATS, g_app.lib.count, g_app.lib.total_tracks));
    ui_text(F_TINY, x + 24, sy + 72, C_DIM, i18n_get(S_FORMATS));
    ui_text(F_TINY, x + w - 24 - ui_text_w(F_TINY, i18n_get(S_STORAGE)), sy + 72, C_DIM, i18n_get(S_STORAGE));

    int ry = sy + 112;
    if (g_app.lib.total_tracks == 0) {
        ui_round(x, ry, w, 170, 22, C_CARD);
        ui_icon(IC_NOTE, x + w / 2 - 24, ry + 26, 48, C_DIM);
        ui_text_c(F_LABEL, x + w / 2, ry + 88, C_TEXT, i18n_get(S_LIBRARY_EMPTY_TITLE));
        ui_text_c(F_SMALL, x + w / 2, ry + 118, C_MUTED, i18n_get(S_LIBRARY_EMPTY_BODY));
    }

    int row = 0;
    for (int p = 0; p < g_app.lib.count; p++) {
        Playlist *pl = &g_app.lib.items[p];
        int hy = ry + row * 64;
        if (hy + 24 >= y && hy <= y + h) {
            ui_text(F_LABEL, x + 4, hy + 18, C_TEXT, pl->name);
            ui_text(F_TINY, x + 4 + ui_text_w(F_LABEL, pl->name) + 12, hy + 22, C_MUTED,
                    i18n_num(S_TRACKS_COUNT, pl->count));
            app_hit(ACT_OPEN_PLAYLIST, p, x, hy, w, 40);
        }
        row++;
        for (int t = 0; t < pl->count; t++) {
            int by = ry + row * 64;
            row++;
            if (by > y + h + 70 || by + 58 < y - 70) continue;
            Track *tr = &pl->tracks[t];
            ui_round(x, by, w, 56, 14, C_CARD);
            ui_art(x + 6, by + 6, 44, 10, tr->logo, C_ACCENT);
            ui_text(F_BODY, x + 62, by + 9, tr->has_file ? C_TEXT : C_DIM, tr->name);
            ui_text(F_TINY, x + 62, by + 32, C_MUTED, pl->name);
            ui_round(x + w - 44, by + 10, 36, 36, 12, C_CARD2);
            ui_icon(IC_PLAY, x + w - 34, by + 19, 14, C_TEXT);
            app_hit(ACT_PLAY_PAIR, (p << 12) | t, x, by, w, 56);
        }
    }

    int content_h = 112 + (row + 1) * 64 + 20;
    float min_scroll = (float)(h - content_h);
    if (min_scroll > 0) min_scroll = 0;
    if (g_app.scroll_target[PAGE_LIBRARY] < min_scroll) g_app.scroll_target[PAGE_LIBRARY] = min_scroll;
    if (g_app.scroll_target[PAGE_LIBRARY] > 0) g_app.scroll_target[PAGE_LIBRARY] = 0;
}

/* ----------------------------------------------------------------- settings */

void pages_draw_settings(int x, int y, int w, int h) {
    float scroll = g_app.scroll[PAGE_SETTINGS];
    int sy = y + (int)scroll;

    /* Language */
    ui_round(x, sy, w, 244, 22, C_CARD);
    ui_text(F_LABEL, x + 24, sy + 20, C_TEXT, i18n_get(S_SET_LANGUAGE));
    int chip_w = (w - 48 - 3 * 12) / 4;
    int chip_h = 50;
    for (int i = 0; i < LANG_COUNT; i++) {
        int r = i / 4, c = i % 4;
        int bx = x + 24 + c * (chip_w + 12);
        int by = sy + 56 + r * (chip_h + 10);
        int active = (int)g_lang == i;
        ui_round(bx, by, chip_w, chip_h, 12, active ? ui_rgb(46, 38, 86) : C_CARD2);
        ui_flag((Lang)i, bx + 12, by + 12, 32, 24);
        ui_text(F_SMALL, bx + 54, by + 16, active ? C_TEXT : C_MUTED, i18n_native_name((Lang)i));
        app_hit(ACT_SET_LANG, i, bx, by, chip_w, chip_h);
    }

    /* Volume */
    int vy = sy + 260;
    ui_round(x, vy, w, 96, 22, C_CARD);
    ui_text(F_LABEL, x + 24, vy + 18, C_TEXT, i18n_get(S_VOLUME));
    char vbuf[16];
    snprintf(vbuf, sizeof(vbuf), "%d%%", g_app.dragging_volume ? g_app.vol_preview : g_app.cfg.volume);
    ui_text_r(F_SMALL, x + w - 24, vy + 22, C_MUTED, vbuf);
    int sx = x + 24, sw = w - 48, sh = 10, svy = vy + 60;
    ui_round(sx, svy, sw, sh, sh / 2, C_CARD2);
    int vol = g_app.dragging_volume ? g_app.vol_preview : g_app.cfg.volume;
    int fill = sw * vol / 100;
    if (fill > 0) ui_round(sx, svy, fill, sh, sh / 2, C_ACCENT);
    ui_circle(sx + fill, svy + sh / 2, 9, C_TEXT);
    app_hit(ACT_SET_VOLUME, 0, sx - 10, svy - 14, sw + 20, 38);

    /* Motion */
    int my = vy + 112;
    ui_round(x, my, w, 64, 18, C_CARD);
    ui_text(F_BODY, x + 24, my + 22, C_TEXT, i18n_get(S_SET_MOTION));
    int toggle_w = 62, tx = x + w - toggle_w - 24, ty = my + 16;
    ui_round(tx, ty, toggle_w, 32, 16, g_app.cfg.animations ? C_ACCENT : C_CARD2);
    int knob_x = g_app.cfg.animations ? tx + toggle_w - 28 : tx + 4;
    ui_circle(knob_x + 14, ty + 16, 12, C_TEXT);
    app_hit(ACT_SET_MOTION, 0, tx, ty, toggle_w, 32);

    /* Refresh */
    int ry = my + 80;
    int bw = 300;
    ui_round(x, ry, bw, 52, 16, C_CARD);
    ui_icon(IC_REFRESH, x + 18, ry + 15, 22, C_TEXT);
    ui_text(F_BODY, x + 52, ry + 16, C_TEXT, i18n_get(S_SET_REFRESH));
    app_hit(ACT_SET_REFRESH, 0, x, ry, bw, 52);

    /* About */
    int ay = ry + 72;
    ui_round(x, ay, w, 148, 22, C_CARD);
    ui_text(F_LABEL, x + 24, ay + 18, C_TEXT, i18n_get(S_SET_ABOUT));
    ui_text(F_SMALL, x + 24, ay + 46, C_MUTED, "Musictt " MUSICTT_VERSION);
    ui_text_wrap(F_SMALL, x + 24, ay + 70, w - 48, 22, C_MUTED, i18n_get(S_ABOUT_BODY));
    ui_text(F_TINY, x + 24, ay + 122, C_DIM, i18n_get(S_FORMATS));

    int content_h = 260 + 112 + 64 + 80 + 52 + 20 + 148 + 24;
    float min_scroll = (float)(h - content_h);
    if (min_scroll > 0) min_scroll = 0;
    if (g_app.scroll_target[PAGE_SETTINGS] < min_scroll) g_app.scroll_target[PAGE_SETTINGS] = min_scroll;
    if (g_app.scroll_target[PAGE_SETTINGS] > 0) g_app.scroll_target[PAGE_SETTINGS] = 0;
}

/* ------------------------------------------------------------- mini player */

void pages_draw_mini(void) {
    const PlayerItem *cur = player_current();
    if (!cur) return;
    int y = APP_H - MINI_H;
    g_app.view_top = y;
    g_app.view_bottom = APP_H;

    ui_rect(0, y, APP_W, 1, C_LINE);
    ui_rect(0, y + 1, APP_W, MINI_H - 1, ui_rgb(14, 14, 21));

    double dur = player_duration();
    double pos = player_position();
    float ratio = dur > 0 ? (float)(pos / dur) : 0.0f;
    if (g_app.dragging_seek) ratio = 0;
    ui_rect(0, y, APP_W, 3, C_CARD);
    ui_rect(0, y, (int)(APP_W * fclampf(ratio, 0.0f, 1.0f)), 3, C_ACCENT);

    ui_art(20, y + 14, 50, 12, cur->logo, C_ACCENT);
    ui_text(F_BODY, 84, y + 16, C_TEXT, cur->title);
    char sub[192];
    PlayerStatus st = player_status();
    const char *state = "";
    if (st == PLAYER_LOADING) state = i18n_get(S_LOADING);
    else if (st == PLAYER_PAUSED) state = i18n_get(S_PAUSED);
    snprintf(sub, sizeof(sub), "%s · %s", cur->playlist, state);
    ui_text(F_TINY, 84, y + 42, C_MUTED, sub);

    int by = y + 19;
    ui_round(APP_W - 196, by, 40, 40, 13, C_CARD);
    ui_icon(IC_PREV, APP_W - 186, by + 10, 20, C_TEXT);
    ui_round(APP_W - 144, by, 40, 40, 13, C_ACCENT);
    ui_icon(st == PLAYER_PLAYING ? IC_PAUSE : IC_PLAY, APP_W - 133, by + 10, 18, ui_rgb(255, 255, 255));
    ui_round(APP_W - 92, by, 40, 40, 13, C_CARD);
    ui_icon(IC_NEXT, APP_W - 82, by + 10, 20, C_TEXT);
    ui_text_r(F_TINY, APP_W - 216, y + 30, C_MUTED, i18n_get(S_NOW_PLAYING));

    app_hit(ACT_OPEN_NOW, 0, 0, y, APP_W - 220, MINI_H);
    app_hit(ACT_PREV, 0, APP_W - 196, by, 40, 40);
    app_hit(ACT_TOGGLE, 0, APP_W - 144, by, 40, 40);
    app_hit(ACT_NEXT, 0, APP_W - 92, by, 40, 40);
}

/* ------------------------------------------------------------ setup screens */

static void setup_card(int w, int h, const char *title, const char *body, const char *hint) {
    int x = (APP_W - w) / 2, y = (APP_H - h) / 2;
    ui_round(x, y, w, h, 26, C_CARD);
    ui_round(x + 28, y + 28, 56, 56, 18, C_ACCENT);
    ui_note_mark(x + 56, y + 56, 30, ui_rgb(255, 255, 255));
    ui_text_wrap(F_H1, x + 28, y + 104, w - 56, 40, C_TEXT, title);
    ui_text_wrap(F_BODY, x + 28, y + 178, w - 56, 26, C_MUTED, body);
    if (hint) ui_text_wrap(F_SMALL, x + 28, y + h - 132, w - 56, 22, C_DIM, hint);
}

void pages_draw_language(void) {
    ui_text_c(F_HERO, APP_W / 2, 54, C_TEXT, i18n_get(S_LANG_TITLE));
    ui_text_c(F_BODY, APP_W / 2, 116, C_MUTED, i18n_get(S_LANG_BODY));
    ui_note_mark(APP_W / 2, 176, 26, C_ACCENT);

    int card_w = 220, card_h = 92, gap = 16;
    int total_w = 4 * card_w + 3 * gap;
    int x0 = (APP_W - total_w) / 2;
    int y0 = 214;
    for (int i = 0; i < LANG_COUNT; i++) {
        int r = i / 4, c = i % 4;
        int bx = x0 + c * (card_w + gap);
        int by = y0 + r * (card_h + gap);
        int active = g_app.lang_sel == i;
        ui_round(bx, by, card_w, card_h, 18, active ? ui_rgb(44, 36, 82) : C_CARD);
        if (active) ui_round(bx, by, 5, card_h, 3, C_ACCENT);
        ui_flag((Lang)i, bx + 18, by + 22, 48, 34);
        ui_text(F_LABEL, bx + 80, by + 20, active ? C_TEXT : C_MUTED, i18n_native_name((Lang)i));
        ui_text(F_TINY, bx + 80, by + 50, C_DIM, i18n_code((Lang)i));
        app_hit(ACT_LANG, i, bx, by, card_w, card_h);
    }

    int bw = 280, bh = 58;
    int bx = (APP_W - bw) / 2, by = 612;
    draw_button(bx, by, bw, bh, i18n_get(S_BTN_CONFIRM), C_ACCENT, ui_rgb(255, 255, 255));
    app_hit(ACT_CONFIRM_LANG, 0, bx, by, bw, bh);
}

void pages_draw_sd_ready(void) {
    int w = 820, h = 460;
    int x = (APP_W - w) / 2, y = (APP_H - h) / 2;
    ui_round(x, y, w, h, 28, C_CARD);
    ui_round(x + 36, y + 36, 64, 64, 20, C_ACCENT);
    ui_icon(IC_CHECK, x + 50, y + 50, 36, ui_rgb(255, 255, 255));
    ui_text(F_H1, x + 120, y + 44, C_TEXT, i18n_get(S_SD_TITLE));
    ui_text_wrap(F_BODY, x + 36, y + 130, w - 72, 28, C_MUTED, i18n_get(S_SD_BODY));
    ui_text_wrap(F_SMALL, x + 36, y + 214, w - 72, 24, C_DIM, i18n_get(S_SD_HINT));

    ui_round(x + 36, y + 282, w - 72, 52, 14, C_PANEL);
    ui_text(F_SMALL, x + 56, y + 298, C_MUTED, "sdmc:/MUSIC/PLAYLISTS  ·  sdmc:/MUSIC/LOGO  ·  sdmc:/MUSIC/CONFIG");
    if (g_app.sd_error[0]) ui_text(F_TINY, x + 36, y + 344, ui_rgb(255, 120, 120), g_app.sd_error);

    int bw = 220, bh = 54;
    int bx = x + w - bw - 36, by = y + h - bh - 36;
    draw_button(bx, by, bw, bh, i18n_get(S_BTN_OK), C_ACCENT, ui_rgb(255, 255, 255));
    app_hit(ACT_OK, 0, bx, by, bw, bh);
}

void pages_draw_relaunch(void) {
    int w = 820, h = 430;
    int x = (APP_W - w) / 2, y = (APP_H - h) / 2;
    ui_round(x, y, w, h, 28, C_CARD);
    ui_round(x + 36, y + 36, 56, 56, 18, C_CARD2);
    ui_icon(IC_REFRESH, x + 50, y + 50, 28, C_ACCENT2);
    ui_text(F_H1, x + 108, y + 44, C_TEXT, i18n_get(S_RELAUNCH_TITLE));
    ui_text_wrap(F_BODY, x + 36, y + 120, w - 72, 28, C_MUTED, i18n_get(S_RELAUNCH_BODY));
    ui_round(x + 36, y + 232, w - 72, 46, 12, C_PANEL);
    ui_text(F_SMALL, x + 54, y + 246, C_MUTED, i18n_get(S_STORAGE));

    int bw = 300, bh = 56;
    int bx = x + w - bw - 36, by = y + h - bh - 36;
    draw_button(bx, by, bw, bh, i18n_get(S_BTN_QUIT), ui_rgb(60, 34, 48), ui_rgb(255, 200, 200));
    app_hit(ACT_QUIT, 0, bx, by, bw, bh);
}
