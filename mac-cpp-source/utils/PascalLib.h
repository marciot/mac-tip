/*
 * Library for handling Pascal-Strings
 *
 * by Thomas Tempelmann, macdev@tempel.org
 */

#pragma push
#pragma cplusplus on

//#include <MacTypes.h>

void    pstrcpy (unsigned char* d, const unsigned char* s);
void    pstrcat (unsigned char* d, const unsigned char* s);
void    pstrcatc (unsigned char* d, const char* s);
void    pstrcatchar (unsigned char* d, unsigned char c);
short   pfindchar (const unsigned char* d, const unsigned char c);
void    pdelchars (unsigned char* d, short offset, short len);
void    pstrins (unsigned char* d, const unsigned char* s, short offset);
short   pstrcmp (const unsigned char* s1, const unsigned char* s2);
short   pstrlen (const unsigned char* s1);
void    pstrncpy (unsigned char* d, const unsigned char* s, short n);   // n: dest buffer size including length char
void    pstrupper (unsigned char* d);
void    pstrextract (unsigned char* d, const unsigned char* s, short offset, short len);
short   pfindstr (const unsigned char* d, const unsigned char* s);
void    pstrnins (unsigned char* d, short destSize, const unsigned char* s, short offset);

#pragma pop
