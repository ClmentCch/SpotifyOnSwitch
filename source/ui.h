#ifndef MUSICTT_UI_H
#define MUSICTT_UI_H

#include <SDL.h>
#include <SDL_ttf.h>

#include "common.h"
#include "color.h"
#include "i18n.h"

typedef struct { Uint8 r, g, b, a; } Col;

enum {
    F_TINY = 0, F_SMALL, F_BODY, F_LABEL, F_TITLE, F_H1, F_HERO, FONT_COUNT
};

enum {
    IC_HOME = 0, IC_PLAYLIST, IC_LIBRARY, IC_SETTINGS, IC_PLAY, IC_PAUSE,
    IC_NEXT, IC_PREV, IC_REFRESH, IC_BACK, IC_VOLUME, IC_NOTE, IC_WARNING,
    IC_CHECK, IC_CLOSE, IC_COUNT
};

int ui_init(SDL_Renderer *renderer);
void ui_shutdown(void);
void ui_reload_fonts(Lang lang);
void ui_begin_frame(void);

Col ui_rgb(int r, int g, int b);
Col ui_rgba(int r, int g, int b, int a);
Col ui_mix(Col a, Col b, float t);
Col ui_alpha(Col c, int a);
Col ui_from_rgb(Rgb rgb);

void ui_rect(int x, int y, int w, int h, Col c);
void ui_round(int x, int y, int w, int h, int radius, Col c);
void ui_round_border(int x, int y, int w, int h, int radius, int thick, Col c, Col bg);
void ui_circle(int cx, int cy, int radius, Col c);
void ui_ring(int cx, int cy, int radius, int thick, Col c);
void ui_gradient(int x, int y, int w, int h, Col top, Col bottom);
void ui_tri(int x, int y, int w, int h, Col c);
void ui_poly(const SDL_Point *points, int count, Col c);

int ui_text(int font, int x, int y, Col c, const char *text);
int ui_text_w(int font, const char *text);
int ui_text_h(int font);
void ui_text_c(int font, int cx, int y, Col c, const char *text);
void ui_text_r(int font, int right_x, int y, Col c, const char *text);
void ui_text_wrap(int font, int x, int y, int max_w, int line_h, Col c, const char *text);
int ui_wrap_height(int font, int max_w, int line_h, const char *text);

void ui_art(int x, int y, int size, int radius, const char *path, Col fallback);
int ui_art_dominant(const char *path, Col fallback, Col *out);
void ui_note_mark(int cx, int cy, int size, Col c);

void ui_flag(Lang lang, int x, int y, int w, int h);
void ui_icon(int icon, int x, int y, int size, Col c);

void ui_push_clip(int x, int y, int w, int h);
void ui_pop_clip(void);

#endif
