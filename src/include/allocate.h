/**
 ** allocate.h ---- some allocate functions
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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

/* temp buffer for blits etc. */
extern void *_GrTempBuffer;
extern unsigned  _GrTempBufferBytes;
#define _GrTempBufferAlloc(b) (                                     \
    ((unsigned)(b) <= _GrTempBufferBytes) ? _GrTempBuffer           \
                      : _GrTempBufferAlloc_(b) )
extern void *_GrTempBufferAlloc_(size_t bytes);
extern void _GrTempBufferFree(void);
