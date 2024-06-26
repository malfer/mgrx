/**
 ** grev_w32.c ---- MGRX events, W32 input
 **
 ** Copyright (C) 2005 Mariano Alvarez Fernandez
 ** [e-mail: malfer@telefonica.net]
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
 ** Contributions by:
 ** 080120 M.Alvarez, intl support
 ** 080204 M.Alvarez, generate wheel events
 ** 081117 Richard, fixed bug in GrMouseWarp
 ** 191112 M.Alvarez, Added code to generate GREV_WMEND events
 ** 211123 M.Alvarez, support window resize
 ** 220318 M.Alvarez, changed ALT09AZ recognition to be more generic
 **/

#include <stdlib.h>
#include <string.h>
#include "libgrx.h"
#include "libwin32.h"
#include "mgrxkeys.h"
#include "ninput.h"
#include "w32keys.h"
#include "arith.h"

static int kbd_lastmod = 0;
static int kbsysencoding = -1;

static void init_w32queue(int queue_size);
static unsigned short StdKeyTranslate(int winkey, int fkbState);

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
    char *s;
    int ue;

    init_w32queue(40);
    s = getenv("MGRXKBSYSENCODING");
    if (s != NULL) kbsysencoding = GrFindEncoding(s);
    if (kbsysencoding < 0) {
        ue = GetACP();
        if (ue == 1251) kbsysencoding = GRENC_CP1251;
        else if (ue == 1252) kbsysencoding = GRENC_CP1252;
        else if (ue == 1253) kbsysencoding = GRENC_CP1253;
        //the utf-8 codepage doesn't work as I expected :-(
        //else if (ue == 65001) kbsysencoding = GRENC_UTF_8;
    }
    if (kbsysencoding < 0) kbsysencoding = GRENC_CP1252;
    //printf("kbsysencoding %d \n", kbsysencoding);
    return 1;
}

/**
 ** _GrEventUnInit - UnInitializes inputs
 **
 ** For internal use only
 **/

void _GrEventUnInit(void)
{
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
    GrEvent ev;
    W32Event evaux;
    int key;
    int nev = 0;

    while (_W32EventQueueLength > 0) {
        EnterCriticalSection(&_csEventQueue);
        evaux = _W32EventQueue[_W32EventQueueRead];
        if (++_W32EventQueueRead == _W32EventQueueSize)
            _W32EventQueueRead = 0;
        _W32EventQueueLength--;
        LeaveCriticalSection(&_csEventQueue);

        switch (evaux.uMsg) {
        case WM_CLOSE:
            ev.type = GREV_WMEND;
            ev.kbstat = 0;
            ev.p1 = 0;
            ev.p2 = 0;
            ev.p3 = 0;
            ev.p4 = 0;
            GrEventEnqueue(&ev);
            nev++;
            break;
        case WM_CHAR:
            ev.type = GREV_PREKEY;
            ev.kbstat = evaux.kbstat;
            ev.p1 = evaux.wParam;
            ev.p2 = 1;
            ev.p3 = 0;
            ev.p4 = 0;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev++;
            break;
        case WM_SYSCHAR:
            key = 0;
            /*if (evaux.wParam >= 'a' && evaux.wParam <= 'z')
                key = altletters[evaux.wParam - 'a'];
            else if (evaux.wParam >= 'A' && evaux.wParam <= 'Z')
                key = altletters[evaux.wParam - 'A'];
            else if (evaux.wParam >= '0' && evaux.wParam <= '9')
                key = altnumbers[evaux.wParam - '0'];
            else*/ if (evaux.wParam == 13)
                key = GrKey_Alt_Return;
            else if (evaux.wParam == 8)
                key = GrKey_Alt_Backspace;
            if (key == 0)
                break;
            ev.type = GREV_PREKEY;
            ev.kbstat = evaux.kbstat;
            ev.p1 = key;
            ev.p2 = GRKEY_KEYCODE;
            ev.p3 = 0;
            ev.p4 = 0;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev++;
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            key = StdKeyTranslate(evaux.wParam, evaux.kbstat);
            if (key == 0 && (evaux.kbstat & GRKBS_ALT)) { // VK_codes for 0..9, A..Z are ascii
                if (evaux.wParam >= 'A' && evaux.wParam <= 'Z')
                    key = altletters[evaux.wParam - 'A'];
                else if (evaux.wParam >= '0' && evaux.wParam <= '9')
                    key = altnumbers[evaux.wParam - '0'];
            }
            if (key == 0)
                break;
            ev.type = GREV_PREKEY;
            ev.kbstat = evaux.kbstat;
            ev.p1 = key;
            ev.p2 = GRKEY_KEYCODE;
            ev.p3 = 0;
            ev.p4 = 0;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev++;
            break;
        case WM_MOUSEMOVE:
            MOUINFO->xpos = LOWORD(evaux.lParam);
            MOUINFO->ypos = HIWORD(evaux.lParam);
            MOUINFO->moved = TRUE;
            GrMouseUpdateCursor();
            if ((MOUINFO->genmmove == GR_GEN_MMOVE_ALWAYS) ||
                ((MOUINFO->genmmove == GR_GEN_MMOVE_IFBUT) &&
                 (MOUINFO->bstatus != 0))) {
                ev.type = GREV_MMOVE;
                ev.kbstat = kbd_lastmod;
                ev.p1 = MOUINFO->bstatus;
                ev.p2 = MOUINFO->xpos;
                ev.p3 = MOUINFO->ypos;
                ev.p4 = 0;
                GrEventEnqueue(&ev);
                MOUINFO->moved = FALSE;
                nev++;
            }
            break;
        case WM_LBUTTONDOWN:
            ev.type = GREV_MOUSE;
            ev.kbstat = evaux.kbstat;
            ev.p1 = GRMOUSE_LB_PRESSED;
            ev.p2 = MOUINFO->xpos;
            ev.p3 = MOUINFO->ypos;
            ev.p4 = 0;
            MOUINFO->bstatus |= GRMOUSE_LB_STATUS;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev++;
            break;
        case WM_MBUTTONDOWN:
            ev.type = GREV_MOUSE;
            ev.kbstat = evaux.kbstat;
            ev.p1 = GRMOUSE_MB_PRESSED;
            ev.p2 = MOUINFO->xpos;
            ev.p3 = MOUINFO->ypos;
            ev.p4 = 0;
            MOUINFO->bstatus |= GRMOUSE_MB_STATUS;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev++;
            break;
        case WM_RBUTTONDOWN:
            ev.type = GREV_MOUSE;
            ev.kbstat = evaux.kbstat;
            ev.p1 = GRMOUSE_RB_PRESSED;
            ev.p2 = MOUINFO->xpos;
            ev.p3 = MOUINFO->ypos;
            ev.p4 = 0;
            MOUINFO->bstatus |= GRMOUSE_RB_STATUS;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev++;
            break;
        case WM_LBUTTONUP:
            ev.type = GREV_MOUSE;
            ev.kbstat = evaux.kbstat;
            ev.p1 = GRMOUSE_LB_RELEASED;
            ev.p2 = MOUINFO->xpos;
            ev.p3 = MOUINFO->ypos;
            ev.p4 = 0;
            MOUINFO->bstatus &= ~GRMOUSE_LB_STATUS;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev++;
            break;
        case WM_MBUTTONUP:
            ev.type = GREV_MOUSE;
            ev.kbstat = evaux.kbstat;
            ev.p1 = GRMOUSE_MB_RELEASED;
            ev.p2 = MOUINFO->xpos;
            ev.p3 = MOUINFO->ypos;
            ev.p4 = 0;
            MOUINFO->bstatus &= ~GRMOUSE_MB_STATUS;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev++;
            break;
        case WM_RBUTTONUP:
            ev.type = GREV_MOUSE;
            ev.kbstat = evaux.kbstat;
            ev.p1 = GRMOUSE_RB_RELEASED;
            ev.p2 = MOUINFO->xpos;
            ev.p3 = MOUINFO->ypos;
            ev.p4 = 0;
            MOUINFO->bstatus &= ~GRMOUSE_RB_STATUS;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev++;
            break;
        case WM_MOUSEWHEEL:
            ev.type = GREV_MOUSE;
            ev.kbstat = evaux.kbstat;
            ev.p2 = MOUINFO->xpos;
            ev.p3 = MOUINFO->ypos;
            ev.p4 = 0;
            ev.p1 = ((short)HIWORD(evaux.wParam) > 0) ?
                    GRMOUSE_B4_PRESSED : GRMOUSE_B5_PRESSED;
            GrEventEnqueue(&ev);
            ev.p1 = ((short)HIWORD(evaux.wParam) > 0) ?
                    GRMOUSE_B4_RELEASED : GRMOUSE_B5_RELEASED;
            GrEventEnqueue(&ev);
            MOUINFO->moved = FALSE;
            kbd_lastmod = evaux.kbstat;
            nev += 2;
            break;
        case WM_SIZE :
            ev.type = GREV_WSZCHG;
            ev.kbstat = 0;
            ev.p1 = 0;
            ev.p2 = 0;
            ev.p3 = LOWORD (evaux.lParam);
            ev.p4 = HIWORD (evaux.lParam);
            if (ev.p3 != _W32ClientWidth || ev.p4 != _W32ClientHeight) {
                GrEventEnqueue(&ev);
                nev++;
            }
            break;
        }
    }
    if (nev == 0) Sleep(1); /* yield */
    return nev;
}

/**
 ** _GrMouseDetect - Returns true if a mouse is detected
 **
 ** For internal use only
 **/

int _GrMouseDetect(void)
{
    return GetSystemMetrics(SM_MOUSEPRESENT);
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
    POINT point;

    MOUINFO->xpos = imax(MOUINFO->xmin, imin(MOUINFO->xmax, x));
    MOUINFO->ypos = imax(MOUINFO->ymin, imin(MOUINFO->ymax, y));
    GrMouseUpdateCursor();
    if (MOUINFO->msstatus == 2) { // hack to be ok with window resize
        point.x = MOUINFO->xpos;
        point.y = MOUINFO->ypos;
        ClientToScreen(hGRXWnd, &point);
        SetCursorPos(point.x, point.y);
    }
}

/** Internal functions **/

static void init_w32queue(int queue_size)
{
    EnterCriticalSection(&_csEventQueue);
    if (_W32EventQueue == NULL || _W32EventQueueSize != queue_size) {
        if (_W32EventQueue != NULL) free(_W32EventQueue);
        _W32EventQueue = (W32Event *)malloc(sizeof(W32Event) * queue_size);
        _W32EventQueueSize = _W32EventQueue ? queue_size : 0;
        _W32EventQueueRead = 0;
        _W32EventQueueWrite = 0;
        _W32EventQueueLength = 0;
    } else {
        int last = -1;
        if (_W32EventQueueLength > 0) {
            last = (_W32EventQueueWrite > 0) ? _W32EventQueueWrite-1 : _W32EventQueueLength-1;
        }
        _W32EventQueueRead = 0;
        _W32EventQueueWrite = 0;
        _W32EventQueueLength = 0;
        if (last >= 0) { // save the last WM_SIZE if any
            if (_W32EventQueue[last].uMsg == WM_SIZE) {
                _W32EventQueue[0] =  _W32EventQueue[last];
                _W32EventQueueWrite = 1;
                _W32EventQueueLength = 1;
            }
        }
    }
    LeaveCriticalSection(&_csEventQueue);
}

static unsigned short StdKeyTranslate(int winkey, int fkbState)
{
    keytrans *k;
    int i;

    if (fkbState & GRKBS_ALT)
        k = altstdkeys;
    else if (fkbState & GRKBS_CTRL)
        k = controlstdkeys;
    else if (fkbState & GRKBS_SHIFT)
        k = shiftstdkeys;
    else
        k = stdkeys;

    for (i = 0; i < NSTDKEYS; i++) {
        if (winkey == k[i].winkey)
            return k[i].grkey;
    }

    return 0;
}
