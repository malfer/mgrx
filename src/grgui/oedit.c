/**
 ** oedit.c ---- Mini GUI for MGRX, specific funtions for edit objects
 **
 ** Copyright (C) 2022 Mariano Alvarez Fernandez
 ** [e-mail: malfer@telefonica.net]
 **
 ** This is a test/demo file of the GRX graphics library.
 ** You can use GRX test/demo files as you want.
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

typedef struct {
    int type;
    int editable;
    int rwidth, rheight;   // dimensions without scrollbars
    GUIScrollbar *vscb;    // vertical scrollbar (can be NULL)
    GUIScrollbar *hscb;    // horizontal scrollbar (can be NULL)
    GUIRawTEdit *rte;      // raw text editor
} EditData;

/***************************/

void GUIObjectSetEdit(GUIObject *o, int id, int x, int y, int width, int height,
                      GrColor bg, GrColor fg, int type, int maxlen, void *initext)
{
    EditData *data;
    int xscb, yscb;
    
    o->type = GUIOBJTYPE_EDIT;
    o->id = id;
    o->x = x;
    o->y = y;
    o->width = width;
    o->height = height;
    o->bg = bg;
    o->fg = fg;

    o->maxlen = maxlen;
    o->text = NULL;
    o->cid = 0;
    o->sgid = 0;
    o->on = 0;
    o->pressed = 0;
    o->selected = 0;
    o->visible = 1;

    data = malloc(sizeof(EditData));
    if (data == NULL) goto error;

    data->type = type;
    data->vscb = NULL;
    data->hscb = NULL;
    data->rte = NULL;
    data->rwidth = width;
    data->rheight = height;
    data->editable = (type & GUI_OEDIT_EDITABLE) ? 1 : 0;
    if (type & GUI_OEDIT_WITHVSCB) data->rwidth -= GUI_SB_DEFAULT_THICK + 1;
    if (type & GUI_OEDIT_WITHHSCB) data->rheight -= GUI_SB_DEFAULT_THICK + 1;
    if (type & GUI_OEDIT_WITHVSCB) {
        xscb = x + width - GUI_SB_DEFAULT_THICK - 2;
        data->vscb = GUIScrollbarCreate(GUI_TSB_VERTICAL, xscb, y+2,
                                        GUI_SB_DEFAULT_THICK, data->rheight-4);
        if (data->vscb == NULL) goto error;
    }
    if (type & GUI_OEDIT_WITHHSCB) {
        yscb = y + height - GUI_SB_DEFAULT_THICK - 2;
        data->hscb = GUIScrollbarCreate(GUI_TSB_HORIZONTAL, x+2, yscb,
                                        GUI_SB_DEFAULT_THICK, data->rwidth-4);
        if (data->hscb == NULL) goto error;
    }
    data->rte = GUIRTECreate(_objectfont, data->editable, o->maxlen,
                             x+3, y+3, data->rwidth-6, data->rheight-6);
    if (data->rte == NULL) goto error;

    GUIRTESetSimpleColors(data->rte, bg, fg, fg);
    GUIRTESetScrollbars(data->rte, data->vscb, data->hscb, NULL);
    GUIRTEPutMultiString(data->rte, initext, 0, _objectchrtype, 0);
    GUIRTEMoveTCursor(data->rte, 0, 0, GUI_RESETMARQUEDAREA, 0);
    GUIRTEResetChanged(data->rte);

    o->data = (void *)data;
    return;

error:
    if (data) {
        if (data->vscb) free(data->vscb);
        if (data->hscb) free(data->hscb);
        free(data);
    }
    o->type = GUIOBJTYPE_NONE;
    o->pressed = -1;
    o->selected = -1;
    o->visible = 0;
}

/***************************/

void _GUIOEditDestroy(GUIObject *o)
{
    EditData *data;
    
    data = (EditData *)(o->data);

    if (data->vscb) free(data->vscb);
    if (data->hscb) free(data->hscb);
    GUIRTEDestroy(data->rte);
    free(data);
}

/***************************/

void *_GUIOEditGetText(GUIObject *o, int chrtype)
{
    EditData *data;
    GUITEditStatus tast;
    
    data = (EditData *)(o->data);
    GUIRTEGetStatus(data->rte, &tast);
    return GUIRTEGetMultiString(data->rte, 0, tast.nlines-1, chrtype);
}

/***************************/

void _GUIOEditSetText(GUIObject *o, void *newtext)
{
    EditData *data;
    
    data = (EditData *)(o->data);
    GUIRTEClear(data->rte);
    GUIRTEPutMultiString(data->rte, newtext, 0, _objectchrtype, 0);
    GUIRTEMoveTCursor(data->rte, 0, 0, GUI_RESETMARQUEDAREA, 0);
    GUIRTEResetChanged(data->rte);
}

/***************************/

void _GUIOEditPaint(GUIObject *o, int dx, int dy)
{
    EditData *data;
    int mouseblock;
    int x, y;
    GrLineOption glo;
    
    data = (EditData *)(o->data);

    x = o->x + dx;
    y = o->y + dy;

    mouseblock = GrMouseBlock(NULL, x, y, x+o->width-1, y+o->height-1);

    GrBox(x, y, x+o->width-1, y+o->height-1, o->fg);
    GrFilledBox(x+1, y+1, x+o->width-2, y+o->height-2, o->bg);

    if (o->selected) {
        glo.lno_color = o->fg;
        glo.lno_width = 1;
        glo.lno_pattlen = 2;
        glo.lno_dashpat = (unsigned char *)"\2\1";
        GrCustomBox(x+2, y+2, x+data->rwidth-3, y+data->rheight-3, &glo);
    } else {
        GrBox(x+2, y+2, x+data->rwidth-3, y+data->rheight-3, o->bg);
    }

    if (data->vscb && data->hscb) { // paint corner
        GrFilledBox(x+data->rwidth-1, y+data->rheight-1,
                    x+data->rwidth-2+GUI_SB_DEFAULT_THICK,
                    y+data->rheight-2+GUI_SB_DEFAULT_THICK,
                    _scbliftcolor);
        GrFilledBox(x+data->rwidth+2, y+data->rheight+2,
                    x+data->rwidth-5+GUI_SB_DEFAULT_THICK,
                    y+data->rheight-5+GUI_SB_DEFAULT_THICK,
                    _scbbgcolor);
    }

    GUIRTESetDims(data->rte, x+3, y+3, data->rwidth-6, data->rheight-6);
    GUIRTESetScrollbarsDisplacement(data->rte, dx, dy);
    GUIRTEReDraw(data->rte);

    if (data->editable) {
        if (o->selected) GUIRTEShowTcursor(data->rte);
        else GUIRTEHideTcursor(data->rte);
    }

    GrMouseUnBlock(mouseblock);

    if (!o->selected && GUIRTEGetChanged(data->rte)) {
        GrEventParEnqueue(GREV_FCHANGE, o->id, 0, 0, 0);
        GUIRTEResetChanged(data->rte);
    }
}

/***************************/

void _GUIOEditRePaint(GUIGroup *g, GUIObject *o)
{
    int x, y;

    x = o->x + g->x;
    y = o->y + g->y;

    _GUIOEditPaint(o, g->x, g->y);
    GUIDBCurCtxBltRectToScreen(x, y, x+o->width-1, y+o->height-1);
}

/***************************/

int _GUIOEditProcessEvent(GUIGroup *g, GUIObject *o, GrEvent *ev)
{
    EditData *data;
    int ret = 0;
    
    data = (EditData *)(o->data);

    if (ev->type == GREV_MOUSE) {
        if (ev->p1 == GRMOUSE_LB_PRESSED) {
            if (!o->selected) {
                o->selected = 1;
                _GUIOEditRePaint(g, o);
            }
            o->pressed = 1;
        } else if (ev->p1 == GRMOUSE_LB_RELEASED) {
            o->pressed = 0;
        }
    } else if (ev->type == GREV_KEY) {
        if (ev->p1 == GrKey_Tab)
            return 0; // to be able to change focus in the group
    }

    if (data->vscb != NULL)
        ret = GUIScrollbarProcessEvent(data->vscb, ev);
    if (!ret && data->hscb != NULL)
        ret = GUIScrollbarProcessEvent(data->hscb, ev);
    if (!ret)
        ret = GUIRTEProcessEvent(data->rte, ev);

    return ret;
}
