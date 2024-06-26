/**
 ** demomgrx.c ---- MGRX Test programs launcher
 **
 ** Copyright (C) 2000,2001,2005,2008,2019-2023,2024 Mariano Alvarez Fernandez
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
#include "mgrx.h"
#include "mgrxkeys.h"
#include "gfaz.h"
#include "drawing.h"

/* default mode */

static int gwidth = 640;
static int gheight = 480;
static int gbpp = 16;

char *mgrxversion = "MGRX 1.5.1";
char *wintitle    = "MGRX 1.5.1, the graphics library";
char *animatedtext[2] = {
    "MGRX 1.5.1, the graphics library for DJGPPv2, Linux, X11, Wayland and Win32",
    "Hello world    Привет мир    Γειά σου Κόσμε    Hola mundo" };

#if defined(__XWIN__) || defined(__WIN32__) || defined(__WAYLAND__)
static int need_restart = 0;
#else
static int need_restart = 1;
#endif

#if defined(__MSDOS__) || defined(__WIN32__)
#define PPMIMG1   "..\\testimg\\pnmtest.ppm"
#define PPMIMG2   "..\\testimg\\pnmtest2.ppm"
#define JPGIMGBG  "..\\testimg\\jpeg3.jpg"
#define PPMIMGOUT "..\\testimg\\demomgrx.ppm"
#else
#define PPMIMG1   "../testimg/pnmtest.ppm"
#define PPMIMG2   "../testimg/pnmtest2.ppm"
#define JPGIMGBG  "../testimg/jpeg3.jpg"
#define PPMIMGOUT "../testimg/demogrx.ppm"
#endif

#define NDEMOS 45

#define ID_ARCTEST   1
#define ID_BB1TEST   2
#define ID_BIGGTEST  3
#define ID_BLITTEST  4
#define ID_CIRCTEST  5
#define ID_CLIPTEST  6
#define ID_CLRTABLE  7
#define ID_COLOROPS  8
#define ID_CURSTEST  9
#define ID_CUSPTEST 10
#define ID_DEMOINTL 11
#define ID_FNTDEMO1 12
#define ID_FNTDEMO2 13
#define ID_FNTDEMO3 14
#define ID_FNTDEMO4 15
#define ID_FONTTEST 16
#define ID_GRADTEST 17
#define ID_JPGTEST  18
#define ID_KEYS     19
#define ID_LIFE     20
#define ID_LIFEDB   21
#define ID_LINETEST 22
#define ID_MOUSETST 23
#define ID_MPOLTEST 24
#define ID_PATRTEST 25
#define ID_PCIRCTST 26
#define ID_PIXMTEST 27
#define ID_PNMTEST  28
#define ID_PNGTEST  29
#define ID_POLYTEST 30
#define ID_POLYTEDB 31
#define ID_RGBTEST  32
#define ID_SBCTEST  33
#define ID_SCLTEST  34
#define ID_SCROLTST 35
#define ID_SPEEDTST 36
#define ID_SPEEDTS2 37
#define ID_TEXTPATT 38
#define ID_WINCLIP  39
#define ID_WINTEST  40
#define ID_WRSZTEST 41
#define ID_MODETEST 50
#define ID_PAGE1    81
#define ID_PAGE2    82
#define ID_EXIT     99

typedef struct {
    int cid;
    char *prog;
    char *text;
} ProgTable;

static ProgTable ptable[NDEMOS] = {
    {ID_ARCTEST, "arctest", "arctest.c -> test arc outline and filled arc drawing"},
    {ID_BB1TEST, "bb1test", "bb1test.c -> test GrBitBlt1bpp routine"},
    {ID_BIGGTEST, "biggtest", "biggtest.c -> test GR_biggest_graphics"},
    {ID_BLITTEST, "blittest", "blittest.c -> test various bitblt-s"},
    {ID_CIRCTEST, "circtest", "circtest.c -> test circle and ellipse rendering"},
    {ID_CLIPTEST, "cliptest", "cliptest.c -> test clipping"},
    {ID_CLRTABLE, "clrtable", "clrtable.c -> test color tables"},
    {ID_COLOROPS, "colorops", "colorops.c -> test WRITE, XOR, OR, and AND draw modes"},
    {ID_CURSTEST, "curstest", "curstest.c -> test cursors"},
    {ID_CUSPTEST, "cusptest", "cusptest.c -> test custom line shapes"},
    {ID_DEMOINTL, "demointl", "demointl.c -> test intl support"},
    {ID_FNTDEMO1, "fontdemo ncen22b.fnt", "fontdemo ncen22b.fnt -> test a GRX font"},
    {ID_FNTDEMO2, "fontdemo ter-114b.res", "fontdemo ter-114b.res -> test a RES font"},
    {ID_FNTDEMO3, "fontdemo ter-114n.fna", "fontdemo ter-114n.fna -> test a FNA font"},
    {ID_FNTDEMO4, "fontdemo ter-114v.psf", "fontdemo ter-114v.psf -> test a PSF font"},
    {ID_FONTTEST, "fonttest", "fonttest.c -> test text drawing"},
    {ID_GRADTEST, "gradtest", "gradtest.c -> test gradients"},
    {ID_JPGTEST, "jpgtest", "jpgtext.c -> text context to jpeg functions"},
    {ID_KEYS, "keys", "keys.c -> test keyboard input"},
    {ID_LIFE, "life", "life.c -> Conway's life program"},
    {ID_LIFEDB, "life_db", "life_db.c -> Double buffer version of life"},
    {ID_LINETEST, "linetest", "linetest.c -> test wide and patterned lines"},
    {ID_MOUSETST, "mousetst", "mousetst.c -> test mouse cursor and mouse/keyboard input"},
    {ID_MPOLTEST, "mpoltest", "mpoltest.c -> test multipolygons"},
    {ID_PATRTEST, "patrtest", "patrtest.c -> test pattern filled and patterned shapes"},
    {ID_PCIRCTST, "pcirctst", "pcirctst.c -> test custom circle and ellipse rendering"},
    {ID_PIXMTEST, "pixmtest", "pixmtest.c -> test pixmap functions"},
    {ID_PNMTEST, "pnmtest", "pnmtext.c -> text context to pnm functions"},
    {ID_PNGTEST, "pngtest", "pngtext.c -> text context to png functions"},
    {ID_POLYTEST, "polytest", "polytest.c -> test polygon rendering"},
    {ID_POLYTEDB, "polytedb", "polytedb.c -> double buffer version of polytest"},
    {ID_RGBTEST, "rgbtest", "rgbtest.c -> show 256 color RGB palette"},
    {ID_SBCTEST, "sbctest", "sbctest.c -> test subcontext operations"},
    {ID_SCLTEST, "scltest", "scltest.c -> test scanline operations"},
    {ID_SCROLTST, "scroltst", "scroltst.c -> test virtual screen set/scroll"},
    {ID_SPEEDTST, "speedtst", "speedtst.c -> check all available frame drivers speed"},
    {ID_SPEEDTS2, "speedts2", "speedts2.c -> check all available frame drivers speed (v2)"},
    {ID_TEXTPATT, "textpatt", "textpatt.c -> test patterned text"},
    {ID_WINCLIP, "winclip", "winclip.c -> clip a drawing to various windows (contexts)"},
    {ID_WINTEST, "wintest", "wintest.c -> test window (context) mapping"},
    {ID_WRSZTEST, "wrsztest", "wrsztest.c -> test window resizing (X11 & W32)"},
    {ID_MODETEST, "modetest", "modetest.c -> test all available graphics modes"},
    {ID_PAGE1, "", "Change to page 1"},
    {ID_PAGE2, "", "Change to page 2"},
    {ID_EXIT, "", "Exit MGRX test programs launcher"}
};

#define PX0 10
#define PX1 115
#define PX2 220
#define PY0 10
#define PY1 54
#define PY2 98
#define PY3 142
#define PY4 186
#define PY5 230
#define PY6 274
#define PY7 318
#define PY8 362

#define NBUTTONSP1 26

static Button bp1[NBUTTONSP1] = {
    {PX0, PY0, 100, 40, IND_BLUE, IND_YELLOW, "ArcTest", BSTATUS_SELECTED, ID_ARCTEST},
    {PX0, PY1, 100, 40, IND_BLUE, IND_YELLOW, "Bb1Test", 0, ID_BB1TEST},
    {PX0, PY2, 100, 40, IND_BLUE, IND_YELLOW, "BigGTest", 0, ID_BIGGTEST},
    {PX0, PY3, 100, 40, IND_BLUE, IND_YELLOW, "BlitTest", 0, ID_BLITTEST},
    {PX0, PY4, 100, 40, IND_BLUE, IND_YELLOW, "CircTest", 0, ID_CIRCTEST},
    {PX0, PY5, 100, 40, IND_BLUE, IND_YELLOW, "ClipTest", 0, ID_CLIPTEST},
    {PX0, PY6, 100, 40, IND_BLUE, IND_YELLOW, "ClrTable", 0, ID_CLRTABLE},
    {PX0, PY7, 100, 40, IND_BLUE, IND_YELLOW, "ColorOps", 0, ID_COLOROPS},
    {PX0, PY8, 100, 40, IND_BLUE, IND_YELLOW, "CursTest", 0, ID_CURSTEST},
    {PX1, PY0, 100, 40, IND_BLUE, IND_YELLOW, "CusPTest", 0, ID_CUSPTEST},
    {PX1, PY1, 100, 40, IND_BLUE, IND_YELLOW, "DemoIntl", 0, ID_DEMOINTL},
    {PX1, PY2, 100, 40, IND_BLUE, IND_YELLOW, "FontDemo1", 0, ID_FNTDEMO1},
    {PX1, PY3, 100, 40, IND_BLUE, IND_YELLOW, "FontDemo2", 0, ID_FNTDEMO2},
    {PX1, PY4, 100, 40, IND_BLUE, IND_YELLOW, "FontDemo3", 0, ID_FNTDEMO3},
    {PX1, PY5, 100, 40, IND_BLUE, IND_YELLOW, "FontDemo4", 0, ID_FNTDEMO4},
    {PX1, PY6, 100, 40, IND_BLUE, IND_YELLOW, "FontTest", 0, ID_FONTTEST},
    {PX1, PY7, 100, 40, IND_BLUE, IND_YELLOW, "GradTest", 0, ID_GRADTEST},
    {PX1, PY8, 100, 40, IND_BLUE, IND_YELLOW, "JpgTest", 0, ID_JPGTEST},
    {PX2, PY0, 100, 40, IND_BLUE, IND_YELLOW, "Keys", 0, ID_KEYS},
    {PX2, PY1, 100, 40, IND_BLUE, IND_YELLOW, "Life", 0, ID_LIFE},
    {PX2, PY2, 100, 40, IND_BLUE, IND_YELLOW, "Life_db", 0, ID_LIFEDB},
    {PX2, PY3, 100, 40, IND_BLUE, IND_YELLOW, "LineTest", 0, ID_LINETEST},
    {PX2, PY4, 100, 40, IND_BLUE, IND_YELLOW, "MouseTst", 0, ID_MOUSETST},
    {PX2, PY6, 100, 40, IND_GREEN, IND_WHITE, "Page 2", 0, ID_PAGE2},
    {PX2, PY7, 100, 40, IND_BROWN, IND_WHITE, "ModeTest", 0, ID_MODETEST},
    {PX2, PY8, 100, 40, IND_RED, IND_WHITE, "Exit", 0, ID_EXIT}
};

#define NBUTTONSP2  21

static Button bp2[NBUTTONSP2] = {
    {PX0, PY0, 100, 40, IND_BLUE, IND_YELLOW, "MpolTest", BSTATUS_SELECTED, ID_MPOLTEST},
    {PX0, PY1, 100, 40, IND_BLUE, IND_YELLOW, "PatrTest", 0, ID_PATRTEST},
    {PX0, PY2, 100, 40, IND_BLUE, IND_YELLOW, "PcircTst", 0, ID_PCIRCTST},
    {PX0, PY3, 100, 40, IND_BLUE, IND_YELLOW, "PixmTest", 0, ID_PIXMTEST},
    {PX0, PY4, 100, 40, IND_BLUE, IND_YELLOW, "PnmTest", 0, ID_PNMTEST},
    {PX0, PY5, 100, 40, IND_BLUE, IND_YELLOW, "PngTest", 0, ID_PNGTEST},
    {PX0, PY6, 100, 40, IND_BLUE, IND_YELLOW, "PolyTest", 0, ID_POLYTEST},
    {PX0, PY7, 100, 40, IND_BLUE, IND_YELLOW, "PolyTeDb", 0, ID_POLYTEDB},
    {PX0, PY8, 100, 40, IND_BLUE, IND_YELLOW, "RgbTest", 0, ID_RGBTEST},
    {PX1, PY0, 100, 40, IND_BLUE, IND_YELLOW, "SbcTest", 0, ID_SBCTEST},
    {PX1, PY1, 100, 40, IND_BLUE, IND_YELLOW, "SclTest", 0, ID_SCLTEST},
    {PX1, PY2, 100, 40, IND_BLUE, IND_YELLOW, "ScrolTst", 0, ID_SCROLTST},
    {PX1, PY3, 100, 40, IND_BLUE, IND_YELLOW, "SpeedTst", 0, ID_SPEEDTST},
    {PX1, PY4, 100, 40, IND_BLUE, IND_YELLOW, "SpeedTs2", 0, ID_SPEEDTS2},
    {PX1, PY5, 100, 40, IND_BLUE, IND_YELLOW, "TextPatt", 0, ID_TEXTPATT},
    {PX1, PY6, 100, 40, IND_BLUE, IND_YELLOW, "WinClip", 0, ID_WINCLIP},
    {PX1, PY7, 100, 40, IND_BLUE, IND_YELLOW, "WinTest", 0, ID_WINTEST},
    {PX1, PY8, 100, 40, IND_BLUE, IND_YELLOW, "WRszTest", 0, ID_WRSZTEST},
    {PX2, PY6, 100, 40, IND_GREEN, IND_WHITE, "Page 1", 0, ID_PAGE1},
    {PX2, PY7, 100, 40, IND_BROWN, IND_WHITE, "ModeTest", 0, ID_MODETEST},
    {PX2, PY8, 100, 40, IND_RED, IND_WHITE, "Exit", 0, ID_EXIT}
};

static Button_Group bgp1 = { 16, 30, bp1, NBUTTONSP1, 0, 0 };
static Button_Group bgp2 = { 16, 30, bp2, NBUTTONSP2, 0, 0 };
static Button_Group *bgact = &bgp1;

static Board brd =
    { 0, 0, 640, 480, IND_BLACK, IND_CYAN, IND_DARKGRAY, 1 };
static Board brdimg =
    { 379, 46, 235, 157, IND_BLACK, IND_CYAN, IND_DARKGRAY, 1 };

static GrFont *grf_std;
static GrFont *grf_big;
static GrFont *grf_gfaz;
static GrFont *grf_intl;
GrTextOption grt_centered;
GrTextOption grt_left;

static GrContext *grcglob = NULL;
static GrContext *grcglobdb = NULL;
static int worg = 0, horg = 0;

/* Internal routines */

static void ini_graphics(void);
static void end_graphics(void);
static GrFont *load_font(char *fname);
static void ini_objects(void);
static void paint_screen(void);
static void the_title(int x, int y);
static void the_info(int x, int y);
static int pev_command(GrEvent * ev);
static int pev_select(GrEvent * ev);
static void paint_foot(char *s);
static void paint_animation(int despl);
static void disaster(char *s);

/************************************************************************/

int main(int argc, char **argv)
{
    GrEvent ev;
    int grevframes = 0;
    long oldtime = 0;

    if (argc >= 4) {
        gwidth = atoi(argv[1]);
        gheight = atoi(argv[2]);
        gbpp = atoi(argv[3]);
    }

    ini_graphics();
    GrSetWindowTitle(wintitle);
    ini_objects();
    gfaz_setfont(grf_gfaz);
    paint_screen();
    // if the videodriver can generate GREV_FRAME events we use it
    grevframes = GrEventGenFrame(GR_GEN_YES);

    while (1) {
        GrEventRead(&ev);
        if (ev.type == GREV_MOUSE) {
            ev.p2 -= worg;
            ev.p3 -= horg;
        }
        if (ev.type == GREV_END)
            break;
        if ((ev.type == GREV_KEY) && (ev.p1 == GrKey_Escape))
            break;
        if (grevframes) {
            if (ev.type == GREV_FRAME) {
                paint_animation(2);
                continue;
            }
        } else if (ev.time > oldtime + 9) {
            //fprintf(stderr, "etime %ld\n", ev.time-oldtime);
            paint_animation(1);
            oldtime = ev.time;
        }
        if ((ev.type == GREV_KEY) && (ev.p1 == 's')) {
            GrSaveContextToPpm(NULL, PPMIMGOUT, "DemoMGRX");
            continue;
        }
        if (pev_button_group(&ev, bgact))
            continue;
        if (pev_command(&ev))
            continue;
        if (pev_select(&ev))
            continue;
        if (ev.type == GREV_MOUSE) {
            if (ev.p1 == GRMOUSE_LB_PRESSED)
                paint_foot("over a button, please");
            else if (ev.p1 == GRMOUSE_LB_RELEASED)
                paint_foot("Hold down left mouse buttom to see a comment");
            continue;
        }
        if (ev.type == GREV_EXPOSE) {
            //printf("expose %ld %ld %ld %ld %d\n",
            //       ev.p1, ev.p2, ev.p3, ev.p4, ev.kbstat);
            if (ev.kbstat == 0) { // no more EXPOSE events follow
                if (gwidth > 640 || gheight > 480)
                    GrClearScreen(GrAllocColor(120, 90, 60));
                paint_screen();
            }
            continue;
        }
    }

    end_graphics();
    return 0;
}

/************************************************************************/

static void ini_graphics(void)
{
    gfaz_ini(gwidth, gheight, gbpp);
    gwidth = GrScreenX();
    gheight = GrScreenY();
    grcglob = NULL;
    if (gwidth > 640 || gheight > 480) {
        GrClearScreen(GrAllocColor(120, 90, 60));
        worg = (gwidth - 640) / 2;
        horg = (gheight - 480) / 2;
        grcglob = GrCreateSubContext(worg, horg, worg + 639, horg + 479,
                                     NULL, NULL);
        GrSetContext(grcglob);
    }
    grcglobdb = GrCreateContext(640, 480, NULL, NULL);
    GrEventGenExpose(GR_GEN_EXPOSE_YES);
}

/************************************************************************/

static void end_graphics(void)
{
    void _GrCloseVideoDriver(void);

    if (grcglobdb) GrDestroyContext(grcglobdb);
    if (grcglob) GrDestroyContext(grcglob);
    gfaz_fin();
    _GrCloseVideoDriver();
}

/************************************************************************/

static GrFont *load_font(char *fname)
{
    GrFont *gf;
    char s[81];
    
    gf = GrLoadFont(fname);
    if (gf == NULL) {
        snprintf(s, 80, "../fonts/%s", fname);
        gf = GrLoadFont(s);
        if (gf == NULL) {
            snprintf(s, 80, "%s not found in ./ or ../fonts/", fname);
            disaster(s);
        }
    }
    
    return gf;
}

/************************************************************************/

static void ini_objects(void)
{
    grf_std = load_font("lucb21.fnt");
    grf_big = load_font("lucb40.fnt");
    grf_gfaz = load_font("tmgrx14b.fnt");
    grf_intl = load_font("tmgrx18b.fnt");

    grt_centered.txo_bgcolor = GrNOCOLOR;
    grt_centered.txo_direct = GR_TEXT_RIGHT;
    grt_centered.txo_xalign = GR_ALIGN_CENTER;
    grt_centered.txo_yalign = GR_ALIGN_CENTER;
    grt_centered.txo_chrtype = GR_UTF8_TEXT;

    grt_left.txo_bgcolor = GrNOCOLOR;
    grt_left.txo_direct = GR_TEXT_RIGHT;
    grt_left.txo_xalign = GR_ALIGN_LEFT;
    grt_left.txo_yalign = GR_ALIGN_CENTER;
    grt_left.txo_chrtype = GR_UTF8_TEXT;
}

/************************************************************************/

static void paint_screen(void)
{
    // use a double buffer to avoid flickering
    GrContext *grc, *grcf;

    grcf = grcglob;
    if (grcglobdb != NULL) { 
        GrSetContext(grcglobdb);
        grcf = grcglobdb;
    }
    paint_board(&brd);
    if (GrNumColors() > 256 && GrJpegSupport()) {
        GrSetClipBox(4,4,GrMaxX()-4,GrMaxY()-4);
        GrLoadContextFromJpeg(NULL, JPGIMGBG, 1);
        GrResetClipBox();
    }
    paint_button_group(bgact);
    paint_board(&brdimg);
    grc = GrCreateSubContext(brdimg.x + 4, brdimg.y + 4,
                             brdimg.x + brdimg.wide - 5,
                             brdimg.y + brdimg.high - 5, grcf, NULL);
    if (bgact == &bgp1)
        GrLoadContextFromPnm(grc, PPMIMG1);
    else
        GrLoadContextFromPnm(grc, PPMIMG2);
    GrDestroyContext(grc);
    the_info(495, 225);
    the_title(495, 370);
    paint_foot("Hold down left mouse buttom to see a comment");
    GrSetContext(grcglob);
    GrBitBlt(NULL, 0, 0, grcglobdb, 0, 0, 639, 479, GrWRITE);
}

/************************************************************************/

static void the_title(int x, int y)
{
    char *t2 = "test programs launcher";

    grt_centered.txo_fgcolor = LIGHTGREEN;

    grt_centered.txo_font = grf_big;
    GrDrawString(mgrxversion, 0, 0 + x, 0 + y, &grt_centered);

    grt_centered.txo_font = grf_std;
    GrDrawString(t2, 0, 0 + x, 30 + y, &grt_centered);

}

/************************************************************************/

static void the_info(int x, int y)
{
    char aux[81];
    char sys[4] = "?";
    int nsys;
    char *kbsysencoding;
    int nenc;

    grt_centered.txo_fgcolor = LIGHTCYAN;
    grt_centered.txo_font = grf_std;

    nsys = GrGetLibrarySystem();
    if (nsys == MGRX_VERSION_GCC_386_DJGPP)
        strcpy(sys, "DJ2");
    else if (nsys == MGRX_VERSION_GCC_386_LINUX)
        strcpy(sys, "L32");
    else if (nsys == MGRX_VERSION_GCC_386_X11)
        strcpy(sys, "X32");
    else if (nsys == MGRX_VERSION_GCC_386_WYL)
        strcpy(sys, "Y32");
    else if (nsys == MGRX_VERSION_GCC_X86_64_LINUX)
        strcpy(sys, "L64");
    else if (nsys == MGRX_VERSION_GCC_X86_64_X11)
        strcpy(sys, "X64");
    else if (nsys == MGRX_VERSION_GCC_X86_64_WYL)
        strcpy(sys, "Y64");
    else if (nsys == MGRX_VERSION_GCC_386_WIN32)
        strcpy(sys, "W32");
    else if (nsys == MGRX_VERSION_GCC_ARM_LINUX)
        strcpy(sys, "LAR");
    else if (nsys == MGRX_VERSION_GCC_ARM_X11)
        strcpy(sys, "XAR");
    else if (nsys == MGRX_VERSION_GCC_ARM_WYL)
        strcpy(sys, "YAR");

    nenc = GrGetKbSysEncoding();
    kbsysencoding = GrStrEncoding(nenc);

    sprintf(aux, "Version: %x System: %s", GrGetLibraryVersion(), sys);
    GrDrawString(aux, 0, 0 + x, 0 + y, &grt_centered);

    sprintf(aux, "KbSysEncoding: %s", kbsysencoding);
    GrDrawString(aux, 0, 0 + x, 20 + y, &grt_centered);

    sprintf(aux, "VideoDriver: %s", GrCurrentVideoDriver()->name);
    GrDrawString(aux, 0, 0 + x, 40 + y, &grt_centered);

    sprintf(aux, "ScreenFrameBuffer: %s", GrFrameDriverName(GrScreenFrameMode()));
    GrDrawString(aux, 0, 0 + x, 60 + y, &grt_centered);

    sprintf(aux, "RamFrameBuffer: %s", GrFrameDriverName(GrCoreFrameMode()));
    GrDrawString(aux, 0, 0 + x, 80 + y, &grt_centered);

    sprintf(aux, "Mode: %dx%d %d bpp", GrCurrentVideoMode()->width,
            GrCurrentVideoMode()->height, GrCurrentVideoMode()->bpp);
    GrDrawString(aux, 0, 0 + x, 100 + y, &grt_centered);
}

/************************************************************************/

static int pev_command(GrEvent * ev)
{
    int i;
    char nprog[121];

    if (ev->type == GREV_COMMAND) {
        if (ev->p1 == ID_EXIT) {
            GrEventParEnqueue(GREV_END, 0, 0, 0, 0);
            return 1;
        }
        if (ev->p1 == ID_PAGE1) {
            bgact = &bgp1;
            paint_screen();
            return 1;
        }
        if (ev->p1 == ID_PAGE2) {
            bgact = &bgp2;
            paint_screen();
            return 1;
        }
        for (i = 0; i < NDEMOS; i++) {
            if (ev->p1 == ptable[i].cid) {
                if (need_restart) {
                    end_graphics();
                }
#if defined(__MSDOS__) || defined(__WIN32__)
                if (ev->p1 == ID_MODETEST)
                    strcpy(nprog, "..\\bin\\");
                else
                    strcpy(nprog, ".\\");
#elif defined(__XWIN__)
                if (ev->p1 == ID_MODETEST)
                    strcpy(nprog, "xterm -e ../bin/x");
                else if (ev->p1 == ID_SPEEDTST)
                    strcpy(nprog, "xterm -e ./x");
                else if (ev->p1 == ID_SPEEDTS2)
                    strcpy(nprog, "xterm -e ./x");
                else
                    strcpy(nprog, "./x");
#elif defined(__WAYLAND__)
                if (ev->p1 == ID_MODETEST)
                    strcpy(nprog, "xterm -e ../bin/w");
                else if (ev->p1 == ID_SPEEDTST)
                    strcpy(nprog, "xterm -e ../w");
                else if (ev->p1 == ID_SPEEDTS2)
                    strcpy(nprog, "xterm -e ../w");
                else
                    strcpy(nprog, "./w");
#else
                if (ev->p1 == ID_MODETEST)
                    strcpy(nprog, "../bin/");
                else
                    strcpy(nprog, "./");
#endif
                strcat(nprog, ptable[i].prog);
                system(nprog);
                if (need_restart) {
                    ini_graphics();
                    GrSetWindowTitle(wintitle);
                    gfaz_setfont(grf_gfaz);
                    GrEventFlush();
                }
                paint_screen();
                return 1;
            }
        }
    }
    return 0;
}

/************************************************************************/

static int pev_select(GrEvent * ev)
{
    int i;

    if (ev->type == GREV_SELECT) {
        for (i = 0; i < NDEMOS; i++) {
            if (ev->p1 == ptable[i].cid) {
                paint_foot(ptable[i].text);
                return 1;
            }
        }
    }
    return 0;
}

/************************************************************************/

static void paint_foot(char *s)
{
    grt_centered.txo_fgcolor = LIGHTGREEN;
    grt_centered.txo_font = grf_std;

    GrSetClipBox(10, 440, 630, 470);
    GrClearClipBox(CYAN);
    GrDrawString(s, 0, 320, 455, &grt_centered);
    GrResetClipBox();
}

/************************************************************************/

static void paint_animation(int despl)
{
    static int pos = 620;
    static int ini = 0;
    static int ind = 0;
    static GrContext *grc, *grcor;
    int ltext, wtext;

    if (!ini) {
        grcor = GrCreateContext(620, 30, NULL, NULL);
        if (grcor == NULL) return;
        grc = GrCreateContext(620, 30, NULL, NULL);
        if (grc == NULL) {
            GrDestroyContext(grcor);
            return;
        }
        GrBitBlt(grcor, 0, 0, NULL, 10, 8, 629, 37, GrWRITE);
        ini = 1;
    }

    if (ind > 1) ind = 0;
    grt_left.txo_fgcolor = LIGHTCYAN;
    grt_left.txo_font = ((ind % 2) == 1) ? grf_intl : grf_std;
    ltext = GrUTF8StrLen((unsigned char *)animatedtext[ind]);
    wtext = GrStringWidth(animatedtext[ind], ltext, &grt_left);

    GrBitBlt(grc, 0, 0, grcor, 0, 0, 619, 29, GrWRITE);
    GrSetContext(grc);
    //GrClearContext(DARKGRAY);
    GrDrawString(animatedtext[ind], ltext, pos, 15, &grt_left);
    GrSetContext(grcglob);
    GrBitBlt(NULL, 10, 8, grc, 0, 0, 619, 29, GrWRITE);

    pos -= despl;
    if (pos <= -wtext) {
        pos = 620;
        ind++;
    }
}

/************************************************************************/

static void disaster(char *s)
{
    void _GrCloseVideoDriver(void);

    char aux[81];

    gfaz_fin();
    _GrCloseVideoDriver();
    printf("DemoMGRX: %s\n", s);
    printf("press Return to continue\n");
    fgets(aux, 80, stdin);
    exit(1);
}
