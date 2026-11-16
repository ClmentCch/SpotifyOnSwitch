#ifndef MUSICTT_DECODERS_H
#define MUSICTT_DECODERS_H

#include "common.h"

typedef struct {
    int16_t *samples;   /* interleaved S16 */
    uint64_t frames;
    int channels;
    int rate;
} DecodedAudio;

/* Decodes a whole file into memory. Returns 0 on success. */
int decode_audio(const char *path, DecodedAudio *out, char *err, size_t err_size, volatile int *cancel);

#endif
