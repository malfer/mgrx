/**
 ** pattmpol.c ---- draw a patterned multi-polygon
 **
 ** Copyright (C) 2023 Mariano Alvarez Fernandez
 ** [e-mail: malfer@telefonica.net]
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


void GrPatternedMultiPolygon(GrMultiPointArray *mpa,GrLinePattern *lp)
{
    int k;
    GrFillArg fval;
    
    fval.p = lp->lnp_pattern;

    for (k=0; k<mpa->npa; k++) {
        _GrDrawCustomPolygon(mpa->p[k].npoints,mpa->p[k].points,lp->lnp_option,
                             &_GrPatternFiller,fval,mpa->p[k].closed,FALSE);
    }
}

void GrPatndAlignMultiPolygon(int xo,int yo,GrMultiPointArray *mpa,GrLinePattern *lp)
{
    int k;
    GrFillArg fval;
    
    xo += CURC->gc_xoffset;
    yo += CURC->gc_yoffset;
    fval.pa.p = lp->lnp_pattern;
    fval.pa.xo = xo;
    fval.pa.yo = yo;

    for (k=0; k<mpa->npa; k++) {
        _GrDrawCustomPolygon(mpa->p[k].npoints,mpa->p[k].points,lp->lnp_option,
                             &_GrPatternAlignFiller,fval,mpa->p[k].closed,FALSE);
    }
}
