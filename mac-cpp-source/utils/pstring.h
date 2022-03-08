/****************************************************************************
 *   Common Libraries (c) 1994 Marcio Teixeira                              *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   To view a copy of the GNU General Public License, go to the following  *
 *   location: <http://www.gnu.org/licenses/>.                              *
 ****************************************************************************/

short           pstrlen( unsigned char *str );
void            psetlen( unsigned char *str, short len );
short           pstrcmp( unsigned char *str1, unsigned char *str2 );
short           pstricmp( unsigned char *str1, unsigned char *str2 );
unsigned char * pstrcat( unsigned char *str1, unsigned char *str2 );
unsigned char * pstrcpy( unsigned char *str1, unsigned char *str2 );
unsigned char * pstrparams( unsigned char *str, unsigned char *p0, unsigned char *p1, unsigned char *p2, unsigned char *p3 );
short           pstrmemcmp( unsigned char *str, unsigned char *mem );
short           pstrmemicmp( unsigned char *str, unsigned char *mem );