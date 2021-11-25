/************************************************************

    pstring.h
   
       AUTHOR: Marcio Luis Teixeira
       CREATED: 9/17/94
       
       LAST REVISION: 11/25/21
       
       (c) 1994-1995 by Marcio Luis Teixeira.
       All rights reserved.
  
*************************************************************/

short           pstrlen( unsigned char *str );
void            psetlen( unsigned char *str, short len );
short           pstrcmp( unsigned char *str1, unsigned char *str2 );
short           pstricmp( unsigned char *str1, unsigned char *str2 );
unsigned char * pstrcat( unsigned char *str1, unsigned char *str2 );
unsigned char * pstrcpy( unsigned char *str1, unsigned char *str2 );
unsigned char * pstrparams( unsigned char *str, unsigned char *p0, unsigned char *p1, unsigned char *p2, unsigned char *p3 );
short           pstrmemcmp( unsigned char *str, unsigned char *mem );
short           pstrmemicmp( unsigned char *str, unsigned char *mem );