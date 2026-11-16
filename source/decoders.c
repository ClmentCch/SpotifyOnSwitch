#include "decoders.h"

#define DR_MP3_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#include "dr_mp3.h"
#include "dr_flac.h"
#include "dr_wav.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

/* One track is fully decoded before playback starts: seeking is exact and the
   audio callback never touches the SD card. 192 MiB is about 18 minutes of
   44.1 kHz stereo. */
#define PCM_BUDGET_BYTES (192u * 1024u * 1024u)

typedef struct {
    int16_t *samples;
    uint64_t frames;
    uint64_t capacity;   /* frames */
    int channels;
    int rate;
} Builder;

static int builder_reserve(Builder *b, uint64_t extra_frames) {
    uint64_t need = b->frames + extra_frames;
    if (need <= b->capacity) return 0;
    uint64_t cap = b->capacity ? b->capacity : 65536;
    while (cap < need) cap *= 2;
    uint64_t bytes = cap * (uint64_t)b->channels * 2u;
    if (bytes > PCM_BUDGET_BYTES) {
        /* Try the exact size before giving up. */
        cap = need;
        bytes = cap * (uint64_t)b->channels * 2u;
        if (bytes > PCM_BUDGET_BYTES) return -2;
    }
    int16_t *grown = (int16_t *)realloc(b->samples, (size_t)bytes);
    if (!grown) return -1;
    b->samples = grown;
    b->capacity = cap;
    return 0;
}

static int builder_append(Builder *b, const int16_t *src, uint64_t frames) {
    int rc = builder_reserve(b, frames);
    if (rc != 0) return rc;
    memcpy(b->samples + b->frames * (uint64_t)b->channels, src,
           (size_t)(frames * (uint64_t)b->channels * 2u));
    b->frames += frames;
    return 0;
}

static void set_err(char *err, size_t n, const char *text) {
    if (err && n) snprintf(err, n, "%s", text);
}

static int decode_mp3(const char *path, DecodedAudio *out, char *err, size_t err_size, volatile int *cancel) {
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, path, NULL)) {
        set_err(err, err_size, "mp3 open failed");
        return -1;
    }
    Builder b;
    memset(&b, 0, sizeof(b));
    b.channels = (int)mp3.channels;
    b.rate = (int)mp3.sampleRate;
    if (b.channels < 1 || b.channels > 2) {
        drmp3_uninit(&mp3);
        set_err(err, err_size, "channel count");
        return -1;
    }

    int16_t chunk[4096 * 2];
    int rc = 0;
    for (;;) {
        if (cancel && *cancel) { rc = -3; break; }
        uint64_t got = drmp3_read_pcm_frames_s16(&mp3, 4096, chunk);
        if (got == 0) break;
        rc = builder_append(&b, chunk, got);
        if (rc != 0) break;
    }
    drmp3_uninit(&mp3);
    if (rc != 0) {
        free(b.samples);
        set_err(err, err_size, rc == -2 ? "too long" : (rc == -3 ? "cancelled" : "out of memory"));
        return -1;
    }
    out->samples = b.samples;
    out->frames = b.frames;
    out->channels = b.channels;
    out->rate = b.rate;
    return 0;
}

static int decode_wav(const char *path, DecodedAudio *out, char *err, size_t err_size, volatile int *cancel) {
    drwav wav;
    if (!drwav_init_file(&wav, path, NULL)) {
        set_err(err, err_size, "wav open failed");
        return -1;
    }
    Builder b;
    memset(&b, 0, sizeof(b));
    b.channels = (int)wav.channels;
    b.rate = (int)wav.sampleRate;
    if (b.channels < 1 || b.channels > 2) {
        drwav_uninit(&wav);
        set_err(err, err_size, "channel count");
        return -1;
    }
    int16_t chunk[4096 * 2];
    int rc = 0;
    for (;;) {
        if (cancel && *cancel) { rc = -3; break; }
        uint64_t got = drwav_read_pcm_frames_s16(&wav, 4096, chunk);
        if (got == 0) break;
        rc = builder_append(&b, chunk, got);
        if (rc != 0) break;
    }
    drwav_uninit(&wav);
    if (rc != 0) {
        free(b.samples);
        set_err(err, err_size, rc == -2 ? "too long" : (rc == -3 ? "cancelled" : "out of memory"));
        return -1;
    }
    out->samples = b.samples;
    out->frames = b.frames;
    out->channels = b.channels;
    out->rate = b.rate;
    return 0;
}

static int decode_flac(const char *path, DecodedAudio *out, char *err, size_t err_size, volatile int *cancel) {
    drflac *flac = drflac_open_file(path, NULL);
    if (!flac) {
        set_err(err, err_size, "flac open failed");
        return -1;
    }
    Builder b;
    memset(&b, 0, sizeof(b));
    b.channels = (int)flac->channels;
    b.rate = (int)flac->sampleRate;
    if (b.channels < 1 || b.channels > 2) {
        drflac_close(flac);
        set_err(err, err_size, "channel count");
        return -1;
    }
    int16_t chunk[4096 * 2];
    int rc = 0;
    for (;;) {
        if (cancel && *cancel) { rc = -3; break; }
        uint64_t got = drflac_read_pcm_frames_s16(flac, 4096, chunk);
        if (got == 0) break;
        rc = builder_append(&b, chunk, got);
        if (rc != 0) break;
    }
    drflac_close(flac);
    if (rc != 0) {
        free(b.samples);
        set_err(err, err_size, rc == -2 ? "too long" : (rc == -3 ? "cancelled" : "out of memory"));
        return -1;
    }
    out->samples = b.samples;
    out->frames = b.frames;
    out->channels = b.channels;
    out->rate = b.rate;
    return 0;
}

static int decode_ogg(const char *path, DecodedAudio *out, char *err, size_t err_size, volatile int *cancel) {
    int error = 0;
    stb_vorbis *v = stb_vorbis_open_filename(path, &error, NULL);
    if (!v) {
        set_err(err, err_size, "ogg open failed");
        return -1;
    }
    stb_vorbis_info info = stb_vorbis_get_info(v);
    Builder b;
    memset(&b, 0, sizeof(b));
    b.channels = (int)info.channels;
    b.rate = (int)info.sample_rate;
    if (b.channels < 1 || b.channels > 2) {
        stb_vorbis_close(v);
        set_err(err, err_size, "channel count");
        return -1;
    }
    int16_t chunk[4096 * 2];
    int rc = 0;
    for (;;) {
        if (cancel && *cancel) { rc = -3; break; }
        int got = stb_vorbis_get_samples_short_interleaved(v, b.channels, chunk, 4096 * b.channels);
        if (got <= 0) break;
        rc = builder_append(&b, chunk, (uint64_t)got);
        if (rc != 0) break;
    }
    stb_vorbis_close(v);
    if (rc != 0) {
        free(b.samples);
        set_err(err, err_size, rc == -2 ? "too long" : (rc == -3 ? "cancelled" : "out of memory"));
        return -1;
    }
    out->samples = b.samples;
    out->frames = b.frames;
    out->channels = b.channels;
    out->rate = b.rate;
    return 0;
}

int decode_audio(const char *path, DecodedAudio *out, char *err, size_t err_size, volatile int *cancel) {
    memset(out, 0, sizeof(*out));
    if (!path) {
        set_err(err, err_size, "no file");
        return -1;
    }
    if (has_suffix_ci(path, ".mp3")) return decode_mp3(path, out, err, err_size, cancel);
    if (has_suffix_ci(path, ".wav")) return decode_wav(path, out, err, err_size, cancel);
    if (has_suffix_ci(path, ".flac")) return decode_flac(path, out, err, err_size, cancel);
    if (has_suffix_ci(path, ".ogg") || has_suffix_ci(path, ".oga")) return decode_ogg(path, out, err, err_size, cancel);
    set_err(err, err_size, "unsupported format");
    return -1;
}
