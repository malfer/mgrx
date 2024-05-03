/**
 ** rawtedit.c ---- Mini GUI for MGRX, raw text editor
 **
 ** Copyright (C) 2022-2024 Mariano Alvarez Fernandez
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
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "grguip.h"

#define MAX_LINE_SIZE 32000
#define INITIAL_LINE_SIZE 10
#define INCR_LINE_SIZE 50

#define MAX_NUMLINES 32000
#define INITIAL_NUMLINES 1000
#define INCR_NUMLINES 1000

#define BLINK_TIME 500

typedef struct {
    unsigned short c;      // glyph number
    unsigned short u;      // UCS2 char
    unsigned char fg;      // char fg color index
    unsigned char font;    // char font index TODO
    int x;                 // x-pos
} RTEChar;

typedef struct {
    RTEChar *cl;           // char array
    unsigned short maxlen; // max chars (auto extended if needed)
    unsigned short len;    // actual number of chars
    int y;                 // y-pos
    int nextx;             // next char x-pos
} RTELine;

typedef struct _guiRawTEdit {
    int editable;          // 0=no, 1=yes
    int xpos, ypos;        // left-upper corner in context
    int width, height;     // rectangle dimensions in context
    GUIScrollbar *vscb;    // vertical scrollbar (can be NULL)
    GUIScrollbar *hscb;    // horizontal scrollbar (can be NULL)
    GrContext *scbctx;     // scrollbar context (can be NULL)
    int scbdx, scbdy;      // scrollbar displacement
    GrColor ctbl[256];     // color table
    unsigned char numcolors;  // num colors in table
    unsigned char bgicolor;   // bg index color
    unsigned char cfgicolor;  // char fg index color for new chars
    unsigned char tcicolor;   // text cursor index color
    GrTextOption grt;      // actual GrTextOption
    RTELine *l;            // line array
    int maxlines;          // max lines (auto extended if needed)
    int nlines;            // actual number of lines
    int flscr;             // first line on screen
    int nlscr;             // lines on screen
    int xorg;              // x origen (chars showed from x>=xorg)
    int xbits, ybits;      // max x-bits, y-bits per char
    int tcstatus;          // text cursor estatus (showed if >= 0)
    int tclpos;            // text cursor line pos
    int tccpos;            // text cursor char pos
    int tcclast;           // last text cursor char pos
    int tcblink;           // text cursor blinking
    long tctime;           // text cursor blink time
    int pressed;           // left mouse pressed
    int full;              // full, no more chars or lines can be added
    int fmline, fmch;      // line&col first char of marked area
    int lmline, lmch;      // line&col last char of marked area
    int pvmline, pvmch;    // line&col pivot char to extend marked area
    int changed;           // text changed after last reset 1=yes, 0=no
    int numchars;          // number of chars in (doesn't includes LF)
    int maxchars;          // numchars limit if != 0 (doesn't includes LF)
    int partloaded;        // last paste operation was partially loaded
} GUIRawTEdit;

static int process_event(GUIRawTEdit *ta, GrEvent *ev);

static void move_tcursor(GUIRawTEdit *ta, int nline, int nchar, int setmark);
static void move_tcursor_nodraw(GUIRawTEdit *ta, int nline, int nchar, int setmark);
static void move_tcursor_rel(GUIRawTEdit *ta, int incrl, int incrc, int setmark);
static void show_tcursor(GUIRawTEdit *ta);
static void hide_tcursor(GUIRawTEdit *ta);
static void draw_tcursor(GUIRawTEdit *ta, int show);
static void redraw(GUIRawTEdit *ta);
static void redraw_line_from(GUIRawTEdit *ta, int line, int col, int blit);
static void redraw_char(GUIRawTEdit *ta, int line, int col, int blit);

static void scroll_up(GUIRawTEdit *ta);
static void scroll_down(GUIRawTEdit *ta);
static void scroll_toshow_tc(GUIRawTEdit *ta, int draw);
static void scroll_nlines_up(GUIRawTEdit *ta, int nlines);
static void scroll_nlines_down(GUIRawTEdit *ta, int nlines);
static void scroll_xorg(GUIRawTEdit *ta, int newxorg);

static int calc_line_col_click(GUIRawTEdit *ta, int x, int y,
                               int *l, int *c, int restrictxy);
static int calc_maxx_in_screen(GUIRawTEdit *ta);
static void redraw_scbs(GUIRawTEdit *ta);

static void print_char(GUIRawTEdit *ta, long ch, int chrtype, int draw);
static void new_line(GUIRawTEdit *ta, int draw);
static void add_char(GUIRawTEdit *ta, long ch, int chrtype, int draw);
static void add_string(GUIRawTEdit *ta, void *s, int len, int chrtype, int draw);
static void add_multistring(GUIRawTEdit *ta, void *s, int len, int chrtype, int draw);
static void join_lines(GUIRawTEdit *ta, int draw);
static void delete_char(GUIRawTEdit *ta, int draw);
static void erase_char(GUIRawTEdit *ta, int draw);

static int in_marqued_area(GUIRawTEdit *ta, int line, int col);
static void reset_marqued_area(GUIRawTEdit *ta, int draw);
static void set_marqued_area(GUIRawTEdit *ta, int fl, int fc, int ll, int lc, int draw);
static void expand_marqued_area(GUIRawTEdit *ta, int draw);
static int delete_marqued_area(GUIRawTEdit *ta, int draw);
static void paste_marqued_area(GUIRawTEdit *ta, int draw);
static int copy_marqued_area(GUIRawTEdit *ta, int clear, int draw);
static int setfg_marqued_area(GUIRawTEdit *ta);

static int nd_del_line(GUIRawTEdit *ta, int line);
static int nd_del_char(GUIRawTEdit *ta, int line, int col);
static int nd_del_char_from(GUIRawTEdit *ta, int line, int col);
static int nd_del_char_to(GUIRawTEdit *ta, int line, int col);
static int nd_del_char_from_to(GUIRawTEdit *ta, int line, int fc, int lc);
static int nd_join_lines(GUIRawTEdit *ta, int line);
static unsigned short * nd_get_ucs2text(GUIRawTEdit *ta, int fline, int fc,
                                        int lline, int lc, int *len);
//static int nd_put_ucs2text(GUIRawTEdit *ta, unsigned short *buf,
//                           int len, int line, int col);
static int nd_add_char(GUIRawTEdit *ta, int line, int col, long ch, int chrtype);
static int nd_new_line(GUIRawTEdit *ta, int line, int col);
static int nd_setfg_char_from_to(GUIRawTEdit *ta, int line, int fc, int lc);

/**************************************************************************/

GUIRawTEdit * GUIRTECreate(GrFont *f, int editable, int maxchars,
                           int x, int y, int width, int height)
{
    GUIRawTEdit *ta;
    int i, initnumlines;

    initnumlines = INITIAL_NUMLINES;
    if (maxchars && maxchars/10 < INITIAL_NUMLINES) {
        initnumlines = maxchars/10;
        if (initnumlines < 10) initnumlines = 10;
    }

    ta = malloc(sizeof(GUIRawTEdit));
    if (ta == NULL) return NULL;
    ta->l = malloc(sizeof(RTELine)*initnumlines);
    if (ta->l == NULL) {
        free(ta);
        return NULL;
    }
    ta->l[0].cl = malloc(sizeof(RTEChar)*INITIAL_LINE_SIZE);
    if (ta->l[0].cl == NULL) {
        free(ta->l);
        free(ta);
        return NULL;
    }
    ta->editable = editable;
    ta->xpos = x;
    ta->ypos = y;
    ta->width = width;
    ta->height = height;
    ta->vscb = NULL;
    ta->hscb = NULL;
    ta->scbctx = NULL;
    ta->scbdx = 0;
    ta->scbdy = 0;
    ta->ctbl[0] = GrWhite();
    ta->ctbl[1] = GrBlack();
    ta->ctbl[2] = GrBlack();
    ta->numcolors = 3;
    ta->bgicolor = 0;
    ta->cfgicolor = 1;
    ta->tcicolor = 2;
    ta->grt.txo_font = (f) ? f : GrGetDefaultFont();
    ta->grt.txo_bgcolor = GrWhite();
    ta->grt.txo_fgcolor = GrBlack();
    ta->grt.txo_direct = GR_TEXT_RIGHT;
    ta->grt.txo_xalign = GR_ALIGN_LEFT;
    ta->grt.txo_yalign = GR_ALIGN_BOTTOM;
    ta->grt.txo_chrtype = GR_WORD_TEXT;
    ta->maxlines = initnumlines;
    ta->nlines = 1;
    ta->flscr = 0;
    ta->xbits = ta->grt.txo_font->maxwidth;
    ta->ybits = ta->grt.txo_font->h.height;
    ta->nlscr = (ta->height-1) / ta->ybits;
    ta->xorg = 0;
    ta->tcstatus = -1;
    ta->tclpos = 0;
    ta->tccpos = 0;
    ta->tcclast = 0;
    ta->tcblink = 1;
    ta->tctime = 0;
    ta->pressed = 0;
    ta->full = 0;
    ta->changed = 0;
    ta->l[0].len = 0;
    ta->l[0].maxlen = INITIAL_LINE_SIZE;
    ta->l[0].y = ta->ybits;
    ta->l[0].nextx = 0;
    for (i=1; i<ta->maxlines; i++) {
        ta->l[i].cl = NULL;
        ta->l[i].len = 0;
        ta->l[i].maxlen = 0;
        ta->l[i].y = ta->ybits * (i + 1);
        ta->l[i].nextx = 0;
    }
    ta->numchars = 0;
    ta->maxchars = maxchars;
    if (ta->maxchars != 0) {
        if (ta->maxchars < 10) ta->maxchars = 10;
    }
    ta->partloaded = 0;
    reset_marqued_area(ta, 0);

    return ta;
}

/**************************************************************************/

void GUIRTESetDims(GUIRawTEdit *ta, int x, int y, int width, int height)
{
    if (ta == NULL) return;

    ta->xpos = x;
    ta->ypos = y;
    ta->width = width;
    ta->height = height;
    ta->nlscr = (ta->height-1) / ta->ybits;
    ta->tcstatus = -1;
    ta->tcblink = 1;
    ta->tctime = 0;
}

/**************************************************************************/

void GUIRTESetScrollbars(GUIRawTEdit *ta, GUIScrollbar *vscb,
                         GUIScrollbar *hscb, GrContext *scbctx)
{
    if (ta == NULL) return;

    ta->vscb = vscb;
    ta->hscb = hscb;
    ta->scbctx = scbctx;
}

/**************************************************************************/

void GUIRTESetScrollbarsDisplacement(GUIRawTEdit *ta, int dx, int dy)
{
    ta->scbdx = dx;
    ta->scbdy = dy;
}

/**************************************************************************/

void GUIRTESetFont(GUIRawTEdit *ta, GrFont *f)
{
    int i, j, w;
    long ybits;

    if (ta == NULL || f == NULL) return;

    ta->grt.txo_font = f;
    ta->xbits = ta->grt.txo_font->maxwidth;
    ta->ybits = ta->grt.txo_font->h.height;
    ta->nlscr = (ta->height-1) / ta->ybits;

    ta->l[ta->flscr].y = ta->ybits;
    ybits = 0;
    for (i=ta->flscr-1; i>=0; i--) {
        ta->l[i].y = ybits;
        ybits -= ta->ybits;
    }
    ybits = ta->ybits * 2;
    for (i=ta->flscr+1; i<ta->maxlines; i++) {
        ta->l[i].y = ybits;
        ybits += ta->ybits;
    }
    for (i=0; i<ta->nlines; i++) {
        ta->l[i].nextx = 0;
        for (j=0; j<ta->l[i].len; j++) {
            ta->l[i].cl[j].x = ta->l[i].nextx;
            ta->l[i].cl[j].c =
                GrFontCharRecode(ta->grt.txo_font, ta->l[i].cl[j].u, GR_UCS2_TEXT);
            w = GrFontCharWidth(ta->grt.txo_font, ta->l[i].cl[j].c);
            ta->l[i].nextx += w;
        }
    }
}

/**************************************************************************/

void GUIRTESetSimpleColors(GUIRawTEdit *ta, GrColor bg, GrColor cfg, GrColor tc)
{
    if (ta == NULL) return;

    ta->ctbl[0] = bg;
    ta->ctbl[1] = cfg;
    ta->ctbl[2] = tc;
    ta->numcolors = 3;
    ta->bgicolor = 0;
    ta->cfgicolor = 1;
    ta->tcicolor = 2;
}

/**************************************************************************/

void GUIRTESetTableColors(GUIRawTEdit *ta, GrColor *ctbl, int numcolors,
                          int bgi, int cfgi, int tci)
{
    int i;

    if (ta == NULL) return;
    if (numcolors < 2) return;
    if (numcolors > 256) numcolors = 256;

    for (i=0; i<numcolors; i++) {
        ta->ctbl[i] = ctbl[i];
    }
    ta->numcolors = numcolors;
    ta->bgicolor = bgi;
    ta->cfgicolor = cfgi;
    ta->tcicolor = tci;
}

/**************************************************************************/

void GUIRTETextColorIndex(GUIRawTEdit *ta, int cfgi)
{
    if (ta == NULL) return;

    ta->cfgicolor = cfgi;
}

/**************************************************************************/

void GUIRTETextColorIndexMA(GUIRawTEdit *ta, int cfgi, int draw)
{
    // this funcction set the fg color and apply to the marqued area
    if (ta == NULL) return;

    ta->cfgicolor = cfgi;

    if (setfg_marqued_area(ta) && draw)
        GUIRTEReDraw(ta);
}

/**************************************************************************/

void GUIRTEBgColorIndex(GUIRawTEdit *ta, int bgi, int draw)
{
    // this funcction set the bg color
    if (ta == NULL) return;

    ta->bgicolor = bgi;

    if (draw)
        GUIRTEReDraw(ta);
}

/**************************************************************************/

void GUIRTECursorColorIndex(GUIRawTEdit *ta, int tci)
{
    if (ta == NULL) return;

    ta->tcicolor = tci;
}

/**************************************************************************/

void GUIRTEDestroy(GUIRawTEdit *ta)
{
    int i;
    
    if (ta == NULL) return;

    for (i=0; i<ta->maxlines; i++) {
        if (ta->l[i].cl) free(ta->l[i].cl);
    }
    free(ta->l);
    free(ta);
}

/**************************************************************************/

void GUIRTEClear(GUIRawTEdit *ta)
{
    int i;
    
    if (ta == NULL) return;

    ta->flscr = 0;
    ta->nlines = 1;
    for (i=0; i<ta->maxlines; i++) {
        ta->l[i].len = 0;
        ta->l[i].y = ta->ybits * (i + 1);
        ta->l[i].nextx = 0;
    }
    ta->xorg = 0;
    ta->tcstatus = -1;
    ta->tclpos = 0;
    ta->tccpos = 0;
    ta->tcclast = 0;
    ta->tcblink = 1;
    ta->tctime = 0;
    ta->pressed = 0;
    ta->numchars = 0;
    ta->full = 0;
    reset_marqued_area(ta, 0);
    
}

/**************************************************************************/

void GUIRTEShowTcursor(GUIRawTEdit *ta)
{
    if (ta == NULL) return;

    ta->tcstatus++;
        
    if (ta->tcstatus == 0) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        draw_tcursor(ta, 1);
        GrResetClipBox();
   }
}

/**************************************************************************/

void GUIRTEHideTcursor(GUIRawTEdit *ta)
{
    if (ta == NULL) return;

    ta->tcstatus--;
        
    if (ta->tcstatus == -1) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        draw_tcursor(ta, 0);
        GrResetClipBox();
    }
}

/**************************************************************************/

void GUIRTEMoveTCursor(GUIRawTEdit *ta, int nline, int nchar, int setmark, int draw)
{
    if (ta == NULL) return;

    if (draw) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        move_tcursor(ta, nline, nchar, setmark);
        scroll_toshow_tc(ta, 1);
        GrResetClipBox();
    } else {
        move_tcursor_nodraw(ta, nline, nchar, setmark);
        scroll_toshow_tc(ta, 0);
    }
}

/**************************************************************************/

void GUIRTENewLine(GUIRawTEdit *ta, int draw)
{
    if (ta == NULL) return;
    if (ta->full) return;

    if (draw) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        hide_tcursor(ta);
    }
    new_line(ta, draw);
    if (draw) {
        scroll_toshow_tc(ta, 1);
        show_tcursor(ta);
        GrResetClipBox();
    }
}

/**************************************************************************/

void GUIRTEPutChar(GUIRawTEdit *ta, long ch, int chrtype, int draw)
{
    if (ta == NULL) return;
    if (ta->full) return;

    if (draw) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        hide_tcursor(ta);
    }
    add_char(ta, ch, chrtype, draw);
    if (draw) {
        show_tcursor(ta);
        GrResetClipBox();
    }
}

/**************************************************************************/

void GUIRTEPutString(GUIRawTEdit *ta, void *s, int len, int chrtype, int draw)
{
    if (ta == NULL) return;
    if (ta->full) return;

    if (draw) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        hide_tcursor(ta);
    }
    add_string(ta, s, len, chrtype, draw);
    if (draw) {
        show_tcursor(ta);
        GrResetClipBox();
    }
}

/**************************************************************************/

void GUIRTEPutMultiString(GUIRawTEdit *ta, void *s, int len, int chrtype, int draw)
{
    if (ta == NULL) return;
    if (ta->full) return;

    if (draw) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        hide_tcursor(ta);
    }
    add_multistring(ta, s, len, chrtype, draw);
    if (draw) {
        show_tcursor(ta);
        GrResetClipBox();
    }
}

/**************************************************************************/

void GUIRTEResetMA(GUIRawTEdit *ta, int draw)
{
    if (ta == NULL) return;

    if (draw) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        hide_tcursor(ta);
    }
    reset_marqued_area(ta, draw);
    if (draw) {
        show_tcursor(ta);
        GrResetClipBox();
    }
}

/**************************************************************************/

void GUIRTESetMA(GUIRawTEdit *ta, int fl, int fc, int ll, int lc, int draw)
{
    if (ta == NULL) return;

    if (draw) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        hide_tcursor(ta);
    }
    set_marqued_area(ta, fl, fc, ll, lc, draw);
    if (draw) {
        show_tcursor(ta);
        GrResetClipBox();
    }
}

/**************************************************************************/

int GUIRTECopyMA(GUIRawTEdit *ta, int clear, int draw)
{
    int ret;

    if (ta == NULL) return 0;

    if (draw) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        hide_tcursor(ta);
    }
    ret = copy_marqued_area(ta, clear, draw);
    if (draw) {
        show_tcursor(ta);
        GrResetClipBox();
    }
    return ret;
}

/**************************************************************************/

void GUIRTEPasteMA(GUIRawTEdit *ta, int draw)
{
    if (ta == NULL) return;

    if (draw) {
        GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
        hide_tcursor(ta);
    }
    paste_marqued_area(ta, draw);
    if (draw) {
        show_tcursor(ta);
        GrResetClipBox();
    }
}

/**************************************************************************/

void GUIRTEReDraw(GUIRawTEdit *ta)
{
    if (ta == NULL) return;

    GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
    hide_tcursor(ta);
    //GrClearContext(ta->bgcolor);
    redraw(ta);
    show_tcursor(ta);
    GrResetClipBox();
}

/**************************************************************************/

void GUIRTEGetStatus(GUIRawTEdit *ta, GUITEditStatus *tast)
{
    if (ta == NULL) return;
    
    tast->nlines = ta->nlines;
    tast->numchars = ta->numchars;
    tast->tclpos = ta->tclpos;
    tast->tccpos = ta->tccpos;
    tast->ncscr = (ta->width) / ta->xbits;
    tast->fmline = ta->fmline;
    tast->fmch = ta->fmch;
    tast->lmline = ta->lmline;
    tast->lmch = ta->lmch;
    tast->full = ta->full;
    tast->partloaded = ta->partloaded;
}

/**************************************************************************/

void *GUIRTEGetString(GUIRawTEdit *ta, int nline, int chrtype)
{
    return GUIRTEGetMultiString(ta, nline, nline, chrtype);
}

/**************************************************************************/

void *GUIRTEGetMultiString(GUIRawTEdit *ta, int fline, int lline, int chrtype)
{
    unsigned char *s;
    unsigned short *h;
    long laux;
    unsigned char *cutf8;
    int i, j, k, len;
    
    if (ta == NULL) return NULL;
    if (fline < 0 || lline >= ta->nlines) return NULL;
    if (fline > lline) return NULL;

    len = 0;
    for (k=fline; k<=lline; k++) {
        len += ta->l[k].len;
    }
    len += lline + 1 - fline; // for \n

    switch (chrtype) {
    case GR_BYTE_TEXT:
    case GR_CP437_TEXT:
    case GR_CP850_TEXT:
    case GR_CP1251_TEXT:
    case GR_CP1252_TEXT:
    case GR_CP1253_TEXT:
    case GR_ISO_8859_1_TEXT:
        s = malloc(len+1);
        if (s == NULL) return NULL;
        j = 0;
        for (k=fline; k<=lline; k++) {
            for (i=0; i<ta->l[k].len; i++)
                s[j++] = GrCharRecodeFromUCS2(ta->l[k].cl[i].u, chrtype);
            if (k < lline) s[j++] = '\n';
        }
        s[j] = '\0';
        return s;
    case GR_UTF8_TEXT:
        for (k=fline; k<=lline; k++) { // extend for multibyte chars
            for (i=0; i<ta->l[k].len; i++) {
                if (ta->l[k].cl[i].u >= 0x80) {
                    len++;
                    if (ta->l[k].cl[i].u >= 0x800) {
                        len++;
                        if (ta->l[k].cl[i].u >= 0x10000)
                            len++;
                    }
                }
            }
        }
        s = malloc((len)+1);
        if (s == NULL) return NULL;
        j = 0;
        cutf8 = (unsigned char *)&laux;
        for (k=fline; k<=lline; k++) {
            for (i=0; i<ta->l[k].len; i++) {
                laux = GrCharRecodeFromUCS2(ta->l[k].cl[i].u, chrtype);
                s[j++] = cutf8[0];
                if (cutf8[1] != '\0') s[j++] = cutf8[1];
                if (cutf8[2] != '\0') s[j++] = cutf8[2];
                if (cutf8[3] != '\0') s[j++] = cutf8[3];
            }
            if (k < lline) s[j++] = '\n';
        }
        s[j] = '\0';
        return s;
    case GR_WORD_TEXT:
    case GR_UCS2_TEXT:
        h = malloc((len+1)*sizeof(unsigned short));
        if (h == NULL) return NULL;
        j = 0;
        for (k=fline; k<=lline; k++) {
            for (i=0; i<ta->l[k].len; i++)
                h[j++] = ta->l[k].cl[i].u;
            if (k < lline) h[j++] = '\n';
        }
        h[j] = '\0';
        return h;
    }

    return NULL;
}

/**************************************************************************/

int GUIRTEGetChanged(GUIRawTEdit *ta)
{
    return ta->changed;
}

/**************************************************************************/

void GUIRTEResetChanged(GUIRawTEdit *ta)
{
    ta->changed = 0;
}

/**************************************************************************/

int GUIRTEProcessEvent(GUIRawTEdit *ta, GrEvent *ev)
{
    int ret;

    GrSetClipBox(ta->xpos, ta->ypos, ta->xpos+ta->width-1, ta->ypos+ta->height-1);
    ret = process_event(ta, ev);
    GrResetClipBox();

    return ret;
}

/**************************************************************************/

static int process_event(GUIRawTEdit *ta, GrEvent *ev)
{
    int l, c, range;

    if (ev->type == GREV_MOUSE) {
        if (ev->p1 == GRMOUSE_LB_PRESSED) {
            if (calc_line_col_click(ta, ev->p2, ev->p3, &l, &c, 1)){
                ta->pressed = 1;
                move_tcursor(ta, l, c, GUI_RESETMARQUEDAREA);
                ta->tcclast = ta->tccpos;
                scroll_toshow_tc(ta, 1);
            }
            return 1;
        } else if (ev->p1 == GRMOUSE_B4_RELEASED) {
            scroll_nlines_up(ta, 3);
            return 1;
        } else if (ev->p1 == GRMOUSE_B5_RELEASED) {
            scroll_nlines_down(ta, 3);
            return 1;
        } else if (ev->p1 == GRMOUSE_LB_RELEASED) {
            ta->pressed = 0;
            return 1;
        } else if (ev->p1 == GRMOUSE_B6_RELEASED) {
            scroll_xorg(ta, ta->xorg-5);
            return 1;
        } else if (ev->p1 == GRMOUSE_B7_RELEASED) {
            scroll_xorg(ta, ta->xorg+5);
            return 1;
        }
    } else if (ev->type == GREV_MMOVE) {
        if ((ev->p1 == GRMOUSE_LB_PRESSED) && ta->pressed) {
            if (calc_line_col_click(ta, ev->p2, ev->p3, &l, &c, 0)){
                move_tcursor(ta, l, c, GUI_EXPANDMARQUEDAREA);
                ta->tcclast = ta->tccpos;
                scroll_toshow_tc(ta, 1);
            }
            return 1;
        }
    } else if (ev->type == GREV_SCBVERT && ta->vscb &&
               ta->vscb->aid == ev->p2) {
        range = ta->flscr - ev->p1;
        if (range > 0) scroll_nlines_up(ta, range);
        if (range < 0) scroll_nlines_down(ta, -range);
        return 1;
    } else if (ev->type == GREV_SCBHORZ && ta->hscb &&
               ta->hscb->aid == ev->p2) {
        scroll_xorg(ta, ev->p1);
        return 1;
    } else if ((ev->type == GREV_KEY) && (ev->p2 != GRKEY_KEYCODE)) {
        if (ev->p1 == GrKey_Control_C) {
            copy_marqued_area(ta, 0, 0);
            return 1;
        } else if (!ta->editable) { // if no editable don't do more
            return 1;
        } else if (ev->p1 == GrKey_Control_V) {
            paste_marqued_area(ta, 1);
            scroll_toshow_tc(ta, 1);
        } else if (ev->p1 == GrKey_Control_X) {
            copy_marqued_area(ta, 1, 1);
            scroll_toshow_tc(ta, 1);
        } else {
            print_char(ta, ev->p1, GrGetChrtypeForUserEncoding(), 1);
            scroll_toshow_tc(ta, 1);
        }
        return 1;
    } else if ((ev->type == GREV_KEY) && (ev->p2 == GRKEY_KEYCODE)) {
        if (ev->p1 == GrKey_Delete && ta->editable) {
            print_char(ta, 0x7f, GrGetChrtypeForUserEncoding(), 1);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Left) {
            move_tcursor_rel(ta, 0, -1, GUI_RESETMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Right) {
            move_tcursor_rel(ta, 0, 1, GUI_RESETMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Up) {
            move_tcursor_rel(ta, -1, 0, GUI_RESETMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Down) {
            move_tcursor_rel(ta, 1, 0, GUI_RESETMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Home) {
            move_tcursor_rel(ta, 0, -ta->tccpos, GUI_RESETMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_End) {
            move_tcursor_rel(ta, 0, ta->l[ta->tclpos].len - ta->tccpos, GUI_RESETMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Control_Home) {
            move_tcursor_rel(ta, -ta->tclpos, -ta->tccpos, GUI_RESETMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Control_End) {
            move_tcursor_rel(ta, ta->nlines - ta->tclpos - 1,
                             ta->l[ta->nlines-1].len - ta->tccpos, GUI_RESETMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_PageUp) {
            move_tcursor_rel(ta, -(ta->nlscr-1), 0, GUI_RESETMARQUEDAREA);
            scroll_nlines_up(ta, ta->nlscr-1);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_PageDown) {
            move_tcursor_rel(ta, ta->nlscr-1, 0, GUI_RESETMARQUEDAREA);
            scroll_nlines_down(ta, ta->nlscr-1);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Shift_Left) {
            move_tcursor_rel(ta, 0, -1, GUI_EXPANDMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Shift_Right) {
            move_tcursor_rel(ta, 0, 1, GUI_EXPANDMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Shift_Up) {
            move_tcursor_rel(ta, -1, 0, GUI_EXPANDMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Shift_Down) {
            move_tcursor_rel(ta, 1, 0, GUI_EXPANDMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Shift_Home) {
            move_tcursor_rel(ta, 0, -ta->tccpos, GUI_EXPANDMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Shift_End) {
            move_tcursor_rel(ta, 0, ta->l[ta->tclpos].len - ta->tccpos, GUI_EXPANDMARQUEDAREA);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Shift_PageUp) {
            move_tcursor_rel(ta, -(ta->nlscr-1), 0, GUI_EXPANDMARQUEDAREA);
            scroll_nlines_up(ta, ta->nlscr-1);
            scroll_toshow_tc(ta, 1);
            return 1;
        } else if (ev->p1 == GrKey_Shift_PageDown) {
            move_tcursor_rel(ta, ta->nlscr-1, 0, GUI_EXPANDMARQUEDAREA);
            scroll_nlines_down(ta, ta->nlscr-1);
            scroll_toshow_tc(ta, 1);
            return 1;
        }
    } else if ((ev->type == GREV_CBREPLY) && (ev->p1 == 1)) {
        paste_marqued_area(ta, 1);
        scroll_toshow_tc(ta, 1);
        return 1;
    } else if ((ev->type == GREV_NULL) && (ta->tcstatus >= 0)) {
        if (ta->tctime == 0) {
            ta->tctime = ev->time;
        } else {
            if (ev->time > ta->tctime + BLINK_TIME) {
              ta->tctime = ev->time;
              ta->tcblink ^= 1;
              draw_tcursor(ta, ta->tcblink);
              //blink_tcursor(ta);
            }
        }
    }
    
    return 0;
}

/**************************************************************************/
/* internal functions, assume ta != NULL and clipbox set                  */
/**************************************************************************/

static void move_tcursor(GUIRawTEdit *ta, int nline, int nchar, int setmark)
{
    if (nline < 0) nline = 0;
    if (nline >= ta->nlines) nline = ta->nlines - 1;
    if (nchar < 0) nchar = 0;
    if (nchar > ta->l[nline].len) nchar = ta->l[nline].len;

    if (ta->tclpos == nline && ta->tccpos == nchar) {
        if (setmark == GUI_RESETMARQUEDAREA) reset_marqued_area(ta, 1);
    } else {
        hide_tcursor(ta);
        ta->tclpos = nline;
        ta->tccpos = nchar;
        if (setmark == GUI_RESETMARQUEDAREA) reset_marqued_area(ta, 1);
        if (setmark == GUI_EXPANDMARQUEDAREA) expand_marqued_area(ta, 1);
        show_tcursor(ta);
    }
}

/**************************************************************************/

static void move_tcursor_nodraw(GUIRawTEdit *ta, int nline, int nchar, int setmark)
{

    if (nline < 0) nline = 0;
    if (nline >= ta->nlines) nline = ta->nlines - 1;
    if (nchar < 0) nchar = 0;
    if (nchar > ta->l[nline].len) nchar = ta->l[nline].len;

    if (ta->tclpos == nline && ta->tccpos == nchar) {
        if (setmark == GUI_RESETMARQUEDAREA) reset_marqued_area(ta, 0);
    } else {
        ta->tclpos = nline;
        ta->tccpos = nchar;
        if (setmark == GUI_RESETMARQUEDAREA) reset_marqued_area(ta, 0);
        if (setmark == GUI_EXPANDMARQUEDAREA) expand_marqued_area(ta, 0);
    }
}

/**************************************************************************/

static void move_tcursor_rel(GUIRawTEdit *ta, int incrl, int incrc, int setmark)
{
    int nline, nchar;
    int save_tcclast = 0;
    
    nline = ta->tclpos + incrl;
    nchar = ta->tccpos + incrc;

    if (incrl == 0) {
        if ((nchar < 0) && (nline > 0)) {
            nline -= 1;
            nchar = ta->l[nline].len;
        }
        if ((nchar > ta->l[nline].len) && (nline < ta->nlines-1)) {
            nline += 1;
            nchar = 0;
        }
    }
    if (incrc == 0) {
        nchar = ta->tcclast;
    } else {
        save_tcclast = 1;
    }

    move_tcursor(ta, nline, nchar, setmark);

    if (save_tcclast) ta->tcclast = ta->tccpos;
}

/**************************************************************************/

static void show_tcursor(GUIRawTEdit *ta)
{
    ta->tcstatus++;
        
    if (ta->tcstatus == 0) {
        draw_tcursor(ta, 1);
   }
}

/**************************************************************************/

static void hide_tcursor(GUIRawTEdit *ta)
{
    ta->tcstatus--;
        
    if (ta->tcstatus == -1) {
        draw_tcursor(ta, 0);
    }
}

/**************************************************************************/

static void draw_tcursor(GUIRawTEdit *ta, int show)
{
    RTELine *l;
    int xpos_org, xpos, ypos;

    if (ta->tclpos < ta->flscr) return;
    l = &(ta->l[ta->tclpos]);
    ypos = l->y;
    if (ypos >= ta->height) return;
    xpos_org = (ta->tccpos >= l->len) ? l->nextx : l->cl[ta->tccpos].x;
    xpos = xpos_org - ta->xorg;
    if ((xpos < 0) || (xpos >= ta->width)) return;

    xpos += ta->xpos;
    ypos += ta->ypos;
    if (show) {
        GrVLine(xpos, ypos, ypos-ta->ybits+1, ta->ctbl[ta->tcicolor]);
        GrVLine(xpos+1, ypos, ypos-ta->ybits+1, ta->ctbl[ta->tcicolor]);
        GUIDBCurCtxBltRectToScreen(xpos, ypos-ta->ybits+1, xpos+1, ypos);
        ta->tctime = 0; // reset blink
    } else if (ta->tccpos >= l->len) {
        GrVLine(xpos, ypos, ypos-ta->ybits+1, ta->ctbl[ta->bgicolor]);
        GrVLine(xpos+1, ypos, ypos-ta->ybits+1, ta->ctbl[ta->bgicolor]);
        GUIDBCurCtxBltRectToScreen(xpos, ypos-ta->ybits+1, xpos+1, ypos);
    } else {
        redraw_char(ta, ta->tclpos, ta->tccpos, 1);
    }
}

/**************************************************************************/

static void redraw(GUIRawTEdit *ta)
{
    int i, cl;
    
    GrHLine(ta->xpos, ta->xpos+ta->width-1, ta->ypos, ta->ctbl[ta->bgicolor]);

    cl = 0;
    for (i=ta->flscr; i<ta->nlines; i++) {
        redraw_line_from(ta, i, 0, 0);
        cl++;
        if (ta->l[i].y >= ta->height) break;
    }
    
    i = ta->nlscr + 1 - cl;
    if (i > 0)
        GrFilledBox(ta->xpos, ta->ypos+(cl*ta->ybits)+1,
                    ta->xpos+ta->width-1, ta->ypos+(cl+i)*ta->ybits,
                    ta->ctbl[ta->bgicolor]);
    
    GUIDBCurCtxBltRectToScreen(ta->xpos, ta->ypos, ta->xpos+ta->width-1,
                               ta->ypos+ta->height-1);
    redraw_scbs(ta);
}

/**************************************************************************/

static void redraw_line_from(GUIRawTEdit *ta, int line, int col, int blit)
{
    int j, x1;
    
    if (ta->l[line].y >= (ta->height+ta->ybits)) return;
    for (j=col; j<ta->l[line].len; j++) {
        redraw_char(ta, line, j, 0);
    }
    x1 = ta->l[line].nextx - ta->xorg;
    if (x1 < 0) x1 = 0;
    if (x1 < ta->width) {
        GrFilledBox(ta->xpos+x1, ta->ypos+ta->l[line].y-ta->ybits+1,
                    ta->xpos+ta->width-1, ta->ypos+ta->l[line].y,
                    ta->ctbl[ta->bgicolor]);
    }
    x1 = (col >= ta->l[line].len) ? ta->l[line].nextx - ta->xorg : 
                                  ta->l[line].cl[col].x - ta->xorg;
    if (x1 < 0) x1 = 0;
    if (x1 < ta->width && blit)
        GUIDBCurCtxBltRectToScreen(ta->xpos+x1, ta->ypos+ta->l[line].y-ta->ybits+1,
                                   ta->xpos+ta->width-1, ta->ypos+ta->l[line].y);
}

/**************************************************************************/

static void redraw_char(GUIRawTEdit *ta, int line, int col, int blit)
{
    GrTextOption grt2;
    int x1, w;
    
    if (ta->l[line].y > (GrMaxY()+ta->ybits)) return;
    if (col >= ta->l[line].len) return;
    grt2 = ta->grt;
    if (in_marqued_area(ta, line, col)) {
        grt2.txo_fgcolor = ta->ctbl[ta->bgicolor];
        grt2.txo_bgcolor = ta->ctbl[ta->l[line].cl[col].fg];
    } else {
        grt2.txo_fgcolor = ta->ctbl[ta->l[line].cl[col].fg];
        grt2.txo_bgcolor = ta->ctbl[ta->bgicolor];
    }
    x1 = ta->l[line].cl[col].x-ta->xorg;
    GrDrawChar(ta->l[line].cl[col].c, ta->xpos+x1, ta->ypos+ta->l[line].y, &grt2);

    if (blit) {
        w = (col >= ta->l[line].len-1) ? ta->l[line].nextx : ta->l[line].cl[col+1].x;
        w -= ta->l[line].cl[col].x;
        GUIDBCurCtxBltRectToScreen(ta->xpos+x1, ta->ypos+ta->l[line].y-ta->ybits+1,
                                   ta->xpos+x1+w-1, ta->ypos+ta->l[line].y);
    }
                                 
}

/**************************************************************************/

static void scroll_up(GUIRawTEdit *ta)
{
    int i;
    
    if (ta->flscr == 0) return;
    ta->flscr -= 1;
    for (i=0; i<ta->nlines; i++) {
        ta->l[i].y += ta->ybits;
    }
}

/**************************************************************************/

static void scroll_down(GUIRawTEdit *ta)
{
    int i;
    
    ta->flscr += 1;
    for (i=0; i<ta->nlines; i++) {
        ta->l[i].y -= ta->ybits;
    }
}

/**************************************************************************/

static void scroll_toshow_tc(GUIRawTEdit *ta, int draw)
{
    RTELine *l;
    int xpos_org, xpos;
    int need_redraw = 0;

    l = &(ta->l[ta->tclpos]);

    while (l->y > ta->height-1) {
        scroll_down(ta);
        need_redraw = 1;
    }
    while (l->y < ta->ybits) {
        scroll_up(ta);
        need_redraw = 1;
    }

    xpos_org = (ta->tccpos >= l->len) ? l->nextx : l->cl[ta->tccpos].x;
    xpos = xpos_org - ta->xorg;
    while (xpos < 0) {
        ta->xorg -= ta->xbits;
        if (ta->xorg < 0) ta->xorg = 0;
        need_redraw = 1;
        xpos = xpos_org - ta->xorg;
    }
    while (xpos > ta->width-1) {
        ta->xorg += ta->xbits;
        need_redraw = 1;
        xpos = xpos_org - ta->xorg;
    }

    if (draw && need_redraw) {
        hide_tcursor(ta);
        redraw(ta);
        show_tcursor(ta);
    }
}

/**************************************************************************/

static void scroll_nlines_up(GUIRawTEdit *ta, int nlines)
{
    int i;

    if (ta->flscr - nlines < 0) nlines = ta->flscr;
    
    if (nlines <= 0) return;
    
    ta->flscr -= nlines;
    for (i=0; i<ta->nlines; i++) {
        ta->l[i].y += ta->ybits * nlines;
    }
            
    hide_tcursor(ta);
    redraw(ta);
    show_tcursor(ta);
}

/**************************************************************************/

static void scroll_nlines_down(GUIRawTEdit *ta, int nlines)
{
    int i, llscr;

    llscr = ta->flscr + ta->nlscr - 1;
    if (llscr + nlines >= ta->nlines) {
        nlines = ta->nlines - 1 - llscr;
    }
    
    if (nlines <= 0) return;
    
    ta->flscr += nlines;
    for (i=0; i<ta->nlines; i++) {
        ta->l[i].y -= ta->ybits * nlines;
    }
            
    hide_tcursor(ta);
    redraw(ta);
    show_tcursor(ta);
}

/**************************************************************************/

static void scroll_xorg(GUIRawTEdit *ta, int newxorg)
{
    if (newxorg < 0) newxorg = 0;
    if (ta->xorg != newxorg) {
        ta->xorg = newxorg;
        hide_tcursor(ta);
        redraw(ta);
        show_tcursor(ta);
    }
}

/**************************************************************************/

static int calc_line_col_click(GUIRawTEdit *ta, int x, int y,
                               int *l, int *c, int restrictxy)
{
    RTELine *tali;
    int i, xnext, w;

    x -= ta->xpos;
    y -= ta->ypos;
    
    if (restrictxy && (x < 0 || x > ta->width-1)) return 0;
    if (restrictxy && (y < 0 || y > ta->height-1)) return 0;
    
    *l = y / ta->ybits;
    if (*l > ta->nlscr) *l = ta->nlscr;
    *l += ta->flscr;
    if (*l >= ta->nlines) *l = ta->nlines - 1;
    if (*l < 0) *l = 0;
    
    tali = &(ta->l[*l]);
    *c = 0;
    x += ta->xorg;
    if (x >= tali->nextx)
        *c = tali->len;
    else {
        xnext = tali->nextx;
        for (i=tali->len-1; i>=0; i--) {
            w = xnext - tali->cl[i].x;
            if (x > tali->cl[i].x+w/2) {
                *c = i+1;
                break;
            }
            xnext = tali->cl[i].x;
        }
    }
    
    return 1;
}

/**************************************************************************/

static int calc_maxx_in_screen(GUIRawTEdit *ta)
{
    int i, maxx;

    maxx = 0;

    for (i=0; i<ta->nlscr; i++) {
        if ((i+ta->flscr) > ta->nlines) break;
        if (ta->l[i+ta->flscr].nextx > maxx) maxx = ta->l[i+ta->flscr].nextx;
    }

    return maxx;
}

/**************************************************************************/

static void redraw_scbs(GUIRawTEdit *ta)
{
    GrContext grcaux;
    int max;

    if (ta->vscb == NULL && ta->hscb == NULL) return;

    if (ta->vscb) {
        max = ta->nlines;
        if (max < ta->nlscr) max = ta->nlscr;
        GUIScrollbarSetLimits(ta->vscb, 0, max,
                              ta->flscr, ta->flscr+ta->nlscr);
    }

    if (ta->hscb) {
        max = calc_maxx_in_screen(ta);
        if (max < ta->width-1) max = ta->width-1;
        GUIScrollbarSetLimits(ta->hscb, 0, max,
                              ta->xorg, ta->xorg+ta->width-1);
    }

    GrSaveContext(&grcaux);
    if (ta->scbctx) {
        GrSetContext(ta->scbctx);
    } else {
        GrResetClipBox();
    }

    if (ta->vscb != NULL) {
        GUIScrollbarPaint(ta->vscb, ta->scbdx, ta->scbdy, 1);
    }
    if (ta->hscb != NULL) {
        GUIScrollbarPaint(ta->hscb, ta->scbdx, ta->scbdy, 1);
    }

    GrSetContext(&grcaux);
}

/**************************************************************************/
/* edit function from the text cursor position,                           */
/* they assume the clipbox is set and the tex cursor hidden               */
/* excepting print_char                                                   */
/**************************************************************************/

static void print_char(GUIRawTEdit *ta, long ch, int chrtype, int draw)
{
    if (draw) hide_tcursor(ta);

    if (ch == '\r') {
        if (!ta->full)
            new_line(ta, draw);
    } else if (ch == '\b') {
        delete_char(ta, draw);
    } else if (ch == 0x7f) {
        erase_char(ta, draw);
    } else {
        if (!ta->full)
            add_char(ta, ch, chrtype, draw);
    }

    if (draw) show_tcursor(ta);
}

/**************************************************************************/

static void new_line(GUIRawTEdit *ta, int draw)
{
    delete_marqued_area(ta, draw);

    if (!nd_new_line(ta, ta->tclpos, ta->tccpos)) return;

    ta->tclpos += 1;
    ta->tccpos = 0;
    ta->tcclast = ta->tccpos;
    reset_marqued_area(ta, 0);
    if (ta->l[ta->tclpos].y >= ta->height) scroll_down(ta);
    if (draw) redraw(ta);
}

/**************************************************************************/

static void add_char(GUIRawTEdit *ta, long ch, int chrtype, int draw) 
{
    delete_marqued_area(ta, draw);

    if (!nd_add_char(ta, ta->tclpos, ta->tccpos, ch, chrtype)) return;

    ta->tccpos += 1;
    ta->tcclast = ta->tccpos;
    reset_marqued_area(ta, 0);
    if (draw) redraw_line_from(ta, ta->tclpos, ta->tccpos-1, 1);
}

/**************************************************************************/

static void add_string(GUIRawTEdit *ta, void *s, int len, int chrtype, int draw)
{
    unsigned short *sword = (unsigned short *)s;
    unsigned char *sbyte = (unsigned char *)s;
    int i, ind, nb;
    int chwide = 1;
    long ch;

    if (len <= 0) len = GrStrLen(s, chrtype);

    if (chrtype == GR_UCS2_TEXT || chrtype == GR_WORD_TEXT) chwide = 2;
    else if (chrtype == GR_UTF8_TEXT) chwide = 0; // variable wide

    delete_marqued_area(ta, draw);

    ind = 0;
    for (i=0; i<len; i++) {
        if (chwide == 0) {
            ch = GrNextUTF8Char(s, &nb);
            s += nb;
        } else {
            ch = (chwide == 1) ? sbyte[i] : sword[i];
        }
        if (!nd_add_char(ta, ta->tclpos, ta->tccpos+ind, ch, chrtype))
            break;
        ind++;
    }

    ta->tccpos += ind;
    ta->tcclast = ta->tccpos;
    reset_marqued_area(ta, 0);
    if (draw) redraw_line_from(ta, ta->tclpos, ta->tccpos-ind, 1);
}

/**************************************************************************/

static void add_multistring(GUIRawTEdit *ta, void *s, int len, int chrtype, int draw)
{
    unsigned short *sword = (unsigned short *)s;
    unsigned char *sbyte = (unsigned char *)s;
    int i, ind, nb, needredraw = 0;
    int chwide = 1;
    long ch;

    if (len <= 0) len = GrStrLen(s, chrtype);

    if (chrtype == GR_UCS2_TEXT || chrtype == GR_WORD_TEXT) chwide = 2;
    else if (chrtype == GR_UTF8_TEXT) chwide = 0; // variable wide

    delete_marqued_area(ta, draw);

    ind = 0;
    for (i=0; i<len; i++) {
        if (chwide == 0) {
            ch = GrNextUTF8Char(s, &nb);
            s += nb;
        } else {
            ch = (chwide == 1) ? sbyte[i] : sword[i];
        }
        if (ch == '\n') {
            if (ind > 0) {
                ta->tccpos += ind;
                ind = 0;
            }
            if (!nd_new_line(ta, ta->tclpos, ta->tccpos)) break;
            ta->tclpos += 1;
            ta->tccpos = 0;
            if (ta->l[ta->tclpos].y >= ta->height) scroll_down(ta);
            needredraw = 1;
        } else {
            if (!nd_add_char(ta, ta->tclpos, ta->tccpos+ind, ch, chrtype))
                break;
            ind++;
        }
    }

    if (draw && needredraw) redraw(ta);

    if (ind > 0) {
        if (draw && !needredraw) redraw_line_from(ta, ta->tclpos, ta->tccpos, 1);
        ta->tccpos += ind;
    }
    ta->tcclast = ta->tccpos;
    reset_marqued_area(ta, 0);
}

/**************************************************************************/

static void join_lines(GUIRawTEdit *ta, int draw)
{
    // used only by delete_char and erase_char
    // so do not need delete_marqued_area by now
    int olen1;
    
    if (ta->tclpos < 1) return;
    olen1 = ta->l[ta->tclpos-1].len;
    
    if (!nd_join_lines(ta, ta->tclpos)) return;

    ta->tclpos -= 1;
    ta->tccpos = olen1;
    ta->tcclast = ta->tccpos;
    if (ta->l[ta->tclpos].y < ta->ybits) scroll_up(ta);
    reset_marqued_area(ta, 0);
    if (draw) redraw(ta);
}

/**************************************************************************/

static void delete_char(GUIRawTEdit *ta, int draw)
{
    if (delete_marqued_area(ta, draw)) return;

    if (ta->tccpos < 1) {
        join_lines(ta, draw);
        return;
    }

    ta->tccpos -= 1;
    nd_del_char(ta, ta->tclpos, ta->tccpos);

    ta->tcclast = ta->tccpos;
    reset_marqued_area(ta, 0);
    if (draw) redraw_line_from(ta, ta->tclpos, ta->tccpos, 1);
}

/**************************************************************************/

static void erase_char(GUIRawTEdit *ta, int draw)
{
    if (delete_marqued_area(ta, draw)) return;

    if (ta->tccpos >= ta->l[ta->tclpos].len) {
        if (ta->tclpos < ta->nlines-1) {
            ta->tclpos += 1;
            ta->tccpos = 0;
            join_lines(ta, draw);
        }
        return;
    }

    nd_del_char(ta, ta->tclpos, ta->tccpos);

    ta->tcclast = ta->tccpos;
    reset_marqued_area(ta, 0);
    if (draw) redraw_line_from(ta, ta->tclpos, ta->tccpos, 1);
}

/**************************************************************************/
/* marqued area functions                                                 */
/**************************************************************************/

static int in_marqued_area(GUIRawTEdit *ta, int line, int col)
{
    if ((line > ta->fmline) && (line < ta->lmline)) return 1;
    if ((line == ta->fmline) && (col < ta->fmch)) return 0;
    if ((line == ta->lmline) && (col >= ta->lmch)) return 0;
    if ((line == ta->fmline) || (line == ta->lmline)) return 1;
    return 0;
}

/**************************************************************************/

static void reset_marqued_area(GUIRawTEdit *ta, int draw)
{
    int fmline, lmline, fmch, lmch;
    int i;
    
    fmline = ta->fmline;
    lmline = ta->lmline;
    fmch = ta->fmch;
    lmch = ta->lmch;

    ta->fmline = ta->lmline = ta->pvmline = ta->tclpos;
    ta->fmch = ta->lmch = ta->pvmch = ta->tccpos;

    if (draw) {
        if ((fmline == lmline) && (fmch == lmch)) return;
        hide_tcursor(ta);
        for (i=fmline; i<=lmline; i++) {
            if (i >= ta->flscr) redraw_line_from(ta, i, 0, 1);
        }
        show_tcursor(ta);
    }
}

/**************************************************************************/

static void set_marqued_area(GUIRawTEdit *ta, int fl, int fc, int ll, int lc, int draw)
{
    if (fl < 0) { // fl < 0 set all
        fl = 0;
        fc = 0;
        ll = ta->nlines - 1;
        lc = ta->l[ll].len;
    } else {
        if (ll < fl) ll = fl;
        if (ll >= ta->nlines) ll = ta->nlines - 1;
        if (fl > ll) fl = ll;
        if (fc < 0) fc = 0;
        if (fc > ta->l[fl].len) fc = ta->l[fl].len;
        if (lc < 0) lc = 0;
        if (lc > ta->l[ll].len) lc = ta->l[ll].len;
        if (fl == ll && lc < fc) lc = fc;
    }

    if (draw) hide_tcursor(ta);
    ta->fmline = ta->pvmline = fl;
    ta->fmch = ta->pvmch = fc;
    ta->lmline = ta->tclpos = ll;
    ta->lmch = ta->tccpos = lc;
    scroll_toshow_tc(ta, 0);
    if (draw) {
        redraw(ta);
        show_tcursor(ta);
    }
}

/**************************************************************************/

static void expand_marqued_area(GUIRawTEdit *ta, int draw)
{
    int fmline, lmline;
    int minline, maxline;
    int left, i, check, into;
    
    fmline = ta->fmline;
    lmline = ta->lmline;

    left = 0;
    if (ta->tclpos < ta->pvmline) left = 1;
    if ((ta->tclpos == ta->pvmline) && (ta->tccpos < ta->pvmch)) left = 1;
    if (left) {
        ta->fmline = ta->tclpos;
        ta->fmch = ta->tccpos;
        ta->lmline = ta->pvmline;
        ta->lmch = ta->pvmch;
    } else {
        ta->fmline = ta->pvmline;
        ta->fmch = ta->pvmch;
        ta->lmline = ta->tclpos;
        ta->lmch = ta->tccpos;
    }

    if (draw) {
        minline = (fmline < ta->fmline) ? fmline : ta->fmline;
        maxline = (lmline > ta->lmline) ? lmline : ta->lmline;
        //printf("mark %d %d\n", minline, maxline);
        hide_tcursor(ta);
        for (i=minline; i<=maxline; i++) {
            if (i < ta->flscr) continue;
            if (ta->l[i].y >= (ta->height+ta->ybits)) break;
            check = 0;
            if (i==fmline || i==lmline || i==ta->fmline || i==ta->lmline) check = 1;
            if (!check) {
                into = 0;
                if (i>fmline && i<lmline) into +=1;
                if (i>ta->fmline && i<ta->lmline) into +=1;
                if (into == 1) check = 1;
            }
            if (check) redraw_line_from(ta, i, 0, 1);
        }
        show_tcursor(ta);
    }
}

/**************************************************************************/

static int delete_marqued_area(GUIRawTEdit *ta, int draw)
{
    int i;

    if ((ta->fmline == ta->lmline) && (ta->fmch == ta->lmch)) return 0;

    if (draw) hide_tcursor(ta);

    for (i=0; i<(ta->lmline - ta->fmline - 1); i++) {
        nd_del_line(ta, ta->fmline+1);
    }
    if (ta->lmline != ta->fmline) {
        nd_del_char_from(ta, ta->fmline, ta->fmch);
        nd_del_char_to(ta, ta->fmline+1, ta->lmch);
        nd_join_lines(ta, ta->fmline+1);
    } else {
        nd_del_char_from_to(ta, ta->fmline, ta->fmch, ta->lmch);
    }
    
    ta->tclpos = ta->fmline;
    ta->tccpos = ta->fmch;
    ta->tcclast = ta->tccpos;
    reset_marqued_area(ta, 0);

    if (draw) {
        while (ta->l[ta->tclpos].y >= ta->height) scroll_down(ta);
        while (ta->l[ta->tclpos].y < ta->ybits) scroll_up(ta);
        redraw(ta);
        show_tcursor(ta);
    }

    return 1;
}

/**************************************************************************/

static int copy_marqued_area(GUIRawTEdit *ta, int clear, int draw)
{
    unsigned short *buf;
    int len, rlen;
    
    if ((ta->fmline == ta->lmline) && (ta->fmch == ta->lmch)) return 0;

    buf = nd_get_ucs2text(ta, ta->fmline, ta->fmch,
                         ta->lmline, ta->lmch, &len);
    if (buf == NULL) return 0;

    rlen = GrPutClipBoard(buf, len, GR_UCS2_TEXT);
    free(buf);
    if (rlen != len) return 0;

    if (clear) delete_marqued_area(ta, draw);
    return 1;
}

/**************************************************************************/

static void paste_marqued_area(GUIRawTEdit *ta, int draw)
{
    unsigned short *buf;
    int len, status;

    if (ta->full) return;

    status = GrIsClipBoardReadyToGet();
    if (status != 1) return;

    len = GrGetClipBoardLen(&(ta->partloaded));
    if (len <=0) return;

    buf = GrGetClipBoard(len, GR_UCS2_TEXT);
    if (buf == NULL) return;

    if (draw) hide_tcursor(ta);
    add_multistring(ta, buf, len, GR_UCS2_TEXT, draw);
    if (draw) show_tcursor(ta);

    free(buf);
}

/**************************************************************************/

static int setfg_marqued_area(GUIRawTEdit *ta)
{
    // this function doesn't draw, call redraw after if needed
    int i;

    if ((ta->fmline == ta->lmline) && (ta->fmch == ta->lmch)) return 0;

    if (ta->fmline == ta->lmline) {
        nd_setfg_char_from_to(ta, ta->fmline, ta->fmch, ta->lmch);
    } else {
        nd_setfg_char_from_to(ta, ta->fmline, ta->fmch, ta->l[ta->fmline].len);
        for (i=ta->fmline+1; i<ta->lmline; i++)
            nd_setfg_char_from_to(ta, i, 0, ta->l[i].len);
        nd_setfg_char_from_to(ta, ta->lmline, 0, ta->lmch);
    }

    return 1;
}

/**************************************************************************/
/* nd_ functions do their job without drawing anything and without        */
/* moving the text cursor                                                 */
/**************************************************************************/

static int nd_del_line(GUIRawTEdit *ta, int line)
{
    int i;

    if ((line < 0) || (line >= ta->nlines)) return 0;
    ta->nlines -= 1;
    ta->numchars -= ta->l[line].len;
    free(ta->l[line].cl);
    for (i=line; i<ta->nlines; i++) {
        ta->l[i] = ta->l[i+1];
        ta->l[i].y -= ta->ybits;
    }
    ta->l[ta->nlines].cl = NULL;
    ta->l[ta->nlines].len = 0;
    ta->l[ta->nlines].maxlen = 0;
    ta->l[ta->nlines].nextx = 0;

    ta->full = 0;
    ta->changed = 1;
    return 1;
}

/**************************************************************************/

static int nd_del_char(GUIRawTEdit *ta, int line, int col)
{
    // del a char in a line, doesn't join lines
    int i, x;
    unsigned short newlen;

    if ((line < 0) || (line >= ta->nlines)) return 0;
    if ((col < 0) || (col >= ta->l[line].len)) return 0;

    newlen = ta->l[line].len - 1;
    x = (col == ta->l[line].len-1) ? ta->l[line].nextx : ta->l[line].cl[col+1].x;
    x -= ta->l[line].cl[col].x;
    for (i=col; i<newlen; i++) {
        ta->l[line].cl[i] = ta->l[line].cl[i+1];
        ta->l[line].cl[i].x -= x;
    }
    ta->l[line].len = newlen;
    ta->l[line].nextx -= x;

    ta->numchars--;
    ta->full = 0;
    ta->changed = 1;
    return 1;
}

/**************************************************************************/

static int nd_del_char_from(GUIRawTEdit *ta, int line, int col)
{
    // del from char "col" (included) to the last line char
    int delchars;

    if ((line < 0) || (line >= ta->nlines)) return 0;
    if ((col < 0) || (col >= ta->l[line].len)) return 0;
    delchars = ta->l[line].len - col;
    ta->l[line].len = col;
    ta->l[line].nextx = ta->l[line].cl[col].x;

    ta->numchars -= delchars;
    ta->full = 0;
    ta->changed = 1;
    return 1;
}

/**************************************************************************/

static int nd_del_char_to(GUIRawTEdit *ta, int line, int col)
{
    // del from char "0" to char "col" (excluded, but it can be len line!)
    int i, x, delchars;
    unsigned short newlen;

    if ((line < 0) || (line >= ta->nlines)) return 0;
    if ((col < 1) || (col > ta->l[line].len)) return 0;
    newlen = ta->l[line].len - col;
    delchars = ta->l[line].len - newlen;
    x = (col == ta->l[line].len) ? ta->l[line].nextx : ta->l[line].cl[col].x;
    for (i=0; i<newlen; i++) {
        ta->l[line].cl[i] = ta->l[line].cl[i+col];
        ta->l[line].cl[i].x -= x;
    }
    ta->l[line].len = newlen;
    ta->l[line].nextx -= x;

    ta->numchars -= delchars;
    ta->full = 0;
    ta->changed = 1;
    return 1;
}

/**************************************************************************/

static int nd_del_char_from_to(GUIRawTEdit *ta, int line, int fc, int lc)
{
    // del from char "fc" (included) to char "lc" (excluded, but it can be len line!) 
    int i, x, delchars;
    unsigned short newlen;

    if ((line < 0) || (line >= ta->nlines)) return 0;
    delchars = lc - fc;
    if ((fc < 0) || (delchars < 1)) return 0;
    if (lc > ta->l[line].len) return 0;
    newlen = ta->l[line].len - delchars;
    x = (lc == ta->l[line].len) ? ta->l[line].nextx : ta->l[line].cl[lc].x;
    x -= ta->l[line].cl[fc].x;
    for (i=fc; i<newlen; i++) {
        ta->l[line].cl[i] = ta->l[line].cl[i+delchars];
        ta->l[line].cl[i].x -= x;
    }
    ta->l[line].len = newlen;
    ta->l[line].nextx -= x;

    ta->numchars -= delchars;
    ta->full = 0;
    ta->changed = 1;
    return 1;
}

/**************************************************************************/

static int nd_join_lines(GUIRawTEdit *ta, int line)
{
    // join line-1 with line
    int i;
    unsigned short csize, asize;
    void *ptr;
    RTELine *tali1, *tali2;
    
    if ((line < 1) || (line >= ta->nlines)) return 0;
    
    tali1 = &(ta->l[line-1]);
    tali2 = &(ta->l[line]);
    
    if (tali1->cl == NULL) {
        tali1->cl = malloc(sizeof(RTEChar)*INITIAL_LINE_SIZE);
        if (tali1->cl == NULL) {
            ta->full = 1;
            return 0;
        }
        tali1->maxlen = INITIAL_LINE_SIZE;
    }
    
    csize = tali1->maxlen - tali1->len;
    asize = 0;
    while (csize+asize < tali2->len) asize += INCR_LINE_SIZE;

    if (tali1->maxlen + asize > MAX_LINE_SIZE) {
        ta->full = 1;
        return 0;
    }

    if (asize > 0) {
        ptr = realloc(tali1->cl, sizeof(RTEChar) * (tali1->maxlen + asize));
        if (ptr == NULL) {
            ta->full = 1;
            return 0;
        }
        tali1->cl = ptr;
        tali1->maxlen += asize;
    }

    for (i=0; i<tali2->len; i++) {
        tali1->cl[tali1->len+i] = tali2->cl[i];
        tali1->cl[tali1->len+i].x += tali1->nextx;
    }
    tali1->len += tali2->len;
    tali1->nextx += tali2->nextx;

    ta->numchars += tali2->len; // because nd_del_line will substract it
    nd_del_line(ta, line);
    
    return 1;
}

/**************************************************************************/

static unsigned short * nd_get_ucs2text(GUIRawTEdit *ta, int fline, int fc,
                                        int lline, int lc, int *len)
{
    unsigned short *buf;
    int nchars, i, j, ind;

    if ((fline < 0) || (fline > lline) || (lline >= ta->nlines)) return NULL;
    if ((fline == lline) && (fc >= lc)) return NULL;
    if ((fc < 0) || (fc > ta->l[fline].len)) return NULL;
    if ((lc < 0) || (lc > ta->l[lline].len)) return NULL;

    nchars = 0;
    if (fline == lline) {
        nchars = lc - fc;
    } else {
        nchars = ta->l[fline].len - fc;
        nchars += 1; // for \n
        for (j=fline+1; j<lline; j++) {
            nchars += ta->l[j].len;
            nchars += 1; // for \n
        }
        nchars += lc;
    }
    nchars += 1; // for \0

    buf = (unsigned short *)malloc(nchars*sizeof(unsigned short));
    if (buf == NULL) return NULL;

    ind = 0;
    if (fline == lline) {
        for (i=fc; i<lc; i++) {
            buf[ind++] = ta->l[fline].cl[i].u;
        }
    } else {
        for (i=fc; i<ta->l[fline].len; i++) {
            buf[ind++] = ta->l[fline].cl[i].u;
        }
        buf[ind++] = '\n';
        for (j=fline+1; j<lline; j++) {
            for (i=0; i<ta->l[j].len; i++) {
                buf[ind++] = ta->l[j].cl[i].u;
            }
            buf[ind++] = '\n';
        }
        for (i=0; i<lc; i++) {
            buf[ind++] = ta->l[lline].cl[i].u;
        }
    }
    *len = ind;
    buf[ind] = 0;

    return buf;
}

/**************************************************************************/
/*
static int nd_put_ucs2text(GUIRawTEdit *ta, unsigned short *buf,
                           int len, int line, int col)
{
    int i;

    if ((line < 0) || (line >= ta->nlines)) return 0;
    if ((col < 0) || (col > ta->l[line].len)) return 0;

    for (i=0; i<len; i++) {
        if (buf[i] == 0) return 0;
        if (buf[i] == '\n') {
            if (!nd_new_line(ta, line, col)) return 0;
            line++;
            col = 0;
        } else {
            if (!nd_add_char(ta, line, col, buf[i], GR_UCS2_TEXT)) return 0;
            col++;
        }
    }

    return 1;
}
*/
/**************************************************************************/

static int nd_add_char(GUIRawTEdit *ta, int line, int col, long ch, int chrtype)
{
    void *ptr;
    unsigned short chr, chu;
    RTELine *tali;
    RTEChar *tach;
    int i, w;

    if ((line < 0) || (line >= ta->nlines)) return 0;
    if ((col < 0) || (col > ta->l[line].len)) return 0;

    if (ta->maxchars && ta->numchars+ta->nlines >= ta->maxchars) {
        ta->full = 1;
        return 0;
    }

    tali = &(ta->l[line]);
    if (tali->cl == NULL) {
        tali->cl = malloc(sizeof(RTEChar)*INITIAL_LINE_SIZE);
        if (tali->cl == NULL) {
            ta->full = 1;
            return 0;
        }
        tali->maxlen = INITIAL_LINE_SIZE;
    }
    if (tali->len >= tali->maxlen) {
        if (tali->maxlen + INCR_LINE_SIZE > MAX_LINE_SIZE) {
            ta->full = 1;
            return 0;
        }
        ptr = realloc(tali->cl, sizeof(RTEChar) * (tali->maxlen + INCR_LINE_SIZE));
        if (ptr == NULL) {
            ta->full = 1;
            return 0;
        }
        tali->cl = ptr;
        tali->maxlen += INCR_LINE_SIZE;
    }

    chr = GrFontCharRecode(ta->grt.txo_font, ch, chrtype);
    w = GrFontCharWidth(ta->grt.txo_font, chr);
    // we do this to show glyps if needed using GR_BYTE_TEXT or GR_WORD_TEXT
    // and also retain the real UCS2 char
    if (chrtype == GR_BYTE_TEXT || chrtype == GR_WORD_TEXT)
        chu = GrFontCharReverseRecode(ta->grt.txo_font, chr, GR_UCS2_TEXT);
    else if (chrtype == GR_UCS2_TEXT)
        chu = ch;
    else
        chu = GrCharRecodeToUCS2(ch, chrtype);

    tali->len += 1;

    for (i=tali->len-1; i>col; i--) {
        tali->cl[i] = tali->cl[i-1];
        tali->cl[i].x += w;
    }

    tach = &(tali->cl[col]);
    tach->c = chr;
    tach->u = chu;
    tach->fg = ta->cfgicolor;
    if (col >= tali->len-1) tach->x = tali->nextx;
    
    tali->nextx += w;

    ta->numchars++;
    ta->changed = 1;
    return 1;
}

/**************************************************************************/

static int nd_new_line(GUIRawTEdit *ta, int line, int col)
{
    void *ptr;
    RTEChar *tach = NULL;
    int i, j, x, y, incrnlines, newmaxlines;
    unsigned short ncc, size = 0;

    if ((line < 0) || (line >= ta->nlines)) return 0;
    if ((col < 0) || (col > ta->l[line].len)) return 0;

    if (ta->maxchars && ta->numchars+ta->nlines >= ta->maxchars) {
        ta->full = 1;
        return 0;
    }

    if (ta->nlines >= ta->maxlines) {
        if (ta->maxchars) {
            incrnlines = ta->maxchars/10;
            if (incrnlines < 10) incrnlines = 10;
        } else {
            incrnlines = INCR_NUMLINES;
        }
        newmaxlines = ta->maxlines + incrnlines;
        if (newmaxlines > MAX_NUMLINES) {
            ta->full = 1;
            return 0;
        }
        ptr = realloc(ta->l, sizeof(RTELine) * newmaxlines);
        if (ptr == NULL) {
            ta->full = 1;
            return 0;
        }
        ta->l = ptr;
        y = ta->l[ta->maxlines-1].y + ta->ybits;
        for (i=ta->maxlines; i<newmaxlines; i++) {
            ta->l[i].cl = NULL;
            ta->l[i].len = 0;
            ta->l[i].maxlen = 0;
            ta->l[i].y = y;
            ta->l[i].nextx = 0;
            y += ta->ybits;
        }
        ta->maxlines = newmaxlines;
    }

    ncc = ta->l[line].len - col;
    if (ncc > 0) {
        size = INITIAL_LINE_SIZE;
        while (size < ncc) size += INCR_LINE_SIZE;
        tach = malloc(sizeof(RTEChar)*size);
        if (tach == NULL) {
            ta->full = 1;
            return 0;
        }
    }

    ta->nlines += 1;
    for (i=ta->nlines-1; i>line; i--) {
        ta->l[i] = ta->l[i-1];
        ta->l[i].y += ta->ybits;
    }
    line += 1;
    ta->l[line].cl = NULL;
    ta->l[line].len = 0;
    ta->l[line].maxlen = 0;
    ta->l[line].y = ta->l[line-1].y + ta->ybits;
    ta->l[line].nextx = 0;

    if (ncc > 0) {
        ta->l[line].cl = tach;
        ta->l[line].maxlen = size;
        x = ta->l[line-1].cl[col].x;
        j = col;
        for (i=0; i<ncc; i++) {
            ta->l[line].cl[i] = ta->l[line-1].cl[j];
            ta->l[line].cl[i].x -= x;
            j++;
        }
        ta->l[line].nextx = ta->l[line-1].nextx - x;
        ta->l[line-1].nextx = x;
        ta->l[line].len = ncc;
        ta->l[line-1].len -= ncc;
    }

    ta->changed = 1;
    return 1;
}

/**************************************************************************/

static int nd_setfg_char_from_to(GUIRawTEdit *ta, int line, int fc, int lc)
{
    // set fg color from char "fc" (included) to char "lc" (excluded, but it can be len line!) 
    int i, numchars;

    if ((line < 0) || (line >= ta->nlines)) return 0;
    numchars = lc - fc;
    if ((fc < 0) || (numchars < 1)) return 0;
    if (lc > ta->l[line].len) return 0;
    for (i=fc; i<lc; i++) {
        ta->l[line].cl[i].fg = ta->cfgicolor;
    }
    return 1;
}

