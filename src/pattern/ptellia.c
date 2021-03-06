/**
 ** ptellia.c
 **
 ** Copyright (C) 1997, Michael Goffioul
 ** [e-mail : goffioul@emic.ucl.ac.be]
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
 ** 200626 M.Alvarez, adding GrPatndAlignEllipseArc
 **
 **/

#include "libgrx.h"
#include "shapes.h"

void GrPatternedEllipseArc(int xc,int yc,int xa,int ya,int start,int end,int style,GrLinePattern *lp)
{
    int points[GR_MAX_ELLIPSE_POINTS+2][2];
    GrFillArg fval;
    int numpts;
    int close, circle;
    
    numpts = GrGenerateEllipseArc(xc,yc,xa,ya,start,end,points);
    close = FALSE;
    circle = TRUE;
    if (style == GR_ARC_STYLE_CLOSE2) {
        points[numpts][0] = xc;
        points[numpts][1] = yc;
        numpts++;
        points[numpts][0] = points[0][0];
        points[numpts][1] = points[0][1];
        numpts++;
        close = TRUE;
        circle = FALSE;
    }
    if (style == GR_ARC_STYLE_CLOSE1) {
        points[numpts][0] = points[0][0];
        points[numpts][1] = points[0][1];
        numpts++;
        close = TRUE;
        circle = FALSE;
    }
    fval.p = lp->lnp_pattern;
    _GrDrawCustomPolygon(numpts,points,lp->lnp_option,
                         &_GrPatternFiller,fval,close,circle);
}

void GrPatndAlignEllipseArc(int xo,int yo,int xc,int yc,int xa,int ya,int start,int end,int style,GrLinePattern *lp)
{
    int points[GR_MAX_ELLIPSE_POINTS+2][2];
    GrFillArg fval;
    int numpts;
    int close, circle;
    
    numpts = GrGenerateEllipseArc(xc,yc,xa,ya,start,end,points);
    close = FALSE;
    circle = TRUE;
    if (style == GR_ARC_STYLE_CLOSE2) {
        points[numpts][0] = xc;
        points[numpts][1] = yc;
        numpts++;
        points[numpts][0] = points[0][0];
        points[numpts][1] = points[0][1];
        numpts++;
        close = TRUE;
        circle = FALSE;
    }
    if (style == GR_ARC_STYLE_CLOSE1) {
        points[numpts][0] = points[0][0];
        points[numpts][1] = points[0][1];
        numpts++;
        close = TRUE;
        circle = FALSE;
    }
    fval.pa.p = lp->lnp_pattern;
    fval.pa.xo = xo + CURC->gc_xoffset;
    fval.pa.yo = yo  + CURC->gc_xoffset;
    _GrDrawCustomPolygon(numpts,points,lp->lnp_option,
                         &_GrPatternAlignFiller,fval,close,circle);
}
