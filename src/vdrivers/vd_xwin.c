/**
 ** vd_xwin.c ---- the standard X Window driver
 **
 ** Author:     Ulrich Leodolter
 ** E-mail:     ulrich@lab1.psy.univie.ac.at
 ** Date:       Thu Sep 28 09:29:26 1995
 ** RCSId:      $Id: vd_xwin.c 1.2 1995/11/19 19:28:12 ulrich Exp $
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
 ** 071201 M.Alvarez, Added videomodes for wide monitors
 ** 071201 M.Alvarez, The modes higher than the X resolution 
 **                   are made 'non-present'
 ** 071201 M.Alvarez, go to fullscreen if w,h == X resolution
 **                   GR_biggest_graphics is honored
 ** 170714 M.Alvarez, Use BStore only if not generated GREV_EXPOSE events
 ** 191022 M.Alvarez, Added SubstructureRedirectMask to go/return from fullsecreen
 ** 191112 M.Alvarez, Added code to generate GREV_WMEND events
 ** 211110 M.Alvarez, support window resize
 ** 211206 M.Alvarez, when window resizing is active create a big backing store and
 **                   not recreate it in every setmode, so resizing is smooth
 ** 220206 M.Alvarez, X11 clipboard support
 **/

#include "libgrx.h"
#include "libxwin.h"
#include "grdriver.h"
#include "arith.h"

Display *       _XGrDisplay = NULL;
int             _XGrScreen;
Window          _XGrWindow = None;
Pixmap          _XGrBitmap;
Pixmap          _XGrPattern;
Pixmap          _XGrBStore;
Drawable        _XGRActDrawable;
Colormap        _XGrColormap;
GC              _XGrGC;
GC              _XGrBitmapGC;
GC              _XGrPatternGC;
GC              _XGrDefaultGC;
XIM             _XGrXim = NULL;
XIC             _XGrXic = NULL;
unsigned long   _XGrForeColor;
unsigned long   _XGrBackColor;
unsigned int    _XGrColorOper;
unsigned int    _XGrDepth;
unsigned int    _XGrBitsPerPixel;
unsigned int	_XGrScanlinePad;
unsigned int    _XGrMaxWidth;
unsigned int    _XGrMaxHeight;
unsigned int    _XGrCurWidth;
unsigned int    _XGrCurHeight;
int             _XGrBStoreInited = 0;
int             _XGrFullScreen = 0;
int             _XGrWMaped = 0;
int             _XGrGenExposeEvents = GR_GEN_NO;
int             _XGrGenWMEndEvents = GR_GEN_NO;
int             _XGrGenWSzChgEvents = FALSE;
Atom            _wmDeleteWindow;
int             _XGrUserHadSetWName = 0;

unsigned long   _XGrColorPlanes[8];
unsigned int    _XGrColorNumPlanes;
unsigned long   _XGrColorPixels[2];
unsigned int    _XGrColorNumPixels;

char *_XGrClassNames[6] = {
    "StaticGray",
    "GrayScale",
    "StaticColor",
    "PseudoColor",
    "TrueColor",
    "DirectColor"
};

static void setbank(int bk);
static void setrwbanks(int rb, int wb);
static void loadcolor(int c, int r, int g, int b);
static int setmode(GrVideoMode *mp, int noclear);

GrVideoModeExt grtextext = {
    GR_frameText,                       /* frame driver */
    NULL,                               /* frame driver override */
    NULL,                               /* frame buffer address */
    { 0, 0, 0 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    setmode,                            /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    NULL,                               /* bank set function */
    NULL,                               /* double bank set function */
    NULL                                /* color loader */
};

static GrVideoModeExt grxwinext = {
    GR_frameUndef,                      /* frame driver */
    NULL,                               /* frame driver override */
    NULL,                               /* frame buffer address */
    { 0, 0, 0 },                        /* color precisions */
    { 0, 0, 0 },                        /* color component bit positions */
    0,                                  /* mode flag bits */
    setmode,                            /* mode set */
    NULL,                               /* virtual size set */
    NULL,                               /* virtual scroll */
    setbank,                            /* bank set function */
    setrwbanks,                         /* double bank set */
    loadcolor                           /* color loader */
};

static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext  */
    {  TRUE,  8,   80,   25,  0x00,    80, 1,    &grtextext  },
    { FALSE,  0,  320,  240,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0,  640,  480,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0,  800,  600,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 1024,  768,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 1280, 1024,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 1600, 1200,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 1440,  900,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 1680, 1050,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 1920, 1080,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 1920, 1200,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 2560, 1440,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 3840, 2160,  0x00,     0, 0,    &grxwinext  },
    { FALSE,  0, 9999, 9999,  0x00,     0, 0,    &grxwinext  }
};

static void goto_fullscreen(Display *dsp, Window win)
{
    XEvent xev;
    Atom wm_state = XInternAtom(dsp, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(dsp, "_NET_WM_STATE_FULLSCREEN", False);
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
    xev.xclient.data.l[1] = fullscreen;
    xev.xclient.data.l[2] = 0;
    XSendEvent(dsp, DefaultRootWindow(dsp), False,
               SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}

static void returnfrom_fullscreen(Display *dsp, Window win)
{
    XEvent xev;
    Atom wm_state = XInternAtom(dsp, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(dsp, "_NET_WM_STATE_FULLSCREEN", False);
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 0; // _NET_WM_STATE_REMOVE
    xev.xclient.data.l[1] = fullscreen;
    xev.xclient.data.l[2] = 0;
    XSendEvent(dsp, DefaultRootWindow(dsp), False,
               SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}

static void setbank(int bk)
{}

static void setrwbanks(int rb, int wb)
{}

static void loadcolor(int c, int r, int g, int b)
{
    GRX_ENTER();
    if (_XGrDisplay != NULL
        && _XGrColormap != None
        && !(_XGrColorNumPixels == 1
        && (c < _XGrColorPixels[0] || c > _XGrColorPixels[1])) ) {
        XColor xcolor;
    
        DBGPRINTF(DBG_DRIVER,("Setting X11 color %d to r=%d, g=%d, b=%d\n",c,r,g,b));
        xcolor.pixel = c;
        xcolor.red   = r * 257;
        xcolor.green = g * 257;
        xcolor.blue  = b * 257;
        xcolor.flags = DoRed|DoGreen|DoBlue;
        XStoreColor(_XGrDisplay, _XGrColormap, &xcolor);
    }
    GRX_LEAVE();
}

static int setmode(GrVideoMode *mp, int noclear)
{
    XEvent xevent;
    int res = FALSE;
    
    GRX_ENTER();
    if (_XGrDisplay == NULL || _XGrWindow == None) goto done;
    
    //printf("setmode %d %d\n", mp->width, mp->height);

    if (_XGrFullScreen) {
        returnfrom_fullscreen(_XGrDisplay, _XGrWindow);
        _XGrFullScreen = 0;
    }

    if (_XGrWMaped && (!_XGrGenWSzChgEvents ||
        mp->extinfo->mode == GR_frameText)) {
        XUnmapWindow(_XGrDisplay, _XGrWindow);
        _XGrWMaped = 0;
    }
    
    if (_XGrBStoreInited && !_XGrWMaped) {
        XFreePixmap(_XGrDisplay, _XGrBStore);
        _XGrBStoreInited = 0;
    }
    
    if (mp->extinfo->mode != GR_frameText) {
        XSizeHints *hints;
        char name[100];
        
        if (!_XGrBStoreInited) {
            _XGrGenExposeEvents = GR_GEN_NO;
            if (!_XGrGenWSzChgEvents) {
                _XGrBStore = XCreatePixmap(_XGrDisplay, _XGrWindow, mp->width,
                                           mp->height, _XGrDepth);
                XFillRectangle(_XGrDisplay, _XGrBStore, _XGrGC, 0, 0,
                               mp->width, mp->height);
            } else {
                _XGrBStore = XCreatePixmap(_XGrDisplay, _XGrWindow, _XGrMaxWidth,
                                          _XGrMaxHeight, _XGrDepth);
                XFillRectangle(_XGrDisplay, _XGrBStore, _XGrGC, 0, 0,
                               _XGrMaxWidth, _XGrMaxHeight);
            }
            _XGRActDrawable = (Drawable) _XGrBStore;
            _XGrBStoreInited = 1;
        }

        //here because I used it in some tests, don't uncomment
        //XFillRectangle(_XGrDisplay, _XGrBStore, _XGrGC, 0, 0, mp->width, mp->height);
        //XFillRectangle(_XGrDisplay, _XGrWindow, _XGrGC, 0, 0, mp->width, mp->height);
        //XCopyArea(_XGrDisplay, _XGrWindow, _XGrBStore,
        //          _XGrDefaultGC, 0, 0, mp->width, mp->height, 0, 0);
        //_XGRActDrawable = (Drawable *) &_XGrWindow;

        grxwinext.frame = (char *) &_XGRActDrawable;

        if (!_XGrUserHadSetWName) {
            sprintf (name, "mgrx %dx%dx%d", mp->width, mp->height, mp->bpp);
            XStoreName(_XGrDisplay, _XGrWindow, name);
            XSetIconName(_XGrDisplay, _XGrWindow, name);
        }
        
        if (!_XGrWMaped) {
            hints = XAllocSizeHints();
            if (hints) {
                hints->min_width = mp->width;
                hints->min_height = mp->height;
                hints->max_width = mp->width;
                hints->max_height = mp->height;
                hints->flags = PMaxSize | PMinSize;
                XSetWMNormalHints(_XGrDisplay, _XGrWindow, hints);
            }
            XResizeWindow(_XGrDisplay, _XGrWindow, mp->width, mp->height);
            XMapRaised(_XGrDisplay, _XGrWindow);
            _XGrWMaped = 1;
            
            /* Wait until window appears on screen */
            while (1) {
                XNextEvent(_XGrDisplay, &xevent);
                if (xevent.type == MapNotify) break;
            }

            if ((mp->width == _XGrMaxWidth) && (mp->height == _XGrMaxHeight)) {
                // Go to fullscreen if w,h == X resolution
                goto_fullscreen(_XGrDisplay, _XGrWindow);
                _XGrFullScreen = 1;
            }
            else if (hints) {
                // Important we must do this after the window is showed,
                // if not some  WM resize back again (like the kde one)
                if (_XGrGenWSzChgEvents) {
                    hints->min_width = DRVINFO->minwgw;
                    hints->min_height = DRVINFO->minwgh;
                    hints->max_width = _XGrMaxWidth;
                    hints->max_height = _XGrMaxHeight;
                    hints->flags = PMaxSize | PMinSize;
                    XSetWMNormalHints(_XGrDisplay, _XGrWindow, hints);
                    XResizeWindow(_XGrDisplay, _XGrWindow, mp->width, mp->height);
                }
            }
            if (hints) XFree(hints);
        }

        _XGrCurWidth = mp->width;
        _XGrCurHeight = mp->height;
    }
    res = TRUE;
done:
    GRX_RETURN(res);
}

static int (*previous_error_handler)(Display *dpy, XErrorEvent *ev) = NULL;

static int error_handler(Display *dpy, XErrorEvent *ev)
{
    char buffer[200];
    
    /*
     * This error is triggerd in fd_xwin.c:bltv2r() if the source rectangle
     * is not fully contained in the root window
     */
    if (ev->request_code == X_GetImage && ev->error_code == BadMatch)
        return 0;

    if (previous_error_handler)
        return (*previous_error_handler) (dpy, ev);
    
    XGetErrorText(dpy, ev->error_code, buffer, sizeof(buffer)),
    fprintf(stderr, "MGRX: XError: %s XID=%ld request_code=%d serial=%lu\n",
            buffer, ev->resourceid, ev->request_code, ev->serial);
    return 0;
}

static GrVideoMode * _xw_selectmode( GrVideoDriver * drv, int w, int h, int bpp,
                                     int txt, unsigned int * ep )
{
    GrVideoMode *mp, *res;
    GRX_ENTER();
    if (txt) {
        res = _gr_selectmode(drv, w, h, bpp, txt, ep);
        goto done;
    }
    for (mp = &modes[1]; mp < &modes[itemsof(modes)-1]; mp++) {
        if ( mp->present && mp->width == w && mp->height == h) {
            res = _gr_selectmode (drv,w,h,bpp,txt,ep);
            goto done;
        }
    }
    /* no predefined mode found. Create a new mode */
    mp->present = TRUE;
    mp->width = (w > _XGrMaxWidth) ? _XGrMaxWidth : w;
    mp->height = (h > _XGrMaxHeight) ? _XGrMaxHeight : h;
    mp->lineoffset = (mp->width * mp->bpp) / 8;
    res = _gr_selectmode (drv,w,h,bpp,txt,ep);
done:
    GRX_RETURN(res);
}

static int detect(void)
{
    int res;
    GRX_ENTER();
    res = (getenv("DISPLAY") ? TRUE : FALSE);
    GRX_RETURN(res);
}

static int init(char *options)
{
    Window root;
    XSetWindowAttributes attr;
    unsigned long mask;
    Visual *visual;
    XPixmapFormatValues *pfmt;
    int pfmt_count;
    GrVideoMode *mp;
    unsigned int depth, bpp, pad;
    int private_colormap = FALSE;
    int i, j, res = FALSE;
    
    GRX_ENTER();
    _XGrDisplay = XOpenDisplay ("");
    if (_XGrDisplay == NULL) goto done; /* FALSE */
    
    if (options) {
        char opt[101] = {0};
        char *token;
        strncpy(opt, options, 100);
        token = strtok(opt, ":");
        while (token != NULL) {
            if (strncmp ("privcmap", token, 8) == 0) private_colormap = TRUE;
            if (strncmp ("rszwin", token, 6) == 0) _XGrGenWSzChgEvents = TRUE;
            token = strtok(NULL, ":");
        }
    }

    if (_XGrGenWSzChgEvents) {
        _GrVideoDriverXWIN.drvflags |= GR_DRIVERF_WINDOW_RESIZE;
    } else {
        _GrVideoDriverXWIN.drvflags &= ~GR_DRIVERF_WINDOW_RESIZE;
    }

    previous_error_handler = XSetErrorHandler(error_handler);

    _XGrScreen = DefaultScreen(_XGrDisplay);
    _XGrDepth = depth = DefaultDepth(_XGrDisplay, _XGrScreen);
    _XGrMaxWidth = DisplayWidth(_XGrDisplay, _XGrScreen);
    _XGrMaxHeight = DisplayHeight(_XGrDisplay, _XGrScreen);
    // TODO
    //_XGrMaxWinWidth = min(_XGrMaxWidth, DRVINFO->biggw);
    //_XGrMaxWinHeight = min(_XGrMaxHeight, DRVINFO->biggh);
    
    visual = DefaultVisual(_XGrDisplay, _XGrScreen);
    root = RootWindow(_XGrDisplay, _XGrScreen);
    
    _XGrColorNumPlanes = 0;
    _XGrColorNumPixels = 0;
    
    if (visual->class == PseudoColor && (depth == 4 || depth == 8)) {
        if (!private_colormap) {
            /* Allocate at least 4 planes (= 16 colors) of writable cells */
            _XGrColormap = DefaultColormap(_XGrDisplay, _XGrScreen);
            for (i = depth - 1; i >= 4; i--) {
                if (XAllocColorCells(_XGrDisplay, _XGrColormap, True, /* contiguous */
                    _XGrColorPlanes, i, _XGrColorPixels, 1)) {
                    _XGrColorNumPlanes = i;
                    _XGrColorNumPixels = 1;
                    _GR_firstFreeColor = _XGrColorPixels[0];
                    _XGrColorPixels[1] = _XGrColorPixels[0];
                    for (j = 0; j < _XGrColorNumPlanes; j++) {
                        _XGrColorPixels[1] |= _XGrColorPlanes[j];
                    }
                _GR_lastFreeColor = _XGrColorPixels[1]-1;
                break;
                }
            }
            if (_XGrColorNumPlanes == 0)
                private_colormap = TRUE;
        }
        if (private_colormap) {
            _XGrColormap = XCreateColormap(_XGrDisplay, root, visual, AllocAll);
        }
        grxwinext.cprec[0] = visual->bits_per_rgb;
        grxwinext.cprec[1] = visual->bits_per_rgb;
        grxwinext.cprec[2] = visual->bits_per_rgb;
        grxwinext.cpos[0] = 0;
        grxwinext.cpos[1] = 0;
        grxwinext.cpos[2] = 0;
        /* loadcolor (0, 0, 0, 0); */       /* load black */
    } else if (visual->class == TrueColor
        && (depth == 8 || depth == 15 || depth == 16 || depth == 24)) {
        int i, pos, prec;
        unsigned long mask[3];
    
        _XGrColormap = (Colormap)0;
        mask[0] = visual->red_mask;
        mask[1] = visual->green_mask;
        mask[2] = visual->blue_mask;
        for (i = 0; i < 3; i++) {
            for (pos = 0; !(mask[i] & 1); pos++, mask[i]>>=1);
            for (prec = 0; (mask[i] & 1); prec++, mask[i]>>=1);
            grxwinext.cprec[i] = prec;
            grxwinext.cpos[i] = pos;
        }
    } else {
        XCloseDisplay(_XGrDisplay);
        fprintf(stderr, "X11 driver init: Visual class=%s depth=%d not supported\n",
                _XGrClassNames[visual->class], depth);
        goto done;
    }

    _XGrForeColor = GrNOCOLOR;    /* force XSetForeground */
    _XGrBackColor = GrNOCOLOR;    /* force XSetBackground */
    _XGrColorOper = C_WRITE;
        
    mask = 0L;
    attr.backing_store = NotUseful;
    mask |= CWBackingStore;
    if (_XGrColormap) {
        attr.colormap = _XGrColormap;
        mask |= CWColormap;
    }
    // I think is better to have this undefined, so in windows resizes the WM
    // doesn't apply background and the transition is smothy
    //attr.background_pixel = 0L;
    //mask |= CWBackPixel;
    //attr.border_pixel = 0L;
    //mask |= CWBorderPixel;
    attr.event_mask = (StructureNotifyMask
        | KeyPressMask
        | KeyReleaseMask
        | ButtonPressMask
        | ButtonReleaseMask
        | ButtonMotionMask
        | ExposureMask
        | PointerMotionMask);
    mask |= CWEventMask;
    _XGrWindow = XCreateWindow(_XGrDisplay,
                               root,             /* parent */
                               0,                /* x */
                               0,                /* y */
                               1,                /* width */
                               1,                /* height */
                               0,                /* border_width */
                               depth,            /* depth */
                               InputOutput,      /* class */
                               visual,           /* visual */
                               mask,             /* valuemask */
                               &attr);           /* attributes */
    _XGrBitmap = XCreatePixmap(_XGrDisplay, root, 128, 128, 1);
    _XGrBitmapGC = XCreateGC(_XGrDisplay, _XGrBitmap, 0L, NULL);
    _XGrPattern = XCreatePixmap(_XGrDisplay, root, 8, 1, 1);
    _XGrPatternGC = XCreateGC(_XGrDisplay, _XGrPattern, 0L, NULL);
    _XGrDefaultGC = DefaultGC(_XGrDisplay, _XGrScreen);
    _XGrGC = XCreateGC (_XGrDisplay, _XGrWindow, 0L, NULL);
    _XGrWMaped = 0;
        
    _wmDeleteWindow = XInternAtom(_XGrDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(_XGrDisplay, _XGrWindow, &_wmDeleteWindow, 1);
        
    /* is this required ?? */
    if (_XGrColormap) {
        XInstallColormap(_XGrDisplay, _XGrColormap);
        XSetWindowColormap(_XGrDisplay, _XGrWindow, _XGrColormap);
    }
        
    pfmt = XListPixmapFormats(_XGrDisplay, &pfmt_count);
    if (!pfmt || pfmt_count <= 0) {
        XCloseDisplay(_XGrDisplay);
        fprintf(stderr, "X11 driver init: cannot list pixmap formats\n");
        goto done;
    }
        
    bpp = 0;
    pad = 0;
    for (i = 0; i < pfmt_count; i++) {
        if (pfmt[i].depth == depth) {
            bpp = pfmt[i].bits_per_pixel;
            pad = pfmt[i].scanline_pad;
            break;
        }
    }
    XFree(pfmt);
    if (!bpp) {
        XCloseDisplay(_XGrDisplay);
        fprintf(stderr, "X11 driver init: cannot find pixmap format\n");
        goto done;
    }
        
    switch (depth) {
        case  1: grxwinext.mode = GR_frameXWIN1;
                 break;
        case  4: grxwinext.mode = GR_frameXWIN4;
                 break;
        case  8: grxwinext.mode = GR_frameXWIN8;
                 break;
        case 15:
        case 16: grxwinext.mode = GR_frameXWIN16;
                 break;
        case 24: switch (bpp) {
                    case 24: grxwinext.mode = GR_frameXWIN24;
                             break;
                    case 32: grxwinext.mode = (visual->red_mask & 0xff000000) ?
                                GR_frameXWIN32H : GR_frameXWIN32L;
                             break;
                 }
                 break;
    }
        
    _XGrBitsPerPixel = bpp;
    _XGrScanlinePad  = pad;
        
    /* fixed size modes */
    for (mp = &modes[1]; mp < &modes[itemsof(modes)-1]; mp++) {
        mp->present = TRUE;
        mp->bpp = bpp;
        mp->lineoffset = (mp->width * bpp) / 8;
        if (mp->width > _XGrMaxWidth) mp->present = FALSE;
        if (mp->height > _XGrMaxHeight) mp->present = FALSE;
    }
    /* this is the variable size mode */
    mp->present = FALSE;
    mp->bpp = bpp;
    mp->lineoffset = 0;

    _XGrIniClipBoard();

    res = TRUE;
done:
    GRX_RETURN( res );
}

static void reset(void)
{
    GRX_ENTER();
    if (previous_error_handler) XSetErrorHandler(previous_error_handler);
    if (_XGrDisplay) XCloseDisplay(_XGrDisplay);
    _XGrDisplay = NULL;
    _XGrWindow = None;
    _XGrWMaped = 0;
    _GR_firstFreeColor =  0;
    _GR_lastFreeColor  = -1;
    _XGrGenWSzChgEvents = FALSE;
    GRX_LEAVE();
}

int genexpose(int gen)
{
    if (gen == _XGrGenExposeEvents) return _XGrGenExposeEvents;

    if (_XGrWindow == None || !_XGrBStoreInited) {
        _XGrGenExposeEvents = GR_GEN_NO;
        return _XGrGenExposeEvents;
    }

    _XGrGenExposeEvents = gen;
    if (_XGrGenExposeEvents == GR_GEN_NO)
        _XGRActDrawable = (Drawable) _XGrBStore;
    else
        _XGRActDrawable = (Drawable) _XGrWindow;
    return _XGrGenExposeEvents;
}

int genwmend(int gen)
{
    _XGrGenWMEndEvents = gen;
    return _XGrGenWMEndEvents;
}

GrVideoDriver _GrVideoDriverXWIN = {
    "xwin",                             /* name */
    GR_XWIN,                            /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    detect,                             /* detection routine */
    init,                               /* initialization routine */
    reset,                              /* reset routine */
    _xw_selectmode,                     /* special mode select routine */
    GR_DRIVERF_USER_RESOLUTION,         /* arbitrary resolution possible */
    0,                                  /* inputdriver, not used by now */
    genexpose,                          /* generate GREV_EXPOSE events */
    genwmend,                           /* generate GREV_WMEND events */
    NULL                                /* generate GREV_FRAME events */
};
