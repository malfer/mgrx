/**
 ** vtable.c ---- a table of available video drivers
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
 ** Contributions by:
 ** 240324 M.Alvarez, Reorganization and added videodrivers for Wayland
 **/

#include "libgrx.h"
#include "grdriver.h"

#if defined(__MSDOS__)
GrVideoDriver *_GrVideoDriverTable[] = {
    &_GrVideoDriverVESA,
    &_GrVideoDriverSTDVGA,
    &_GrVideoDriverSTDEGA,
    &_GrDriverMEM,
    NULL};

#elif defined(__WIN32__)
GrVideoDriver *_GrVideoDriverTable[] = {
    &_GrVideoDriverWIN32,
    &_GrDriverMEM,
    NULL};

#elif defined(__XWIN__)
GrVideoDriver *_GrVideoDriverTable[] = {
    #if defined(XF86DGA_DRIVER)
    &_GrVideoDriverXF86DGA,
    #endif
    &_GrVideoDriverXWIN,
    &_GrDriverMEM,
    NULL};

#elif defined(__WAYLAND__)
GrVideoDriver *_GrVideoDriverTable[] = {
    &_GrVideoDriverWAYLAND,
    &_GrDriverMEM,
    NULL};

#elif defined(__linux__)
GrVideoDriver *_GrVideoDriverTable[] = {
    &_GrVideoDriverLINUXFB,
    #if defined(LNDRM_DRIVER)
    &_GrVideoDriverLINUXDRM,
    #endif
    &_GrDriverMEM,
    NULL};
#endif
