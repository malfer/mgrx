/**
 ** libwyl.h - GRX library Wayland private include file
 **
 ** Copyright (C) 2024 Mariano Alvarez Fernandez
 ** [e-mail: malfer at telefonica.net]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#ifndef _LIBWYL_H_
#define _LIBWYL_H_

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include "xdg-shell-client-protocol.h"

enum pointer_event_mask {
    POINTER_EVENT_ENTER = 1 << 0,
    POINTER_EVENT_LEAVE = 1 << 1,
    POINTER_EVENT_MOTION = 1 << 2,
    POINTER_EVENT_BUTTON = 1 << 3,
    POINTER_EVENT_AXIS = 1 << 4,
    POINTER_EVENT_AXIS_SOURCE = 1 << 5,
    POINTER_EVENT_AXIS_STOP = 1 << 6,
    POINTER_EVENT_AXIS_DISCRETE = 1 << 7,
};

struct pointer_event {
    uint32_t event_mask;
    wl_fixed_t surface_x, surface_y;
    uint32_t button, state;
    uint32_t time;
    uint32_t serial;
    struct {
        int valid;
        wl_fixed_t value;
        int32_t discrete;
    } axes[2];
    uint32_t axis_source;
};

#define MGRX_MAX_OUTPUTS 5 // Max outputs we can handle

struct wgr_client_state {
    /* Globals */
    struct wl_display *wl_display;
    struct wl_registry *wl_registry;
    struct wl_shm *wl_shm;
    struct wl_compositor *wl_compositor;
    struct xdg_wm_base *xdg_wm_base;
    struct wl_seat *wl_seat;
    int num_outputs;
    struct wl_output *wl_output[MGRX_MAX_OUTPUTS];
   /* Objects */
    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct wl_keyboard *wl_keyboard;
    struct wl_pointer *wl_pointer;
    /* State */
    uint32_t last_frame;
    struct pointer_event pointer_event;
    struct xkb_context *xkb_context;
    struct xkb_keymap *xkb_keymap;
    struct xkb_state *xkb_state;
};

extern struct wgr_client_state _WGrState;
extern unsigned int _WGrMaxWidth;
extern unsigned int _WGrMaxHeight0;
extern unsigned int _WGrCurWidth;
extern unsigned int _WGrCurHeight;
extern unsigned int _WGrCurOffset;
extern unsigned int _WGrCurSize;
extern struct wl_shm_pool *_WGrPool;
extern char *_WGrFrame;
extern char *_WGrFrameDB;
extern int _WGrFd;

extern int _WGrEventInited;
extern int _WGrGenWMEndEvents;
extern int _WGrGenFrameEvents;
extern int _WGrGenWSzChgEvents;
extern int _WGrUserHadSetWName;

extern const struct wl_pointer_listener _Gr_wyl_pointer_listener;
extern const struct wl_keyboard_listener _Gr_wyl_keyboard_listener;

#endif
