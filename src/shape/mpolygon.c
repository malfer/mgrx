/**
 ** mpolygon.c ---- draw a multi-polygon
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

void GrMultiPolygon(GrMultiPointArray *mpa,GrColor c)
{
    int k;
    GrFillArg fval;

    fval.color = c;

    for (k=0; k<mpa->npa; k++) {
        _GrDrawPolygon(mpa->p[k].npoints,mpa->p[k].points,
                       &_GrSolidFiller,fval,mpa->p[k].closed);
    }
}
