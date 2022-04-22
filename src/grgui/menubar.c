/**
 ** menubar.c ---- Mini GUI for MGRX, menubar
 **
 ** Copyright (C) 2002,2006,2019 Mariano Alvarez Fernandez
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "grguip.h"

#define MAX_MENUBARITEMS 20

static GUIContext *gctx;

static struct {
    int width;
    int height;
    int starty;
    int startx[MAX_MENUBARITEMS];
    int length[MAX_MENUBARITEMS];
} mbd;

static GUIMenuBar *smb = NULL;

static GrTextOption grtopt;

static int smbshowed = 0;
static int smbinuse = 0;

static long mbshortcutkey = GrKey_Alt_Return; // default to activate the menu bar

static void menubarhide(int restore);
static void calculate_menubar_dims(void);
static void show_menubar_item(int ind, int selected);
static void blt_menubar_item(int ind);

void _GUIMenuBarInit(void)
{
    smb = NULL;

    grtopt.txo_font = _menufont;
    grtopt.txo_fgcolor = _menufgcolor;
    grtopt.txo_bgcolor = GrNOCOLOR;
    grtopt.txo_chrtype = GR_WORD_TEXT;
    grtopt.txo_direct = GR_TEXT_RIGHT;
    grtopt.txo_xalign = GR_ALIGN_LEFT;
    grtopt.txo_yalign = GR_ALIGN_TOP;

    GrEventAddHook(_GUIMenuBarHookEvent);
}

void _GUIMenuBarEnd(void)
{
    menubarhide(0);
    GrEventDeleteHook(_GUIMenuBarHookEvent);
}

void GUIMenuBarSet(GUIMenuBar *mb)
{
    smb = mb;
}

void GUIMenuBarSetShortCut(long key)
{
    mbshortcutkey = key;
}

static long alt_table[26] = {
    GrKey_Alt_A, GrKey_Alt_B, GrKey_Alt_C, GrKey_Alt_D, GrKey_Alt_E,
    GrKey_Alt_F, GrKey_Alt_G, GrKey_Alt_H, GrKey_Alt_I, GrKey_Alt_J,
    GrKey_Alt_K, GrKey_Alt_L, GrKey_Alt_M, GrKey_Alt_N, GrKey_Alt_O,
    GrKey_Alt_P, GrKey_Alt_Q, GrKey_Alt_R, GrKey_Alt_S, GrKey_Alt_T,
    GrKey_Alt_U, GrKey_Alt_V, GrKey_Alt_W, GrKey_Alt_X, GrKey_Alt_Y,
    GrKey_Alt_Z};

static long greek_to_latin(long key)
{
    //                    "ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ"
    static char table[] = "ABGDEZHUIKLMNJOPRSTYFXCV";
    unsigned char *skey = (unsigned char *)&key;

    if (skey[0] == 0xCE && skey[1] >= 0x91 && skey[1] <= 0xa1) {
        return table[skey[1]-0x91];
    }
    if (skey[0] == 0xCE && skey[1] >= 0xa3 && skey[1] <= 0xa9) {
        return table[skey[1]-0x92];
    }

    return key;
}

void GUIMenuBarSetI18nFields(void)
{
    int i, nb;
    unsigned char *p;
    long key;

    if (smb == NULL) return;

    for (i=0; i<smb->nitems; i++) {
        if (smb->i[i].sid >= 0) {
            smb->i[i].title = (char *)GrI18nGetString(smb->i[i].sid);
            p = (unsigned char *)strchr(smb->i[i].title, '&');
            if (p) {
                if (_menuchrtype == GR_UTF8_TEXT) {
                    key = GrNextUTF8Char((p+1), &nb);
                    key = GrUTF8toupper(key);
                    key = greek_to_latin(key);
                } else {
                    key = *(p+1);
                    if (key < 128) key = toupper(key);
                }
                if (key >= 'A' && key <= 'Z')
                    smb->i[i].key = alt_table[key-'A'];
            }
        }
    }
}

int GUIMenuBarSetItemEnable(int idm, int enable)
{
    int i;

    if (smb == NULL) return -1;

    for (i=0; i<smb->nitems; i++) {
        if (smb->i[i].idm == idm) {
            smb->i[i].enabled = enable;
            return 0;
        }
    }

    return -1;
}

int GUIMenuBarGetItemEnable(int idm)
{
    int i;

    if (smb == NULL) return -1;

    for (i=0; i<smb->nitems; i++) {
        if (smb->i[i].idm == idm) {
            return smb->i[i].enabled;
        }
    }

    return -1;
}

void GUIMenuBarShow(void)
{
    GrContext grcaux;
    int i;

    if (smb == NULL) return;

    calculate_menubar_dims();
    gctx = GUIContextCreate(0, 0, mbd.width, mbd.height, 1);
    if (gctx == NULL) return;

    smbshowed = 1;

    GrSaveContext(&grcaux);
    GUIContextSaveUnder(gctx);
    GrSetContext(gctx->c);
    GrClearContext(_menubgcolor);
    GrHLine(0, mbd.width-1, mbd.height-1, _menufgcolor);

    _GUIKeyShortCutReset();
    for (i=0; i<smb->nitems; i++) {
        show_menubar_item(i, 0);
        if (smb->i[i].enabled)
            _GUIMenuSetKeyShortCut(smb->i[i].idm);
    }
    GUIDBCurCtxBltToScreen();

    GrSetContext(&grcaux);
}

void GUIMenuBarHide(void)
{
    menubarhide(1);
}

static void menubarhide(int restore)
{
    if (!smbshowed) return;

    smbshowed = 0;

    _GUIKeyShortCutReset();
    if (restore) GUIContextRestoreUnder(gctx);
    GUIContextDestroy(gctx);
}
/*
void GUIMenuBarRedraw(void)
{
    GrContext grcaux;
    int i;

    if (smb == NULL) return;
    if (gctx == NULL) return;
    if (!smbshowed) return;

    GrSaveContext(&grcaux);
    //GUIContextSaveUnder(gctx);
    GrSetContext(gctx->c);
    GrClearContext(_menubgcolor);
    GrHLine(0, mbd.width-1, mbd.height-1, _menufgcolor);

    for (i=0; i<smb->nitems; i++) {
        show_menubar_item(i, 0);
    }
    GUIDBCurCtxBltToScreen();

    GrSetContext(&grcaux);
}
*/
void GUIMenuBarEnable(void)
{
}

void GUIMenuBarDisable(void)
{
}

int GUIMenuBarGetHeight(void)
{
    calculate_menubar_dims();
    return mbd.height;
}

int _GUIMenuBarInUse(void)
{
    return smbinuse;
}

int _GUIMenuBarHookEvent(GrEvent *ev)
{
    GrContext grcaux;
    GrEvent evaux, evq;
    int i, c, pos;

    if (smb == NULL) return 0;
    if (!smbshowed) return 0;
    //if (_GUIDialogRunning()) return 0;
    if (_GUIGetNoHookNow()) return 0;

    evaux = *ev;

    if (ev->type == GREV_KEY && ev->p2 == GRKEY_KEYCODE) {
        for (i = 0; i<smb->nitems; i++) {
            if (ev->p1 == smb->i[i].key) {
                evq.type = GREV_PRIVGUI;
                evq.p1 = GPEV_MENUCANCEL;
                evq.p2 = 0;
                evq.p3 = 0;
                evq.p4 = 0;
                GrEventEnqueue(&evq);
                evq.p1 = GPEV_ACTMENUBAR;
                evq.p2 = i;
                evq.p3 = 0;
                evq.p4 = 0;
                GrEventEnqueue(&evq);
                return 1;
            }
        }
        if (smbinuse) {
            i = 0;
            if (ev->p1 == GrKey_Left) i = -1;
            if (ev->p1 == GrKey_Right) i = 1;
            if (i) {
                pos = smb->select + i;
                if (pos < 0) pos = smb->nitems - 1;
                if (pos >= smb->nitems) pos = 0;
                evq.type = GREV_PRIVGUI;
                evq.p1 = GPEV_MENUCANCEL;
                evq.p2 = 0;
                evq.p3 = 0;
                evq.p4 = 0;
                GrEventEnqueue(&evq);
                evq.p1 = GPEV_ACTMENUBAR;
                evq.p2 = pos;
                evq.p3 = 0;
                evq.p4 = 0;
                GrEventEnqueue(&evq);
                return 1;
            }
        } else {
            if (ev->p1 == mbshortcutkey) {
                evq.type = GREV_PRIVGUI;
                evq.p1 = GPEV_MENUCANCEL;
                evq.p2 = 0; 
                evq.p3 = 0;
                evq.p4 = 0;
                GrEventEnqueue(&evq);
                evq.p1 = GPEV_ACTMENUBAR;
                evq.p2 = smb->select; // last menu selected
                evq.p3 = 0;
                evq.p4 = 0;
                GrEventEnqueue(&evq);
                return 1;
            }
        }
    }

    if (ev->type == GREV_MOUSE) {
        c = ev->p1;
        switch (c) {
            case GRMOUSE_LB_PRESSED :
                if (!GUIContextTransMouseEvIfInCtx(gctx, &evaux))
                    return 0;
                pos = -1;
                for (i=0; i<smb->nitems; i++) {
                    if (evaux.p2 > mbd.startx[i] &&
                        evaux.p2 < mbd.startx[i] + mbd.length[i]) {
                        pos = i;
                        break;
                    }
                }
                if (pos < 0) return 0;
                if (smbinuse && pos == smb->select) return 1;
                evq.type = GREV_PRIVGUI;
                evq.p1 = GPEV_MENUCANCEL;
                evq.p2 = 0;
                evq.p3 = 0;
                GrEventEnqueue(&evq);
                evq.p1 = GPEV_ACTMENUBAR;
                evq.p2 = pos;
                GrEventEnqueue(&evq);
                return 1;
            case GRMOUSE_LB_RELEASED :
                return 0;
        }
    }

    if (ev->type == GREV_MMOVE) {
        if (smbinuse && (ev->p1 & GRMOUSE_LB_STATUS)) {
            if (!GUIContextTransMouseEvIfInCtx(gctx, &evaux))
                return 0;
            pos = -1;
            for (i=0; i<smb->nitems; i++) {
                if (evaux.p2 > mbd.startx[i] &&
                    evaux.p2 < mbd.startx[i] + mbd.length[i]) {
                    pos = i;
                    break;
                }
            }
            if (pos < 0) return 0;
            if (pos == smb->select) return 1;
            evq.type = GREV_PRIVGUI;
            evq.p1 = GPEV_MENUCANCEL;
            evq.p2 = 0;
            evq.p3 = 0;
            GrEventEnqueue(&evq);
            evq.p1 = GPEV_ACTMENUBAR;
            evq.p2 = pos;
            GrEventEnqueue(&evq);
            return 1;
        }
    }

    if (ev->type == GREV_PRIVGUI && ev->p1 == GPEV_ACTMENUBAR) {
        smbinuse = 1;
        smb->select = ev->p2;
        GrSaveContext(&grcaux);
        show_menubar_item(smb->select,1);
        blt_menubar_item(smb->select);
        _GUIMenuRun(smb->i[smb->select].idm, mbd.startx[smb->select]+1,
                    mbd.height+1, 0);
        show_menubar_item(smb->select,0);
        blt_menubar_item(smb->select);
        GrSetContext(&grcaux);
        smbinuse = 0;
        return 1;
    }

    return 0;
}

/***/

static void calculate_menubar_dims(void)
{
    int i, l, space, land;

    grtopt.txo_font = _menufont;
    mbd.width = GrScreenX() + 1;
    mbd.height = _menufont->h.height + 4;
    mbd.starty = 1;
    mbd.startx[0] = 1;
    space = _menufont->maxwidth;
    land = GrFontCharWidth(_menufont, '&');
    for (i=1; i<smb->nitems; i++) {
        l = GrFontStringWidth(_menufont, smb->i[i-1].title, 0, _menuchrtype);
        if (strchr(smb->i[i-1].title, '&') != NULL) l -= land;
        mbd.length[i-1] = l + 4;
        mbd.startx[i] = mbd.startx[i-1] + l + 4 + space;
    }
    l = GrFontStringWidth(_menufont, smb->i[i-1].title, 0, _menuchrtype);
    if (strchr(smb->i[i-1].title, '&') != NULL) l -= land;
    mbd.length[i-1] = l + 4;
}

static void show_menubar_item(int ind, int selected)
{
    int i, x, y, len, aps, incrx;
    unsigned short *wtext;

    GrSetContext(gctx->c);

    x = mbd.startx[ind];
    y = mbd.starty;
    GrFilledBox(x, y, x+mbd.length[ind]-1, y+mbd.height-4,
                selected ? _menubgscolor : _menubgcolor);
    len = GrStrLen(smb->i[ind].title, _menuchrtype);
    wtext = GrFontTextRecode(grtopt.txo_font, smb->i[ind].title, len, _menuchrtype);
    if (wtext == NULL) return;
    aps = -1;
    for (i=0; i<len; i++) {
        if (wtext[i] == '&') aps = i;
    }
    x = mbd.startx[ind] + 2;
    grtopt.txo_fgcolor = selected ? _menufgscolor : _menufgcolor;
    if (aps >= 0){
        if (aps > 0) {
            GrDrawString(wtext, aps, x, y, &grtopt);
            incrx = GrStringWidth(wtext, aps, &grtopt);
        } else {
            incrx = 0;
        }
        grtopt.txo_fgcolor |= GR_UNDERLINE_TEXT;
        GrDrawString(&(wtext[aps+1]), 1, x+incrx, y, &grtopt);
        incrx += GrStringWidth(&(wtext[aps+1]), 1, &grtopt);
        grtopt.txo_fgcolor &= ~GR_UNDERLINE_TEXT;
        GrDrawString(&(wtext[aps+2]), len-aps-2, x+incrx, y, &grtopt);
    }
    else
        GrDrawString(wtext, len, x, y, &grtopt);
    
    free(wtext);
}

static void blt_menubar_item(int ind)
{
    int x, y;

    x = mbd.startx[ind];
    y = mbd.starty;
    GUIDBCurCtxBltRectToScreen(x, y, x+mbd.length[ind]-1, y+mbd.height-4);
}
