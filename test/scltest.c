/**
 ** scltest.c ---- scanline functions test
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

#include "test.h"

static void PrintInfo(long t)
{
    char aux[81];
    int x, y;

    sprintf(aux, " Elapsed time: %ld msec ", t);
    x = (GrMaxX() -
        GrFontStringWidth(GrGetDefaultFont(), aux, strlen(aux), GR_BYTE_TEXT)) / 2;
    y = (GrMaxY() -
        GrFontStringHeight(GrGetDefaultFont(), aux, strlen(aux), GR_BYTE_TEXT)) / 2;
    GrTextXY(x, y, aux, GrWhite(), GrBlack());
}

TESTFUNC(scltest)
{
    GrContext *ctx;
    int  w, h;
    GrColor c1, c2, c3, *cbuf;
    const GrColor *rbuf;
    int i, j;
    GrEvent ev;
    long t1, t2;

    // Restrict the test to 640x480 to not be so long
    if (GrSizeX() > 640 && GrSizeY() > 480) {
        w = 640;
        h = 480;
        int worg = (GrSizeX() - w) / 2;
        int horg = (GrSizeY() - h) / 2;
        ctx = GrCreateSubContext(worg, horg, worg+w-1, horg+h-1, NULL, NULL);
        GrSetContext(ctx);
    } else {
        w = GrSizeX();
        h = GrSizeY();
    }

    c1 = GrAllocColor(255,100,0);
    c2 = GrAllocColor(0,0,180);
    c3 = GrAllocColor(0,255,0);
    drawing(0,0,w,h,c1,c2);
    GrBox(0, 0, w-1, h-1, c3);
    GrEventWaitKeyOrClick(&ev);

    cbuf = malloc(sizeof(GrColor)*w);
    if (cbuf == NULL) return;

    GrMouseEraseCursor();
    t1 = GrMsecTime();
    for (j=0; j<h; j++) {
        rbuf = GrGetScanline(0, w-1, 0);
        if (rbuf == NULL) return;
        for (i=0; i<w; i++) cbuf[i] = rbuf[i];

        for (i=1; i<h; i++) {
            rbuf = GrGetScanline(0, w-1, i);
            if (rbuf == NULL) return;
            GrPutScanline(0, w-1, i-1, rbuf, GrWRITE);
        }
        GrPutScanline(0, w-1, h-1, cbuf, GrWRITE);
    }
    t2 = GrMsecTime();
    GrMouseDisplayCursor();
    PrintInfo(t2-t1);
    GrEventWaitKeyOrClick(&ev);
}
