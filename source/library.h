#ifndef MUSICTT_LIBRARY_H
#define MUSICTT_LIBRARY_H

#include "common.h"

typedef struct {
    char id[ID_MAX_LEN];
    int number;
    char name[NAME_MAX_LEN];
    char file[PATH_MAX_LEN];   /* resolved absolute sdmc path */
    char logo[PATH_MAX_LEN];   /* resolved absolute sdmc path, "" when missing */
    int has_file;
} Track;

typedef struct {
    char id[ID_MAX_LEN];
    char slug[NAME_MAX_LEN];   /* folder name */
    char name[NAME_MAX_LEN];
    char dir[PATH_MAX_LEN];
    char logo[PATH_MAX_LEN];
    Track *tracks;
    int count;
} Playlist;

typedef struct {
    Playlist *items;
    int count;
    int total_tracks;
    int errors;                /* playlists skipped or partly broken */
    char last_error[192];
} Library;

/* Override the sdmc:/MUSIC root (used by the host tests). */
void library_set_root(const char *root);
const char *library_root(void);

/* Scans PLAYLISTS/. Returns 0 on success; an empty library is not an error.
   On a missing/unreadable PLAYLISTS directory returns -1 with last_error set. */
int library_scan(Library *lib);
void library_free(Library *lib);

int library_find_playlist(const Library *lib, const char *id);
int library_find_track(const Library *lib, const char *playlist_id, const char *track_id);
int library_track_supported(const char *path);

#endif
