/**
 ** upolylin.c
 **
 ** Copyright (C), Michael Goffioul
 ** [goffioul@emic.ucl.ac.be]
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
#include "usercord.h"

void GrUsrPolyLine(int numpts,int points[][2],GrColor c)
{
    int pt;
    int tmp[numpts][2];
    
    for ( pt = 0; pt < numpts; pt++) {
        tmp[pt][0] = points[pt][0];
        tmp[pt][1] = points[pt][1];
        U2SX(tmp[pt][0],CURC);
        U2SY(tmp[pt][1],CURC);
    }
    GrPolyLine(numpts,tmp,c);
}
