/**
 ** grev_wyl.c ---- MGRX events, Wayland input
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "libgrx.h"
#include "mgrxkeys.h"
#include "ninput.h"
#include "arith.h"
#include "libwyl.h"
#include <linux/input-event-codes.h>
#include "wylkeys.h"

int _WGrEventInited = 0;

static int kbsysencoding = -1;
static int numgrxeventread = 0;
static int kbd_lastmod = 0;

static void _WGrEnqueuKeyEvent(uint32_t key, char *buf);
static int _WGrKeyCode(unsigned int keywyl, unsigned int state, long *p1, long *p2);

/**
 ** _GrEventInit - Initializes inputs
 **
 ** Returns 1 on success
 **         0 on error
 **
 ** For internal use only
 **/

int _GrEventInit(void)
{
    int ret = 0;

    if (_WGrState.wl_display) {
        _WGrEventInited = 1;
        ret = 1;
    }
    kbsysencoding = GRENC_UTF_8;
    memset(&(_WGrState.pointer_event), 0, sizeof(struct pointer_event));
    return ret;
}

/**
 ** _GrEventUnInit - UnInitializes inputs
 **
 ** For internal use only
 **/

void _GrEventUnInit(void)
{
    MOUINFO->msstatus = 0;
    _WGrEventInited = 0;
}

/**
 ** _GrGetKbSysEncoding - Get kb system encoding
 **
 **/

int _GrGetKbSysEncoding(void)
{
    return kbsysencoding;
}

/**
 ** _GrReadInputs - Reads inputs and sets events
 **
 ** For internal use only
 **/

int _GrReadInputs(void)
{
    int nevents;

    wl_display_roundtrip(_WGrState.wl_display);
    //wl_display_dispatch(_WGrState.wl_display);

    nevents = numgrxeventread;
//    if (nevents == 0) usleep(1000L);   // wait 1 ms to not eat 100% cpu
//    else fprintf(stderr, "En readinputs %d\n", nevents);
    numgrxeventread = 0;
    return nevents;
}

/**
 ** _GrMouseDetect - Returns true if a mouse is detected
 **
 ** For internal use only
 **/

int _GrMouseDetect(void)
{
    if (_WGrState.wl_pointer != NULL) {
        MOUINFO->msstatus = 1;
        MOUINFO->bstatus = 0;
        return 1;
    }
    return 0;
}

/**
 ** GrMouseSetSpeed
 **
 **/

void GrMouseSetSpeed(int spmult, int spdiv)
{
    MOUINFO->spmult = umin(16, umax(1, spmult));
    MOUINFO->spdiv  = umin(16, umax(1, spdiv));
}

/**
 ** GrMouseSetAccel
 **
 **/

void GrMouseSetAccel(int thresh, int accel)
{
    MOUINFO->thresh = umin(64, umax(1, thresh));
    MOUINFO->accel  = umin(16, umax(1, accel));
}

/**
 ** GrMouseSetLimits
 **
 **/

void GrMouseSetLimits(int x1, int y1, int x2, int y2)
{
    isort(x1, x2);
    isort(y1, y2);
    MOUINFO->xmin = imax(0, imin(x1, SCRN->gc_xmax));
    MOUINFO->ymin = imax(0, imin(y1, SCRN->gc_ymax));
    MOUINFO->xmax = imax(0, imin(x2, SCRN->gc_xmax));
    MOUINFO->ymax = imax(0, imin(y2, SCRN->gc_ymax));
}

/**
 ** GrMouseWarp
 **
 **/

void GrMouseWarp(int x, int y)
{
    MOUINFO->xpos = imax(MOUINFO->xmin, imin(MOUINFO->xmax, x));
    MOUINFO->ypos = imax(MOUINFO->ymin, imin(MOUINFO->ymax, y));
    GrMouseUpdateCursor();
}

/* Wayland pointer stuff */

static void
wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
                uint32_t serial, struct wl_surface *surface,
                wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    struct wgr_client_state *client_state = data;
    client_state->pointer_event.event_mask |= POINTER_EVENT_ENTER;
    client_state->pointer_event.serial = serial;
    client_state->pointer_event.surface_x = surface_x;
    client_state->pointer_event.surface_y = surface_y;
}

static void
wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                uint32_t serial, struct wl_surface *surface)
{
    struct wgr_client_state *client_state = data;
    client_state->pointer_event.serial = serial;
    client_state->pointer_event.event_mask |= POINTER_EVENT_LEAVE;
}

static void
wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time,
                wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    struct wgr_client_state *client_state = data;
    client_state->pointer_event.event_mask |= POINTER_EVENT_MOTION;
    client_state->pointer_event.time = time;
    client_state->pointer_event.surface_x = surface_x;
    client_state->pointer_event.surface_y = surface_y;
}

static void
wl_pointer_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial,
                uint32_t time, uint32_t button, uint32_t state)
{
    struct wgr_client_state *client_state = data;
    client_state->pointer_event.event_mask |= POINTER_EVENT_BUTTON;
    client_state->pointer_event.time = time;
    client_state->pointer_event.serial = serial;
    client_state->pointer_event.button = button;
    client_state->pointer_event.state = state;
}

static void
wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time,
                uint32_t axis, wl_fixed_t value)
{
    if (axis > 1) return;
    struct wgr_client_state *client_state = data;
    client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS;
    client_state->pointer_event.time = time;
    client_state->pointer_event.axes[axis].valid = TRUE;
    client_state->pointer_event.axes[axis].value = value;
}

static void
wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                    uint32_t axis_source)
{
    struct wgr_client_state *client_state = data;
    client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_SOURCE;
    client_state->pointer_event.axis_source = axis_source;
}

static void
wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                    uint32_t time, uint32_t axis)
{
    if (axis > 1) return;
    struct wgr_client_state *client_state = data;
    client_state->pointer_event.time = time;
    client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_STOP;
    client_state->pointer_event.axes[axis].valid = TRUE;
}

static void
wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
                        uint32_t axis, int32_t discrete)
{
    if (axis > 1) return;
    struct wgr_client_state *client_state = data;
    client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_DISCRETE;
    client_state->pointer_event.axes[axis].valid = TRUE;
    client_state->pointer_event.axes[axis].discrete = discrete;
}

static void
wl_pointer_frame(void *data, struct wl_pointer *wl_pointer)
{
    GrEvent evaux;
    int x, y;
    struct wgr_client_state *client_state = data;
    struct pointer_event *event = &client_state->pointer_event;

    if (event->event_mask & POINTER_EVENT_ENTER) {
        // Define an invisible cursor
        wl_pointer_set_cursor(wl_pointer, event->serial, NULL, 0, 0);
    }

    if (event->event_mask & POINTER_EVENT_LEAVE) {
    }

    if (event->event_mask & POINTER_EVENT_MOTION) {
        x = wl_fixed_to_int(event->surface_x);
        y = wl_fixed_to_int(event->surface_y);
        MOUINFO->xpos = x;
        MOUINFO->ypos = y;
        GrMouseUpdateCursor();
        MOUINFO->moved  = TRUE;
        if ((MOUINFO->genmmove == GR_GEN_MMOVE_ALWAYS) ||
            ((MOUINFO->genmmove == GR_GEN_MMOVE_IFBUT) &&
             (MOUINFO->bstatus != 0))) {
            evaux.type = GREV_MMOVE;
            evaux.kbstat = kbd_lastmod;
            evaux.p1 = MOUINFO->bstatus;
            evaux.p2 = MOUINFO->xpos;
            evaux.p3 = MOUINFO->ypos;
            evaux.p4 = 0;
            GrEventEnqueue(&evaux);
            MOUINFO->moved = FALSE;
            numgrxeventread++;
        }
    }

    if (event->event_mask & POINTER_EVENT_BUTTON) {
        int sendgrevent = 0;
        evaux.type = GREV_MOUSE;
        evaux.kbstat = kbd_lastmod;
        evaux.p2 = MOUINFO->xpos;
        evaux.p3 = MOUINFO->ypos;
        evaux.p4 = 0;
        if (event->state == WL_POINTER_BUTTON_STATE_PRESSED) {
            switch (event->button) {
                case BTN_LEFT:   evaux.p1 = GRMOUSE_LB_PRESSED;
                                 MOUINFO->bstatus |= GRMOUSE_LB_STATUS;
                                 sendgrevent = 1;
                                 break;
                case BTN_MIDDLE: evaux.p1 = GRMOUSE_MB_PRESSED;
                                 sendgrevent = 1;
                                 MOUINFO->bstatus |= GRMOUSE_MB_STATUS;
                                 break;
                case BTN_RIGHT:  evaux.p1 = GRMOUSE_RB_PRESSED;
                                 sendgrevent = 1;
                                 MOUINFO->bstatus |= GRMOUSE_RB_STATUS;
                                 break;
                case BTN_SIDE:   evaux.p1 = GRMOUSE_B8_PRESSED;
                                 sendgrevent = 1;
                                 break;
                case BTN_EXTRA:  evaux.p1 = GRMOUSE_B9_PRESSED;
                                 sendgrevent = 1;
                                 break;
            }
        } else if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
            switch (event->button) {
                case BTN_LEFT:   evaux.p1 = GRMOUSE_LB_RELEASED;
                                 if (MOUINFO->bstatus & GRMOUSE_LB_STATUS) {
                                    MOUINFO->bstatus &= ~GRMOUSE_LB_STATUS;
                                    sendgrevent = 1;
                                 }
                                 break;
                case BTN_MIDDLE: evaux.p1 = GRMOUSE_MB_RELEASED;
                                 if (MOUINFO->bstatus & GRMOUSE_MB_STATUS) {
                                    MOUINFO->bstatus &= ~GRMOUSE_MB_STATUS;
                                    sendgrevent = 1;
                                 }
                                 break;
                case BTN_RIGHT:  evaux.p1 = GRMOUSE_RB_RELEASED;
                                 if (MOUINFO->bstatus & GRMOUSE_RB_STATUS) {
                                    MOUINFO->bstatus &= ~GRMOUSE_RB_STATUS;
                                    sendgrevent = 1;
                                 }
                                 break;
                case BTN_SIDE:   evaux.p1 = GRMOUSE_B8_RELEASED;
                                 sendgrevent = 1;
                                 break;
                case BTN_EXTRA:  evaux.p1 = GRMOUSE_B9_RELEASED;
                                 sendgrevent = 1;
                                 break;
            }

        }
        if (sendgrevent) {
            GrEventEnqueue(&evaux);
            MOUINFO->moved = FALSE;
            numgrxeventread++;
        }
    }

    uint32_t axis_events = POINTER_EVENT_AXIS
        | POINTER_EVENT_AXIS_SOURCE
        | POINTER_EVENT_AXIS_STOP
        | POINTER_EVENT_AXIS_DISCRETE;

    int whellbutton = 0;
    if ((event->event_mask & axis_events) &&
        (event->axis_source == 0)) { // wheel
        if (event->axes[0].valid) { // vertical
            if (event->axes[0].discrete > 0) whellbutton = 5;
            else if (event->axes[0].discrete < 0) whellbutton = 4;
        }
        else if (event->axes[1].valid) { // horizontal
            if (event->axes[1].discrete > 0) whellbutton = 7;
            else if (event->axes[1].discrete < 0) whellbutton = 6;
        }
    }
    if (whellbutton) {
        evaux.type = GREV_MOUSE;
        evaux.kbstat = kbd_lastmod;
        evaux.p2 = MOUINFO->xpos;
        evaux.p3 = MOUINFO->ypos;
        evaux.p4 = 0;
        switch (whellbutton) {
            case 4 :evaux.p1 = GRMOUSE_B4_PRESSED; break;
            case 5 :evaux.p1 = GRMOUSE_B5_PRESSED; break;
            case 6 :evaux.p1 = GRMOUSE_B6_PRESSED; break;
            case 7 :evaux.p1 = GRMOUSE_B7_PRESSED; break;
        }
        GrEventEnqueue(&evaux);
        switch (whellbutton) {
            case 4 :evaux.p1 = GRMOUSE_B4_RELEASED; break;
            case 5 :evaux.p1 = GRMOUSE_B5_RELEASED; break;
            case 6 :evaux.p1 = GRMOUSE_B6_RELEASED; break;
            case 7 :evaux.p1 = GRMOUSE_B7_RELEASED; break;
        }
        GrEventEnqueue(&evaux);
        MOUINFO->moved = FALSE;
        numgrxeventread += 2;
    }

   memset(event, 0, sizeof(*event));
}

const struct wl_pointer_listener _Gr_wyl_pointer_listener = {
    .enter = wl_pointer_enter,
    .leave = wl_pointer_leave,
    .motion = wl_pointer_motion,
    .button = wl_pointer_button,
    .axis = wl_pointer_axis,
    .frame = wl_pointer_frame,
    .axis_source = wl_pointer_axis_source,
    .axis_stop = wl_pointer_axis_stop,
    .axis_discrete = wl_pointer_axis_discrete,
};

/* Wayland keyboard stuff */

static void
wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
            uint32_t format, int32_t fd, uint32_t size)
{
    struct wgr_client_state *client_state = data;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        fprintf(stderr, "Disaster, format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1");
        exit(1);
    }

    char *map_shm = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_shm == MAP_FAILED) {
        fprintf(stderr, "Disaster, map_shm == MAP_FAILED");
        exit(1);
    }

    struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
                    client_state->xkb_context, map_shm,
                    XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map_shm, size);
    close(fd);

    struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
    xkb_keymap_unref(client_state->xkb_keymap);
    xkb_state_unref(client_state->xkb_state);
    client_state->xkb_keymap = xkb_keymap;
    client_state->xkb_state = xkb_state;
}

static void
wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
            uint32_t serial, struct wl_surface *surface,
            struct wl_array *keys)
{
    //fprintf(stderr, "keyboard enter\n");
}

static void
wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
            uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    struct wgr_client_state *client_state = data;
    char buf[128];
    uint32_t keycode = key + 8;

    //xkb_keysym_t sym = xkb_state_key_get_one_sym(
    //                client_state->xkb_state, keycode);

    //xkb_keysym_get_name(sym, buf, sizeof(buf));
    //char *action = state == WL_KEYBOARD_KEY_STATE_PRESSED ? "press" : "release";
    //fprintf(stderr, "key(%d) %s: sym: %-12s (%d), \n", keycode, action, buf, sym);
    xkb_state_key_get_utf8(client_state->xkb_state, keycode, buf, sizeof(buf));
    //fprintf(stderr, "utf8(%d): '%s'\n", (int)strlen(buf), buf);

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        //fprintf(stderr, "key %d\n", key);
        _WGrEnqueuKeyEvent(key, buf);
    }
}

static void
wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
            uint32_t serial, struct wl_surface *surface)
{
    //fprintf(stderr, "keyboard leave\n");
}

static void
wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
            uint32_t serial, uint32_t mods_depressed,
            uint32_t mods_latched, uint32_t mods_locked,
            uint32_t group)
{
    struct wgr_client_state *client_state = data;
    xkb_state_update_mask(client_state->xkb_state,
            mods_depressed, mods_latched, mods_locked, 0, 0, group);
    //fprintf(stderr, "keyboard modifiers %8x %8x %8x\n",
    //        mods_depressed, mods_latched, mods_locked);

    kbd_lastmod = 0;
    if (mods_depressed & 1) kbd_lastmod |= GRKBS_LEFTSHIFT;
    if (mods_depressed & 4) kbd_lastmod |= GRKBS_CTRL;
    if (mods_depressed & 8) kbd_lastmod |= GRKBS_ALT;
    if (mods_locked & 2) kbd_lastmod |= GRKBS_CAPSLOCK;
    //fprintf(stderr, "mgrx modifiers %8x\n", kbd_lastmod);
}

static void
wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
            int32_t rate, int32_t delay)
{
    //fprintf(stderr, "keyboard repeat_info\n");
}

const struct wl_keyboard_listener _Gr_wyl_keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .key = wl_keyboard_key,
    .modifiers = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info,
};

static void _WGrEnqueuKeyEvent(uint32_t key, char *buf)
{
    GrEvent evaux;
    int have_event = 0;
    int count;
   char *cp1;

    evaux.type = GREV_PREKEY;
    evaux.kbstat = kbd_lastmod;
    evaux.p3 = 0;
    evaux.p4 = 0;

    if (_WGrKeyCode(key, kbd_lastmod, &evaux.p1, &evaux.p2)) {
        have_event = 1;
    } else {
        count = strlen(buf);
        if ((count >= 1) && (count <= 4)) {
            evaux.p1 = 0;
            cp1 = (char *)&evaux.p1;
            for(int i=0; i < count; i++)
                cp1[i] = buf[i];
            evaux.p2 = count;
            have_event = 1;
        }
    }

    if (have_event) {
        GrEventEnqueue(&evaux);
        MOUINFO->moved = FALSE;
        numgrxeventread++;
    }
}

static int _WGrKeyCode(unsigned int keywyl, unsigned int state, long *p1, long *p2)
{
    KeyEntry *k, *kp, *lastkp;

    if (state & GRKBS_ALT) {
        lastkp = &_KTAlt[sizeof(_KTAlt)/sizeof(_KTAlt[0])];
        kp = _KTAlt;
    } else if (state & GRKBS_LEFTSHIFT) {
        lastkp = &_KTShift[sizeof(_KTShift)/sizeof(_KTShift[0])];
        kp = _KTShift;
    } else if (state & GRKBS_CTRL) {
        lastkp = &_KTControl[sizeof(_KTControl)/sizeof(_KTControl[0])];
        kp = _KTControl;
    } else {
        lastkp = &_KTVoid[sizeof(_KTVoid)/sizeof(_KTVoid[0])];
        kp = _KTVoid;
    }

     for (k = kp; k < lastkp; k = k + 1) {
        if (keywyl == k->keywyl) {
            *p1 = k->key;
            *p2 = GRKEY_KEYCODE;
            return 1;
        }
    }

    return 0;
}
