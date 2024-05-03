/**
 ** ftable.c ---- a table of available frame drivers
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
 ** Contributions by:
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 ** 230517 M.Alvarez, added the new 32bpp linear and memory framebuffers
 ** 230717 M.Alvarez, added the new 24bpp linear and memory framebuffers
 ** 230717 M.Alvarez, added the new 16bpp linear and memory framebuffers
 ** 240324 M.Alvarez, Reorganization and added framedrivers for Wayland
 **/

#include "libgrx.h"
#include "grdriver.h"

GrFrameDriver *_GrFrameDriverTable[] = {
/* first the drivers for video based context */
#if defined(__MSDOS__) 
    &_GrFrameDriverEGAVGA1,
    &_GrFrameDriverEGA4,
    &_GrFrameDriverSVGA4,
    &_GrFrameDriverVGA8X,
    &_GrFrameDriverSVGA8,
    &_GrFrameDriverSVGA16,
    &_GrFrameDriverSVGA24,
    &_GrFrameDriverSVGA32L,
    &_GrFrameDriverSVGA32H,
    &_GrFrameDriverSVGA8_LFB,
    &_GrFrameDriverSVGA16_LFB,
    &_GrFrameDriverSVGA24_LFB,
    &_GrFrameDriverSVGA32L_LFB,
    &_GrFrameDriverSVGA32H_LFB,

#elif defined(__WIN32__)
    &_GrFrameDriverNLFB16,
    &_GrFrameDriverNLFB24,
    &_GrFrameDriverNLFB32L,
    &_GrFrameDriverNLFB32H,
    &_GrFrameDriverWIN32_8,
    &_GrFrameDriverWIN32_24,

#elif defined(__XWIN__)
    #if defined(XF86DGA_DRIVER)
    &_GrFrameDriverSVGA8_LFB,
    &_GrFrameDriverSVGA16_LFB,
    &_GrFrameDriverSVGA24_LFB,
    &_GrFrameDriverSVGA32L_LFB,
    &_GrFrameDriverSVGA32H_LFB,
    #endif
    &_GrFrameDriverNLFB16,
    &_GrFrameDriverNLFB24,
    &_GrFrameDriverNLFB32L,
    &_GrFrameDriverNLFB32H,
    &_GrFrameDriverXWIN8,
    &_GrFrameDriverXWIN16,
    &_GrFrameDriverXWIN24,
    &_GrFrameDriverXWIN32L,
    &_GrFrameDriverXWIN32H,

#elif defined(__WAYLAND__)
    &_GrFrameDriverNLFB16,
    &_GrFrameDriverNLFB24,
    &_GrFrameDriverNLFB32L,
    &_GrFrameDriverNLFB32H,

#elif defined(__linux__)
    &_GrFrameDriverNLFB16,
    &_GrFrameDriverNLFB24,
    &_GrFrameDriverNLFB32L,
    &_GrFrameDriverNLFB32H,
    &_GrFrameDriverSVGA8_LFB,
    &_GrFrameDriverLNXFB_16,
    &_GrFrameDriverSVGA24_LFB,
    &_GrFrameDriverLNXFB_32L,
    &_GrFrameDriverLNXFB_32H,
#endif

/* now the drivers for RAM based context */
    &_GrFrameDriverRAM1,
    &_GrFrameDriverRAM4,
    &_GrFrameDriverRAM8,
    &_GrFrameDriverRAM16,
    &_GrFrameDriverRAM24,
    &_GrFrameDriverRAM32L,
    &_GrFrameDriverRAM32H,
    &_GrFrameDriverNRAM16,
    &_GrFrameDriverNRAM24,
    &_GrFrameDriverNRAM32L,
    &_GrFrameDriverNRAM32H,
    NULL
};
