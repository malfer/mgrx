/**
 ** fillell2.c ---- filled ellipse arc
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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
 **/

#include "libgrx.h"
#include "shapes.h"

void GrFilledEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrColor c)
{
    int pnts[GR_MAX_ELLIPSE_POINTS+1][2];
    GrFillArg fval;
    int npts;

    npts  = GrGenerateEllipseArc(xc,yc,xa,ya,start,end,pnts);
    if(style == GR_ARC_STYLE_CLOSE2) {
        pnts[npts][0] = xc;
        pnts[npts][1] = yc;
        npts++;
    }
    fval.color = c;
    _GrScanPolygon(npts,pnts,&_GrSolidFiller,fval);
}
