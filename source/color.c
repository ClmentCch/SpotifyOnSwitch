#include "color.h"

static int rgb_max(int a, int b, int c) { return a > b ? (a > c ? a : c) : (b > c ? b : c); }
static int rgb_min(int a, int b, int c) { return a < b ? (a < c ? a : c) : (b < c ? b : c); }

Rgb color_dominant(const uint8_t *pixels, int width, int height, int pitch) {
    Rgb fallback = {124, 92, 255};
    if (!pixels || width <= 0 || height <= 0) return fallback;

    enum { HB = 18, SB = 4, VB = 4 };
    static uint64_t weight[HB][SB][VB];
    static uint64_t sum_r[HB][SB][VB], sum_g[HB][SB][VB], sum_b[HB][SB][VB];
    memset(weight, 0, sizeof(weight));
    memset(sum_r, 0, sizeof(sum_r));
    memset(sum_g, 0, sizeof(sum_g));
    memset(sum_b, 0, sizeof(sum_b));

    int step_x = width > 320 ? width / 240 : 1;
    int step_y = height > 320 ? height / 240 : 1;

    for (int y = 0; y < height; y += step_y) {
        const uint8_t *row = pixels + (size_t)y * (size_t)pitch;
        for (int x = 0; x < width; x += step_x) {
            const uint8_t *p = row + (size_t)x * 4;
            int r = p[0], g = p[1], b = p[2];
            int mx = rgb_max(r, g, b), mn = rgb_min(r, g, b);
            int v = mx;
            int s = mx == 0 ? 0 : (mx - mn) * 255 / mx;
            if (v < 28 || v > 246) continue;       /* skip pure black/white */
            if (s < 12) continue;                  /* skip greys */

            float h;
            int d = mx - mn;
            if (d == 0) h = 0;
            else if (mx == r) h = (float)(g - b) / (float)d;
            else if (mx == g) h = 2.0f + (float)(b - r) / (float)d;
            else h = 4.0f + (float)(r - g) / (float)d;
            h *= 60.0f;
            if (h < 0) h += 360.0f;

            int hi = (int)(h / (360.0f / HB)) % HB;
            int si = iclamp(s * SB / 256, 0, SB - 1);
            int vi = iclamp(v * VB / 256, 0, VB - 1);
            /* Mid tones carry the mood best; very dark/bright pixels weigh less. */
            int mid = 128 - abs(v - 150);
            if (mid < 8) mid = 8;
            uint64_t w = (uint64_t)(mid * (s + 30));
            weight[hi][si][vi] += w;
            sum_r[hi][si][vi] += (uint64_t)r * w;
            sum_g[hi][si][vi] += (uint64_t)g * w;
            sum_b[hi][si][vi] += (uint64_t)b * w;
        }
    }

    int best = -1, bh = 0, bs = 0, bv = 0;
    for (int h = 0; h < HB; h++)
        for (int s = 0; s < SB; s++)
            for (int v = 0; v < VB; v++) {
                int score = (int)weight[h][s][v];
                /* Prefer saturated buckets a little. */
                score += (int)(weight[h][s][v] * (uint32_t)s / (VB * 2));
                if (best < 0 || score > (long long)best) { best = (int)(score > 2000000000LL ? 2000000000LL : score); bh = h; bs = s; bv = v; }
            }

    if (best <= 0) return fallback;
    uint64_t w = weight[bh][bs][bv];
    if (!w) return fallback;
    Rgb out;
    out.r = (uint8_t)(sum_r[bh][bs][bv] / w);
    out.g = (uint8_t)(sum_g[bh][bs][bv] / w);
    out.b = (uint8_t)(sum_b[bh][bs][bv] / w);
    return out;
}

static Rgb mix(Rgb a, Rgb b, float t) {
    Rgb o;
    o.r = (uint8_t)(a.r + (b.r - a.r) * t);
    o.g = (uint8_t)(a.g + (b.g - a.g) * t);
    o.b = (uint8_t)(a.b + (b.b - a.b) * t);
    return o;
}

static Rgb scale(Rgb c, float f) {
    Rgb o;
    o.r = (uint8_t)iclamp((int)(c.r * f), 0, 255);
    o.g = (uint8_t)iclamp((int)(c.g * f), 0, 255);
    o.b = (uint8_t)iclamp((int)(c.b * f), 0, 255);
    return o;
}

ColorTheme color_theme(Rgb base) {
    ColorTheme t;
    Rgb night = {9, 9, 15};
    Rgb lifted = mix(base, (Rgb){255, 255, 255}, 0.18f);
    t.top = mix(night, lifted, 0.42f);
    t.bottom = mix(night, lifted, 0.10f);
    t.accent = mix(lifted, (Rgb){255, 255, 255}, 0.10f);
    t.glow = mix(night, base, 0.55f);
    /* Keep the accent readable on the dark gradient. */
    int mx = rgb_max(t.accent.r, t.accent.g, t.accent.b);
    if (mx < 150) t.accent = scale(t.accent, 150.0f / (float)(mx ? mx : 1));
    (void)scale;
    return t;
}
