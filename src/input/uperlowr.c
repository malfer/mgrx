/**
 ** uperlowr.c ---- very limited tolower/toupper Unicode
 **
 ** Copyright (C) 2022 Mariano Alvarez Fernandez
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

// ordered!! table with unicode capital and small letters in iso8859-1/7/5
#define PAIRS 86
static unsigned short upper_lower_table[PAIRS][2] = {
    {0x00C0, 0x00E0 }, // À à latin-1 (iso8859-1)
    {0x00C1, 0x00E1 }, // Á á 
    {0x00C2, 0x00E2 }, // Â â 
    {0x00C3, 0x00E3 }, // Ã ã 
    {0x00C4, 0x00E4 }, // Ä ä 
    {0x00C5, 0x00E5 }, // Å å 
    {0x00C6, 0x00E6 }, // Æ æ 
    {0x00C7, 0x00E7 }, // Ç ç 
    {0x00C8, 0x00E8 }, // È è 
    {0x00C9, 0x00E9 }, // É é 
    {0x00CA, 0x00EA }, // Ê ê 
    {0x00CB, 0x00EB }, // Ë ë 
    {0x00CC, 0x00EC }, // Ì ì 
    {0x00CD, 0x00ED }, // Í í 
    {0x00CE, 0x00EE }, // Î î 
    {0x00CF, 0x00EF }, // Ï ï 
    {0x00D0, 0x00F0 }, // Ð ð 
    {0x00D1, 0x00F1 }, // Ñ ñ 
    {0x00D2, 0x00F2 }, // Ò ò 
    {0x00D3, 0x00F3 }, // Ó ó 
    {0x00D4, 0x00F4 }, // Ô ô 
    {0x00D5, 0x00F5 }, // Õ õ 
    {0x00D6, 0x00F6 }, // Ö ö 
    {0x00D8, 0x00F8 }, // Ø ø 
    {0x00D9, 0x00F9 }, // Ù ù 
    {0x00DA, 0x00FA }, // Ú ú 
    {0x00DB, 0x00FB }, // Û û 
    {0x00DC, 0x00FC }, // Ü ü 
    {0x00DD, 0x00FD }, // Ý ý 
    {0x00DE, 0x00FE }, // Þ þ 
    {0x0391, 0x03B1 }, // Α α greek (iso8859-7)
    {0x0392, 0x03B2 }, // Β β 
    {0x0393, 0x03B3 }, // Γ γ 
    {0x0394, 0x03B4 }, // Δ δ 
    {0x0395, 0x03B5 }, // Ε ε 
    {0x0396, 0x03B6 }, // Ζ ζ 
    {0x0397, 0x03B7 }, // Η η 
    {0x0398, 0x03B8 }, // Θ θ 
    {0x0399, 0x03B9 }, // Ι ι 
    {0x039A, 0x03BA }, // Κ κ 
    {0x039B, 0x03BB }, // Λ λ 
    {0x039C, 0x03BC }, // Μ μ 
    {0x039D, 0x03BD }, // Ν ν 
    {0x039E, 0x03BE }, // Ξ ξ 
    {0x039F, 0x03BF }, // Ο ο 
    {0x03A0, 0x03C0 }, // Π π 
    {0x03A1, 0x03C1 }, // Ρ ρ 
    {0x03A3, 0x03C3 }, // Σ σ 
    {0x03A4, 0x03C4 }, // Τ τ 
    {0x03A5, 0x03C5 }, // Υ υ 
    {0x03A6, 0x03C6 }, // Φ φ 
    {0x03A7, 0x03C7 }, // Χ χ 
    {0x03A8, 0x03C8 }, // Ψ ψ 
    {0x03A9, 0x03C9 }, // Ω ω 
    {0x0410, 0x0430 }, // А а cyrillic (iso8859-5)
    {0x0411, 0x0431 }, // Б б 
    {0x0412, 0x0432 }, // В в 
    {0x0413, 0x0433 }, // Г г 
    {0x0414, 0x0434 }, // Д д 
    {0x0415, 0x0435 }, // Е е 
    {0x0416, 0x0436 }, // Ж ж 
    {0x0417, 0x0437 }, // З з 
    {0x0418, 0x0438 }, // И и 
    {0x0419, 0x0439 }, // Й й 
    {0x041A, 0x043A }, // К к 
    {0x041B, 0x043B }, // Л л 
    {0x041C, 0x043C }, // М м 
    {0x041D, 0x043D }, // Н н 
    {0x041E, 0x043E }, // О о 
    {0x041F, 0x043F }, // П п 
    {0x0420, 0x0440 }, // Р р 
    {0x0421, 0x0441 }, // С с 
    {0x0422, 0x0442 }, // Т т 
    {0x0423, 0x0443 }, // У у 
    {0x0424, 0x0444 }, // Ф ф 
    {0x0425, 0x0445 }, // Х х 
    {0x0426, 0x0446 }, // Ц ц 
    {0x0427, 0x0447 }, // Ч ч 
    {0x0428, 0x0448 }, // Ш ш 
    {0x0429, 0x0449 }, // Щ щ 
    {0x042A, 0x044A }, // Ъ ъ 
    {0x042B, 0x044B }, // Ы ы 
    {0x042C, 0x044C }, // Ь ь 
    {0x042D, 0x044D }, // Э э 
    {0x042E, 0x044E }, // Ю ю 
    {0x042F, 0x044F }, // Я я 
};

static int compara(const void *arg1, const void *arg2)
{
  if (*(unsigned short *)arg1 < *(unsigned short *)arg2) return -1;
  else if (*(unsigned short *)arg1 > *(unsigned short *)arg2) return 1;
  else return 0;
}

unsigned short GrUCS2tolower(unsigned short chr)
{
    unsigned short *ind;
    
    if (chr < 128)
        return tolower(chr);

    ind = bsearch((void *)&chr, (void *)(&upper_lower_table[0][0]),
                  PAIRS, 2*sizeof(unsigned short), compara);

    if (ind != NULL)
        return *(ind+1);
    else
        return chr;
}

unsigned short GrUCS2toupper(unsigned short chr)
{
    unsigned short *ind;
    
    if (chr < 128)
        return toupper(chr);

    ind = bsearch((void *)&chr, (void *)(&upper_lower_table[0][1]),
                  PAIRS, 2*sizeof(unsigned short), compara);

    if (ind != NULL)
        return *(ind-1);
    else
        return chr;
}

// both functions expect an utf8 char packed in a long and return another one

long GrUTF8tolower(long chr)
{
    long uchr;
    unsigned char *caux;

    caux = (unsigned char *)&chr;
    
    if (caux[0] < 128) {
        caux[0] = tolower(caux[0]);
        return chr;
    }

    _GrRecode_UTF8_UCS2(caux, &uchr);
    uchr = GrUCS2tolower(uchr);
    _GrRecode_UCS2_UTF8(uchr, caux);

    return chr;
}

long GrUTF8toupper(long chr)
{
    long uchr;
    unsigned char *caux;

    caux = (unsigned char *)&chr;
    
    if (caux[0] < 128) {
        caux[0] = toupper(caux[0]);
        return chr;
    }

    _GrRecode_UTF8_UCS2(caux, &uchr);
    uchr = GrUCS2toupper(uchr);
    _GrRecode_UCS2_UTF8(uchr, caux);

    return chr;
}
