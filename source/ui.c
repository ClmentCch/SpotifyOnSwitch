#include "ui.h"

#include <SDL_image.h>
#include <math.h>

#include "color.h"

/* The CJK font is linked into .rodata by the Makefile. */
extern const unsigned char _binary_assets_MusicttCJK_ttc_start[];
extern const unsigned char _binary_assets_MusicttCJK_ttc_end[];
extern const unsigned char _binary_assets_NotoSans_Regular_ttf_start[];
extern const unsigned char _binary_assets_NotoSans_Regular_ttf_end[];

#define TEXT_CACHE_MAX 320
#define ART_CACHE_MAX  72
#define CORNER_CACHE_MAX 24
#define GRAD_CACHE_MAX 8
#define DOM_CACHE_MAX 48

static SDL_Renderer *R;

static TTF_Font *g_primary[FONT_COUNT];
static TTF_Font *g_fallback[FONT_COUNT];
static int g_primary_is_cjk;

static const int FONT_PX[FONT_COUNT] = { 14, 16, 19, 22, 27, 34, 44 };

static uint64_t g_frame;
static SDL_Rect g_clip_stack[8];
static int g_clip_depth;

/* ------------------------------------------------------------------ colours */

Col ui_rgb(int r, int g, int b) { Col c = { (Uint8)r, (Uint8)g, (Uint8)b, 255 }; return c; }
Col ui_rgba(int r, int g, int b, int a) { Col c = { (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a }; return c; }
Col ui_mix(Col a, Col b, float t) {
    t = fclampf(t, 0.0f, 1.0f);
    Col c;
    c.r = (Uint8)(a.r + (b.r - a.r) * t);
    c.g = (Uint8)(a.g + (b.g - a.g) * t);
    c.b = (Uint8)(a.b + (b.b - a.b) * t);
    c.a = (Uint8)(a.a + (b.a - a.a) * t);
    return c;
}
Col ui_alpha(Col c, int a) { c.a = (Uint8)iclamp(a, 0, 255); return c; }
Col ui_from_rgb(Rgb rgb) { return ui_rgb(rgb.r, rgb.g, rgb.b); }

static void set_color(Col c) {
    SDL_SetRenderDrawColor(R, c.r, c.g, c.b, c.a);
    SDL_SetRenderDrawBlendMode(R, SDL_BLENDMODE_BLEND);
}

/* ------------------------------------------------------------------- basics */

void ui_rect(int x, int y, int w, int h, Col c) {
    if (w <= 0 || h <= 0) return;
    SDL_Rect r = { x, y, w, h };
    set_color(c);
    SDL_RenderFillRect(R, &r);
}

static SDL_Texture *make_corner_tex(int radius) {
    if (radius <= 0) return NULL;
    SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, radius, radius, 32, SDL_PIXELFORMAT_RGBA32);
    if (!s) return NULL;
    SDL_LockSurface(s);
    Uint32 *px = (Uint32 *)s->pixels;
    for (int y = 0; y < radius; y++) {
        for (int x = 0; x < radius; x++) {
            int inside = 0;
            for (int sy = 0; sy < 3; sy++) {
                for (int sx = 0; sx < 3; sx++) {
                    float fx = (float)x + (sx + 0.5f) / 3.0f;
                    float fy = (float)y + (sy + 0.5f) / 3.0f;
                    float dx = fx - (float)radius;
                    float dy = fy - (float)radius;
                    if (dx * dx + dy * dy <= (float)radius * (float)radius) inside++;
                }
            }
            Uint8 a = (Uint8)(inside * 255 / 9);
            px[y * (s->pitch / 4) + x] = ((Uint32)a << 24) | 0x00FFFFFFu;
        }
    }
    SDL_UnlockSurface(s);
    SDL_Texture *t = SDL_CreateTextureFromSurface(R, s);
    SDL_FreeSurface(s);
    if (t) SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    return t;
}

typedef struct { int radius; SDL_Texture *tex; } CornerCache;
static CornerCache g_corners[CORNER_CACHE_MAX];
static int g_corner_next;

static SDL_Texture *get_corner(int radius) {
    if (radius <= 0) return NULL;
    for (int i = 0; i < CORNER_CACHE_MAX; i++) {
        if (g_corners[i].tex && g_corners[i].radius == radius) return g_corners[i].tex;
    }
    SDL_Texture *t = make_corner_tex(radius);
    if (!t) return NULL;
    int slot = -1;
    for (int i = 0; i < CORNER_CACHE_MAX; i++) {
        if (!g_corners[i].tex) { slot = i; break; }
    }
    if (slot < 0) {
        slot = g_corner_next;
        g_corner_next = (g_corner_next + 1) % CORNER_CACHE_MAX;
        SDL_DestroyTexture(g_corners[slot].tex);
    }
    g_corners[slot].radius = radius;
    g_corners[slot].tex = t;
    return t;
}

static SDL_Texture *make_disc_tex(int radius, int inner) {
    int size = radius * 2;
    if (size <= 0) return NULL;
    SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
    if (!s) return NULL;
    SDL_LockSurface(s);
    Uint32 *px = (Uint32 *)s->pixels;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int inside = 0;
            for (int sy = 0; sy < 3; sy++) {
                for (int sx = 0; sx < 3; sx++) {
                    float fx = (float)x + (sx + 0.5f) / 3.0f - (float)radius;
                    float fy = (float)y + (sy + 0.5f) / 3.0f - (float)radius;
                    float d2 = fx * fx + fy * fy;
                    if (d2 <= (float)radius * (float)radius && d2 >= (float)inner * (float)inner) inside++;
                }
            }
            Uint8 a = (Uint8)(inside * 255 / 9);
            px[y * (s->pitch / 4) + x] = ((Uint32)a << 24) | 0x00FFFFFFu;
        }
    }
    SDL_UnlockSurface(s);
    SDL_Texture *t = SDL_CreateTextureFromSurface(R, s);
    SDL_FreeSurface(s);
    if (t) SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    return t;
}

typedef struct { int radius, inner; SDL_Texture *tex; } DiscCache;
static DiscCache g_discs[24];
static int g_disc_next;

static SDL_Texture *get_disc(int radius, int inner) {
    if (radius <= 0) return NULL;
    for (int i = 0; i < 24; i++) {
        if (g_discs[i].tex && g_discs[i].radius == radius && g_discs[i].inner == inner) return g_discs[i].tex;
    }
    SDL_Texture *t = make_disc_tex(radius, inner);
    if (!t) return NULL;
    int slot = -1;
    for (int i = 0; i < 24; i++) if (!g_discs[i].tex) { slot = i; break; }
    if (slot < 0) {
        slot = g_disc_next;
        g_disc_next = (g_disc_next + 1) % 24;
        SDL_DestroyTexture(g_discs[slot].tex);
    }
    g_discs[slot].radius = radius;
    g_discs[slot].inner = inner;
    g_discs[slot].tex = t;
    return t;
}

static void draw_disc(int cx, int cy, int radius, int inner, Col c) {
    SDL_Texture *t = get_disc(radius, inner);
    if (!t) return;
    SDL_SetTextureColorMod(t, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(t, c.a);
    SDL_Rect dst = { cx - radius, cy - radius, radius * 2, radius * 2 };
    SDL_RenderCopy(R, t, NULL, &dst);
}

void ui_circle(int cx, int cy, int radius, Col c) { draw_disc(cx, cy, radius, 0, c); }
void ui_ring(int cx, int cy, int radius, int thick, Col c) {
    int inner = radius - thick;
    if (inner < 0) inner = 0;
    draw_disc(cx, cy, radius, inner, c);
}

void ui_round(int x, int y, int w, int h, int radius, Col c) {
    if (w <= 0 || h <= 0) return;
    if (radius <= 0) { ui_rect(x, y, w, h, c); return; }
    if (radius * 2 > w) radius = w / 2;
    if (radius * 2 > h) radius = h / 2;

    ui_rect(x + radius, y, w - 2 * radius, h, c);
    ui_rect(x, y + radius, radius, h - 2 * radius, c);
    ui_rect(x + w - radius, y + radius, radius, h - 2 * radius, c);

    SDL_Texture *t = get_corner(radius);
    if (!t) return;
    SDL_SetTextureColorMod(t, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(t, c.a);
    SDL_Rect dst = { x, y, radius, radius };
    SDL_RenderCopy(R, t, NULL, &dst);
    dst.x = x + w - radius;
    SDL_RenderCopyEx(R, t, NULL, &dst, 90.0, NULL, SDL_FLIP_NONE);
    dst.y = y + h - radius;
    SDL_RenderCopyEx(R, t, NULL, &dst, 180.0, NULL, SDL_FLIP_NONE);
    dst.x = x;
    SDL_RenderCopyEx(R, t, NULL, &dst, 270.0, NULL, SDL_FLIP_NONE);
}

void ui_round_border(int x, int y, int w, int h, int radius, int thick, Col c, Col bg) {
    ui_round(x - thick, y - thick, w + thick * 2, h + thick * 2, radius + thick, c);
    ui_round(x, y, w, h, radius, bg);
}

void ui_gradient(int x, int y, int w, int h, Col top, Col bottom) {
    static struct { Col top, bottom; SDL_Texture *tex; } cache[GRAD_CACHE_MAX];
    static int next;
    SDL_Texture *t = NULL;
    for (int i = 0; i < GRAD_CACHE_MAX; i++) {
        if (cache[i].tex && cache[i].top.r == top.r && cache[i].top.g == top.g && cache[i].top.b == top.b &&
            cache[i].bottom.r == bottom.r && cache[i].bottom.g == bottom.g && cache[i].bottom.b == bottom.b) {
            t = cache[i].tex;
            break;
        }
    }
    if (!t) {
        SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, 1, 256, 32, SDL_PIXELFORMAT_RGBA32);
        if (!s) { ui_rect(x, y, w, h, top); return; }
        SDL_LockSurface(s);
        Uint32 *px = (Uint32 *)s->pixels;
        for (int i = 0; i < 256; i++) {
            float f = (float)i / 255.0f;
            Uint8 r = (Uint8)(top.r + (bottom.r - top.r) * f);
            Uint8 g = (Uint8)(top.g + (bottom.g - top.g) * f);
            Uint8 b = (Uint8)(top.b + (bottom.b - top.b) * f);
            px[i] = ((Uint32)255 << 24) | ((Uint32)b << 16) | ((Uint32)g << 8) | r;
        }
        SDL_UnlockSurface(s);
        t = SDL_CreateTextureFromSurface(R, s);
        SDL_FreeSurface(s);
        if (!t) { ui_rect(x, y, w, h, top); return; }
        int slot = next;
        next = (next + 1) % GRAD_CACHE_MAX;
        if (cache[slot].tex) SDL_DestroyTexture(cache[slot].tex);
        cache[slot].top = top;
        cache[slot].bottom = bottom;
        cache[slot].tex = t;
    }
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_NONE);
    SDL_Rect dst = { x, y, w, h };
    SDL_RenderCopy(R, t, NULL, &dst);
}

void ui_poly(const SDL_Point *points, int count, Col c) {
    if (count < 3) return;
    SDL_Vertex verts[16];
    int indices[48];
    if (count > 15) count = 15;
    float cx = 0, cy = 0;
    for (int i = 0; i < count; i++) { cx += (float)points[i].x; cy += (float)points[i].y; }
    cx /= (float)count;
    cy /= (float)count;
    for (int i = 0; i < count; i++) {
        verts[i].position.x = (float)points[i].x;
        verts[i].position.y = (float)points[i].y;
        verts[i].color.r = c.r; verts[i].color.g = c.g; verts[i].color.b = c.b; verts[i].color.a = c.a;
        verts[i].tex_coord.x = 0; verts[i].tex_coord.y = 0;
    }
    verts[count].position.x = cx;
    verts[count].position.y = cy;
    verts[count].color = verts[0].color;
    verts[count].tex_coord.x = 0; verts[count].tex_coord.y = 0;
    int n = 0;
    for (int i = 0; i < count; i++) {
        indices[n++] = count;
        indices[n++] = i;
        indices[n++] = (i + 1) % count;
    }
    SDL_RenderGeometry(R, NULL, verts, count + 1, indices, n);
}

void ui_tri(int x, int y, int w, int h, Col c) {
    SDL_Point p[3] = { { x, y }, { x, y + h }, { x + w, y + h / 2 } };
    ui_poly(p, 3, c);
}

static void ui_line(int x1, int y1, int x2, int y2, int thick, Col c) {
    float dx = (float)(x2 - x1), dy = (float)(y2 - y1);
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.01f) return;
    float nx = -dy / len * (float)thick * 0.5f;
    float ny = dx / len * (float)thick * 0.5f;
    SDL_Vertex v[4];
    SDL_FPoint pos[4] = {
        { (float)x1 + nx, (float)y1 + ny },
        { (float)x2 + nx, (float)y2 + ny },
        { (float)x2 - nx, (float)y2 - ny },
        { (float)x1 - nx, (float)y1 - ny },
    };
    for (int i = 0; i < 4; i++) {
        v[i].position = pos[i];
        v[i].color.r = c.r; v[i].color.g = c.g; v[i].color.b = c.b; v[i].color.a = c.a;
        v[i].tex_coord.x = 0; v[i].tex_coord.y = 0;
    }
    int idx[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(R, NULL, v, 4, idx, 6);
}

void ui_push_clip(int x, int y, int w, int h) {
    SDL_Rect r = { x, y, w, h };
    if (g_clip_depth > 0) {
        SDL_Rect cur = g_clip_stack[g_clip_depth - 1];
        int x0 = imax(cur.x, r.x), y0 = imax(cur.y, r.y);
        int x1 = imin(cur.x + cur.w, r.x + r.w), y1 = imin(cur.y + cur.h, r.y + r.h);
        r.x = x0; r.y = y0; r.w = imax(0, x1 - x0); r.h = imax(0, y1 - y0);
    }
    if (g_clip_depth < 8) g_clip_stack[g_clip_depth] = r;
    g_clip_depth++;
    SDL_RenderSetClipRect(R, &r);
}

void ui_pop_clip(void) {
    if (g_clip_depth > 0) g_clip_depth--;
    if (g_clip_depth == 0) SDL_RenderSetClipRect(R, NULL);
    else SDL_RenderSetClipRect(R, &g_clip_stack[g_clip_depth - 1]);
}

/* --------------------------------------------------------------------- text */

typedef struct {
    int font;
    uint32_t hash;
    char text[192];
    SDL_Texture *tex;
    int w, h;
    uint64_t used;
} TextEntry;

static TextEntry g_texts[TEXT_CACHE_MAX];

static uint32_t fnv1a(const char *s) {
    uint32_t h = 2166136261u;
    while (*s) { h ^= (uint8_t)*s++; h *= 16777619u; }
    return h;
}

static const char *utf8_next(const char **sp, uint32_t *cp) {
    const unsigned char *s = (const unsigned char *)*sp;
    uint32_t c = s[0];
    int n = 1;
    if (c >= 0xF0) { c &= 0x07; n = 4; }
    else if (c >= 0xE0) { c &= 0x0F; n = 3; }
    else if (c >= 0xC0) { c &= 0x1F; n = 2; }
    else { *cp = c; *sp += 1; return *sp; }
    for (int i = 1; i < n; i++) {
        if ((s[i] & 0xC0) != 0x80) { *cp = s[0]; *sp += 1; return *sp; }
        c = (c << 6) | (s[i] & 0x3F);
    }
    *cp = c;
    *sp += n;
    return *sp;
}

static TTF_Font *font_for(int f, uint32_t cp) {
    if (f < 0 || f >= FONT_COUNT) f = F_BODY;
    if (g_primary[f] && TTF_GlyphIsProvided32(g_primary[f], cp)) return g_primary[f];
    if (g_fallback[f] && TTF_GlyphIsProvided32(g_fallback[f], cp)) return g_fallback[f];
    return g_primary[f] ? g_primary[f] : g_fallback[f];
}

void ui_reload_fonts(Lang lang) {
    for (int i = 0; i < TEXT_CACHE_MAX; i++) {
        if (g_texts[i].tex) SDL_DestroyTexture(g_texts[i].tex);
        memset(&g_texts[i], 0, sizeof(g_texts[i]));
    }
    for (int i = 0; i < FONT_COUNT; i++) {
        if (g_primary[i]) { TTF_CloseFont(g_primary[i]); g_primary[i] = NULL; }
        if (g_fallback[i]) { TTF_CloseFont(g_fallback[i]); g_fallback[i] = NULL; }
    }

    const size_t cjk_size = (size_t)(_binary_assets_MusicttCJK_ttc_end - _binary_assets_MusicttCJK_ttc_start);
    const size_t latin_size = (size_t)(_binary_assets_NotoSans_Regular_ttf_end - _binary_assets_NotoSans_Regular_ttf_start);
    const int face = i18n_cjk_face(lang);
    g_primary_is_cjk = i18n_uses_cjk(lang);

    for (int i = 0; i < FONT_COUNT; i++) {
        if (g_primary_is_cjk) {
            SDL_RWops *rw = SDL_RWFromConstMem(_binary_assets_MusicttCJK_ttc_start, (int)cjk_size);
            g_primary[i] = rw ? TTF_OpenFontIndexRW(rw, 0, face, FONT_PX[i]) : NULL;
            SDL_RWops *rw2 = SDL_RWFromConstMem(_binary_assets_NotoSans_Regular_ttf_start, (int)latin_size);
            g_fallback[i] = rw2 ? TTF_OpenFontRW(rw2, 0, FONT_PX[i]) : NULL;
        } else {
            SDL_RWops *rw = SDL_RWFromConstMem(_binary_assets_NotoSans_Regular_ttf_start, (int)latin_size);
            g_primary[i] = rw ? TTF_OpenFontRW(rw, 0, FONT_PX[i]) : NULL;
            SDL_RWops *rw2 = SDL_RWFromConstMem(_binary_assets_MusicttCJK_ttc_start, (int)cjk_size);
            g_fallback[i] = rw2 ? TTF_OpenFontIndexRW(rw2, 0, face, FONT_PX[i]) : NULL;
        }
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
}

static SDL_Texture *text_build(int fi, const char *text, int *out_w, int *out_h) {
    TTF_Font *base = g_primary[fi];
    if (!base) return NULL;

    struct Run { TTF_Font *font; char buf[256]; } runs[48];
    int nruns = 0;
    const char *p = text;
    while (*p && nruns < 48) {
        const char *start = p;
        uint32_t cp = 0;
        utf8_next(&p, &cp);
        if (cp == '\n') continue;
        TTF_Font *f = font_for(fi, cp);
        size_t len = (size_t)(p - start);
        if (nruns > 0 && runs[nruns - 1].font == f) {
            size_t have = strlen(runs[nruns - 1].buf);
            if (have + len + 1 < sizeof(runs[0].buf)) {
                memcpy(runs[nruns - 1].buf + have, start, len);
                runs[nruns - 1].buf[have + len] = 0;
            }
        } else {
            struct Run *r = &runs[nruns++];
            r->font = f;
            if (len >= sizeof(r->buf)) len = sizeof(r->buf) - 1;
            memcpy(r->buf, start, len);
            r->buf[len] = 0;
        }
    }

    int total_w = 0;
    for (int i = 0; i < nruns; i++) {
        int w = 0, h = 0;
        if (TTF_SizeUTF8(runs[i].font, runs[i].buf, &w, &h) == 0) total_w += w;
    }
    int line_h = TTF_FontHeight(base);
    if (total_w <= 0) total_w = 1;
    if (line_h <= 0) line_h = FONT_PX[fi];

    SDL_Surface *comp = SDL_CreateRGBSurfaceWithFormat(0, total_w, line_h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!comp) return NULL;
    SDL_FillRect(comp, NULL, 0);
    SDL_SetSurfaceBlendMode(comp, SDL_BLENDMODE_BLEND);

    int x = 0;
    int baseline = TTF_FontAscent(base);
    SDL_Color white = { 255, 255, 255, 255 };
    for (int i = 0; i < nruns; i++) {
        SDL_Surface *glyphs = TTF_RenderUTF8_Blended(runs[i].font, runs[i].buf, white);
        if (!glyphs) continue;
        SDL_Rect dst;
        dst.x = x;
        dst.y = baseline - TTF_FontAscent(runs[i].font);
        dst.w = glyphs->w;
        dst.h = glyphs->h;
        SDL_BlitSurface(glyphs, NULL, comp, &dst);
        x += glyphs->w;
        SDL_FreeSurface(glyphs);
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(R, comp);
    *out_w = total_w;
    *out_h = line_h;
    SDL_FreeSurface(comp);
    if (tex) {
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(tex, SDL_ScaleModeLinear);
    }
    return tex;
}

static TextEntry *text_get(int fi, const char *text) {
    if (fi < 0 || fi >= FONT_COUNT) fi = F_BODY;
    uint32_t h = fnv1a(text) ^ (uint32_t)(fi * 2654435761u);
    for (int i = 0; i < TEXT_CACHE_MAX; i++) {
        if (g_texts[i].tex && g_texts[i].hash == h && g_texts[i].font == fi && strcmp(g_texts[i].text, text) == 0) {
            g_texts[i].used = g_frame;
            return &g_texts[i];
        }
    }
    int slot = -1;
    for (int i = 0; i < TEXT_CACHE_MAX; i++) {
        if (!g_texts[i].tex) { slot = i; break; }
    }
    if (slot < 0) {
        slot = 0;
        for (int i = 1; i < TEXT_CACHE_MAX; i++) {
            if (g_texts[i].used < g_texts[slot].used) slot = i;
        }
        SDL_DestroyTexture(g_texts[slot].tex);
        memset(&g_texts[slot], 0, sizeof(g_texts[slot]));
    }
    TextEntry *e = &g_texts[slot];
    e->font = fi;
    e->hash = h;
    snprintf(e->text, sizeof(e->text), "%s", text);
    e->tex = text_build(fi, text, &e->w, &e->h);
    e->used = g_frame;
    return e->tex ? e : NULL;
}

int ui_text_w(int font, const char *text) {
    TextEntry *e = text_get(font, text);
    return e ? e->w : 0;
}

int ui_text_h(int font) {
    if (font < 0 || font >= FONT_COUNT || !g_primary[font]) return 0;
    return TTF_FontHeight(g_primary[font]);
}

int ui_text(int font, int x, int y, Col c, const char *text) {
    if (!text || !*text) return 0;
    TextEntry *e = text_get(font, text);
    if (!e || !e->tex) return 0;
    SDL_SetTextureColorMod(e->tex, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(e->tex, c.a);
    SDL_Rect dst = { x, y, e->w, e->h };
    SDL_RenderCopy(R, e->tex, NULL, &dst);
    return e->w;
}

void ui_text_c(int font, int cx, int y, Col c, const char *text) {
    ui_text(font, cx - ui_text_w(font, text) / 2, y, c, text);
}

void ui_text_r(int font, int right_x, int y, Col c, const char *text) {
    ui_text(font, right_x - ui_text_w(font, text), y, c, text);
}

static int wrap_lines(int font, int max_w, const char **lines, int *line_count, int max_lines) {
    /* Only used by callers below via ui_text_wrap; kept simple and static. */
    (void)font; (void)max_w; (void)lines; (void)line_count; (void)max_lines;
    return 0;
}

int ui_wrap_height(int font, int max_w, int line_h, const char *text) {
    if (!text || !*text) return 0;
    char line[256];
    int lines = 0;
    const char *p = text;
    while (*p) {
        const char *line_start = p;
        const char *last_space = NULL;
        size_t take = 0;
        while (*p) {
            const char *word_end = p;
            while (*word_end && *word_end != ' ') word_end++;
            size_t candidate = (size_t)(word_end - line_start);
            if (candidate >= sizeof(line)) candidate = sizeof(line) - 1;
            memcpy(line, line_start, candidate);
            line[candidate] = 0;
            if (ui_text_w(font, line) <= max_w || take == 0) {
                take = candidate;
                p = word_end;
                if (*p == ' ') { last_space = p; p++; }
                else break;
            } else {
                break;
            }
        }
        (void)last_space;
        if (take == 0) break;
        lines++;
        if (!*p) break;
    }
    return lines * line_h;
}

void ui_text_wrap(int font, int x, int y, int max_w, int line_h, Col c, const char *text) {
    if (!text || !*text) return;
    char line[256];
    int ly = y;
    const char *p = text;
    while (*p) {
        const char *line_start = p;
        size_t take = 0;
        while (*p) {
            const char *word_end = p;
            while (*word_end && *word_end != ' ') word_end++;
            size_t candidate = (size_t)(word_end - line_start);
            if (candidate >= sizeof(line)) candidate = sizeof(line) - 1;
            memcpy(line, line_start, candidate);
            line[candidate] = 0;
            if (ui_text_w(font, line) <= max_w || take == 0) {
                take = candidate;
                p = word_end;
                if (*p == ' ') p++;
                else break;
            } else {
                break;
            }
        }
        if (take == 0) break;
        memcpy(line, line_start, take);
        line[take] = 0;
        ui_text(font, x, ly, c, line);
        ly += line_h;
    }
}

/* ------------------------------------------------------------------ artwork */

typedef struct {
    char path[PATH_MAX_LEN];
    int size, radius;
    SDL_Texture *tex;
    uint64_t used;
} ArtEntry;
static ArtEntry g_arts[ART_CACHE_MAX];

typedef struct { char path[PATH_MAX_LEN]; Rgb color; int valid; } DomEntry;
static DomEntry g_doms[DOM_CACHE_MAX];

static uint8_t sample_channel(const uint8_t *p, int c) { return p[c]; }

static SDL_Surface *round_surface(SDL_Surface *src, int size, int radius) {
    if (!src) return NULL;
    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(src);
    if (!rgba) return NULL;

    SDL_Surface *dst = SDL_CreateRGBSurfaceWithFormat(0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
    if (!dst) { SDL_FreeSurface(rgba); return NULL; }

    float sw = (float)rgba->w, sh = (float)rgba->h;
    float scale = (float)size / (sw > sh ? sw : sh);
    float src_w = (float)size / scale;
    float src_h = (float)size / scale;
    float off_x = (sw - src_w) * 0.5f;
    float off_y = (sh - src_h) * 0.5f;

    SDL_LockSurface(rgba);
    SDL_LockSurface(dst);
    const uint8_t *sp = (const uint8_t *)rgba->pixels;
    uint8_t *dp = (uint8_t *)dst->pixels;
    int spitch = rgba->pitch, dpitch = dst->pitch;
    float rr = (float)radius;

    for (int y = 0; y < size; y++) {
        float fy = off_y + ((float)y + 0.5f) / scale - 0.5f;
        int y0 = (int)floorf(fy);
        float ty = fy - (float)y0;
        int y1 = y0 + 1;
        if (y0 < 0) y0 = 0;
        if (y1 >= rgba->h) y1 = rgba->h - 1;
        if (y0 >= rgba->h) y0 = rgba->h - 1;

        for (int x = 0; x < size; x++) {
            float fx = off_x + ((float)x + 0.5f) / scale - 0.5f;
            int x0 = (int)floorf(fx);
            float tx = fx - (float)x0;
            int x1 = x0 + 1;
            if (x0 < 0) x0 = 0;
            if (x1 >= rgba->w) x1 = rgba->w - 1;
            if (x0 >= rgba->w) x0 = rgba->w - 1;

            const uint8_t *p00 = sp + y0 * spitch + x0 * 4;
            const uint8_t *p10 = sp + y0 * spitch + x1 * 4;
            const uint8_t *p01 = sp + y1 * spitch + x0 * 4;
            const uint8_t *p11 = sp + y1 * spitch + x1 * 4;
            for (int c = 0; c < 4; c++) {
                float v = (1 - ty) * ((1 - tx) * sample_channel(p00, c) + tx * sample_channel(p10, c)) +
                          ty * ((1 - tx) * sample_channel(p01, c) + tx * sample_channel(p11, c));
                dp[y * dpitch + x * 4 + c] = (uint8_t)v;
            }

            float alpha = 1.0f;
            if (radius > 0) {
                float ccx = rr, ccy = rr;
                if (x >= radius) ccx = (float)(size - radius);
                if (y >= radius) ccy = (float)(size - radius);
                if (x < radius || x >= size - radius) {
                    if (y < radius || y >= size - radius) {
                        float dx = (float)x + 0.5f - ccx;
                        float dy = (float)y + 0.5f - ccy;
                        float d = sqrtf(dx * dx + dy * dy);
                        if (d > rr) alpha = 0.0f;
                        else alpha = fclampf(rr - d + 0.5f, 0.0f, 1.0f);
                    }
                }
            }
            dp[y * dpitch + x * 4 + 3] = (uint8_t)(dp[y * dpitch + x * 4 + 3] * alpha);
        }
    }
    SDL_UnlockSurface(dst);
    SDL_UnlockSurface(rgba);
    SDL_FreeSurface(rgba);
    return dst;
}

static SDL_Surface *load_art_surface(const char *path, int size, int radius) {
    if (!path || !*path) return NULL;
    /* Read through stdio instead of SDL_RWFromFile: the Switch SD card is a
       newlib device, and this keeps image loading independent of SDL's
       filesystem layer. */
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len <= 0 || len > 24 * 1024 * 1024) { fclose(f); return NULL; }
    rewind(f);
    unsigned char *buf = (unsigned char *)malloc((size_t)len);
    if (!buf) { fclose(f); return NULL; }
    size_t got = fread(buf, 1, (size_t)len, f);
    fclose(f);
    if (got != (size_t)len) { free(buf); return NULL; }
    SDL_RWops *rw = SDL_RWFromConstMem(buf, (int)len);
    SDL_Surface *raw = rw ? IMG_Load_RW(rw, 1) : NULL;
    free(buf);
    if (!raw) return NULL;
    return round_surface(raw, size, radius);
}

void ui_art(int x, int y, int size, int radius, const char *path, Col fallback) {
    uint64_t used = g_frame;
    if (path && *path) {
        for (int i = 0; i < ART_CACHE_MAX; i++) {
            ArtEntry *e = &g_arts[i];
            if (e->tex && e->size == size && e->radius == radius && strcmp(e->path, path) == 0) {
                e->used = used;
                SDL_Rect dst = { x, y, size, size };
                SDL_RenderCopy(R, e->tex, NULL, &dst);
                return;
            }
        }
    }
    SDL_Texture *tex = NULL;
    SDL_Surface *s = load_art_surface(path, size, radius);
    if (s) {
        tex = SDL_CreateTextureFromSurface(R, s);
        SDL_FreeSurface(s);
    }
    if (tex) {
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        int slot = -1;
        for (int i = 0; i < ART_CACHE_MAX; i++) if (!g_arts[i].tex) { slot = i; break; }
        if (slot < 0) {
            slot = 0;
            for (int i = 1; i < ART_CACHE_MAX; i++) if (g_arts[i].used < g_arts[slot].used) slot = i;
            SDL_DestroyTexture(g_arts[slot].tex);
            memset(&g_arts[slot], 0, sizeof(g_arts[slot]));
        }
        g_arts[slot].tex = tex;
        g_arts[slot].size = size;
        g_arts[slot].radius = radius;
        g_arts[slot].used = used;
        snprintf(g_arts[slot].path, sizeof(g_arts[slot].path), "%s", path);
        SDL_Rect dst = { x, y, size, size };
        SDL_RenderCopy(R, tex, NULL, &dst);
        return;
    }
    /* Replacement image: gradient tile with a note. */
    ui_round(x, y, size, size, radius, ui_alpha(fallback, 120));
    ui_note_mark(x + size / 2, y + size / 2, size * 55 / 100, ui_alpha(ui_rgb(255, 255, 255), 200));
}

int ui_art_dominant(const char *path, Col fallback, Col *out) {
    if (!path || !*path) { *out = fallback; return 0; }
    for (int i = 0; i < DOM_CACHE_MAX; i++) {
        if (g_doms[i].valid && strcmp(g_doms[i].path, path) == 0) {
            *out = ui_from_rgb(g_doms[i].color);
            return 1;
        }
    }
    SDL_Surface *s = load_art_surface(path, 96, 10);
    if (!s) { *out = fallback; return 0; }
    SDL_LockSurface(s);
    Rgb rgb = color_dominant((const uint8_t *)s->pixels, s->w, s->h, s->pitch);
    SDL_UnlockSurface(s);
    SDL_FreeSurface(s);

    int slot = 0;
    for (int i = 0; i < DOM_CACHE_MAX; i++) {
        if (!g_doms[i].valid) { slot = i; break; }
    }
    snprintf(g_doms[slot].path, sizeof(g_doms[slot].path), "%s", path);
    g_doms[slot].color = rgb;
    g_doms[slot].valid = 1;
    *out = ui_from_rgb(rgb);
    return 1;
}

/* -------------------------------------------------------------------- icons */

void ui_icon(int icon, int x, int y, int size, Col c) {
    switch (icon) {
        case IC_HOME: {
            SDL_Point roof[4] = { { x, y + size * 45 / 100 }, { x + size / 2, y + size * 8 / 100 },
                                  { x + size, y + size * 45 / 100 }, { x + size / 2, y + size * 60 / 100 } };
            ui_poly(roof, 4, c);
            ui_round(x + size / 5, y + size * 45 / 100, size * 3 / 5, size * 40 / 100, size / 10, c);
            break;
        }
        case IC_PLAYLIST:
            for (int i = 0; i < 3; i++) ui_round(x + size / 8, y + size * (15 + i * 24) / 100, size * 6 / 10, size / 12, size / 24, c);
            ui_circle(x + size * 78 / 100, y + size * 72 / 100, size / 9, c);
            ui_rect(x + size * 72 / 100, y + size * 42 / 100, size / 10, size * 32 / 100, c);
            break;
        case IC_LIBRARY:
            ui_round(x + size / 8, y + size / 5, size / 5, size * 3 / 5, size / 20, c);
            ui_round(x + size * 38 / 100, y + size / 5, size / 5, size * 3 / 5, size / 20, c);
            ui_round(x + size * 66 / 100, y + size * 28 / 100, size / 5, size * 52 / 100, size / 20, c);
            break;
        case IC_SETTINGS: {
            ui_ring(x + size / 2, y + size / 2, size * 42 / 100, size * 16 / 100, c);
            for (int i = 0; i < 8; i++) {
                float a = (float)i * 3.14159265f / 4.0f;
                int cx = x + size / 2 + (int)(cosf(a) * (float)size * 0.42f);
                int cy = y + size / 2 + (int)(sinf(a) * (float)size * 0.42f);
                ui_circle(cx, cy, size / 12, c);
            }
            break;
        }
        case IC_PLAY:
            ui_tri(x + size / 5, y + size / 8, size * 7 / 10, size * 3 / 4, c);
            break;
        case IC_PAUSE:
            ui_round(x + size / 4, y + size / 8, size / 5, size * 3 / 4, size / 16, c);
            ui_round(x + size * 55 / 100, y + size / 8, size / 5, size * 3 / 4, size / 16, c);
            break;
        case IC_NEXT:
            ui_tri(x + size * 12 / 100, y + size / 4, size * 5 / 10, size / 2, c);
            ui_tri(x + size * 40 / 100, y + size / 4, size * 5 / 10, size / 2, c);
            ui_round(x + size * 80 / 100, y + size / 4, size / 10, size / 2, size / 24, c);
            break;
        case IC_PREV:
            ui_round(x + size * 10 / 100, y + size / 4, size / 10, size / 2, size / 24, c);
            ui_tri(x + size * 55 / 100, y + size / 4, size * 5 / 10, size / 2, c);
            ui_tri(x + size * 27 / 100, y + size / 4, size * 5 / 10, size / 2, c);
            break;
        case IC_REFRESH: {
            ui_ring(x + size / 2, y + size / 2, size * 40 / 100, size * 12 / 100, c);
            SDL_Point p[3] = { { x + size * 78 / 100, y + size * 14 / 100 },
                               { x + size * 96 / 100, y + size * 34 / 100 },
                               { x + size * 66 / 100, y + size * 34 / 100 } };
            ui_poly(p, 3, c);
            break;
        }
        case IC_BACK:
            ui_tri(x + size * 22 / 100, y + size / 4, size * 5 / 10, size / 2, c);
            break;
        case IC_VOLUME: {
            SDL_Point sp[4] = { { x + size * 12 / 100, y + size * 38 / 100 },
                                { x + size * 38 / 100, y + size * 38 / 100 },
                                { x + size * 62 / 100, y + size * 12 / 100 },
                                { x + size * 62 / 100, y + size * 88 / 100 } };
            SDL_Point sp2[3] = { { x + size * 12 / 100, y + size * 62 / 100 },
                                 { x + size * 38 / 100, y + size * 62 / 100 },
                                 { x + size * 62 / 100, y + size * 88 / 100 } };
            ui_poly(sp, 4, c);
            ui_poly(sp2, 3, c);
            for (int i = 0; i < 3; i++) {
                int rr = size * (46 + i * 16) / 100;
                ui_ring(x + size * 62 / 100, y + size / 2, rr, size / 20, ui_alpha(c, 170 - i * 40));
            }
            break;
        }
        case IC_NOTE: {
            ui_circle(x + size * 32 / 100, y + size * 74 / 100, size / 6, c);
            ui_rect(x + size * 44 / 100, y + size * 20 / 100, size / 10, size * 55 / 100, c);
            SDL_Point flag[3] = { { x + size * 44 / 100, y + size * 20 / 100 },
                                  { x + size * 82 / 100, y + size * 32 / 100 },
                                  { x + size * 44 / 100, y + size * 44 / 100 } };
            ui_poly(flag, 3, c);
            break;
        }
        case IC_WARNING: {
            SDL_Point tri[3] = { { x + size / 2, y + size / 10 }, { x + size * 95 / 100, y + size * 9 / 10 },
                                 { x + size * 5 / 100, y + size * 9 / 10 } };
            ui_poly(tri, 3, c);
            ui_rect(x + size / 2 - size / 16, y + size * 38 / 100, size / 8, size * 30 / 100, ui_rgb(12, 12, 18));
            ui_circle(x + size / 2, y + size * 78 / 100, size / 14, ui_rgb(12, 12, 18));
            break;
        }
        case IC_CHECK:
            ui_line(x + size * 18 / 100, y + size * 52 / 100, x + size * 42 / 100, y + size * 76 / 100, size / 9, c);
            ui_line(x + size * 42 / 100, y + size * 76 / 100, x + size * 84 / 100, y + size * 26 / 100, size / 9, c);
            break;
        case IC_CLOSE:
            ui_line(x + size / 4, y + size / 4, x + size * 3 / 4, y + size * 3 / 4, size / 9, c);
            ui_line(x + size * 3 / 4, y + size / 4, x + size / 4, y + size * 3 / 4, size / 9, c);
            break;
        default:
            break;
    }
}

void ui_note_mark(int cx, int cy, int size, Col c) {
    ui_icon(IC_NOTE, cx - size / 2, cy - size / 2, size, c);
}

/* -------------------------------------------------------------------- flags */

static void star(int cx, int cy, int radius, Col c) {
    SDL_Point pts[10];
    for (int i = 0; i < 10; i++) {
        float a = -1.5707963f + (float)i * 3.14159265f / 5.0f;
        float r = (i & 1) ? radius * 0.42f : (float)radius;
        pts[i].x = cx + (int)lroundf(cosf(a) * r);
        pts[i].y = cy + (int)lroundf(sinf(a) * r);
    }
    ui_poly(pts, 10, c);
}

static void uk_flag(int x, int y, int w, int h) {
    ui_rect(x, y, w, h, ui_rgb(1, 33, 105));
    Col white = ui_rgb(255, 255, 255);
    Col red = ui_rgb(200, 16, 46);
    int t = imax(2, h / 5);
    SDL_Point a[4] = { { x, y }, { x + w * 30 / 100, y }, { x + w, y + h }, { x + w * 70 / 100, y + h } };
    ui_poly(a, 4, white);
    SDL_Point b[4] = { { x + w * 70 / 100, y }, { x + w, y }, { x + w * 30 / 100, y + h }, { x, y + h } };
    ui_poly(b, 4, white);
    /* Red diagonals, thinner. */
    SDL_Point ra[4] = { { x, y + h / 4 }, { x + w / 4, y }, { x + w, y + h * 3 / 4 }, { x + w * 3 / 4, y + h } };
    ui_poly(ra, 4, red);
    SDL_Point rb[4] = { { x + w * 3 / 4, y }, { x + w, y }, { x + w / 4, y + h }, { x, y + h } };
    ui_poly(rb, 4, red);
    /* White cross. */
    ui_rect(x + w / 2 - t, y, t * 2, h, white);
    ui_rect(x, y + h / 2 - t, w, t * 2, white);
    /* Red cross. */
    ui_rect(x + w / 2 - t / 2, y, t, h, red);
    ui_rect(x, y + h / 2 - t / 2, w, t, red);
}

void ui_flag(Lang lang, int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    switch (lang) {
        case LANG_FR:
            ui_rect(x, y, w / 3, h, ui_rgb(0, 85, 164));
            ui_rect(x + w / 3, y, w / 3, h, ui_rgb(255, 255, 255));
            ui_rect(x + 2 * w / 3, y, w - 2 * w / 3, h, ui_rgb(239, 65, 53));
            break;
        case LANG_EN:
            uk_flag(x, y, w, h);
            break;
        case LANG_ES:
            ui_rect(x, y, w, h, ui_rgb(198, 11, 30));
            ui_rect(x, y + h / 4, w, h / 2, ui_rgb(255, 196, 0));
            break;
        case LANG_DE:
            ui_rect(x, y, w, h / 3, ui_rgb(0, 0, 0));
            ui_rect(x, y + h / 3, w, h / 3, ui_rgb(221, 0, 0));
            ui_rect(x, y + 2 * h / 3, w, h - 2 * h / 3, ui_rgb(255, 206, 0));
            break;
        case LANG_IT:
            ui_rect(x, y, w / 3, h, ui_rgb(0, 146, 70));
            ui_rect(x + w / 3, y, w / 3, h, ui_rgb(255, 255, 255));
            ui_rect(x + 2 * w / 3, y, w - 2 * w / 3, h, ui_rgb(206, 43, 55));
            break;
        case LANG_PT:
            ui_rect(x, y, w, h, ui_rgb(218, 41, 28));
            ui_rect(x, y, w * 2 / 5, h, ui_rgb(0, 102, 0));
            ui_circle(x + w * 2 / 5, y + h / 2, imin(w, h) / 5, ui_rgb(255, 221, 0));
            break;
        case LANG_JA:
            ui_rect(x, y, w, h, ui_rgb(255, 255, 255));
            ui_circle(x + w / 2, y + h / 2, imin(w, h) * 3 / 10, ui_rgb(188, 0, 45));
            break;
        case LANG_KO:
            ui_rect(x, y, w, h, ui_rgb(255, 255, 255));
            ui_circle(x + w / 2, y + h / 2, imin(w, h) / 4, ui_rgb(205, 46, 58));
            {
                SDL_Point half[9];
                int cx = x + w / 2, cy = y + h / 2, r = imin(w, h) / 4;
                half[0].x = cx - r; half[0].y = cy;
                for (int i = 0; i <= 7; i++) {
                    float a = 3.14159265f * (float)i / 7.0f;
                    half[i + 1].x = cx + (int)(cosf(a) * r);
                    half[i + 1].y = cy + (int)(sinf(a) * r);
                }
                SDL_Point fan[10];
                fan[0].x = cx; fan[0].y = cy;
                int n = 1;
                fan[n++] = half[0];
                for (int i = 0; i <= 7; i++) fan[n++] = half[i + 1];
                ui_poly(fan, n, ui_rgb(0, 71, 160));
            }
            for (int g = 0; g < 4; g++) {
                int ox = (g % 2) ? x + w * 74 / 100 : x + w * 10 / 100;
                int oy = (g / 2) ? y + h * 68 / 100 : y + h * 14 / 100;
                for (int k = 0; k < 3; k++) ui_rect(ox, oy + k * h / 12, w / 6, h / 22, ui_rgb(0, 0, 0));
            }
            break;
        case LANG_ZH:
            ui_rect(x, y, w, h, ui_rgb(222, 41, 16));
            star(x + w / 5, y + h / 4, imin(w, h) / 5, ui_rgb(255, 222, 0));
            for (int i = 0; i < 4; i++) {
                int sx = x + w * (36 + (i % 2) * 14) / 100;
                int sy = y + h * (8 + (i / 2) * 16) / 100;
                star(sx, sy, imin(w, h) / 12, ui_rgb(255, 222, 0));
            }
            break;
        case LANG_RU:
            ui_rect(x, y, w, h / 3, ui_rgb(255, 255, 255));
            ui_rect(x, y + h / 3, w, h / 3, ui_rgb(0, 57, 166));
            ui_rect(x, y + 2 * h / 3, w, h - 2 * h / 3, ui_rgb(213, 43, 30));
            break;
        case LANG_NL:
            ui_rect(x, y, w, h / 3, ui_rgb(174, 28, 40));
            ui_rect(x, y + h / 3, w, h / 3, ui_rgb(255, 255, 255));
            ui_rect(x, y + 2 * h / 3, w, h - 2 * h / 3, ui_rgb(33, 70, 139));
            break;
        case LANG_PL:
            ui_rect(x, y, w, h / 2, ui_rgb(255, 255, 255));
            ui_rect(x, y + h / 2, w, h - h / 2, ui_rgb(220, 20, 60));
            break;
        default:
            ui_rect(x, y, w, h, ui_rgb(60, 60, 70));
            break;
    }
}

/* ------------------------------------------------------------------- set-up */

int ui_init(SDL_Renderer *renderer) {
    R = renderer;
    if (TTF_Init() != 0) return -1;
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    memset(g_texts, 0, sizeof(g_texts));
    memset(g_arts, 0, sizeof(g_arts));
    memset(g_doms, 0, sizeof(g_doms));
    memset(g_corners, 0, sizeof(g_corners));
    memset(g_discs, 0, sizeof(g_discs));
    g_corner_next = g_disc_next = 0;
    g_frame = 1;
    return 0;
}

void ui_shutdown(void) {
    for (int i = 0; i < TEXT_CACHE_MAX; i++) if (g_texts[i].tex) SDL_DestroyTexture(g_texts[i].tex);
    for (int i = 0; i < ART_CACHE_MAX; i++) if (g_arts[i].tex) SDL_DestroyTexture(g_arts[i].tex);
    for (int i = 0; i < CORNER_CACHE_MAX; i++) if (g_corners[i].tex) SDL_DestroyTexture(g_corners[i].tex);
    for (int i = 0; i < 24; i++) if (g_discs[i].tex) SDL_DestroyTexture(g_discs[i].tex);
    memset(g_texts, 0, sizeof(g_texts));
    memset(g_arts, 0, sizeof(g_arts));
    memset(g_corners, 0, sizeof(g_corners));
    memset(g_discs, 0, sizeof(g_discs));
    for (int i = 0; i < FONT_COUNT; i++) {
        if (g_primary[i]) { TTF_CloseFont(g_primary[i]); g_primary[i] = NULL; }
        if (g_fallback[i]) { TTF_CloseFont(g_fallback[i]); g_fallback[i] = NULL; }
    }
    TTF_Quit();
    IMG_Quit();
}

void ui_begin_frame(void) {
    g_frame++;
    g_clip_depth = 0;
    SDL_RenderSetClipRect(R, NULL);
    SDL_SetRenderDrawBlendMode(R, SDL_BLENDMODE_BLEND);
    (void)wrap_lines;
}
