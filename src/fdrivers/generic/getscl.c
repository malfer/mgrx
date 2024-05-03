/**
 ** generic/getiscl.c ---- generic getscanline routine
 **
 ** Copyright (c) 1998 Hartmut Schirmer
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

GrColor *getscanline(GrFrame *c,int x,int y,int w)
{
   GrColor *pixels;
   GrColor *p;
   GRX_ENTER();
   p = pixels = _GrTempBufferAlloc(sizeof(GrColor) * (w+1));
   if (pixels) {
     for ( ; w > 0; --w)
       *(p++) = readpixel(c,x++,y);
   }
   GRX_RETURN(pixels);
}
