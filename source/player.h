#ifndef MUSICTT_PLAYER_H
#define MUSICTT_PLAYER_H

#include "common.h"

#define PLAYER_QUEUE_MAX 512

typedef struct {
    char key[ID_MAX_LEN * 2];
    char title[NAME_MAX_LEN];
    char playlist[NAME_MAX_LEN];
    char file[PATH_MAX_LEN];
    char logo[PATH_MAX_LEN];
} PlayerItem;

typedef enum {
    PLAYER_IDLE = 0,
    PLAYER_LOADING,
    PLAYER_PLAYING,
    PLAYER_PAUSED,
    PLAYER_ERROR
} PlayerStatus;

/* Opens the console audio output. Returns 0 on success. */
int player_init(void);
void player_shutdown(void);

void player_set_queue(const PlayerItem *items, int count);
void player_play_index(int index);
void player_toggle_pause(void);
void player_next(void);          /* next track, wraps to the first */
void player_previous(void);      /* restart when >3s, else previous track */
void player_seek(double seconds);
void player_set_volume(int volume);  /* 0..100 */
int player_volume(void);

/* Call once per frame: applies finished loads and auto-advances. */
void player_poll(void);

PlayerStatus player_status(void);
int player_index(void);
int player_count(void);
const PlayerItem *player_current(void);
const char *player_error(void);
double player_position(void);
double player_duration(void);

#endif
