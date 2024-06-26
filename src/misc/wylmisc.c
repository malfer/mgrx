/**
 ** wylmisc.c - miscellaneous functions for Wayland
 **
 ** Copyright (C) 2024 Mariano Alvarez Fernandez
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
 **/

#include <unistd.h>
#include <time.h>
//#include <sys/times.h>
#include "libgrx.h"
#include "libwyl.h"

void GrSetWindowTitle(char *title)
{
    if (_WGrState.wl_display) {
        xdg_toplevel_set_title(_WGrState.xdg_toplevel, title);
        _WGrUserHadSetWName = 1;
    } else {
        _WGrUserHadSetWName = 0;
    }
}

void GrSleep( int msec )
{
  usleep(msec*1000L);
}

/*long GrMsecTime( void )
{
  return ((long)times(NULL) * (1000L / sysconf(_SC_CLK_TCK)));
}*/

long GrMsecTime( void )
{
  static time_t orig = 0;
  struct timespec tp;
  
  clock_gettime(CLOCK_MONOTONIC, &tp);
  if (orig == 0) orig = tp.tv_sec;
  return (((long)(tp.tv_sec - orig) * 1000L) + ((long)tp.tv_nsec / 1000000L));
}
