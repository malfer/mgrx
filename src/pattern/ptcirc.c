/**
 ** ptcirc.c
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
 ** 200626 M.Alvarez, adding GrPatndAlignCircle
 **
 **/

#include "libgrx.h"

void GrPatternedCircle(int xc,int yc,int r,GrLinePattern *lp)
{
    GrPatternedEllipse(xc,yc,r,r,lp);
}

void GrPatndAlignCircle(int xo,int yo,int xc,int yc,int r,GrLinePattern *lp)
{
    GrPatndAlignEllipse(xo,yo,xc,yc,r,r,lp);
}
