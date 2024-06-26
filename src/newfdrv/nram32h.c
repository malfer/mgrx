/**
 ** nram32l.c ---- the new 32bpp (24bpp padded high) linear
 **                in-memory and video frame buffer drivers
 **
 ** Copyright (c) 2019 Mariano Alvarez Fernandez
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
 ** This driver doesn't use all the memfill, mempeek infraestructure, only
 ** C standard bit operations and C standard functions.
 **
 **/

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"
#include "allocate.h"

#define PIX2COL(col) ((col)>>8)
#define COL2PIX(col) ((col)<<8)

#include "ndrvr32.h"

/* the linear in-memory frambe buffer */

GrFrameDriver _GrFrameDriverNRAM32H = {
    GR_frameNRAM32H,            /* frame mode */
    GR_frameUndef,              /* compatible RAM frame mode */
    FALSE,                      /* onscreen */
    4,                          /* scan line width alignment */
    1,                          /* number of planes */
    32,                         /* bits per pixel */
    32*16*1024L*1024L,          /* max plane size the code can handle */
    NULL,
    readpixel,
    drawpixel,
    drawline,
    drawhline,
    drawvline,
    drawblock,
    drawbitmap,
    drawpattern,
    bitbltovl,
    NULL,
    NULL,
    getscanline,
    putscanline
};

/* the linear video frame buffer */

GrFrameDriver _GrFrameDriverNLFB32H = {
    GR_frameNLFB32H,            /* frame mode */
    GR_frameNRAM32H,            /* compatible RAM frame mode */
    TRUE,                       /* onscreen */
    4,                          /* line width alignment */
    1,                          /* number of planes */
    32,                         /* bits per pixel */
    32*16*1024L*1024L,          /* max plane size the code can handle */
    NULL,
    readpixel,
    drawpixel,
    drawline,
    drawhline,
    drawvline,
    drawblock,
    drawbitmap,
    drawpattern,
    bitbltovl,
    bitbltnoo,
    bitbltnoo,
    getscanline,
    putscanline
};
