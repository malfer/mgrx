/**
 ** pfcirca.c
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
 ** 200625 M.Alvarez, adding GrPatAlignFilledCircleArc
 **
 **/

#include "libgrx.h"

void GrPatternFilledCircleArc(int xc,int yc,int r,int start,int end,int style,GrPattern *p)
{
    GrPatternFilledEllipseArc(xc,yc,r,r,start,end,style,p);
}

void GrPatAlignFilledCircleArc(int xo,int yo,int xc,int yc,int r,int start,int end,int style,GrPattern *p)
{
    GrPatAlignFilledEllipseArc(xo,yo,xc,yc,r,r,start,end,style,p);
}
