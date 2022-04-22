/**
 ** clb_w32.c ---- Win32 clipboard
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
#include "libwin32.h"

// the W32 implementation uses UCS2 for the clipboard buffer

static int clipboard_maxchars = 32000;
static unsigned short *clipboard = NULL;
static int clipboard_len = 0;
static DWORD cbseq = 0;
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
    cbseq = 0;
}

int GrPutClipBoard(void *buf, int len, int chrtype)
{
    HGLOBAL mem;
    wchar_t *wstr;
    int i;

    clear_cb();

    if ((clipboard_maxchars != GR_CB_NOLIMIT) &&
        (len > clipboard_maxchars)) {
        len = clipboard_maxchars;
        partially_loaded = 1;
    }

    clipboard = GrTextRecodeToUCS2(buf, len, chrtype);
    if (!clipboard) return 0;

    clipboard_len = len;

    if (!OpenClipboard(hGRXWnd)) {
        clear_cb();
        return 0;
    }
    mem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t)*len+1);
    if (mem == NULL) {
        clear_cb();
        CloseClipboard();
        return 0;
    }
    wstr = (wchar_t *)GlobalLock(mem);
    if (wstr == NULL) {
        clear_cb();
        GlobalFree(mem);
        CloseClipboard();
        return 0;
    }
    for (i=0; i<len; i++)
        wstr[i] = (wchar_t)clipboard[i];
    wstr[len] = 0;

    GlobalUnlock(mem);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, mem);
    CloseClipboard();
    cbseq = GetClipboardSequenceNumber();

    return len;
}

int GrIsClipBoardReadyToGet(void)
{
    static DWORD cbseq2;
    HANDLE hdata;
    wchar_t *wstr;
    int len;

    cbseq2 = GetClipboardSequenceNumber();

    if (cbseq2 == cbseq) { // the cb has not changed
        if (clipboard_len > 0)
            return 1; // ready
        else
            return 0; // no data
    }

    if (!OpenClipboard(hGRXWnd)) {
        return 0; // can not open clipboard, so no data
    }
    hdata = GetClipboardData(CF_UNICODETEXT);
    if (hdata == NULL) { // error, so no data
        CloseClipboard();
        return 0;
    }
    wstr = (wchar_t *)GlobalLock(hdata);
    if (wstr == NULL) { // error, so no data
        CloseClipboard();
        return 0;
    }
    len = GrStrLen(wstr, GR_UCS2_TEXT);

    clear_cb();

    if ((clipboard_maxchars != GR_CB_NOLIMIT) &&
        (len > clipboard_maxchars)) {
        len = clipboard_maxchars;
        partially_loaded = 1;
    }

    clipboard = GrTextRecodeToUCS2(wstr, len, GR_UCS2_TEXT);
    if (!clipboard){ // error, so no data
        GlobalUnlock(hdata);
        CloseClipboard();
        return 0;
    }

    clipboard_len = len;

    GlobalUnlock(hdata);
    CloseClipboard();
    cbseq = GetClipboardSequenceNumber();

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
