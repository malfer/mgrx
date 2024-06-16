#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <grgui.h>
#include <mgrxcolr.h>

#define COMMAND_EXIT         1
#define COMMAND_LOAD         2
#define COMMAND_SAVE         3
#define COMMAND_SAVEAS       4
#define COMMAND_ABOUT        5
#define COMMAND_TP_CUT       6
#define COMMAND_TP_COPY      7
#define COMMAND_TP_PASTE     8
#define COMMAND_TP_SELALL    9

#define IDT_EDIT   1
#define IDT_FNAME  2
#define IDT_STATUS 3

GUITile *gtedit, *gtfname, *gtstatus;
GUITextPanel *gtp = NULL;
//we need a real buffer at least FCDLG_MAXLEN_FNAME+1 bytes to use in file chooser dialog
char editedfname[FCDLG_MAXLEN_FNAME+1] = "new_file";
GrTextOption grt_infol, grt_infor;
int new_file = 1; //to detect if no file loaded so save=save_as

void setup_menus(void)
{
    static GUIMenuItem itemsm1[5] = {
        {GUI_MI_OPER, 1, "&Load file...", 'L', NULL, 0, COMMAND_LOAD, 0, 0},
        {GUI_MI_OPER, 1, "&Save file", 'S', NULL, 0, COMMAND_SAVE, 0, 0},
        {GUI_MI_OPER, 1, "Save &As...", 'A', NULL, 0, COMMAND_SAVEAS, 0, 0},
        {GUI_MI_SEP, 1, "", 0, NULL, 0, 0, 0},
        {GUI_MI_OPER, 1, "E&xit", 'X', "Alt+X", GrKey_Alt_X, COMMAND_EXIT, 0, 0}};

    static GUIMenu menu1 = {1, 5, 0, itemsm1};

    static GUIMenuItem itemsm2[4] = {
        {GUI_MI_OPER, 1, "Cu&t", 'T', "Ctrl+X", GrKey_Control_X, COMMAND_TP_CUT, 0},
        {GUI_MI_OPER, 1, "&Copy", 'C', "Ctrl+C", GrKey_Control_C, COMMAND_TP_COPY, 0},
        {GUI_MI_OPER, 1, "&Paste", 'P', "Ctrl+V", GrKey_Control_V, COMMAND_TP_PASTE, 0},
        {GUI_MI_OPER, 1, "Select &All", 'A', "Ctrl+A", GrKey_Control_A, COMMAND_TP_SELALL, 0}};

    static GUIMenu menu2 = {2, 4, 0, itemsm2};

    static GUIMenuItem itemsm3[1] = {
        {GUI_MI_OPER, 1, "&About", 'A', NULL, 0, COMMAND_ABOUT, 0, 0}};

    static GUIMenu menu3 = {3, 1, 0, itemsm3};

    static GUIMenuBarItem mbitems[3] = {
        {"&File", 1, GrKey_Alt_F, 1},
        {"&Edit", 1, GrKey_Alt_E, 2},
        {"&Help", 1, GrKey_Alt_H, 3}};

    static GUIMenuBar menubar = {3 ,0, mbitems};

    GUIMenusSetChrType(GR_UTF8_TEXT); //this source is UTF8 coded
    GUIMenusSetFont(&GrFont_PX11x22);
    GUIMenusSetColors(EGAC_WHITE, EGAC_RED, EGAC_BLACK,
                      EGAC_YELLOW, EGAC_LIGHTGRAY, EGAC_DARKGRAY);

    GUIMenuRegister(&menu1);
    GUIMenuRegister(&menu2);
    GUIMenuRegister(&menu3);
    GUIMenuBarSet(&menubar);
}

void setup_objects(void)
{
    int mbheight, stheight;

    GUIObjectsSetColors(EGAC_BLACK, EGAC_LIGHTGRAY, EGAC_DARKGRAY);
    GUIObjectsSetFont(&GrFont_PX8x18);
    GUIScrollbarsSetColors(EGAC_LIGHTGRAY, EGAC_DARKGRAY);
    GUIDialogsSetColors(EGAC_BLACK, EGAC_CYAN, EGAC_DARKGRAY, EGAC_WHITE);
    GUIDialogsSetTitleFont(&GrFont_PX11x22);
    GUICDialogsSetColors(EGAC_CYAN, EGAC_WHITE);
    GUICDialogsSetObjColors(EGAC_WHITE, EGAC_BLACK, EGAC_LIGHTGRAY, EGAC_WHITE);
    GUICDialogsSetFont(&GrFont_PX8x18);

    mbheight = GUIMenuBarGetHeight();
    stheight = GrFont_PX8x18.h.height + 2;
    gtedit = GUITileCreate(IDT_EDIT, GUI_TT_ACTIVEBWSCB, 0, mbheight,
                           GrSizeX(), GrSizeY()-mbheight-stheight);
    gtfname = GUITileCreate(IDT_FNAME, GUI_TT_BORDERLESS, 0, GrSizeY()-stheight,
                            GrSizeX()/2, stheight);
    gtstatus = GUITileCreate(IDT_STATUS, GUI_TT_BORDERLESS, GrSizeX()/2,
                             GrSizeY()-stheight, GrSizeX()/2, stheight);
    if (gtedit == NULL || gtfname == NULL || gtstatus == NULL) exit(1);
    GUITileRegister(gtedit);
    GUITileRegister(gtfname);
    GUITileRegister(gtstatus);
    GUITilesSetColors(EGAC_BLACK, EGAC_LIGHTGRAY, EGAC_WHITE);

    grt_infol.txo_font = &GrFont_PX8x18;
    grt_infol.txo_fgcolor = EGAC_BLACK;
    grt_infol.txo_bgcolor = GrNOCOLOR;
    grt_infol.txo_direct = GR_TEXT_RIGHT;
    grt_infol.txo_xalign = GR_ALIGN_RIGHT;
    grt_infol.txo_yalign = GR_ALIGN_TOP;
    grt_infol.txo_chrtype = GR_UTF8_TEXT;
    grt_infor = grt_infol;
    grt_infol.txo_xalign = GR_ALIGN_LEFT;

    if (gtp == NULL) { //it is not NULL if we are resizing window
        gtp = GUITPCreate(gtedit->p, NULL);
        if (gtp == NULL) exit(1);
        GUITPClear(gtp);
    } else {
        GUITPSetPanel(gtp, gtedit->p);
    }
    GUITPSetSimpleColors(gtp, EGAC_WHITE, EGAC_BLACK, EGAC_RED);
    GUITPSetFont(gtp, &GrFont_PX8x18);
}

void paint_gtedit(void *data)
{
    GUITPReDraw(gtp);
}

int process_gtedit_event(void *data, GrEvent *ev)
{
    int ret;

    ret = GUITPProcessEvent(gtp, ev);
    if (ret) GUITilePaint(IDT_STATUS);
    return ret;
}

void paint_gtfname(void *data)
{
    GrClearContext(EGAC_LIGHTGRAY);
    GrDrawString(editedfname, 0, 8, 1, &grt_infol);
}

void paint_gtstatus(void *data)
{
    char s[81];
    GUITEditStatus tast;

    GUITPGetStatus(gtp, &tast);
    snprintf(s, 80, "line %d of %d, col %d",
             tast.tclpos+1, tast.nlines, tast.tccpos+1);
    GrClearContext(EGAC_LIGHTGRAY);
    GrDrawString(s, 0, GrMaxX()-8, 1, &grt_infor);

    s[0] = '\0';
    if (GUITPGetChanged(gtp)) strcat(s,"(M) ");
    if (tast.full) strcat(s, "FULL ");
    if (tast.partloaded) strcpy(s, "CBPL ");
    if (s[0]) GrDrawString(s, 0, 2, 1, &grt_infol);
}

int load_editedfile(void)
{
    FILE *fin;
    char aux[251];

    fin = fopen(editedfname, "r");
    if (fin == NULL) return 0; //TODO show error
    GUITPHideTCursor(gtp);
    while (fgets(aux, 250, fin) != NULL) {
        GUITPPutMultiStringNoDraw(gtp, aux, 0, GR_UTF8_TEXT);
    }
    GUITPMoveTCursor(gtp, 0, 0, GUI_RESETMARQUEDAREA);
    GUITPShowTCursor(gtp);
    fclose(fin);
    return 1;
}

int save_editedfile(void)
{
    FILE *fout;
    GUITEditStatus tast;
    char *s;
    int i;
    int first = 1;

    fout = fopen(editedfname, "w");
    if (fout == NULL) return 0; //TODO show error
    GUITPGetStatus(gtp, &tast);
    for (i=0; i<tast.nlines; i++) {
        s = GUITPGetString(gtp, i, GR_UTF8_TEXT);
        if (s == NULL) break; //TODO show error
        if (!first) putc( '\n', fout);
        fputs(s, fout);
        first = 0;
        free(s);
    }
    fclose(fout);
    return 1;
}

int test_not_saved(void)
{
    int result = 0;

    if (GUITPGetChanged(gtp)) { //if the edited file has been modified
        result = GUICDialogYesNoCancel("Modified file, save it?", NULL,
                                       0, "Yes", "No", "Cancel");
        if (result == 1) {
            if (new_file)
                result = GUICDialogFileChooser("Save File As", 480, 320,
                                               editedfname, FCDLGF_ASKOVERWRITE);
            if (result == 1)
                if (!save_editedfile()) result = -1;
        }
    }
    return result;
}

int process_command(GrEvent *ev)
{
    int result;

    if (ev->type == GREV_COMMAND) {
        switch (ev->p1) {
            case COMMAND_TP_CUT :
                result = GUITPCopyMA(gtp, 1);
                GUITilePaint(IDT_EDIT);
                GUITilePaint(IDT_STATUS);
                return 1;
            case COMMAND_TP_COPY :
                result = GUITPCopyMA(gtp, 0);
                GUITilePaint(IDT_STATUS);
                return 1;
            case COMMAND_TP_PASTE :
                GUITPPasteMA(gtp);
                GUITilePaint(IDT_EDIT);
                GUITilePaint(IDT_STATUS);
                return 1;
            case COMMAND_TP_SELALL :
                GUITPSetMA(gtp, -1, 0, 0, 0);
                GUITilePaint(IDT_EDIT);
                GUITilePaint(IDT_STATUS);
                return 1;

            case COMMAND_LOAD :
                result = test_not_saved();
                if (result == -1) return 1;
                result = GUICDialogFileChooser("Load File", 480, 320, editedfname, 0);
                if (result == 1) {
                    GUITPClear(gtp);
                    load_editedfile();
                    GUITPResetChanged(gtp);
                    GUITilePaint(IDT_EDIT);
                    GUITPShowTCursor(gtp);
                    GUITilePaint(IDT_FNAME);
                    GUITilePaint(IDT_STATUS);
                    new_file = 0;
                }
                return 1;
            case COMMAND_SAVE :
                if (new_file) {
                    result = GUICDialogFileChooser("Save File As", 480, 320,
                                                   editedfname, FCDLGF_ASKOVERWRITE);
                    if (result != 1) return 1;
                }
                save_editedfile();
                GUITPResetChanged(gtp);
                GUITilePaint(IDT_FNAME);
                GUITilePaint(IDT_STATUS);
                new_file = 0;
                return 1;
            case COMMAND_SAVEAS :
                result = GUICDialogFileChooser("Save File As", 480, 320,
                                               editedfname, FCDLGF_ASKOVERWRITE);
                if (result == 1) {
                    save_editedfile();
                    GUITPResetChanged(gtp);
                    GUITilePaint(IDT_FNAME);
                    GUITilePaint(IDT_STATUS);
                    new_file = 0;
                }
                return 1;

            case COMMAND_ABOUT :
                {
                    char *abouttext[2] = {
                        "GrGUI+MGRX example program number 13",
                        "a small text editor"};
                    GUICDialogInfo("About", (void **)abouttext, 2, "Ok");
                }
                return 1;

            case COMMAND_EXIT :
                GrEventParEnqueue(GREV_END, 0, 0, 0, 0);
                return 1;
       }
    }
    return 0;
}

int main()
{
    GrEvent ev;
    int windoww = 800;
    int windowh = 600;
    int terminate = 0;

    GrSetDriverExt(NULL, "rszwin"); //let window resizes in windowed plattforms
    GrSetMinWindowDims(640, 480); //set minimal window dimensions
    GrSetWindowTitle("GrGUI example 13"); //in windowed plattforms set the win title

    while(1) {
        GrSetMode(GR_width_height_bpp_graphics, windoww, windowh, 8);
        GUIInit(1, 1); //init input and use double buffer
        GrSetDefaultFont(&GrFont_PX8x18); //this is an intermal font
        GrEventGenWMEnd(GR_GEN_WMEND_YES); //in windowed plattforms gen WMEND event
        GrGenEgaColorTable();
        GrSetComposeKey(GrKey_Alt_U); //to compose chars using ALT_U + hex digits
        GrSetClipBoardMaxChars(GR_CB_NOLIMIT); //we don't want clipboard limit

        setup_menus();
        setup_objects();
        GUIPanelSetClCallBacks(gtedit->p, paint_gtedit, process_gtedit_event);
        GUIPanelSetClCallBacks(gtfname->p, paint_gtfname, NULL);
        GUIPanelSetClCallBacks(gtstatus->p, paint_gtstatus, NULL);

        GUIMenuBarShow();
        GUITilePaint(IDT_EDIT);
        GUITilePaint(IDT_FNAME);
        GUITilePaint(IDT_STATUS);
        GUITPShowTCursor(gtp); //show text cursor

        while(1) {
            GrEventRead(&ev);
            if (ev.type == GREV_END || (ev.type == GREV_WMEND)) {
                if (test_not_saved() == -1) continue;
                terminate = 1;
                break;
            }
            if (ev.type == GREV_WSZCHG) { //window resize
                windoww = ev.p3;
                windowh = ev.p4;
                break;
            }
            if (process_command(&ev)) continue;
            GUITilesProcessEvent(&ev);
        }

        GUITPHideTCursor(gtp); //hide text cursor
        GUITilesDestroyAll();
        GUIEnd();
        if (terminate) break;
    }

    GUITPDestroy(gtp);
    GrSetMode(GR_default_text);

    return 0;
}
