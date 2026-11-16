#include "player.h"

#include <SDL.h>
#include <math.h>

#include "decoders.h"

#define DEVICE_RATE 48000
#define DEVICE_FRAMES 1024

typedef struct {
    int16_t *samples;
    uint64_t frames;
    int channels;
    int rate;
} Pcm;

static SDL_AudioDeviceID g_dev;
static SDL_AudioSpec g_spec;
static SDL_AudioStream *g_stream;

static SDL_mutex *g_mtx;
static Pcm g_pcm;              /* current track, read by the audio callback */
static Pcm g_pending;          /* produced by the loader thread */
static SDL_atomic_t g_pending_state;  /* 0 none, 1 ready, 2 failed */
static char g_pending_error[160];
static SDL_Thread *g_loader;
static volatile int g_cancel;

static SDL_atomic_t g_pos;      /* current frame */
static SDL_atomic_t g_ended;
static SDL_atomic_t g_gain_q;   /* volume * 1000 */
static float g_gain;

static PlayerStatus g_status = PLAYER_IDLE;
static char g_error[160];
static int g_volume = 70;

static PlayerItem g_queue[PLAYER_QUEUE_MAX];
static int g_queue_count;
static int g_queue_index;

int player_volume(void) { return g_volume; }

static void audio_cb(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    memset(stream, 0, (size_t)len);

    if (!g_pcm.samples || !g_stream || g_pcm.rate <= 0) {
        return;
    }

    int written = 0;
    int guard = 0;
    while (written < len && guard++ < 64) {
        int got = SDL_AudioStreamGet(g_stream, stream + written, len - written);
        if (got > 0) {
            written += got;
            continue;
        }
        if (SDL_AtomicGet(&g_ended)) break;

        uint64_t pos = (uint64_t)SDL_AtomicGet(&g_pos);
        if (pos >= g_pcm.frames) {
            SDL_AtomicSet(&g_ended, 1);
            break;
        }
        uint64_t left = g_pcm.frames - pos;
        int chunk = (int)(left > 2048 ? 2048 : left);
        if (SDL_AudioStreamPut(g_stream, g_pcm.samples + pos * (uint64_t)g_pcm.channels,
                               chunk * g_pcm.channels * (int)sizeof(int16_t)) < 0) {
            break;
        }
        SDL_AtomicSet(&g_pos, (int)(pos + (uint64_t)chunk));
    }

    /* Volume with a short ramp so dragging the slider never clicks. */
    float target = (float)SDL_AtomicGet(&g_gain_q) / 1000.0f;
    int16_t *samples = (int16_t *)stream;
    int frames = len / (int)(sizeof(int16_t) * 2);
    for (int i = 0; i < frames; i++) {
        g_gain += (target - g_gain) * 0.02f;
        samples[i * 2] = (int16_t)(samples[i * 2] * g_gain);
        samples[i * 2 + 1] = (int16_t)(samples[i * 2 + 1] * g_gain);
    }
}

int player_init(void) {
    if (g_dev) return 0;
    g_mtx = SDL_CreateMutex();
    if (!g_mtx) return -1;

    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = DEVICE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = DEVICE_FRAMES;
    want.callback = audio_cb;

    g_dev = SDL_OpenAudioDevice(NULL, 0, &want, &g_spec,
                                SDL_AUDIO_ALLOW_FREQUENCY_CHANGE |
                                SDL_AUDIO_ALLOW_FORMAT_CHANGE |
                                SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    if (!g_dev) {
        snprintf(g_error, sizeof(g_error), "%s", SDL_GetError());
        return -1;
    }
    SDL_PauseAudioDevice(g_dev, 1);
    SDL_AtomicSet(&g_gain_q, g_volume * 1000 / 100);
    g_gain = (float)g_volume / 100.0f;
    g_status = PLAYER_IDLE;
    return 0;
}

void player_shutdown(void) {
    g_cancel = 1;
    if (g_loader) {
        SDL_WaitThread(g_loader, NULL);
        g_loader = NULL;
    }
    if (g_dev) {
        SDL_PauseAudioDevice(g_dev, 1);
        SDL_CloseAudioDevice(g_dev);
        g_dev = 0;
    }
    if (g_stream) {
        SDL_FreeAudioStream(g_stream);
        g_stream = NULL;
    }
    free(g_pcm.samples);
    memset(&g_pcm, 0, sizeof(g_pcm));
    free(g_pending.samples);
    memset(&g_pending, 0, sizeof(g_pending));
    if (g_mtx) {
        SDL_DestroyMutex(g_mtx);
        g_mtx = NULL;
    }
}

void player_set_queue(const PlayerItem *items, int count) {
    if (count > PLAYER_QUEUE_MAX) count = PLAYER_QUEUE_MAX;
    if (count < 0) count = 0;
    if (items && count > 0) memcpy(g_queue, items, (size_t)count * sizeof(PlayerItem));
    g_queue_count = count;
    if (g_queue_index >= count) g_queue_index = 0;
}

const PlayerItem *player_current(void) {
    if (g_queue_count <= 0 || g_queue_index < 0 || g_queue_index >= g_queue_count) return NULL;
    return &g_queue[g_queue_index];
}

int player_index(void) { return g_queue_count ? g_queue_index : -1; }
int player_count(void) { return g_queue_count; }
PlayerStatus player_status(void) { return g_status; }
const char *player_error(void) { return g_error; }

double player_duration(void) {
    if (!g_pcm.samples || g_pcm.rate <= 0) return 0.0;
    return (double)g_pcm.frames / (double)g_pcm.rate;
}

double player_position(void) {
    if (!g_pcm.samples || g_pcm.rate <= 0) return 0.0;
    int pos = SDL_AtomicGet(&g_pos);
    if (pos < 0) pos = 0;
    return (double)pos / (double)g_pcm.rate;
}

static int loader_main(void *data) {
    char *path = (char *)data;
    DecodedAudio decoded;
    char err[160] = {0};
    int rc = decode_audio(path, &decoded, err, sizeof(err), &g_cancel);
    free(path);

    if (g_cancel) {
        free(decoded.samples);
        return 0;
    }

    SDL_LockMutex(g_mtx);
    free(g_pending.samples);
    memset(&g_pending, 0, sizeof(g_pending));
    if (rc == 0) {
        g_pending.samples = decoded.samples;
        g_pending.frames = decoded.frames;
        g_pending.channels = decoded.channels;
        g_pending.rate = decoded.rate;
        snprintf(g_pending_error, sizeof(g_pending_error), "%s", "");
        SDL_AtomicSet(&g_pending_state, decoded.frames ? 1 : 2);
    } else {
        snprintf(g_pending_error, sizeof(g_pending_error), "%s", err);
        SDL_AtomicSet(&g_pending_state, 2);
    }
    SDL_UnlockMutex(g_mtx);
    return 0;
}

void player_play_index(int index) {
    if (index < 0 || index >= g_queue_count) return;

    /* Ask a running loader to stop and collect it before starting another. */
    if (g_loader) {
        g_cancel = 1;
        SDL_WaitThread(g_loader, NULL);
        g_loader = NULL;
    }
    g_cancel = 0;

    g_queue_index = index;
    SDL_AtomicSet(&g_pending_state, 0);
    SDL_AtomicSet(&g_ended, 0);
    SDL_AtomicSet(&g_pos, 0);
    g_error[0] = 0;
    g_status = PLAYER_LOADING;

    if (g_dev) SDL_PauseAudioDevice(g_dev, 1);

    const PlayerItem *item = &g_queue[index];
    char *path = (char *)malloc(strlen(item->file) + 1);
    if (!path) {
        g_status = PLAYER_ERROR;
        snprintf(g_error, sizeof(g_error), "out of memory");
        return;
    }
    strcpy(path, item->file);
    g_loader = SDL_CreateThread(loader_main, "musictt-load", path);
    if (!g_loader) {
        free(path);
        g_status = PLAYER_ERROR;
        snprintf(g_error, sizeof(g_error), "cannot start loader");
    }
}

void player_poll(void) {
    int state = SDL_AtomicGet(&g_pending_state);
    if (state != 0 && g_loader) {
        SDL_WaitThread(g_loader, NULL);
        g_loader = NULL;
    }

    if (state == 2) {
        SDL_LockMutex(g_mtx);
        snprintf(g_error, sizeof(g_error), "%s", g_pending_error);
        SDL_UnlockMutex(g_mtx);
        SDL_AtomicSet(&g_pending_state, 0);
        g_status = PLAYER_ERROR;
        return;
    }

    if (state == 1) {
        SDL_LockMutex(g_mtx);
        Pcm next = g_pending;
        memset(&g_pending, 0, sizeof(g_pending));
        SDL_AtomicSet(&g_pending_state, 0);
        SDL_UnlockMutex(g_mtx);

        if (g_dev) SDL_LockAudioDevice(g_dev);
        free(g_pcm.samples);
        g_pcm = next;
        if (g_stream) {
            SDL_FreeAudioStream(g_stream);
            g_stream = NULL;
        }
        if (g_dev && g_pcm.samples) {
            g_stream = SDL_NewAudioStream(AUDIO_S16SYS, (Uint8)g_pcm.channels, g_pcm.rate,
                                          g_spec.format, g_spec.channels, g_spec.freq);
        }
        SDL_AtomicSet(&g_pos, 0);
        SDL_AtomicSet(&g_ended, 0);
        if (g_dev) SDL_UnlockAudioDevice(g_dev);

        if (!g_stream) {
            g_status = PLAYER_ERROR;
            snprintf(g_error, sizeof(g_error), "audio conversion failed");
            return;
        }
        g_status = PLAYER_PLAYING;
        if (g_dev) SDL_PauseAudioDevice(g_dev, 0);
        return;
    }

    if (g_status == PLAYER_PLAYING && SDL_AtomicGet(&g_ended)) {
        if (g_queue_count > 0 && g_queue_index + 1 < g_queue_count) {
            player_play_index(g_queue_index + 1);
        } else {
            g_status = PLAYER_PAUSED;
        }
    }
}

void player_toggle_pause(void) {
    if (g_status == PLAYER_PLAYING) {
        if (g_dev) SDL_PauseAudioDevice(g_dev, 1);
        g_status = PLAYER_PAUSED;
        return;
    }
    if (g_status == PLAYER_PAUSED) {
        if (SDL_AtomicGet(&g_pos) >= (int)g_pcm.frames) player_seek(0.0);
        if (g_dev) SDL_PauseAudioDevice(g_dev, 0);
        g_status = PLAYER_PLAYING;
        return;
    }
    if (g_status == PLAYER_ERROR || g_status == PLAYER_IDLE || g_status == PLAYER_LOADING) {
        if (g_queue_count > 0) player_play_index(g_status == PLAYER_IDLE ? g_queue_index : g_queue_index);
    }
}

void player_next(void) {
    if (g_queue_count <= 0) return;
    int next = g_queue_index + 1;
    if (next >= g_queue_count) next = 0;
    player_play_index(next);
}

void player_previous(void) {
    if (g_queue_count <= 0) return;
    if (player_position() > 3.0 || g_queue_index == 0) {
        player_seek(0.0);
        return;
    }
    player_play_index(g_queue_index - 1);
}

void player_seek(double seconds) {
    if (!g_pcm.samples || g_pcm.rate <= 0) return;
    if (seconds < 0) seconds = 0;
    double duration = player_duration();
    if (seconds > duration) seconds = duration;
    int frame = (int)(seconds * (double)g_pcm.rate);
    if (frame < 0) frame = 0;
    if (frame > (int)g_pcm.frames) frame = (int)g_pcm.frames;
    if (g_dev) SDL_LockAudioDevice(g_dev);
    if (g_stream) SDL_AudioStreamClear(g_stream);
    SDL_AtomicSet(&g_pos, frame);
    SDL_AtomicSet(&g_ended, 0);
    if (g_dev) SDL_UnlockAudioDevice(g_dev);
}

void player_set_volume(int volume) {
    g_volume = iclamp(volume, 0, 100);
    SDL_AtomicSet(&g_gain_q, g_volume * 1000 / 100);
}
