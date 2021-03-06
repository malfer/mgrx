/**
 ** ptelli.c
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
 ** 200626 M.Alvarez, adding GrPatndAlignEllipse
 **
 **/

#include "libgrx.h"
#include "allocate.h"
#include "shapes.h"

void GrPatternedEllipse(int xc,int yc,int xa,int ya,GrLinePattern *lp)
{
    int points[GR_MAX_ELLIPSE_POINTS][2];
    GrFillArg fval;
    int numpts;
 
    numpts = GrGenerateEllipse(xc,yc,xa,ya,points);
    fval.p = lp->lnp_pattern;
    _GrDrawCustomPolygon(numpts,points,lp->lnp_option,
                         &_GrPatternFiller,fval,TRUE,TRUE);
}

void GrPatndAlignEllipse(int xo,int yo,int xc,int yc,int xa,int ya,GrLinePattern *lp)
{
    int points[GR_MAX_ELLIPSE_POINTS][2];
    GrFillArg fval;
    int numpts;
    
    numpts = GrGenerateEllipse(xc,yc,xa,ya,points);
    xo += CURC->gc_xoffset;
    yo += CURC->gc_yoffset;
    fval.pa.p = lp->lnp_pattern;
    fval.pa.xo = xo + CURC->gc_xoffset;
    fval.pa.yo = yo + CURC->gc_xoffset;
    _GrDrawCustomPolygon(numpts,points,lp->lnp_option,
                         &_GrPatternAlignFiller,fval,TRUE,TRUE);
}
