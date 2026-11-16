#ifndef MUSICTT_COMMON_H
#define MUSICTT_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MUSICTT_VERSION
#define MUSICTT_VERSION "1.0.0"
#endif

/* Everything the app owns lives under this folder on the SD card. */
#define MUSICTT_ROOT      "sdmc:/MUSIC"
#define MUSICTT_PLAYLISTS MUSICTT_ROOT "/PLAYLISTS"
#define MUSICTT_LOGO      MUSICTT_ROOT "/LOGO"
#define MUSICTT_CONFIG    MUSICTT_ROOT "/CONFIG"
#define MUSICTT_SETTINGS  MUSICTT_CONFIG "/settings.json"

#define PATH_MAX_LEN 512
#define NAME_MAX_LEN 128
#define ID_MAX_LEN   96

static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }
static inline float fclampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline int iclamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

/* Case-insensitive suffix test, used for audio extensions. */
static inline int has_suffix_ci(const char *s, const char *suffix) {
    size_t n = strlen(s), m = strlen(suffix);
    if (n < m) return 0;
    for (size_t i = 0; i < m; i++) {
        char a = s[n - m + i], b = suffix[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b) return 0;
    }
    return 1;
}

#endif
