/**
 ** ndrvr16.h ---- the new 16bpp linear in-memory and
 **                video frame buffer drivers
 **
 ** Copyright (c) 2023 Mariano Alvarez Fernandez
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
 ** This driver doesn't use the memfill, mempeek infraestructure, only
 ** C standard bit operations and C standard functions.
 **
 **/

/* frame offset address calculation */
#define FOFS(x,y,lo) umuladd32((y),(lo),((x)<<1))

static INLINE
GrColor readpixel(GrFrame *c, int x, int y)
{
    GR_int16u *ptr;

    GRX_ENTER();
    ptr  = (GR_int16u *)&c->gf_baseaddr[0][FOFS(x,y,c->gf_lineoffset)];
    GRX_RETURN((GrColor)*ptr);
}

static INLINE
void drawpixel(int x, int y, GrColor color)
{
    GR_int16u *ptr, xcolor;
    int op;

    GRX_ENTER();
    ptr = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    op = C_OPER(color);
    xcolor = color & 0xFFFF;
    switch(op) {
        case C_XOR: *ptr ^= xcolor; break;
        case C_OR:  *ptr |= xcolor; break;
        case C_AND: *ptr &= xcolor; break;
        default:    *ptr = xcolor; break;
    }
    GRX_LEAVE();
}

static void drawvline(int x, int y, int h, GrColor color)
{
    unsigned lwdt;
    GR_int16u *ptr, xcolor;
    int op, i;

    GRX_ENTER();
    lwdt = (CURC->gc_lineoffset) >> 1;
    ptr = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    op = C_OPER(color);
    xcolor = color & 0xFFFF;
    switch(op) {
        case C_XOR: for (i=0; i<h; i++) {*ptr ^= xcolor; ptr += lwdt;} break;
        case C_OR:  for (i=0; i<h; i++) {*ptr |= xcolor; ptr += lwdt;} break;
        case C_AND: for (i=0; i<h; i++) {*ptr &= xcolor; ptr += lwdt;} break;
        default:    for (i=0; i<h; i++) {*ptr = xcolor; ptr += lwdt;} break;
    }
    GRX_LEAVE();
}

#ifdef GR_int64
static void drawhline(int x, int y, int w, GrColor color)
{
    GR_int16u *ptr16, xcolor;
    GR_int64u *ptr64, repcolor;
    int op, i, w4, wr;

    GRX_ENTER();
    ptr64 = (GR_int64u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    op = C_OPER(color);
    xcolor = color & 0xFFFF;
    repcolor = ((GR_int64u)xcolor << 48) | ((GR_int64u)xcolor << 32) |
               ((GR_int64u)xcolor << 16) | xcolor;
    w4 = w >> 2;
    wr = w & 3;
    switch(op) {
        case C_XOR: for (i=0; i<w4; i++) {*ptr64 ^= repcolor; ptr64++;} break;
        case C_OR:  for (i=0; i<w4; i++) {*ptr64 |= repcolor; ptr64++;} break;
        case C_AND: for (i=0; i<w4; i++) {*ptr64 &= repcolor; ptr64++;} break;
        default:    for (i=0; i<w4; i++) {*ptr64 = repcolor; ptr64++;} break;
    }
    if (wr) {
        ptr16 = (GR_int16u *)ptr64;
        switch(op) {
            case C_XOR: for (i=0; i<wr; i++) {*ptr16 ^= xcolor; ptr16++;} break;
            case C_OR:  for (i=0; i<wr; i++) {*ptr16 |= xcolor; ptr16++;} break;
            case C_AND: for (i=0; i<wr; i++) {*ptr16 &= xcolor; ptr16++;} break;
            default:    for (i=0; i<wr; i++) {*ptr16 = xcolor; ptr16++;} break;
        }
    }
    GRX_LEAVE();
}

static void drawblock(int x, int y, int w, int h, GrColor color)
{
    GR_int16u *ptr16, xcolor;
    GR_int64u *ptr64, repcolor;
    char *ptr8;
    int op, i, j, w4, wr;

    GRX_ENTER();
    ptr8 = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    op = C_OPER(color);
    xcolor = color & 0xFFFF;
    repcolor = ((GR_int64u)xcolor << 48) | ((GR_int64u)xcolor << 32) |
               ((GR_int64u)xcolor << 16) | xcolor;
    w4 = w >> 2;
    wr = w & 3;
    for (j=0; j<h; j++) {
        ptr64 = (GR_int64u *)ptr8;
        switch(op) {
            case C_XOR: for (i=0; i<w4; i++) {*ptr64 ^= repcolor; ptr64++;} break;
            case C_OR:  for (i=0; i<w4; i++) {*ptr64 |= repcolor; ptr64++;} break;
            case C_AND: for (i=0; i<w4; i++) {*ptr64 &= repcolor; ptr64++;} break;
            default:    for (i=0; i<w4; i++) {*ptr64 = repcolor; ptr64++;} break;
        }
        if (wr) {
            ptr16 = (GR_int16u *)ptr64;
            switch(op) {
                case C_XOR: for (i=0; i<wr; i++) {*ptr16 ^= xcolor; ptr16++;} break;
                case C_OR:  for (i=0; i<wr; i++) {*ptr16 |= xcolor; ptr16++;} break;
                case C_AND: for (i=0; i<wr; i++) {*ptr16 &= xcolor; ptr16++;} break;
                default:    for (i=0; i<wr; i++) {*ptr16 = xcolor; ptr16++;} break;
            }
        }
        ptr8 += CURC->gc_lineoffset;
    }
    GRX_LEAVE();
}
#else
static void drawhline(int x, int y, int w, GrColor color)
{
    GR_int16u *ptr16, xcolor;
    GR_int32u *ptr32, repcolor;
    int op, i, w2, wr;

    GRX_ENTER();
    ptr32 = (GR_int32u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    op = C_OPER(color);
    xcolor = color & 0xFFFF;
    repcolor = ((GR_int32u)xcolor << 16) | xcolor;
    w2 = w >> 1;
    wr = w & 1;
    switch(op) {
        case C_XOR: for (i=0; i<w2; i++) {*ptr32 ^= repcolor; ptr32++;} break;
        case C_OR:  for (i=0; i<w2; i++) {*ptr32 |= repcolor; ptr32++;} break;
        case C_AND: for (i=0; i<w2; i++) {*ptr32 &= repcolor; ptr32++;} break;
        default:    for (i=0; i<w2; i++) {*ptr32 = repcolor; ptr32++;} break;
    }
    if (wr) {
        ptr16 = (GR_int16u *)ptr32;
        switch(op) {
            case C_XOR: *ptr16 ^= xcolor; break;
            case C_OR:  *ptr16 |= xcolor; break;
            case C_AND: *ptr16 &= xcolor; break;
            default:    *ptr16 = xcolor; break;
        }
    }
    GRX_LEAVE();
}

static void drawblock(int x, int y, int w, int h, GrColor color)
{
    GR_int16u *ptr16, xcolor;
    GR_int32u *ptr32, repcolor;
    char *ptr8;
    int op, i, j, w2, wr;

    GRX_ENTER();
    ptr8 = &CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    op = C_OPER(color);
    xcolor = color & 0xFFFF;
    repcolor = ((GR_int32u)xcolor << 16) | xcolor;
    w2 = w >> 1;
    wr = w & 1;
    for (j=0; j<h; j++) {
        ptr32 = (GR_int32u *)ptr8;
        switch(op) {
            case C_XOR: for (i=0; i<w2; i++) {*ptr32 ^= repcolor; ptr32++;} break;
            case C_OR:  for (i=0; i<w2; i++) {*ptr32 |= repcolor; ptr32++;} break;
            case C_AND: for (i=0; i<w2; i++) {*ptr32 &= repcolor; ptr32++;} break;
            default:    for (i=0; i<w2; i++) {*ptr32 = repcolor; ptr32++;} break;
        }
        if (wr) {
            ptr16 = (GR_int16u *)ptr32;
            switch(op) {
                case C_XOR: *ptr16 ^= xcolor; break;
                case C_OR:  *ptr16 |= xcolor; break;
                case C_AND: *ptr16 &= xcolor; break;
                default:    *ptr16 = xcolor; break;
            }
        }
        ptr8 += CURC->gc_lineoffset;
    }
    GRX_LEAVE();
}
#endif

static void drawline(int x, int y, int dx, int dy, GrColor color)
{
    GR_int16u *ptr, xcolor;
    int cnt, err, yoff, op;
    
    GRX_ENTER();
    op = C_OPER(color);
    xcolor = color & 0xFFFF;

    yoff = 1;
    if (dx < 0) {
        x += dx; dx = (-dx);
        y += dy; dy = (-dy);
    }
    if (dy < 0) {
        yoff = (-1);
        dy   = (-dy);
    }
    if (dx > dy) {
        err = (cnt = dx) >> 1;
        do {
            ptr = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
            switch(op) {
                case C_XOR: *ptr ^= xcolor; break;
                case C_OR:  *ptr |= xcolor; break;
                case C_AND: *ptr &= xcolor; break;
                default:    *ptr = xcolor; break;
            }
            if ((err -= dy) < 0) err += dx,y += yoff;
            x++;
        } while(--cnt >= 0);
    }
    else {
        err = (cnt = dy) >> 1;
        do {
            ptr = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
            switch(op) {
                case C_XOR: *ptr ^= xcolor; break;
                case C_OR:  *ptr |= xcolor; break;
                case C_AND: *ptr &= xcolor; break;
                default:    *ptr = xcolor; break;
            }
            if ((err -= dx) < 0) err += dy,x++;
            y += yoff;
        } while(--cnt >= 0);
    }
    GRX_LEAVE();
}

static void drawbitmap(int x, int y, int w, int h, char *bmp, int pitch,
                       int start, GrColor fg, GrColor bg)
{
    GR_int16u *ptr, xcolorfg, xcolorbg;
    int opfg, opbg;
    unsigned char *bitp;
    unsigned char bits;
    unsigned char mask;
    int xx;

    GRX_ENTER();
    opfg = C_OPER(fg);
    xcolorfg = fg & 0xFFFF;
    if (fg == GrNOCOLOR) opfg = -1; // special value to do nothing in switch
    opbg = C_OPER(bg);
    xcolorbg = bg & 0xFFFF;
    if (bg == GrNOCOLOR) opbg = -1; // special value to do nothing in switch

    h += y;
    bmp += (unsigned int)start >> 3;
    start &= 7;
    while (1) {
        ptr = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
        bitp = (unsigned char *)bmp;
        bits = *bitp;
        mask = 0x80 >> start;
        xx = 0;
        while (1) {
            if (bits & mask) {
                switch(opfg) {
                    case C_XOR:   *ptr ^= xcolorfg; break;
                    case C_OR:    *ptr |= xcolorfg; break;
                    case C_AND:   *ptr &= xcolorfg; break;
                    case C_WRITE: *ptr = xcolorfg; break;
                }
            } else {
                switch(opbg) {
                    case C_XOR:   *ptr ^= xcolorbg; break;
                    case C_OR:    *ptr |= xcolorbg; break;
                    case C_AND:   *ptr &= xcolorbg; break;
                    case C_WRITE: *ptr = xcolorbg; break;
                }
            }
            if (++xx == w) break;
            if ((mask >>= 1) == 0) bits = *++bitp, mask = 0x80;
            ptr++;
        }
        if (++y == h) break;
        bmp += pitch;
    }
    GRX_LEAVE();
}

static void drawpattern(int x, int y, int w, char patt, GrColor fg, GrColor bg)
{
    //no mejorada por no tener caso de uso
    GR_int8u mask;

    GRX_ENTER();
    mask = 0x80;
    w += x;
    do {
        drawpixel(x, y, (patt & mask) ? fg : bg);
        if((mask >>= 1) == 0) mask = 0x80;
    } while (++x != w);
    GRX_LEAVE();
}

static void bitbltovl(GrFrame *dst, int dx, int dy,
                      GrFrame *src, int sx, int sy,
                      int w, int h, GrColor op)
{
    GR_int16u *sptr, sbuf[w], *dptr, skipc;
    int op2, i, j;
    int sypos, dypos, incr;

    GRX_ENTER();
    op2 = C_OPER(op);
    skipc = op & 0xFFFF;

    // can overlap so check for blit direction
    if(sy <= dy) {
        sypos = sy + h - 1;
        dypos = dy + h - 1;
        incr = -1;
    } else {
        sypos = sy;
        dypos = dy;
        incr = 1;
    }
    
    for (i=0; i<h; i++) {
        sptr = (GR_int16u *)&src->gf_baseaddr[0][FOFS(sx,sypos,src->gf_lineoffset)];
        memcpy((void *)sbuf, (void *)sptr, sizeof(GR_int16u)*w);
        dptr = (GR_int16u *)&dst->gf_baseaddr[0][FOFS(dx,dypos,dst->gf_lineoffset)];
        switch(op2) {
            case C_XOR:
                for (j=0; j<w; j++) {*dptr ^= sbuf[j]; dptr++;}
                break;
            case C_OR:
                for (j=0; j<w; j++) {*dptr |= sbuf[j]; dptr++;}
                break;
            case C_AND:
                for (j=0; j<w; j++) {*dptr &= sbuf[j]; dptr++;}
                break;
            case C_IMAGE:
                for (j=0; j<w; j++) {
                    if (sbuf[j] != skipc)
                        *dptr =sbuf[j];
                    dptr++;
                }
                break;
            default:
                memcpy((void *)dptr, (void *)sbuf, sizeof(GR_int16u)*w); // C_WRITE
                break;
        }
        sypos += incr;
        dypos += incr;
    }
    GRX_LEAVE();
}

static void bitbltnoo(GrFrame *dst, int dx, int dy,
                      GrFrame *src, int sx, int sy,
                      int w, int h, GrColor op)
{
    GR_int16u *sptr, *dptr, skipc;
    int op2, i, j;

    GRX_ENTER();
    op2 = C_OPER(op);
    skipc = op & 0xFFFF;

    for (i=0; i<h; i++) {
        sptr = (GR_int16u *)&src->gf_baseaddr[0][FOFS(sx,sy+i,src->gf_lineoffset)];
        dptr = (GR_int16u *)&dst->gf_baseaddr[0][FOFS(dx,dy+i,dst->gf_lineoffset)];
        switch(op2) {
            case C_XOR:
                for (j=0; j<w; j++) {*dptr ^= *sptr; sptr++; dptr++;}
                break;
            case C_OR:
                for (j=0; j<w; j++) {*dptr |= *sptr; sptr++; dptr++;}
                break;
            case C_AND:
                for (j=0; j<w; j++) {*dptr &= *sptr; sptr++; dptr++;}
                break;
            case C_IMAGE:
                for (j=0; j<w; j++) {
                    if (*sptr != skipc)
                        *dptr = *sptr;
                    sptr++;
                    dptr++;
                }
                break;
            default:
                memcpy((void *)dptr, (void *)sptr, sizeof(GR_int16u)*w);
                break;
        }
    }
    GRX_LEAVE();
}

static
GrColor *getscanline(GrFrame *c, int x, int y, int w)
{
    GR_int16u *ptr;
    GrColor *pixels;
    int i;

    GRX_ENTER();
    pixels = _GrTempBufferAlloc(sizeof(GrColor) * (w+1));
    ptr = (GR_int16u *)&c->gf_baseaddr[0][FOFS(x,y,c->gf_lineoffset)];
    if (pixels) {
        for (i=0; i<w; i++) {
            pixels[i] = *ptr;
            ptr++;
        }
    }
    GRX_RETURN(pixels);
}

static
void putscanline(int x, int y, int w, const GrColor *scl, GrColor op)
{
    GR_int16u *ptr;
    GrColor skipc;
    int op2, i;

    GRX_ENTER();
    op2 = C_OPER(op);
    skipc = op & GrCVALUEMASK;
    ptr = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];

    switch(op2) {
        case C_XOR:
            for (i=0; i<w; i++) {*ptr ^= scl[i] & 0xFFFF; ptr++;}
            break;
        case C_OR:
            for (i=0; i<w; i++) {*ptr |= scl[i] & 0xFFFF; ptr++;}
            break;
        case C_AND:
            for (i=0; i<w; i++) {*ptr &= scl[i] & 0xFFFF; ptr++;}
            break;
        case C_IMAGE:
            for (i=0; i<w; i++) {
                if (scl[i] != skipc)
                    *ptr = scl[i] & 0xFFFF;
                ptr++;
            }
            break;
        default:
            for (i=0; i<w; i++) {*ptr = scl[i] & 0xFFFF; ptr++;}
            break;
    }
    GRX_LEAVE();
}
