/**
 ** txtpanel.c ---- Mini GUI for MGRX, text editor in a panel
 **
 ** Copyright (C) 2022 Mariano Alvarez Fernandez
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

typedef struct _guiTextPanel {
    GUIPanel *p;           // panel container
    GUIRawTEdit *tedit;    // raw twxt editor
} GUITextPanel;

/**************************************************************************/

GUITextPanel * GUITPCreate(GUIPanel *p, GrFont *f)
{
    GUITextPanel *ta;

    ta = malloc(sizeof(GUITextPanel));
    if (ta == NULL) return NULL;

    ta->p = p;
    
    ta->tedit = GUIRTECreate(f, 1, 0, 0, 0, ta->p->cl->gc_xmax+1,
                             ta->p->cl->gc_ymax+1);
    if (ta->tedit == NULL) {
        free(ta);
        return NULL;
    }

    GUIRTESetScrollbars(ta->tedit, ta->p->vscb, ta->p->hscb, ta->p->gc->c);

    return ta;
}

/**************************************************************************/

void GUITPSetPanel(GUITextPanel *ta, GUIPanel *p)
{
    if (ta == NULL || p == NULL) return;

    ta->p = p;
    GUIRTESetDims(ta->tedit, 0, 0, ta->p->cl->gc_xmax+1, ta->p->cl->gc_ymax+1);
    GUIRTESetScrollbars(ta->tedit, ta->p->vscb, ta->p->hscb, ta->p->gc->c);
}

/**************************************************************************/

void GUITPSetFont(GUITextPanel *ta, GrFont *f)
{
    if (ta == NULL) return;

    GUIRTESetFont(ta->tedit, f);
}

/**************************************************************************/

void GUITPSetSimpleColors(GUITextPanel *ta, GrColor bg, GrColor cfg, GrColor tc)
{
    if (ta == NULL) return;

    GUIRTESetSimpleColors(ta->tedit, bg, cfg, tc);
}

/**************************************************************************/

void GUITPSetTableColors(GUITextPanel *ta, GrColor *ctbl, int numcolors,
                         int bgi, int cfgi, int tci)
{
    if (ta == NULL) return;

    GUIRTESetTableColors(ta->tedit, ctbl, numcolors, bgi, cfgi, tci);
}

/**************************************************************************/

void GUITPTextColorIndex(GUITextPanel *ta, int cfgi)
{
    if (ta == NULL) return;

    GUIRTETextColorIndex(ta->tedit, cfgi);
}

/**************************************************************************/

void GUITPCursorColorIndex(GUITextPanel *ta, int tci)
{
    if (ta == NULL) return;

    GUIRTECursorColorIndex(ta->tedit, tci);
}

/**************************************************************************/

void GUITPDestroy(GUITextPanel *ta)
{
    if (ta == NULL) return;

    GUIRTEDestroy(ta->tedit);
    free(ta);
}

/**************************************************************************/

void GUITPClear(GUITextPanel *ta)
{
    if (ta == NULL) return;

    GUIRTEClear(ta->tedit);
}

/**************************************************************************/

void GUITPShowTCursor(GUITextPanel *ta)
{
    GrContext ctxsave;

    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTEShowTcursor(ta->tedit);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPHideTCursor(GUITextPanel *ta)
{
    GrContext ctxsave;

    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTEHideTcursor(ta->tedit);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPMoveTCursor(GUITextPanel *ta, int nline, int nchar, int setmark)
{
    GrContext ctxsave;

    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTEMoveTCursor(ta->tedit, nline, nchar, setmark, 1);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPNewLine(GUITextPanel *ta)
{
    GrContext ctxsave;

    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTENewLine(ta->tedit, 1);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPPutChar(GUITextPanel *ta, long ch, int chrtype)
{
    GrContext ctxsave;

    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTEPutChar(ta->tedit, ch, chrtype, 1);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPPutString(GUITextPanel *ta, void *s, int len, int chrtype)
{
    GrContext ctxsave;

    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTEPutString(ta->tedit, s, len, chrtype, 1);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPPutMultiString(GUITextPanel *ta, void *s, int len, int chrtype)
{
    GrContext ctxsave;

    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTEPutMultiString(ta->tedit, s, len, chrtype, 1);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPReDraw(GUITextPanel *ta)
{
    GrContext ctxsave;

    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTEReDraw(ta->tedit);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPTextColorIndexMA(GUITextPanel *ta, int cfgi)
{
    GrContext ctxsave;

    // this funcction set the fg color and apply to the marqued area
    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTETextColorIndexMA(ta->tedit, cfgi, 1);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPBgColorIndex(GUITextPanel *ta, int bgi)
{
    GrContext ctxsave;

    // this funcction set the bg color
    if (ta == NULL) return;

    GrSaveContext(&ctxsave);
    GrSetContext(ta->p->cl);
    GUIRTEBgColorIndex(ta->tedit, bgi, 1);
    GrSetContext(&ctxsave);
}

/**************************************************************************/

void GUITPGetStatus(GUITextPanel *ta, GUITEditStatus *tast)
{
    if (ta == NULL) return;

    GUIRTEGetStatus(ta->tedit, tast);
}

/**************************************************************************/

void *GUITPGetString(GUITextPanel *ta, int nline, int chrtype)
{
    if (ta == NULL) return NULL;

    return GUIRTEGetString(ta->tedit, nline, chrtype);
}

/**************************************************************************/

void *GUITPGetMultiString(GUITextPanel *ta, int fline, int lline, int chrtype)
{
    if (ta == NULL) return NULL;

    return GUIRTEGetMultiString(ta->tedit, fline, lline, chrtype);
}

/**************************************************************************/

int GUITPProcessEvent(GUITextPanel *ta, GrEvent *ev)
{
    // because called by panel, the context is expected to be set
    int ret;
    //GrContext ctxsave;

    if (ta == NULL) return 0;

    //GrSaveContext(&ctxsave);
    //GrSetContext(ta->p->cl);
    ret = GUIRTEProcessEvent(ta->tedit, ev);
    //GrSetContext(&ctxsave);

    return ret;
}

/**************************************************************************/

