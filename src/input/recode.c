/**
 ** recode.c ---- MGRX recode, some tables borrowed from libiconv
 **
 ** Copyright (C) 2008, 2020, 2022 Mariano Alvarez Fernandez
 ** [e-mail: malfer at telefonica.net]
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "libgrx.h"
#include "mgrxkeys.h"
#include "ninput.h"

static unsigned short cpcom[32] = {
/* 0x00 */
0x003F, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
/* 0x10 */
0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8,
0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC
};

static unsigned short r_cpcom[31][2] = {
{0x00A7, 21}, {0x00B6, 20}, {0x2022,  7}, {0x203C, 19},
{0x2190, 27}, {0x2191, 24}, {0x2192, 26}, {0x2193, 25},
{0x2194, 29}, {0x2195, 18}, {0x21A8, 23}, {0x221F, 28},
{0x25AC, 22}, {0x25B2, 30}, {0x25BA, 16}, {0x25BC, 31},
{0x25C4, 17}, {0x25CB,  9}, {0x25D8,  8}, {0x25D9, 10},
{0x263A,  1}, {0x263B,  2}, {0x263C, 15}, {0x2640, 12},
{0x2642, 11}, {0x2660,  6}, {0x2663,  5}, {0x2665,  3},
{0x2666,  4}, {0x266A, 13}, {0x266B, 14}
};

int _GrRecode_CP437_UCS2(unsigned char src, long *des)
{
  static unsigned short cp437[128] = {
  /* 0x80 */
  0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
  0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
  /* 0x90 */
  0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
  0x00ff, 0x00d6, 0x00dc, 0x00a2, 0x00a3, 0x00a5, 0x20a7, 0x0192,
  /* 0xa0 */
  0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
  0x00bf, 0x2310, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
  /* 0xb0 */
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
  0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
  /* 0xc0 */
  0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
  0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
  /* 0xd0 */
  0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
  0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
  /* 0xe0 */
  0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x00b5, 0x03c4,
  0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229,
  /* 0xf0 */
  0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
  0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0
  };

  if (src < 0x20)
    *des = cpcom[src];
  else if (src < 0x80)
    *des = src;
  else
    *des = cp437[src-0x80];
  return 1;
}

int _GrRecode_CP850_UCS2(unsigned char src, long *des)
{
  static unsigned short cp850[128] = {
  /* 0x80 */
  0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
  0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
  /* 0x90 */
  0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
  0x00ff, 0x00d6, 0x00dc, 0x00f8, 0x00a3, 0x00d8, 0x00d7, 0x0192,
  /* 0xa0 */
  0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
  0x00bf, 0x00ae, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
  /* 0xb0 */
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00c1, 0x00c2, 0x00c0,
  0x00a9, 0x2563, 0x2551, 0x2557, 0x255d, 0x00a2, 0x00a5, 0x2510,
  /* 0xc0 */
  0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x00e3, 0x00c3,
  0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x00a4,
  /* 0xd0 */
  0x00f0, 0x00d0, 0x00ca, 0x00cb, 0x00c8, 0x0131, 0x00cd, 0x00ce,
  0x00cf, 0x2518, 0x250c, 0x2588, 0x2584, 0x00a6, 0x00cc, 0x2580,
  /* 0xe0 */
  0x00d3, 0x00df, 0x00d4, 0x00d2, 0x00f5, 0x00d5, 0x00b5, 0x00fe,
  0x00de, 0x00da, 0x00db, 0x00d9, 0x00fd, 0x00dd, 0x00af, 0x00b4,
  /* 0xf0 */
  0x00ad, 0x00b1, 0x2017, 0x00be, 0x00b6, 0x00a7, 0x00f7, 0x00b8,
  0x00b0, 0x00a8, 0x00b7, 0x00b9, 0x00b3, 0x00b2, 0x25a0, 0x00a0
  };

  if (src < 0x20)
    *des = cpcom[src];
  else if (src < 0x80)
    *des = src;
  else
    *des = cp850[src-0x80];
  return 1;
}

int _GrRecode_CP1251_UCS2(unsigned char src, long *des)
{
  static unsigned short cp1251[128] = {
  /* 0x80 */
  0x0402, 0x0403, 0x201a, 0x0453, 0x201e, 0x2026, 0x2020, 0x2021,
  0x20ac, 0x2030, 0x0409, 0x2039, 0x040a, 0x040c, 0x040b, 0x040f,
  /* 0x90 */
  0x0452, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
  0x0098, 0x2122, 0x0459, 0x203a, 0x045a, 0x045c, 0x045b, 0x045f,
  /* 0xa0 */
  0x00a0, 0x040e, 0x045e, 0x0408, 0x00a4, 0x0490, 0x00a6, 0x00a7,
  0x0401, 0x00a9, 0x0404, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0407,
  /* 0xb0 */
  0x00b0, 0x00b1, 0x0406, 0x0456, 0x0491, 0x00b5, 0x00b6, 0x00b7,
  0x0451, 0x2116, 0x0454, 0x00bb, 0x0458, 0x0405, 0x0455, 0x0457,
  /* 0xc0 */
  0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
  0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
  /* 0xd0 */
  0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
  0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
  /* 0xe0 */
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
  0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
  /* 0xf0 */
  0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
  0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
  };

  if (src < 0x80)
    *des = src;
  else
    *des = cp1251[src-0x80];
  return 1;
}

int _GrRecode_CP1252_UCS2(unsigned char src, long *des)
{
  static unsigned short cp1252[32] = {
  /* 0x80 */
  0x20ac, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
  0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008d, 0x017d, 0x008f,
  /* 0x90 */
  0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
  0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x009d, 0x017e, 0x0178
  };

  if (src < 0x80 || src >= 0xa0)
    *des = src;
  else
    *des = cp1252[src-0x80];
  return 1;
}

int _GrRecode_CP1253_UCS2(unsigned char src, long *des)
{
  static unsigned short cp1253[128] = {
  /* 0x80 */
  0x20ac, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
  0x0088, 0x2030, 0x008a, 0x2039, 0x008c, 0x008d, 0x008e, 0x008f,
  /* 0x90 */
  0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
  0x0098, 0x2122, 0x009a, 0x203a, 0x009c, 0x009d, 0x009e, 0x009f,
  /* 0xa0 */
  0x00a0, 0x0385, 0x0386, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
  0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x2015,
  /* 0xb0 */
  0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x0384, 0x00b5, 0x00b6, 0x00b7,
  0x0388, 0x0389, 0x038a, 0x00bb, 0x038c, 0x00bd, 0x038e, 0x038f,
  /* 0xc0 */
  0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
  0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
  /* 0xd0 */
  0x03a0, 0x03a1, 0x00d2, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7,
  0x03a8, 0x03a9, 0x03aa, 0x03ab, 0x03ac, 0x03ad, 0x03ae, 0x03af,
  /* 0xe0 */
  0x03b0, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7,
  0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
  /* 0xf0 */
  0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7,
  0x03c8, 0x03c9, 0x03ca, 0x03cb, 0x03cc, 0x03cd, 0x03ce, 0x00ff,
  };

  if (src < 0x80)
    *des = src;
  else
    *des = cp1253[src-0x80];
  return 1;
}

int _GrRecode_ISO88591_UCS2(unsigned char src, long *des)
{
  *des = src;
  return 1;
}

int _GrRecode_UTF8_UCS2(const unsigned char *src, long *des)
{
/* When converting any illformed UCP will be changed to 0xFFFD,
 * see Unicode stardard, conformance D92 */
    int nb = 1;

    if (src[0] < 0x80) {  /* ASCII one byte */
        *des = src[0];
        return nb;
    }
    if (src[0] >= 0xC2 && src[0] <= 0xDF) { /* two bytes */
        if ((src[1] & 0xC0) != 0x80) {
            goto ILLFORMED;
        }
        *des = (((unsigned int)(src[0]) & 0x1f) << 6) |
               ((unsigned int)(src[1]) & 0x3f);
        return 2;
    }
    if ((src[0] & 0xf0) == 0xe0) { /* three bytes */
        if (src[0] == 0xE0) {
            if (src[1] < 0xA0 || src[1] > 0xBF) {
                goto ILLFORMED;
            }
        } else if (src[0] == 0xED) {
            if (src[1] < 0x80 || src[1] > 0x9F) {
                goto ILLFORMED;
            }
        } else {
            if ((src[1] & 0xC0) != 0x80) {
            goto ILLFORMED;
            }
        }
        if ((src[2] & 0xC0) != 0x80) {
            nb = 2;
            goto ILLFORMED;
        }
        *des = (((unsigned int)(src[0]) & 0x0f) << 12) |
               (((unsigned int)(src[1]) & 0x3f) << 6) |
               ((unsigned int)(src[2]) & 0x3f);
        return 3;
    }
    if (src[0] >= 0xF0 && src[0] <= 0xF4) { /* four bytes */
    /* NOTE, because 4 bytes are out of UCS2 range even if they are well
     * formed we will return 0xFFFD ever, but with the correct byte count */
        if (src[0] == 0xF0) {
            if (src[1] < 0x90 || src[1] > 0xBF) {
                goto ILLFORMED;
            }
        } else if (src[0] == 0xF4) {
            if (src[1] < 0x80 || src[1] > 0x8F) {
                goto ILLFORMED;
            }
        } else {
            if ((src[1] & 0xC0) != 0x80) {
            goto ILLFORMED;
            }
        }
        if ((src[2] & 0xC0) != 0x80) {
            nb = 2;
            goto ILLFORMED;
        }
        if ((src[3] & 0xC0) != 0x80) {
            nb = 3;
            goto ILLFORMED;
        }
        *des = (unsigned int)0xFFFD;
        return 4;
    }

    // Here for forbiden bytes in UTF-8 C0-C1, F5-FF

ILLFORMED:
    *des = (unsigned int)0xFFFD;
    return nb;
}

int _GrRecode_mgrx512_UCS2(unsigned short src, long *des)
{
  static unsigned short mgrx512s[32] = {
  /* 0x0000 */
  0x003F, 0x2500, 0x2502, 0x2508, 0x250A, 0x250C, 0x2510, 0x2514,
  0x2518, 0x251C, 0x2524, 0x252C, 0x2534, 0x253C, 0x2591, 0x2592,
  /* 0x0010 */
  0x2593, 0x2588, 0x263A, 0x263B, 0x263C, 0x2640, 0x2642, 0x2660,
  0x2663, 0x2665, 0x2666, 0x266A, 0x266B, 0x2714, 0x2718, 0x003F
  };

  static unsigned short mgrx512u[256] = {
  /* 0x0100 */
  0x2190, 0x2191, 0x2192, 0x2193, 0x2194, 0x2195, 0x21B5, 0x21D0,
  0x21D1, 0x21D2, 0x21D3, 0x21D4, 0x21D5, 0x003F, 0x2200, 0x2203,
  /* 0x0110 */
  0x2204, 0x2205, 0x2206, 0x2208, 0x2209, 0x221A, 0x221E, 0x2229,
  0x222A, 0x2248, 0x2260, 0x2261, 0x003F, 0x003F, 0x003F, 0x003F,
  /* 0x0120 */
  0x00A0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407,
  0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x003F, 0x040E, 0x040F,
  /* 0x0130 */
  0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
  0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
  /* 0x0140 */
  0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
  0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
  /* 0x0150 */
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
  0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
  /* 0x0160 */
  0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
  0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
  /* 0x0170 */
  0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457,
  0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x00A7, 0x045E, 0x045F,
  /* 0x0180 */
  0x2320, 0x23AE, 0x2321, 0x239B, 0x239C, 0x239D, 0x239E, 0x239F,
  0x23A0, 0x23A1, 0x23A2, 0x23A3, 0x23A4, 0x23A5, 0x23A6, 0x23A7,
  /* 0x0190 */
  0x23A8, 0x23A9, 0x003F, 0x23AB, 0x23AC, 0x23AD, 0x003F, 0x003F,
  0x003F, 0x003F, 0x003F, 0x003F, 0x003F, 0x003F, 0x003F, 0x003F,
  /* 0x01A0 */
  0x00A0, 0x2018, 0x2019, 0x00A3, 0x20AC, 0x003F, 0x00A6, 0x00A7,
  0x00A8, 0x00A9, 0x003F, 0x00AB, 0x00AC, 0x003F, 0x003F, 0x2015,
  /* 0x01B0 */
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x0385, 0x0386, 0x00B7,
  0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
  /* 0x01C0 */
  0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
  0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
  /* 0x01D0 */
  0x03A0, 0x03A1, 0x003F, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
  0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
  /* 0x01E0 */
  0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
  0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
  /* 0x01F0 */
  0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
  0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x003F
  };

  if (src < 0x20) {
    *des = mgrx512s[src];
    return 1;
  } else if (src < 0x100) {
    return _GrRecode_CP1252_UCS2(src, des);
  } else if (src < 0x200) {
    *des = mgrx512u[src-0x100];
    return 1;
  } else
    return 0;
}

int _GrRecode_ISO88595_UCS2(unsigned char src, long *des)
{
  static unsigned short iso88595[96] = {
  /* 0xa0 */
  0x00A0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407,
  0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x003F, 0x040E, 0x040F,
  /* 0xb0 */
  0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
  0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
  /* 0xc0 */
  0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
  0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
  /* 0xd0 */
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
  0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
  /* 0xe0 */
  0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
  0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
  /* 0xf0 */
  0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457,
  0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x00A7, 0x045E, 0x045F
  };

  if (src < 0xa0) {
    *des = src;
    return 1;
  } else if (src < 0x100) {
    *des = iso88595[src-0xa0];
    return 1;
  } else
    return 0;
}

int _GrRecode_ISO88597_UCS2(unsigned char src, long *des)
{
  static unsigned short iso88597[96] = {
  /* 0xA0 */
  0x00A0, 0x2018, 0x2019, 0x00A3, 0x20AC, 0x003F, 0x00A6, 0x00A7,
  0x00A8, 0x00A9, 0x003F, 0x00AB, 0x00AC, 0x003F, 0x003F, 0x2015,
  /* 0xB0 */
  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x0385, 0x0386, 0x00B7,
  0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
  /* 0xC0 */
  0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
  0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
  /* 0xD0 */
  0x03A0, 0x03A1, 0x003F, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
  0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
  /* 0xE0 */
  0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
  0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
  /* 0xF0 */
  0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
  0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x003F
  };

  if (src < 0xa0) {
    *des = src;
    return 1;
  } else if (src < 0x100) {
    *des = iso88597[src-0xa0];
    return 1;
  } else
    return 0;
}

int _GrRecode_CP437Ext_UCS2(unsigned short src, long *des)
{
  static unsigned short cp437ext[66] = {
  0x00a4, 0x00a6, 0x00a8, 0x00a9, 0x00ad, 0x00ae, 0x00af, 0x00b3, 
  0x00b4, 0x00b8, 0x00b9, 0x00be, 0x00c0, 0x00c1, 0x00c2, 0x00c3, 
  0x00c8, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 0x00d0, 
  0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d7, 0x00d8, 0x00d9, 0x00da, 
  0x00db, 0x00dd, 0x00de, 0x00e3, 0x00f0, 0x00f5, 0x00f8, 0x00fd, 
  0x00fe, 0x20ac, 0x201a, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6,
  0x2030, 0x0160, 0x2039, 0x0152, 0x017d, 0x2018, 0x2019, 0x201c,
  0x201d, 0x2013, 0x2014, 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153,
  0x017e, 0x0178
  };

  if (src < 0x100) {
    return _GrRecode_CP437_UCS2(src, des);;
  } else if (src < 0x142) {
    *des = cp437ext[src-0x100];
    return 1;
  } else
    return 0;
}

static int compara(const void *arg1, const void *arg2)
{
  if (*(unsigned short *)arg1 < *(unsigned short *)arg2) return -1;
  else if (*(unsigned short *)arg1 > *(unsigned short *)arg2) return 1;
  else return 0;
}

int _GrRecode_UCS2_CP437(long src, unsigned char *des)
{
  static unsigned short r_cp437[128][2] = {
  {0x00a0, 255}, {0x00a1, 173}, {0x00a2, 155}, {0x00a3, 156},
  {0x00a5, 157}, {0x00aa, 166}, {0x00ab, 174}, {0x00ac, 170},
  {0x00b0, 248}, {0x00b1, 241}, {0x00b2, 253}, {0x00b5, 230},
  {0x00b7, 250}, {0x00ba, 167}, {0x00bb, 175}, {0x00bc, 172},
  {0x00bd, 171}, {0x00bf, 168}, {0x00c4, 142}, {0x00c5, 143},
  {0x00c6, 146}, {0x00c7, 128}, {0x00c9, 144}, {0x00d1, 165},
  {0x00d6, 153}, {0x00dc, 154}, {0x00df, 225}, {0x00e0, 133},
  {0x00e1, 160}, {0x00e2, 131}, {0x00e4, 132}, {0x00e5, 134},
  {0x00e6, 145}, {0x00e7, 135}, {0x00e8, 138}, {0x00e9, 130},
  {0x00ea, 136}, {0x00eb, 137}, {0x00ec, 141}, {0x00ed, 161},
  {0x00ee, 140}, {0x00ef, 139}, {0x00f1, 164}, {0x00f2, 149},
  {0x00f3, 162}, {0x00f4, 147}, {0x00f6, 148}, {0x00f7, 246},
  {0x00f9, 151}, {0x00fa, 163}, {0x00fb, 150}, {0x00fc, 129},
  {0x00ff, 152}, {0x0192, 159}, {0x0393, 226}, {0x0398, 233},
  {0x03a3, 228}, {0x03a6, 232}, {0x03a9, 234}, {0x03b1, 224},
  {0x03b4, 235}, {0x03b5, 238}, {0x03c0, 227}, {0x03c3, 229},
  {0x03c4, 231}, {0x03c6, 237}, {0x207f, 252}, {0x20a7, 158},
  {0x2219, 249}, {0x221a, 251}, {0x221e, 236}, {0x2229, 239},
  {0x2248, 247}, {0x2261, 240}, {0x2264, 243}, {0x2265, 242},
  {0x2310, 169}, {0x2320, 244}, {0x2321, 245}, {0x2500, 196},
  {0x2502, 179}, {0x250c, 218}, {0x2510, 191}, {0x2514, 192},
  {0x2518, 217}, {0x251c, 195}, {0x2524, 180}, {0x252c, 194},
  {0x2534, 193}, {0x253c, 197}, {0x2550, 205}, {0x2551, 186},
  {0x2552, 213}, {0x2553, 214}, {0x2554, 201}, {0x2555, 184},
  {0x2556, 183}, {0x2557, 187}, {0x2558, 212}, {0x2559, 211},
  {0x255a, 200}, {0x255b, 190}, {0x255c, 189}, {0x255d, 188},
  {0x255e, 198}, {0x255f, 199}, {0x2560, 204}, {0x2561, 181},
  {0x2562, 182}, {0x2563, 185}, {0x2564, 209}, {0x2565, 210},
  {0x2566, 203}, {0x2567, 207}, {0x2568, 208}, {0x2569, 202},
  {0x256a, 216}, {0x256b, 215}, {0x256c, 206}, {0x2580, 223},
  {0x2584, 220}, {0x2588, 219}, {0x258c, 221}, {0x2590, 222},
  {0x2591, 176}, {0x2592, 177}, {0x2593, 178}, {0x25a0, 254}
  };
  unsigned short *ind;

  if (src < 0x80) {
    *des = src;
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_cp437, 128,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_cpcom, 31,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  return 0;
}

int _GrRecode_UCS2_CP850(long src, unsigned char *des)
{
  static unsigned short r_cp850[128][2] = {
  {0x00a0, 255}, {0x00a1, 173}, {0x00a2, 189}, {0x00a3, 156},
  {0x00a4, 207}, {0x00a5, 190}, {0x00a6, 221}, {0x00a7, 245},
  {0x00a8, 249}, {0x00a9, 184}, {0x00aa, 166}, {0x00ab, 174},
  {0x00ac, 170}, {0x00ad, 240}, {0x00ae, 169}, {0x00af, 238},
  {0x00b0, 248}, {0x00b1, 241}, {0x00b2, 253}, {0x00b3, 252},
  {0x00b4, 239}, {0x00b5, 230}, {0x00b6, 244}, {0x00b7, 250},
  {0x00b8, 247}, {0x00b9, 251}, {0x00ba, 167}, {0x00bb, 175},
  {0x00bc, 172}, {0x00bd, 171}, {0x00be, 243}, {0x00bf, 168},
  {0x00c0, 183}, {0x00c1, 181}, {0x00c2, 182}, {0x00c3, 199},
  {0x00c4, 142}, {0x00c5, 143}, {0x00c6, 146}, {0x00c7, 128},
  {0x00c8, 212}, {0x00c9, 144}, {0x00ca, 210}, {0x00cb, 211},
  {0x00cc, 222}, {0x00cd, 214}, {0x00ce, 215}, {0x00cf, 216},
  {0x00d0, 209}, {0x00d1, 165}, {0x00d2, 227}, {0x00d3, 224},
  {0x00d4, 226}, {0x00d5, 229}, {0x00d6, 153}, {0x00d7, 158},
  {0x00d8, 157}, {0x00d9, 235}, {0x00da, 233}, {0x00db, 234},
  {0x00dc, 154}, {0x00dd, 237}, {0x00de, 232}, {0x00df, 225},
  {0x00e0, 133}, {0x00e1, 160}, {0x00e2, 131}, {0x00e3, 198},
  {0x00e4, 132}, {0x00e5, 134}, {0x00e6, 145}, {0x00e7, 135},
  {0x00e8, 138}, {0x00e9, 130}, {0x00ea, 136}, {0x00eb, 137},
  {0x00ec, 141}, {0x00ed, 161}, {0x00ee, 140}, {0x00ef, 139},
  {0x00f0, 208}, {0x00f1, 164}, {0x00f2, 149}, {0x00f3, 162},
  {0x00f4, 147}, {0x00f5, 228}, {0x00f6, 148}, {0x00f7, 246},
  {0x00f8, 155}, {0x00f9, 151}, {0x00fa, 163}, {0x00fb, 150},
  {0x00fc, 129}, {0x00fd, 236}, {0x00fe, 231}, {0x00ff, 152},
  {0x0131, 213}, {0x0192, 159}, {0x2017, 242}, {0x2500, 196},
  {0x2502, 179}, {0x250c, 218}, {0x2510, 191}, {0x2514, 192},
  {0x2518, 217}, {0x251c, 195}, {0x2524, 180}, {0x252c, 194},
  {0x2534, 193}, {0x253c, 197}, {0x2550, 205}, {0x2551, 186},
  {0x2554, 201}, {0x2557, 187}, {0x255a, 200}, {0x255d, 188},
  {0x2560, 204}, {0x2563, 185}, {0x2566, 203}, {0x2569, 202},
  {0x256c, 206}, {0x2580, 223}, {0x2584, 220}, {0x2588, 219},
  {0x2591, 176}, {0x2592, 177}, {0x2593, 178}, {0x25a0, 254}
  };
  unsigned short *ind;

  if (src < 0x80) {
    *des = src;
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_cp850, 128,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_cpcom, 31,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  return 0;
}

int _GrRecode_UCS2_CP1251(long src, unsigned char *des)
{
  static unsigned short r_cp1251[128][2] = {
  {0x0098, 152}, {0x00a0, 160}, {0x00a4, 164}, {0x00a6, 166}, 
  {0x00a7, 167}, {0x00a9, 169}, {0x00ab, 171}, {0x00ac, 172}, 
  {0x00ad, 173}, {0x00ae, 174}, {0x00b0, 176}, {0x00b1, 177}, 
  {0x00b5, 181}, {0x00b6, 182}, {0x00b7, 183}, {0x00bb, 187}, 
  {0x0401, 168}, {0x0402, 128}, {0x0403, 129}, {0x0404, 170}, 
  {0x0405, 189}, {0x0406, 178}, {0x0407, 175}, {0x0408, 163}, 
  {0x0409, 138}, {0x040a, 140}, {0x040b, 142}, {0x040c, 141}, 
  {0x040e, 161}, {0x040f, 143}, {0x0410, 192}, {0x0411, 193}, 
  {0x0412, 194}, {0x0413, 195}, {0x0414, 196}, {0x0415, 197}, 
  {0x0416, 198}, {0x0417, 199}, {0x0418, 200}, {0x0419, 201}, 
  {0x041a, 202}, {0x041b, 203}, {0x041c, 204}, {0x041d, 205}, 
  {0x041e, 206}, {0x041f, 207}, {0x0420, 208}, {0x0421, 209}, 
  {0x0422, 210}, {0x0423, 211}, {0x0424, 212}, {0x0425, 213}, 
  {0x0426, 214}, {0x0427, 215}, {0x0428, 216}, {0x0429, 217}, 
  {0x042a, 218}, {0x042b, 219}, {0x042c, 220}, {0x042d, 221}, 
  {0x042e, 222}, {0x042f, 223}, {0x0430, 224}, {0x0431, 225}, 
  {0x0432, 226}, {0x0433, 227}, {0x0434, 228}, {0x0435, 229}, 
  {0x0436, 230}, {0x0437, 231}, {0x0438, 232}, {0x0439, 233}, 
  {0x043a, 234}, {0x043b, 235}, {0x043c, 236}, {0x043d, 237}, 
  {0x043e, 238}, {0x043f, 239}, {0x0440, 240}, {0x0441, 241}, 
  {0x0442, 242}, {0x0443, 243}, {0x0444, 244}, {0x0445, 245}, 
  {0x0446, 246}, {0x0447, 247}, {0x0448, 248}, {0x0449, 249}, 
  {0x044a, 250}, {0x044b, 251}, {0x044c, 252}, {0x044d, 253}, 
  {0x044e, 254}, {0x044f, 255}, {0x0451, 184}, {0x0452, 144}, 
  {0x0453, 131}, {0x0454, 186}, {0x0455, 190}, {0x0456, 179}, 
  {0x0457, 191}, {0x0458, 188}, {0x0459, 154}, {0x045a, 156}, 
  {0x045b, 158}, {0x045c, 157}, {0x045e, 162}, {0x045f, 159}, 
  {0x0490, 165}, {0x0491, 180}, {0x2013, 150}, {0x2014, 151}, 
  {0x2018, 145}, {0x2019, 146}, {0x201a, 130}, {0x201c, 147}, 
  {0x201d, 148}, {0x201e, 132}, {0x2020, 134}, {0x2021, 135}, 
  {0x2022, 149}, {0x2026, 133}, {0x2030, 137}, {0x2039, 139}, 
  {0x203a, 155}, {0x20ac, 136}, {0x2116, 185}, {0x2122, 153}, 
  };
  unsigned short *ind;

  if (src < 0x80 ) {
    *des = src;
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_cp1251, 128,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  return 0;
}

int _GrRecode_UCS2_CP1252(long src, unsigned char *des)
{
  static unsigned short r_cp1252[32][2] = {
  {0x0081, 129}, {0x008d, 141}, {0x008f, 143}, {0x0090, 144},
  {0x009d, 157}, {0x0152, 140}, {0x0153, 156}, {0x0160, 138},
  {0x0161, 154}, {0x0178, 159}, {0x017d, 142}, {0x017e, 158},
  {0x0192, 131}, {0x02c6, 136}, {0x02dc, 152}, {0x2013, 150},
  {0x2014, 151}, {0x2018, 145}, {0x2019, 146}, {0x201a, 130},
  {0x201c, 147}, {0x201d, 148}, {0x201e, 132}, {0x2020, 134},
  {0x2021, 135}, {0x2022, 149}, {0x2026, 133}, {0x2030, 137},
  {0x2039, 139}, {0x203a, 155}, {0x20ac, 128}, {0x2122, 153}
  };
  unsigned short *ind;

  if (src < 0x80 || (src >=0xa0 && src <0x100)) {
    *des = src;
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_cp1252, 32,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  return 0;
}

int _GrRecode_UCS2_CP1253(long src, unsigned char *des)
{
  static unsigned short r_cp1253[128][2] = {
  {0x0081, 129}, {0x0088, 136}, {0x008a, 138}, {0x008c, 140},
  {0x008d, 141}, {0x008e, 142}, {0x008f, 143}, {0x0090, 144},
  {0x0098, 152}, {0x009a, 154}, {0x009c, 156}, {0x009d, 157},
  {0x009e, 158}, {0x009f, 159}, {0x00a0, 160}, {0x00a3, 163},
  {0x00a4, 164}, {0x00a5, 165}, {0x00a6, 166}, {0x00a7, 167},
  {0x00a8, 168}, {0x00a9, 169}, {0x00aa, 170}, {0x00ab, 171},
  {0x00ac, 172}, {0x00ad, 173}, {0x00ae, 174}, {0x00b0, 176},
  {0x00b1, 177}, {0x00b2, 178}, {0x00b3, 179}, {0x00b5, 181},
  {0x00b6, 182}, {0x00b7, 183}, {0x00bb, 187}, {0x00bd, 189},
  {0x00d2, 210}, {0x00ff, 255}, {0x0192, 131}, {0x0384, 180},
  {0x0385, 161}, {0x0386, 162}, {0x0388, 184}, {0x0389, 185},
  {0x038a, 186}, {0x038c, 188}, {0x038e, 190}, {0x038f, 191},
  {0x0390, 192}, {0x0391, 193}, {0x0392, 194}, {0x0393, 195},
  {0x0394, 196}, {0x0395, 197}, {0x0396, 198}, {0x0397, 199},
  {0x0398, 200}, {0x0399, 201}, {0x039a, 202}, {0x039b, 203},
  {0x039c, 204}, {0x039d, 205}, {0x039e, 206}, {0x039f, 207},
  {0x03a0, 208}, {0x03a1, 209}, {0x03a3, 211}, {0x03a4, 212},
  {0x03a5, 213}, {0x03a6, 214}, {0x03a7, 215}, {0x03a8, 216},
  {0x03a9, 217}, {0x03aa, 218}, {0x03ab, 219}, {0x03ac, 220},
  {0x03ad, 221}, {0x03ae, 222}, {0x03af, 223}, {0x03b0, 224},
  {0x03b1, 225}, {0x03b2, 226}, {0x03b3, 227}, {0x03b4, 228},
  {0x03b5, 229}, {0x03b6, 230}, {0x03b7, 231}, {0x03b8, 232},
  {0x03b9, 233}, {0x03ba, 234}, {0x03bb, 235}, {0x03bc, 236},
  {0x03bd, 237}, {0x03be, 238}, {0x03bf, 239}, {0x03c0, 240},
  {0x03c1, 241}, {0x03c2, 242}, {0x03c3, 243}, {0x03c4, 244},
  {0x03c5, 245}, {0x03c6, 246}, {0x03c7, 247}, {0x03c8, 248},
  {0x03c9, 249}, {0x03ca, 250}, {0x03cb, 251}, {0x03cc, 252},
  {0x03cd, 253}, {0x03ce, 254}, {0x2013, 150}, {0x2014, 151},
  {0x2015, 175}, {0x2018, 145}, {0x2019, 146}, {0x201a, 130},
  {0x201c, 147}, {0x201d, 148}, {0x201e, 132}, {0x2020, 134},
  {0x2021, 135}, {0x2022, 149}, {0x2026, 133}, {0x2030, 137},
  {0x2039, 139}, {0x203a, 155}, {0x20ac, 128}, {0x2122, 153},
  };
  unsigned short *ind;

  if (src < 0x80 ) {
    *des = src;
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_cp1253, 128,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  return 0;
}

int _GrRecode_UCS2_ISO88591(long src, unsigned char *des)
{
  if (src < 0x100) {
    *des = (unsigned char)src;
    return 1;
  }
  return 0;
}

int _GrRecode_UCS2_UTF8(long src, unsigned char *des)
{
    des[0] = des[1] = des[2] = des[3] = 0;
    
    if (src < 0x80) {  /* one byte */
        des[0] = src;
        return 1;
    }
    if (src < 0x800) { /* two bytes */
        des[0] = 0xc0 | (src >> 6);
        des[1] = 0x80 | (src & 0x3f);
        return 2;
    }
    if (src < 0xD800 || (src > 0xDFFF && src < 0x10000)) { /* three bytes */
        des[0] = 0xe0 | (src >> 12);
        des[1] = 0x80 | ((src >> 6) & 0x3f);
        des[2] = 0x80 | (src & 0x3f);
        return 3;
    }
    /* out of BMP range or surrogates returns 0xFFFD */
    des[0] = 0xEF;
    des[1] = 0xBF;
    des[2] = 0xBD;
    return 3;
}

int _GrRecode_UCS2_mgrx512(long src, long *des)
{
  static unsigned short r_mgrx512[273][2] = {
  {0x0152,140}, {0x0153,156}, {0x0160,138}, {0x0161,154}, {0x0178,159}, {0x017D,142}, 
  {0x017E,158}, {0x0192,131}, {0x02C6,136}, {0x02DC,152}, {0x0384,436}, {0x0385,437}, 
  {0x0386,438}, {0x0388,440}, {0x0389,441}, {0x038A,442}, {0x038C,444}, {0x038E,446}, 
  {0x038F,447}, {0x0390,448}, {0x0391,449}, {0x0392,450}, {0x0393,451}, {0x0394,452}, 
  {0x0395,453}, {0x0396,454}, {0x0397,455}, {0x0398,456}, {0x0399,457}, {0x039A,458}, 
  {0x039B,459}, {0x039C,460}, {0x039D,461}, {0x039E,462}, {0x039F,463}, {0x03A0,464}, 
  {0x03A1,465}, {0x03A3,467}, {0x03A4,468}, {0x03A5,469}, {0x03A6,470}, {0x03A7,471}, 
  {0x03A8,472}, {0x03A9,473}, {0x03AA,474}, {0x03AB,475}, {0x03AC,476}, {0x03AD,477}, 
  {0x03AE,478}, {0x03AF,479}, {0x03B0,480}, {0x03B1,481}, {0x03B2,482}, {0x03B3,483}, 
  {0x03B4,484}, {0x03B5,485}, {0x03B6,486}, {0x03B7,487}, {0x03B8,488}, {0x03B9,489}, 
  {0x03BA,490}, {0x03BB,491}, {0x03BC,492}, {0x03BD,493}, {0x03BE,494}, {0x03BF,495}, 
  {0x03C0,496}, {0x03C1,497}, {0x03C2,498}, {0x03C3,499}, {0x03C4,500}, {0x03C5,501}, 
  {0x03C6,502}, {0x03C7,503}, {0x03C8,504}, {0x03C9,505}, {0x03CA,506}, {0x03CB,507}, 
  {0x03CC,508}, {0x03CD,509}, {0x03CE,510}, {0x0401,289}, {0x0402,290}, {0x0403,291}, 
  {0x0404,292}, {0x0405,293}, {0x0406,294}, {0x0407,295}, {0x0408,296}, {0x0409,297}, 
  {0x040A,298}, {0x040B,299}, {0x040C,300}, {0x040E,302}, {0x040F,303}, {0x0410,304}, 
  {0x0411,305}, {0x0412,306}, {0x0413,307}, {0x0414,308}, {0x0415,309}, {0x0416,310}, 
  {0x0417,311}, {0x0418,312}, {0x0419,313}, {0x041A,314}, {0x041B,315}, {0x041C,316}, 
  {0x041D,317}, {0x041E,318}, {0x041F,319}, {0x0420,320}, {0x0421,321}, {0x0422,322}, 
  {0x0423,323}, {0x0424,324}, {0x0425,325}, {0x0426,326}, {0x0427,327}, {0x0428,328}, 
  {0x0429,329}, {0x042A,330}, {0x042B,331}, {0x042C,332}, {0x042D,333}, {0x042E,334}, 
  {0x042F,335}, {0x0430,336}, {0x0431,337}, {0x0432,338}, {0x0433,339}, {0x0434,340}, 
  {0x0435,341}, {0x0436,342}, {0x0437,343}, {0x0438,344}, {0x0439,345}, {0x043A,346}, 
  {0x043B,347}, {0x043C,348}, {0x043D,349}, {0x043E,350}, {0x043F,351}, {0x0440,352}, 
  {0x0441,353}, {0x0442,354}, {0x0443,355}, {0x0444,356}, {0x0445,357}, {0x0446,358}, 
  {0x0447,359}, {0x0448,360}, {0x0449,361}, {0x044A,362}, {0x044B,363}, {0x044C,364}, 
  {0x044D,365}, {0x044E,366}, {0x044F,367}, {0x0451,369}, {0x0452,370}, {0x0453,371}, 
  {0x0454,372}, {0x0455,373}, {0x0456,374}, {0x0457,375}, {0x0458,376}, {0x0459,377}, 
  {0x045A,378}, {0x045B,379}, {0x045C,380}, {0x045E,382}, {0x045F,383}, {0x2013,150}, 
  {0x2014,151}, {0x2015,431}, {0x2018,145}, {0x2018,417}, {0x2019,146}, {0x2019,418}, 
  {0x201A,130}, {0x201C,147}, {0x201D,148}, {0x201E,132}, {0x2020,134}, {0x2021,135}, 
  {0x2022,149}, {0x2026,133}, {0x2030,137}, {0x2039,139}, {0x203A,155}, {0x20AC,128}, 
  {0x20AC,420}, {0x2116,368}, {0x2122,153}, {0x2190,256}, {0x2191,257}, {0x2192,258}, 
  {0x2193,259}, {0x2194,260}, {0x2195,261}, {0x21B5,262}, {0x21D0,263}, {0x21D1,264}, 
  {0x21D2,265}, {0x21D3,266}, {0x21D4,267}, {0x21D5,268}, {0x2200,270}, {0x2203,271}, 
  {0x2204,272}, {0x2205,273}, {0x2206,274}, {0x2208,275}, {0x2209,276}, {0x221A,277}, 
  {0x221E,278}, {0x2229,279}, {0x222A,280}, {0x2248,281}, {0x2260,282}, {0x2261,283}, 
  {0x2320,384}, {0x2321,386}, {0x239B,387}, {0x239C,388}, {0x239D,389}, {0x239E,390}, 
  {0x239F,391}, {0x23A0,392}, {0x23A1,393}, {0x23A2,394}, {0x23A3,395}, {0x23A4,396}, 
  {0x23A5,397}, {0x23A6,398}, {0x23A7,399}, {0x23A8,400}, {0x23A9,401}, {0x23AB,403}, 
  {0x23AC,404}, {0x23AD,405}, {0x23AE,385}, {0x2500,  1}, {0x2502,  2}, {0x2508,  3}, 
  {0x250A,  4}, {0x250C, 05}, {0x2510,  6}, {0x2514,  7}, {0x2518,  8}, {0x251C,  9}, 
  {0x2524, 10}, {0x252C, 11}, {0x2534, 12}, {0x253C, 13}, {0x2588, 17}, {0x2591, 14}, 
  {0x2592, 15}, {0x2593, 16}, {0x263A, 18}, {0x263B, 19}, {0x263C, 20}, {0x2640, 21}, 
  {0x2642, 22}, {0x2660, 23}, {0x2663, 24}, {0x2665, 25}, {0x2666, 26}, {0x266A, 27}, 
  {0x266B, 28}, {0x2714, 29}, {0x2718, 30}};
  
  unsigned short *ind;

  if ((src > 0x1F && src < 0x80) || (src >= 0xA0 && src < 0x100)) {
    *des = src;
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_mgrx512, 273,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  return 0;
}

int _GrRecode_UCS2_ISO88595(long src, unsigned char *des)
{
  static unsigned short r_iso88595[95][2] = {
  {0x00A0,160}, {0x00A7,253}, {0x0401,161}, {0x0402,162}, {0x0403,163}, {0x0404,164},
  {0x0405,165}, {0x0406,166}, {0x0407,167}, {0x0408,168}, {0x0409,169}, {0x040A,170},
  {0x040B,171}, {0x040C,172}, {0x040E,174}, {0x040F,175}, {0x0410,176}, {0x0411,177},
  {0x0412,178}, {0x0413,179}, {0x0414,180}, {0x0415,181}, {0x0416,182}, {0x0417,183},
  {0x0418,184}, {0x0419,185}, {0x041A,186}, {0x041B,187}, {0x041C,188}, {0x041D,189},
  {0x041E,190}, {0x041F,191}, {0x0420,192}, {0x0421,193}, {0x0422,194}, {0x0423,195},
  {0x0424,196}, {0x0425,197}, {0x0426,198}, {0x0427,199}, {0x0428,200}, {0x0429,201},
  {0x042A,202}, {0x042B,203}, {0x042C,204}, {0x042D,205}, {0x042E,206}, {0x042F,207},
  {0x0430,208}, {0x0431,209}, {0x0432,210}, {0x0433,211}, {0x0434,212}, {0x0435,213},
  {0x0436,214}, {0x0437,215}, {0x0438,216}, {0x0439,217}, {0x043A,218}, {0x043B,219},
  {0x043C,220}, {0x043D,221}, {0x043E,222}, {0x043F,223}, {0x0440,224}, {0x0441,225},
  {0x0442,226}, {0x0443,227}, {0x0444,228}, {0x0445,229}, {0x0446,230}, {0x0447,231},
  {0x0448,232}, {0x0449,233}, {0x044A,234}, {0x044B,235}, {0x044C,236}, {0x044D,237},
  {0x044E,238}, {0x044F,239}, {0x0451,241}, {0x0452,242}, {0x0453,243}, {0x0454,244},
  {0x0455,245}, {0x0456,246}, {0x0457,247}, {0x0458,248}, {0x0459,249}, {0x045A,250},
  {0x045B,251}, {0x045C,252}, {0x045E,254}, {0x045F,255}, {0x2116,240}};
  
  unsigned short *ind;

  if (src > 0x1F && src < 0x80) {
    *des = src;
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_iso88595, 95,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  return 0;
}

int _GrRecode_UCS2_ISO88597(long src, unsigned char *des)
{
  static unsigned short r_iso88597[90][2] = {
  {0x00A0,160}, {0x00A3,163}, {0x00A6,166}, {0x00A7,167}, {0x00A8,168}, {0x00A9,169},
  {0x00AB,171}, {0x00AC,172}, {0x00B0,176}, {0x00B1,177}, {0x00B2,178}, {0x00B3,179},
  {0x00B7,183}, {0x00BB,187}, {0x00BD,189}, {0x0384,180}, {0x0385,181}, {0x0386,182},
  {0x0388,184}, {0x0389,185}, {0x038A,186}, {0x038C,188}, {0x038E,190}, {0x038F,191},
  {0x0390,192}, {0x0391,193}, {0x0392,194}, {0x0393,195}, {0x0394,196}, {0x0395,197},
  {0x0396,198}, {0x0397,199}, {0x0398,200}, {0x0399,201}, {0x039A,202}, {0x039B,203},
  {0x039C,204}, {0x039D,205}, {0x039E,206}, {0x039F,207}, {0x03A0,208}, {0x03A1,209},
  {0x03A3,211}, {0x03A4,212}, {0x03A5,213}, {0x03A6,214}, {0x03A7,215}, {0x03A8,216},
  {0x03A9,217}, {0x03AA,218}, {0x03AB,219}, {0x03AC,220}, {0x03AD,221}, {0x03AE,222},
  {0x03AF,223}, {0x03B0,224}, {0x03B1,225}, {0x03B2,226}, {0x03B3,227}, {0x03B4,228},
  {0x03B5,229}, {0x03B6,230}, {0x03B7,231}, {0x03B8,232}, {0x03B9,233}, {0x03BA,234},
  {0x03BB,235}, {0x03BC,236}, {0x03BD,237}, {0x03BE,238}, {0x03BF,239}, {0x03C0,240},
  {0x03C1,241}, {0x03C2,242}, {0x03C3,243}, {0x03C4,244}, {0x03C5,245}, {0x03C6,246},
  {0x03C7,247}, {0x03C8,248}, {0x03C9,249}, {0x03CA,250}, {0x03CB,251}, {0x03CC,252},
  {0x03CD,253}, {0x03CE,254}, {0x2015,175}, {0x2018,161}, {0x2019,162}, {0x20AC,164}};
  
  unsigned short *ind;

  if (src > 0x1F && src < 0x80) {
    *des = src;
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_iso88597, 90,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  return 0;
}

int _GrRecode_UCS2_CP437Ext(long src, long *des)
{
  static unsigned short r_cp437ext[66][2] = {
  {0x00a4,256}, {0x00a6,257}, {0x00a8,258}, {0x00a9,259}, {0x00ad,260}, {0x00ae,261}, 
  {0x00af,262}, {0x00b3,263}, {0x00b4,264}, {0x00b8,265}, {0x00b9,266}, {0x00be,267}, 
  {0x00c0,268}, {0x00c1,269}, {0x00c2,270}, {0x00c3,271}, {0x00c8,272}, {0x00ca,273}, 
  {0x00cb,274}, {0x00cc,275}, {0x00cd,276}, {0x00ce,277}, {0x00cf,278}, {0x00d0,279}, 
  {0x00d2,280}, {0x00d3,281}, {0x00d4,282}, {0x00d5,283}, {0x00d7,284}, {0x00d8,285}, 
  {0x00d9,286}, {0x00da,287}, {0x00db,288}, {0x00dd,289}, {0x00de,290}, {0x00e3,291}, 
  {0x00f0,292}, {0x00f5,293}, {0x00f8,294}, {0x00fd,295}, {0x00fe,296}, {0x0152,307}, 
  {0x0153,319}, {0x0160,305}, {0x0161,317}, {0x0178,321}, {0x017d,308}, {0x017e,320}, 
  {0x02c6,303}, {0x02dc,315}, {0x2013,313}, {0x2014,314}, {0x2018,309}, {0x2019,310}, 
  {0x201a,298}, {0x201c,311}, {0x201d,312}, {0x201e,299}, {0x2020,301}, {0x2021,302}, 
  {0x2026,300}, {0x2030,304}, {0x2039,306}, {0x203a,318}, {0x20ac,297}, {0x2122,316}};

  unsigned char cdes;
  unsigned short *ind;
  
  if (_GrRecode_UCS2_CP437(src, &cdes)) {
    *des = cdes;
    return 1;
  }

  ind = bsearch((void *)&src, (void *)r_cp437ext, 66,
                2*sizeof(unsigned short), compara);
  if (ind != NULL) {
    *des = *(ind+1);
    return 1;
  }

  return 0;
}

int _GrRecodeEvent(GrEvent *ev, int srcenc, int desenc)
{
  long aux;
  unsigned char caux[4];
  int i;

  if (ev->p2 == GRKEY_KEYCODE) return 0;
  
  /* don't recode ASCII keys (include controls) */
  if (ev->p1 < 0x80) return 0;

  switch (srcenc) {
  case GRENC_CP437:
    _GrRecode_CP437_UCS2((unsigned char)ev->p1, &aux);
    break;
  case GRENC_CP850:
    _GrRecode_CP850_UCS2((unsigned char)ev->p1, &aux);
    break;
  case GRENC_CP1251:
    _GrRecode_CP1251_UCS2((unsigned char)ev->p1, &aux);
    break;
  case GRENC_CP1252:
    _GrRecode_CP1252_UCS2((unsigned char)ev->p1, &aux);
    break;
  case GRENC_CP1253:
    _GrRecode_CP1253_UCS2((unsigned char)ev->p1, &aux);
    break;
  case GRENC_ISO_8859_1:
    _GrRecode_ISO88591_UCS2((unsigned char)ev->p1, &aux);
    break;
  case GRENC_UTF_8:
    _GrRecode_UTF8_UCS2((unsigned char *)ev->cp1, &aux);
    break;
  case GRENC_UCS_2:
    aux = ev->p1;
    break;
  }

  switch (desenc) {
  case GRENC_CP437:
    caux[0] = '?';
    _GrRecode_UCS2_CP437(aux, caux);
    ev->p1 = caux[0];
    ev->p2 = 1;
    return 1;
  case GRENC_CP850:
    caux[0] = '?';
    _GrRecode_UCS2_CP850(aux, caux);
    ev->p1 = caux[0];
    ev->p2 = 1;
    return 1;
  case GRENC_CP1251:
    caux[0] = '?';
    _GrRecode_UCS2_CP1251(aux, caux);
    ev->p1 = caux[0];
    ev->p2 = 1;
    return 1;
  case GRENC_CP1252:
    caux[0] = '?';
    _GrRecode_UCS2_CP1252(aux, caux);
    ev->p1 = caux[0];
    ev->p2 = 1;
    return 1;
  case GRENC_CP1253:
    caux[0] = '?';
    _GrRecode_UCS2_CP1253(aux, caux);
    ev->p1 = caux[0];
    ev->p2 = 1;
    return 1;
  case GRENC_ISO_8859_1:
    caux[0] = '?';
    _GrRecode_UCS2_ISO88591(aux, caux);
    ev->p1 = caux[0];
    ev->p2 = 1;
    return 1;
  case GRENC_UTF_8:
    ev->p2 = _GrRecode_UCS2_UTF8(aux, caux);
    ev->p1 = 0;
    for (i=0; i<ev->p2; i++)
      ev->cp1[i] = caux[i];
    return 1;
  case GRENC_UCS_2:
    ev->p1 = aux;
    ev->p2 = 2;
    return 1;
  }

  return 0;
}

unsigned short GrCharRecodeToUCS2(long chr,int chrtype)
{
  long aux;
  
  aux = '?';
  switch (chrtype) {
  case GR_CP437_TEXT:
    _GrRecode_CP437_UCS2((unsigned char)chr, &aux);
    return (unsigned short)aux;
  case GR_CP850_TEXT:
    _GrRecode_CP850_UCS2((unsigned char)chr, &aux);
    return (unsigned short)aux;
  case GR_CP1251_TEXT:
    _GrRecode_CP1251_UCS2((unsigned char)chr, &aux);
    return (unsigned short)aux;
  case GR_CP1252_TEXT:
    _GrRecode_CP1252_UCS2((unsigned char)chr, &aux);
    return (unsigned short)aux;
  case GR_CP1253_TEXT:
    _GrRecode_CP1253_UCS2((unsigned char)chr, &aux);
    return (unsigned short)aux;
  case GR_ISO_8859_1_TEXT:
    _GrRecode_ISO88591_UCS2((unsigned char)chr, &aux);
    return (unsigned short)aux;
  case GR_UTF8_TEXT:
    return GrUTF8ToUCS2((unsigned char *)(&chr));
  default: // GR_UCS2_TEXT, GR_BYTE_TEXT, GR_WORD_TEXT
    return (unsigned short)chr;
  }
}

long GrCharRecodeFromUCS2(long chr,int chrtype)
{
  //long aux2;
  unsigned char caux;

  caux = '?';
  switch (chrtype) {
  case GR_CP437_TEXT:
    _GrRecode_UCS2_CP437(chr, &caux);
    return caux;
  case GR_CP850_TEXT:
    _GrRecode_UCS2_CP850(chr, &caux);
    return caux;
  case GR_CP1251_TEXT:
    _GrRecode_UCS2_CP1251(chr, &caux);
    return caux;
  case GR_CP1252_TEXT:
    _GrRecode_UCS2_CP1252(chr, &caux);
    return caux;
  case GR_CP1253_TEXT:
    _GrRecode_UCS2_CP1253(chr, &caux);
    return caux;
  case GR_ISO_8859_1_TEXT:
    _GrRecode_UCS2_ISO88591(chr, &caux);
    return caux;
  case GR_UTF8_TEXT:
    return GrUCS2ToUTF8(chr);
    //_GrRecode_UCS2_UTF8(chr, (unsigned char *)&aux2);
    // return aux2;
  default: // GR_UCS2_TEXT, GR_BYTE_TEXT, GR_WORD_TEXT
    return chr;
  }
}

unsigned short *GrTextRecodeToUCS2(const void *text,int length,int chrtype)
{
  unsigned short *buf, *buf2;
  int i, len2;
  long aux;

  if (length <= 0) length = 0;

  buf = NULL;
  switch (chrtype) {
  case GR_BYTE_TEXT:
  case GR_CP437_TEXT:
  case GR_CP850_TEXT:
  case GR_CP1251_TEXT:
  case GR_CP1252_TEXT:
  case GR_CP1253_TEXT:
  case GR_ISO_8859_1_TEXT:
    buf = calloc(length+1, sizeof(unsigned short));
    if (buf == NULL) return NULL;
    for (i=0; i<length; i++) {
        aux = ((unsigned char *)text)[i];
        buf[i] = GrCharRecodeToUCS2(aux, chrtype);
    }
    return buf;
  case GR_WORD_TEXT:
  case GR_UCS2_TEXT:
    buf = calloc(length+1, sizeof(unsigned short));
    if (buf == NULL) return NULL;
    for (i=0; i<length; i++) buf[i] = ((unsigned short *)text)[i];
    return buf;
  case GR_UTF8_TEXT:
    if (length == 0) {
        buf = calloc(1, sizeof(unsigned short));
        return buf;
    }
    buf = GrUTF8StrToUCS2Str((unsigned char *)text, &len2, length);
    if (buf == NULL) return NULL;
    if (len2 < length) { /* better that return NULL we add '?' */
                         /* so programmer can see he has a bug */
        buf2 = realloc(buf, (length+1) * sizeof(unsigned short));
        if (buf2 == NULL) {
            free(buf);
            return NULL;
        }
        for (i=len2; i<length; i++) buf2[i] = '?';
        buf2[length] = 0;
        return buf2;
    } else {
        return buf;
    }
  }
  
  return NULL;
}

void *GrTextRecodeFromUCS2(const unsigned short *text,int length,int chrtype)
{
  unsigned char *sbuf = NULL;
  unsigned short *hbuf = NULL;
  long laux;
  unsigned char *cutf8;
  int i, j, rlen;

  if (length <= 0) length = 0;

  switch (chrtype) {
  case GR_BYTE_TEXT:
  case GR_CP437_TEXT:
  case GR_CP850_TEXT:
  case GR_CP1251_TEXT:
  case GR_CP1252_TEXT:
  case GR_CP1253_TEXT:
  case GR_ISO_8859_1_TEXT:
    sbuf = malloc(length+1);
    if (sbuf == NULL) return NULL;
    for (i=0; i<length; i++)
        sbuf[i] = GrCharRecodeFromUCS2(text[i], chrtype);
    sbuf[length] = '\0';
    return sbuf;
  case GR_UTF8_TEXT:
    rlen = length;
    for (i=0; i<length; i++) {
        if (text[i] >= 0x80) {
            rlen++;
            if (text[i] >= 0x800) {
                rlen++;
                if (text[i] >= 0x10000)
                    rlen++;
            }
        }
    }
    sbuf = malloc(rlen+1);
    if (sbuf == NULL) return NULL;
    j = 0;
    cutf8 = (unsigned char *)&laux;
    for (i=0; i<length; i++) {
        laux = GrCharRecodeFromUCS2(text[i], chrtype);
        sbuf[j++] = cutf8[0];
        if (cutf8[1] != '\0') sbuf[j++] = cutf8[1];
        if (cutf8[2] != '\0') sbuf[j++] = cutf8[2];
        if (cutf8[3] != '\0') sbuf[j++] = cutf8[3];
    }
    sbuf[j] = '\0';
    return sbuf;
  case GR_WORD_TEXT:
  case GR_UCS2_TEXT:
    hbuf = malloc((length+1)*sizeof(unsigned short));
    if (hbuf == NULL) return NULL;
    for (i=0; i<length; i++)
        hbuf[i] = text[i];
    hbuf[length] = '\0';
    return hbuf;
  }

  return NULL;
}

unsigned char *GrTextRecodeToUTF8(const void *text,int length,int chrtype)
{
  unsigned char *stext, *sbuf, *cutf8;
  unsigned short *htext;
  int i, j, rlen;
  long aux;

  if (length <= 0) length = 0;

  switch (chrtype) {
  case GR_BYTE_TEXT:
  case GR_CP437_TEXT:
  case GR_CP850_TEXT:
  case GR_CP1251_TEXT:
  case GR_CP1252_TEXT:
  case GR_CP1253_TEXT:
  case GR_ISO_8859_1_TEXT:
    rlen = length;
    stext = (unsigned char *)text;
    for (i=0; i<length; i++) {
        aux = GrCharRecodeToUCS2(stext[i], chrtype);
        if (aux >= 0x80) {
            rlen++;
            if (aux >= 0x800) {
                rlen++;
                if (aux >= 0x10000)
                    rlen++;
            }
        }
    }
    sbuf = malloc(rlen+1);
    if (sbuf == NULL) return NULL;
    j = 0;
    cutf8 = (unsigned char *)&aux;
    for (i=0; i<length; i++) {
        aux = GrCharRecodeToUCS2(stext[i], chrtype);
        aux = GrUCS2ToUTF8(aux);
        sbuf[j++] = cutf8[0];
        if (cutf8[1] != '\0') sbuf[j++] = cutf8[1];
        if (cutf8[2] != '\0') sbuf[j++] = cutf8[2];
        if (cutf8[3] != '\0') sbuf[j++] = cutf8[3];
    }
    sbuf[j] = '\0';
    return sbuf;
  case GR_UTF8_TEXT:
    return (unsigned char *)strdup((char *)text);
  case GR_WORD_TEXT:
  case GR_UCS2_TEXT:
    rlen = length;
    htext = (unsigned short *)text;
    for (i=0; i<length; i++) {
        if (htext[i] >= 0x80) {
            rlen++;
            if (htext[i] >= 0x800) {
                rlen++;
                if (htext[i] >= 0x10000)
                    rlen++;
            }
        }
    }
    sbuf = malloc(rlen+1);
    if (sbuf == NULL) return NULL;
    j = 0;
    cutf8 = (unsigned char *)&aux;
    for (i=0; i<length; i++) {
        aux = GrUCS2ToUTF8(htext[i]);
        sbuf[j++] = cutf8[0];
        if (cutf8[1] != '\0') sbuf[j++] = cutf8[1];
        if (cutf8[2] != '\0') sbuf[j++] = cutf8[2];
        if (cutf8[3] != '\0') sbuf[j++] = cutf8[3];
    }
    sbuf[j] = '\0';
    return sbuf;
  }
  
  return NULL;
}

void *GrTextRecodeFromUTF8(const unsigned char *text,int length,int chrtype)
{
  unsigned char *sbuf = NULL;
  unsigned short *hbuf = NULL;
  long laux;
  int i, nb;

  if (length <= 0) length = 0;

  switch (chrtype) {
  case GR_BYTE_TEXT:
  case GR_CP437_TEXT:
  case GR_CP850_TEXT:
  case GR_CP1251_TEXT:
  case GR_CP1252_TEXT:
  case GR_CP1253_TEXT:
  case GR_ISO_8859_1_TEXT:
    sbuf = malloc(length+1);
    if (sbuf == NULL) return NULL;
    for (i=0; i<length; i++) {
        laux = GrNextUTF8Char(text, &nb);
        laux = GrCharRecodeToUCS2(laux, GR_UTF8_TEXT);
        sbuf[i] = GrCharRecodeFromUCS2(laux, chrtype);
        text += nb;
    }
    sbuf[length] = '\0';
    return sbuf;
  case GR_UTF8_TEXT:
    return strdup((char *)text);
  case GR_WORD_TEXT:
  case GR_UCS2_TEXT:
    hbuf = malloc((length+1)*sizeof(unsigned short));
    if (hbuf == NULL) return NULL;
    for (i=0; i<length; i++) {
        laux = GrNextUTF8Char(text, &nb);
        hbuf[i] = GrCharRecodeToUCS2(laux, GR_UTF8_TEXT);
        text += nb;
    }
    hbuf[length] = '\0';
    return hbuf;
  }

  return NULL;
}

int GrStrLen(const void *text, int chrtype)
{
  int length = 0;
  unsigned char *cs;
  unsigned short *ss;
    
  switch (chrtype) {
    case GR_BYTE_TEXT:
    case GR_CP437_TEXT:
    case GR_CP850_TEXT:
    case GR_CP1251_TEXT:
    case GR_CP1252_TEXT:
    case GR_CP1253_TEXT:
    case GR_ISO_8859_1_TEXT:
      cs = (unsigned char *)text;
      while(*cs) {
        length++;
        cs++;
      }
      return length;
    case GR_UTF8_TEXT:
      return GrUTF8StrLen((unsigned char *)text);
    case GR_WORD_TEXT:
    case GR_UCS2_TEXT:
      ss = (unsigned short *)text;
      while(*ss) {
        length++;
        ss++;
      }
      return length;
    default:
      return 0;
  }
}

/* UTF-8 utility functions */

int GrUTF8StrLen(const unsigned char *s)
{
    int nch = 0;

    while (*s) {
        // ASCII (1 byte)
        if (s[0] < 0x80) {
            nch++; s++;
            continue;
        }

        // two bytes
        if (s[0] >= 0xC2 && s[0] <= 0xDF) {
            if ((s[1] & 0xC0) != 0x80) {
                // ILLFORMED, consume 1 byte
                nch++; s++;
                continue;
            }
            nch++; s += 2;
            continue;
        }

        // three bytes
        if ((s[0] & 0xf0) == 0xe0) {
            if (s[0] == 0xE0) {
                if (s[1] < 0xA0 || s[1] > 0xBF) {
                    // ILLFORMED, consume 1 byte
                    nch++; s++;
                    continue;
                }
            } else if (s[0] == 0xED) {
                if (s[1] < 0x80 || s[1] > 0x9F) {
                    // ILLFORMED, consume 1 byte
                    nch++; s++;
                    continue;
                }
            } else {
                if ((s[1] & 0xC0) != 0x80) {
                    // ILLFORMED, consume 1 byte
                    nch++; s++;
                    continue;
                }
            }
            if ((s[2] & 0xC0) != 0x80) {
                // ILLFORMED, consume 2 bytes
                nch++; s += 2;
                continue;
            }
            nch++; s += 3;
            continue;
        }

        // four bytes
        if (s[0] >= 0xF0 && s[0] <= 0xF4) {
            if (s[0] == 0xF0) {
                if (s[1] < 0x90 || s[1] > 0xBF) {
                    // ILLFORMED, consume 1 byte
                    nch++; s++;
                    continue;
                }
            } else if (s[0] == 0xF4) {
                if (s[1] < 0x80 || s[1] > 0x8F) {
                    // ILLFORMED, consume 1 byte
                    nch++; s++;
                    continue;
                }
            } else {
                if ((s[1] & 0xC0) != 0x80) {
                    // ILLFORMED, consume 1 byte
                    nch++; s++;
                    continue;
                }
            }
            if ((s[2] & 0xC0) != 0x80) {
                // ILLFORMED, consume 2 bytes
                nch++; s += 2;
                continue;
            }
            if ((s[3] & 0xC0) != 0x80) {
                // ILLFORMED, consume 3 bytes
                nch++; s += 3;
                continue;
            }
            nch++; s += 4;
            continue;
        }

        // Here for forbiden bytes in UTF-8 C0-C1, F5-FF
        nch++; s++;
   }

    return nch;
}

/* returns a char[4] packed in a long */
long GrNextUTF8Char(const unsigned char *s, int *nb)
{
  long res = 0;
  unsigned char *caux;
  long des;
  int i;

  caux = (unsigned char *)&res;

  *nb = _GrRecode_UTF8_UCS2(s, &des);

  for (i=0; i<*nb; i++)
    caux[i] = s[i];
    
  return res;
}

unsigned short GrUTF8ToUCS2(const unsigned char *s)
{
  long des = (unsigned char)'?';

  _GrRecode_UTF8_UCS2(s, &des);
  return des;
}

/* returns a char[4] packed in a long */
long GrUCS2ToUTF8(unsigned short ch)
{
  long res = (unsigned char)'?';
  unsigned char *caux;

  caux = (unsigned char *)&res;
  _GrRecode_UCS2_UTF8(ch, caux);
  return res;
}

/*
unsigned short GrUTF8ToUCS2nb(unsigned char *s, int *nb)
{
  long des;

  *nb = _GrRecode_UTF8_UCS2(s, &des);
  if (*nb == 0) {
    des = (unsigned char)'?';
    *nb = 1;
  }
  return des;
}
*/

/* returns a char[4] packed in a long */
/*
long GrUCS2ToUTF8nb(unsigned short ch, int *nb)
{
  long res = (unsigned char)'?';
  unsigned char *caux;

  caux = (unsigned char *)&res;
  *nb = _GrRecode_UCS2_UTF8(ch, caux);
  return res;
}
*/

unsigned short *GrUTF8StrToUCS2Str(const unsigned char *s, int *ulen, int maxlen)
{
  unsigned short *buf;
  long des;
  int i, nb;

  *ulen = GrUTF8StrLen(s);
  if (maxlen > 0 && *ulen > maxlen) *ulen = maxlen;
  buf = calloc(*ulen+1, sizeof(unsigned short));
  if (buf == NULL) return NULL;

  des = 0;
  for (i=0; i<*ulen; i++) {
    nb = _GrRecode_UTF8_UCS2(s, &des);
    if (nb == 0) {
      des = '?';
      nb = 1;
    }
    buf[i] = des;
    s += nb;
  }

  return buf;
}
