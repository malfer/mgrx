/**
 ** vd_wylnd.c ---- Linux Wayland driver
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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "libwyl.h"

struct wgr_client_state _WGrState = { 0 };
unsigned int _WGrMaxWidth = 640;
unsigned int _WGrMaxHeight = 480;
unsigned int _WGrCurWidth;
unsigned int _WGrCurHeight;
unsigned int _WGrCurOffset;
unsigned int _WGrCurSize;
struct wl_shm_pool *_WGrPool = NULL;
char *_WGrFrame = NULL;
//char *_WGrFrameDB = NULL;
int _WGrFd = -1;

int _WGrGenWMEndEvents = GR_GEN_NO;
int _WGrGenFrameEvents = GR_GEN_NO;
int _WGrGenWSzChgEvents = FALSE;
int _WGrUserHadSetWName = 0;

static int _WGRNoMoreFrames = 0;
static int _WGRUnmapFrame = 0;
static int _WGRLastFrame = 0;

static void setbank(int bk);
static void setrwbanks(int rb, int wb);
static int setmode(GrVideoMode * mp, int noclear);
static int settext(GrVideoMode * mp, int noclear);

static GrVideoModeExt grtwylext = {
    GR_frameText,               /* frame driver */
    NULL,                       /* frame driver override */
    NULL,                       /* frame buffer address */
    {6, 6, 6},                  /* color precisions */
    {0, 0, 0},                  /* color component bit positions */
    0,                          /* mode flag bits */
    settext,                    /* mode set */
    NULL,                       /* virtual size set */
    NULL,                       /* virtual scroll */
    NULL,                       /* bank set function */
    NULL,                       /* double bank set function */
    NULL,                       /* color loader */
};

static GrVideoModeExt grxwylext = {
    GR_frameNLFB32L,            /* frame driver */
    NULL,                       /* frame driver override */
    NULL,                       /* frame buffer address */
    { 8, 8, 8 },                /* color precisions */
    {16, 8, 0 },                /* color component bit positions */
    GR_VMODEF_LINEAR,           /* mode flag bits */
    setmode,                    /* mode set */
    NULL,                       /* virtual size set */
    NULL,                       /* virtual scroll */
    setbank,                    /* bank set function */
    setrwbanks,                 /* double bank set function */
    NULL                        /* color loader */
};

static void setbank(int bk)
{}

static void setrwbanks(int rb, int wb)
{}

static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext  */
    {  TRUE,  8,   80,   25,  0x00,    80, 1,    &grtwylext },
    { FALSE, 32,  320,  240,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32,  640,  480,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32,  800,  600,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 1024,  768,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 1280, 1024,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 1600, 1200,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 1440,  900,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 1680, 1050,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 1920, 1080,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 1920, 1200,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 2560, 1440,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 3840, 2160,  0x00,     0, 0,    &grxwylext },
    { FALSE, 32, 9999, 9999,  0x00,     0, 0,    &grxwylext }
};

/* Shared memory support code */
static void randname(char *buf)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long r = ts.tv_nsec;
    for (int i = 0; i < 6; ++i) {
        buf[i] = 'A'+(r&15)+(r&16)*2;
        r >>= 5;
    }
}

static int create_shm_file(void)
{
    int retries = 100;
    do {
        char name[] = "/wl_shm-XXXXXX";
        randname(name + sizeof(name) - 7);
        --retries;
        int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd >= 0) {
            shm_unlink(name);
            return fd;
        }
    } while (retries > 0 && errno == EEXIST);
    return -1;
}

static int allocate_shm_file(size_t size)
{
    int fd = create_shm_file();
    if (fd < 0)
        return -1;
    int ret;
    do {
        ret = ftruncate(fd, size);
    } while (ret < 0 && errno == EINTR);
    if (ret < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    /* Sent by the compositor when it's no longer using this buffer */
    wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

static const struct wl_callback_listener wl_surface_frame_listener;

static void wl_surface_frame_done(void *data, struct wl_callback *cb, uint32_t time)
{
    struct wgr_client_state *state = data;

    /* Destroy this callback */
    wl_callback_destroy(cb);

    if (_WGRNoMoreFrames) { // No more frames
        if (_WGRUnmapFrame) { // Unmap de surface
            wl_surface_attach(state->wl_surface, NULL, 0, 0);
            wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
            wl_surface_commit(state->wl_surface);
            //fprintf(stderr, "Unmaping\n");
        }
        _WGRLastFrame = 1;
        //fprintf(stderr, "No more frames\n");
        return;
    }

    //memcpy((void *)_WGrFrame, (void *)_WGrFrameDB, _WGrCurSize);
    if (_WGrGenFrameEvents && _WGrEventInited) {
        GrEventParEnqueue(GREV_FRAME, 0, 0, 0, 0);
    }

    /* Submit a frame for this event */
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(_WGrPool, 0,
            _WGrCurWidth, _WGrCurHeight, _WGrCurOffset, WL_SHM_FORMAT_XRGB8888);
    wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);

    wl_surface_attach(state->wl_surface, buffer, 0, 0);
    wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);

    /* Request another frame */
    cb = wl_surface_frame(state->wl_surface);
    wl_callback_add_listener(cb, &wl_surface_frame_listener, state);

    wl_surface_commit(state->wl_surface);

    state->last_frame = time;
}

static const struct wl_callback_listener wl_surface_frame_listener = {
    .done = wl_surface_frame_done,
};

static void resetmode(int unmapframe)
{
    if (_WGrFrame == NULL) return;

    //fprintf(stderr, "En reset mode\n");
    _WGRNoMoreFrames = 1;
    _WGRUnmapFrame = unmapframe;
    _WGRLastFrame = 0;

    while (!_WGRLastFrame) { // wait to process last frame
        wl_display_roundtrip(_WGrState.wl_display);
    }
    wl_display_dispatch_pending(_WGrState.wl_display);

    wl_shm_pool_destroy(_WGrPool);
    munmap(_WGrFrame, _WGrCurSize);
    close(_WGrFd);
    _WGrFrame = NULL;
    _WGrFd = -1;
}

static int setmode(GrVideoMode * mp, int noclear)
{
    int size = mp->lineoffset * mp->height;
    char name[100];

    resetmode(0);
    _WGrFd = allocate_shm_file(size);
    if (_WGrFd == -1) return FALSE;

    //fprintf(stderr, "En setmode\n");

    if (!_WGrUserHadSetWName) {
        sprintf (name, "mgrx %dx%dx%d", mp->width, mp->height, mp->bpp);
        xdg_toplevel_set_title(_WGrState.xdg_toplevel, name);
    }

    _WGrFrame = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, _WGrFd, 0);
    if (_WGrFrame == MAP_FAILED) {
        close(_WGrFd);
        _WGrFrame = NULL;
        return FALSE;
    }
    /*if (_WGrFrameDB == NULL) {
        _WGrFrameDB = malloc(_WGrMaxWidth*4*_WGrMaxHeight);
        if (_WGrFrameDB == NULL) {
            munmap(_WGrFrame, size);
            close(_WGrFd);
            _WGrFrame = NULL;
            return FALSE;
        }
    }*/

    _WGrPool = wl_shm_create_pool(_WGrState.wl_shm, _WGrFd, size);

    //mp->extinfo->frame = _WGrFrameDB;
    mp->extinfo->frame = _WGrFrame;
    _WGrCurWidth = mp->width;
    _WGrCurHeight = mp->height;
    _WGrCurOffset = mp->lineoffset;
    _WGrCurSize = size;

    if (!noclear) {
        memset(_WGrFrame, 0, size);
        //memset(_WGrFrameDB, 0, size);
    }

    struct wl_buffer *buffer = wl_shm_pool_create_buffer(_WGrPool, 0,
            _WGrCurWidth, _WGrCurHeight, _WGrCurOffset, WL_SHM_FORMAT_XRGB8888);
    wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);

    wl_surface_attach(_WGrState.wl_surface, buffer, 0, 0);
    wl_surface_damage_buffer(_WGrState.wl_surface, 0, 0, INT32_MAX, INT32_MAX);

    _WGRNoMoreFrames = 0;
    struct wl_callback *cb = wl_surface_frame(_WGrState.wl_surface);
    wl_callback_add_listener(cb, &wl_surface_frame_listener, &_WGrState);

    wl_surface_commit(_WGrState.wl_surface);

//    wl_display_roundtrip(_WGrState.wl_display);

    return TRUE;
}

static int settext(GrVideoMode * mp, int noclear)
{
    //fprintf(stderr, "En settext\n");
    resetmode(1);
    return TRUE;
}

static int detect(void)
{
    int res = FALSE;
    struct wl_display *display;

    GRX_ENTER();
    display = wl_display_connect(NULL);
    if (display != NULL) {
        res = TRUE;
        wl_display_disconnect(display);
    }
    GRX_RETURN(res);
}

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                                   int32_t width, int32_t height, struct wl_array *states)
{
    /*int maximized = 0;
    int fullscreen = 0;
    int resizing = 0;
    int activated = 0;

    int32_t *pos;
    wl_array_for_each(pos, states) {
        if (*pos == 1) maximized = 1;
        else if (*pos == 2) fullscreen = 1;
        else if (*pos == 3) resizing = 1;
        else if (*pos == 4) activated = 1;
    }*/

    if (_WGrGenWSzChgEvents && _WGrEventInited) {
        if (width != 0 && height != 0) {
            if (width < DRVINFO->minwgw) width = DRVINFO->minwgw;
            if (height < DRVINFO->minwgh) height = DRVINFO->minwgh;
            if (width != _WGrCurWidth || height != _WGrCurHeight) {
                GrEventParEnqueue(GREV_WSZCHG, 0, 0, width, height);
            }
        }
    }
    //fprintf(stderr, "toplevel configure %d %d m%d f%d r%d a%d\n",
    //        width, height, maximized, fullscreen, resizing, activated);
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
    if (_WGrGenWMEndEvents == GR_GEN_NO) {
        exit(1);
    } else if (_WGrEventInited) {
        GrEventParEnqueue(GREV_WMEND, 0, 0, 0, 0);
    }
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                                  uint32_t serial)
{
    struct wgr_client_state *state = data;

    xdg_surface_ack_configure(xdg_surface, serial);
    wl_surface_commit(state->wl_surface);
    //fprintf(stderr, "xdg_surface configure\n");
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
                                 uint32_t capabilities)
{
    struct wgr_client_state *state = data;

    bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

    if (have_keyboard && state->wl_keyboard == NULL) {
        state->wl_keyboard = wl_seat_get_keyboard(state->wl_seat);
        wl_keyboard_add_listener(state->wl_keyboard,
            &_Gr_wyl_keyboard_listener, state);
    } else if (!have_keyboard && state->wl_keyboard != NULL) {
        wl_keyboard_release(state->wl_keyboard);
        state->wl_keyboard = NULL;
    }

    bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

    if (have_pointer && state->wl_pointer == NULL) {
        state->wl_pointer = wl_seat_get_pointer(state->wl_seat);
        wl_pointer_add_listener(state->wl_pointer,
            &_Gr_wyl_pointer_listener, state);
    } else if (!have_pointer && state->wl_pointer != NULL) {
        wl_pointer_release(state->wl_pointer);
        state->wl_pointer = NULL;
    }
}

static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{
    //fprintf(stderr, "seat name: %s\n", name);
}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name,
};

static void wl_output_geometry(void *data, struct wl_output *wl_output,
                               int32_t x, int32_t y,
                               int32_t physical_width, int32_t physical_height,
                               int32_t subpixel,
                               const char *make, const char *model,
                               int32_t output_transform)
{
}

static void wl_output_mode(void *data, struct wl_output *wl_output, uint32_t flags,
                           int32_t width, int32_t height, int32_t refresh)
{
    //fprintf(stderr, "Output mode %d %d %d %d\n", flags, width, height, refresh);
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        if (width > _WGrMaxWidth) _WGrMaxWidth = width;
        if (height > _WGrMaxHeight) _WGrMaxHeight = height;
    }
}

static void wl_output_done(void *data, struct wl_output *wl_output)
{
}

static void wl_output_scale(void *data, struct wl_output *wl_output, int32_t scale)
{
}

static void wl_output_name(void *data, struct wl_output *wl_output, const char *name)
{
}
static void wl_output_description(void *data, struct wl_output *wl_output,
                                  const char *description)
{
}

static const struct wl_output_listener wl_output_listener = {
    .geometry = wl_output_geometry,
    .mode = wl_output_mode,
    .done = wl_output_done,
    .scale = wl_output_scale,
    .name = wl_output_name,
    .description = wl_output_description,
};

static void registry_global(void *data, struct wl_registry *wl_registry,
                            uint32_t name, const char *interface, uint32_t version)
{
    struct wgr_client_state *state = data;

    if (strcmp(interface, wl_shm_interface.name) == 0) {
        state->wl_shm = wl_registry_bind(
                wl_registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state->wl_compositor = wl_registry_bind(
                wl_registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        state->xdg_wm_base = wl_registry_bind(
                wl_registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_wm_base,
                &xdg_wm_base_listener, state);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        state->wl_seat = wl_registry_bind(
                wl_registry, name, &wl_seat_interface, 7);
        wl_seat_add_listener(state->wl_seat,
                &wl_seat_listener, state);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        if (state->num_outputs < MGRX_MAX_OUTPUTS) {
            state->wl_output[state->num_outputs] = wl_registry_bind(
                    wl_registry, name, &wl_output_interface, 4);
            wl_output_add_listener(state->wl_output[state->num_outputs],
                    &wl_output_listener, state);
            state->num_outputs++;
        }
    }
}

static void registry_global_remove(void *data, struct wl_registry *wl_registry,
                                   uint32_t name)
{
}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static int init(char *options)
{
    int res = FALSE;
    GrVideoMode *mp;

    GRX_ENTER();
    _WGrState.wl_display = wl_display_connect(NULL);
    if (_WGrState.wl_display == NULL) goto done;

    if (options) {
        char opt[101] = {0};
        char *token;
        strncpy(opt, options, 100);
        token = strtok(opt, ":");
        while (token != NULL) {
            if (strncmp ("rszwin", token, 6) == 0) _WGrGenWSzChgEvents = TRUE;
            token = strtok(NULL, ":");
        }
    }

    if (_WGrGenWSzChgEvents) {
        _GrVideoDriverWAYLAND.drvflags |= GR_DRIVERF_WINDOW_RESIZE;
    } else {
        _GrVideoDriverWAYLAND.drvflags &= ~GR_DRIVERF_WINDOW_RESIZE;
    }

    _WGrState.wl_registry = wl_display_get_registry(_WGrState.wl_display);
    _WGrState.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    wl_registry_add_listener(_WGrState.wl_registry, &wl_registry_listener, &_WGrState);
    /* The first roundtrip gets the list of advertised globals */
    wl_display_roundtrip(_WGrState.wl_display);
    /* In the second roundtrip we get the wl_seat::capabilities */
    wl_display_roundtrip(_WGrState.wl_display);

    _WGrState.wl_surface = wl_compositor_create_surface(_WGrState.wl_compositor);
    _WGrState.xdg_surface = xdg_wm_base_get_xdg_surface(_WGrState.xdg_wm_base,
                                                        _WGrState.wl_surface);
    xdg_surface_add_listener(_WGrState.xdg_surface, &xdg_surface_listener, &_WGrState);
    _WGrState.xdg_toplevel = xdg_surface_get_toplevel(_WGrState.xdg_surface);
    xdg_toplevel_add_listener(_WGrState.xdg_toplevel, &xdg_toplevel_listener, &_WGrState);
    wl_surface_commit(_WGrState.wl_surface);
    /* In the third roundtrip we get the xdg surface configure */
    wl_display_roundtrip(_WGrState.wl_display);
    //fprintf(stderr, "Init wyl\n");

    /* 32 bpp fixed size modes */
    for (mp = &modes[1]; mp < &modes[itemsof(modes)-1]; mp++) {
        mp->present = TRUE;
        mp->bpp = 32;
        mp->lineoffset = mp->width * 4;
        if (mp->width > _WGrMaxWidth) mp->present = FALSE;
        if (mp->height > _WGrMaxHeight) mp->present = FALSE;
    }
    /* and the variable 32 bpp size mode */
    mp->present = FALSE;
    mp->bpp = 32;
    mp->lineoffset = 0; // to be calclulated in _wyl_selectmode

    res = TRUE;
done:
    GRX_RETURN(res);
}

static void reset(void)
{
    GRX_ENTER();
    if (_WGrState.wl_display) {
        wl_display_disconnect(_WGrState.wl_display);
        xkb_keymap_unref(_WGrState.xkb_keymap);
        xkb_state_unref(_WGrState.xkb_state);
        xkb_context_unref(_WGrState.xkb_context);
        _WGrState.wl_display = NULL;
        // TODO release things really

        memset(&_WGrState, 0, sizeof(_WGrState));
        _WGrPool = NULL;
        _WGrFrame = NULL;
        _WGrFd = -1;
        //free(_WGrFrameDB);
        //_WGrFrameDB = NULL;
        _WGrGenWMEndEvents = GR_GEN_NO;
        _WGrGenFrameEvents = GR_GEN_NO;
        _WGrGenWSzChgEvents = FALSE;
        _WGrUserHadSetWName = 0;
        _WGRNoMoreFrames = 0;
        _WGRUnmapFrame = 0;
        _WGRLastFrame = 0;
        _WGrMaxWidth = 640;
        _WGrMaxHeight = 480;
    }
    GRX_LEAVE();
}

static GrVideoMode * _wyl_selectmode(GrVideoDriver * drv, int w, int h, int bpp,
                                     int txt, unsigned int * ep )
{
    GrVideoMode *mp, *res = NULL;
    GRX_ENTER();
    if (txt) {
        res = _gr_selectmode(drv, w, h, bpp, txt, ep);
        goto done;
    }
    for (mp = &modes[1]; mp < &modes[itemsof(modes)-1]; mp++) {
        if ( mp->present && mp->width == w && mp->height == h) {
            res = _gr_selectmode (drv,w,h,bpp,txt,ep);
            goto done;
        }
    }
    /* no predefined mode found. Create a new mode */
    mp->present = TRUE;
    mp->width = (w > _WGrMaxWidth) ? _WGrMaxWidth : w;
    mp->height = (h > _WGrMaxHeight) ? _WGrMaxHeight : h;
    mp->bpp = 32;
    mp->lineoffset = mp->width * 4;
    res = _gr_selectmode (drv,w,h,bpp,txt,ep);
done:
    GRX_RETURN(res);
}

static int genwmend(int gen)
{
    _WGrGenWMEndEvents = gen;
    return _WGrGenWMEndEvents;
}

static int genframe(int gen)
{
    _WGrGenFrameEvents = gen;
    return _WGrGenFrameEvents;
}

GrVideoDriver _GrVideoDriverWAYLAND = {
    "wayland",                          /* name */
    GR_WAYLAND,                         /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    detect,                             /* detection routine */
    init,                               /* initialization routine */
    reset,                              /* reset routine */
    _wyl_selectmode,                    /* special mode select routine */
    GR_DRIVERF_USER_RESOLUTION |        /* arbitrary resolution possible */
    GR_DRIVERF_GEN_GREV_FRAME,          /* generates GREV_FRAME events */
    0,                                  /* inputdriver, not used by now */
    NULL,                               /* generate GREV_EXPOSE events */
    genwmend,                           /* generate GREV_WMEND events */
    genframe                            /* generate GREV_FRAME events */
};
