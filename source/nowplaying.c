#include "app.h"

#include <math.h>

static void fmt_time(char *buf, size_t n, double seconds) {
    if (seconds < 0) seconds = 0;
    int total = (int)seconds;
    int h = total / 3600;
    int m = (total % 3600) / 60;
    int s = total % 60;
    if (h > 0) snprintf(buf, n, "%d:%02d:%02d", h, m, s);
    else snprintf(buf, n, "%02d:%02d", m, s);
}

void now_update(double dt) {
    (void)dt;
}

void now_draw(void) {
    const PlayerItem *cur = player_current();
    if (!cur) return;

    Col top = g_app.theme_ready ? g_app.theme_top : ui_rgb(20, 18, 36);
    Col bottom = g_app.theme_ready ? g_app.theme_bottom : ui_rgb(8, 8, 13);
    Col accent = g_app.theme_ready ? g_app.theme_accent : C_ACCENT;
    ui_gradient(0, 0, APP_W, APP_H, top, bottom);

    /* Soft halo around the artwork, built from translucent discs. */
    for (int i = 8; i >= 1; i--) {
        ui_circle(APP_W / 2, 250, 120 + i * 34, ui_alpha(accent, 6));
    }
    ui_rect(0, 0, APP_W, APP_H, ui_alpha(ui_rgb(0, 0, 0), 40));

    float anim = g_app.cfg.animations ? (float)g_app.now_anim : 1.0f;
    float ease = 1.0f - powf(1.0f - anim, 3.0f);
    int slide = (int)((1.0f - ease) * 36.0f);

    ui_text_c(F_TINY, APP_W / 2, 30, ui_alpha(C_TEXT, 160), i18n_get(S_NOW_PLAYING));

    int bx = 30, by = 24;
    ui_round(bx, by, 46, 46, 15, ui_alpha(ui_rgb(0, 0, 0), 70));
    ui_icon(IC_BACK, bx + 14, by + 13, 20, C_TEXT);
    app_hit(ACT_CLOSE_NOW, 0, bx, by, 46, 46);

    double duration = player_duration();
    double position = g_app.dragging_seek ? g_app.seek_preview : player_position();
    if (position > duration) position = duration;

    int art = 296;
    int ax = (APP_W - art) / 2;
    int ay = 76 + slide;
    ui_round(ax - 6, ay - 6, art + 12, art + 12, 30, ui_alpha(ui_rgb(0, 0, 0), 80));
    ui_art(ax, ay, art, 26, cur->logo, accent);

    PlayerStatus st = player_status();
    if (st == PLAYER_LOADING) {
        ui_round(ax, ay, art, art, 26, ui_alpha(ui_rgb(0, 0, 0), 140));
        ui_text_c(F_LABEL, APP_W / 2, ay + art / 2 - 10, C_TEXT, i18n_get(S_LOADING));
    }

    ui_text_c(F_H1, APP_W / 2, 404 + slide, C_TEXT, cur->title);
    ui_text_c(F_BODY, APP_W / 2, 446 + slide, ui_alpha(C_TEXT, 170), cur->playlist);

    int sx = 340, sw = 600, sy = 502 + slide, sh = 12;
    ui_round(sx, sy, sw, sh, sh / 2, ui_alpha(ui_rgb(0, 0, 0), 90));
    float ratio = duration > 0 ? (float)(position / duration) : 0.0f;
    ratio = fclampf(ratio, 0.0f, 1.0f);
    int fill = (int)(sw * ratio);
    if (fill > 0) ui_round(sx, sy, fill, sh, sh / 2, accent);
    ui_circle(sx + fill, sy + sh / 2, 10, C_TEXT);
    app_hit(ACT_SEEK, 0, sx - 14, sy - 16, sw + 28, sh + 32);

    char posbuf[16], durbuf[16];
    fmt_time(posbuf, sizeof(posbuf), position);
    fmt_time(durbuf, sizeof(durbuf), duration);
    ui_text(F_SMALL, sx, sy + 24, ui_alpha(C_TEXT, 170), posbuf);
    ui_text_r(F_SMALL, sx + sw, sy + 24, ui_alpha(C_TEXT, 170), durbuf);

    int cy = 586 + slide;
    ui_round(556, cy - 24, 48, 48, 16, ui_alpha(ui_rgb(0, 0, 0), 70));
    ui_icon(IC_PREV, 568, cy - 12, 24, C_TEXT);
    app_hit(ACT_PREV, 0, 556, cy - 24, 48, 48);

    ui_round(626, cy - 38, 76, 76, 26, accent);
    ui_icon(st == PLAYER_PLAYING ? IC_PAUSE : IC_PLAY, 646, cy - 20, 36, ui_rgb(255, 255, 255));
    app_hit(ACT_TOGGLE, 0, 626, cy - 38, 76, 76);

    ui_round(724, cy - 24, 48, 48, 16, ui_alpha(ui_rgb(0, 0, 0), 70));
    ui_icon(IC_NEXT, 736, cy - 12, 24, C_TEXT);
    app_hit(ACT_NEXT, 0, 724, cy - 24, 48, 48);

    int vy = 668 + slide;
    ui_icon(IC_VOLUME, 548, vy - 2, 22, ui_alpha(C_TEXT, 190));
    int vsx = 586, vsw = 150, vsh = 8;
    ui_round(vsx, vy + 4, vsw, vsh, vsh / 2, ui_alpha(ui_rgb(0, 0, 0), 90));
    int vol = g_app.dragging_volume ? g_app.vol_preview : g_app.cfg.volume;
    int vfill = vsw * vol / 100;
    if (vfill > 0) ui_round(vsx, vy + 4, vfill, vsh, vsh / 2, ui_alpha(C_TEXT, 220));
    ui_circle(vsx + vfill, vy + 8, 7, C_TEXT);
    app_hit(ACT_VOL_PLAYER, 0, vsx - 12, vy - 10, vsw + 24, 28);
    char vbuf[16];
    snprintf(vbuf, sizeof(vbuf), "%d%%", vol);
    ui_text(F_TINY, vsx + vsw + 16, vy + 1, ui_alpha(C_TEXT, 170), vbuf);
}
