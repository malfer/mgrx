/**
 ** pfellia.c
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 **  Copyright (C) 1992, Csaba Biegl
 **    820 Stirrup Dr, Nashville, TN, 37221
 **    csaba@vuse.vanderbilt.edu
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
 ** 200625 M.Alvarez, adding GrPatAlignFilledEllipseArc
 **
 **/

#include "libgrx.h"
#include "clipping.h"
#include "shapes.h"

void GrPatternFilledEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrPattern *p)
{
    int points[GR_MAX_ELLIPSE_POINTS+1][2];
    GrFillArg fa;
    int numpts;

    numpts = GrGenerateEllipseArc(xc,yc,xa,ya,start,end,points);
        
    if (style == GR_ARC_STYLE_CLOSE2) {
        points[numpts][0] = xc;
        points[numpts][1] = yc;
        numpts++;
    }
    fa.p = p;
    //_GrScanConvexPoly((-numpts),points,&_GrPatternFiller,fa);
    _GrScanPolygon(numpts, points,&_GrPatternFiller,fa);
}

void GrPatAlignFilledEllipseArc(int xo,int yo,int xc,int yc,int xa,int ya,int start,int end,int style,GrPattern *p)
{
    int points[GR_MAX_ELLIPSE_POINTS+1][2];
    GrFillArg fa;
    int numpts;

    numpts = GrGenerateEllipseArc(xc,yc,xa,ya,start,end,points);
        
    if (style == GR_ARC_STYLE_CLOSE2) {
        points[numpts][0] = xc;
        points[numpts][1] = yc;
        numpts++;
    }

    fa.pa.p = p;
    fa.pa.xo = xo + CURC->gc_xoffset;
    fa.pa.yo = yo + CURC->gc_yoffset;
    _GrScanPolygon(numpts, points,&_GrPatternAlignFiller,fa);
}
