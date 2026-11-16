#include "app.h"

#include <dirent.h>
#include <math.h>
#include <sys/stat.h>

App g_app;
SDL_Renderer *g_renderer;
SDL_Window *g_window;

static SDL_GameController *g_pad;
static int g_pad_index = -1;
static double g_axis_repeat;
static int g_axis_dir;

int app_has_track(void) { return player_current() != NULL; }

/* --------------------------------------------------------------- utilities */

static void ensure_dir(const char *path) {
    mkdir(path, 0777);
}

static int dir_ok(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int prepare_dirs(char *err, size_t err_size) {
    ensure_dir(MUSICTT_ROOT);
    ensure_dir(MUSICTT_PLAYLISTS);
    ensure_dir(MUSICTT_LOGO);
    ensure_dir(MUSICTT_CONFIG);
    if (!dir_ok(MUSICTT_PLAYLISTS) || !dir_ok(MUSICTT_LOGO) || !dir_ok(MUSICTT_CONFIG)) {
        snprintf(err, err_size, "%s", MUSICTT_ROOT);
        return -1;
    }
    return 0;
}

void app_save(void) {
    settings_save(&g_app.cfg);
    g_app.needs_save = 0;
    g_app.last_save = g_app.time;
}

void app_toast(const char *text, int kind) {
    if (!text || !*text) return;
    if (g_app.toast_count < 3) {
        Toast *t = &g_app.toasts[g_app.toast_count++];
        snprintf(t->text, sizeof(t->text), "%s", text);
        t->kind = kind;
        t->t = g_app.time;
    } else {
        Toast *t = &g_app.toasts[2];
        snprintf(t->text, sizeof(t->text), "%s", text);
        t->kind = kind;
        t->t = g_app.time;
    }
    g_app.toast_time = g_app.time;
}

void app_hit(int action, int arg, int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    if (y + h < g_app.view_top || y > g_app.view_bottom) return;
    if (x + w < 0 || x > APP_W || y + h < 0 || y > APP_H) return;
    if (g_app.hit_count >= (int)(sizeof(g_app.hits) / sizeof(g_app.hits[0]))) return;
    Hit *hit = &g_app.hits[g_app.hit_count++];
    hit->action = action;
    hit->arg = arg;
    hit->rect.x = x;
    hit->rect.y = y;
    hit->rect.w = w;
    hit->rect.h = h;
}

void app_refresh(void) {
    library_free(&g_app.lib);
    if (library_scan(&g_app.lib) != 0) {
        app_toast(i18n_str(S_ERR_SD, g_app.lib.last_error), 1);
        g_app.page = PAGE_HOME;
        return;
    }
    if (g_app.lib.errors > 0 && g_app.lib.last_error[0]) {
        app_toast(i18n_str(S_TOAST_PLAYLIST_ERROR, g_app.lib.last_error), 1);
    }
    if (g_app.playlist_index >= g_app.lib.count) g_app.playlist_index = 0;
    app_toast(i18n_get(S_REFRESHED), 0);
}

void app_play_track(int playlist_index, int track_index) {
    if (playlist_index < 0 || playlist_index >= g_app.lib.count) return;
    Playlist *pl = &g_app.lib.items[playlist_index];
    if (track_index < 0 || track_index >= pl->count) return;

    Track *tr = &pl->tracks[track_index];
    if (!tr->has_file) {
        if (!library_track_supported(tr->file)) app_toast(i18n_str(S_ERR_TRACK_FORMAT, tr->file), 1);
        else app_toast(i18n_str(S_ERR_TRACK_MISSING, tr->file), 1);
        return;
    }
    if (!g_app.audio_ok) {
        app_toast(i18n_get(S_ERR_AUDIO), 1);
        return;
    }

    static PlayerItem items[PLAYER_QUEUE_MAX];
    int count = imin(pl->count, PLAYER_QUEUE_MAX);
    for (int i = 0; i < count; i++) {
        PlayerItem *it = &items[i];
        memset(it, 0, sizeof(*it));
        snprintf(it->key, sizeof(it->key), "%s/%s", pl->id, pl->tracks[i].id);
        snprintf(it->title, sizeof(it->title), "%s", pl->tracks[i].name);
        snprintf(it->playlist, sizeof(it->playlist), "%s", pl->name);
        snprintf(it->file, sizeof(it->file), "%s", pl->tracks[i].file);
        snprintf(it->logo, sizeof(it->logo), "%s", pl->tracks[i].logo);
    }
    player_set_queue(items, count);
    player_play_index(track_index);
    g_app.now_open = 1;
    g_app.now_anim = 0.0;
    snprintf(g_app.cfg.last_playlist, sizeof(g_app.cfg.last_playlist), "%s", pl->id);
    snprintf(g_app.cfg.last_track, sizeof(g_app.cfg.last_track), "%s", pl->tracks[track_index].id);
    g_app.needs_save = 1;
}

void app_sync_current(void) {
    const PlayerItem *cur = player_current();
    if (!cur) return;
    if (strcmp(cur->key, g_app.cur_key) == 0) return;
    snprintf(g_app.cur_key, sizeof(g_app.cur_key), "%s", cur->key);
    settings_push_recent(&g_app.cfg, cur->key);
    g_app.needs_save = 1;

    Col fallback = C_ACCENT;
    Col dominant = fallback;
    ui_art_dominant(cur->logo, fallback, &dominant);
    Rgb base = { dominant.r, dominant.g, dominant.b };
    ColorTheme theme = color_theme(base);
    g_app.theme_top = ui_from_rgb(theme.top);
    g_app.theme_bottom = ui_from_rgb(theme.bottom);
    g_app.theme_accent = ui_from_rgb(theme.accent);
    g_app.theme_ready = 1;
    g_app.now_anim = 0.0;
}

void app_set_language(Lang lang) {
    if (lang < 0 || lang >= LANG_COUNT) return;
    g_lang = lang;
    g_app.cfg.language = lang;
    ui_reload_fonts(lang);
    app_save();
}

/* ------------------------------------------------------------------ input */

static void logical_point(float wx, float wy, int *lx, int *ly) {
    float fx = 0, fy = 0;
    SDL_RenderWindowToLogical(g_renderer, (int)wx, (int)wy, &fx, &fy);
    *lx = (int)fx;
    *ly = (int)fy;
}

static int hit_at(int x, int y) {
    for (int i = g_app.hit_count - 1; i >= 0; i--) {
        SDL_Rect *r = &g_app.hits[i].rect;
        if (x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h) return i;
    }
    return -1;
}

static void set_focus(int index) {
    if (g_app.hit_count <= 0) { g_app.focus = 0; return; }
    g_app.focus = iclamp(index, 0, g_app.hit_count - 1);
    g_app.focus_visible = 1;
    g_app.focus_t = 0;
}

static void move_focus(int dx, int dy) {
    if (g_app.hit_count <= 0) return;
    if (!g_app.focus_visible) { set_focus(0); return; }
    SDL_Rect cur = g_app.hits[iclamp(g_app.focus, 0, g_app.hit_count - 1)].rect;
    int cx = cur.x + cur.w / 2, cy = cur.y + cur.h / 2;
    int best = -1;
    long best_score = 0;
    for (int i = 0; i < g_app.hit_count; i++) {
        if (i == g_app.focus) continue;
        SDL_Rect r = g_app.hits[i].rect;
        int x = r.x + r.w / 2, y = r.y + r.h / 2;
        int ddx = x - cx, ddy = y - cy;
        if (dx > 0 && ddx < 12) continue;
        if (dx < 0 && ddx > -12) continue;
        if (dy > 0 && ddy < 12) continue;
        if (dy < 0 && ddy > -12) continue;
        long primary, secondary;
        if (dx != 0) {
            primary = (long)ddx * dx;
            secondary = (long)ddy * ddy;
        } else {
            primary = (long)ddy * dy;
            secondary = (long)ddx * ddx;
        }
        long score = primary + secondary * 4;
        if (score < 0) continue;
        if (best < 0 || score < best_score) { best = i; best_score = score; }
    }
    if (best >= 0) set_focus(best);
}

static void ensure_focus_visible(void) {
    if (!g_app.focus_visible || g_app.hit_count <= 0) return;
    Hit *h = &g_app.hits[iclamp(g_app.focus, 0, g_app.hit_count - 1)];
    Page p = g_app.page;
    float *target = &g_app.scroll_target[p];
    int vy0 = g_app.view_top, vy1 = g_app.view_bottom;
    if (h->rect.y < vy0) *target += (float)(vy0 - h->rect.y);
    else if (h->rect.y + h->rect.h > vy1) *target += (float)(vy1 - (h->rect.y + h->rect.h));
    if (*target > 0) *target = 0;
}

static void activate_focused(void) {
    if (!g_app.focus_visible || g_app.hit_count <= 0) return;
    Hit *h = &g_app.hits[iclamp(g_app.focus, 0, g_app.hit_count - 1)];
    app_activate(h->action, h->arg);
}

void app_activate(int action, int arg) {
    switch (action) {
        case ACT_NAV:
            g_app.page = (Page)iclamp(arg, 0, PAGE_COUNT - 1);
            g_app.page_anim = 0;
            g_app.focus_visible = 0;
            g_app.focus = 0;
            break;
        case ACT_OPEN_PLAYLIST:
            if (arg >= 0 && arg < g_app.lib.count) {
                g_app.playlist_index = arg;
                g_app.page = PAGE_PLAYLIST;
                g_app.page_anim = 0;
                g_app.focus_visible = 0;
                g_app.focus = 0;
                snprintf(g_app.cfg.last_playlist, sizeof(g_app.cfg.last_playlist), "%s", g_app.lib.items[arg].id);
                g_app.needs_save = 1;
            }
            break;
        case ACT_PLAY_TRACK:
            app_play_track(g_app.playlist_index, arg);
            break;
        case ACT_PLAY_ALL:
            app_play_track(g_app.playlist_index, 0);
            break;
        case ACT_PLAY_PAIR:
            app_play_track((arg >> 12) & 0xFFF, arg & 0xFFF);
            break;
        case ACT_TOGGLE:
            player_toggle_pause();
            app_sync_current();
            break;
        case ACT_NEXT:
            player_next();
            app_sync_current();
            break;
        case ACT_PREV:
            player_previous();
            app_sync_current();
            break;
        case ACT_OPEN_NOW:
            if (player_current()) {
                g_app.now_open = 1;
                g_app.now_anim = 0;
                g_app.focus_visible = 0;
            }
            break;
        case ACT_CLOSE_NOW:
            g_app.now_open = 0;
            g_app.focus_visible = 0;
            break;
        case ACT_REFRESH:
        case ACT_SET_REFRESH:
            app_refresh();
            break;
        case ACT_LANG:
            if (arg >= 0 && arg < LANG_COUNT) {
                g_app.lang_sel = arg;
                g_lang = (Lang)arg;
                ui_reload_fonts((Lang)arg);
            }
            break;
        case ACT_CONFIRM_LANG: {
            Lang lang = (Lang)iclamp(g_app.lang_sel, 0, LANG_COUNT - 1);
            g_lang = lang;
            g_app.cfg.language = lang;
            ui_reload_fonts(lang);
            char err[192] = {0};
            if (prepare_dirs(err, sizeof(err)) == 0) {
                settings_defaults(&g_app.cfg);
                g_app.cfg.language = lang;
                settings_save(&g_app.cfg);
                g_app.screen = SCR_SD_READY;
            } else {
                snprintf(g_app.sd_error, sizeof(g_app.sd_error), "%s", err);
                app_toast(i18n_get(S_ERR_SD), 1);
            }
            g_app.focus_visible = 0;
            break;
        }
        case ACT_SET_LANG:
            app_set_language((Lang)iclamp(arg, 0, LANG_COUNT - 1));
            break;
        case ACT_SET_MOTION:
            g_app.cfg.animations = !g_app.cfg.animations;
            g_app.needs_save = 1;
            break;
        case ACT_SET_VOLUME:
            g_app.cfg.volume = iclamp(arg, 0, 100);
            player_set_volume(g_app.cfg.volume);
            g_app.needs_save = 1;
            break;
        case ACT_VOL_PLAYER:
            g_app.cfg.volume = iclamp(arg, 0, 100);
            player_set_volume(g_app.cfg.volume);
            g_app.needs_save = 1;
            break;
        case ACT_SEEK:
            break;
        case ACT_OK:
            if (g_app.screen == SCR_SD_READY) g_app.screen = SCR_RELAUNCH;
            g_app.focus_visible = 0;
            break;
        case ACT_QUIT:
            g_app.quit = 1;
            break;
        case ACT_RETRY: {
            char err[192] = {0};
            if (prepare_dirs(err, sizeof(err)) == 0) {
                g_app.screen = SCR_SD_READY;
                g_app.sd_error[0] = 0;
            } else {
                snprintf(g_app.sd_error, sizeof(g_app.sd_error), "%s", err);
            }
            break;
        }
        default:
            break;
    }
}

static void handle_button(int action) {
    app_activate(action, 0);
}

static void handle_pad_button(Uint8 button) {
    switch (button) {
        case SDL_CONTROLLER_BUTTON_A: activate_focused(); break;
        case SDL_CONTROLLER_BUTTON_B:
            if (g_app.now_open) app_activate(ACT_CLOSE_NOW, 0);
            else if (g_app.page == PAGE_PLAYLIST) app_activate(ACT_NAV, PAGE_PLAYLISTS);
            else if (g_app.screen == SCR_LANG) { /* nothing */ }
            break;
        case SDL_CONTROLLER_BUTTON_X: handle_button(ACT_TOGGLE); break;
        case SDL_CONTROLLER_BUTTON_Y: handle_button(ACT_REFRESH); break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            if (g_app.page > 0) app_activate(ACT_NAV, g_app.page - 1);
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            if (g_app.page < PAGE_COUNT - 1) app_activate(ACT_NAV, g_app.page + 1);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: move_focus(0, -1); break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: move_focus(0, 1); break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: move_focus(-1, 0); break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: move_focus(1, 0); break;
        case SDL_CONTROLLER_BUTTON_START:
            if (g_app.page == PAGE_HOME) app_activate(ACT_OPEN_NOW, 0);
            break;
        default: break;
    }
    g_axis_repeat = 0;
}

static void handle_axis(Sint16 x, Sint16 y) {
    const Sint16 dead = 14000;
    int dx = x > dead ? 1 : (x < -dead ? -1 : 0);
    int dy = y > dead ? 1 : (y < -dead ? -1 : 0);
    int dir = dx != 0 ? dx : (dy != 0 ? dy : 0);
    if (dir == 0) { g_axis_dir = 0; g_axis_repeat = 0; return; }
    if (dir != g_axis_dir) {
        g_axis_dir = dir;
        move_focus(dx, dy);
        g_axis_repeat = g_app.time + 0.32;
        return;
    }
    if (g_app.time >= g_axis_repeat) {
        move_focus(dx, dy);
        g_axis_repeat = g_app.time + 0.12;
    }
}

static void touch_start(int x, int y) {
    g_app.touch_down = 1;
    g_app.touch_x = g_app.touch_start_x = x;
    g_app.touch_y = g_app.touch_last_y = g_app.touch_start_y = y;
    g_app.touch_moved = 0;
    g_app.dragging_scroll = 0;
    g_app.dragging_seek = 0;
    g_app.dragging_volume = 0;
    g_app.touch_hit = hit_at(x, y);
    g_app.focus_visible = 0;
    g_app.focus_t = 0;

    if (g_app.touch_hit >= 0) {
        Hit *h = &g_app.hits[g_app.touch_hit];
        if (h->action == ACT_SEEK) {
            g_app.dragging_seek = 1;
            float ratio = (float)(x - h->rect.x) / (float)imax(1, h->rect.w);
            g_app.seek_preview = fclampf(ratio, 0.0f, 1.0f) * player_duration();
        } else if (h->action == ACT_SET_VOLUME || h->action == ACT_VOL_PLAYER) {
            g_app.dragging_volume = 1;
            int v = (int)lroundf(fclampf((float)(x - h->rect.x) / (float)imax(1, h->rect.w), 0.0f, 1.0f) * 100.0f);
            g_app.vol_preview = v;
        }
    }
}

static void touch_move(int x, int y) {
    if (!g_app.touch_down) return;
    g_app.touch_x = x;
    g_app.touch_y = y;
    int dx = x - g_app.touch_start_x;
    int dy = y - g_app.touch_start_y;
    if (!g_app.touch_moved && (abs(dx) > 10 || abs(dy) > 10)) {
        g_app.touch_moved = 1;
        if (!g_app.dragging_seek && !g_app.dragging_volume) g_app.dragging_scroll = 1;
    }

    if (g_app.dragging_seek && g_app.touch_hit >= 0) {
        Hit *h = &g_app.hits[g_app.touch_hit];
        float ratio = (float)(x - h->rect.x) / (float)imax(1, h->rect.w);
        g_app.seek_preview = fclampf(ratio, 0.0f, 1.0f) * player_duration();
    } else if (g_app.dragging_volume && g_app.touch_hit >= 0) {
        Hit *h = &g_app.hits[g_app.touch_hit];
        g_app.vol_preview = (int)lroundf(fclampf((float)(x - h->rect.x) / (float)imax(1, h->rect.w), 0.0f, 1.0f) * 100.0f);
    } else if (g_app.dragging_scroll) {
        Page p = g_app.page;
        g_app.scroll_target[p] += (float)(y - g_app.touch_last_y);
        if (g_app.scroll_target[p] > 0) g_app.scroll_target[p] = 0;
    }
    g_app.touch_last_y = y;
}

static void touch_end(int x, int y) {
    (void)x; (void)y;
    if (g_app.dragging_seek) {
        player_seek(g_app.seek_preview);
        g_app.dragging_seek = 0;
    } else if (g_app.dragging_volume) {
        g_app.cfg.volume = iclamp(g_app.vol_preview, 0, 100);
        player_set_volume(g_app.cfg.volume);
        g_app.needs_save = 1;
        g_app.dragging_volume = 0;
    } else if (!g_app.touch_moved && g_app.touch_hit >= 0) {
        Hit *h = &g_app.hits[g_app.touch_hit];
        app_activate(h->action, h->arg);
    }
    g_app.touch_down = 0;
    g_app.touch_hit = -1;
    g_app.dragging_scroll = 0;
}

static void handle_event(SDL_Event *e) {
    switch (e->type) {
        case SDL_QUIT:
            g_app.quit = 1;
            break;
        case SDL_CONTROLLERDEVICEADDED:
            if (!g_pad) {
                g_pad = SDL_GameControllerOpen(e->cdevice.which);
                if (g_pad) g_pad_index = e->cdevice.which;
            }
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (g_pad && e->cdevice.which == g_pad_index) {
                SDL_GameControllerClose(g_pad);
                g_pad = NULL;
                g_pad_index = -1;
            }
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            handle_pad_button(e->cbutton.button);
            break;
        case SDL_CONTROLLERAXISMOTION:
            if (e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX || e->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                Sint16 x = SDL_GameControllerGetAxis(g_pad, SDL_CONTROLLER_AXIS_LEFTX);
                Sint16 y = SDL_GameControllerGetAxis(g_pad, SDL_CONTROLLER_AXIS_LEFTY);
                handle_axis(x, y);
            } else if (e->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT || e->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
                g_app.cfg.volume = iclamp(g_app.cfg.volume + (e->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT ? 4 : -4), 0, 100);
                player_set_volume(g_app.cfg.volume);
                g_app.needs_save = 1;
            }
            break;
        case SDL_FINGERDOWN: {
            int w = 0, h = 0;
            SDL_GetWindowSize(g_window, &w, &h);
            int x, y;
            logical_point(e->tfinger.x * (float)w, e->tfinger.y * (float)h, &x, &y);
            touch_start(x, y);
            break;
        }
        case SDL_FINGERMOTION: {
            int w = 0, h = 0;
            SDL_GetWindowSize(g_window, &w, &h);
            int x, y;
            logical_point(e->tfinger.x * (float)w, e->tfinger.y * (float)h, &x, &y);
            touch_move(x, y);
            break;
        }
        case SDL_FINGERUP: {
            int w = 0, h = 0;
            SDL_GetWindowSize(g_window, &w, &h);
            int x, y;
            logical_point(e->tfinger.x * (float)w, e->tfinger.y * (float)h, &x, &y);
            touch_end(x, y);
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
            if (e->button.button == SDL_BUTTON_LEFT) {
                int x, y;
                logical_point((float)e->button.x, (float)e->button.y, &x, &y);
                touch_start(x, y);
            }
            break;
        case SDL_MOUSEMOTION:
            if (g_app.touch_down) {
                int x, y;
                logical_point((float)e->motion.x, (float)e->motion.y, &x, &y);
                touch_move(x, y);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (e->button.button == SDL_BUTTON_LEFT) {
                int x, y;
                logical_point((float)e->button.x, (float)e->button.y, &x, &y);
                touch_end(x, y);
            }
            break;
        case SDL_KEYDOWN:
            switch (e->key.keysym.sym) {
                case SDLK_ESCAPE:
                    if (g_app.now_open) app_activate(ACT_CLOSE_NOW, 0);
                    else if (g_app.page == PAGE_PLAYLIST) app_activate(ACT_NAV, PAGE_PLAYLISTS);
                    break;
                case SDLK_RETURN: case SDLK_KP_ENTER: activate_focused(); break;
                case SDLK_SPACE: app_activate(ACT_TOGGLE, 0); break;
                case SDLK_y: app_activate(ACT_REFRESH, 0); break;
                case SDLK_UP: move_focus(0, -1); break;
                case SDLK_DOWN: move_focus(0, 1); break;
                case SDLK_LEFT: move_focus(-1, 0); break;
                case SDLK_RIGHT: move_focus(1, 0); break;
                case SDLK_PLUS: case SDLK_EQUALS:
                    app_activate(ACT_SET_VOLUME, iclamp(g_app.cfg.volume + 5, 0, 100));
                    break;
                case SDLK_MINUS:
                    app_activate(ACT_SET_VOLUME, iclamp(g_app.cfg.volume - 5, 0, 100));
                    break;
                case SDLK_o: if (g_app.now_open) app_activate(ACT_CLOSE_NOW, 0); else app_activate(ACT_OPEN_NOW, 0); break;
                case SDLK_r: app_activate(ACT_REFRESH, 0); break;
                default: break;
            }
            break;
        default:
            break;
    }
}

/* -------------------------------------------------------------- draw frame */

static void draw_toasts(void) {
    if (g_app.toast_count == 0) return;
    int y = 24;
    for (int i = g_app.toast_count - 1; i >= 0; i--) {
        Toast *t = &g_app.toasts[i];
        double age = g_app.time - t->t;
        if (age > 4.2) continue;
        int alpha = 255;
        if (age > 3.6) alpha = (int)(255 * (1.0 - (age - 3.6) / 0.6));
        int w = imin(560, ui_text_w(F_SMALL, t->text) + 48);
        int x = APP_W - w - 28;
        Col bg = t->kind ? ui_rgb(64, 26, 38) : C_CARD2;
        ui_round(x, y, w, 44, 14, ui_alpha(bg, imin(235, alpha)));
        ui_round(x, y, 4, 44, 2, ui_alpha(t->kind ? ui_rgb(255, 98, 98) : C_ACCENT, alpha));
        ui_text(F_SMALL, x + 18, y + 13, ui_alpha(C_TEXT, alpha), t->text);
        y += 52;
    }
}

static void draw_focus_ring(void) {
    if (!g_app.focus_visible || g_app.hit_count <= 0) return;
    if (g_app.focus < 0 || g_app.focus >= g_app.hit_count) return;
    SDL_Rect r = g_app.hits[g_app.focus].rect;
    Col c = g_app.theme_ready ? g_app.theme_accent : C_ACCENT;
    int radius = 12;
    ui_rect(r.x + radius, r.y - 2, r.w - 2 * radius, 3, ui_alpha(c, 235));
    ui_rect(r.x + radius, r.y + r.h - 1, r.w - 2 * radius, 3, ui_alpha(c, 235));
    ui_rect(r.x - 2, r.y + radius, 3, r.h - 2 * radius, ui_alpha(c, 235));
    ui_rect(r.x + r.w - 1, r.y + radius, 3, r.h - 2 * radius, ui_alpha(c, 235));
    for (int corner = 0; corner < 4; corner++) {
        int ccx = (corner & 1) ? r.x + r.w - radius : r.x + radius;
        int ccy = (corner & 2) ? r.y + r.h - radius : r.y + radius;
        for (int i = 0; i <= 6; i++) {
            float a = 0;
            if (corner == 0) a = 3.14159f + (float)i * 1.5708f / 6.0f;
            else if (corner == 1) a = 4.71239f + (float)i * 1.5708f / 6.0f;
            else if (corner == 3) a = 1.5708f + (float)i * 1.5708f / 6.0f;
            else a = 0 + (float)i * 1.5708f / 6.0f;
            ui_circle(ccx + (int)(cosf(a) * radius), ccy + (int)(sinf(a) * radius), 2, ui_alpha(c, 235));
        }
    }
}

static void draw_frame(void) {
    ui_begin_frame();
    g_app.hit_count = 0;
    g_app.view_top = 0;
    g_app.view_bottom = APP_H;

    ui_rect(0, 0, APP_W, APP_H, C_BG);

    if (g_app.screen == SCR_LANG) {
        pages_draw_language();
    } else if (g_app.screen == SCR_SD_READY) {
        pages_draw_sd_ready();
    } else if (g_app.screen == SCR_RELAUNCH) {
        pages_draw_relaunch();
    } else {
        pages_draw_sidebar();
        pages_draw_header();

        int bottom = app_has_track() ? APP_H - MINI_H : APP_H;
        g_app.content_top = HEADER_H;
        g_app.content_bottom = bottom;
        g_app.view_top = HEADER_H;
        g_app.view_bottom = bottom;

        if (g_app.page == PAGE_PLAYLIST) pages_draw_playlist(SIDEBAR_W, HEADER_H, APP_W - SIDEBAR_W, bottom - HEADER_H);
        else {
            ui_push_clip(SIDEBAR_W, HEADER_H, APP_W - SIDEBAR_W, bottom - HEADER_H);
            g_app.view_top = HEADER_H;
            g_app.view_bottom = bottom;
            int cx = SIDEBAR_W + 36;
            int cw = APP_W - SIDEBAR_W - 72;
            int ch = bottom - HEADER_H;
            if (g_app.page == PAGE_HOME) pages_draw_home(cx, HEADER_H + 14, cw, ch - 14);
            else if (g_app.page == PAGE_PLAYLISTS) pages_draw_playlists(cx, HEADER_H + 14, cw, ch - 14);
            else if (g_app.page == PAGE_LIBRARY) pages_draw_library(cx, HEADER_H + 14, cw, ch - 14);
            else pages_draw_settings(cx, HEADER_H + 14, cw, ch - 14);
            ui_pop_clip();
        }

        if (app_has_track()) pages_draw_mini();
        if (g_app.now_open) {
            g_app.hit_count = 0;
            g_app.view_top = 0;
            g_app.view_bottom = APP_H;
            now_draw();
        }
    }

    draw_focus_ring();
    draw_toasts();
}

/* --------------------------------------------------------------- lifecycle */

static void init_pad(void) {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            g_pad = SDL_GameControllerOpen(i);
            if (g_pad) { g_pad_index = i; break; }
        }
    }
}

int app_run(void) {
    memset(&g_app, 0, sizeof(g_app));
    g_app.page = PAGE_HOME;
    g_app.playlist_index = 0;
    g_app.scanning = 1;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    g_window = SDL_CreateWindow("Musictt", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                APP_W, APP_H, SDL_WINDOW_SHOWN);
    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    if (!g_renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(g_renderer, APP_W, APP_H);
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);

    if (ui_init(g_renderer) != 0) {
        fprintf(stderr, "ui_init failed\n");
        SDL_Quit();
        return 1;
    }

    int loaded = settings_load(&g_app.cfg);
    if (loaded != 0 || g_app.cfg.language < 0) {
        settings_defaults(&g_app.cfg);
        g_app.screen = SCR_LANG;
        g_app.lang_sel = g_app.cfg.language >= 0 ? g_app.cfg.language : 0;
        g_lang = (Lang)g_app.lang_sel;
    } else {
        g_lang = (Lang)iclamp(g_app.cfg.language, 0, LANG_COUNT - 1);
        g_app.lang_sel = g_app.cfg.language;
        g_app.screen = SCR_MAIN;
        char err[192] = {0};
        if (prepare_dirs(err, sizeof(err)) == 0) {
            if (library_scan(&g_app.lib) != 0) app_toast(i18n_get(S_ERR_SD), 1);
            else if (g_app.lib.errors > 0 && g_app.lib.last_error[0]) {
                app_toast(i18n_str(S_TOAST_PLAYLIST_ERROR, g_app.lib.last_error), 1);
            }
        } else {
            app_toast(i18n_get(S_ERR_SD), 1);
        }
    }
    ui_reload_fonts((Lang)g_app.lang_sel);

    g_app.audio_ok = player_init() == 0;
    if (!g_app.audio_ok) app_toast(i18n_get(S_ERR_AUDIO), 1);
    player_set_volume(g_app.cfg.volume);
    init_pad();

    uint32_t last = SDL_GetTicks();
    while (!g_app.quit) {
        uint32_t now = SDL_GetTicks();
        double dt = (double)(now - last) / 1000.0;
        if (dt > 0.1) dt = 0.1;
        last = now;
        g_app.time += dt;

        SDL_Event e;
        while (SDL_PollEvent(&e)) handle_event(&e);

        player_poll();
        app_sync_current();

        /* Smooth scrolling and animations. */
        for (int i = 0; i < PAGE_COUNT; i++) {
            float k = g_app.cfg.animations ? 0.22f : 1.0f;
            g_app.scroll[i] += (g_app.scroll_target[i] - g_app.scroll[i]) * k;
            if (fabsf(g_app.scroll_target[i] - g_app.scroll[i]) < 0.5f) g_app.scroll[i] = g_app.scroll_target[i];
        }
        g_app.page_anim += dt * 5.0;
        if (g_app.page_anim > 1.0) g_app.page_anim = 1.0;
        if (g_app.now_open && g_app.now_anim < 1.0) {
            g_app.now_anim += dt * 5.5;
            if (g_app.now_anim > 1.0) g_app.now_anim = 1.0;
        }
        g_app.focus_t += dt;
        if (g_app.touch_down && g_app.dragging_scroll) {
            /* keep focus hidden while dragging */
            g_app.focus_visible = 0;
        }
        if (g_app.needs_save && g_app.time - g_app.last_save > 1.2) app_save();
        if (g_app.toast_count > 0) {
            while (g_app.toast_count > 0 && g_app.time - g_app.toasts[0].t > 4.3) {
                for (int i = 1; i < g_app.toast_count; i++) g_app.toasts[i - 1] = g_app.toasts[i];
                g_app.toast_count--;
            }
        }

        now_update(dt);
        draw_frame();
        SDL_RenderPresent(g_renderer);
    }

    app_save();
    player_shutdown();
    library_free(&g_app.lib);
    ui_shutdown();
    if (g_pad) SDL_GameControllerClose(g_pad);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    return 0;
}
