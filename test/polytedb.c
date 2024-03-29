/**
 ** polytedb.c ---- test polygon rendering using double buffer
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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

#include <string.h>
#include <stdio.h>

#include "test.h"

GrContext *grc; // double buffer

static GrColor *EGA;
#define black EGA[0]
#define red   EGA[12]
#define blue  EGA[1]
#define white EGA[15]

static void printdata(int n, int points[][2], int convex)
{
    int i, xpos;
    char buf[41];

    if (!convex)
        sprintf(buf, "Points");
    else
        sprintf(buf, "Convex");

    xpos = GrMaxX() - 90;
    GrTextXY(xpos, 0, buf, white, black);

    for (i=0; i<n; i++) {
        sprintf(buf, "%d %d", points[i][0], points[i][1]);
        GrTextXY(xpos, (i+1)*16, buf, white, black);
    }
}

static void testpoly(int n, int points[][2], int convex)
{
    GrEvent ev;

    GrSetContext( grc );

    GrClearContext(black);
    printdata(n, points, 0);
    GrPolygon(n, points, white);
    GrFilledPolygon(n, points, red);
    GrBitBlt(GrScreenContext(),0,0,grc,0,0,grc->gc_xmax,grc->gc_ymax,GrWRITE);
    GrEventWaitKeyOrClick(&ev);
    if (convex || (n <= 3)) {
        GrClearContext(black);
        printdata(n, points, 1);
        GrPolygon(n, points, white);
        GrFilledConvexPolygon(n, points, blue);
        GrBitBlt(GrScreenContext(),0,0,grc,0,0,grc->gc_xmax,grc->gc_ymax,GrWRITE);
        GrEventWaitKeyOrClick(&ev);
    }

    GrSetContext( NULL );
}

static void doscan(void)
{
    int pts[4][2];
    int ww = GrSizeX() / 10;
    int hh = GrSizeY() / 10;
    int sx = (GrSizeX() - 2*ww) / 32;
    int sy = (GrSizeY() - 2*hh) / 32;
    int  ii, jj;
    GrColor color;
    
    pts[0][1] = 0;
    pts[1][1] = hh;
    pts[2][1] = 2*hh;
    pts[3][1] = hh;
    color = 0;
    for (ii = 0; ii < 32; ii++) {
        pts[0][0] = ww;
        pts[1][0] = 2*ww;
        pts[2][0] = ww;
        pts[3][0] = 0;
        for (jj = 0; jj < 32; jj++) {
            GrFilledPolygon(4, pts, EGA[color] | GrXOR);
            GrBitBlt(GrScreenContext(),pts[3][0],pts[0][1],grc,pts[3][0],pts[0][1],
                     pts[1][0],pts[2][1],GrWRITE);
            color = (color + 1) & 15;
            pts[0][0] += sx;
            pts[1][0] += sx;
            pts[2][0] += sx;
            pts[3][0] += sx;
        }
        pts[0][1] += sy;
        pts[1][1] += sy;
        pts[2][1] += sy;
        pts[3][1] += sy;
    }
}

static void doscan_convex(void)
{
    int pts[4][2];
    int ww = GrSizeX() / 10;
    int hh = GrSizeY() / 10;
    int sx = (GrSizeX() - 2*ww) / 32;
    int sy = (GrSizeY() - 2*hh) / 32;
    int  ii, jj;
    GrColor color;
    
    pts[0][1] = 0;
    pts[1][1] = hh;
    pts[2][1] = 2*hh;
    pts[3][1] = hh;
    color = 0;
    for (ii = 0; ii < 32; ii++) {
        pts[0][0] = ww;
        pts[1][0] = 2*ww;
        pts[2][0] = ww;
        pts[3][0] = 0;
        for (jj = 0; jj < 32; jj++) {
            GrFilledConvexPolygon(4, pts, EGA[color] | GrXOR);
            GrBitBlt(GrScreenContext(),pts[3][0],pts[0][1],grc,pts[3][0],pts[0][1],
                     pts[1][0],pts[2][1],GrWRITE);
            color = (color + 1) & 15;
            pts[0][0] += sx;
            pts[1][0] += sx;
            pts[2][0] += sx;
            pts[3][0] += sx;
        }
        pts[0][1] += sy;
        pts[1][1] += sy;
        pts[2][1] += sy;
        pts[3][1] += sy;
    }
}

static void speedtest(void)
{
    clock_t t1, t2, t3, t4;
    char msg[81];
    int i, rounds;

    rounds = 2;
    GrSetContext( grc );
    
    GrClearScreen(black);
    GrClearContext(black);
    t1 = GrMsecTime();
    for (i=0; i<rounds; i++) {
        doscan();
    }
    t2 = GrMsecTime();

    GrClearContext(black);
    t3 = GrMsecTime();
    for (i=0; i<rounds; i++) {
        doscan_convex();
    }
    t4 = GrMsecTime();

    GrSetContext( NULL );

    sprintf(msg, "Times to scan %d polygons", 1024*rounds);
    GrTextXY(0, 0, msg, white, black);
    sprintf(msg, "   with 'GrFilledPolygon': %.2f (s)",
            (double)(t2 - t1) / (double)1000);
    GrTextXY(0, 18, msg, white, black);
    sprintf(msg, "   with 'GrFilledConvexPolygon': %.2f (s)",
            (double)(t4 - t3) / (double)1000);
    GrTextXY(0, 36, msg, white, black);
}

TESTFUNC(ptest)
{
    char buff[300];
    int  pts[300][2];
    int  ii,collect;
    int  convex;
    FILE *fp;
    GrEvent ev;

    if ( (grc = GrCreateContext( GrScreenX(),GrScreenY(),NULL,NULL )) == NULL ) {
        printf("Error creating double buffer\n");
        exit(1);
    }

    fp = fopen("polytest.dat","r");
    if (fp == NULL) return;
    EGA = GrAllocEgaColors();
    ii  = collect = convex = 0;
    while (fgets(buff, 299, fp) != NULL) {
        if (!collect) {
            if (strncmp(buff, "begin", 5) == 0) {
                convex  = (buff[5] == 'c');
                collect = 1;
                ii      = 0;
            }
            continue;
        }
        if (strncmp(buff, "end", 3) == 0) {
            testpoly(ii, pts, convex);
            collect = 0;
            continue;
        }
        if (sscanf(buff, "%d %d", &pts[ii][0], &pts[ii][1]) == 2) ii++;
    }
    fclose(fp);
    speedtest();
    GrEventWaitKeyOrClick(&ev);
}
