/**
 ** mpoltest.c ---- test multi-polygon
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

/* default mode */

static int gwidth = 800;
static int gheight = 600;
static int gbpp = 8;

int main(int argc,char **argv)
{
    GrEvent ev;
    GrMultiPointArray *mpa = NULL;
    int pt1[5][2] = {{100,200},{400,120},{700,200},{700,500},{100,500}};
    int pt2[4][2] = {{200,300},{600,300},{600,400},{200,400}};
    int pt3[4][2] = {{300,450},{500,450},{500,480},{300,480}};
    int pt4[4][2] = {{100,100},{700,100},{700,150},{100,150}};
    int pt5[4][2] = {{100,50},{700,50},{700,80},{100,80}};
    unsigned char bmpdata[8] = {0x00, 0x7E, 0x82, 0x82, 0x82, 0x82, 0x7E, 0x00};
    GrBitmap bm1 = {0, 8, NULL, 0, 0, 0};
    GrPattern *lg1 = NULL;
    GrLineOption lopt = { 0, 7, 0, NULL };
    GrLinePattern lpat;
    GrMultiPointArray *mpdon = NULL;
    int don1[GR_MAX_ELLIPSE_POINTS][2];
    int don2[GR_MAX_ELLIPSE_POINTS][2];

    if (argc >= 4) {
        gwidth = atoi(argv[1]);
        gheight = atoi(argv[2]);
        gbpp = atoi(argv[3]);
    }

    GrSetMode(GR_width_height_bpp_graphics, gwidth, gheight, gbpp);

    GrEventInit();
    GrMouseDisplayCursor();

    mpa = malloc(sizeof(GrMultiPointArray)+sizeof(GrPointArray)*9);
    if (mpa == NULL) goto end;
    mpa->npa = 5;
    mpa->p[0].npoints = 5;
    mpa->p[0].closed = 1;
    mpa->p[0].points = pt1;
    mpa->p[1].npoints = 4;
    mpa->p[1].closed = 0;
    mpa->p[1].points = pt2;
    mpa->p[2].npoints = 4;
    mpa->p[2].closed = 1;
    mpa->p[2].points = pt3;
    mpa->p[3].npoints = 4;
    mpa->p[3].closed = 1;
    mpa->p[3].points = pt4;
    mpa->p[4].npoints = 4;
    mpa->p[4].closed = 0;
    mpa->p[4].points = pt5;

    mpdon = malloc(sizeof(GrMultiPointArray)+sizeof(GrPointArray)*1);
    if (mpdon == NULL) goto end;
    mpdon->npa = 2;
    mpdon->p[0].npoints = GrGenerateEllipse(400, 300, 300, 200, don1);
    mpdon->p[0].closed = 1;
    mpdon->p[0].points = don1;
    mpdon->p[1].npoints = GrGenerateEllipse(400, 300, 200, 100, don2);
    mpdon->p[1].closed = 1;
    mpdon->p[1].points = don2;

    GrFilledMultiPolygon(mpa, GrAllocColor(100,200,200));
    GrEventWaitKeyOrClick(&ev);

    bm1.bmp_fgcolor = GrWhite();
    bm1.bmp_bgcolor = GrBlack();
    bm1.bmp_data = (char *)bmpdata;
    GrClearContext(GrBlack());
    GrPatternFilledMultiPolygon(mpa, (GrPattern *)&bm1);
    GrEventWaitKeyOrClick(&ev);

    lg1 = GrCreateLinGradient(0, GrMaxY(), GrMaxX(), 0);
    if (lg1 == NULL) goto end;
    GrAddGradientStop(lg1, 0, GrAllocColor(255, 0, 0));
    GrAddGradientStop(lg1, 80, GrAllocColor(0, 255, 0));
    GrAddGradientStop(lg1, 100, GrAllocColor(0, 255, 0));
    GrAddGradientStop(lg1, 200, GrAllocColor(0, 0, 255));
    GrAddGradientStop(lg1, 255, GrAllocColor(0, 255, 255));
    GrGenGradientColorTbl(lg1);
    GrClearContext(GrBlack());
    GrPatternFilledMultiPolygon(mpa, lg1);
    GrEventWaitKeyOrClick(&ev);

    GrClearContext(GrWhite());
    GrPatAlignFilledMultiPolygon(GrMaxX()/2, GrMaxY()/2, mpa, lg1);
    GrEventWaitKeyOrClick(&ev);

    GrClearContext(GrBlack());
    GrMultiPolygon(mpa, GrAllocColor(100,200,200));
    GrEventWaitKeyOrClick(&ev);

    lopt.lno_color = GrAllocColor(100,200,200);
    GrClearContext(GrBlack());
    GrCustomMultiPolygon(mpa, &lopt);
    GrEventWaitKeyOrClick(&ev);

    lpat.lnp_pattern = lg1;
    lpat.lnp_option = &lopt;
    GrClearContext(GrBlack());
    GrPatternedMultiPolygon(mpa, &lpat);
    GrEventWaitKeyOrClick(&ev);

    GrClearContext(GrWhite());
    GrPatndAlignMultiPolygon(GrMaxX()/2, GrMaxY()/2, mpa, &lpat);
    GrEventWaitKeyOrClick(&ev);

    GrClearContext(GrBlack());
    GrPatternFilledMultiPolygon(mpdon, (GrPattern *)&bm1);
    GrCustomMultiPolygon(mpdon, &lopt);
    GrEventWaitKeyOrClick(&ev);

end:
    if (mpa) free(mpa);
    if (mpdon) free(mpdon);
    if (lg1) GrDestroyPattern(lg1);
    GrEventUnInit();
    GrSetMode(GR_default_text);
    return 0;
}
