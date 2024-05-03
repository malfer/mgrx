#include <stdlib.h>
#include <stdio.h>
#include <grgui.h>
#include <mgrxcolr.h>
#include "gr12i18n.h"

#if defined(__MSDOS__) || defined(__WIN32__)
#define PICDIR  "..\\testimg\\"
#else
#define PICDIR  "../testimg/"
#endif
#define PICTURE1 PICDIR"jpeg1.jpg"
#define PICTURE2 PICDIR"jpeg2.jpg"
#define PICTURE3 PICDIR"jpeg4.jpg"

/* commands generate by menus */

#define COMMAND_PICTURE1            1
#define COMMAND_PICTURE2            2
#define COMMAND_PICTURE3            3
#define COMMAND_EXIT               10
#define COMMAND_DLG_GRGUI          20
#define COMMAND_DLG_ABOUT          21
#define COMMAND_LAN_ENGLISH        30
#define COMMAND_LAN_SPANISH        31
#define COMMAND_LAN_GREEK          32

/* tiles ids */

#define IDT_CANVAS                  1

/* global variables */

static GUITile *tile1;
static GrContext *imgctx = NULL;
static int imgwidth, imgheight;
static int nimgloaded = 0;
static char *imgname[3] = {PICTURE1, PICTURE2, PICTURE3};
static long restart_command = 0;
static GrFont *objfont = NULL;
static GrFont *dlgtfont = NULL;
static GrFont *cdlgfont = NULL;
static GrFont *menufont = NULL;
static int language = 0;

/* function declarations */
void setup_menus(void);
void setup_etc(void);
void change_language(int lid);
void load_image(int n);
void load_image_file(char *s);
void destroy_image(void);
void paint_panel1(void *udata);
int dialog_grgui(void);
int dialog_about(void);

int main()
{
    int w = 640;
    int h = 480;
    int bpp = 32;

    // set default driver and ask for user window resize if it is supported
    GrSetDriverExt(NULL, "rszwin");
    // we can call here because the driver is set
    GrSetUserEncoding(GRENC_UTF_8);
    GrEventGenWMEnd(GR_GEN_WMEND_YES);
    GrSetFontPath("../fonts/;./");
    objfont = GrLoadFont("tmgrx16b.fnt");
    dlgtfont = GrLoadFont("tmgrx32b.fnt");
    cdlgfont = GrLoadFont("tmgrx18b.fnt");
    menufont = GrLoadFont("tmgrx18n.fnt");
    setup_i18n();
    GrI18nSetLang(language);

    while (1) {
        int exitloop = 0;
        GrEvent ev;
        int result;
        int nimg;
        
        GrSetMode(GR_width_height_bpp_graphics, w, h, bpp);
        GrGenWebColorTable();
        GUIInit(1, 1);
        setup_menus();
        setup_etc();
        GUIMenuBarShow();

        // This is an example of minimal status recovery after a
        // GREV_WSZCHG event, and works okey at the first time
        // 1- repaint the last image, note that load_image only reload
        // the image context if required
        load_image(nimgloaded);
        GUITilePaint(IDT_CANVAS);
        // 2- if we were waiting for a dialog when a GREV_WSZCHG come,
        // relaunch it again
        if (restart_command) {
            GrEventParEnqueueFirst(GREV_COMMAND, restart_command, 0, 0, 0);
            restart_command = 0;
        }

        while (1) {
            GrEventRead(&ev);
            if (((ev.type == GREV_KEY) && (ev.p1 == GrKey_Escape)) ||
                ((ev.type == GREV_COMMAND) && (ev.p1 == COMMAND_EXIT)) ||
                 (ev.type == GREV_WMEND)) {
                exitloop = 1;
                break;
            }
            if (ev.type == GREV_COMMAND) {
                if (ev.p1 >= COMMAND_PICTURE1 && ev.p1 <= COMMAND_PICTURE3) {
                    nimg = ev.p1 - COMMAND_PICTURE1;
                    load_image(nimg);
                    GUITilePaint(IDT_CANVAS);
                    continue;
                }
                if (ev.p1 == COMMAND_DLG_GRGUI) {
                    result = dialog_grgui();
                    if (result == -3) restart_command = COMMAND_DLG_GRGUI;
                    continue;
                }
                if (ev.p1 == COMMAND_DLG_ABOUT) {
                    result = dialog_about();
                    if (result == -3) restart_command = COMMAND_DLG_ABOUT;
                    continue;
                }
                if (ev.p1 >= COMMAND_LAN_ENGLISH && ev.p1 <= COMMAND_LAN_GREEK) {
                    change_language(ev.p1 - COMMAND_LAN_ENGLISH);
                    continue;
                }
            }
            if (ev.type == GREV_WSZCHG) {
                w = ev.p3;
                h = ev.p4;
                break;
            }
        }

        GUIEnd();
        if (exitloop) break;
    }
    GrI18nEnd();
    destroy_image();
    GrUnloadFont(menufont);
    GrUnloadFont(objfont);
    GrUnloadFont(dlgtfont);
    GrUnloadFont(cdlgfont);
    GrSetMode(GR_default_text);

    return 0;
}

void setup_menus(void)
{
    static GUIMenuItem itemsm1[5] = {
        {GUI_MI_OPER, 1, "", 0, NULL, 0, COMMAND_PICTURE1, 0, SID_MENU1_1}, 
        {GUI_MI_OPER, 1, "", 0, NULL, 0, COMMAND_PICTURE2, 0, SID_MENU1_2}, 
        {GUI_MI_OPER, 1, "", 0, NULL, 0, COMMAND_PICTURE3, 0, SID_MENU1_3}, 
        {GUI_MI_SEP, 1, "", 0, NULL, 0, 0, 0, 0}, 
        {GUI_MI_OPER, 1, "", 0, "Alt+X", GrKey_Alt_X, COMMAND_EXIT, 0, SID_MENU1_4}};

    static GUIMenu menu1 = {1, 5, 0, itemsm1};

    static GUIMenuItem itemsm2[2] = {
        {GUI_MI_OPER, 1, "", 0, NULL, 0, COMMAND_DLG_GRGUI, 0, SID_MENU2_1},
        {GUI_MI_OPER, 1, "", 0, NULL, 0, COMMAND_DLG_ABOUT, 0, SID_MENU2_2}};

    static GUIMenu menu2 = {2, 2, 0, itemsm2};

    static GUIMenuItem itemsm3[3] = {
        {GUI_MI_OPER, 1, "", 0, NULL, 0, COMMAND_LAN_ENGLISH, 0, SID_MENU3_1},
        {GUI_MI_OPER, 1, "", 0, NULL, 0, COMMAND_LAN_SPANISH, 0, SID_MENU3_2},
        {GUI_MI_OPER, 1, "", 0, NULL, 0, COMMAND_LAN_GREEK, 0, SID_MENU3_3}};

    static GUIMenu menu3 = {3, 3, 0, itemsm3};

    static GUIMenuBarItem mbitems[3] = {
        {"", 1, GrKey_Alt_F, 1, SID_MBAR1}, 
        {"", 1, GrKey_Alt_L, 3, SID_MBAR2}, 
        {"", 1, GrKey_Alt_H, 2, SID_MBAR3}};

    static GUIMenuBar menubar = {3 ,0, mbitems};

    GUIMenusSetChrType(GR_UTF8_TEXT); // this source is UTF8 coded
    GUIMenusSetFont(menufont);
    GUIMenusSetColors(WEBC_WHITE, WEBC_RED, WEBC_BLACK,
                      WEBC_YELLOW, WEBC_LIGHTGRAY, WEBC_DARKGRAY);

    GUIMenuRegister(&menu1);
    GUIMenuRegister(&menu2);
    GUIMenuRegister(&menu3);
    GUIMenuBarSet(&menubar);

    GUIMenusSetI18nFields();
    GUIMenuBarSetI18nFields();
}

void setup_etc(void)
{
    int mbheight;
    
    GUIObjectsSetColors(WEBC_KHAKI, WEBC_PERU, WEBC_SIENNA);
    GUIObjectsSetFont(objfont);
    GUIDialogsSetColors(WEBC_BLACK, WEBC_ORANGE, WEBC_MAROON,
                            WEBC_ANTIQUEWHITE);
    GUICDialogsSetColors(WEBC_TAN, WEBC_BLACK);
    GUIDialogsSetTitleFont(dlgtfont);
    GUICDialogsSetFont(cdlgfont);

    mbheight = GUIMenuBarGetHeight();
    tile1 = GUITileCreate(IDT_CANVAS, GUI_TT_BORDERLESS, 0, mbheight,
                          GrSizeX(), GrSizeY()-mbheight);
    GUIPanelSetClCallBacks(tile1->p, paint_panel1, NULL);
    GUITileRegister(tile1);
}

void change_language(int lid)
{
    if (lid < 0 || lid >= NUM_LANGUAGES) return;
    if (lid == language) return;
    language = lid;
    GrI18nSetLang(language);
    GUIMenusSetI18nFields();
    GUIMenuBarSetI18nFields();
    GUIMenuBarHide();
    GUIMenuBarShow();
}

void load_image(int n)
{
    if (n<0 || n>2) n = 0; // to be safe

    if (imgctx && n == nimgloaded) {
        // ok we have the image context already, but if the GrSetMode was
        // changed to an incompatible framemode we have to reload again!!
        //printf("modes: %d %d\n",
        //       imgctx->gc_driver->mode,
        //       GrScreenContext()->gc_driver->rmode);
        if (imgctx->gc_driver->mode == GrScreenContext()->gc_driver->rmode)
            return; // we don't need to reload
    }

    load_image_file(imgname[n]);
    nimgloaded = n;
}

void load_image_file(char *fimg)
{
    destroy_image();

    if (GrNumColors() > 256 && GrJpegSupport()) {
        if (GrQueryJpeg(fimg, &imgwidth, &imgheight) == 0) {
            imgctx = GrCreateContext(imgwidth, imgheight, NULL, NULL);
            if (imgctx != NULL) {
                if (GrLoadContextFromJpeg(imgctx, fimg, 1) != 0) {
                    destroy_image();;
                }
            }
        }
    }
}

void destroy_image(void)
{
    if (imgctx) {
        GrDestroyContext(imgctx);
        imgctx = NULL;
    }
}

void paint_panel1(void *udata)
{
    if (imgctx) {
        GrStretchBlt(NULL, 0, 0, GrMaxX(), GrMaxY(), imgctx, 0, 0,
                     imgwidth-1, imgheight-1, GrWRITE);
    } else {
        GrFilledBox(0, 0, GrMaxX(), GrMaxY(), WEBC_GOLDENROD);
    }
}

int dialog_grgui(void)
{
    static char *abouttext[4];

    abouttext[0] = _(SID_DLGHELLO1);
    abouttext[1] = _(SID_DLGHELLO2);
    abouttext[2] = _(SID_DLGHELLO3);
    abouttext[3] = _(SID_DLGHELLO4);

    return GUICDialogInfo(_(SID_DLGHELLOT), (void **)abouttext, 4, _(SID_OK));
}

int dialog_about(void)
{
    static char *abouttext[3];

    abouttext[0] = _(SID_DLGABOUT1);
    abouttext[1] = _(SID_DLGABOUT2);
    abouttext[2] = _(SID_DLGABOUT3);

    return GUICDialogInfo(_(SID_DLGABOUTT), (void **)abouttext, 3, _(SID_OK));
}
