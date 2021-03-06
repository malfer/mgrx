/**
 ** pfcirc.c
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
 ** 200624 M.Alvarez, adding GrPatAlignFilledCircle
 **
 **/

#include "libgrx.h"

void GrPatternFilledCircle(int xc,int yc,int r,GrPattern *p)
{
    GrPatternFilledEllipse(xc,yc,r,r,p);
}

void GrPatAlignFilledCircle(int xo,int yo,int xc,int yc,int r,GrPattern *p)
{
    GrPatAlignFilledEllipse(xo,yo,xc,yc,r,r,p);
}
