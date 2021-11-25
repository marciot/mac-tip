/************************************************************

    pstring.c

       AUTHOR: Marcio Luis Teixeira
       CREATED: 9/17/94

       LAST REVISION: 11/25/21

       (c) 1994-1995 by Marcio Luis Teixeira.
       All rights reserved.

*************************************************************/

#include "pstring.h"
#include <ctype.h>

short pstrlen( unsigned char *str ) {
    return (short) *str;
}

void psetlen( unsigned char *str, short len ) {
    *str = (unsigned char) len;
}

short pstrcmp( unsigned char *str1, unsigned char *str2 ) {
    unsigned char i;

    i = *str1;
    while (i--)
        if (*str1++ != *str2++)
            return 1;
    return 0;
}

short pstricmp( unsigned char *str1, unsigned char *str2 ) {
    unsigned char i;

    if (*str1 != *str2)
        return 1;

    i = *str1;
    while (i--)
        if (tolower( *str1++ ) != tolower( *str2++ ))
            return 1;
    return 0;
}

short pstrmemcmp( unsigned char *str, unsigned char *mem ) {
    unsigned char i;

    i = *str;
    str++;
    while (i--)
        if (*str++ != *mem++)
            return 1;
    return 0;
}

short pstrmemicmp( unsigned char *str, unsigned char *mem ) {
    unsigned char i;

    i = *str;
    str++;
    while (i--)
        if (tolower( *str++ ) != tolower( *mem++ ))
            return 1;
    return 0;
}

unsigned char * pstrcat( unsigned char *str1, unsigned char *str2 ) {
    short   len1;
    short   len2;

    len2 = pstrlen( str2 );
    len1 = pstrlen( str1 );
    if (len1 + len2 > 255) return str1;
    BlockMove( str2 + 1, str1 + len1 + 1, len2 );
    psetlen( str1, len1 + len2 );
    return str1;
}

unsigned char * pstrcpy( unsigned char *str1, unsigned char *str2 ) {
    *str1 = '\0';
    str1 = pstrcat( str1, str2 );
    return str1;
}

unsigned char *pstrparams( unsigned char *str,
                           unsigned char *p0, unsigned char *p1,
                           unsigned char *p2, unsigned char *p3 ) {
    short                   len;
    unsigned char           *c;

    len = pstrlen( str );
    c = str;
    while( (len--) > 0 ) {
        c++;
        if( *c == '^' ) {
            short           pLen;
            unsigned char   *pStr;

            switch( *(c+1) ) {
                case '0': pStr = p0; break;
                case '1': pStr = p1; break;
                case '2': pStr = p2; break;
                case '3': pStr = p3; break;
                default: continue;
            }
            if( pStr == NULL ) continue;

            pLen = pstrlen( pStr );
            BlockMove( c + 2, c + pLen, len - 1 );
            BlockMove( pStr + 1, c, pLen );
            c += pLen;
            psetlen( str, pstrlen( str ) + pLen - 2 );
            len -= 2;
        }
    }
    return str;
}