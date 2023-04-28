/**
 ** fillmpol.c ---- fill a multi-polygon with a solid color
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

void GrFilledMultiPolygon(GrMultiPointArray *mpa,GrColor c)
{
    GrFillArg fval;
    fval.color = c;
    _GrScanMultiPolygon(mpa,&_GrSolidFiller,fval);
}
