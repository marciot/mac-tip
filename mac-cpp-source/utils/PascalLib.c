/*
 * Library for handling Pascal-Strings
 *
 * by Thomas Tempelmann, macdev@tempel.org
 *
 * Originally by Markus Fritze.
 * Changes and additions by Thomas Tempelmann:
 *  - pstrlen neu
 *  - BlockMoveData statt BlockMove
 *  - pstrcmp liefert nicht Boolean sondern short, wie bei strcmp.
 *  - pstrupper neu
 * 28.12.95 TT:
 *  - alle Zugriffe auf s[0] und d[0] word-expandiert, bevor mit ihnen gerechnet wird,
 *    um zu verhindern, daß bei der Länge 255 ein Überlauf nach 0 passiert.
 * 08.04.95 TT:
 *  - pstrcmp() lieferte "C"<"BB", da das Längenbyte mit verglichen wurde. Korrigiert.
 *  - BlockMoveData() durch copyBytes() ersetzt, da vermutlich i.d.R. schneller.
 * 09.04.95 TT:
 *  - Geschw-optimiert: pfindchar, pstrupper, pdelchars
 *  - pstrextract() neu
 * 30.4.96 TT:
 *  - copyBytes kopierte falsch, wenn source und dest sich überlagern und dest>source war.
 * 18.7.96 TT:
 *  - strcatc neu
 * 14.8.96 TT:
 *  - sicherheitshalber 'far' accesses eingeführt.
 * 23.07.00 TT:
 *  - disabled the "far" pragmas
 *  - changed "StringPtr" into "unsigned char*" so that it can be compiled for Windows, too.
 */

#include <ctype.h>

#include <string.h>
#include "PascalLib.h"

#pragma push
#pragma cplusplus on

/*
 * We need far data access in this module
 * (alternatively we could setup A5 appropriately):
 */
/* !TT July 23, 2000 - removed this
#ifndef __powerc
#pragma far_data on
#pragma far_strings on
#pragma far_vtables on
#endif
*/

static void copyBytes (const unsigned char* s0, unsigned char* d, unsigned short len)
{
    const unsigned char* s = s0;
    if (d > s) {
        // rückwärts kopieren
        if (len) {
            d += len;
            s += len;
            do { *--d = *--s; } while (--len);
        }
    } else {
        if (len) do { *d++ = *s++; } while (--len);
    }
}

void pstrcpy (unsigned char* d, const unsigned char* s)
{
    copyBytes (s, d, ((unsigned short)s[0])+1);
}

void pstrncpy (unsigned char* d, const unsigned char* s, short n)
// n: dest buffer size including length char
{
    if (n) {
        if (n > ((unsigned short)s[0])+1) n = ((unsigned short)s[0])+1;
        copyBytes (s, d, n);
        if (n < ((unsigned short)d[0])+1) d[0] = n-1;
    }
}

void pstrcat (unsigned char* d, const unsigned char* s)
{
    copyBytes (s + 1, d + ((unsigned short)d[0]) + 1, s[0]);
    d[0] += s[0];
}

void pstrcatc (unsigned char* d, const char* s)
{
    short n = strlen(s);
    copyBytes ((unsigned char*)s, d + ((unsigned short)d[0]) + 1, n);
    d[0] += n;
}

void pstrcatchar (unsigned char* d, const unsigned char c)
{
    d[++d[0]] = c;
}

short pfindchar (const unsigned char* d, const unsigned char c)
{
    short i, dlen = d[0];
    for (i = 1; i <= dlen; i++) {
        if (d[i] == c) return i;
    }
    return 0;
}

void pstrextract (unsigned char* d, const unsigned char* s, short offset, short len)
{
    short slen;
    if (offset < 1 || offset > (slen = s[0])) {
        d[0] = 0;
        return;
    }
    if (((offset-1) + len) > slen) {
        len = slen - offset + 1;
    }
    if (len <= 0) {
        d[0] = 0;
        return;
    }
    copyBytes (s + offset, d + 1, len);
    d[0] = len;
}

void pdelchars (unsigned char* d, short offset, short len)
{
    short dlen;
    if (offset > (dlen = d[0])) return; // Offset zu groß
    if (len <= 0) return;               // Länge zu klein
    if ((offset + len) > (dlen + 1)) {  // Länge zu groß?
        d[0] = offset;                  // String zurechtstutzen
        return;
    }
    copyBytes (d + offset + len, d + offset, dlen - len - offset + 1);
    d[0] -= len;
}

void pstrins (unsigned char* d, const unsigned char* s, short offset)
{
    short slen, dlen;
    if ((slen = s[0]) == 0) return;                 // einzufügender String = 0 Bytes => raus
    if (offset > (dlen = d[0])) {                   // ggf. an den String anhängen
        offset = dlen + 1;
    }
    copyBytes (d + offset, d + offset + slen, dlen + 1 - offset);   // String nach hinten
    copyBytes (s + 1, d + offset, slen);            // String einfügen
    d[0] += slen;                                   // und die Länge updaten
}

short pstrcmp (const unsigned char* s10, const unsigned char* s20)
{
    const unsigned char* s1 = s10;
    const unsigned char* s2 = s20;
    unsigned char   l1 = *s1++, l2 = *s2++;
    unsigned char   len = (l1 < l2)? l1:l2;

    if (len) do {
        if (*s1++ != *s2++) {
            return (s1[-1] < s2[-1])? -1: +1;
        }
    } while (--len);
    if (l1 == l2) {
        return 0;
    } else {
        return (l1 < l2)? -1: +1;
    }
}

short pstrlen (const unsigned char* s1)
{
    return s1[0];
}

void pstrupper (unsigned char* d)
{
    short i, dlen = d[0];
    for (i = 1; i <= dlen; i++) {
        d[i] = toupper (d[i]);
    }
}

typedef unsigned char byte;

/*
short pfindstr (const unsigned char* target, const unsigned char* search)
{
    unsigned char* s = search;
    unsigned char* d = target;
    short slen = s[0];
    if (slen) {
        short i;
        short iend = d[0] - slen + 1;
        for (i = 1; i <= iend; i++) {
            if (d[i] == s[1]) {
                short len = slen - 1;
                if (len) {
                    unsigned char* s1 = &s[2], d1 = &d[i+1];
                    do {
                        if (*s1++ != *d1++) {
                            break;
                        }
                    } while (--len);
                }
                if (len == 0) return i;
            }
        }
    }
    return 0;
}
*/

short pfindstr (const unsigned char* d, const unsigned char* s0)
{
    const unsigned char* s = s0;
    short slen = *s++;
    if (slen == 0) return 0;
    --slen;
    {
        byte c = *s++;
        short i;
        short end = d[0] - slen;
        for (i = 1; i <= end; i++) {
            if (d[i] == c) {
                const byte *sp = s, *dp = &d[i+1];
                byte len = slen;
                do {
                    if (len-- == 0) return i;   // gefunden
                } while (*sp++ == *dp++);
            }
        }
    }
    return 0;
}

void pstrnins (unsigned char* d, short destSize, const unsigned char* s, short offset)
// TT 25 Sep 97: does no write over end of dest string
// 'destSize': size of dest string including length byte
{
    short slen, dlen;
    if (offset < 1) return;
    if ((slen = s[0]) == 0) return;                 // einzufügender String = 0 Bytes => raus
    if (offset > (dlen = d[0])) {                   // ggf. an den String anhängen
        offset = dlen + 1;
    }
    if (offset >= destSize) return;
    if (offset + slen > destSize) {
        slen = destSize - offset;
        dlen = offset - 1;
    } else {
        if (slen + dlen >= destSize) {
            dlen = destSize - 1 - slen;
        }
        copyBytes (d + offset, d + offset + slen, dlen + 1 - offset);   // String nach hinten
    }
    copyBytes (s + 1, d + offset, slen);            // String einfügen
    d[0] = dlen + slen;                             // und die Länge updaten
}

#pragma pop
