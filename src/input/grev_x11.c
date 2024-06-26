/**
 ** grev_x11.c ---- MGRX events, X11 input
 **
 ** Copyright (C) 2005 Mariano Alvarez Fernandez
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
 ** Contributions by:
 ** 070505 M.Alvarez, Using a Pixmap for BackingStore
 ** 080113 M.Alvarez, intl support
 ** 191112 M.Alvarez, Added code to generate GREV_WMEND events
 ** 211110 M.Alvarez, support window resize
 ** 211215 M.Alvarez, better keycode conversion, using a table 
 **                   for every state instead of a big table
 ** 220206 M.Alvarez, X11 clipboard support
 **/

#include <stdlib.h>
#include <string.h>
#include "libgrx.h"
#include "libxwin.h"
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xlocale.h>
#include "mgrxkeys.h"
#include "ninput.h"
#include "x11keys.h"
#include "arith.h"

static int kbd_lastmod = 0;
static int kbsysencoding = -1;
static int utf8support = 0;

static int _XKeyEventToGrKey(XKeyEvent *xkey, long *p1, long *p2);
static int _Xutf8KeyEventToGrKey(XKeyEvent *xkey, long *p1, long *p2);
static int _XGrX11KeyCode(KeySym keysym, unsigned int state, long *p1, long *p2);
static unsigned int _XGrModifierKey(KeySym keysym, int type);
static XIMStyle _XGrChooseBetterStyle(XIMStyle style1, XIMStyle style2);
static int _XGrOpenXIMandXIC(Display *dpy, Window win, XIM *im, XIC *ic);

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
    static char cbits[8] = { 0,0,0,0,0,0,0,0, };
    Pixmap csource, cmask;
    XColor cfore, cback;
    Cursor curs;
    char *s;

    if (!_XGrDisplay) {
        return 0;
    }

    if (GrMouseDetect()) { // Define an invisible X cursor for _XGrWindow
        if(_XGrWindowedMode) {
            csource = cmask = XCreateBitmapFromData(
                              _XGrDisplay, _XGrWindow, cbits, 8, 8);
            cfore.red = cfore.green = cfore.blue = 0;
            cback.red = cback.green = cback.blue = 0;
            curs = XCreatePixmapCursor(
                   _XGrDisplay, csource, cmask, &cfore, &cback, 0, 0);
            XDefineCursor (_XGrDisplay, _XGrWindow, curs);
        }
    }

    if (_XGrOpenXIMandXIC(_XGrDisplay, _XGrWindow, &_XGrXim, &_XGrXic)) {
        utf8support = 1;
        kbsysencoding = GRENC_UTF_8;
    }
    else {
        utf8support = 0;
        s = getenv("MGRXKBSYSENCODING");
        if (s != NULL) kbsysencoding = GrFindEncoding(s);
        if (kbsysencoding < 0) kbsysencoding = GRENC_ISO_8859_1;
    }

    return 1;
}

/**
 ** _GrEventUnInit - UnInitializes inputs
 **
 ** For internal use only
 **/

void _GrEventUnInit(void)
{
    if (utf8support) {
        XDestroyIC(_XGrXic);
        XCloseIM(_XGrXim);
        utf8support = 0;
    }
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
    GrEvent evaux;
    int nev = 0;
    int count;
    XEvent xev;
    KeySym keysym;
    XEvent sendxev;

    if (!_XGrDisplay) return 0;

    // I think is better the QueuedAfterFlush mode, but...
    //count = XEventsQueued(_XGrDisplay, QueuedAfterReading);
    count = XEventsQueued(_XGrDisplay, QueuedAfterFlush);
    if (count <= 0) {
        //XFlush(_XGrDisplay);
        usleep(1000L);   // wait 1 ms to not eat 100% cpu
        return 0;
    }

    while (--count >= 0) {
        XNextEvent(_XGrDisplay, &xev);
        if (utf8support && XFilterEvent(&xev, None)) {
            continue;
        }
        switch (xev.type) {
        case ClientMessage:
            if (xev.xclient.data.l[0] == _wmDeleteWindow) {
                if (_XGrGenWMEndEvents == GR_GEN_WMEND_NO) {
                    exit(1);
                } else {
                    GrEventParEnqueue(GREV_WMEND, 0, 0, 0, 0);
                }
            }
            break;

        case ConfigureNotify:
            //printf("ConfigureNotify %d %d %d %d %d\n",
            //       (int)xev.xconfigure.send_event,
            //       xev.xconfigure.x, xev.xconfigure.y,
            //       xev.xconfigure.width, xev.xconfigure.height);
            if (_XGrGenWSzChgEvents && !xev.xconfigure.send_event) {
                // we don't want synthetic ConfigureNotify events
                int w = xev.xconfigure.width;
                int h = xev.xconfigure.height;
                if (w != _XGrCurWidth || h != _XGrCurHeight) {
                    GrEventParEnqueue(GREV_WSZCHG, 0, 0, w, h);
                }
            }
            break;

        case Expose:
            //printf("Expose %d %d %d %d %d %d\n",
            //       (int)xev.xexpose.send_event,
            //       xev.xexpose.x, xev.xexpose.count,
            //       xev.xexpose.y, xev.xexpose.width, xev.xexpose.height);
            if (_XGrGenExposeEvents == GR_GEN_EXPOSE_NO) {
                if (_XGrBStoreInited) {
                    XCopyArea(_XGrDisplay, _XGrBStore, _XGrWindow, _XGrDefaultGC,
                              xev.xexpose.x, xev.xexpose.y, xev.xexpose.width,
                              xev.xexpose.height, xev.xexpose.x, xev.xexpose.y);
                    //_XGrCopyBStore(xev.xexpose.x, xev.xexpose.y,
                    //               xev.xexpose.width, xev.xexpose.height);
                }
            } else {
                evaux.type = GREV_EXPOSE;
                evaux.kbstat = xev.xexpose.count; // if != 0 more expose follow
                evaux.p1 = xev.xexpose.x;
                evaux.p2 = xev.xexpose.y;
                evaux.p3 = xev.xexpose.width;
                evaux.p4 = xev.xexpose.height;
                GrEventEnqueue(&evaux);
            }
            break;

        case MotionNotify:
            if (_XGrWindowedMode) {
                MOUINFO->xpos = xev.xmotion.x;
                MOUINFO->ypos = xev.xmotion.y;
            } else {
                MOUINFO->xpos += xev.xmotion.x;
                MOUINFO->ypos += xev.xmotion.y;
            }
            GrMouseUpdateCursor();
            MOUINFO->moved  = TRUE;
            if ((MOUINFO->genmmove == GR_GEN_MMOVE_ALWAYS) ||
                ((MOUINFO->genmmove == GR_GEN_MMOVE_IFBUT) &&
                 (MOUINFO->bstatus != 0))) {
                evaux.type = GREV_MMOVE;
                evaux.kbstat = kbd_lastmod;
                evaux.p1 = MOUINFO->bstatus;
                evaux.p2 = MOUINFO->xpos;
                evaux.p3 = MOUINFO->ypos;
                GrEventEnqueue(&evaux);
                MOUINFO->moved = FALSE;
                nev++;
            }
            break;

        case ButtonPress:
        case ButtonRelease:
            evaux.type = GREV_MOUSE;
            evaux.kbstat = kbd_lastmod;
            evaux.p2 = MOUINFO->xpos;
            evaux.p3 = MOUINFO->ypos;
            switch (xev.xbutton.type) {
            case ButtonPress:
                switch (xev.xbutton.button) {
                case Button1: evaux.p1 = GRMOUSE_LB_PRESSED;
                              MOUINFO->bstatus |= GRMOUSE_LB_STATUS;
                              break;
                case Button2: evaux.p1 = GRMOUSE_MB_PRESSED;
                              MOUINFO->bstatus |= GRMOUSE_MB_STATUS;
                              break;
                case Button3: evaux.p1 = GRMOUSE_RB_PRESSED;
                              MOUINFO->bstatus |= GRMOUSE_RB_STATUS;
                              break;
                case Button4: evaux.p1 = GRMOUSE_B4_PRESSED;
                              break;
                case Button5: evaux.p1 = GRMOUSE_B5_PRESSED;
                              break;
                case 6      : evaux.p1 = GRMOUSE_B6_PRESSED;
                              break;
                case 7      : evaux.p1 = GRMOUSE_B7_PRESSED;
                              break;
                case 8      : evaux.p1 = GRMOUSE_B8_PRESSED;
                              break;
                case 9      : evaux.p1 = GRMOUSE_B9_PRESSED;
                              break;
                }
                break;
            case ButtonRelease:
                switch (xev.xbutton.button) {
                case Button1: evaux.p1 = GRMOUSE_LB_RELEASED;
                              MOUINFO->bstatus &= ~GRMOUSE_LB_STATUS;
                              break;
                case Button2: evaux.p1 = GRMOUSE_MB_RELEASED;
                              MOUINFO->bstatus &= ~GRMOUSE_MB_STATUS;
                              break;
                case Button3: evaux.p1 = GRMOUSE_RB_RELEASED;
                              MOUINFO->bstatus &= ~GRMOUSE_RB_STATUS;
                              break;
                case Button4: evaux.p1 = GRMOUSE_B4_RELEASED;
                              break;
                case Button5: evaux.p1 = GRMOUSE_B5_RELEASED;
                              break;
                case 6      : evaux.p1 = GRMOUSE_B6_RELEASED;
                              break;
                case 7      : evaux.p1 = GRMOUSE_B7_RELEASED;
                              break;
                case 8      : evaux.p1 = GRMOUSE_B8_RELEASED;
                              break;
                case 9      : evaux.p1 = GRMOUSE_B9_RELEASED;
                              break;
                }
                break;
            }
            GrEventEnqueue(&evaux);
            MOUINFO->moved = FALSE;
            nev++;
            break;

        case KeyPress:
            //keysym = XKeycodeToKeysym(_XGrDisplay, xev.xkey.keycode, 0);
            keysym = XkbKeycodeToKeysym(_XGrDisplay, xev.xkey.keycode, 0, 0);
            if (IsModifierKey(keysym)) {
                _XGrModifierKey(keysym, xev.type);
            } else {
                evaux.type = GREV_PREKEY;
                evaux.kbstat = kbd_lastmod;
                evaux.p3 = 0;
                if (utf8support) {
                    if (_Xutf8KeyEventToGrKey(&xev.xkey,&evaux.p1,&evaux.p2)) {
                        GrEventEnqueue(&evaux);
                        MOUINFO->moved = FALSE;
                        nev++;
                    }
                }
                else {
                    if (_XKeyEventToGrKey(&xev.xkey,&evaux.p1,&evaux.p2)) {
                        GrEventEnqueue(&evaux);
                        MOUINFO->moved = FALSE;
                        nev++;
                    }
                }
            }
            break;

        case KeyRelease:
            //keysym = XKeycodeToKeysym(_XGrDisplay, xev.xkey.keycode, 0);
            keysym = XkbKeycodeToKeysym(_XGrDisplay, xev.xkey.keycode, 0, 0);
            if (IsModifierKey (keysym)) {
                _XGrModifierKey (keysym, xev.type);
            }
            break;

        case SelectionClear:
            _XGrCBOwnSelection = 0;
            //printf("SelectionClear\n");
            break;

        case SelectionRequest:
            //printf("SelectionRequest %ld %ld %ld\n", xev.xselectionrequest.selection,
            //       _XGrCBSelection, xev.xselectionrequest.target);
            //if (xev.xselectionrequest.selection != _XGrCBSelection) break;
            sendxev.xselection.type = SelectionNotify;
            sendxev.xselection.requestor = xev.xselectionrequest.requestor;
            sendxev.xselection.selection = xev.xselectionrequest.selection;
            sendxev.xselection.time = xev.xselectionrequest.time;
            sendxev.xselection.target = xev.xselectionrequest.target;
            sendxev.xselection.property = xev.xselectionrequest.property;
            if (sendxev.xselection.target == _XGrCBTargets) {
                Atom target_list[4];
                target_list[0] = _XGrCBTargets;
                target_list[1] = _XGrCBUTF8;
                target_list[2] = _XGrCBText;
                target_list[3] = XA_STRING;
                //printf("SelectionRequest Targets\n");
                XChangeProperty(_XGrDisplay, xev.xselectionrequest.requestor,
                                sendxev.xselection.property,
                                XA_ATOM, 32, PropModeReplace,
                                (unsigned char*)&target_list, 4);
            } else if (sendxev.xselection.target == XA_STRING ||
                sendxev.xselection.target == _XGrCBText) {
                //printf("SelectionRequest Text\n");
                XChangeProperty(_XGrDisplay, xev.xselectionrequest.requestor,
                                sendxev.xselection.property,
                                sendxev.xselection.target, 8, PropModeReplace,
                                _XGrClipboard, _XGrClipboardByteLen);
            } else if (sendxev.xselection.target == _XGrCBUTF8) {
                //printf("SelectionRequest UTF8\n");
                XChangeProperty(_XGrDisplay, xev.xselectionrequest.requestor,
                                sendxev.xselection.property,
                                sendxev.xselection.target, 8, PropModeReplace,
                                _XGrClipboard, _XGrClipboardByteLen);
            } else {
                //printf("SelectionRequest None\n");
                sendxev.xselection.property = None;
            }
            XSendEvent(_XGrDisplay, xev.xselectionrequest.requestor, True, 0, &sendxev);
            break;

        case SelectionNotify:
            //printf("SelectionNotify %ld %ld\n", xev.xselection.target,
            //       xev.xselection.property);
            evaux.type = GREV_CBREPLY;
            evaux.kbstat = 0;
            evaux.p1 = _XGrLoadClipBoard();
            evaux.p2 = 0;
            evaux.p3 = 0;
            evaux.p4 = 0;
            GrEventEnqueue(&evaux);
            nev++;
            break;
        }
    }

    if (nev == 0) {
        XFlush(_XGrDisplay);
        usleep(1000L);   // wait 1 ms to not eat 100% cpu
    }

    return nev;
}

/**
 ** _GrMouseDetect - Returns true if a mouse is detected
 **
 ** For internal use only
 **/

int _GrMouseDetect(void)
{
    if (_XGrDisplay) {
        MOUINFO->msstatus = 1;
        return 1;
    }
    return 0;
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
    MOUINFO->xpos = imax(MOUINFO->xmin, imin(MOUINFO->xmax, x));
    MOUINFO->ypos = imax(MOUINFO->ymin, imin(MOUINFO->ymax, y));
    if (_XGrDisplay) {
        GrMouseUpdateCursor();
        if (MOUINFO->msstatus == 2) { // hack to be ok with window resize
            XWarpPointer(_XGrDisplay, _XGrWindow, _XGrWindow, 0, 0,
                         GrScreenX(), GrScreenY(), MOUINFO->xpos, MOUINFO->ypos);
        }
    }
}

/** Internal functions **/

static int _XKeyEventToGrKey(XKeyEvent *xkey, long *p1, long *p2)
{
    unsigned int state;
    char buffer[20];
    KeySym keysym;
    int count;

    state = xkey->state & (ShiftMask | ControlMask | Mod1Mask);
    count = XLookupString(xkey, buffer, sizeof(buffer),
                          &keysym, (XComposeStatus *)NULL);

    if ((count == 1) && ((state & Mod1Mask) == 0)) {
        *p1 = (unsigned char) buffer[0];
        *p2 = 1;
        return 1;
    }

    return _XGrX11KeyCode(keysym, state, p1, p2);
}

static int _Xutf8KeyEventToGrKey(XKeyEvent *xkey, long *p1, long *p2)
{
    unsigned int state;
    char buffer[20];
    KeySym keysym;
    Status status;
    char *cp1;
    int count;
    int i;

    state = xkey->state & (ShiftMask | ControlMask | Mod1Mask);
    count = Xutf8LookupString(_XGrXic, (XKeyPressedEvent *)xkey,
             buffer, sizeof(buffer), &keysym, &status);

    //printf("keysym %8x  state %8x status %8x\n", (int)keysym, (int)state, (int)status);
    
    if (((status == XLookupChars) || (status == XLookupBoth))
        && (count >= 1) && (count <= 4) && ((state & Mod1Mask) == 0)) {
        *p1 = 0;
        cp1 = (char *)p1;
        for(i=0; i < count; i++)
            cp1[i] = buffer[i];
        *p2 = count;
        return count;
    }

    if ((status == XLookupKeySym) || status == XLookupBoth) {
        return _XGrX11KeyCode(keysym, state, p1, p2);
    }

    return 0;
}

static int _XGrX11KeyCode(KeySym keysym, unsigned int state, long *p1, long *p2)
{
    KeyEntry *kp, *lastkp;

    switch (state) {
        case Mod1Mask:
            lastkp = &_KTMod1[sizeof(_KTMod1)/sizeof(_KTMod1[0])];
            for (kp = _KTMod1; kp < lastkp; kp = kp + 1) {
                if (keysym == kp->keysym) {
                    *p1 = kp->key;
                    *p2 = GRKEY_KEYCODE;
                    return 1;
                }
            }
            break;
        case ShiftMask:
            lastkp = &_KTShift[sizeof(_KTShift)/sizeof(_KTShift[0])];
            for (kp = _KTShift; kp < lastkp; kp = kp + 1) {
                if (keysym == kp->keysym) {
                    *p1 = kp->key;
                    *p2 = GRKEY_KEYCODE;
                    return 1;
                }
            }
            break;
        case 0:
            lastkp = &_KTVoid[sizeof(_KTVoid)/sizeof(_KTVoid[0])];
            for (kp = _KTVoid; kp < lastkp; kp = kp + 1) {
                if (keysym == kp->keysym) {
                    *p1 = kp->key;
                    *p2 = GRKEY_KEYCODE;
                    return 1;
                }
            }
            break;
        case ControlMask:
            lastkp = &_KTControl[sizeof(_KTControl)/sizeof(_KTControl[0])];
            for (kp = _KTControl; kp < lastkp; kp = kp + 1) {
                if (keysym == kp->keysym) {
                    *p1 = kp->key;
                    *p2 = GRKEY_KEYCODE;
                    return 1;
                }
            }
            break;
    }
    return 0;
}

static unsigned int _XGrModifierKey(KeySym keysym, int type)
{
    if (type == KeyPress) {
        switch (keysym) {
        case XK_Shift_L:    kbd_lastmod |= GRKBS_LEFTSHIFT; break;
        case XK_Shift_R:    kbd_lastmod |= GRKBS_RIGHTSHIFT; break;
        case XK_Control_L:
        case XK_Control_R:  kbd_lastmod |= GRKBS_CTRL; break;
        case XK_Alt_L:
        case XK_Alt_R:
        case XK_Meta_L:
        case XK_Meta_R:     kbd_lastmod |= GRKBS_ALT; break;
        case XK_Num_Lock:   kbd_lastmod |= GRKBS_NUMLOCK; break;
        case XK_Caps_Lock:  kbd_lastmod |= GRKBS_CAPSLOCK; break;
        //case XK_Scroll_Lock: kbd_lastmod |= GRKBS_SCROLLOCK; break;
        case XK_Insert:     kbd_lastmod |= GRKBS_INSERT; break;
        }
    }
    if (type == KeyRelease) {
        switch (keysym) {
        case XK_Shift_L:    kbd_lastmod &= ~GRKBS_LEFTSHIFT; break;
        case XK_Shift_R:    kbd_lastmod &= ~GRKBS_RIGHTSHIFT; break;
        case XK_Control_L:
        case XK_Control_R:  kbd_lastmod &= ~GRKBS_CTRL; break;
        case XK_Alt_L:
        case XK_Alt_R:
        case XK_Meta_L:
        case XK_Meta_R:     kbd_lastmod &= ~GRKBS_ALT; break;
        case XK_Num_Lock:   kbd_lastmod &= ~GRKBS_NUMLOCK; break;
        case XK_Caps_Lock:  kbd_lastmod &= ~GRKBS_CAPSLOCK; break;
        //case XK_Scroll_Lock: kbd_lastmod &= ~GRKBS_SCROLLOCK; break;
        case XK_Insert:     kbd_lastmod &= ~GRKBS_INSERT; break;
        }
    }
    return kbd_lastmod;
}

static XIMStyle _XGrChooseBetterStyle(XIMStyle style1, XIMStyle style2)
{
  XIMStyle s,t;
  XIMStyle preedit = XIMPreeditArea | XIMPreeditCallbacks |
    XIMPreeditPosition | XIMPreeditNothing | XIMPreeditNone;
  XIMStyle status = XIMStatusArea | XIMStatusCallbacks |
    XIMStatusNothing | XIMStatusNone;

  if (style1 == 0) return style2;
  if (style2 == 0) return style1;
  if ((style1 & (preedit | status)) == (style2 & (preedit | status)))
    return style1;
  s = style1 & preedit;
  t = style2 & preedit;
  if (s != t) {
    if ((s | t) & XIMPreeditCallbacks)
      return (s == XIMPreeditCallbacks)?style1:style2;
    else if ((s | t) & XIMPreeditPosition)
      return (s == XIMPreeditPosition)?style1:style2;
    else if ((s | t) & XIMPreeditArea)
      return (s == XIMPreeditArea)?style1:style2;
    else if ((s | t) & XIMPreeditNothing)
      return (s == XIMPreeditNothing)?style1:style2;
  }
  else {
    s = style1 & status;
    t = style2 & status;
    if ((s | t) & XIMStatusCallbacks)
      return (s == XIMStatusCallbacks)?style1:style2;
    else if ((s | t) & XIMStatusArea)
      return (s == XIMStatusArea)?style1:style2;
    else if ((s | t) & XIMStatusNothing)
      return (s == XIMStatusNothing)?style1:style2;
  }
  return style2;
}

static int _XGrOpenXIMandXIC(Display *dpy, Window win, XIM *im, XIC *ic)
{
  XIMStyle app_supported_styles;
  XIMStyles *im_supported_styles;
  XIMStyle style;
  XIMStyle best_style;
  int i;

  if (setlocale(LC_ALL, "") == NULL) return 0;

  if (!XSupportsLocale()) return 0;
  if (XSetLocaleModifiers("") == NULL) return 0;
  if ((*im = XOpenIM(dpy, NULL, NULL, NULL)) == NULL) return 0;

  app_supported_styles = XIMPreeditNone | XIMPreeditNothing;
  app_supported_styles |= XIMStatusNone | XIMStatusNothing;

  XGetIMValues(*im, XNQueryInputStyle, &im_supported_styles, NULL);
  best_style = 0;

  for (i=0; i < im_supported_styles->count_styles; i++) {
    style = im_supported_styles->supported_styles[i];
    if ((style & app_supported_styles) == style)
      best_style = _XGrChooseBetterStyle(style, best_style);
  }
  XFree(im_supported_styles);

  if (best_style == 0) {
    XCloseIM(*im);
    *im = NULL;
    return 0;
  }

  *ic = XCreateIC(*im,
          XNInputStyle, best_style,
          XNClientWindow, win,
          NULL);

  if (ic == NULL) {
    XCloseIM(*im);
    *im = NULL;
    return 0;
  }

  XSetICFocus(*ic);
  return 1;
}
