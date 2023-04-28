/**
 ** custmpol.c ---- draw a closed dashed and/or wide multi-polygon
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

void GrCustomMultiPolygon(GrMultiPointArray *mpa,const GrLineOption *o)
{
    int k;
    GrFillArg fval;

    fval.color = o->lno_color;

    for (k=0; k<mpa->npa; k++) {
        _GrDrawCustomPolygon(mpa->p[k].npoints,mpa->p[k].points,o,
                             &_GrSolidFiller,fval,mpa->p[k].closed,FALSE);
    }
}
