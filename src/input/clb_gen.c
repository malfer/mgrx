/**
 ** clb_gen.c ---- generic clipboard (internal)
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

// the MGRX generic implementation uses UCS2 for the clipboard buffer

static int clipboard_maxchars = 32000;
static unsigned short *clipboard = NULL;
static int clipboard_len = 0;
static int partially_loaded = 0;

static void clear_cb(void)
{
    if (clipboard) free(clipboard);
    clipboard = NULL;
    clipboard_len = 0;
    partially_loaded = 0;
}

void GrSetClipBoardMaxChars(int maxchars)
{
    if (maxchars <= 0) maxchars = GR_CB_NOLIMIT;
    clipboard_maxchars = maxchars;
}

void GrClearClipBoard(void)
{
    clear_cb();
}

int GrPutClipBoard(void *buf, int len, int chrtype)
{
    clear_cb();

    if ((clipboard_maxchars != GR_CB_NOLIMIT) &&
        (len > clipboard_maxchars)) {
        len = clipboard_maxchars;
        partially_loaded = 1;
    }

    clipboard = GrTextRecodeToUCS2(buf, len, chrtype);
    if (!clipboard) return 0;

    clipboard_len = len;

    return len;
}

int GrIsClipBoardReadyToGet(void)
{
    if (clipboard_len > 0)
        return 1; // ready
    else
        return 0; // no data
}

void *GrGetClipBoard(int maxlen, int chrtype)
{
    int len;

    if (!clipboard) return NULL;

    len = (maxlen > clipboard_len) ? clipboard_len : maxlen;

    return GrTextRecodeFromUCS2(clipboard, len, chrtype);
}

int GrGetClipBoardLen(int *partloaded)
{
    if (partloaded) *partloaded = partially_loaded;
    if (!clipboard) return 0;
    return clipboard_len;
}
