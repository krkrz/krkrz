#include "tjsCommHead.h"
/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_COMMA = 258,
     T_EQUAL = 259,
     T_AMPERSANDEQUAL = 260,
     T_VERTLINEEQUAL = 261,
     T_CHEVRONEQUAL = 262,
     T_MINUSEQUAL = 263,
     T_PLUSEQUAL = 264,
     T_PERCENTEQUAL = 265,
     T_SLASHEQUAL = 266,
     T_BACKSLASHEQUAL = 267,
     T_ASTERISKEQUAL = 268,
     T_LOGICALOREQUAL = 269,
     T_LOGICALANDEQUAL = 270,
     T_RBITSHIFTEQUAL = 271,
     T_LARITHSHIFTEQUAL = 272,
     T_RARITHSHIFTEQUAL = 273,
     T_QUESTION = 274,
     T_LOGICALOR = 275,
     T_LOGICALAND = 276,
     T_VERTLINE = 277,
     T_CHEVRON = 278,
     T_AMPERSAND = 279,
     T_NOTEQUAL = 280,
     T_EQUALEQUAL = 281,
     T_DISCNOTEQUAL = 282,
     T_DISCEQUAL = 283,
     T_SWAP = 284,
     T_LT = 285,
     T_GT = 286,
     T_LTOREQUAL = 287,
     T_GTOREQUAL = 288,
     T_RARITHSHIFT = 289,
     T_LARITHSHIFT = 290,
     T_RBITSHIFT = 291,
     T_PERCENT = 292,
     T_SLASH = 293,
     T_BACKSLASH = 294,
     T_ASTERISK = 295,
     T_EXCRAMATION = 296,
     T_TILDE = 297,
     T_DECREMENT = 298,
     T_INCREMENT = 299,
     T_NEW = 300,
     T_DELETE = 301,
     T_TYPEOF = 302,
     T_PLUS = 303,
     T_MINUS = 304,
     T_SHARP = 305,
     T_DOLLAR = 306,
     T_ISVALID = 307,
     T_INVALIDATE = 308,
     T_INSTANCEOF = 309,
     T_LPARENTHESIS = 310,
     T_DOT = 311,
     T_LBRACKET = 312,
     T_THIS = 313,
     T_SUPER = 314,
     T_GLOBAL = 315,
     T_RBRACKET = 316,
     T_CLASS = 317,
     T_RPARENTHESIS = 318,
     T_COLON = 319,
     T_SEMICOLON = 320,
     T_LBRACE = 321,
     T_RBRACE = 322,
     T_CONTINUE = 323,
     T_FUNCTION = 324,
     T_DEBUGGER = 325,
     T_DEFAULT = 326,
     T_CASE = 327,
     T_EXTENDS = 328,
     T_FINALLY = 329,
     T_PROPERTY = 330,
     T_PRIVATE = 331,
     T_PUBLIC = 332,
     T_PROTECTED = 333,
     T_STATIC = 334,
     T_RETURN = 335,
     T_BREAK = 336,
     T_EXPORT = 337,
     T_IMPORT = 338,
     T_SWITCH = 339,
     T_IN = 340,
     T_INCONTEXTOF = 341,
     T_FOR = 342,
     T_WHILE = 343,
     T_DO = 344,
     T_IF = 345,
     T_VAR = 346,
     T_CONST = 347,
     T_ENUM = 348,
     T_GOTO = 349,
     T_THROW = 350,
     T_TRY = 351,
     T_SETTER = 352,
     T_GETTER = 353,
     T_ELSE = 354,
     T_CATCH = 355,
     T_OMIT = 356,
     T_SYNCHRONIZED = 357,
     T_WITH = 358,
     T_INT = 359,
     T_REAL = 360,
     T_STRING = 361,
     T_OCTET = 362,
     T_FALSE = 363,
     T_NULL = 364,
     T_TRUE = 365,
     T_VOID = 366,
     T_NAN = 367,
     T_INFINITY = 368,
     T_UPLUS = 369,
     T_UMINUS = 370,
     T_EVAL = 371,
     T_POSTDECREMENT = 372,
     T_POSTINCREMENT = 373,
     T_IGNOREPROP = 374,
     T_PROPACCESS = 375,
     T_ARG = 376,
     T_EXPANDARG = 377,
     T_INLINEARRAY = 378,
     T_ARRAYARG = 379,
     T_INLINEDIC = 380,
     T_DICELM = 381,
     T_WITHDOT = 382,
     T_THIS_PROXY = 383,
     T_WITHDOT_PROXY = 384,
     T_CONSTVAL = 385,
     T_SYMBOL = 386,
     T_REGEXP = 387,
     T_VARIANT = 388
   };
#endif
/* Tokens.  */
#define T_COMMA 258
#define T_EQUAL 259
#define T_AMPERSANDEQUAL 260
#define T_VERTLINEEQUAL 261
#define T_CHEVRONEQUAL 262
#define T_MINUSEQUAL 263
#define T_PLUSEQUAL 264
#define T_PERCENTEQUAL 265
#define T_SLASHEQUAL 266
#define T_BACKSLASHEQUAL 267
#define T_ASTERISKEQUAL 268
#define T_LOGICALOREQUAL 269
#define T_LOGICALANDEQUAL 270
#define T_RBITSHIFTEQUAL 271
#define T_LARITHSHIFTEQUAL 272
#define T_RARITHSHIFTEQUAL 273
#define T_QUESTION 274
#define T_LOGICALOR 275
#define T_LOGICALAND 276
#define T_VERTLINE 277
#define T_CHEVRON 278
#define T_AMPERSAND 279
#define T_NOTEQUAL 280
#define T_EQUALEQUAL 281
#define T_DISCNOTEQUAL 282
#define T_DISCEQUAL 283
#define T_SWAP 284
#define T_LT 285
#define T_GT 286
#define T_LTOREQUAL 287
#define T_GTOREQUAL 288
#define T_RARITHSHIFT 289
#define T_LARITHSHIFT 290
#define T_RBITSHIFT 291
#define T_PERCENT 292
#define T_SLASH 293
#define T_BACKSLASH 294
#define T_ASTERISK 295
#define T_EXCRAMATION 296
#define T_TILDE 297
#define T_DECREMENT 298
#define T_INCREMENT 299
#define T_NEW 300
#define T_DELETE 301
#define T_TYPEOF 302
#define T_PLUS 303
#define T_MINUS 304
#define T_SHARP 305
#define T_DOLLAR 306
#define T_ISVALID 307
#define T_INVALIDATE 308
#define T_INSTANCEOF 309
#define T_LPARENTHESIS 310
#define T_DOT 311
#define T_LBRACKET 312
#define T_THIS 313
#define T_SUPER 314
#define T_GLOBAL 315
#define T_RBRACKET 316
#define T_CLASS 317
#define T_RPARENTHESIS 318
#define T_COLON 319
#define T_SEMICOLON 320
#define T_LBRACE 321
#define T_RBRACE 322
#define T_CONTINUE 323
#define T_FUNCTION 324
#define T_DEBUGGER 325
#define T_DEFAULT 326
#define T_CASE 327
#define T_EXTENDS 328
#define T_FINALLY 329
#define T_PROPERTY 330
#define T_PRIVATE 331
#define T_PUBLIC 332
#define T_PROTECTED 333
#define T_STATIC 334
#define T_RETURN 335
#define T_BREAK 336
#define T_EXPORT 337
#define T_IMPORT 338
#define T_SWITCH 339
#define T_IN 340
#define T_INCONTEXTOF 341
#define T_FOR 342
#define T_WHILE 343
#define T_DO 344
#define T_IF 345
#define T_VAR 346
#define T_CONST 347
#define T_ENUM 348
#define T_GOTO 349
#define T_THROW 350
#define T_TRY 351
#define T_SETTER 352
#define T_GETTER 353
#define T_ELSE 354
#define T_CATCH 355
#define T_OMIT 356
#define T_SYNCHRONIZED 357
#define T_WITH 358
#define T_INT 359
#define T_REAL 360
#define T_STRING 361
#define T_OCTET 362
#define T_FALSE 363
#define T_NULL 364
#define T_TRUE 365
#define T_VOID 366
#define T_NAN 367
#define T_INFINITY 368
#define T_UPLUS 369
#define T_UMINUS 370
#define T_EVAL 371
#define T_POSTDECREMENT 372
#define T_POSTINCREMENT 373
#define T_IGNOREPROP 374
#define T_PROPACCESS 375
#define T_ARG 376
#define T_EXPANDARG 377
#define T_INLINEARRAY 378
#define T_ARRAYARG 379
#define T_INLINEDIC 380
#define T_DICELM 381
#define T_WITHDOT 382
#define T_THIS_PROXY 383
#define T_WITHDOT_PROXY 384
#define T_CONSTVAL 385
#define T_SYMBOL 386
#define T_REGEXP 387
#define T_VARIANT 388




/* Copy the first part of user declarations.  */
#line 1 "syntax/tjs.y"

/*---------------------------------------------------------------------------*/
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
/*---------------------------------------------------------------------------*/
/* tjs.y */
/* TJS2 bison input file */

#if defined(__APPLE__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "tjsInterCodeGen.h"
#include "tjsScriptBlock.h"
#include "tjsError.h"
#include "tjsArray.h"
#include "tjsDictionary.h"

#define YYMALLOC	::malloc
#define YYREALLOC	::realloc
#define YYFREE		::free

/* param */
#define YYPARSE_PARAM pm
#define YYLEX_PARAM pm

/* script block */
#define sb ((tTJSScriptBlock*)pm)

/* current context */
#define cc (sb->GetCurrentContext())

/* current node */
#define cn (cc->GetCurrentNode())

/* lexical analyzer */
#define lx (sb->GetLexicalAnalyzer())


namespace TJS {

/* yylex/yyerror prototype decl */
#define YYLEX_PROTO_DECL int yylex(YYSTYPE *yylex, void *pm);

int __yyerror(char * msg, void *pm);


#define yyerror(msg) __yyerror(msg, pm);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 62 "syntax/tjs.y"
{
	tjs_int			num;
	tTJSExprNode *		np;
}
/* Line 193 of yacc.c.  */
#line 424 "tjs.tab.c"
	YYSTYPE;
YYLEX_PROTO_DECL

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 437 "tjs.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1529

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  134
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  111
/* YYNRULES -- Number of rules.  */
#define YYNRULES  284
/* YYNRULES -- Number of states.  */
#define YYNSTATES  480

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   388

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    10,    13,    17,    19,
      21,    23,    26,    28,    30,    32,    34,    36,    39,    42,
      45,    47,    49,    51,    53,    55,    57,    59,    61,    63,
      65,    66,    71,    72,    73,    81,    82,    83,    93,    94,
      95,   103,   104,   109,   119,   120,   121,   124,   126,   127,
     129,   130,   132,   135,   138,   141,   143,   147,   150,   155,
     156,   159,   161,   165,   167,   169,   171,   173,   175,   177,
     182,   189,   198,   199,   203,   206,   208,   209,   216,   217,
     223,   224,   228,   232,   238,   239,   241,   244,   246,   250,
     253,   257,   262,   264,   268,   269,   276,   278,   280,   283,
     286,   287,   295,   296,   300,   305,   308,   309,   315,   316,
     319,   320,   326,   328,   332,   334,   337,   341,   342,   349,
     350,   357,   361,   364,   365,   371,   373,   377,   382,   386,
     388,   390,   394,   396,   400,   402,   406,   410,   414,   418,
     422,   426,   430,   434,   438,   442,   446,   450,   454,   458,
     462,   466,   468,   474,   476,   480,   482,   486,   488,   492,
     494,   498,   500,   504,   506,   510,   514,   518,   522,   524,
     528,   532,   536,   540,   542,   546,   550,   554,   556,   560,
     564,   566,   570,   574,   578,   581,   584,   586,   589,   592,
     595,   598,   601,   604,   607,   610,   613,   616,   619,   622,
     625,   628,   631,   634,   638,   642,   647,   650,   655,   658,
     663,   666,   668,   672,   674,   678,   683,   685,   686,   691,
     694,   697,   700,   701,   705,   707,   709,   711,   713,   715,
     717,   719,   721,   723,   725,   727,   728,   732,   733,   737,
     742,   744,   746,   750,   751,   753,   755,   757,   758,   763,
     765,   769,   770,   772,   773,   780,   781,   783,   787,   791,
     795,   796,   798,   799,   807,   808,   810,   812,   816,   819,
     822,   824,   826,   828,   830,   831,   840,   841,   843,   847,
     852,   857,   861,   865,   869
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     135,     0,    -1,   136,    -1,    -1,   137,   138,    -1,    -1,
     138,   139,    -1,   138,     1,    65,    -1,   140,    -1,   141,
      -1,    65,    -1,   201,    65,    -1,   149,    -1,   152,    -1,
     143,    -1,   146,    -1,   154,    -1,    81,    65,    -1,    68,
      65,    -1,    70,    65,    -1,   159,    -1,   167,    -1,   176,
      -1,   184,    -1,   190,    -1,   191,    -1,   193,    -1,   195,
      -1,   196,    -1,   199,    -1,    -1,    66,   142,   138,    67,
      -1,    -1,    -1,    88,   144,    55,   201,    63,   145,   139,
      -1,    -1,    -1,    89,   147,   139,    88,    55,   201,    63,
     148,    65,    -1,    -1,    -1,    90,    55,   150,   201,   151,
      63,   139,    -1,    -1,   149,    99,   153,   139,    -1,    87,
      55,   155,    65,   157,    65,   158,    63,   139,    -1,    -1,
      -1,   156,   160,    -1,   201,    -1,    -1,   201,    -1,    -1,
     201,    -1,   160,    65,    -1,    91,   161,    -1,    92,   161,
      -1,   162,    -1,   161,     3,   162,    -1,   131,   163,    -1,
     131,   163,     4,   200,    -1,    -1,    64,   164,    -1,   165,
      -1,   164,    22,   165,    -1,   131,    -1,   111,    -1,   104,
      -1,   105,    -1,   106,    -1,   107,    -1,   131,    30,   166,
      31,    -1,   131,    30,   131,    30,   166,    34,    -1,   131,
      30,   131,    30,   131,    30,   166,    36,    -1,    -1,   166,
       3,   164,    -1,   166,     3,    -1,   164,    -1,    -1,    69,
     131,   168,   171,   163,   141,    -1,    -1,    69,   170,   171,
     163,   141,    -1,    -1,    55,   175,    63,    -1,    55,   172,
      63,    -1,    55,   173,     3,   175,    63,    -1,    -1,   173,
      -1,   173,     3,    -1,   174,    -1,   173,     3,   174,    -1,
     131,   163,    -1,   131,    19,   163,    -1,   131,   163,     4,
     200,    -1,    40,    -1,   131,   163,    40,    -1,    -1,    75,
     131,    66,   177,   178,    67,    -1,   179,    -1,   181,    -1,
     179,   181,    -1,   181,   179,    -1,    -1,    97,    55,   131,
     163,    63,   180,   141,    -1,    -1,   183,   182,   141,    -1,
      98,    55,    63,   163,    -1,    98,   163,    -1,    -1,    62,
     131,   185,   186,   141,    -1,    -1,    73,   200,    -1,    -1,
      73,   200,     3,   187,   188,    -1,   189,    -1,   188,     3,
     189,    -1,   200,    -1,    80,    65,    -1,    80,   201,    65,
      -1,    -1,    84,    55,   201,    63,   192,   141,    -1,    -1,
     103,    55,   201,    63,   194,   139,    -1,    72,   201,    64,
      -1,    71,    64,    -1,    -1,    96,   197,   139,   198,   139,
      -1,   100,    -1,   100,    55,    63,    -1,   100,    55,   131,
      63,    -1,    95,   201,    65,    -1,   203,    -1,   202,    -1,
     202,    90,   201,    -1,   203,    -1,   202,     3,   203,    -1,
     204,    -1,   204,    29,   203,    -1,   204,     4,   203,    -1,
     204,     5,   203,    -1,   204,     6,   203,    -1,   204,     7,
     203,    -1,   204,     8,   203,    -1,   204,     9,   203,    -1,
     204,    10,   203,    -1,   204,    11,   203,    -1,   204,    12,
     203,    -1,   204,    13,   203,    -1,   204,    14,   203,    -1,
     204,    15,   203,    -1,   204,    18,   203,    -1,   204,    17,
     203,    -1,   204,    16,   203,    -1,   205,    -1,   205,    19,
     204,    64,   204,    -1,   206,    -1,   205,    20,   206,    -1,
     207,    -1,   206,    21,   207,    -1,   208,    -1,   207,    22,
     208,    -1,   209,    -1,   208,    23,   209,    -1,   210,    -1,
     209,    24,   210,    -1,   211,    -1,   210,    25,   211,    -1,
     210,    26,   211,    -1,   210,    27,   211,    -1,   210,    28,
     211,    -1,   212,    -1,   211,    30,   212,    -1,   211,    31,
     212,    -1,   211,    32,   212,    -1,   211,    33,   212,    -1,
     213,    -1,   212,    34,   213,    -1,   212,    35,   213,    -1,
     212,    36,   213,    -1,   214,    -1,   213,    48,   214,    -1,
     213,    49,   214,    -1,   216,    -1,   214,    37,   216,    -1,
     214,    38,   216,    -1,   214,    39,   216,    -1,   215,   216,
      -1,   214,    40,    -1,   217,    -1,    41,   216,    -1,    42,
     216,    -1,    43,   216,    -1,    44,   216,    -1,    45,   224,
      -1,    53,   216,    -1,    52,   216,    -1,   217,    52,    -1,
      46,   216,    -1,    47,   216,    -1,    50,   216,    -1,    51,
     216,    -1,    48,   216,    -1,    49,   216,    -1,    24,   216,
      -1,    40,   216,    -1,   217,    54,   216,    -1,   217,    85,
     216,    -1,    55,   104,    63,   216,    -1,   104,   216,    -1,
      55,   105,    63,   216,    -1,   105,   216,    -1,    55,   106,
      63,   216,    -1,   106,   216,    -1,   218,    -1,   218,    86,
     217,    -1,   221,    -1,    55,   201,    63,    -1,   218,    57,
     201,    61,    -1,   224,    -1,    -1,   218,    56,   219,   131,
      -1,   218,    44,    -1,   218,    43,    -1,   218,    41,    -1,
      -1,    56,   220,   131,    -1,   130,    -1,   131,    -1,    58,
      -1,    59,    -1,   169,    -1,    60,    -1,   111,    -1,   227,
      -1,   231,    -1,   236,    -1,   241,    -1,    -1,    11,   222,
     132,    -1,    -1,    38,   223,   132,    -1,   218,    55,   225,
      63,    -1,   101,    -1,   226,    -1,   225,     3,   226,    -1,
      -1,    40,    -1,   215,    -1,   200,    -1,    -1,    57,   228,
     229,    61,    -1,   230,    -1,   229,     3,   230,    -1,    -1,
     200,    -1,    -1,    37,    57,   232,   233,   235,    61,    -1,
      -1,   234,    -1,   233,     3,   234,    -1,   200,     3,   200,
      -1,   131,    64,   200,    -1,    -1,     3,    -1,    -1,    55,
      92,    63,    57,   237,   238,    61,    -1,    -1,   239,    -1,
     240,    -1,   239,     3,   240,    -1,    49,   130,    -1,    48,
     130,    -1,   130,    -1,   111,    -1,   236,    -1,   241,    -1,
      -1,    55,    92,    63,    37,    57,   242,   243,    61,    -1,
      -1,   244,    -1,   243,     3,   244,    -1,   130,     3,    49,
     130,    -1,   130,     3,    48,   130,    -1,   130,     3,   130,
      -1,   130,     3,   111,    -1,   130,     3,   236,    -1,   130,
       3,   241,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   219,   219,   224,   224,   230,   232,   233,   240,   241,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     270,   270,   277,   278,   277,   284,   287,   284,   293,   294,
     293,   300,   300,   306,   316,   317,   317,   319,   325,   326,
     331,   332,   337,   341,   342,   349,   350,   355,   357,   362,
     364,   368,   369,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   384,   386,   387,   388,   393,   393,   403,   403,
     416,   418,   419,   420,   424,   426,   427,   431,   432,   436,
     438,   440,   445,   447,   459,   458,   467,   468,   469,   470,
     474,   474,   485,   485,   494,   495,   501,   501,   508,   510,
     511,   511,   516,   517,   521,   526,   527,   534,   533,   541,
     540,   547,   548,   553,   553,   560,   561,   562,   568,   573,
     577,   578,   583,   584,   589,   590,   591,   592,   593,   594,
     595,   596,   597,   598,   599,   600,   601,   602,   603,   604,
     605,   610,   611,   619,   620,   624,   625,   630,   631,   635,
     636,   640,   641,   645,   646,   647,   648,   649,   653,   654,
     655,   656,   657,   661,   662,   663,   664,   669,   670,   671,
     675,   676,   677,   678,   679,   683,   687,   688,   689,   690,
     691,   692,   693,   694,   695,   696,   697,   698,   699,   700,
     701,   702,   703,   704,   705,   706,   707,   708,   709,   710,
     711,   715,   716,   721,   722,   723,   724,   725,   725,   729,
     730,   731,   732,   732,   740,   742,   745,   746,   747,   748,
     749,   750,   751,   752,   753,   754,   754,   757,   757,   765,
     770,   771,   772,   776,   777,   778,   779,   785,   785,   794,
     795,   800,   801,   806,   806,   816,   818,   819,   824,   825,
     832,   834,   841,   841,   851,   853,   859,   860,   866,   867,
     868,   869,   870,   871,   876,   876,   888,   890,   891,   896,
     897,   898,   899,   900,   901
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "\",\"", "\"=\"", "\"&=\"", "\"|=\"",
  "\"^=\"", "\"-=\"", "\"+=\"", "\"%=\"", "\"/=\"", "\"\\\\=\"", "\"*=\"",
  "\"||=\"", "\"&&=\"", "\">>>=\"", "\"<<=\"", "\">>=\"", "\"?\"",
  "\"||\"", "\"&&\"", "\"|\"", "\"^\"", "\"&\"", "\"!=\"", "\"==\"",
  "\"!==\"", "\"===\"", "\"<->\"", "\"<\"", "\">\"", "\"<=\"", "\">=\"",
  "\">>\"", "\"<<\"", "\">>>\"", "\"%\"", "\"/\"", "\"\\\\\"", "\"*\"",
  "\"!\"", "\"~\"", "\"--\"", "\"++\"", "\"new\"", "\"delete\"",
  "\"typeof\"", "\"+\"", "\"-\"", "\"#\"", "\"$\"", "\"isvalid\"",
  "\"invalidate\"", "\"instanceof\"", "\"(\"", "\".\"", "\"[\"",
  "\"this\"", "\"super\"", "\"global\"", "\"]\"", "\"class\"", "\")\"",
  "\":\"", "\";\"", "\"{\"", "\"}\"", "\"continue\"", "\"function\"",
  "\"debugger\"", "\"default\"", "\"case\"", "\"extends\"", "\"finally\"",
  "\"property\"", "\"private\"", "\"public\"", "\"protected\"",
  "\"static\"", "\"return\"", "\"break\"", "\"export\"", "\"import\"",
  "\"switch\"", "\"in\"", "\"incontextof\"", "\"for\"", "\"while\"",
  "\"do\"", "\"if\"", "\"var\"", "\"const\"", "\"enum\"", "\"goto\"",
  "\"throw\"", "\"try\"", "\"setter\"", "\"getter\"", "\"else\"",
  "\"catch\"", "\"...\"", "\"synchronized\"", "\"with\"", "\"int\"",
  "\"real\"", "\"string\"", "\"octet\"", "\"false\"", "\"null\"",
  "\"true\"", "\"void\"", "\"NaN\"", "\"Infinity\"", "T_UPLUS", "T_UMINUS",
  "T_EVAL", "T_POSTDECREMENT", "T_POSTINCREMENT", "T_IGNOREPROP",
  "T_PROPACCESS", "T_ARG", "T_EXPANDARG", "T_INLINEARRAY", "T_ARRAYARG",
  "T_INLINEDIC", "T_DICELM", "T_WITHDOT", "T_THIS_PROXY",
  "T_WITHDOT_PROXY", "T_CONSTVAL", "T_SYMBOL", "T_REGEXP", "T_VARIANT",
  "$accept", "program", "global_list", "@1", "def_list",
  "block_or_statement", "statement", "block", "@2", "while", "@3", "@4",
  "do_while", "@5", "@6", "if", "@7", "@8", "if_else", "@9", "for",
  "for_first_clause", "@10", "for_second_clause", "for_third_clause",
  "variable_def", "variable_def_inner", "variable_id_list", "variable_id",
  "variable_type", "type_sym", "type_sym_single", "type_args", "func_def",
  "@11", "func_expr_def", "@12", "func_decl_arg_opt", "func_decl_arg_list",
  "func_decl_arg_at_least_one", "func_decl_arg", "func_decl_arg_collapse",
  "property_def", "@13", "property_handler_def_list",
  "property_handler_setter", "@14", "property_handler_getter", "@15",
  "property_getter_handler_head", "class_def", "@16", "class_extender",
  "@17", "extends_list", "extends_name", "return", "switch", "@18", "with",
  "@19", "case", "try", "@20", "catch", "throw", "expr_no_comma", "expr",
  "comma_expr", "assign_expr", "cond_expr", "logical_or_expr",
  "logical_and_expr", "inclusive_or_expr", "exclusive_or_expr", "and_expr",
  "identical_expr", "compare_expr", "shift_expr", "add_sub_expr",
  "mul_div_expr", "mul_div_expr_and_asterisk", "unary_expr",
  "incontextof_expr", "priority_expr", "@21", "@22", "factor_expr", "@23",
  "@24", "func_call_expr", "call_arg_list", "call_arg", "inline_array",
  "@25", "array_elm_list", "array_elm", "inline_dic", "@26",
  "dic_elm_list", "dic_elm", "dic_dummy_elm_opt", "const_inline_array",
  "@27", "const_array_elm_list_opt", "const_array_elm_list",
  "const_array_elm", "const_inline_dic", "@28", "const_dic_elm_list",
  "const_dic_elm", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   134,   135,   137,   136,   138,   138,   138,   139,   139,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     142,   141,   144,   145,   143,   147,   148,   146,   150,   151,
     149,   153,   152,   154,   155,   156,   155,   155,   157,   157,
     158,   158,   159,   160,   160,   161,   161,   162,   162,   163,
     163,   164,   164,   165,   165,   165,   165,   165,   165,   165,
     165,   165,   166,   166,   166,   166,   168,   167,   170,   169,
     171,   171,   171,   171,   172,   172,   172,   173,   173,   174,
     174,   174,   175,   175,   177,   176,   178,   178,   178,   178,
     180,   179,   182,   181,   183,   183,   185,   184,   186,   186,
     187,   186,   188,   188,   189,   190,   190,   192,   191,   194,
     193,   195,   195,   197,   196,   198,   198,   198,   199,   200,
     201,   201,   202,   202,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   204,   204,   205,   205,   206,   206,   207,   207,   208,
     208,   209,   209,   210,   210,   210,   210,   210,   211,   211,
     211,   211,   211,   212,   212,   212,   212,   213,   213,   213,
     214,   214,   214,   214,   214,   215,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   217,   217,   218,   218,   218,   218,   219,   218,   218,
     218,   218,   220,   218,   221,   221,   221,   221,   221,   221,
     221,   221,   221,   221,   221,   222,   221,   223,   221,   224,
     225,   225,   225,   226,   226,   226,   226,   228,   227,   229,
     229,   230,   230,   232,   231,   233,   233,   233,   234,   234,
     235,   235,   237,   236,   238,   238,   239,   239,   240,   240,
     240,   240,   240,   240,   242,   241,   243,   243,   243,   244,
     244,   244,   244,   244,   244
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     0,     2,     3,     1,     1,
       1,     2,     1,     1,     1,     1,     1,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     4,     0,     0,     7,     0,     0,     9,     0,     0,
       7,     0,     4,     9,     0,     0,     2,     1,     0,     1,
       0,     1,     2,     2,     2,     1,     3,     2,     4,     0,
       2,     1,     3,     1,     1,     1,     1,     1,     1,     4,
       6,     8,     0,     3,     2,     1,     0,     6,     0,     5,
       0,     3,     3,     5,     0,     1,     2,     1,     3,     2,
       3,     4,     1,     3,     0,     6,     1,     1,     2,     2,
       0,     7,     0,     3,     4,     2,     0,     5,     0,     2,
       0,     5,     1,     3,     1,     2,     3,     0,     6,     0,
       6,     3,     2,     0,     5,     1,     3,     4,     3,     1,
       1,     3,     1,     3,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     5,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     3,     3,     3,     1,     3,
       3,     3,     3,     1,     3,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     2,     2,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     3,     3,     4,     2,     4,     2,     4,
       2,     1,     3,     1,     3,     4,     1,     0,     4,     2,
       2,     2,     0,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     3,     0,     3,     4,
       1,     1,     3,     0,     1,     1,     1,     0,     4,     1,
       3,     0,     1,     0,     6,     0,     1,     3,     3,     3,
       0,     1,     0,     7,     0,     1,     1,     3,     2,     2,
       1,     1,     1,     1,     0,     8,     0,     1,     3,     4,
       4,     3,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       3,     0,     2,     5,     1,     0,     0,   235,     0,     0,
     237,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   222,   247,   226,   227,
     229,     0,    10,    30,     0,    78,     0,     0,     0,     0,
       0,     0,     0,     0,    32,    35,     0,     0,     0,     0,
     123,     0,     0,     0,     0,   230,   224,   225,     6,     8,
       9,    14,    15,    12,    13,    16,    20,     0,    21,   228,
      22,    23,    24,    25,    26,    27,    28,    29,     0,   130,
     132,   134,   151,   153,   155,   157,   159,   161,   163,   168,
     173,   177,     0,   180,   186,   211,   213,   216,   231,   232,
     233,   234,     7,     0,    78,   201,   253,     0,   202,   187,
     188,   189,   190,     0,     0,   191,   195,   196,   199,   200,
     197,   198,   193,   192,     0,     0,     0,     0,     0,     0,
     251,   106,     5,    18,    76,    80,    19,   122,     0,     0,
     115,     0,    17,     0,    45,     0,     0,    38,    59,    53,
      55,    54,     0,     0,     0,   206,   208,   210,    41,    52,
      11,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   185,   184,   194,     0,     0,   221,   220,   219,   243,
     217,     0,     0,   236,   255,   238,     0,     0,     0,     0,
     214,   223,   252,   129,     0,   249,   108,     0,    80,    84,
      59,   121,    94,   116,     0,     0,     0,    47,     0,     0,
       0,     0,    57,     0,   128,     0,     0,     0,   133,   131,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   150,   149,   148,   135,     0,   154,   156,   158,
     160,   162,   164,   165,   166,   167,   169,   170,   171,   172,
     174,   175,   176,   178,   179,   181,   182,   183,   203,   204,
     244,   240,   246,   245,     0,   241,     0,     0,   212,   225,
       0,   260,   256,     0,   262,   205,   207,   209,   251,   248,
       0,     0,    31,    59,    92,    59,     0,    85,    87,     0,
       0,     0,   117,    48,    46,     0,     0,    39,    65,    66,
      67,    68,    64,    63,    60,    61,     0,    56,   125,     0,
     119,    42,     0,   243,   239,   218,   215,     0,     0,   261,
       0,   274,   264,   250,   109,   107,     0,    59,    89,    82,
      86,    81,    79,     0,    59,     0,    96,    97,   102,     0,
       0,    49,    33,     0,     0,    72,     0,    58,     0,   124,
       0,   152,   242,   259,   258,   257,   254,   276,     0,     0,
       0,   271,   270,   272,     0,   265,   266,   273,   110,    77,
      90,     0,    93,    88,     0,     0,     0,   105,    95,    98,
      99,     0,   118,    50,     0,     0,     0,    63,    75,     0,
      62,   126,     0,   120,     0,     0,   277,   269,   268,   263,
       0,     0,    91,    83,    59,    59,   103,     0,    51,    34,
      36,    40,    72,    74,    69,   127,     0,     0,   275,   267,
     111,   112,   114,     0,   104,     0,     0,    63,     0,    73,
       0,     0,   282,   281,   283,   284,   278,     0,   100,    43,
      37,    72,    70,   280,   279,   113,     0,     0,   101,    71
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     3,     5,    58,    59,    60,   132,    61,
     145,   414,    62,   146,   456,    63,   240,   374,    64,   247,
      65,   235,   236,   370,   437,    66,    67,   149,   150,   242,
     418,   335,   419,    68,   228,    69,   135,   230,   316,   317,
     318,   319,    70,   321,   365,   366,   476,   367,   411,   368,
      71,   226,   311,   431,   450,   451,    72,    73,   369,    74,
     380,    75,    76,   153,   339,    77,   222,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,   296,   129,    96,   103,   107,
      97,   294,   295,    98,   130,   224,   225,    99,   214,   301,
     302,   350,   100,   352,   394,   395,   396,   101,   387,   425,
     426
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -421
static const yytype_int16 yypact[] =
{
    -421,    28,  -421,  -421,  -421,   309,   -44,  -421,  1259,   -24,
    -421,  1259,  1259,  1259,  1259,  1259,   174,  1259,  1259,  1259,
    1259,  1259,  1259,  1259,  1259,   597,  -421,  -421,  -421,  -421,
    -421,   -79,  -421,  -421,     3,   -49,    26,    41,  1259,   -16,
     693,    48,    62,    65,  -421,  -421,   110,    19,    19,  1259,
    -421,   111,  1259,  1259,  1259,  -421,  -421,  -421,  -421,  -421,
    -421,  -421,  -421,    75,  -421,  -421,  -421,   115,  -421,  -421,
    -421,  -421,  -421,  -421,  -421,  -421,  -421,  -421,   119,    11,
    -421,   663,   128,   162,   165,   166,   164,   136,   194,   107,
     -18,   239,  1259,  -421,    29,    67,  -421,  -421,  -421,  -421,
    -421,  -421,  -421,    63,  -421,  -421,  -421,    73,  -421,  -421,
    -421,  -421,  -421,   736,   116,   213,  -421,  -421,  -421,  -421,
    -421,  -421,  -421,  -421,   131,   832,   928,  1024,   137,    70,
    1259,  -421,  -421,  -421,  -421,   148,  -421,  -421,   135,   140,
    -421,   145,  -421,  1259,  1067,   161,   501,  -421,   157,   219,
    -421,   219,   170,   501,  1259,  -421,  -421,  -421,  -421,  -421,
    -421,  1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,
    1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,
    1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,
    1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,  1259,
    1259,  -421,  -421,  -421,  1259,  1259,  -421,  -421,  -421,  1163,
    -421,  1259,   174,  -421,  1302,  -421,    55,  1259,  1259,  1259,
    -421,  -421,  -421,  -421,    16,  -421,   163,   405,   148,   -31,
     157,  -421,  -421,  -421,   175,   179,   -17,  -421,  1259,   151,
    1259,    71,   241,    19,  -421,   152,   190,   501,  -421,  -421,
    -421,  -421,  -421,  -421,  -421,  -421,  -421,  -421,  -421,  -421,
    -421,  -421,  -421,  -421,  -421,  -421,   198,   162,   165,   166,
     164,   136,   194,   194,   194,   194,   107,   107,   107,   107,
     -18,   -18,   -18,   239,   239,  -421,  -421,  -421,  -421,  -421,
    1259,  -421,  -421,  1259,    15,  -421,   127,   202,  -421,   201,
     263,   264,  -421,   217,  -421,  -421,  -421,  -421,  1259,  -421,
    1259,   218,  -421,   157,  -421,     8,   227,   288,  -421,   229,
     218,    38,  -421,  1259,  -421,   230,   240,  -421,  -421,  -421,
    -421,  -421,  -421,   266,   272,  -421,  1259,  -421,   242,   501,
    -421,  -421,  1259,  1398,  -421,  -421,  -421,  1259,  1259,  1302,
     238,  -421,   -32,  -421,   297,  -421,   218,   157,    69,  -421,
     -31,  -421,  -421,   246,   -35,   236,   208,   210,  -421,   218,
     243,  -421,  -421,  1259,   248,    86,    71,  -421,   -38,  -421,
     501,  -421,  -421,  -421,  -421,  -421,  -421,   183,   184,   185,
     224,  -421,  -421,  -421,   256,   315,  -421,  -421,  -421,  -421,
    -421,  1259,  -421,  -421,   258,   188,   259,  -421,  -421,  -421,
    -421,   218,  -421,  1259,   501,   260,   501,   294,   272,    68,
    -421,  -421,   262,  -421,   323,    23,  -421,  -421,  -421,  -421,
     -32,  1259,  -421,  -421,   157,   157,  -421,   265,  -421,  -421,
    -421,  -421,   144,    71,  -421,  -421,    21,   183,  -421,  -421,
     324,  -421,  -421,   267,  -421,   501,   269,   299,   118,   272,
     206,   207,  -421,  -421,  -421,  -421,  -421,  1259,  -421,  -421,
    -421,   144,  -421,  -421,  -421,  -421,   218,    94,  -421,  -421
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -421,  -421,  -421,  -421,   199,  -143,  -421,  -253,  -421,  -421,
    -421,  -421,  -421,  -421,  -421,  -421,  -421,  -421,  -421,  -421,
    -421,  -421,  -421,  -421,  -421,  -421,   103,   292,    98,  -226,
    -239,   -34,  -420,  -421,  -421,  -421,  -421,   117,  -421,  -421,
      10,    12,  -421,  -421,  -421,   -23,  -421,    -3,  -421,  -421,
    -421,  -421,  -421,  -421,  -421,  -124,  -421,  -421,  -421,  -421,
    -421,  -421,  -421,  -421,  -421,  -421,  -203,   -25,  -421,  -129,
    -172,  -421,   193,   195,   200,   203,   211,    95,    97,   -39,
     -70,  -204,    42,   171,   369,  -421,  -421,  -421,  -421,  -421,
     371,  -421,    49,  -421,  -421,  -421,    83,  -421,  -421,  -421,
      45,  -421,  -344,  -421,  -421,  -421,   -28,  -340,  -421,  -421,
     -40
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -217
static const yytype_int16 yytable[] =
{
     128,   223,   334,   239,   320,   293,   292,   266,   393,   314,
     245,   300,   397,   138,   161,   141,   388,   389,   343,   308,
     406,   102,   458,   390,   152,   421,   447,   357,     4,   241,
     196,   197,   248,   106,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     105,   477,   131,   108,   109,   110,   111,   112,   355,   116,
     117,   118,   119,   120,   121,   122,   123,   362,   133,   460,
     461,   443,   241,   401,    47,    48,   390,   309,   344,   391,
     223,   203,   134,   204,   448,   223,   393,   356,   128,   358,
     397,   136,   303,   422,   155,   156,   157,   443,   392,   444,
     315,   162,   464,   399,   341,   137,   465,   354,   206,   402,
     207,   208,   304,   142,   205,   139,   412,   143,   234,   237,
     144,   443,   209,   210,   211,   444,   283,   284,   472,   246,
     479,   400,   462,   377,   202,   363,   364,   249,   407,   293,
     292,   193,   194,   195,   383,   384,   300,   179,   180,   444,
     148,   463,   472,   212,   280,   281,   282,   206,   436,   207,
     208,   185,   186,   187,   188,   147,   154,   155,   156,   157,
     381,   209,   210,   211,   158,   328,   329,   330,   331,   223,
     159,   223,   332,   181,   160,     7,   297,   182,   184,   183,
     328,   329,   330,   331,   216,   213,   379,   332,   432,   231,
     220,   221,   333,   229,   459,   215,   232,   223,   453,   454,
     233,     9,    10,   325,   223,   327,   238,   417,   223,   223,
     223,   241,   243,   478,   189,   190,   191,   192,   452,   113,
      26,    27,    28,    29,    30,   244,   310,   423,   322,   326,
     285,   286,   287,   104,   323,   336,   288,   289,   328,   329,
     330,   331,   338,   340,  -216,   332,  -216,  -216,   345,   305,
     306,   307,   342,   346,   452,   347,   348,   349,  -216,  -216,
    -216,   439,   223,   441,   351,   457,   198,   199,   200,   201,
     272,   273,   274,   275,    33,    55,   276,   277,   278,   279,
     359,   360,   361,   372,   376,   373,   375,   378,   371,   386,
     398,   405,   223,   408,    56,    57,   364,   363,   413,    -4,
       6,   416,   469,   424,   427,   428,   124,   429,   430,   434,
       7,   433,   435,   440,   442,   445,   446,   467,   455,   471,
     468,   227,   108,     8,   470,   202,   473,   474,   223,   324,
     151,   337,   420,   475,   410,   313,     9,    10,   415,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,   409,    25,    26,    27,    28,    29,    30,
     403,    31,   404,   267,    32,    33,   268,    34,    35,    36,
      37,    38,   269,   298,    39,   114,   270,   115,   438,    40,
      41,   353,   382,    42,   385,   271,    43,    44,    45,    46,
      47,    48,   449,     0,    49,    50,     6,   466,     0,     0,
       0,     0,    51,    52,    53,    54,     7,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,     8,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    56,
      57,     0,     9,    10,     0,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,     0,
      25,    26,    27,    28,    29,    30,     0,    31,     0,     0,
      32,    33,   312,    34,    35,    36,    37,    38,     0,     0,
      39,     0,     0,     0,     0,    40,    41,     0,     0,    42,
       0,     0,    43,    44,    45,    46,    47,    48,     0,     0,
      49,    50,     0,     0,     0,     0,     0,     0,    51,    52,
      53,    54,     7,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,     8,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    56,    57,     0,     9,    10,
       0,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,    27,    28,
      29,    30,     0,    31,     0,     0,    32,    33,     0,    34,
      35,    36,    37,    38,     0,     0,    39,     0,     0,     0,
       0,    40,    41,     0,     0,    42,     0,     0,    43,    44,
      45,    46,    47,    48,     0,     0,    49,    50,     0,     0,
       0,     0,     0,     0,    51,    52,    53,    54,     7,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,     8,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    56,    57,     0,     9,    10,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,    29,    30,     0,     0,
       0,     0,     0,     0,     0,     0,   104,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,     0,     0,     0,     0,     0,     0,     0,   124,
       0,     0,   178,     0,     0,     0,     0,     0,     0,     0,
       0,   125,   126,   127,     7,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    56,    57,     0,
       9,    10,     0,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,     7,    25,    26,
      27,    28,    29,    30,     0,     0,     0,     0,   140,     0,
       8,     0,   104,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     9,    10,     0,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,    28,    29,    30,    52,    53,    54,
       0,     0,     0,     0,    55,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    56,    57,     0,     0,     0,   124,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      52,    53,    54,     7,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,     8,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    56,    57,     0,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
      28,    29,    30,     0,     0,   217,     0,     0,     0,     0,
       0,   104,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    52,    53,    54,     7,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,     8,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    56,    57,     0,     9,    10,     0,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,    29,    30,     0,
       0,   218,     0,     0,     0,     0,     0,   104,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,    53,    54,     7,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,     8,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    56,    57,
       0,     9,    10,     0,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     7,    25,
      26,    27,    28,    29,    30,     0,     0,   219,     0,     0,
       0,     8,     0,   104,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     9,    10,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,    29,    30,    52,    53,
      54,     0,   -44,     0,     0,    55,   104,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    56,    57,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    52,    53,    54,     7,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    56,    57,     0,
       9,    10,     0,   290,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,     0,    25,    26,
      27,    28,    29,    30,     0,     0,     0,     0,     0,     0,
       0,     0,   104,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   291,     0,     0,    52,    53,    54,
       7,     0,     0,     0,    55,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    56,    57,     0,     9,    10,     0,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,     7,    25,    26,    27,    28,    29,    30,
       0,     0,     0,     0,     0,     0,     8,     0,   104,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
      28,    29,    30,    52,    53,    54,     0,     0,     0,     0,
      55,   104,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    56,
      57,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    52,    53,    54,     7,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,     8,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    56,   299,     0,     9,    10,     0,   290,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,    29,    30,     0,
       0,     0,     0,     0,     0,     0,     0,   104,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,    53,    54,     0,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    56,    57
};

static const yytype_int16 yycheck[] =
{
      25,   130,   241,   146,   230,   209,   209,   179,   352,    40,
     153,   214,   352,    38,     3,    40,    48,    49,     3,     3,
      55,    65,   442,    55,    49,    63,     3,    19,     0,    64,
      48,    49,   161,    57,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
       8,   471,   131,    11,    12,    13,    14,    15,   311,    17,
      18,    19,    20,    21,    22,    23,    24,   320,    65,    48,
      49,     3,    64,     4,    91,    92,    55,    61,    63,   111,
     209,    52,   131,    54,    61,   214,   430,   313,   113,   315,
     430,    65,    37,   131,    52,    53,    54,     3,   130,    31,
     131,    90,   446,   356,   247,    64,   446,   310,    41,    40,
      43,    44,    57,    65,    85,   131,   369,    55,   143,   144,
      55,     3,    55,    56,    57,    31,   196,   197,    34,   154,
      36,   357,   111,   336,    92,    97,    98,   162,   364,   343,
     343,    34,    35,    36,   347,   348,   349,    19,    20,    31,
     131,   130,    34,    86,   193,   194,   195,    41,   411,    43,
      44,    25,    26,    27,    28,    55,    55,   125,   126,   127,
     342,    55,    56,    57,    99,   104,   105,   106,   107,   308,
      65,   310,   111,    21,    65,    11,   211,    22,    24,    23,
     104,   105,   106,   107,    63,   132,   339,   111,   401,    64,
      63,   131,   131,    55,   443,   132,    66,   336,   434,   435,
      65,    37,    38,   238,   343,   240,    55,   131,   347,   348,
     349,    64,     3,   476,    30,    31,    32,    33,   431,    55,
      56,    57,    58,    59,    60,    65,    73,   380,    63,    88,
     198,   199,   200,    69,    65,     4,   204,   205,   104,   105,
     106,   107,   100,    63,    41,   111,    43,    44,   131,   217,
     218,   219,    64,    61,   467,    64,     3,     3,    55,    56,
      57,   414,   401,   416,    57,   131,    37,    38,    39,    40,
     185,   186,   187,   188,    66,   111,   189,   190,   191,   192,
      63,     3,    63,    63,    22,    55,    30,    55,   323,    61,
       3,    55,   431,    67,   130,   131,    98,    97,    65,     0,
       1,    63,   455,   130,   130,   130,    92,    61,     3,   131,
      11,    63,    63,    63,    30,    63,     3,     3,    63,    30,
      63,   132,   290,    24,    65,   293,   130,   130,   467,   236,
      48,   243,   376,   467,   367,   228,    37,    38,   373,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,   366,    55,    56,    57,    58,    59,    60,
     360,    62,   360,   180,    65,    66,   181,    68,    69,    70,
      71,    72,   182,   212,    75,    16,   183,    16,   413,    80,
      81,   308,   343,    84,   349,   184,    87,    88,    89,    90,
      91,    92,   430,    -1,    95,    96,     1,   447,    -1,    -1,
      -1,    -1,   103,   104,   105,   106,    11,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,    37,    38,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    56,    57,    58,    59,    60,    -1,    62,    -1,    -1,
      65,    66,    67,    68,    69,    70,    71,    72,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    80,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,    89,    90,    91,    92,    -1,    -1,
      95,    96,    -1,    -1,    -1,    -1,    -1,    -1,   103,   104,
     105,   106,    11,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    37,    38,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    -1,    62,    -1,    -1,    65,    66,    -1,    68,
      69,    70,    71,    72,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    80,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
      89,    90,    91,    92,    -1,    -1,    95,    96,    -1,    -1,
      -1,    -1,    -1,    -1,   103,   104,   105,   106,    11,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   130,   131,    -1,    37,    38,    -1,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    56,    57,    58,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,   105,   106,    11,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    11,    55,    56,
      57,    58,    59,    60,    -1,    -1,    -1,    -1,    65,    -1,
      24,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    38,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    56,    57,    58,    59,    60,   104,   105,   106,
      -1,    -1,    -1,    -1,   111,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,    -1,    -1,    92,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   105,   106,    11,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    37,
      38,    -1,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    -1,    -1,    63,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,    11,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,    37,    38,    -1,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    -1,
      -1,    63,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,   106,    11,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,    37,    38,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    11,    55,
      56,    57,    58,    59,    60,    -1,    -1,    63,    -1,    -1,
      -1,    24,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    37,    38,    -1,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    56,    57,    58,    59,    60,   104,   105,
     106,    -1,    65,    -1,    -1,   111,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,   105,   106,    11,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    56,
      57,    58,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,   104,   105,   106,
      11,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,    37,    38,    -1,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    11,    55,    56,    57,    58,    59,    60,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      38,    -1,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,   104,   105,   106,    -1,    -1,    -1,    -1,
     111,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,    11,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,    37,    38,    -1,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,   106,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   135,   136,   137,     0,   138,     1,    11,    24,    37,
      38,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    55,    56,    57,    58,    59,
      60,    62,    65,    66,    68,    69,    70,    71,    72,    75,
      80,    81,    84,    87,    88,    89,    90,    91,    92,    95,
      96,   103,   104,   105,   106,   111,   130,   131,   139,   140,
     141,   143,   146,   149,   152,   154,   159,   160,   167,   169,
     176,   184,   190,   191,   193,   195,   196,   199,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   221,   224,   227,   231,
     236,   241,    65,   222,    69,   216,    57,   223,   216,   216,
     216,   216,   216,    55,   218,   224,   216,   216,   216,   216,
     216,   216,   216,   216,    92,   104,   105,   106,   201,   220,
     228,   131,   142,    65,   131,   170,    65,    64,   201,   131,
      65,   201,    65,    55,    55,   144,   147,    55,   131,   161,
     162,   161,   201,   197,    55,   216,   216,   216,    99,    65,
      65,     3,    90,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    29,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    30,
      31,    32,    33,    34,    35,    36,    48,    49,    37,    38,
      39,    40,   216,    52,    54,    85,    41,    43,    44,    55,
      56,    57,    86,   132,   232,   132,    63,    63,    63,    63,
      63,   131,   200,   203,   229,   230,   185,   138,   168,    55,
     171,    64,    66,    65,   201,   155,   156,   201,    55,   139,
     150,    64,   163,     3,    65,   139,   201,   153,   203,   201,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   204,   206,   207,   208,
     209,   210,   211,   211,   211,   211,   212,   212,   212,   212,
     213,   213,   213,   214,   214,   216,   216,   216,   216,   216,
      40,   101,   200,   215,   225,   226,   219,   201,   217,   131,
     200,   233,   234,    37,    57,   216,   216,   216,     3,    61,
      73,   186,    67,   171,    40,   131,   172,   173,   174,   175,
     163,   177,    63,    65,   160,   201,    88,   201,   104,   105,
     106,   107,   111,   131,   164,   165,     4,   162,   100,   198,
      63,   139,    64,     3,    63,   131,    61,    64,     3,     3,
     235,    57,   237,   230,   200,   141,   163,    19,   163,    63,
       3,    63,   141,    97,    98,   178,   179,   181,   183,   192,
     157,   201,    63,    55,   151,    30,    22,   200,    55,   139,
     194,   204,   226,   200,   200,   234,    61,   242,    48,    49,
      55,   111,   130,   236,   238,   239,   240,   241,     3,   141,
     163,     4,    40,   174,   175,    55,    55,   163,    67,   181,
     179,   182,   141,    65,   145,   201,    63,   131,   164,   166,
     165,    63,   131,   139,   130,   243,   244,   130,   130,    61,
       3,   187,   200,    63,   131,    63,   141,   158,   201,   139,
      63,   139,    30,     3,    31,    63,     3,     3,    61,   240,
     188,   189,   200,   163,   163,    63,   148,   131,   166,   164,
      48,    49,   111,   130,   236,   241,   244,     3,    63,   139,
      65,    30,    34,   130,   130,   189,   180,   166,   141,    36
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 224 "syntax/tjs.y"
    { sb->PushContextStack(TJS_W("global"),
												ctTopLevel); ;}
    break;

  case 4:
#line 226 "syntax/tjs.y"
    { sb->PopContextStack(); ;}
    break;

  case 7:
#line 233 "syntax/tjs.y"
    { if(sb->CompileErrorCount>20)
												YYABORT;
											  else yyerrok; ;}
    break;

  case 11:
#line 247 "syntax/tjs.y"
    { cc->CreateExprCode((yyvsp[(1) - (2)].np)); ;}
    break;

  case 17:
#line 253 "syntax/tjs.y"
    { cc->DoBreak(); ;}
    break;

  case 18:
#line 254 "syntax/tjs.y"
    { cc->DoContinue(); ;}
    break;

  case 19:
#line 255 "syntax/tjs.y"
    { cc->DoDebugger(); ;}
    break;

  case 30:
#line 270 "syntax/tjs.y"
    { cc->EnterBlock(); ;}
    break;

  case 31:
#line 272 "syntax/tjs.y"
    { cc->ExitBlock(); ;}
    break;

  case 32:
#line 277 "syntax/tjs.y"
    { cc->EnterWhileCode(false); ;}
    break;

  case 33:
#line 278 "syntax/tjs.y"
    { cc->CreateWhileExprCode((yyvsp[(4) - (5)].np), false); ;}
    break;

  case 34:
#line 279 "syntax/tjs.y"
    { cc->ExitWhileCode(false); ;}
    break;

  case 35:
#line 284 "syntax/tjs.y"
    { cc->EnterWhileCode(true); ;}
    break;

  case 36:
#line 287 "syntax/tjs.y"
    { cc->CreateWhileExprCode((yyvsp[(6) - (7)].np), true); ;}
    break;

  case 37:
#line 288 "syntax/tjs.y"
    { cc->ExitWhileCode(true); ;}
    break;

  case 38:
#line 293 "syntax/tjs.y"
    { cc->EnterIfCode(); ;}
    break;

  case 39:
#line 294 "syntax/tjs.y"
    { cc->CreateIfExprCode((yyvsp[(4) - (4)].np)); ;}
    break;

  case 40:
#line 295 "syntax/tjs.y"
    { cc->ExitIfCode(); ;}
    break;

  case 41:
#line 300 "syntax/tjs.y"
    { cc->EnterElseCode(); ;}
    break;

  case 42:
#line 301 "syntax/tjs.y"
    { cc->ExitElseCode(); ;}
    break;

  case 43:
#line 310 "syntax/tjs.y"
    { cc->ExitForCode(); ;}
    break;

  case 44:
#line 316 "syntax/tjs.y"
    { cc->EnterForCode(); ;}
    break;

  case 45:
#line 317 "syntax/tjs.y"
    { cc->EnterForCode(); ;}
    break;

  case 47:
#line 319 "syntax/tjs.y"
    { cc->EnterForCode();
											  cc->CreateExprCode((yyvsp[(1) - (1)].np)); ;}
    break;

  case 48:
#line 325 "syntax/tjs.y"
    { cc->CreateForExprCode(NULL); ;}
    break;

  case 49:
#line 326 "syntax/tjs.y"
    { cc->CreateForExprCode((yyvsp[(1) - (1)].np)); ;}
    break;

  case 50:
#line 331 "syntax/tjs.y"
    { cc->SetForThirdExprCode(NULL); ;}
    break;

  case 51:
#line 332 "syntax/tjs.y"
    { cc->SetForThirdExprCode((yyvsp[(1) - (1)].np)); ;}
    break;

  case 57:
#line 355 "syntax/tjs.y"
    { cc->AddLocalVariable(
												lx->GetString((yyvsp[(1) - (2)].num))); ;}
    break;

  case 58:
#line 357 "syntax/tjs.y"
    { cc->InitLocalVariable(
											  lx->GetString((yyvsp[(1) - (4)].num)), (yyvsp[(4) - (4)].np)); ;}
    break;

  case 76:
#line 393 "syntax/tjs.y"
    { sb->PushContextStack(
												lx->GetString((yyvsp[(2) - (2)].num)),
											  ctFunction);
											  cc->EnterBlock();;}
    break;

  case 77:
#line 398 "syntax/tjs.y"
    { cc->ExitBlock(); sb->PopContextStack(); ;}
    break;

  case 78:
#line 403 "syntax/tjs.y"
    { sb->PushContextStack(
												TJS_W("(anonymous)"),
											  ctExprFunction);
											  cc->EnterBlock(); ;}
    break;

  case 79:
#line 408 "syntax/tjs.y"
    { cc->ExitBlock();
											  tTJSVariant v(cc);
											  sb->PopContextStack();
											  (yyval.np) = cc->MakeNP0(T_CONSTVAL);
											  (yyval.np)->SetValue(v); ;}
    break;

  case 89:
#line 436 "syntax/tjs.y"
    { cc->AddFunctionDeclArg(
												lx->GetString((yyvsp[(1) - (2)].num)), NULL); ;}
    break;

  case 90:
#line 438 "syntax/tjs.y"
    { cc->AddFunctionDeclArg(
												lx->GetString((yyvsp[(1) - (3)].num)), NULL); ;}
    break;

  case 91:
#line 440 "syntax/tjs.y"
    { cc->AddFunctionDeclArg(
												lx->GetString((yyvsp[(1) - (4)].num)), (yyvsp[(4) - (4)].np)); ;}
    break;

  case 92:
#line 445 "syntax/tjs.y"
    { cc->AddFunctionDeclArgCollapse(
												NULL); ;}
    break;

  case 93:
#line 447 "syntax/tjs.y"
    { cc->AddFunctionDeclArgCollapse(
												lx->GetString((yyvsp[(1) - (3)].num))); ;}
    break;

  case 94:
#line 459 "syntax/tjs.y"
    { sb->PushContextStack(
												lx->GetString((yyvsp[(2) - (3)].num)),
												ctProperty); ;}
    break;

  case 95:
#line 463 "syntax/tjs.y"
    { sb->PopContextStack(); ;}
    break;

  case 100:
#line 474 "syntax/tjs.y"
    { sb->PushContextStack(
												TJS_W("(setter)"),
												ctPropertySetter);
											  cc->EnterBlock();
											  cc->SetPropertyDeclArg(
												lx->GetString((yyvsp[(3) - (5)].num))); ;}
    break;

  case 101:
#line 480 "syntax/tjs.y"
    { cc->ExitBlock();
											  sb->PopContextStack(); ;}
    break;

  case 102:
#line 485 "syntax/tjs.y"
    { sb->PushContextStack(
												TJS_W("(getter)"),
												ctPropertyGetter);
											  cc->EnterBlock(); ;}
    break;

  case 103:
#line 489 "syntax/tjs.y"
    { cc->ExitBlock();
											  sb->PopContextStack(); ;}
    break;

  case 106:
#line 501 "syntax/tjs.y"
    { sb->PushContextStack(
												lx->GetString((yyvsp[(2) - (2)].num)),
												ctClass); ;}
    break;

  case 107:
#line 505 "syntax/tjs.y"
    { sb->PopContextStack(); ;}
    break;

  case 109:
#line 510 "syntax/tjs.y"
    { cc->CreateExtendsExprCode((yyvsp[(2) - (2)].np), true); ;}
    break;

  case 110:
#line 511 "syntax/tjs.y"
    { cc->CreateExtendsExprCode((yyvsp[(2) - (3)].np), false); ;}
    break;

  case 114:
#line 521 "syntax/tjs.y"
    { cc->CreateExtendsExprCode((yyvsp[(1) - (1)].np), false); ;}
    break;

  case 115:
#line 526 "syntax/tjs.y"
    { cc->ReturnFromFunc(NULL); ;}
    break;

  case 116:
#line 527 "syntax/tjs.y"
    { cc->ReturnFromFunc((yyvsp[(2) - (3)].np)); ;}
    break;

  case 117:
#line 534 "syntax/tjs.y"
    { cc->EnterSwitchCode((yyvsp[(3) - (4)].np)); ;}
    break;

  case 118:
#line 535 "syntax/tjs.y"
    { cc->ExitSwitchCode(); ;}
    break;

  case 119:
#line 541 "syntax/tjs.y"
    { cc->EnterWithCode((yyvsp[(3) - (4)].np)); ;}
    break;

  case 120:
#line 542 "syntax/tjs.y"
    { cc->ExitWithCode(); ;}
    break;

  case 121:
#line 547 "syntax/tjs.y"
    { cc->ProcessCaseCode((yyvsp[(2) - (3)].np)); ;}
    break;

  case 122:
#line 548 "syntax/tjs.y"
    { cc->ProcessCaseCode(NULL); ;}
    break;

  case 123:
#line 553 "syntax/tjs.y"
    { cc->EnterTryCode(); ;}
    break;

  case 124:
#line 556 "syntax/tjs.y"
    { cc->ExitTryCode(); ;}
    break;

  case 125:
#line 560 "syntax/tjs.y"
    { cc->EnterCatchCode(NULL); ;}
    break;

  case 126:
#line 561 "syntax/tjs.y"
    { cc->EnterCatchCode(NULL); ;}
    break;

  case 127:
#line 562 "syntax/tjs.y"
    { cc->EnterCatchCode(
												lx->GetString((yyvsp[(3) - (4)].num))); ;}
    break;

  case 128:
#line 568 "syntax/tjs.y"
    { cc->ProcessThrowCode((yyvsp[(2) - (3)].np)); ;}
    break;

  case 129:
#line 573 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 130:
#line 577 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 131:
#line 578 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_IF, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 132:
#line 583 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 133:
#line 584 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_COMMA, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 134:
#line 589 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 135:
#line 590 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_SWAP, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 136:
#line 591 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_EQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 137:
#line 592 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_AMPERSANDEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 138:
#line 593 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_VERTLINEEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 139:
#line 594 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_CHEVRONEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 140:
#line 595 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_MINUSEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 141:
#line 596 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PLUSEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 142:
#line 597 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PERCENTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 143:
#line 598 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_SLASHEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 144:
#line 599 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_BACKSLASHEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 145:
#line 600 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_ASTERISKEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 146:
#line 601 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALOREQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 147:
#line 602 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALANDEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 148:
#line 603 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RARITHSHIFTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 149:
#line 604 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LARITHSHIFTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 150:
#line 605 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RBITSHIFTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 151:
#line 610 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 152:
#line 613 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP3(T_QUESTION, (yyvsp[(1) - (5)].np), (yyvsp[(3) - (5)].np), (yyvsp[(5) - (5)].np)); ;}
    break;

  case 153:
#line 619 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 154:
#line 620 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALOR, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 155:
#line 624 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 156:
#line 626 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALAND, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 157:
#line 630 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 158:
#line 631 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_VERTLINE, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 159:
#line 635 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 160:
#line 636 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_CHEVRON, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 161:
#line 640 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 162:
#line 641 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_AMPERSAND, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 163:
#line 645 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 164:
#line 646 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_NOTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 165:
#line 647 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_EQUALEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 166:
#line 648 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_DISCNOTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 167:
#line 649 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_DISCEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 168:
#line 653 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 169:
#line 654 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 170:
#line 655 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_GT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 171:
#line 656 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LTOREQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 172:
#line 657 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_GTOREQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 173:
#line 661 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 174:
#line 662 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RARITHSHIFT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 175:
#line 663 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LARITHSHIFT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 176:
#line 664 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RBITSHIFT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 177:
#line 669 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 178:
#line 670 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PLUS, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 179:
#line 671 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_MINUS, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 180:
#line 675 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 181:
#line 676 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PERCENT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 182:
#line 677 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_SLASH, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 183:
#line 678 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_BACKSLASH, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 184:
#line 679 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_ASTERISK, (yyvsp[(1) - (2)].np), (yyvsp[(2) - (2)].np)); ;}
    break;

  case 185:
#line 683 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (2)].np); ;}
    break;

  case 186:
#line 687 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 187:
#line 688 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EXCRAMATION, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 188:
#line 689 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_TILDE, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 189:
#line 690 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_DECREMENT, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 190:
#line 691 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INCREMENT, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 191:
#line 692 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(2) - (2)].np); (yyval.np)->SetOpecode(T_NEW); ;}
    break;

  case 192:
#line 693 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INVALIDATE, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 193:
#line 694 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ISVALID, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 194:
#line 695 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ISVALID, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 195:
#line 696 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_DELETE, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 196:
#line 697 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_TYPEOF, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 197:
#line 698 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_SHARP, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 198:
#line 699 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_DOLLAR, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 199:
#line 700 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_UPLUS, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 200:
#line 701 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_UMINUS, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 201:
#line 702 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_IGNOREPROP, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 202:
#line 703 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_PROPACCESS, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 203:
#line 704 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_INSTANCEOF, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 204:
#line 705 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_IN, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 205:
#line 706 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INT, (yyvsp[(4) - (4)].np)); ;}
    break;

  case 206:
#line 707 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INT, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 207:
#line 708 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_REAL, (yyvsp[(4) - (4)].np)); ;}
    break;

  case 208:
#line 709 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_REAL, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 209:
#line 710 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_STRING, (yyvsp[(4) - (4)].np)); ;}
    break;

  case 210:
#line 711 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_STRING, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 211:
#line 715 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 212:
#line 717 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_INCONTEXTOF, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 213:
#line 721 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 214:
#line 722 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(2) - (3)].np); ;}
    break;

  case 215:
#line 723 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LBRACKET, (yyvsp[(1) - (4)].np), (yyvsp[(3) - (4)].np)); ;}
    break;

  case 216:
#line 724 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 217:
#line 725 "syntax/tjs.y"
    { lx->SetNextIsBareWord(); ;}
    break;

  case 218:
#line 726 "syntax/tjs.y"
    { tTJSExprNode * node = cc->MakeNP0(T_CONSTVAL);
												  node->SetValue(lx->GetValue((yyvsp[(4) - (4)].num)));
												  (yyval.np) = cc->MakeNP2(T_DOT, (yyvsp[(1) - (4)].np), node); ;}
    break;

  case 219:
#line 729 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_POSTINCREMENT, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 220:
#line 730 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_POSTDECREMENT, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 221:
#line 731 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EVAL, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 222:
#line 732 "syntax/tjs.y"
    { lx->SetNextIsBareWord(); ;}
    break;

  case 223:
#line 733 "syntax/tjs.y"
    { tTJSExprNode * node = cc->MakeNP0(T_CONSTVAL);
												  node->SetValue(lx->GetValue((yyvsp[(3) - (3)].num)));
												  (yyval.np) = cc->MakeNP1(T_WITHDOT, node); ;}
    break;

  case 224:
#line 740 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_CONSTVAL);
												  (yyval.np)->SetValue(lx->GetValue((yyvsp[(1) - (1)].num))); ;}
    break;

  case 225:
#line 742 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_SYMBOL);
												  (yyval.np)->SetValue(tTJSVariant(
													lx->GetString((yyvsp[(1) - (1)].num)))); ;}
    break;

  case 226:
#line 745 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_THIS); ;}
    break;

  case 227:
#line 746 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_SUPER); ;}
    break;

  case 228:
#line 747 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 229:
#line 748 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_GLOBAL); ;}
    break;

  case 230:
#line 749 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_VOID); ;}
    break;

  case 231:
#line 750 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 232:
#line 751 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 233:
#line 752 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 234:
#line 753 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 235:
#line 754 "syntax/tjs.y"
    { lx->SetStartOfRegExp(); ;}
    break;

  case 236:
#line 755 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_REGEXP);
												  (yyval.np)->SetValue(lx->GetValue((yyvsp[(3) - (3)].num))); ;}
    break;

  case 237:
#line 757 "syntax/tjs.y"
    { lx->SetStartOfRegExp(); ;}
    break;

  case 238:
#line 758 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_REGEXP);
												  (yyval.np)->SetValue(lx->GetValue((yyvsp[(3) - (3)].num))); ;}
    break;

  case 239:
#line 765 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LPARENTHESIS, (yyvsp[(1) - (4)].np), (yyvsp[(3) - (4)].np)); ;}
    break;

  case 240:
#line 770 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_OMIT); ;}
    break;

  case 241:
#line 771 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ARG, (yyvsp[(1) - (1)].np)); ;}
    break;

  case 242:
#line 772 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_ARG, (yyvsp[(3) - (3)].np), (yyvsp[(1) - (3)].np)); ;}
    break;

  case 243:
#line 776 "syntax/tjs.y"
    { (yyval.np) = NULL; ;}
    break;

  case 244:
#line 777 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EXPANDARG, NULL); ;}
    break;

  case 245:
#line 778 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EXPANDARG, (yyvsp[(1) - (1)].np)); ;}
    break;

  case 246:
#line 779 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 247:
#line 785 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_INLINEARRAY);
										  cc->PushCurrentNode(node); ;}
    break;

  case 248:
#line 789 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 249:
#line 794 "syntax/tjs.y"
    { cn->Add((yyvsp[(1) - (1)].np)); ;}
    break;

  case 250:
#line 795 "syntax/tjs.y"
    { cn->Add((yyvsp[(3) - (3)].np)); ;}
    break;

  case 251:
#line 800 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ARRAYARG, NULL); ;}
    break;

  case 252:
#line 801 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ARRAYARG, (yyvsp[(1) - (1)].np)); ;}
    break;

  case 253:
#line 806 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_INLINEDIC);
										  cc->PushCurrentNode(node); ;}
    break;

  case 254:
#line 811 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 256:
#line 818 "syntax/tjs.y"
    { cn->Add((yyvsp[(1) - (1)].np)); ;}
    break;

  case 257:
#line 819 "syntax/tjs.y"
    { cn->Add((yyvsp[(3) - (3)].np)); ;}
    break;

  case 258:
#line 824 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_DICELM, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 259:
#line 825 "syntax/tjs.y"
    { tTJSVariant val(lx->GetString((yyvsp[(1) - (3)].num)));
										  tTJSExprNode *node0 = cc->MakeNP0(T_CONSTVAL);
										  node0->SetValue(val);
										  (yyval.np) = cc->MakeNP2(T_DICELM, node0, (yyvsp[(3) - (3)].np)); ;}
    break;

  case 262:
#line 841 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_CONSTVAL);
										  iTJSDispatch2 * dsp = TJSCreateArrayObject();
										  node->SetValue(tTJSVariant(dsp, dsp));
										  dsp->Release();
										  cc->PushCurrentNode(node); ;}
    break;

  case 263:
#line 848 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 268:
#line 866 "syntax/tjs.y"
    { cn->AddArrayElement(- lx->GetValue((yyvsp[(2) - (2)].num))); ;}
    break;

  case 269:
#line 867 "syntax/tjs.y"
    { cn->AddArrayElement(+ lx->GetValue((yyvsp[(2) - (2)].num))); ;}
    break;

  case 270:
#line 868 "syntax/tjs.y"
    { cn->AddArrayElement(lx->GetValue((yyvsp[(1) - (1)].num))); ;}
    break;

  case 271:
#line 869 "syntax/tjs.y"
    { cn->AddArrayElement(tTJSVariant());  ;}
    break;

  case 272:
#line 870 "syntax/tjs.y"
    { cn->AddArrayElement((yyvsp[(1) - (1)].np)->GetValue()); ;}
    break;

  case 273:
#line 871 "syntax/tjs.y"
    { cn->AddArrayElement((yyvsp[(1) - (1)].np)->GetValue()); ;}
    break;

  case 274:
#line 876 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_CONSTVAL);
										  iTJSDispatch2 * dsp = TJSCreateDictionaryObject();
										  node->SetValue(tTJSVariant(dsp, dsp));
										  dsp->Release();
										  cc->PushCurrentNode(node); ;}
    break;

  case 275:
#line 883 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 279:
#line 896 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (4)].num)), - lx->GetValue((yyvsp[(4) - (4)].num))); ;}
    break;

  case 280:
#line 897 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (4)].num)), + lx->GetValue((yyvsp[(4) - (4)].num))); ;}
    break;

  case 281:
#line 898 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), lx->GetValue((yyvsp[(3) - (3)].num))); ;}
    break;

  case 282:
#line 899 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), tTJSVariant()); ;}
    break;

  case 283:
#line 900 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), (yyvsp[(3) - (3)].np)->GetValue()); ;}
    break;

  case 284:
#line 901 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), (yyvsp[(3) - (3)].np)->GetValue()); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3466 "tjs.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 906 "syntax/tjs.y"



}
