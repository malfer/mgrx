/**
 ** setdrvr.c ---- video driver setup
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
 ** 211110 M.Alvarez, added GrSetDriverExt
 ** 211121 M.Alvarez, added MGRXMINWG environment variable
 **/

#include <ctype.h>

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"

#define NCB 100

static char * nxtoken(char *p, char *token)
{
    while (*p == ' ') p++;
    while( *p && (*p != ' ')) *token++ = *p++;
    *token = '\0';
    return (p);
}

int GrSetDriver(char *drvspec)
{
    return GrSetDriverExt(drvspec, NULL);
}

int GrSetDriverExt(char *drvspec, char *drvopt)
{
    static int firsttime = TRUE;
    GrVideoDriver *drv = NULL;
    char options[NCB+1] = {0};
    char *biggdims, *minwgdims;
    
    if ((biggdims = getenv("MGRXBIGG")) != NULL) {
        sscanf(biggdims, "%d %d", &DRVINFO->biggw, &DRVINFO->biggh);
    }
    if ((minwgdims = getenv("MGRXMINWG")) != NULL) {
        sscanf(minwgdims, "%d %d", &DRVINFO->minwgw, &DRVINFO->minwgh);
    }
    
    if (!drvspec) drvspec = getenv("MGRXDRV");
    if (drvspec) {
        char dspec[NCB+1] = {0};
        char name[NCB+1] = {0};
        char t[NCB+1] = {0};
        char *p = dspec;
        strncpy(dspec, drvspec, NCB); // protection for long strings
        while (p = nxtoken(p, t), t[0] != '\0') {
            if (strlen(t) == 2) {
                #define CH16(c1, c2)  (((c1) << 8) | (c2))
                void *oaddr = NULL;
                int  cf = FALSE;
                switch (CH16(tolower(t[0]), tolower(t[1]))) {
                    case CH16('n', 'c'):
                    case CH16('g', 'c'): oaddr = &DRVINFO->defgc; cf = TRUE; break;
                    case CH16('t', 'c'): oaddr = &DRVINFO->deftc; cf = TRUE; break;
                    case CH16('g', 'w'): oaddr = &DRVINFO->defgw; break;
                    case CH16('t', 'w'): oaddr = &DRVINFO->deftw; break;
                    case CH16('g', 'h'): oaddr = &DRVINFO->defgh; break;
                    case CH16('t', 'h'): oaddr = &DRVINFO->defth; break;
                }
                if (oaddr) {
                    long optval;
                    p = nxtoken(p, t);
                    if (sscanf(t, "%ld", &optval) > 0) {
                        if (cf) {
                            switch (toupper(t[strlen(t) - 1])) {
                                case 'K': optval <<= 10; break;
                                case 'M': optval <<= 20; break;
                            }
                            *((GrColor *)oaddr) = optval;
                            continue;
                        }
                        *((int *)oaddr) = (int)optval;
                    }
                    continue;
                }
            }
            strcpy(name, t);
        }
        for (p = name; (p = strchr(p,':')) != NULL; p++) {
            if (p[1] == ':') {
                strcpy(options, &p[2]);
                *p = '\0';
                break;
            }
        }
        if (name[0] != '\0') {
            int ii = 0,found = FALSE;
            while (!found && ((drv = _GrVideoDriverTable[ii++]) != NULL)) {
                char *n = name;
                for (p = drv->name; ; p++,n++) {
                    if (tolower(*p) != tolower(*n)) break;
                    if (*p == '\0') { found = TRUE; break; }
                }
            }
            if (!found) return(FALSE);
        }
    }
    if (!drv) {
        GrVideoDriver *dp;
        int ii, maxmodes = 0;
        for (ii = 0; (dp = _GrVideoDriverTable[ii]) != NULL; ii++) {
            if (dp->detect && (*dp->detect)()) {
                int nm = 0;
                for ( ; dp; dp = dp->inherit) nm += dp->nmodes;
                if (nm > maxmodes) {
                    drv = _GrVideoDriverTable[ii];
                    maxmodes = nm;
                }
            }
        }
        if(!drv) return(FALSE);
    }
    _GrCloseVideoDriver();
    if (firsttime) {
        atexit(_GrCloseVideoDriver);
        firsttime = FALSE;
    }
    if (drvopt) strncat(options, drvopt, NCB-strlen(options));
    if (!drv->init || drv->init(options)) {
        DRVINFO->vdriver = drv;
        return(TRUE);
    }
    return(FALSE);
}

void _GrCloseVideoDriver(void)
{
    sttcopy(&DRVINFO->fdriver,&DRVINFO->tdriver);
    if(DRVINFO->vdriver != NULL) {
        if(DRVINFO->vdriver->reset) (*DRVINFO->vdriver->reset)();
        DRVINFO->vdriver = NULL;
    }
}

void GrSetMinWindowDims(int width, int height)
{
    DRVINFO->minwgw = width;
    DRVINFO->minwgh = height;
}
