/**
 ** clb_x11.c ---- X11 clipboard
 **
 ** Copyright (C) 2022 Mariano Alvarez Fernandez
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libgrx.h"
#include "libxwin.h"
#include <X11/Xatom.h>

// the MGRX X11 implementation uses UTF8 for the clipboard buffer

Atom _XGrCBTargets;
Atom _XGrCBText;
Atom _XGrCBUTF8;
Atom _XGrCBSelection;
Atom _XGrCBIncr;
Atom _XGrCBProperty;
int  _XGrCBOwnSelection = 0;         // 1 if we own the selection
unsigned char *_XGrClipboard = NULL; // the clipboard buffer
int _XGrClipboardLen = 0;            // the clipboard len (in chars)
int _XGrClipboardByteLen = 0;        // the clipboard len (in bytes)

static int clipboard_maxchars = 32000;
static int waiting_selection_notify = 0;
static int loaded_clipboard = 0;
static int partially_loaded = 0;

static void clear_cb(void)
{
    if (_XGrClipboard) free(_XGrClipboard);
    _XGrClipboard = NULL;
    _XGrClipboardLen = 0;
    _XGrClipboardByteLen = 0;
    partially_loaded = 0;
}

void GrSetClipBoardMaxChars(int maxchars)
{
    if (maxchars <= 0) maxchars = GR_CB_NOLIMIT;
    clipboard_maxchars = maxchars;
}

void _XGrIniClipBoard(void)
{
    // this function is called by the x11 video driver only
    _XGrCBSelection = XInternAtom(_XGrDisplay, "CLIPBOARD", False);
    _XGrCBTargets = XInternAtom(_XGrDisplay, "TARGETS", False);
    _XGrCBText = XInternAtom(_XGrDisplay, "TEXT", False);
    _XGrCBUTF8 = XInternAtom(_XGrDisplay, "UTF8_STRING", False);
    _XGrCBIncr = XInternAtom(_XGrDisplay, "INCR", False);
    _XGrCBProperty = XInternAtom(_XGrDisplay, "MGRX_CB", False);
}

void GrClearClipBoard(void)
{
    if (_XGrCBOwnSelection) {
        XSetSelectionOwner(_XGrDisplay, _XGrCBSelection, None, CurrentTime);
        _XGrCBOwnSelection = 0;
    }

    clear_cb();
    loaded_clipboard = 0;
}

int GrPutClipBoard(void *buf, int len, int chrtype)
{
    clear_cb();

    if ((clipboard_maxchars != GR_CB_NOLIMIT) &&
        (len > clipboard_maxchars)) {
        len = clipboard_maxchars;
        partially_loaded = 1;
    }

    _XGrClipboard = GrTextRecodeToUTF8(buf, len, chrtype);
    if (!_XGrClipboard) return 0;

    XSetSelectionOwner(_XGrDisplay, _XGrCBSelection, _XGrWindow, CurrentTime);

    _XGrClipboardLen = len;
    _XGrClipboardByteLen = strlen((char *)_XGrClipboard);
    _XGrCBOwnSelection = 1;

    return len;
}

int GrIsClipBoardReadyToGet(void)
{
    if (_XGrCBOwnSelection || loaded_clipboard) {
        if (_XGrClipboardLen > 0)
            return 1; // ready
        else
            return 0; // no data
    }

    if (XGetSelectionOwner(_XGrDisplay, _XGrCBSelection) == None)
        return 0; // no data

    XConvertSelection(_XGrDisplay, _XGrCBSelection, _XGrCBUTF8,
                       _XGrCBProperty, _XGrWindow, CurrentTime);
    waiting_selection_notify = 1;

    return -1; // pending data, wait for a GREV_CBREPLY event
}

int _XGrLoadClipBoard(void)
{
    // this function is called by the x11 video driver only
    Atom type;
    int format, i, ret = 0;
    unsigned long nitems, len, remain;
    unsigned char *data = NULL;

    if (waiting_selection_notify) {
        /* First get type and size. */
        XGetWindowProperty(_XGrDisplay, _XGrWindow, _XGrCBProperty, 0, 0,
                           False, AnyPropertyType,
                           &type, &format, &nitems, &len, &data);
        if (data) XFree(data);
        if (type == _XGrCBIncr) { // data too large, INCR not implemented
            ret = 0;
            //printf("XClipboard, data too large\n");
        } else {
            XGetWindowProperty(_XGrDisplay, _XGrWindow, _XGrCBProperty, 0, len,
                               False, AnyPropertyType,
                               &type, &format, &nitems, &remain, &data);
            clear_cb();
            //printf("XClipboard, read %ld bytes\n", len);
            if ((clipboard_maxchars != GR_CB_NOLIMIT) &&
                (len > clipboard_maxchars)) {
                len = clipboard_maxchars;
                partially_loaded = 1;
            }
            _XGrClipboard = malloc(len+1);
            if (_XGrClipboard) {
                for (i=0; i<len; i++) {
                    _XGrClipboard[i] = data[i];
                }
                _XGrClipboard[len] = '\0';
                _XGrClipboardLen = GrUTF8StrLen(_XGrClipboard);
                _XGrClipboardByteLen = len;
                loaded_clipboard = 1;
                ret = 1;
            }
            if (data) XFree(data);
        }
    }

    XDeleteProperty(_XGrDisplay, _XGrWindow, _XGrCBProperty);
    waiting_selection_notify = 0;

    return ret;
}

void *GrGetClipBoard(int maxlen, int chrtype)
{
    int len;

    if (!_XGrClipboard) return NULL;

    len = (maxlen > _XGrClipboardLen) ? _XGrClipboardLen : maxlen;

    loaded_clipboard = 0;

    return GrTextRecodeFromUTF8(_XGrClipboard, len, chrtype);
}

int GrGetClipBoardLen(int *partloaded)
{
    if (partloaded) *partloaded = partially_loaded;
    if (!_XGrClipboard) return 0;
    return _XGrClipboardLen;
}
