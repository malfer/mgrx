/**
 ** libxwin.h - GRX library X Windows private include file
 **
 ** Author:     Ulrich Leodolter
 ** E-mail:     ulrich@lab1.psy.univie.ac.at
 ** Date:       Thu Sep 28 09:46:21 1995
 ** RCSId:      $Id$
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
 ** 170714 M.Alvarez, Use BStore only when GREV_EXPOSE events are not generated
 ** 211110 M.Alvarez, Support window resize
 ** 220206 M.Alvarez, X11 clipboard support
 **/

#ifndef _LIBXWIN_H_
#define _LIBXWIN_H_

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <unistd.h>

#if defined(XF86DGA_DRIVER)
#include <X11/extensions/xf86dga.h>
extern int _XGrWindowedMode;
#else
#define _XGrWindowedMode 1
#endif

extern Display *        _XGrDisplay;
extern int              _XGrScreen;
extern Colormap         _XGrColormap;
extern Window           _XGrWindow;
extern Pixmap           _XGrBitmap;
extern Pixmap           _XGrPattern;
extern Pixmap           _XGrBStore;
extern Drawable         _XGRActDrawable;
extern GC               _XGrGC;
extern GC               _XGrBitmapGC;
extern GC               _XGrPatternGC;
extern GC               _XGrDefaultGC;
extern XIM              _XGrXim;
extern XIC              _XGrXic;
extern unsigned long    _XGrForeColor;
extern unsigned long    _XGrBackColor;
extern unsigned int     _XGrColorOper;
extern unsigned int     _XGrDepth;
extern unsigned int     _XGrBitsPerPixel;
extern unsigned int     _XGrMaxWidth;
extern unsigned int     _XGrMaxHeight;
extern unsigned int     _XGrCurWidth;
extern unsigned int     _XGrCurHeight;
extern int              _XGrBStoreInited;
extern int              _XGrFullScreen;
extern int              _XGrGenExposeEvents;
extern int              _XGrGenWMEndEvents;
extern int              _XGrGenWSzChgEvents;
extern Atom             _wmDeleteWindow;
extern int              _XGrUserHadSetWName;

extern unsigned long    _XGrColorPlanes[8];
extern unsigned int     _XGrColorNumPlanes;
extern unsigned long    _XGrColorPixels[2];
extern unsigned int     _XGrColorNumPixels;
extern char *           _XGrClassNames[6];

extern Atom             _XGrCBTargets;
extern Atom             _XGrCBText;
extern Atom             _XGrCBUTF8;
extern Atom             _XGrCBSelection;
extern Atom             _XGrCBIncr;
extern Atom             _XGrCBProperty;
extern int              _XGrCBOwnSelection;
extern unsigned char *  _XGrClipboard;
extern int              _XGrClipboardLen;
extern int              _XGrClipboardByteLen;

void _XGrCopyBStore(int x, int y, int width, int lenght);

void _XGrIniClipBoard(void);
int _XGrLoadClipBoard(void);

#endif
