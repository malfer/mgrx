/**
 ** grgui.h ---- Mini GUI for MGRX, public header 
 **
 ** Copyright (C) 2002,2006,2019-2024 Mariano Alvarez Fernandez
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

#ifndef __GRGUI_H_INCLUDED__
#define __GRGUI_H_INCLUDED__

#include <mgrx.h>
#include <mgrxkeys.h>

/**
 ** EVENTS (Aditional types)
 **/

// NOTE!! we use codes not used by MGRX but under GREV_USER range. So if MGRX
// adds new codes we need to check our codes don't conflict

#define GREV_COMMAND 50      // command event, p1=id, p2=subid (if any)
#define GREV_SELECT  51      // select event, p1=id, p2=subid (if any)
#define GREV_FCHANGE 52      // field changed, p1=id, p2=subid  (if any)
#define GREV_SCBVERT 53      // vert scrollbar moved, p1=new lift inivalue, p2=aid
#define GREV_SCBHORZ 54      // horz scrollbar moved, p1=new lift inivalue, p2=aid
#define GREV_PRIVGUI 55      // private GUI event, p1=subtype (user doesn't see it)
// at this time GrGui never generates GREV_END, so it is like a user event really
#define GREV_END     60      // end program event

/**
 ** SETUP
 **/

void GUIInit(int initgrevent, int doublebuffer);
void GUIEnd(void);

GrContext *GUIGetGlobalContext(void);
void GUIDBPauseBltsToScreen(void);
void GUIDBRestartBltsToScreen(void);
void GUIDBCurCtxBltToScreen(void);
void GUIDBCurCtxBltRectToScreen(int x1, int y1, int x2, int y2);
void GUIDBManageExposeEvents(int manage);

/**
 ** GUI CONTEXTS
 **/

typedef struct {
    int xorg;                // x position into the screen context
    int yorg;                // y position into the screen context
    int width;               //
    int height;              //
    GrContext *c;            // drawing sub context (screen or memory if DB)
    GrContext *su;           // save under memory context, can be NULL
    GrContext *sc;           // screen subcontext (NULL if !DB)
} GUIContext;

GUIContext *GUIContextCreate(int x, int y, int width, int height, int su);
void GUIContextSaveUnder(GUIContext *gctx);
void GUIContextRestoreUnder(GUIContext *gctx);
void GUIContextDestroy(GUIContext *gctx);
int GUIContextTransMouseEvIfInCtx(GUIContext *gctx, GrEvent *ev);
void GUIContextTransMouseEv(GUIContext *gctx, GrEvent *ev);

/**
 ** MENUS
 **/

#define GUI_MI_OPER   0      // Menu item that generates a GEV_COMMAND event
#define GUI_MI_MENU   1      // Menu item that pop-up another menu
#define GUI_MI_SEP    2      // Menu item separator
#define GUI_MI_TITLE  3      // Title menu, a non active item, normally the first item

typedef struct {
    int type;                // GUI_MI_OPER, GUI_MI_MENU, GUI_MI_SEP
    int enabled;             // 0=no, 1=yes
    char *title;             // Item title, '&' marks the key
    long key;                // Key to select the item
    char *fktitle;           // Fast Key title, can be NULL
    long fastkey;            // Key for fast access
    int id;                  // Returned id for GUI_MI_OPER or GUI_MI_MENU
    int tag;                 // 0=no tag, 1=ok, 2=point
    int sid;                 // string id, for i18n use
} GUIMenuItem;

typedef struct {
    int idm;                 // Menu id
    int nitems;              // Items number
    int select;              // Selected item
    GUIMenuItem *i;          // Items table
    int border;              // 0=1 bit normal border, 1=4 bit border
} GUIMenu;

void GUIMenusSetChrType(int chrtype);
void GUIMenusSetFont(GrFont *fnt);
void GUIMenusSetFontByName(char *fntname);
void GUIMenusSetColors(GrColor bg, GrColor bgs, GrColor fg,
                       GrColor fgs, GrColor fgna, GrColor fgsna);
void GUIMenusSetColors2(GrColor bgt, GrColor fgt, GrColor inb);
void GUIMenusSetI18nFields(void);
int GUIMenuRegister(GUIMenu *m);
int GUIMenuSetTag(int idmenu, int idop, int tag);
int GUIMenuSetUniqueTag(int idmenu, int idop, int tag);
int GUIMenuSetEnable(int idmenu, int type, int id, int enable);
char *GUIMenuGetTitle(int idmenu, int type, int id);
int GUIMenuGetDims(int idmenu, int *width, int *height);
int GUIMenuGetTag(int idmenu, int idop);
int GUIMenuGetEnable(int idmenu, int type, int id);
int GUIMenuRun(int idmenu, int x, int y, int minwidth);

/**
 ** MENUBAR
 **/

typedef struct {
    char *title;             // Item title
    int enabled;             // 0=no, 1=yes
    long key;                // Key to select the item
    int idm;                 // Menu id to run
    int sid;                 // string id, for i18n use
} GUIMenuBarItem;

typedef struct {
    int nitems;              // Items number
    int select;              // Selected item
    GUIMenuBarItem *i;       // Items table
//  int enabled;             // 0=no, 1=yes
} GUIMenuBar;

void GUIMenuBarSet(GUIMenuBar *mb);
void GUIMenuBarSetShortCut(long key);
void GUIMenuBarSetI18nFields(void);
int GUIMenuBarSetItemEnable(int idm, int enable);
int GUIMenuBarGetItemEnable(int idm);
void GUIMenuBarShow(void);
void GUIMenuBarHide(void);
void GUIMenuBarEnable(void);
void GUIMenuBarDisable(void);
int GUIMenuBarGetHeight(void);

/**
 ** SCROLLBARS (used by panels, list objects, edit objects...)
 **/

#define GUI_TSB_VERTICAL 0
#define GUI_TSB_HORIZONTAL 1

typedef struct {
    int type;               // vertical or horizontal
    int x, y;               // position
    int dx, dy;             // latest displacement requested to paint
    int width, height;      // dimensions
    int minvalue, maxvalue; // max limits
    int inivalue, endvalue; // lift limits
    int liftx, lifty;       // lift position
    int liftw, lifth;       // lift dimensions
    int pressed;            // left mouse pressed
    int pressedmode;        // 1=go up, 0=move, -1=go down
    int lastvaluesend;      // so GREV_NULL can repeat while pressed
    int pressed_pos;        // x or y pos whem pressed over lift
    int lastinivalue;       // last inivalue when a move was sent
    long lasttimesend;      // last time a event was send
    long intv;              // interval to send a new one
    long aid;               // automatic generated id
} GUIScrollbar;

void GUIScrollbarsSetColors(GrColor bg, GrColor lift);

GUIScrollbar * GUIScrollbarCreate(int type, int x, int y, int thick, int lenght);
long GUIScrollbarGetAid(GUIScrollbar *scb);
void GUIScrollbarSetLimits(GUIScrollbar *scb, int minvalue, int maxvalue,
                           int inivalue, int endvalue);
void GUIScrollbarPaint(GUIScrollbar *scb, int dx, int dy, int blt);
int GUIScrollbarProcessEvent(GUIScrollbar *scb, GrEvent *ev);

/**
 ** PANELS
 **/

#define GUI_PCAPB_SU 1       // save under
#define GUI_PCAPB_VSCB 2     // vertical scrollbar
#define GUI_PCAPB_HSCB 4     // horizontal scrollbar

typedef struct {
    int wborder;             // border width
    int htitle;              // title area height (can be 0)
    void * udata;            // data set by user
    GUIContext *gc;          // GUI context
    GrContext *cl;           // client area subcontext
    GrContext *ct;           // title area subcontext (can be NULL)
    int xcl, ycl;            // client area corner screen coord
    GUIScrollbar *vscb;      // vertical scrollbar (can be NULL)
    GUIScrollbar *hscb;      // horizontal scrollbar (can be NULL)
    void (*paintcl)(void *);            // function to draw inside cl (can be NULL)
    void (*painttl)(void *);            // function to draw inside tl (can be NULL)
    int (*proevcl)(void *, GrEvent *);  // function to process event (can be NULL)
} GUIPanel;

GUIPanel * GUIPanelCreate(int x, int y, int width, int height, int capbs,
                            int wborder, int htitle);
void GUIPanelSetClCallBacks(GUIPanel *p, void (*paintcl)(void *),
                             int (*proevcl)(void *, GrEvent *));
void GUIPanelSetTlCallBack(GUIPanel *p, void (*painttl)(void *));
void GUIPanelSetUserData(GUIPanel *p, void *udata);
void * GUIPanelGetUserData(GUIPanel *p);
void GUIPanelPaint(GUIPanel *p, GrColor lcolor, GrColor bcolor);
void GUIPanelRePaintScb(GUIPanel *p);
void GUIPanelRePaintBorder(GUIPanel *p, GrColor lcolor, GrColor bcolor);
int GUIPanelProcessEvent(GUIPanel *p, GrEvent *ev);
void GUIPanelDestroy(GUIPanel *p);

/**
 ** TILES
 **/

#define GUI_TT_BORDERLESS     1
#define GUI_TT_STATICBORDER   2
#define GUI_TT_ACTIVEBORDER   3
#define GUI_TT_ACTIVEBWSCB    4  // active border with both scrollbars
#define GUI_TT_ACTIVEBWVSCB   5  // active border with vertical scrollbar
#define GUI_TT_ACTIVEBWHSCB   6  // active border with horizontal scrollbar

typedef struct {
    int idt;                 // tile id
    int type;                // see defines
    int active;              // 0=no, 1=yes
    int selected;            // 0=no, 1=yes, only active panels can be selected
    GUIPanel *p;             // Panel
} GUITile;

GUITile * GUITileCreate(int idt, int type, int x, int y, int width, int height);
void GUITileDestroy(GUITile *t);
int GUITileRegister(GUITile *t);
GUITile * GUITileUnRegister(int idt);
void GUITilePaint(int idt);
GUITile * GUITileGetPointer(int idt);

void GUITilesSetColors(GrColor l, GrColor b, GrColor sb);
int GUITilesGetSelected(void);
void GUITilesSetSelected(int idt, int paint);
int GUITilesProcessEvent(GrEvent *ev);
int GUITilesProcessTileEvent(GrEvent *ev, int idt);
void GUITilesDestroyAll(void);

/**
 ** DIALOGS
 **/

typedef struct {
    void *title;             // can be NULL
    int theight;             // title height
    int running;             // 0=no, 1=yes
    GUIPanel *p;             // Panel
    GrColor fg, bg;          // foreground & background colors
    void *exdata;            // extended data (can be NULL), commom and group
                             // dialogs store here his GUIGroup pointer
    int result;              // result after running
} GUIDialog;

/* Raw dialogs */

void GUIDialogsSetTitleChrType(int chrtype);
void GUIDialogsSetTitleFont(GrFont *fnt);
void GUIDialogsSetTitleFontByName(char *fntname);
void GUIDialogsSetColors(GrColor line, GrColor border, GrColor bg, GrColor fg);
void GUIDialogsGetDims(int *titleheight, int *borderwidth);

GUIDialog * GUIDialogCreate(void *title, int x, int y, int width, int height);
void GUIDialogDestroy(GUIDialog *d);
int GUIDialogRun(GUIDialog *d);

/* Common dialogs */

void GUICDialogsSetChrType(int chrtype);
void GUICDialogsSetFont(GrFont *fnt);
void GUICDialogsSetFontByName(char *fntname);
void GUICDialogsSetColors(GrColor btbg, GrColor btfg);
void GUICDialogsSetObjColors(GrColor objbg, GrColor objfg, GrColor objbggray, GrColor labelfg);

int GUICDialogInfo(void *title, void **text, int nlin, void *ok);
int GUICDialogYesNo(void *title, void **text, int nlin, void *yes, void *no);
int GUICDialogYesNoCancel(void *title, void **text, int nlin,
                          void *yes, void *no, void *cancel);

/* File chooser dialog */

typedef struct {
    void *dirs;
    void *files;
    void *dirpath;
    void *fname;
    void *ok;
    void *cancel;
    void *overwrite;
    void *yes;
    void *no;
} GUIFileChooserLabels;

// if not called or setting fcl to NULL default English label will be used
void GUIFileChooserSetLabels(GUIFileChooserLabels *fcl);

#define FCDLG_MAXLEN_FNAME 240 // fname parameter must be char[FCDLG_MAXLEN_FNAME+1] at least

#define FCDLGF_ASKOVERWRITE 1 // flags: ask user if overwrite a existing file

int GUICDialogFileChooser(void *title, int width, int height, char *fname, int flags);

/**
 ** OBJECTS
 **/

#define GUIOBJTYPE_NONE      0
#define GUIOBJTYPE_LABEL     1
#define GUIOBJTYPE_TEXT      2
#define GUIOBJTYPE_LIGHT     3
#define GUIOBJTYPE_BUTTON    4
#define GUIOBJTYPE_ENTRY     5
#define GUIOBJTYPE_LIST      6
#define GUIOBJTYPE_DLIST     7
#define GUIOBJTYPE_REGLIST   8
#define GUIOBJTYPE_EDIT      9

typedef struct {
    int type;                // Object type
    int id;                  // object id (ALL) unique within the group
    int visible;             // 1=yes (default after Set), 0=no (ALL)
    int x, y;                // Position (ALL)
    int width, height;       // Dimension (ALL)
    GrColor bg, fg;          // background, foreground colors (ALL)
    void *text;              // text string (1,3,4, initial text for 5,9)
    int maxlen;              // max string len (5,9)
    int pressed;             // object clicked (4,5,6,7,8,9)
                             // (1,2,3) = -1. not pressable
    int selected;            // selected object (4,5,6,7,8,9) only one in the group
                             // (1,2,3) = -1. not selectable
    int cid;                 // command id (4)
    int sgid;                // subgroup id (4) 0=normal button,
                             // 1=on/off button, >100 subgroup member
    int on;                  // on status (3,4) if sgid != 0 (for 4)
    void *data;              // Private data (2,5,6,7,8,9)
} GUIObject;

void GUIObjectsSetChrType(int chrtype);
void GUIObjectsSetFont(GrFont *fnt);
void GUIObjectsSetFontByName(char *fntname);
void GUIObjectsSetColors(GrColor l, GrColor sh1, GrColor sh2);

void GUIObjectSetLabel(GUIObject *o, int id, int x, int y, int width, int height,
                       GrColor bg, GrColor fg, void *text);

void GUIObjectSetText(GUIObject *o, int id, int x, int y, int width, int height,
                      GrColor bg, GrColor fg, void **text, int nlin, char xalign,
                      GrFont *f);

void GUIObjectSetLight(GUIObject *o, int id, int x, int y, int width, int height,
                       GrColor bg, GrColor fg, void *text, int on);

void GUIObjectSetButton(GUIObject *o, int id, int x, int y, int width, int height,
                        GrColor bg, GrColor fg, void *text, int cid, int sgid, int on);

void GUIObjectSetEntry(GUIObject *o, int id, int x, int y, int width, int height,
                       GrColor bg, GrColor fg, int maxlen, void *initext);

void GUIObjectSetList(GUIObject *o, int id, int x, int y, int width, int height,
                      GrColor bg, GrColor fg, void **reg, int nreg, int vreg, int sreg);

void GUIObjectSetDList(GUIObject *o, int id, int x, int y, int width, int height,
                       GrColor bg, GrColor fg, void **reg, int nreg, int sreg);

void GUIObjectSetRegList(GUIObject *o, int id, int x, int y, int width, int height,
                         GrColor bg, GrColor fg, int nreg, int regheight, int sreg,
                         void (*paint_reg)(int x, int y, int ind, int selected,
                         GrColor bg, GrColor fg));

// Edit type flags, OR them as desired
#define GUI_OEDIT_EDITABLE 1
#define GUI_OEDIT_WITHVSCB 2
#define GUI_OEDIT_WITHHSCB 4

void GUIObjectSetEdit(GUIObject *o, int id, int x, int y, int width, int height,
                      GrColor bg, GrColor fg, int type, int maxlen, void *initext);

void GUIObjectUpdList(GUIObject *o, void **reg, int nreg, int vreg, int sreg);
void GUIObjectUpdDList(GUIObject *o, void **reg, int nreg, int sreg);

/**
 ** GROUPS of objects
 **/

typedef struct {
    int x, y;                 // left upper coordinates
    int nobj;                 // number of objects
    int selected;             // selected object (-1 if none)
    GUIObject *o;             // guiobject array
    GUIPanel *p;              // panel container (can be NULL)
} GUIGroup;

GUIGroup * GUIGroupCreate(int nobj, int x, int y);
void GUIGroupSetPanel(GUIGroup *g, GUIPanel *p);
void GUIGroupDestroy(GUIGroup *g);
int GUIGroupGetSelected(GUIGroup *g);
void GUIGroupSetSelected(GUIGroup *g, int id, int paint);
void GUIGroupSetVisible(GUIGroup *g, int id, int visible);
void GUIGroupRestartAfterVisibleChanges(GUIGroup *g);
void GUIGroupPaint(GUIGroup *g);
void GUIGroupRePaintObject(GUIGroup *g, int id);
void *GUIGroupGetText(GUIGroup *g, int id, int chrtype);
void GUIGroupSetText(GUIGroup *g, int id, void *newtext);
void GUIGroupSetOn(GUIGroup *g, int id, int paint);
int GUIGroupGetOn(GUIGroup *g, int id);
int GUIGroupProcessEvent(GUIGroup *g, GrEvent *ev);
void GUIGroupCalcMinDim(GUIGroup *g, int *width, int *height);

/* GUIGroup based dialogs */

GUIDialog * GUIGroupDialogCreate(void *title, GUIGroup *go,
                                 int (*proevfn)(void *, GrEvent *));

/**
 ** TEXTPANEL text editor in a panel
 **/

typedef struct _guiTextPanel GUITextPanel;

typedef struct{
    int nlines;            // actual number of lines
    int numchars;          // actual number of chars
    int tclpos;            // text cursor line pos
    int tccpos;            // text cursor char pos
    int ncscr;             // min nº chars fitting on screen
    int fmline, fmch;      // line&col first char of marked area
    int lmline, lmch;      // line&col last char of marked area
    int full;              // text buffer is full
    int partloaded;        // last paste operation was partially loaded
} GUITEditStatus;

// setmark values
#define GUI_RESETMARQUEDAREA 1
#define GUI_EXPANDMARQUEDAREA -1

GUITextPanel * GUITPCreate(GUIPanel *p, GrFont *f);
void GUITPSetPanel(GUITextPanel *ta, GUIPanel *p);
void GUITPSetFont(GUITextPanel *ta, GrFont *f);
void GUITPSetSimpleColors(GUITextPanel *ta, GrColor bg, GrColor cfg, GrColor tc);
void GUITPSetTableColors(GUITextPanel *ta, GrColor *ctbl, int numcolors,
                         int bgi, int cfgi, int tci);
void GUITPTextColorIndex(GUITextPanel *ta, int cfgi);
void GUITPTextColorIndexMA(GUITextPanel *ta, int cfgi);
void GUITPBgColorIndex(GUITextPanel *ta, int bgi);
void GUITPCursorColorIndex(GUITextPanel *ta, int tci);
void GUITPDestroy(GUITextPanel *ta);
void GUITPClear(GUITextPanel *ta);
void GUITPShowTCursor(GUITextPanel *ta);
void GUITPHideTCursor(GUITextPanel *ta);
void GUITPMoveTCursor(GUITextPanel *ta, int nline, int nchar, int setmark);
void GUITPNewLine(GUITextPanel *ta);
void GUITPPutChar(GUITextPanel *ta, long ch, int chrtype);
void GUITPPutString(GUITextPanel *ta, void *s, int len, int chrtype);
void GUITPPutMultiString(GUITextPanel *ta, void *s, int len, int chrtype);
void GUITPPutMultiStringNoDraw(GUITextPanel *ta, void *s, int len, int chrtype);
void GUITPResetMA(GUITextPanel *ta);
void GUITPSetMA(GUITextPanel *ta, int fl, int fc, int ll, int lc);
int GUITPCopyMA(GUITextPanel *ta, int clear);
void GUITPPasteMA(GUITextPanel *ta);
void GUITPReDraw(GUITextPanel *ta);
void GUITPGetStatus(GUITextPanel *ta, GUITEditStatus *tast);
int GUITPGetChanged(GUITextPanel *ta);
void GUITPResetChanged(GUITextPanel *ta);
void *GUITPGetString(GUITextPanel *ta, int nline, int chrtype);
void *GUITPGetMultiString(GUITextPanel *ta, int fline, int lline, int chrtype);
int GUITPProcessEvent(GUITextPanel *ta, GrEvent *ev);

/**
 ** Raw Data List (mainly for LIST & DLIST use, but it can be useful by itself)
 **/

#define GUIRDLTYPE_SIMPLE    1

typedef struct _guiRawDataList GUIRawDataList;

GUIRawDataList * GUIRDLCreate(int type, int x, int y, int width, int height,
                              GrColor bg, GrColor fg, void **reg, int nreg, int sreg);
void GUIRDLDestroy(GUIRawDataList *rdl);
void GUIRDLSetSreg(GUIRawDataList *rdl, int sreg);
int GUIRDLGetSreg(GUIRawDataList *rdl);
void GUIRDLPaint(GUIRawDataList *rdl, int dx, int dy, int blt);
void GUIRDLSetSelected(GUIRawDataList *rdl, int selected, int paint, int blt);
int GUIRDLProcessEvent(GUIRawDataList *rdl, GrEvent *ev);

/**
 ** Raw Data List2 a more generic version, used by GUIOBJTYPE_REGLIST (for now)
 **/

typedef struct {
    int type;             // RDL type
    int nreg;             // num of reg in list
    int x, y;             // position
    int width, height;    // dimensions
    GrColor bg, fg;       // background, foreground colors
    int regheight;        // height of a reg
    void (*paint_reg)(int x, int y, int ind, int selected,
                      GrColor bg, GrColor fg); // paint reg function
} GUIRDL2Intf;

typedef struct _guiRawDataList2 GUIRawDataList2;

GUIRawDataList2 * GUIRDL2Create(GUIRDL2Intf *f, int sreg);
void GUIRDL2Destroy(GUIRawDataList2 *rdl2);
void GUIRDL2SetSreg(GUIRawDataList2 *rdl2, int sreg);
int GUIRDL2GetSreg(GUIRawDataList2 *rdl2);
void GUIRDL2Paint(GUIRawDataList2 *rdl2, int dx, int dy, int blt);
void GUIRDL2SetSelected(GUIRawDataList2 *rdl2, int selected, int paint, int blt);
int GUIRDL2ProcessEvent(GUIRawDataList2 *rdl2, GrEvent *ev);

/**
 ** Raw Text Editor, used by GUITextPanel and by GUIOBJTYPE_EDIT
 **/

typedef struct _guiRawTEdit GUIRawTEdit;

GUIRawTEdit * GUIRTECreate(GrFont *f, int editable, int maxchars,
                           int x, int y, int width, int height);
void GUIRTESetDims(GUIRawTEdit *ta, int x, int y, int width, int height);
void GUIRTESetScrollbars(GUIRawTEdit *ta, GUIScrollbar *vscb,
                         GUIScrollbar *hscb, GrContext *scbctx);
void GUIRTESetScrollbarsDisplacement(GUIRawTEdit *ta, int dx, int dy);
void GUIRTESetFont(GUIRawTEdit *ta, GrFont *f);
void GUIRTESetSimpleColors(GUIRawTEdit *ta, GrColor bg, GrColor cfg, GrColor tc);
void GUIRTESetTableColors(GUIRawTEdit *ta, GrColor *ctbl, int numcolors,
                          int bgi, int cfgi, int tci);
void GUIRTETextColorIndex(GUIRawTEdit *ta, int cfgi);
void GUIRTECursorColorIndex(GUIRawTEdit *ta, int tci);
void GUIRTETextColorIndexMA(GUIRawTEdit *ta, int cfgi, int draw);
void GUIRTEBgColorIndex(GUIRawTEdit *ta, int bgi, int draw);
void GUIRTEDestroy(GUIRawTEdit *ta);
void GUIRTEClear(GUIRawTEdit *ta);
void GUIRTEShowTcursor(GUIRawTEdit *ta);
void GUIRTEHideTcursor(GUIRawTEdit *ta);
void GUIRTEMoveTCursor(GUIRawTEdit *ta, int nline, int nchar, int setmark, int draw);
void GUIRTENewLine(GUIRawTEdit *ta, int draw);
void GUIRTEPutChar(GUIRawTEdit *ta, long ch, int chrtype, int draw);
void GUIRTEPutString(GUIRawTEdit *ta, void *s, int len, int chrtype, int draw);
void GUIRTEPutMultiString(GUIRawTEdit *ta, void *s, int len, int chrtype, int draw);
void GUIRTEResetMA(GUIRawTEdit *ta, int draw);
void GUIRTESetMA(GUIRawTEdit *ta, int fl, int fc, int ll, int lc, int draw);
int GUIRTECopyMA(GUIRawTEdit *ta, int clear, int draw);
void GUIRTEPasteMA(GUIRawTEdit *ta, int draw);
void GUIRTEReDraw(GUIRawTEdit *ta);
void GUIRTEGetStatus(GUIRawTEdit *ta, GUITEditStatus *tast);
void *GUIRTEGetString(GUIRawTEdit *ta, int nline, int chrtype);
void *GUIRTEGetMultiString(GUIRawTEdit *ta, int fline, int lline, int chrtype);
int GUIRTEGetChanged(GUIRawTEdit *ta);
void GUIRTEResetChanged(GUIRawTEdit *ta);
int GUIRTEProcessEvent(GUIRawTEdit *ta, GrEvent *ev);

/**
 ** Some utility functions
 **/

int GrCheckCoordInto(int x, int y, int xo, int yo, int width, int height);
void GrDrawBorder(int x1, int y1, int x2, int y2, int wborder,
                  GrColor lcolor, GrColor bcolor);

#endif /*__GRGUI_H_INCLUDED__*/
