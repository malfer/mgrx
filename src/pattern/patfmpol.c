/**
 ** patfmpol.c
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
#include "clipping.h"
#include "shapes.h"

void GrPatternFilledMultiPolygon(GrMultiPointArray *mpa,GrPattern *p)
{
    GrFillArg fa;

    fa.p = p;
    _GrScanMultiPolygon(mpa,&_GrPatternFiller,fa);
}

void GrPatAlignFilledMultiPolygon(int xo,int yo,GrMultiPointArray *mpa,GrPattern *p)
{
    GrFillArg fa;
    xo += CURC->gc_xoffset;
    yo += CURC->gc_yoffset;

    fa.pa.p = p;
    fa.pa.xo = xo;
    fa.pa.yo = yo;
    _GrScanMultiPolygon(mpa,&_GrPatternAlignFiller,fa);
}
