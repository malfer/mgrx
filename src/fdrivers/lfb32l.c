/**
 ** lfb32l.c ---- the 16M color padded SVGA linear frame buffer driver (low)
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu].
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
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 ** Andrzej Lawa [FidoNet: Andrzej Lawa 2:480/19.77]
 **
 **/

/* some systems map LFB in normal user space (eg. Linux/svgalib) */
/* near pointer stuff is equal to ram stuff :)                   */
/* in this is the far pointer code using %fs descriptor          */
#ifndef LFB_BY_NEAR_POINTER

#define PIX2COL(col) ((col)&0xFFFFFF)
#define COL2PIX(col) ((col)&0xFFFFFF)
#define FAR_ACCESS
#include "fdrivers/driver32.h"

GrFrameDriver _GrFrameDriverSVGA32L_LFB = {
    GR_frameSVGA32L_LFB,        /* frame mode */
    GR_frameRAM32L,             /* compatible RAM frame mode */
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
    bitblt,
    bltv2r,
    bltr2v,
    _GrFrDrvGenericGetScanline,
    _GrFrDrvGenericPutScanline
};

#endif
