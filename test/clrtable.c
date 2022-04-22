/**
 ** clrtablel.c ---- test color tables
 **
 ** Copyright (C) 2019 Mariano Alvarez Fernandez
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mgrx.h"
#include "mgrxkeys.h"
#include "mgrxcolr.h"

/* default mode */

static int gwidth = 1024;
static int gheight = 768;
static int gbpp = 24;

void paint_box(int x1, int y1, int x2, int y2, GrColor bcolor, char *name)
{
    int r, g, b;
    double lum;
    GrColor tcolor;

    GrQueryColor(bcolor, &r, &g, &b);
    lum = 0.299*r + 0.587*g + 0.114*b;
    if (lum > 128)
        tcolor = GrBlack();
    else
        tcolor = GrWhite();
    GrFilledBox(x1, y1, x2, y2, bcolor);
    GrTextXY(x1+2, y1+2, name, tcolor, GrNOCOLOR);
}

void paint_ansi_group(int fr, int le, int wide, int high, int row)
{
    int i;
    GrColor bcolor;
    char snum[41];

    for (i=fr; i<le; i++) {
        sprintf(snum, "%d", i);
        bcolor = GrAnsColorTable[i].color;
        paint_box((i-fr)*wide+10, row*high+50, (i-fr+1)*wide,
                  (row+1)*high+50, bcolor, snum);
    }
}

int main(int argc,char **argv)
{
    GrEvent ev;
    int i, j, ind, wide, high;
    int egatableloaded = 0;
    int webtableloaded = 0;
    int anstableloaded = 0;
    GrColor bcolor;
    
    if (argc >= 4) {
        gwidth = atoi(argv[1]);
        gheight = atoi(argv[2]);
        gbpp = atoi(argv[3]);
    }

    GrSetMode(GR_width_height_bpp_graphics, gwidth, gheight, gbpp);

    GrEventInit();
    GrMouseDisplayCursor();
    GrSetDefaultFont(&GrFont_PC8x16);
    
    if (!GrGenEgaColorTable()) {
        egatableloaded = 1;
        GrClearScreen(EGAC_BLACK);
        GrTextXY(10, 10, "EGA COLOR TABLE", EGAC_WHITE, EGAC_BLACK);
        wide = (GrMaxX()-10) / 4;
        high = (GrMaxY()-52) / 4;
        ind = 0;
        for (i=0; i<4; i++) {
            for (j=0; j<4; j++) {
                if (ind < NUM_EGA_COLORS) {
                    bcolor = GrEgaColorTable[ind].color;
                    paint_box(j*wide+10, i*high+50, (j+1)*wide, (i+1)*high+50,
                                bcolor, GrEgaColorTable[ind].name);
                    ind++;
                }
            }
        }
        GrEventWaitKeyOrClick(&ev);
    }

    wide = GrMaxX() / 2;
    high = (GrMaxY()-52) / 2;

    GrClearScreen(GrAllocColor2(EGAR_BLACK));
    GrTextXY(10, 10, "EGA COLOR STATIC RGB DEFS", GrAllocColor2(EGAR_WHITE),
             GrAllocColor2(EGAR_BLACK));
    GrFilledBox(10, 50, wide-10, high+30, GrAllocColor2(EGAR_RED));
    GrFilledBox(wide+10, 50, wide*2-10, high+30, GrAllocColor2(EGAR_GREEN));
    GrFilledBox(10, high+50, wide-10, high*2+30, GrAllocColor2(EGAR_BLUE));
    GrFilledBox(wide+10, high+50, wide*2-10, high*2+30, GrAllocColor2(EGAR_YELLOW));
    GrEventWaitKeyOrClick(&ev);
    
    if (egatableloaded) {
        GrClearScreen(EGAC_BLACK);
        GrTextXY(10, 10, "EGA COLOR DYNAMIC DEFS", EGAC_WHITE, EGAC_BLACK);
        GrFilledBox(10, 50, wide-10, high+30, EGAC_RED);
        GrFilledBox(wide+10, 50, wide*2-10, high+30, EGAC_GREEN);
        GrFilledBox(10, high+50, wide-10, high*2+30, EGAC_BLUE);
        GrFilledBox(wide+10, high+50, wide*2-10, high*2+30, EGAC_YELLOW);
        GrEventWaitKeyOrClick(&ev);
    }

    if (egatableloaded) {
        GrClearScreen(EgaColor(EGAI_BLACK));
        GrTextXY(10, 10, "EGA COLOR INDEXES DEFS", EgaColor(EGAI_WHITE),
                 EgaColor(EGAI_BLACK));
        GrFilledBox(10, 50, wide-10, high+30, EgaColor(EGAI_RED));
        GrFilledBox(wide+10, 50, wide*2-10, high+30, EgaColor(EGAI_GREEN));
        GrFilledBox(10, high+50, wide-10, high*2+30, EgaColor(EGAI_BLUE));
        GrFilledBox(wide+10, high+50, wide*2-10, high*2+30, EgaColor(EGAI_YELLOW));
        GrEventWaitKeyOrClick(&ev);
    }

    GrResetColors();
    if (!GrGenWebColorTable()) {
        webtableloaded = 1;
        GrClearScreen(WEBC_BLACK);
        GrTextXY(10, 10, "WEB COLOR TABLE", WEBC_WHITE, WEBC_BLACK);
        wide = (GrMaxX()-10) / 4;
        high = (GrMaxY()-52) / 35;
        ind = 0;
        for (i=0; i<35; i++) {
            for (j=0; j<4; j++) {
                if (ind < NUM_WEB_COLORS) {
                    bcolor = GrWebColorTable[ind].color;
                    paint_box(j*wide+10, i*high+50, (j+1)*wide, (i+1)*high+50,
                                bcolor, GrWebColorTable[ind].name);
                    ind++;
                }
            }
        }
        GrEventWaitKeyOrClick(&ev);
    }

    wide = GrMaxX() / 2;
    high = (GrMaxY()-52) / 2;

    GrClearScreen(GrAllocColor2(WEBC_BLACK));
    GrTextXY(10, 10, "WEB COLOR STATIC RGB DEFS", GrAllocColor2(WEBR_WHITE), GrAllocColor2(WEBR_BLACK));
    wide = GrMaxX() / 2;
    GrFilledBox(10, 50, wide-10, high+30, GrAllocColor2(WEBR_ORANGE));
    GrFilledBox(wide+10, 50, wide*2-10, high+30, GrAllocColor2(WEBR_TURQUOISE));
    GrFilledBox(10, high+50, wide-10, high*2+30, GrAllocColor2(WEBR_INDIGO));
    GrFilledBox(wide+10, high+50, wide*2-10, high*2+30, GrAllocColor2(WEBR_YELLOWGREEN));
    GrEventWaitKeyOrClick(&ev);
    
    if (webtableloaded) {
        GrClearScreen(WEBC_BLACK);
        GrTextXY(10, 10, "WEB COLOR DYNAMIC DEFS", WEBC_WHITE, WEBC_BLACK);
        wide = GrMaxX() / 2;
        GrFilledBox(10, 50, wide-10, high+30, WEBC_ORANGE);
        GrFilledBox(wide+10, 50, wide*2-10, high+30, WEBC_TURQUOISE);
        GrFilledBox(10, high+50, wide-10, high*2+30, WEBC_INDIGO);
        GrFilledBox(wide+10, high+50, wide*2-10, high*2+30, WEBC_YELLOWGREEN);
        GrEventWaitKeyOrClick(&ev);
    }

    if (webtableloaded) {
        GrClearScreen(WebColor(WEBI_BLACK));
        GrTextXY(10, 10, "WEB COLOR INDEXES DEFS", WebColor(WEBI_WHITE), WebColor(WEBI_BLACK));
        wide = GrMaxX() / 2;
        GrFilledBox(10, 50, wide-10, high+30, WebColor(WEBI_ORANGE));
        GrFilledBox(wide+10, 50, wide*2-10, high+30, WebColor(WEBI_TURQUOISE));
        GrFilledBox(10, high+50, wide-10, high*2+30, WebColor(WEBI_INDIGO));
        GrFilledBox(wide+10, high+50, wide*2-10, high*2+30, WebColor(WEBI_YELLOWGREEN));
        GrEventWaitKeyOrClick(&ev);
    }

    GrResetColors();
    if (!GrGenAnsColorTable()) {
        anstableloaded = 1;
        GrClearScreen(ANSC_BLACK);
        GrTextXY(10, 10, "ANSI COLOR TABLE by names", ANSC_WHITE, ANSC_BLACK);
        wide = (GrMaxX()-10) / 8;
        high = (GrMaxY()-52) / 32;
        ind = 0;
        for (i=0; i<32; i++) {
            for (j=0; j<8; j++) {
                if (ind < NUM_ANS_COLORS) {
                    bcolor = GrAnsColorTable[ind].color;
                    paint_box(j*wide+10, i*high+50, (j+1)*wide, (i+1)*high+50,
                                bcolor, GrAnsColorTable[ind].name);
                    ind++;
                }
            }
        }
        GrEventWaitKeyOrClick(&ev);
    }

    if (anstableloaded) {
        GrClearScreen(ANSC_BLACK);
        GrTextXY(10, 10, "ANSI COLOR TABLE by numbers", ANSC_WHITE, ANSC_BLACK);
        wide = (GrMaxX()-10) / 18;
        high = (GrMaxY()-52) / 18;
        paint_ansi_group(0, 8, wide, high, 0);
        paint_ansi_group(8, 16, wide, high, 1);
        paint_ansi_group(16, 34, wide, high, 3);
        paint_ansi_group(52, 70, wide, high, 4);
        paint_ansi_group(88, 106, wide, high, 5);
        paint_ansi_group(124, 142, wide, high, 6);
        paint_ansi_group(160, 178, wide, high, 7);
        paint_ansi_group(196, 214, wide, high, 8);
        paint_ansi_group(34, 52, wide, high, 9);
        paint_ansi_group(70, 88, wide, high, 10);
        paint_ansi_group(106, 124, wide, high, 11);
        paint_ansi_group(142, 160, wide, high, 12);
        paint_ansi_group(178, 196, wide, high, 13);
        paint_ansi_group(214, 232, wide, high, 14);
        paint_ansi_group(232, 244, wide, high, 16);
        paint_ansi_group(244, 256, wide, high, 17);
        GrEventWaitKeyOrClick(&ev);
    }

    wide = GrMaxX() / 2;
    high = (GrMaxY()-52) / 2;

    GrClearScreen(GrAllocColor2(ANSC_BLACK));
    GrTextXY(10, 10, "ANS COLOR STATIC RGB DEFS", GrAllocColor2(ANSR_WHITE), GrAllocColor2(ANSR_BLACK));
    wide = GrMaxX() / 2;
    GrFilledBox(10, 50, wide-10, high+30, GrAllocColor2(ANSR_ORANGE));
    GrFilledBox(wide+10, 50, wide*2-10, high+30, GrAllocColor2(ANSR_TURQUOISE));
    GrFilledBox(10, high+50, wide-10, high*2+30, GrAllocColor2(ANSR_INDIANRED));
    GrFilledBox(wide+10, high+50, wide*2-10, high*2+30, GrAllocColor2(ANSR_YELLOW6));
    GrEventWaitKeyOrClick(&ev);
    
    if (anstableloaded) {
        GrClearScreen(ANSC_BLACK);
        GrTextXY(10, 10, "ANS COLOR DYNAMIC DEFS", ANSC_WHITE, ANSC_BLACK);
        wide = GrMaxX() / 2;
        GrFilledBox(10, 50, wide-10, high+30, ANSC_ORANGE);
        GrFilledBox(wide+10, 50, wide*2-10, high+30, ANSC_TURQUOISE);
        GrFilledBox(10, high+50, wide-10, high*2+30, ANSC_INDIANRED);
        GrFilledBox(wide+10, high+50, wide*2-10, high*2+30, ANSC_YELLOW6);
        GrEventWaitKeyOrClick(&ev);
    }

    if (anstableloaded) {
        GrClearScreen(AnsColor(ANSI_BLACK));
        GrTextXY(10, 10, "ANS COLOR INDEXES DEFS", AnsColor(ANSI_WHITE), AnsColor(ANSI_BLACK));
        wide = GrMaxX() / 2;
        GrFilledBox(10, 50, wide-10, high+30, AnsColor(ANSI_ORANGE));
        GrFilledBox(wide+10, 50, wide*2-10, high+30, AnsColor(ANSI_TURQUOISE));
        GrFilledBox(10, high+50, wide-10, high*2+30, AnsColor(ANSI_INDIANRED));
        GrFilledBox(wide+10, high+50, wide*2-10, high*2+30, AnsColor(ANSI_YELLOW6));
        GrEventWaitKeyOrClick(&ev);
    }

    GrEventUnInit();
    GrSetMode(GR_default_text);
    return 0;
}
