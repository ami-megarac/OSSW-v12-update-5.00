/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     _TRUE = 258,
     _FALSE = 259,
     _MULTIVAL = 260,
     _INT = 261,
     _ESCAPED = 262,
     _TAG = 263,
     _STRING = 264
   };
#endif
#define _TRUE 258
#define _FALSE 259
#define _MULTIVAL 260
#define _INT 261
#define _ESCAPED 262
#define _TAG 263
#define _STRING 264




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 102 "slp_attr_y.y"
typedef union YYSTYPE {
    int _i;
    char *_s;
    SLPAttrList *_atl;
} YYSTYPE;
/* Line 1249 of yacc.c.  */
#line 60 "slp_attr_y.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



