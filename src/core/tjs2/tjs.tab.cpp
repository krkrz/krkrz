#include "tjsCommHead.h"
/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "2.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
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


#include <malloc.h>


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



/* Line 189 of yacc.c  */
#line 128 "tjs.tab.cpp"

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
     T_ARROW = 325,
     T_DEBUGGER = 326,
     T_DEFAULT = 327,
     T_CASE = 328,
     T_EXTENDS = 329,
     T_FINALLY = 330,
     T_PROPERTY = 331,
     T_PRIVATE = 332,
     T_PUBLIC = 333,
     T_PROTECTED = 334,
     T_STATIC = 335,
     T_RETURN = 336,
     T_BREAK = 337,
     T_EXPORT = 338,
     T_IMPORT = 339,
     T_SWITCH = 340,
     T_IN = 341,
     T_INCONTEXTOF = 342,
     T_FOR = 343,
     T_WHILE = 344,
     T_DO = 345,
     T_IF = 346,
     T_VAR = 347,
     T_CONST = 348,
     T_ENUM = 349,
     T_GOTO = 350,
     T_THROW = 351,
     T_TRY = 352,
     T_SETTER = 353,
     T_GETTER = 354,
     T_ELSE = 355,
     T_CATCH = 356,
     T_OMIT = 357,
     T_SYNCHRONIZED = 358,
     T_WITH = 359,
     T_INT = 360,
     T_REAL = 361,
     T_STRING = 362,
     T_OCTET = 363,
     T_FALSE = 364,
     T_NULL = 365,
     T_TRUE = 366,
     T_VOID = 367,
     T_NAN = 368,
     T_INFINITY = 369,
     T_UPLUS = 370,
     T_UMINUS = 371,
     T_EVAL = 372,
     T_POSTDECREMENT = 373,
     T_POSTINCREMENT = 374,
     T_IGNOREPROP = 375,
     T_PROPACCESS = 376,
     T_ARG = 377,
     T_EXPANDARG = 378,
     T_INLINEARRAY = 379,
     T_ARRAYARG = 380,
     T_INLINEDIC = 381,
     T_DICELM = 382,
     T_WITHDOT = 383,
     T_THIS_PROXY = 384,
     T_WITHDOT_PROXY = 385,
     T_CONSTVAL = 386,
     T_SYMBOL = 387,
     T_REGEXP = 388,
     T_VARIANT = 389
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 60 "syntax/tjs.y"

	tjs_int			num;
	tTJSExprNode *		np;



/* Line 214 of yacc.c  */
#line 305 "tjs.tab.cpp"
} YYSTYPE;
YYLEX_PROTO_DECL

# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 317 "tjs.tab.cpp"

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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1809

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  135
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  112
/* YYNRULES -- Number of rules.  */
#define YYNRULES  279
/* YYNRULES -- Number of states.  */
#define YYNSTATES  472

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   389

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
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134
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
     156,   159,   162,   165,   168,   171,   174,   175,   182,   183,
     189,   190,   194,   198,   204,   205,   207,   209,   213,   216,
     221,   223,   227,   228,   234,   237,   239,   242,   246,   248,
     249,   256,   258,   260,   263,   266,   267,   275,   276,   280,
     285,   288,   289,   295,   296,   299,   300,   306,   308,   312,
     314,   317,   321,   322,   329,   330,   337,   341,   344,   345,
     351,   353,   357,   362,   366,   368,   370,   374,   376,   380,
     382,   386,   390,   394,   398,   402,   406,   410,   414,   418,
     422,   426,   430,   434,   438,   442,   446,   448,   454,   456,
     460,   462,   466,   468,   472,   474,   478,   480,   484,   486,
     490,   494,   498,   502,   504,   508,   512,   516,   520,   522,
     526,   530,   534,   536,   540,   544,   546,   550,   554,   558,
     561,   564,   566,   569,   572,   575,   578,   581,   584,   587,
     590,   593,   596,   599,   602,   605,   608,   611,   614,   618,
     623,   626,   631,   634,   639,   642,   644,   648,   650,   654,
     659,   661,   662,   667,   670,   673,   676,   677,   681,   683,
     685,   687,   689,   691,   693,   695,   697,   699,   701,   703,
     705,   706,   710,   711,   715,   720,   722,   724,   728,   729,
     731,   733,   735,   736,   741,   743,   747,   748,   750,   751,
     758,   759,   761,   765,   769,   773,   774,   776,   777,   785,
     786,   788,   790,   794,   797,   800,   802,   804,   806,   808,
     809,   818,   819,   821,   825,   830,   835,   839,   843,   847
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     136,     0,    -1,   137,    -1,    -1,   138,   139,    -1,    -1,
     139,   140,    -1,   139,     1,    65,    -1,   141,    -1,   142,
      -1,    65,    -1,   203,    65,    -1,   150,    -1,   153,    -1,
     144,    -1,   147,    -1,   155,    -1,    82,    65,    -1,    68,
      65,    -1,    71,    65,    -1,   160,    -1,   165,    -1,   178,
      -1,   186,    -1,   192,    -1,   193,    -1,   195,    -1,   197,
      -1,   198,    -1,   201,    -1,    -1,    66,   143,   139,    67,
      -1,    -1,    -1,    89,   145,    55,   203,    63,   146,   140,
      -1,    -1,    -1,    90,   148,   140,    89,    55,   203,    63,
     149,    65,    -1,    -1,    -1,    91,    55,   151,   203,   152,
      63,   140,    -1,    -1,   150,   100,   154,   140,    -1,    88,
      55,   156,    65,   158,    65,   159,    63,   140,    -1,    -1,
      -1,   157,   161,    -1,   203,    -1,    -1,   203,    -1,    -1,
     203,    -1,   161,    65,    -1,    92,   162,    -1,    93,   162,
      -1,   163,    -1,   162,     3,   163,    -1,   132,   164,    -1,
     132,   164,     4,   202,    -1,    -1,    64,   132,    -1,    64,
     112,    -1,    64,   105,    -1,    64,   106,    -1,    64,   107,
      -1,    64,   108,    -1,    -1,    69,   132,   166,   169,   164,
     142,    -1,    -1,    69,   168,   169,   164,   142,    -1,    -1,
      55,   173,    63,    -1,    55,   170,    63,    -1,    55,   171,
       3,   173,    63,    -1,    -1,   171,    -1,   172,    -1,   171,
       3,   172,    -1,   132,   164,    -1,   132,   164,     4,   202,
      -1,    40,    -1,   132,   164,    40,    -1,    -1,    70,   175,
     176,   164,   177,    -1,   132,   164,    -1,   169,    -1,    57,
      61,    -1,    57,   203,    61,    -1,   142,    -1,    -1,    76,
     132,    66,   179,   180,    67,    -1,   181,    -1,   183,    -1,
     181,   183,    -1,   183,   181,    -1,    -1,    98,    55,   132,
     164,    63,   182,   142,    -1,    -1,   185,   184,   142,    -1,
      99,    55,    63,   164,    -1,    99,   164,    -1,    -1,    62,
     132,   187,   188,   142,    -1,    -1,    74,   202,    -1,    -1,
      74,   202,     3,   189,   190,    -1,   191,    -1,   190,     3,
     191,    -1,   202,    -1,    81,    65,    -1,    81,   203,    65,
      -1,    -1,    85,    55,   203,    63,   194,   142,    -1,    -1,
     104,    55,   203,    63,   196,   140,    -1,    73,   203,    64,
      -1,    72,    64,    -1,    -1,    97,   199,   140,   200,   140,
      -1,   101,    -1,   101,    55,    63,    -1,   101,    55,   132,
      63,    -1,    96,   203,    65,    -1,   205,    -1,   204,    -1,
     204,    91,   203,    -1,   205,    -1,   204,     3,   205,    -1,
     206,    -1,   206,    29,   205,    -1,   206,     4,   205,    -1,
     206,     5,   205,    -1,   206,     6,   205,    -1,   206,     7,
     205,    -1,   206,     8,   205,    -1,   206,     9,   205,    -1,
     206,    10,   205,    -1,   206,    11,   205,    -1,   206,    12,
     205,    -1,   206,    13,   205,    -1,   206,    14,   205,    -1,
     206,    15,   205,    -1,   206,    18,   205,    -1,   206,    17,
     205,    -1,   206,    16,   205,    -1,   207,    -1,   207,    19,
     206,    64,   206,    -1,   208,    -1,   207,    20,   208,    -1,
     209,    -1,   208,    21,   209,    -1,   210,    -1,   209,    22,
     210,    -1,   211,    -1,   210,    23,   211,    -1,   212,    -1,
     211,    24,   212,    -1,   213,    -1,   212,    25,   213,    -1,
     212,    26,   213,    -1,   212,    27,   213,    -1,   212,    28,
     213,    -1,   214,    -1,   213,    30,   214,    -1,   213,    31,
     214,    -1,   213,    32,   214,    -1,   213,    33,   214,    -1,
     215,    -1,   214,    34,   215,    -1,   214,    35,   215,    -1,
     214,    36,   215,    -1,   216,    -1,   215,    48,   216,    -1,
     215,    49,   216,    -1,   218,    -1,   216,    37,   218,    -1,
     216,    38,   218,    -1,   216,    39,   218,    -1,   217,   218,
      -1,   216,    40,    -1,   219,    -1,    41,   218,    -1,    42,
     218,    -1,    43,   218,    -1,    44,   218,    -1,    45,   226,
      -1,    53,   218,    -1,    52,   218,    -1,   219,    52,    -1,
      46,   218,    -1,    47,   218,    -1,    50,   218,    -1,    51,
     218,    -1,    48,   218,    -1,    49,   218,    -1,    24,   218,
      -1,    40,   218,    -1,   219,    54,   218,    -1,    55,   105,
      63,   218,    -1,   105,   218,    -1,    55,   106,    63,   218,
      -1,   106,   218,    -1,    55,   107,    63,   218,    -1,   107,
     218,    -1,   220,    -1,   220,    87,   219,    -1,   223,    -1,
      55,   203,    63,    -1,   220,    57,   203,    61,    -1,   226,
      -1,    -1,   220,    56,   221,   132,    -1,   220,    44,    -1,
     220,    43,    -1,   220,    41,    -1,    -1,    56,   222,   132,
      -1,   131,    -1,   132,    -1,    58,    -1,    59,    -1,   167,
      -1,   174,    -1,    60,    -1,   112,    -1,   229,    -1,   233,
      -1,   238,    -1,   243,    -1,    -1,    11,   224,   133,    -1,
      -1,    38,   225,   133,    -1,   220,    55,   227,    63,    -1,
     102,    -1,   228,    -1,   227,     3,   228,    -1,    -1,    40,
      -1,   217,    -1,   202,    -1,    -1,    57,   230,   231,    61,
      -1,   232,    -1,   231,     3,   232,    -1,    -1,   202,    -1,
      -1,    37,    57,   234,   235,   237,    61,    -1,    -1,   236,
      -1,   235,     3,   236,    -1,   202,     3,   202,    -1,   132,
      64,   202,    -1,    -1,     3,    -1,    -1,    55,    93,    63,
      57,   239,   240,    61,    -1,    -1,   241,    -1,   242,    -1,
     241,     3,   242,    -1,    49,   131,    -1,    48,   131,    -1,
     131,    -1,   112,    -1,   238,    -1,   243,    -1,    -1,    55,
      93,    63,    37,    57,   244,   245,    61,    -1,    -1,   246,
      -1,   245,     3,   246,    -1,   131,     3,    49,   131,    -1,
     131,     3,    48,   131,    -1,   131,     3,   131,    -1,   131,
       3,   112,    -1,   131,     3,   238,    -1,   131,     3,   243,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   218,   218,   223,   223,   229,   231,   232,   239,   240,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     269,   269,   276,   277,   276,   283,   286,   283,   292,   293,
     292,   299,   299,   305,   315,   316,   316,   318,   324,   325,
     330,   331,   336,   340,   341,   348,   349,   354,   356,   361,
     363,   364,   365,   366,   367,   368,   373,   373,   383,   383,
     396,   398,   399,   400,   404,   406,   410,   411,   415,   417,
     422,   424,   444,   444,   456,   457,   461,   462,   463,   469,
     468,   477,   478,   479,   480,   484,   484,   495,   495,   504,
     505,   511,   511,   518,   520,   521,   521,   526,   527,   531,
     536,   537,   544,   543,   551,   550,   557,   558,   563,   563,
     570,   571,   572,   578,   583,   587,   588,   593,   594,   599,
     600,   601,   602,   603,   604,   605,   606,   607,   608,   609,
     610,   611,   612,   613,   614,   615,   620,   621,   629,   630,
     634,   635,   640,   641,   645,   646,   650,   651,   655,   656,
     657,   658,   659,   663,   664,   665,   666,   667,   671,   672,
     673,   674,   679,   680,   681,   685,   686,   687,   688,   689,
     693,   697,   698,   699,   700,   701,   702,   703,   704,   705,
     706,   707,   708,   709,   710,   711,   712,   713,   714,   715,
     716,   717,   718,   719,   720,   724,   725,   730,   731,   732,
     733,   734,   734,   738,   739,   740,   741,   741,   749,   751,
     754,   755,   756,   757,   758,   759,   760,   761,   762,   763,
     764,   764,   767,   767,   775,   780,   781,   782,   786,   787,
     788,   789,   795,   795,   804,   805,   810,   811,   816,   816,
     826,   828,   829,   834,   835,   842,   844,   851,   851,   861,
     863,   869,   870,   876,   877,   878,   879,   880,   881,   886,
     886,   898,   900,   901,   906,   907,   908,   909,   910,   911
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
  "\"->\"", "\"debugger\"", "\"default\"", "\"case\"", "\"extends\"",
  "\"finally\"", "\"property\"", "\"private\"", "\"public\"",
  "\"protected\"", "\"static\"", "\"return\"", "\"break\"", "\"export\"",
  "\"import\"", "\"switch\"", "\"in\"", "\"incontextof\"", "\"for\"",
  "\"while\"", "\"do\"", "\"if\"", "\"var\"", "\"const\"", "\"enum\"",
  "\"goto\"", "\"throw\"", "\"try\"", "\"setter\"", "\"getter\"",
  "\"else\"", "\"catch\"", "\"...\"", "\"synchronized\"", "\"with\"",
  "\"int\"", "\"real\"", "\"string\"", "\"octet\"", "\"false\"",
  "\"null\"", "\"true\"", "\"void\"", "\"NaN\"", "\"Infinity\"", "T_UPLUS",
  "T_UMINUS", "T_EVAL", "T_POSTDECREMENT", "T_POSTINCREMENT",
  "T_IGNOREPROP", "T_PROPACCESS", "T_ARG", "T_EXPANDARG", "T_INLINEARRAY",
  "T_ARRAYARG", "T_INLINEDIC", "T_DICELM", "T_WITHDOT", "T_THIS_PROXY",
  "T_WITHDOT_PROXY", "T_CONSTVAL", "T_SYMBOL", "T_REGEXP", "T_VARIANT",
  "$accept", "program", "global_list", "$@1", "def_list",
  "block_or_statement", "statement", "block", "$@2", "while", "$@3", "$@4",
  "do_while", "$@5", "$@6", "if", "$@7", "$@8", "if_else", "$@9", "for",
  "for_first_clause", "$@10", "for_second_clause", "for_third_clause",
  "variable_def", "variable_def_inner", "variable_id_list", "variable_id",
  "variable_type", "func_def", "$@11", "func_expr_def", "$@12",
  "func_decl_arg_opt", "func_decl_arg_list", "func_decl_arg_at_least_one",
  "func_decl_arg", "func_decl_arg_collapse", "arrow_expr_def", "$@13",
  "arrow_expr_arg_opt", "arrow_expr_body", "property_def", "$@14",
  "property_handler_def_list", "property_handler_setter", "$@15",
  "property_handler_getter", "$@16", "property_getter_handler_head",
  "class_def", "$@17", "class_extender", "$@18", "extends_list",
  "extends_name", "return", "switch", "$@19", "with", "$@20", "case",
  "try", "$@21", "catch", "throw", "expr_no_comma", "expr", "comma_expr",
  "assign_expr", "cond_expr", "logical_or_expr", "logical_and_expr",
  "inclusive_or_expr", "exclusive_or_expr", "and_expr", "identical_expr",
  "compare_expr", "shift_expr", "add_sub_expr", "mul_div_expr",
  "mul_div_expr_and_asterisk", "unary_expr", "incontextof_expr",
  "priority_expr", "$@22", "$@23", "factor_expr", "$@24", "$@25",
  "func_call_expr", "call_arg_list", "call_arg", "inline_array", "$@26",
  "array_elm_list", "array_elm", "inline_dic", "$@27", "dic_elm_list",
  "dic_elm", "dic_dummy_elm_opt", "const_inline_array", "$@28",
  "const_array_elm_list_opt", "const_array_elm_list", "const_array_elm",
  "const_inline_dic", "$@29", "const_dic_elm_list", "const_dic_elm", 0
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
     385,   386,   387,   388,   389
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   135,   136,   138,   137,   139,   139,   139,   140,   140,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     143,   142,   145,   146,   144,   148,   149,   147,   151,   152,
     150,   154,   153,   155,   156,   157,   156,   156,   158,   158,
     159,   159,   160,   161,   161,   162,   162,   163,   163,   164,
     164,   164,   164,   164,   164,   164,   166,   165,   168,   167,
     169,   169,   169,   169,   170,   170,   171,   171,   172,   172,
     173,   173,   175,   174,   176,   176,   177,   177,   177,   179,
     178,   180,   180,   180,   180,   182,   181,   184,   183,   185,
     185,   187,   186,   188,   188,   189,   188,   190,   190,   191,
     192,   192,   194,   193,   196,   195,   197,   197,   199,   198,
     200,   200,   200,   201,   202,   203,   203,   204,   204,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   206,   206,   207,   207,
     208,   208,   209,   209,   210,   210,   211,   211,   212,   212,
     212,   212,   212,   213,   213,   213,   213,   213,   214,   214,
     214,   214,   215,   215,   215,   216,   216,   216,   216,   216,
     217,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   219,   219,   220,   220,   220,
     220,   221,   220,   220,   220,   220,   222,   220,   223,   223,
     223,   223,   223,   223,   223,   223,   223,   223,   223,   223,
     224,   223,   225,   223,   226,   227,   227,   227,   228,   228,
     228,   228,   230,   229,   231,   231,   232,   232,   234,   233,
     235,   235,   235,   236,   236,   237,   237,   239,   238,   240,
     240,   241,   241,   242,   242,   242,   242,   242,   242,   244,
     243,   245,   245,   245,   246,   246,   246,   246,   246,   246
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
       2,     2,     2,     2,     2,     2,     0,     6,     0,     5,
       0,     3,     3,     5,     0,     1,     1,     3,     2,     4,
       1,     3,     0,     5,     2,     1,     2,     3,     1,     0,
       6,     1,     1,     2,     2,     0,     7,     0,     3,     4,
       2,     0,     5,     0,     2,     0,     5,     1,     3,     1,
       2,     3,     0,     6,     0,     6,     3,     2,     0,     5,
       1,     3,     4,     3,     1,     1,     3,     1,     3,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     5,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       3,     3,     3,     1,     3,     3,     3,     3,     1,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     2,
       2,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     3,     4,
       2,     4,     2,     4,     2,     1,     3,     1,     3,     4,
       1,     0,     4,     2,     2,     2,     0,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     3,     0,     3,     4,     1,     1,     3,     0,     1,
       1,     1,     0,     4,     1,     3,     0,     1,     0,     6,
       0,     1,     3,     3,     3,     0,     1,     0,     7,     0,
       1,     1,     3,     2,     2,     1,     1,     1,     1,     0,
       8,     0,     1,     3,     4,     4,     3,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       3,     0,     2,     5,     1,     0,     0,   230,     0,     0,
     232,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   216,   242,   220,   221,
     224,     0,    10,    30,     0,    68,    82,     0,     0,     0,
       0,     0,     0,     0,     0,    32,    35,     0,     0,     0,
       0,   118,     0,     0,     0,     0,   225,   218,   219,     6,
       8,     9,    14,    15,    12,    13,    16,    20,     0,    21,
     222,   223,    22,    23,    24,    25,    26,    27,    28,    29,
       0,   125,   127,   129,   146,   148,   150,   152,   154,   156,
     158,   163,   168,   172,     0,   175,   181,   205,   207,   210,
     226,   227,   228,   229,     7,     0,    68,   196,   248,     0,
     197,   182,   183,   184,   185,     0,     0,   186,   190,   191,
     194,   195,   192,   193,   188,   187,     0,     0,     0,     0,
       0,     0,   246,   101,     5,    18,    66,    70,    70,    19,
     117,     0,     0,   110,     0,    17,     0,    45,     0,     0,
      38,    59,    53,    55,    54,     0,     0,     0,   200,   202,
     204,    41,    52,    11,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   180,   179,   189,     0,   215,   214,
     213,   238,   211,     0,     0,   231,   250,   233,     0,     0,
       0,     0,   208,   217,   247,   124,     0,   244,   103,     0,
      70,    74,    59,    59,    85,    59,   116,    89,   111,     0,
       0,     0,    47,     0,     0,     0,     0,    57,     0,   123,
       0,     0,     0,   128,   126,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   145,   144,   143,
     130,     0,   149,   151,   153,   155,   157,   159,   160,   161,
     162,   164,   165,   166,   167,   169,   170,   171,   173,   174,
     176,   177,   178,   198,   239,   235,   241,   240,     0,   236,
       0,     0,   206,   219,     0,   255,   251,     0,   257,   199,
     201,   203,   246,   243,     0,     0,    31,    59,    80,    59,
       0,    75,    76,     0,     0,    84,     0,     0,   112,    48,
      46,     0,     0,    39,    62,    63,    64,    65,    61,    60,
       0,    56,   120,     0,   114,    42,     0,   238,   234,   212,
     209,     0,     0,   256,     0,   269,   259,   245,   104,   102,
       0,    78,    72,     0,    71,    69,     0,    88,    83,     0,
      59,     0,    91,    92,    97,     0,     0,    49,    33,     0,
       0,    58,     0,   119,     0,   147,   237,   254,   253,   252,
     249,   271,     0,     0,     0,   266,   265,   267,     0,   260,
     261,   268,   105,    67,     0,    81,    77,     0,    86,     0,
       0,     0,   100,    90,    93,    94,     0,   113,    50,     0,
       0,     0,   121,     0,   115,     0,     0,   272,   264,   263,
     258,     0,     0,    79,    73,    87,    59,    59,    98,     0,
      51,    34,    36,    40,   122,     0,     0,   270,   262,   106,
     107,   109,     0,    99,     0,     0,     0,     0,   277,   276,
     278,   279,   273,     0,    95,    43,    37,   275,   274,   108,
       0,    96
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     3,     5,    59,    60,    61,   134,    62,
     148,   419,    63,   149,   455,    64,   245,   380,    65,   252,
      66,   240,   241,   376,   439,    67,    68,   152,   153,   247,
      69,   230,    70,   137,   232,   320,   321,   322,   323,    71,
     138,   235,   368,    72,   327,   371,   372,   470,   373,   416,
     374,    73,   228,   315,   432,   449,   450,    74,    75,   375,
      76,   384,    77,    78,   156,   343,    79,   224,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,   300,   131,    98,   105,
     109,    99,   298,   299,   100,   132,   226,   227,   101,   216,
     305,   306,   354,   102,   356,   398,   399,   400,   103,   391,
     426,   427
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -353
static const yytype_int16 yypact[] =
{
    -353,    24,  -353,  -353,  -353,   319,   -32,  -353,  1483,   -27,
    -353,  1483,  1483,  1483,  1483,  1483,    59,  1483,  1483,  1483,
    1483,  1483,  1483,  1483,  1483,   610,  -353,  -353,  -353,  -353,
    -353,   -78,  -353,  -353,    -4,   -61,  -353,    10,    18,  1483,
     -26,   707,    44,    65,    78,  -353,  -353,    80,    17,    17,
    1483,  -353,   102,  1483,  1483,  1483,  -353,  -353,  -353,  -353,
    -353,  -353,  -353,  -353,    81,  -353,  -353,  -353,   115,  -353,
    -353,  -353,  -353,  -353,  -353,  -353,  -353,  -353,  -353,  -353,
     118,     4,  -353,   271,     7,   166,   167,   169,   172,   127,
     145,   125,    82,   174,  1483,  -353,    31,   107,  -353,  -353,
    -353,  -353,  -353,  -353,  -353,    60,  -353,  -353,  -353,    64,
    -353,  -353,  -353,  -353,  -353,   804,    69,   187,  -353,  -353,
    -353,  -353,  -353,  -353,  -353,  -353,   136,   901,   998,  1095,
     141,    74,  1483,  -353,  -353,  -353,  -353,   150,   -38,  -353,
    -353,   144,   143,  -353,   151,  -353,  1483,  1192,   160,   513,
    -353,   155,   226,  -353,   226,   170,   513,  1483,  -353,  -353,
    -353,  -353,  -353,  -353,  1483,  1483,  1483,  1483,  1483,  1483,
    1483,  1483,  1483,  1483,  1483,  1483,  1483,  1483,  1483,  1483,
    1483,  1483,  1483,  1483,  1483,  1483,  1483,  1483,  1483,  1483,
    1483,  1483,  1483,  1483,  1483,  1483,  1483,  1483,  1483,  1483,
    1483,  1483,  1483,  1483,  -353,  -353,  -353,  1483,  -353,  -353,
    -353,  1289,  -353,  1483,    59,  -353,  1580,  -353,    -5,  1483,
    1483,  1483,  -353,  -353,  -353,  -353,    15,  -353,   158,   416,
     150,   -31,   155,   155,  -353,   155,  -353,  -353,  -353,   171,
     176,    73,  -353,  1483,   156,  1483,    95,   245,    17,  -353,
     149,   188,   513,  -353,  -353,  -353,  -353,  -353,  -353,  -353,
    -353,  -353,  -353,  -353,  -353,  -353,  -353,  -353,  -353,  -353,
    -353,   194,   166,   167,   169,   172,   127,   145,   145,   145,
     145,   125,   125,   125,   125,    82,    82,    82,   174,   174,
    -353,  -353,  -353,  -353,  1483,  -353,  -353,  1483,     9,  -353,
     128,   198,  -353,   197,   259,   260,  -353,   210,  -353,  -353,
    -353,  -353,  1483,  -353,  1483,   202,  -353,   155,  -353,   155,
     206,   267,  -353,   209,   202,  -353,   -35,    87,  -353,  1483,
    -353,   227,   236,  -353,  -353,  -353,  -353,  -353,  -353,  -353,
    1483,  -353,   237,   513,  -353,  -353,  1483,  1677,  -353,  -353,
    -353,  1483,  1483,  1580,   232,  -353,   -20,  -353,   291,  -353,
     202,    51,  -353,   -31,  -353,  -353,  1386,  -353,  -353,   240,
      72,   229,   199,   201,  -353,   202,   238,  -353,  -353,  1483,
     234,  -353,   -48,  -353,   513,  -353,  -353,  -353,  -353,  -353,
    -353,   177,   178,   179,   208,  -353,  -353,  -353,   244,   304,
    -353,  -353,  -353,  -353,  1483,  -353,  -353,   248,  -353,   251,
     181,   252,  -353,  -353,  -353,  -353,   202,  -353,  1483,   513,
     253,   513,  -353,   254,  -353,   311,    16,  -353,  -353,  -353,
    -353,   -20,  1483,  -353,  -353,  -353,   155,   155,  -353,   255,
    -353,  -353,  -353,  -353,  -353,    25,   177,  -353,  -353,   318,
    -353,  -353,   261,  -353,   513,   257,   192,   195,  -353,  -353,
    -353,  -353,  -353,  1483,  -353,  -353,  -353,  -353,  -353,  -353,
     202,  -353
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -353,  -353,  -353,  -353,   191,  -148,  -353,  -237,  -353,  -353,
    -353,  -353,  -353,  -353,  -353,  -353,  -353,  -353,  -353,  -353,
    -353,  -353,  -353,  -353,  -353,  -353,    86,   279,    83,  -212,
    -353,  -353,  -353,  -353,  -127,  -353,  -353,   -34,   -29,  -353,
    -353,  -353,  -353,  -353,  -353,  -353,   -41,  -353,   -37,  -353,
    -353,  -353,  -353,  -353,  -353,  -353,  -126,  -353,  -353,  -353,
    -353,  -353,  -353,  -353,  -353,  -353,  -353,  -206,   -25,  -353,
    -130,  -179,  -353,   153,   154,   159,   161,   162,    49,    61,
     -28,   -56,  -205,    45,   126,   329,  -353,  -353,  -353,  -353,
    -353,   330,  -353,     1,  -353,  -353,  -353,    38,  -353,  -353,
    -353,    -2,  -353,  -352,  -353,  -353,  -353,   -79,  -343,  -353,
    -353,   -93
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -211
static const yytype_int16 yytable[] =
{
     130,   244,   225,   271,   397,   296,   297,   164,   250,   318,
     304,   234,   347,   401,   141,   422,   144,   231,   312,   446,
     324,   325,   366,   326,     4,   155,   182,   183,   392,   393,
     108,    33,   307,   104,   253,   394,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   268,
     269,   270,   308,   107,   133,   404,   110,   111,   112,   113,
     114,   135,   118,   119,   120,   121,   122,   123,   124,   125,
       7,   136,   348,   456,   457,   139,   313,   447,   359,   397,
     394,   225,   140,   206,   423,   207,   225,   365,   401,   367,
     130,   405,   395,   460,   233,   165,     9,    10,   158,   159,
     160,   319,   461,   317,   345,   360,   142,   361,   358,   145,
     208,   396,   209,   210,   115,    26,    27,    28,    29,    30,
     146,   239,   242,   403,   211,   212,   213,   411,   106,    36,
     199,   200,   251,   147,   381,   150,   246,   458,   417,   205,
     254,   296,   297,   288,   289,   387,   388,   304,   208,   151,
     209,   210,   188,   189,   190,   191,   459,   157,   412,   196,
     197,   198,   211,   212,   213,    48,    49,   385,   285,   286,
     287,    56,   158,   159,   160,   192,   193,   194,   195,   438,
     162,   161,   225,   163,   225,   369,   370,   184,   301,   185,
      57,    58,   186,   215,   214,   383,   187,   217,   433,   218,
     334,   335,   336,   337,   222,   231,   223,   338,   236,   237,
     225,   201,   202,   203,   204,   243,   238,   225,   331,   246,
     333,   225,   225,   225,   452,   453,   451,   339,  -210,   248,
    -210,  -210,   314,   471,   328,   249,   424,   277,   278,   279,
     280,   329,  -210,  -210,  -210,   332,   290,   291,   292,   340,
     342,   344,   293,   281,   282,   283,   284,   451,   346,   350,
     349,   351,   352,   353,   309,   310,   311,   355,    33,   362,
     363,   441,   364,   443,   225,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     378,   379,   382,   390,   402,   410,   413,   421,   370,   369,
     181,   126,   225,   418,   377,   430,   465,   431,   425,   428,
     429,   434,   435,   436,   445,   437,   442,   444,   454,    -4,
       6,   463,   466,   467,   464,   229,   468,   330,   154,   406,
       7,   341,   415,   225,   407,   414,   272,   469,   273,   110,
     302,   409,   205,     8,   274,   116,   117,   275,   386,   276,
     357,   389,   448,   462,   420,     0,     9,    10,     0,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,     0,    25,    26,    27,    28,    29,    30,
       0,    31,     0,     0,    32,    33,     0,    34,    35,    36,
      37,    38,    39,   440,     0,    40,     0,     0,     0,     0,
      41,    42,     0,     0,    43,     0,     0,    44,    45,    46,
      47,    48,    49,     0,     0,    50,    51,     6,     0,     0,
       0,     0,     0,    52,    53,    54,    55,     7,     0,     0,
       0,    56,     0,     0,     0,     0,     0,     0,     0,     0,
       8,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      57,    58,     0,     9,    10,     0,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,    28,    29,    30,     0,    31,     0,
       0,    32,    33,   316,    34,    35,    36,    37,    38,    39,
       0,     0,    40,     0,     0,     0,     0,    41,    42,     0,
       0,    43,     0,     0,    44,    45,    46,    47,    48,    49,
       0,     0,    50,    51,     0,     0,     0,     0,     0,     0,
      52,    53,    54,    55,     7,     0,     0,     0,    56,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    57,    58,     0,
       9,    10,     0,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,     0,    25,    26,
      27,    28,    29,    30,     0,    31,     0,     0,    32,    33,
       0,    34,    35,    36,    37,    38,    39,     0,     0,    40,
       0,     0,     0,     0,    41,    42,     0,     0,    43,     0,
       0,    44,    45,    46,    47,    48,    49,     0,     0,    50,
      51,     0,     0,     0,     0,     0,     0,    52,    53,    54,
      55,     7,     0,     0,     0,    56,     0,     0,     0,     0,
       0,     0,     0,     0,     8,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    57,    58,     0,     9,    10,     0,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,    29,
      30,     0,     0,     0,     0,     0,     0,     0,     0,   106,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   126,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   127,   128,   129,     7,     0,
       0,     0,    56,     0,     0,     0,     0,     0,     0,     0,
       0,     8,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    57,    58,     0,     9,    10,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,    29,    30,     0,     0,
       0,     0,   143,     0,     0,     0,   106,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    53,    54,    55,     7,     0,     0,     0,    56,
       0,     0,     0,     0,     0,     0,     0,     0,     8,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    57,    58,
       0,     9,    10,     0,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,    28,    29,    30,     0,     0,     0,     0,     0,
       0,     0,     0,   106,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   126,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    53,
      54,    55,     7,     0,     0,     0,    56,     0,     0,     0,
       0,     0,     0,     0,     0,     8,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    57,    58,     0,     9,    10,
       0,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,    27,    28,
      29,    30,     0,     0,   219,     0,     0,     0,     0,     0,
     106,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,     7,
       0,     0,     0,    56,     0,     0,     0,     0,     0,     0,
       0,     0,     8,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    57,    58,     0,     9,    10,     0,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,    29,    30,     0,
       0,   220,     0,     0,     0,     0,     0,   106,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    53,    54,    55,     7,     0,     0,     0,
      56,     0,     0,     0,     0,     0,     0,     0,     0,     8,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    57,
      58,     0,     9,    10,     0,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,     0,
      25,    26,    27,    28,    29,    30,     0,     0,   221,     0,
       0,     0,     0,     0,   106,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      53,    54,    55,     7,     0,     0,     0,    56,     0,     0,
       0,     0,     0,     0,     0,     0,     8,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    57,    58,     0,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
      28,    29,    30,     0,     0,     0,     0,   -44,     0,     0,
       0,   106,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    53,    54,    55,
       7,     0,     0,     0,    56,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    57,    58,     0,     9,    10,     0,   294,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,     0,    25,    26,    27,    28,    29,    30,
       0,     0,     0,     0,     0,     0,     0,     0,   106,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   295,     0,     0,    53,    54,    55,     7,     0,     0,
       0,    56,     0,     0,     0,     0,     0,     0,     0,     0,
       8,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      57,    58,     0,     9,    10,     0,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,    28,    29,    30,   408,     0,     0,
       0,     0,     0,     0,     0,   106,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    53,    54,    55,     7,     0,     0,     0,    56,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    57,    58,     0,
       9,    10,     0,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,     0,    25,    26,
      27,    28,    29,    30,     0,     0,     0,     0,     0,     0,
       0,     0,   106,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    53,    54,
      55,     7,     0,     0,     0,    56,     0,     0,     0,     0,
       0,     0,     0,     0,     8,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    57,    58,     0,     9,    10,     0,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,    29,
      30,     0,     0,     0,     0,     0,     0,     0,     0,   106,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    53,    54,    55,     7,     0,
       0,     0,    56,     0,     0,     0,     0,     0,     0,     0,
       0,     8,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    57,   303,     0,     9,    10,     0,   294,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,    29,    30,     0,     0,
       0,     0,     0,     0,     0,     0,   106,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    53,    54,    55,     0,     0,     0,     0,    56,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    57,    58
};

static const yytype_int16 yycheck[] =
{
      25,   149,   132,   182,   356,   211,   211,     3,   156,    40,
     216,   138,     3,   356,    39,    63,    41,    55,     3,     3,
     232,   233,    57,   235,     0,    50,    19,    20,    48,    49,
      57,    66,    37,    65,   164,    55,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,    57,     8,   132,     4,    11,    12,    13,    14,
      15,    65,    17,    18,    19,    20,    21,    22,    23,    24,
      11,   132,    63,    48,    49,    65,    61,    61,   315,   431,
      55,   211,    64,    52,   132,    54,   216,   324,   431,   326,
     115,    40,   112,   445,   132,    91,    37,    38,    53,    54,
      55,   132,   445,   230,   252,   317,   132,   319,   314,    65,
      41,   131,    43,    44,    55,    56,    57,    58,    59,    60,
      55,   146,   147,   360,    55,    56,    57,    55,    69,    70,
      48,    49,   157,    55,   340,    55,    64,   112,   375,    94,
     165,   347,   347,   199,   200,   351,   352,   353,    41,   132,
      43,    44,    25,    26,    27,    28,   131,    55,   370,    34,
      35,    36,    55,    56,    57,    92,    93,   346,   196,   197,
     198,   112,   127,   128,   129,    30,    31,    32,    33,   416,
      65,   100,   312,    65,   314,    98,    99,    21,   213,    22,
     131,   132,    23,   133,    87,   343,    24,   133,   404,    63,
     105,   106,   107,   108,    63,    55,   132,   112,    64,    66,
     340,    37,    38,    39,    40,    55,    65,   347,   243,    64,
     245,   351,   352,   353,   436,   437,   432,   132,    41,     3,
      43,    44,    74,   470,    63,    65,   384,   188,   189,   190,
     191,    65,    55,    56,    57,    89,   201,   202,   203,     4,
     101,    63,   207,   192,   193,   194,   195,   463,    64,    61,
     132,    64,     3,     3,   219,   220,   221,    57,    66,    63,
       3,   419,    63,   421,   404,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      63,    55,    55,    61,     3,    55,    67,    63,    99,    98,
      29,    93,   432,    65,   329,    61,   454,     3,   131,   131,
     131,    63,    61,   132,     3,    63,    63,    63,    63,     0,
       1,     3,    65,   131,    63,   134,   131,   241,    49,   363,
      11,   248,   373,   463,   363,   372,   183,   463,   184,   294,
     214,   366,   297,    24,   185,    16,    16,   186,   347,   187,
     312,   353,   431,   446,   379,    -1,    37,    38,    -1,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    60,
      -1,    62,    -1,    -1,    65,    66,    -1,    68,    69,    70,
      71,    72,    73,   418,    -1,    76,    -1,    -1,    -1,    -1,
      81,    82,    -1,    -1,    85,    -1,    -1,    88,    89,    90,
      91,    92,    93,    -1,    -1,    96,    97,     1,    -1,    -1,
      -1,    -1,    -1,   104,   105,   106,   107,    11,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,    37,    38,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    56,    57,    58,    59,    60,    -1,    62,    -1,
      -1,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    -1,    76,    -1,    -1,    -1,    -1,    81,    82,    -1,
      -1,    85,    -1,    -1,    88,    89,    90,    91,    92,    93,
      -1,    -1,    96,    97,    -1,    -1,    -1,    -1,    -1,    -1,
     104,   105,   106,   107,    11,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    56,
      57,    58,    59,    60,    -1,    62,    -1,    -1,    65,    66,
      -1,    68,    69,    70,    71,    72,    73,    -1,    -1,    76,
      -1,    -1,    -1,    -1,    81,    82,    -1,    -1,    85,    -1,
      -1,    88,    89,    90,    91,    92,    93,    -1,    -1,    96,
      97,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,    11,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   105,   106,   107,    11,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,    37,    38,    -1,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    56,    57,    58,    59,    60,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,   106,   107,    11,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,    37,    38,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      56,    57,    58,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    70,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,
     106,   107,    11,    -1,    -1,    -1,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,    37,    38,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,
      69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,   106,   107,    11,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,    37,    38,    -1,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    -1,
      -1,    63,    -1,    -1,    -1,    -1,    -1,    69,    70,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   105,   106,   107,    11,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,    37,    38,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    56,    57,    58,    59,    60,    -1,    -1,    63,    -1,
      -1,    -1,    -1,    -1,    69,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     105,   106,   107,    11,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,    37,
      38,    -1,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,   106,   107,
      11,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,    37,    38,    -1,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    60,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    70,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   102,    -1,    -1,   105,   106,   107,    11,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     131,   132,    -1,    37,    38,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    56,    57,    58,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,   106,   107,    11,    -1,    -1,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    -1,    55,    56,
      57,    58,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,   106,
     107,    11,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   105,   106,   107,    11,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,    37,    38,    -1,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    -1,    55,    56,    57,    58,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,   106,   107,    -1,    -1,    -1,    -1,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   136,   137,   138,     0,   139,     1,    11,    24,    37,
      38,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    55,    56,    57,    58,    59,
      60,    62,    65,    66,    68,    69,    70,    71,    72,    73,
      76,    81,    82,    85,    88,    89,    90,    91,    92,    93,
      96,    97,   104,   105,   106,   107,   112,   131,   132,   140,
     141,   142,   144,   147,   150,   153,   155,   160,   161,   165,
     167,   174,   178,   186,   192,   193,   195,   197,   198,   201,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   223,   226,
     229,   233,   238,   243,    65,   224,    69,   218,    57,   225,
     218,   218,   218,   218,   218,    55,   220,   226,   218,   218,
     218,   218,   218,   218,   218,   218,    93,   105,   106,   107,
     203,   222,   230,   132,   143,    65,   132,   168,   175,    65,
      64,   203,   132,    65,   203,    65,    55,    55,   145,   148,
      55,   132,   162,   163,   162,   203,   199,    55,   218,   218,
     218,   100,    65,    65,     3,    91,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    29,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    30,    31,    32,    33,    34,    35,    36,    48,
      49,    37,    38,    39,    40,   218,    52,    54,    41,    43,
      44,    55,    56,    57,    87,   133,   234,   133,    63,    63,
      63,    63,    63,   132,   202,   205,   231,   232,   187,   139,
     166,    55,   169,   132,   169,   176,    64,    66,    65,   203,
     156,   157,   203,    55,   140,   151,    64,   164,     3,    65,
     140,   203,   154,   205,   203,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   206,   208,   209,   210,   211,   212,   213,   213,   213,
     213,   214,   214,   214,   214,   215,   215,   215,   216,   216,
     218,   218,   218,   218,    40,   102,   202,   217,   227,   228,
     221,   203,   219,   132,   202,   235,   236,    37,    57,   218,
     218,   218,     3,    61,    74,   188,    67,   169,    40,   132,
     170,   171,   172,   173,   164,   164,   164,   179,    63,    65,
     161,   203,    89,   203,   105,   106,   107,   108,   112,   132,
       4,   163,   101,   200,    63,   140,    64,     3,    63,   132,
      61,    64,     3,     3,   237,    57,   239,   232,   202,   142,
     164,   164,    63,     3,    63,   142,    57,   142,   177,    98,
      99,   180,   181,   183,   185,   194,   158,   203,    63,    55,
     152,   202,    55,   140,   196,   206,   228,   202,   202,   236,
      61,   244,    48,    49,    55,   112,   131,   238,   240,   241,
     242,   243,     3,   142,     4,    40,   172,   173,    61,   203,
      55,    55,   164,    67,   183,   181,   184,   142,    65,   146,
     203,    63,    63,   132,   140,   131,   245,   246,   131,   131,
      61,     3,   189,   202,    63,    61,   132,    63,   142,   159,
     203,   140,    63,   140,    63,     3,     3,    61,   242,   190,
     191,   202,   164,   164,    63,   149,    48,    49,   112,   131,
     238,   243,   246,     3,    63,   140,    65,   131,   131,   191,
     182,   142
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
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





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
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

/* Line 1464 of yacc.c  */
#line 223 "syntax/tjs.y"
    { sb->PushContextStack(TJS_W("global"),
												ctTopLevel); ;}
    break;

  case 4:

/* Line 1464 of yacc.c  */
#line 225 "syntax/tjs.y"
    { sb->PopContextStack(); ;}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 232 "syntax/tjs.y"
    { if(sb->CompileErrorCount>20)
												YYABORT;
											  else yyerrok; ;}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 246 "syntax/tjs.y"
    { cc->CreateExprCode((yyvsp[(1) - (2)].np)); ;}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 252 "syntax/tjs.y"
    { cc->DoBreak(); ;}
    break;

  case 18:

/* Line 1464 of yacc.c  */
#line 253 "syntax/tjs.y"
    { cc->DoContinue(); ;}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 254 "syntax/tjs.y"
    { cc->DoDebugger(); ;}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 269 "syntax/tjs.y"
    { cc->EnterBlock(); ;}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 271 "syntax/tjs.y"
    { cc->ExitBlock(); ;}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 276 "syntax/tjs.y"
    { cc->EnterWhileCode(false); ;}
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 277 "syntax/tjs.y"
    { cc->CreateWhileExprCode((yyvsp[(4) - (5)].np), false); ;}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 278 "syntax/tjs.y"
    { cc->ExitWhileCode(false); ;}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 283 "syntax/tjs.y"
    { cc->EnterWhileCode(true); ;}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 286 "syntax/tjs.y"
    { cc->CreateWhileExprCode((yyvsp[(6) - (7)].np), true); ;}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 287 "syntax/tjs.y"
    { cc->ExitWhileCode(true); ;}
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 292 "syntax/tjs.y"
    { cc->EnterIfCode(); ;}
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 293 "syntax/tjs.y"
    { cc->CreateIfExprCode((yyvsp[(4) - (4)].np)); ;}
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 294 "syntax/tjs.y"
    { cc->ExitIfCode(); ;}
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 299 "syntax/tjs.y"
    { cc->EnterElseCode(); ;}
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 300 "syntax/tjs.y"
    { cc->ExitElseCode(); ;}
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 309 "syntax/tjs.y"
    { cc->ExitForCode(); ;}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 315 "syntax/tjs.y"
    { cc->EnterForCode(); ;}
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 316 "syntax/tjs.y"
    { cc->EnterForCode(); ;}
    break;

  case 47:

/* Line 1464 of yacc.c  */
#line 318 "syntax/tjs.y"
    { cc->EnterForCode();
											  cc->CreateExprCode((yyvsp[(1) - (1)].np)); ;}
    break;

  case 48:

/* Line 1464 of yacc.c  */
#line 324 "syntax/tjs.y"
    { cc->CreateForExprCode(NULL); ;}
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 325 "syntax/tjs.y"
    { cc->CreateForExprCode((yyvsp[(1) - (1)].np)); ;}
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 330 "syntax/tjs.y"
    { cc->SetForThirdExprCode(NULL); ;}
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 331 "syntax/tjs.y"
    { cc->SetForThirdExprCode((yyvsp[(1) - (1)].np)); ;}
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 354 "syntax/tjs.y"
    { cc->AddLocalVariable(
												lx->GetString((yyvsp[(1) - (2)].num))); ;}
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 356 "syntax/tjs.y"
    { cc->InitLocalVariable(
											  lx->GetString((yyvsp[(1) - (4)].num)), (yyvsp[(4) - (4)].np)); ;}
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 373 "syntax/tjs.y"
    { sb->PushContextStack(
												lx->GetString((yyvsp[(2) - (2)].num)),
											  ctFunction);
											  cc->EnterBlock();;}
    break;

  case 67:

/* Line 1464 of yacc.c  */
#line 378 "syntax/tjs.y"
    { cc->ExitBlock(); sb->PopContextStack(); ;}
    break;

  case 68:

/* Line 1464 of yacc.c  */
#line 383 "syntax/tjs.y"
    { sb->PushContextStack(
												TJS_W("(anonymous)"),
											  ctExprFunction);
											  cc->EnterBlock(); ;}
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 388 "syntax/tjs.y"
    { cc->ExitBlock();
											  tTJSVariant v(cc);
											  sb->PopContextStack();
											  (yyval.np) = cc->MakeNP0(T_CONSTVAL);
											  (yyval.np)->SetValue(v); ;}
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 415 "syntax/tjs.y"
    { cc->AddFunctionDeclArg(
												lx->GetString((yyvsp[(1) - (2)].num)), NULL); ;}
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 417 "syntax/tjs.y"
    { cc->AddFunctionDeclArg(
												lx->GetString((yyvsp[(1) - (4)].num)), (yyvsp[(4) - (4)].np)); ;}
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 422 "syntax/tjs.y"
    { cc->AddFunctionDeclArgCollapse(
												NULL); ;}
    break;

  case 81:

/* Line 1464 of yacc.c  */
#line 424 "syntax/tjs.y"
    { cc->AddFunctionDeclArgCollapse(
												lx->GetString((yyvsp[(1) - (3)].num))); ;}
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 444 "syntax/tjs.y"
    { sb->PushContextStack(TJS_W("(anonymous)"),
											  ctExprFunction); cc->EnterBlock(); ;}
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 448 "syntax/tjs.y"
    { cc->ExitBlock();
											  tTJSVariant v(cc);
											  sb->PopContextStack();
											  (yyval.np) = cc->MakeNP0(T_CONSTVAL);
											  (yyval.np)->SetValue(v); ;}
    break;

  case 84:

/* Line 1464 of yacc.c  */
#line 456 "syntax/tjs.y"
    { cc->AddFunctionDeclArg(lx->GetString((yyvsp[(1) - (2)].num)), NULL); ;}
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 461 "syntax/tjs.y"
    { cc->ReturnFromFunc(NULL); ;}
    break;

  case 87:

/* Line 1464 of yacc.c  */
#line 462 "syntax/tjs.y"
    { cc->ReturnFromFunc((yyvsp[(2) - (3)].np)); ;}
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 469 "syntax/tjs.y"
    { sb->PushContextStack(
												lx->GetString((yyvsp[(2) - (3)].num)),
												ctProperty); ;}
    break;

  case 90:

/* Line 1464 of yacc.c  */
#line 473 "syntax/tjs.y"
    { sb->PopContextStack(); ;}
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 484 "syntax/tjs.y"
    { sb->PushContextStack(
												TJS_W("(setter)"),
												ctPropertySetter);
											  cc->EnterBlock();
											  cc->SetPropertyDeclArg(
												lx->GetString((yyvsp[(3) - (5)].num))); ;}
    break;

  case 96:

/* Line 1464 of yacc.c  */
#line 490 "syntax/tjs.y"
    { cc->ExitBlock();
											  sb->PopContextStack(); ;}
    break;

  case 97:

/* Line 1464 of yacc.c  */
#line 495 "syntax/tjs.y"
    { sb->PushContextStack(
												TJS_W("(getter)"),
												ctPropertyGetter);
											  cc->EnterBlock(); ;}
    break;

  case 98:

/* Line 1464 of yacc.c  */
#line 499 "syntax/tjs.y"
    { cc->ExitBlock();
											  sb->PopContextStack(); ;}
    break;

  case 101:

/* Line 1464 of yacc.c  */
#line 511 "syntax/tjs.y"
    { sb->PushContextStack(
												lx->GetString((yyvsp[(2) - (2)].num)),
												ctClass); ;}
    break;

  case 102:

/* Line 1464 of yacc.c  */
#line 515 "syntax/tjs.y"
    { sb->PopContextStack(); ;}
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 520 "syntax/tjs.y"
    { cc->CreateExtendsExprCode((yyvsp[(2) - (2)].np), true); ;}
    break;

  case 105:

/* Line 1464 of yacc.c  */
#line 521 "syntax/tjs.y"
    { cc->CreateExtendsExprCode((yyvsp[(2) - (3)].np), false); ;}
    break;

  case 109:

/* Line 1464 of yacc.c  */
#line 531 "syntax/tjs.y"
    { cc->CreateExtendsExprCode((yyvsp[(1) - (1)].np), false); ;}
    break;

  case 110:

/* Line 1464 of yacc.c  */
#line 536 "syntax/tjs.y"
    { cc->ReturnFromFunc(NULL); ;}
    break;

  case 111:

/* Line 1464 of yacc.c  */
#line 537 "syntax/tjs.y"
    { cc->ReturnFromFunc((yyvsp[(2) - (3)].np)); ;}
    break;

  case 112:

/* Line 1464 of yacc.c  */
#line 544 "syntax/tjs.y"
    { cc->EnterSwitchCode((yyvsp[(3) - (4)].np)); ;}
    break;

  case 113:

/* Line 1464 of yacc.c  */
#line 545 "syntax/tjs.y"
    { cc->ExitSwitchCode(); ;}
    break;

  case 114:

/* Line 1464 of yacc.c  */
#line 551 "syntax/tjs.y"
    { cc->EnterWithCode((yyvsp[(3) - (4)].np)); ;}
    break;

  case 115:

/* Line 1464 of yacc.c  */
#line 552 "syntax/tjs.y"
    { cc->ExitWithCode(); ;}
    break;

  case 116:

/* Line 1464 of yacc.c  */
#line 557 "syntax/tjs.y"
    { cc->ProcessCaseCode((yyvsp[(2) - (3)].np)); ;}
    break;

  case 117:

/* Line 1464 of yacc.c  */
#line 558 "syntax/tjs.y"
    { cc->ProcessCaseCode(NULL); ;}
    break;

  case 118:

/* Line 1464 of yacc.c  */
#line 563 "syntax/tjs.y"
    { cc->EnterTryCode(); ;}
    break;

  case 119:

/* Line 1464 of yacc.c  */
#line 566 "syntax/tjs.y"
    { cc->ExitTryCode(); ;}
    break;

  case 120:

/* Line 1464 of yacc.c  */
#line 570 "syntax/tjs.y"
    { cc->EnterCatchCode(NULL); ;}
    break;

  case 121:

/* Line 1464 of yacc.c  */
#line 571 "syntax/tjs.y"
    { cc->EnterCatchCode(NULL); ;}
    break;

  case 122:

/* Line 1464 of yacc.c  */
#line 572 "syntax/tjs.y"
    { cc->EnterCatchCode(
												lx->GetString((yyvsp[(3) - (4)].num))); ;}
    break;

  case 123:

/* Line 1464 of yacc.c  */
#line 578 "syntax/tjs.y"
    { cc->ProcessThrowCode((yyvsp[(2) - (3)].np)); ;}
    break;

  case 124:

/* Line 1464 of yacc.c  */
#line 583 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 125:

/* Line 1464 of yacc.c  */
#line 587 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 126:

/* Line 1464 of yacc.c  */
#line 588 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_IF, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 127:

/* Line 1464 of yacc.c  */
#line 593 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 128:

/* Line 1464 of yacc.c  */
#line 594 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_COMMA, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 129:

/* Line 1464 of yacc.c  */
#line 599 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 130:

/* Line 1464 of yacc.c  */
#line 600 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_SWAP, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 131:

/* Line 1464 of yacc.c  */
#line 601 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_EQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 132:

/* Line 1464 of yacc.c  */
#line 602 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_AMPERSANDEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 133:

/* Line 1464 of yacc.c  */
#line 603 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_VERTLINEEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 134:

/* Line 1464 of yacc.c  */
#line 604 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_CHEVRONEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 135:

/* Line 1464 of yacc.c  */
#line 605 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_MINUSEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 136:

/* Line 1464 of yacc.c  */
#line 606 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PLUSEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 137:

/* Line 1464 of yacc.c  */
#line 607 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PERCENTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 138:

/* Line 1464 of yacc.c  */
#line 608 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_SLASHEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 139:

/* Line 1464 of yacc.c  */
#line 609 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_BACKSLASHEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 140:

/* Line 1464 of yacc.c  */
#line 610 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_ASTERISKEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 141:

/* Line 1464 of yacc.c  */
#line 611 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALOREQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 142:

/* Line 1464 of yacc.c  */
#line 612 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALANDEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 143:

/* Line 1464 of yacc.c  */
#line 613 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RARITHSHIFTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 144:

/* Line 1464 of yacc.c  */
#line 614 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LARITHSHIFTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 145:

/* Line 1464 of yacc.c  */
#line 615 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RBITSHIFTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 146:

/* Line 1464 of yacc.c  */
#line 620 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 147:

/* Line 1464 of yacc.c  */
#line 623 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP3(T_QUESTION, (yyvsp[(1) - (5)].np), (yyvsp[(3) - (5)].np), (yyvsp[(5) - (5)].np)); ;}
    break;

  case 148:

/* Line 1464 of yacc.c  */
#line 629 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 149:

/* Line 1464 of yacc.c  */
#line 630 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALOR, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 150:

/* Line 1464 of yacc.c  */
#line 634 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 151:

/* Line 1464 of yacc.c  */
#line 636 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALAND, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 152:

/* Line 1464 of yacc.c  */
#line 640 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 153:

/* Line 1464 of yacc.c  */
#line 641 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_VERTLINE, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 154:

/* Line 1464 of yacc.c  */
#line 645 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 155:

/* Line 1464 of yacc.c  */
#line 646 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_CHEVRON, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 156:

/* Line 1464 of yacc.c  */
#line 650 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 157:

/* Line 1464 of yacc.c  */
#line 651 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_AMPERSAND, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 158:

/* Line 1464 of yacc.c  */
#line 655 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 159:

/* Line 1464 of yacc.c  */
#line 656 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_NOTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 160:

/* Line 1464 of yacc.c  */
#line 657 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_EQUALEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 161:

/* Line 1464 of yacc.c  */
#line 658 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_DISCNOTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 162:

/* Line 1464 of yacc.c  */
#line 659 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_DISCEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 163:

/* Line 1464 of yacc.c  */
#line 663 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 164:

/* Line 1464 of yacc.c  */
#line 664 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 165:

/* Line 1464 of yacc.c  */
#line 665 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_GT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 166:

/* Line 1464 of yacc.c  */
#line 666 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LTOREQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 167:

/* Line 1464 of yacc.c  */
#line 667 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_GTOREQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 168:

/* Line 1464 of yacc.c  */
#line 671 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 169:

/* Line 1464 of yacc.c  */
#line 672 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RARITHSHIFT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 170:

/* Line 1464 of yacc.c  */
#line 673 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LARITHSHIFT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 171:

/* Line 1464 of yacc.c  */
#line 674 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RBITSHIFT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 172:

/* Line 1464 of yacc.c  */
#line 679 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 173:

/* Line 1464 of yacc.c  */
#line 680 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PLUS, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 174:

/* Line 1464 of yacc.c  */
#line 681 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_MINUS, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 175:

/* Line 1464 of yacc.c  */
#line 685 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 176:

/* Line 1464 of yacc.c  */
#line 686 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PERCENT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 177:

/* Line 1464 of yacc.c  */
#line 687 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_SLASH, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 178:

/* Line 1464 of yacc.c  */
#line 688 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_BACKSLASH, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 179:

/* Line 1464 of yacc.c  */
#line 689 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_ASTERISK, (yyvsp[(1) - (2)].np), (yyvsp[(2) - (2)].np)); ;}
    break;

  case 180:

/* Line 1464 of yacc.c  */
#line 693 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (2)].np); ;}
    break;

  case 181:

/* Line 1464 of yacc.c  */
#line 697 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 182:

/* Line 1464 of yacc.c  */
#line 698 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EXCRAMATION, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 183:

/* Line 1464 of yacc.c  */
#line 699 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_TILDE, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 184:

/* Line 1464 of yacc.c  */
#line 700 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_DECREMENT, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 185:

/* Line 1464 of yacc.c  */
#line 701 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INCREMENT, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 186:

/* Line 1464 of yacc.c  */
#line 702 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(2) - (2)].np); (yyval.np)->SetOpecode(T_NEW); ;}
    break;

  case 187:

/* Line 1464 of yacc.c  */
#line 703 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INVALIDATE, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 188:

/* Line 1464 of yacc.c  */
#line 704 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ISVALID, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 189:

/* Line 1464 of yacc.c  */
#line 705 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ISVALID, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 190:

/* Line 1464 of yacc.c  */
#line 706 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_DELETE, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 191:

/* Line 1464 of yacc.c  */
#line 707 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_TYPEOF, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 192:

/* Line 1464 of yacc.c  */
#line 708 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_SHARP, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 193:

/* Line 1464 of yacc.c  */
#line 709 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_DOLLAR, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 194:

/* Line 1464 of yacc.c  */
#line 710 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_UPLUS, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 195:

/* Line 1464 of yacc.c  */
#line 711 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_UMINUS, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 196:

/* Line 1464 of yacc.c  */
#line 712 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_IGNOREPROP, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 197:

/* Line 1464 of yacc.c  */
#line 713 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_PROPACCESS, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 198:

/* Line 1464 of yacc.c  */
#line 714 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_INSTANCEOF, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 199:

/* Line 1464 of yacc.c  */
#line 715 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INT, (yyvsp[(4) - (4)].np)); ;}
    break;

  case 200:

/* Line 1464 of yacc.c  */
#line 716 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INT, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 201:

/* Line 1464 of yacc.c  */
#line 717 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_REAL, (yyvsp[(4) - (4)].np)); ;}
    break;

  case 202:

/* Line 1464 of yacc.c  */
#line 718 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_REAL, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 203:

/* Line 1464 of yacc.c  */
#line 719 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_STRING, (yyvsp[(4) - (4)].np)); ;}
    break;

  case 204:

/* Line 1464 of yacc.c  */
#line 720 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_STRING, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 205:

/* Line 1464 of yacc.c  */
#line 724 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 206:

/* Line 1464 of yacc.c  */
#line 726 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_INCONTEXTOF, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 207:

/* Line 1464 of yacc.c  */
#line 730 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 208:

/* Line 1464 of yacc.c  */
#line 731 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(2) - (3)].np); ;}
    break;

  case 209:

/* Line 1464 of yacc.c  */
#line 732 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LBRACKET, (yyvsp[(1) - (4)].np), (yyvsp[(3) - (4)].np)); ;}
    break;

  case 210:

/* Line 1464 of yacc.c  */
#line 733 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 211:

/* Line 1464 of yacc.c  */
#line 734 "syntax/tjs.y"
    { lx->SetNextIsBareWord(); ;}
    break;

  case 212:

/* Line 1464 of yacc.c  */
#line 735 "syntax/tjs.y"
    { tTJSExprNode * node = cc->MakeNP0(T_CONSTVAL);
												  node->SetValue(lx->GetValue((yyvsp[(4) - (4)].num)));
												  (yyval.np) = cc->MakeNP2(T_DOT, (yyvsp[(1) - (4)].np), node); ;}
    break;

  case 213:

/* Line 1464 of yacc.c  */
#line 738 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_POSTINCREMENT, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 214:

/* Line 1464 of yacc.c  */
#line 739 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_POSTDECREMENT, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 215:

/* Line 1464 of yacc.c  */
#line 740 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EVAL, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 216:

/* Line 1464 of yacc.c  */
#line 741 "syntax/tjs.y"
    { lx->SetNextIsBareWord(); ;}
    break;

  case 217:

/* Line 1464 of yacc.c  */
#line 742 "syntax/tjs.y"
    { tTJSExprNode * node = cc->MakeNP0(T_CONSTVAL);
												  node->SetValue(lx->GetValue((yyvsp[(3) - (3)].num)));
												  (yyval.np) = cc->MakeNP1(T_WITHDOT, node); ;}
    break;

  case 218:

/* Line 1464 of yacc.c  */
#line 749 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_CONSTVAL);
												  (yyval.np)->SetValue(lx->GetValue((yyvsp[(1) - (1)].num))); ;}
    break;

  case 219:

/* Line 1464 of yacc.c  */
#line 751 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_SYMBOL);
												  (yyval.np)->SetValue(tTJSVariant(
													lx->GetString((yyvsp[(1) - (1)].num)))); ;}
    break;

  case 220:

/* Line 1464 of yacc.c  */
#line 754 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_THIS); ;}
    break;

  case 221:

/* Line 1464 of yacc.c  */
#line 755 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_SUPER); ;}
    break;

  case 222:

/* Line 1464 of yacc.c  */
#line 756 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 223:

/* Line 1464 of yacc.c  */
#line 757 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 224:

/* Line 1464 of yacc.c  */
#line 758 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_GLOBAL); ;}
    break;

  case 225:

/* Line 1464 of yacc.c  */
#line 759 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_VOID); ;}
    break;

  case 226:

/* Line 1464 of yacc.c  */
#line 760 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 227:

/* Line 1464 of yacc.c  */
#line 761 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 228:

/* Line 1464 of yacc.c  */
#line 762 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 229:

/* Line 1464 of yacc.c  */
#line 763 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 230:

/* Line 1464 of yacc.c  */
#line 764 "syntax/tjs.y"
    { lx->SetStartOfRegExp(); ;}
    break;

  case 231:

/* Line 1464 of yacc.c  */
#line 765 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_REGEXP);
												  (yyval.np)->SetValue(lx->GetValue((yyvsp[(3) - (3)].num))); ;}
    break;

  case 232:

/* Line 1464 of yacc.c  */
#line 767 "syntax/tjs.y"
    { lx->SetStartOfRegExp(); ;}
    break;

  case 233:

/* Line 1464 of yacc.c  */
#line 768 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_REGEXP);
												  (yyval.np)->SetValue(lx->GetValue((yyvsp[(3) - (3)].num))); ;}
    break;

  case 234:

/* Line 1464 of yacc.c  */
#line 775 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LPARENTHESIS, (yyvsp[(1) - (4)].np), (yyvsp[(3) - (4)].np)); ;}
    break;

  case 235:

/* Line 1464 of yacc.c  */
#line 780 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_OMIT); ;}
    break;

  case 236:

/* Line 1464 of yacc.c  */
#line 781 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ARG, (yyvsp[(1) - (1)].np)); ;}
    break;

  case 237:

/* Line 1464 of yacc.c  */
#line 782 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_ARG, (yyvsp[(3) - (3)].np), (yyvsp[(1) - (3)].np)); ;}
    break;

  case 238:

/* Line 1464 of yacc.c  */
#line 786 "syntax/tjs.y"
    { (yyval.np) = NULL; ;}
    break;

  case 239:

/* Line 1464 of yacc.c  */
#line 787 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EXPANDARG, NULL); ;}
    break;

  case 240:

/* Line 1464 of yacc.c  */
#line 788 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EXPANDARG, (yyvsp[(1) - (1)].np)); ;}
    break;

  case 241:

/* Line 1464 of yacc.c  */
#line 789 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 242:

/* Line 1464 of yacc.c  */
#line 795 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_INLINEARRAY);
										  cc->PushCurrentNode(node); ;}
    break;

  case 243:

/* Line 1464 of yacc.c  */
#line 799 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 244:

/* Line 1464 of yacc.c  */
#line 804 "syntax/tjs.y"
    { cn->Add((yyvsp[(1) - (1)].np)); ;}
    break;

  case 245:

/* Line 1464 of yacc.c  */
#line 805 "syntax/tjs.y"
    { cn->Add((yyvsp[(3) - (3)].np)); ;}
    break;

  case 246:

/* Line 1464 of yacc.c  */
#line 810 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ARRAYARG, NULL); ;}
    break;

  case 247:

/* Line 1464 of yacc.c  */
#line 811 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ARRAYARG, (yyvsp[(1) - (1)].np)); ;}
    break;

  case 248:

/* Line 1464 of yacc.c  */
#line 816 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_INLINEDIC);
										  cc->PushCurrentNode(node); ;}
    break;

  case 249:

/* Line 1464 of yacc.c  */
#line 821 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 251:

/* Line 1464 of yacc.c  */
#line 828 "syntax/tjs.y"
    { cn->Add((yyvsp[(1) - (1)].np)); ;}
    break;

  case 252:

/* Line 1464 of yacc.c  */
#line 829 "syntax/tjs.y"
    { cn->Add((yyvsp[(3) - (3)].np)); ;}
    break;

  case 253:

/* Line 1464 of yacc.c  */
#line 834 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_DICELM, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 254:

/* Line 1464 of yacc.c  */
#line 835 "syntax/tjs.y"
    { tTJSVariant val(lx->GetString((yyvsp[(1) - (3)].num)));
										  tTJSExprNode *node0 = cc->MakeNP0(T_CONSTVAL);
										  node0->SetValue(val);
										  (yyval.np) = cc->MakeNP2(T_DICELM, node0, (yyvsp[(3) - (3)].np)); ;}
    break;

  case 257:

/* Line 1464 of yacc.c  */
#line 851 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_CONSTVAL);
										  iTJSDispatch2 * dsp = TJSCreateArrayObject();
										  node->SetValue(tTJSVariant(dsp, dsp));
										  dsp->Release();
										  cc->PushCurrentNode(node); ;}
    break;

  case 258:

/* Line 1464 of yacc.c  */
#line 858 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 263:

/* Line 1464 of yacc.c  */
#line 876 "syntax/tjs.y"
    { cn->AddArrayElement(- lx->GetValue((yyvsp[(2) - (2)].num))); ;}
    break;

  case 264:

/* Line 1464 of yacc.c  */
#line 877 "syntax/tjs.y"
    { cn->AddArrayElement(+ lx->GetValue((yyvsp[(2) - (2)].num))); ;}
    break;

  case 265:

/* Line 1464 of yacc.c  */
#line 878 "syntax/tjs.y"
    { cn->AddArrayElement(lx->GetValue((yyvsp[(1) - (1)].num))); ;}
    break;

  case 266:

/* Line 1464 of yacc.c  */
#line 879 "syntax/tjs.y"
    { cn->AddArrayElement(tTJSVariant());  ;}
    break;

  case 267:

/* Line 1464 of yacc.c  */
#line 880 "syntax/tjs.y"
    { cn->AddArrayElement((yyvsp[(1) - (1)].np)->GetValue()); ;}
    break;

  case 268:

/* Line 1464 of yacc.c  */
#line 881 "syntax/tjs.y"
    { cn->AddArrayElement((yyvsp[(1) - (1)].np)->GetValue()); ;}
    break;

  case 269:

/* Line 1464 of yacc.c  */
#line 886 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_CONSTVAL);
										  iTJSDispatch2 * dsp = TJSCreateDictionaryObject();
										  node->SetValue(tTJSVariant(dsp, dsp));
										  dsp->Release();
										  cc->PushCurrentNode(node); ;}
    break;

  case 270:

/* Line 1464 of yacc.c  */
#line 893 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 274:

/* Line 1464 of yacc.c  */
#line 906 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (4)].num)), - lx->GetValue((yyvsp[(4) - (4)].num))); ;}
    break;

  case 275:

/* Line 1464 of yacc.c  */
#line 907 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (4)].num)), + lx->GetValue((yyvsp[(4) - (4)].num))); ;}
    break;

  case 276:

/* Line 1464 of yacc.c  */
#line 908 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), lx->GetValue((yyvsp[(3) - (3)].num))); ;}
    break;

  case 277:

/* Line 1464 of yacc.c  */
#line 909 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), tTJSVariant()); ;}
    break;

  case 278:

/* Line 1464 of yacc.c  */
#line 910 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), (yyvsp[(3) - (3)].np)->GetValue()); ;}
    break;

  case 279:

/* Line 1464 of yacc.c  */
#line 911 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), (yyvsp[(3) - (3)].np)->GetValue()); ;}
    break;



/* Line 1464 of yacc.c  */
#line 3854 "tjs.tab.cpp"
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
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
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



/* Line 1684 of yacc.c  */
#line 916 "syntax/tjs.y"



}
