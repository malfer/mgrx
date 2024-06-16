/**
 ** clb_wyl.c ---- Wayland clipboard
 **
 ** Copyright (C) 2024 Mariano Alvarez Fernandez
 ** [e-mail: malfer at telefonica dot net]
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

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "libgrx.h"
#include "libwyl.h"

// the MGRX Wayland implementation uses UTF8 for the clipboard buffer

// prefered MIME types in prefered order
static const char *text_mime_types[] = {
    "text/plain;charset=utf-8",
    "UTF8_STRING",
    "text/plain",
    "TEXT",
    "STRING"
};
static int num_mimes = 5;

static struct wl_data_offer *last_offer = NULL;
static int is_last_offer_selection = 0;
static int prefered_mime_offered = -1;

struct wl_data_source *data_source = NULL;
static int we_own_the_selection = 0;

static int get_last_offer(void);

static unsigned char *_WGrClipboard = NULL; // the clipboard buffer
static int _WGrClipboardLen = 0;            // the clipboard len (in chars)
static int _WGrClipboardByteLen = 0;        // the clipboard len (in bytes)

static int clipboard_maxchars = 32000;
static int loaded_clipboard = 0;
static int partially_loaded = 0;

static void clear_cb(void)
{
    if (_WGrClipboard) free(_WGrClipboard);
    _WGrClipboard = NULL;
    _WGrClipboardLen = 0;
    _WGrClipboardByteLen = 0;
    partially_loaded = 0;
}

static void wl_data_source_target(void *data, struct wl_data_source *source,
                                  const char *mime_type)
{
    //fprintf(stderr, "wl_data_source: target\n");
}

static void wl_data_source_send(void *data, struct wl_data_source *source,
                                const char *mime_type, int32_t fd)
{
    if (_WGrClipboard) write(fd, _WGrClipboard, _WGrClipboardByteLen);

    close(fd);
}

static void wl_data_source_cancelled(void *data, struct wl_data_source *source)
{
    clear_cb();
    we_own_the_selection = 0;
    wl_data_source_destroy(source);
    data_source = NULL;
}

static const struct wl_data_source_listener wl_data_source_listener = {
    .target = wl_data_source_target,
    .send = wl_data_source_send,
    .cancelled = wl_data_source_cancelled,
};

static void wl_data_offer_offer(void *data, struct wl_data_offer *offer,
                                const char *type)
{
    for (int i=0; i<num_mimes; i++) {
        if (strcmp(type, text_mime_types[i]) == 0) {
            if (prefered_mime_offered < 0 || i < prefered_mime_offered) {
                prefered_mime_offered = i;
            }
        }
    }
}

static void wl_data_offer_source_actions(void *data, struct wl_data_offer *offer,
                                         uint32_t source_actions)
{
    //fprintf(stderr, "wl_data_offer: source_actions\n");
}

static void wl_data_offer_action(void *data, struct wl_data_offer *offer,
                                 uint32_t dnd_action)
{
    //fprintf(stderr, "wl_data_offer: action\n");
}

static const struct wl_data_offer_listener wl_data_offer_listener = {
    .offer = wl_data_offer_offer,
    .source_actions = wl_data_offer_source_actions,
    .action = wl_data_offer_action
};

static void wl_data_device_data_offer(void *data,
            struct wl_data_device *data_device, struct wl_data_offer *offer)
{
    if (last_offer) wl_data_offer_destroy(last_offer);
    last_offer = offer;
    is_last_offer_selection = 0;
    prefered_mime_offered = -1;
    wl_data_offer_add_listener(offer, &wl_data_offer_listener, NULL);
}

static void wl_data_device_enter(void *data, struct wl_data_device *data_device,
            uint32_t serial, struct wl_surface *surface,
            wl_fixed_t x_w, wl_fixed_t y_w, struct wl_data_offer *offer)
{
    //fprintf(stderr, "wl_data_device: enter\n");
}

static void wl_data_device_leave(void *data, struct wl_data_device *data_device)
{
    if (last_offer) {
        wl_data_offer_destroy(last_offer);
        last_offer = NULL;
        is_last_offer_selection = 0;
        prefered_mime_offered = -1;
    }
}

static void wl_data_device_motion(void *data, struct wl_data_device *data_device,
            uint32_t time, wl_fixed_t x_w, wl_fixed_t y_w)
{
    //fprintf(stderr, "wl_data_device: motion\n");
}

static void wl_data_device_drop(void *data, struct wl_data_device *data_device)
{
    if (last_offer) {
        wl_data_offer_destroy(last_offer);
        last_offer = NULL;
        is_last_offer_selection = 0;
        prefered_mime_offered = -1;
    }
}

static void data_device_selection(void *data,
            struct wl_data_device *wl_data_device, struct wl_data_offer *offer)
{
    if (offer == NULL) {
        if (last_offer) {
            wl_data_offer_destroy(last_offer);
            last_offer = NULL;
            is_last_offer_selection = 0;
            prefered_mime_offered = -1;
        }
    } else {
        if (we_own_the_selection) {
            wl_data_offer_destroy(offer);
            last_offer = NULL;
            is_last_offer_selection = 0;
            prefered_mime_offered = -1;
        } else {
            last_offer = offer;
            is_last_offer_selection = 1;
            loaded_clipboard = 0;
            we_own_the_selection = 0;
            clear_cb();
        }
    }
}

static const struct wl_data_device_listener wl_data_device_listener = {
    .data_offer = wl_data_device_data_offer,
    .enter = wl_data_device_enter,
    .leave = wl_data_device_leave,
    .motion = wl_data_device_motion,
    .drop = wl_data_device_drop,
    .selection = data_device_selection,
};

static int get_last_offer(void)
{
    int pipefd[2];
    unsigned long count = 0;
    unsigned long count2;

    clear_cb();

    if (last_offer == NULL) return 0;
    //if (pipe2(pipefd, O_CLOEXEC | O_NONBLOCK) == -1) return;
    if (pipe(pipefd) == -1) return 0;

    wl_data_offer_receive(last_offer, text_mime_types[prefered_mime_offered], pipefd[1]);
    close(pipefd[1]);

    //wl_display_flush(_WGrState.wl_display);
    wl_display_roundtrip(_WGrState.wl_display);

    // First read gets clipboard len
    while (1) {
        char buf[1024];
        ssize_t n = read(pipefd[0], buf, sizeof(buf));
        if (n <= 0) {
            break;
        }
        count +=n;
    }
    close(pipefd[0]);

    if (count == 0) return 0;

    if ((clipboard_maxchars != GR_CB_NOLIMIT) && (count > clipboard_maxchars)) {
        count = clipboard_maxchars;
        partially_loaded = 1;
    }

    _WGrClipboard = malloc(count+1);
    if (!_WGrClipboard) return 0;

    // Second read gets the clipboard contents
    //if (pipe2(pipefd, O_CLOEXEC | O_NONBLOCK) == -1) return;
    if (pipe(pipefd) == -1) return 0;

    wl_data_offer_receive(last_offer, text_mime_types[prefered_mime_offered], pipefd[1]);
    close(pipefd[1]);

    //wl_display_flush(_WGrState.wl_display);
    wl_display_roundtrip(_WGrState.wl_display);

    count2 = read(pipefd[0], _WGrClipboard, count);
    close(pipefd[0]);
    if (count2 != count) return 0;
    _WGrClipboard[count] = '\0';
    _WGrClipboardLen = GrUTF8StrLen(_WGrClipboard);
    _WGrClipboardByteLen = count;
    loaded_clipboard = 1;
    return count;
}

void _WGrIniClipBoard(void)
{
    // this function is called by the wayland video driver only
    _WGrState.wl_data_device = wl_data_device_manager_get_data_device(
                                _WGrState.wl_data_device_manager, _WGrState.wl_seat);
    wl_data_device_add_listener(_WGrState.wl_data_device, &wl_data_device_listener,
                                &_WGrState);
}

void GrSetClipBoardMaxChars(int maxchars)
{
    if (maxchars <= 0) maxchars = GR_CB_NOLIMIT;
    clipboard_maxchars = maxchars;
}

void GrClearClipBoard(void)
{
    if (we_own_the_selection) {
        we_own_the_selection = 0;
        wl_data_source_destroy(data_source);
        data_source = NULL;
    }

    clear_cb();
    loaded_clipboard = 0;
}

int GrPutClipBoard(void *buf, int len, int chrtype)
{
    clear_cb();
    if (data_source) {
        we_own_the_selection = 0;
        wl_data_source_destroy(data_source);
        data_source = NULL;
    }

    if ((clipboard_maxchars != GR_CB_NOLIMIT) &&
        (len > clipboard_maxchars)) {
        len = clipboard_maxchars;
        partially_loaded = 1;
    }

    _WGrClipboard = GrTextRecodeToUTF8(buf, len, chrtype);
    if (!_WGrClipboard) return 0;

    data_source = wl_data_device_manager_create_data_source(_WGrState.wl_data_device_manager);
    wl_data_source_add_listener(data_source, &wl_data_source_listener, NULL);
    wl_data_source_offer(data_source, text_mime_types[0]);
    wl_data_device_set_selection(_WGrState.wl_data_device, data_source, _WGrLastKbEnterSerial);

    _WGrClipboardLen = len;
    _WGrClipboardByteLen = strlen((char *)_WGrClipboard);
    we_own_the_selection = 1;

    return len;
}

int GrIsClipBoardReadyToGet(void)
{
    if (we_own_the_selection || loaded_clipboard) {
        if (_WGrClipboardLen > 0)
            return 1; // ready
        else
            return 0; // no data
    }

    if (get_last_offer()) return 1;
    return 0;
}

void *GrGetClipBoard(int maxlen, int chrtype)
{
    int len;

    if (!_WGrClipboard) return NULL;

    len = (maxlen > _WGrClipboardLen) ? _WGrClipboardLen : maxlen;

    return GrTextRecodeFromUTF8(_WGrClipboard, len, chrtype);
}

int GrGetClipBoardLen(int *partloaded)
{
    if (partloaded) *partloaded = partially_loaded;
    if (!_WGrClipboard) return 0;
    return _WGrClipboardLen;
}
