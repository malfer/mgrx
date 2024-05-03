/**
 ** ndrvr24.h ---- the new 24bpp linear in-memory and
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
#define MULT3(x)     ( (x)+(x)+(x) )
#define FOFS(x,y,lo) umuladd32((y),(lo),MULT3(x))

#if BYTE_ORDER==LITTLE_ENDIAN
//#define PUT24PTO32P(PTR32,PTR16,PTR8) {*PTR32 = *PTR8; *PTR32 = (*PTR32 << 16) | *PTR16);}
//#define PUT32PTO24P(PTR32,PTR16,PTR8) {*PTR16 = *PTR32; *PTR8 = *PTR32 >> 16;}
#define PUT24PTO32V(VAL32,PTR16,PTR8) {VAL32 = *PTR8; VAL32 = (VAL32 << 16) | *PTR16;}
//#define PUT32VTO24P(VAL32,PTR16,PTR8) {*PTR16 = VAL32; *PTR8 = VAL32 >> 16;}
#define PUT32VTO24V(VAL32,VAL16,VAL8) {VAL16 = VAL32; VAL8 = VAL32 >> 16;}
#else
//#define PUT24PTO32P(PTR32,PTR16,PTR8) {*PTR32 = *PTR16; *PTR32 = (*PTR32 << 8) | *PTR8);}
//#define PUT32PTO24P(PTR32,PTR16,PTR8) {*PTR16 = *PTR32 >> 8; *PTR8 = *PTR32;}
#define PUT24PTO32V(VAL32,PTR16,PTR8) {VAL32 = *PTR16; VAL32 = (VAL32 << 8) | *PTR8;}
//#define PUT32VTO24P(VAL32,PTR16,PTR8) {*PTR16 = VAL32 >> 8; *PTR8 = VAL32;}
#define PUT32VTO24V(VAL32,VAL16,VAL8) {VAL16 = VAL32 >> 8; VAL8 = VAL32;}
#endif

static INLINE
GrColor readpixel(GrFrame *c, int x, int y)
{
    GR_int16u *ptr16;
    GR_int8u *ptr8;
    GrColor color;

    GRX_ENTER();
    ptr16  = (GR_int16u *)&c->gf_baseaddr[0][FOFS(x,y,c->gf_lineoffset)];
    ptr8 = (GR_int8u *)(ptr16 + 1);
    PUT24PTO32V(color,ptr16,ptr8);
    GRX_RETURN(color);
}

static INLINE
void drawpixel(int x, int y, GrColor color)
{
    GR_int16u *ptr16, color16;
    GR_int8u *ptr8, color8;
    int op;

    GRX_ENTER();
    ptr16 = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    ptr8 = (GR_int8u *)(ptr16 + 1);
    PUT32VTO24V(color,color16,color8);
    op = C_OPER(color);
    switch(op) {
        case C_XOR: *ptr16 ^= color16; *ptr8 ^= color8; break;
        case C_OR:  *ptr16 |= color16; *ptr8 |= color8; break;
        case C_AND: *ptr16 &= color16; *ptr8 &= color8; break;
        default:    *ptr16 = color16; *ptr8 = color8; break;
    }
    GRX_LEAVE();
}

static void drawvline(int x, int y, int h, GrColor color)
{
    unsigned lwdt16, lwdt8;
    GR_int16u *ptr16, color16;
    GR_int8u *ptr8, color8;
    int op, i;

    GRX_ENTER();
    lwdt8 = CURC->gc_lineoffset;
    lwdt16 = lwdt8 >> 1;
    ptr16 = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    ptr8 = (GR_int8u *)(ptr16 + 1);
    PUT32VTO24V(color,color16,color8);
    op = C_OPER(color);
    switch(op) {
        case C_XOR: for (i=0; i<h; i++) {
                        *ptr16 ^= color16; *ptr8 ^= color8;
                        ptr16 += lwdt16; ptr8 += lwdt8;
                    }
                    break;
        case C_OR:  for (i=0; i<h; i++) {
                        *ptr16 |= color16; *ptr8 |= color8;
                        ptr16 += lwdt16; ptr8 += lwdt8;
                    }
                    break;
        case C_AND: for (i=0; i<h; i++) {
                        *ptr16 &= color16; *ptr8 &= color8;
                        ptr16 += lwdt16; ptr8 += lwdt8;
                    }
                    break;
        default:    for (i=0; i<h; i++) {
                        *ptr16 = color16; *ptr8 = color8;
                        ptr16 += lwdt16; ptr8 += lwdt8;
                    }
                    break;
    }
    GRX_LEAVE();
}

static void drawhline(int x, int y, int w, GrColor color)
{
    GR_int32u *ptr32, color3[4] = {0};
    GR_int16u *ptr16, color16;
    GR_int8u *ptr8, color8;
    int op, i, k, w3;

    GRX_ENTER();
    PUT32VTO24V(color,color16,color8);
    if (w > 3) {
        w3 = w >> 2; // div 4
        w = w - (w3 << 2);
        ptr8 = (GR_int8u *)&color3[0];
        for (i=0; i<4; i++) {
            ptr16 = (GR_int16u *)ptr8;
            *ptr16 = color16;
            ptr8 += 2;
            *ptr8 = color8;
            ptr8++;
        }
    } else {
        w3 = 0;
    }

    ptr8 = (GR_int8u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    op = C_OPER(color);

    if (w3 > 0) {
        ptr32 = (GR_int32u *)ptr8;
        switch(op) {
            case C_XOR: for (i=0; i<w3; i++) {
                            *ptr32++ ^= color3[0];
                            *ptr32++ ^= color3[1];
                            *ptr32++ ^= color3[2];
                        }
                        break;
            case C_OR:  for (i=0; i<w3; i++) {
                            *ptr32++ |= color3[0];
                            *ptr32++ |= color3[1];
                            *ptr32++ |= color3[2];
                        }
                        break;
            case C_AND: for (i=0; i<w3; i++) {
                            *ptr32++ &= color3[0];
                            *ptr32++ &= color3[1];
                            *ptr32++ &= color3[2];
                        }
                        break;
            default:    for (i=0; i<w3; i++) {
                            *ptr32++ = color3[0];
                            *ptr32++ = color3[1];
                            *ptr32++ = color3[2];
                        }
                        break;
        }
        if (w == 0) goto done;
        ptr8 = (GR_int8u *)ptr32;
    }

    ptr16 = (GR_int16u *)ptr8;
    ptr8 += 2;

    for (k=0; k<w; k++) {
        switch(op) {
            case C_XOR: *ptr16 ^= color16; *ptr8 ^= color8; break;
            case C_OR:  *ptr16 |= color16; *ptr8 |= color8; break;
            case C_AND: *ptr16 &= color16; *ptr8 &= color8; break;
            default:    *ptr16 = color16; *ptr8 = color8; break;
        }
        ptr8++;
        ptr16 = (GR_int16u *)ptr8;
        ptr8 += 2;
    }

done:
    GRX_LEAVE();
}

static void drawblock(int x, int y, int w, int h, GrColor color)
{
    GR_int32u *ptr32, color3[3] = {0};
    GR_int16u *ptr16, color16;
    GR_int8u *ptr8, *ptr0, color8;
    int op, i, j, k, w3;

    GRX_ENTER();
    PUT32VTO24V(color,color16,color8);
    if (w > 3) {
        w3 = w >> 2; // div 4
        w = w - (w3 << 2);
        ptr8 = (GR_int8u *)&color3[0];
        for (i=0; i<4; i++) {
            ptr16 = (GR_int16u *)ptr8;
            *ptr16 = color16;
            ptr8 += 2;
            *ptr8 = color8;
            ptr8++;
        }
    } else {
        w3 = 0;
    }

    ptr0 = (GR_int8u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    op = C_OPER(color);

    for (j=0; j<h; j++) {
        ptr8 = ptr0;
        if (w3 > 0) {
            ptr32 = (GR_int32u *)ptr8;
            switch(op) {
                case C_XOR: for (i=0; i<w3; i++) {
                                *ptr32++ ^= color3[0];
                                *ptr32++ ^= color3[1];
                                *ptr32++ ^= color3[2];
                            }
                            break;
                case C_OR:  for (i=0; i<w3; i++) {
                                *ptr32++ |= color3[0];
                                *ptr32++ |= color3[1];
                                *ptr32++ |= color3[2];
                            }
                            break;
                case C_AND: for (i=0; i<w3; i++) {
                                *ptr32++ &= color3[0];
                                *ptr32++ &= color3[1];
                                *ptr32++ &= color3[2];
                            }
                            break;
                default:    for (i=0; i<w3; i++) {
                                *ptr32++ = color3[0];
                                *ptr32++ = color3[1];
                                *ptr32++ = color3[2];
                            }
                            break;
            }
            ptr8 = (GR_int8u *)ptr32;
        }
        if (w > 0) {
            ptr16 = (GR_int16u *)ptr8;
            ptr8 += 2;
            for (k=0; k<w; k++) {
                switch(op) {
                    case C_XOR: *ptr16 ^= color16; *ptr8 ^= color8; break;
                    case C_OR:  *ptr16 |= color16; *ptr8 |= color8; break;
                    case C_AND: *ptr16 &= color16; *ptr8 &= color8; break;
                    default:    *ptr16 = color16; *ptr8 = color8; break;
                }
                ptr8++;
                ptr16 = (GR_int16u *)ptr8;
                ptr8 += 2;
            }
        }
        ptr0 += CURC->gc_lineoffset;
    }
    GRX_LEAVE();
}

static void drawline(int x, int y, int dx, int dy, GrColor color)
{
    GR_int16u *ptr16, color16;
    GR_int8u *ptr8, color8;
    int cnt, err, yoff, op;
    
    GRX_ENTER();
    op = C_OPER(color);
    PUT32VTO24V(color,color16,color8);

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
            ptr16 = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
            ptr8 = (GR_int8u *)(ptr16 + 1);
            switch(op) {
                case C_XOR: *ptr16 ^= color16; *ptr8 ^= color8; break;
                case C_OR:  *ptr16 |= color16; *ptr8 |= color8; break;
                case C_AND: *ptr16 &= color16; *ptr8 &= color8; break;
                default:    *ptr16 = color16; *ptr8 = color8; break;
            }
            if ((err -= dy) < 0) err += dx,y += yoff;
            x++;
        } while(--cnt >= 0);
    }
    else {
        err = (cnt = dy) >> 1;
        do {
            ptr16 = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
            ptr8 = (GR_int8u *)(ptr16 + 1);
            switch(op) {
                case C_XOR: *ptr16 ^= color16; *ptr8 ^= color8; break;
                case C_OR:  *ptr16 |= color16; *ptr8 |= color8; break;
                case C_AND: *ptr16 &= color16; *ptr8 &= color8; break;
                default:    *ptr16 = color16; *ptr8 = color8; break;
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
    GR_int16u *ptr16, colorfg16, colorbg16;
    GR_int8u *ptr8, colorfg8, colorbg8;
    unsigned char *bitp;
    unsigned char bits;
    unsigned char mask;
    int xx, opfg, opbg;

    GRX_ENTER();
    PUT32VTO24V(fg,colorfg16,colorfg8);
    opfg = C_OPER(fg);
    if (fg == GrNOCOLOR) opfg = -1; // special value to do nothing in switch
    PUT32VTO24V(bg,colorbg16,colorbg8);
    opbg = C_OPER(bg);
    if (bg == GrNOCOLOR) opbg = -1; // special value to do nothing in switch

    h += y;
    bmp += (unsigned int)start >> 3;
    start &= 7;
    while (1) {
        ptr16 = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
        ptr8 = (GR_int8u *)(ptr16 + 1);
        bitp = (unsigned char *)bmp;
        bits = *bitp;
        mask = 0x80 >> start;
        xx = 0;
        while (1) {
            if (bits & mask) {
                switch(opfg) {
                    case C_XOR:   *ptr16 ^= colorfg16; *ptr8 ^= colorfg8; break;
                    case C_OR:    *ptr16 |= colorfg16; *ptr8 |= colorfg8; break;
                    case C_AND:   *ptr16 &= colorfg16; *ptr8 &= colorfg8; break;
                    case C_WRITE: *ptr16 = colorfg16; *ptr8 = colorfg8; break;
                }
            } else {
                switch(opbg) {
                    case C_XOR:   *ptr16 ^= colorbg16; *ptr8 ^= colorbg8; break;
                    case C_OR:    *ptr16 |= colorbg16; *ptr8 |= colorbg8; break;
                    case C_AND:   *ptr16 &= colorbg16; *ptr8 &= colorbg8; break;
                    case C_WRITE: *ptr16 = colorbg16; *ptr8 = colorbg8; break;
                }
            }
            if (++xx == w) break;
            if ((mask >>= 1) == 0) bits = *++bitp, mask = 0x80;
            ptr8++;
            ptr16 = (GR_int16u *)ptr8;
            ptr8 += 2;
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
    GR_int8u sbuf[w*3], *sptr, *dptr, *sbuf8, *dptr8;
    GR_int32u *sbuf32, *dptr32;
    GR_int32u skipc, imcolor;
    int op2, i, j, w32, w8;
    int sypos, dypos, incr;

    GRX_ENTER();
    op2 = C_OPER(op);
    skipc = op & GrCVALUEMASK;

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

    w32 = (w * 3) >> 2; // div 4
    w8 = (w * 3) - (w32 * 4);
    for (i=0; i<h; i++) {
        sptr = (GR_int8u *)&src->gf_baseaddr[0][FOFS(sx,sypos,src->gf_lineoffset)];
        memcpy((void *)sbuf, (void *)sptr, w*3);
        sbuf32 = (GR_int32u *)sbuf;
        dptr = (GR_int8u *)&dst->gf_baseaddr[0][FOFS(dx,dypos,dst->gf_lineoffset)];
        dptr32 = (GR_int32u *)dptr;
        switch(op2) {
            case C_XOR:
                for (j=0; j<w32; j++) {*dptr32 ^= *sbuf32; dptr32++, sbuf32++;}
                sbuf8 = (GR_int8u *)sbuf32;
                dptr8 = (GR_int8u *)dptr32;
                for (j=0; j<w8; j++) {*dptr8 ^= *sbuf8; dptr8++, sbuf8++;}
                break;
            case C_OR:
                for (j=0; j<w32; j++) {*dptr32 |= *sbuf32; dptr32++, sbuf32++;}
                sbuf8 = (GR_int8u *)sbuf32;
                dptr8 = (GR_int8u *)dptr32;
                for (j=0; j<w8; j++) {*dptr8 |= *sbuf8; dptr8++, sbuf8++;}
                break;
            case C_AND:
                for (j=0; j<w32; j++) {*dptr32 &= *sbuf32; dptr32++, sbuf32++;}
                sbuf8 = (GR_int8u *)sbuf32;
                dptr8 = (GR_int8u *)dptr32;
                for (j=0; j<w8; j++) {*dptr8 &= *sbuf8; dptr8++, sbuf8++;}
                break;
            case C_IMAGE:
                sbuf8 = sbuf;
                for (j=0; j<w; j++) {
                    PUT24PTO32V(imcolor,(GR_int16u *)sbuf8,(sbuf8+2));
                    if (imcolor != skipc) {
                        dptr8 = &(dptr[j*3]);
                        *dptr8 = *sbuf8; dptr8++; sbuf8++;
                        *dptr8 = *sbuf8; dptr8++; sbuf8++;
                        *dptr8 = *sbuf8; sbuf8++;
                    } else {
                        sbuf8 += 3;
                    }
                }
                break;
            default:
                memcpy((void *)dptr, (void *)sbuf, w*3); // C_WRITE
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
    GR_int8u *sptr, *dptr, *sptr8, *dptr8;
    GR_int32u *sptr32, *dptr32;
    GR_int32u skipc, imcolor;
    int op2, i, j, w32, w8;

    GRX_ENTER();
    op2 = C_OPER(op);
    skipc = op & GrCVALUEMASK;

    w32 = (w * 3) >> 2; // div 4
    w8 = (w * 3) - (w32 * 4);
    for (i=0; i<h; i++) {
        sptr = (GR_int8u *)&src->gf_baseaddr[0][FOFS(sx,sy+i,src->gf_lineoffset)];
        sptr32 = (GR_int32u *)sptr;
        dptr = (GR_int8u *)&dst->gf_baseaddr[0][FOFS(dx,dy+i,dst->gf_lineoffset)];
        dptr32 = (GR_int32u *)dptr;
        switch(op2) {
            case C_XOR:
                for (j=0; j<w32; j++) {*dptr32 ^= *sptr32; dptr32++, sptr32++;}
                sptr8 = (GR_int8u *)sptr32;
                dptr8 = (GR_int8u *)dptr32;
                for (j=0; j<w8; j++) {*dptr8 ^= *sptr8; dptr8++, sptr8++;}
                break;
            case C_OR:
                for (j=0; j<w32; j++) {*dptr32 |= *sptr32; dptr32++, sptr32++;}
                sptr8 = (GR_int8u *)sptr32;
                dptr8 = (GR_int8u *)dptr32;
                for (j=0; j<w8; j++) {*dptr8 |= *sptr8; dptr8++, sptr8++;}
                break;
            case C_AND:
                for (j=0; j<w32; j++) {*dptr32 &= *sptr32; dptr32++, sptr32++;}
                sptr8 = (GR_int8u *)sptr32;
                dptr8 = (GR_int8u *)dptr32;
                for (j=0; j<w8; j++) {*dptr8 &= *sptr8; dptr8++, sptr8++;}
                break;
            case C_IMAGE:
                sptr8 = sptr;
                for (j=0; j<w; j++) {
                    PUT24PTO32V(imcolor,(GR_int16u *)sptr8,(sptr8+2));
                    if (imcolor != skipc) {
                        dptr8 = &(dptr[w*3]);
                        *dptr8 = *sptr8; dptr8++; sptr8++;
                        *dptr8 = *sptr8; dptr8++; sptr8++;
                        *dptr8 = *sptr8; sptr8++;
                    } else {
                        sptr8 += 3;
                    }
                }
                break;
            default:
                memcpy((void *)dptr, (void *)sptr, w*3);
                break;
        }
    }
    GRX_LEAVE();
}

static GrColor *getscanline(GrFrame *c, int x, int y, int w)
{
    GR_int16u *ptr16;
    GR_int8u *ptr8;
    GrColor *pixels;
    int i;

    GRX_ENTER();
    pixels = _GrTempBufferAlloc(sizeof(GrColor) * (w+1));
    ptr16 = (GR_int16u *)&c->gf_baseaddr[0][FOFS(x,y,c->gf_lineoffset)];
    ptr8 = (GR_int8u *)(ptr16 + 1);
    if (pixels) {
        for (i=0; i<w; i++) {
            PUT24PTO32V(pixels[i],ptr16,ptr8);
            ptr8++;
            ptr16 = (GR_int16u *)ptr8;
            ptr8 += 2;
        }
    }
    GRX_RETURN(pixels);
}

static void putscanline(int x, int y, int w, const GrColor *scl, GrColor op)
{
    GR_int16u *ptr16, color16;
    GR_int8u *ptr8, color8;
    GrColor skipc;
    int op2, i;

    GRX_ENTER();
    op2 = C_OPER(op);
    skipc = op & GrCVALUEMASK;
    ptr16 = (GR_int16u *)&CURC->gc_baseaddr[0][FOFS(x,y,CURC->gc_lineoffset)];
    ptr8 = (GR_int8u *)(ptr16 + 1);

    for (i=0; i<w; i++) {
        PUT32VTO24V(scl[i],color16,color8);
        switch(op2) {
            case C_XOR:
                *ptr16 ^= color16;
                *ptr8 ^= color8;
                break;
            case C_OR:
                *ptr16 |= color16;
                *ptr8 |= color8;
                break;
            case C_AND:
                *ptr16 &= color16;
                *ptr8 &= color8;
                break;
            case C_IMAGE:
                if (scl[i] != skipc) {
                    *ptr16 = color16;
                    *ptr8 = color8;
                }
                break;
            default:
                *ptr16 = color16;
                *ptr8 = color8;
                break;
        }
        ptr8++;
        ptr16 = (GR_int16u *)ptr8;
        ptr8 += 2;
    }
    GRX_LEAVE();
}
