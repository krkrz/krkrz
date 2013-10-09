#include "tjsCommHead.h"

/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.1"

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
#line 129 "tjs.tab.cpp"

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
     T_REGEXP = 387
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
#line 304 "tjs.tab.cpp"
} YYSTYPE;
YYLEX_PROTO_DECL

# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 316 "tjs.tab.cpp"

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
# if YYENABLE_NLS
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
#define YYLAST   1522

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  133
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  107
/* YYNRULES -- Number of rules.  */
#define YYNRULES  264
/* YYNRULES -- Number of states.  */
#define YYNSTATES  444

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   387

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
     125,   126,   127,   128,   129,   130,   131,   132
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
     129,   130,   132,   135,   138,   141,   143,   147,   149,   153,
     154,   160,   161,   166,   167,   171,   175,   181,   182,   184,
     186,   190,   192,   196,   198,   201,   202,   209,   211,   213,
     216,   219,   220,   227,   228,   232,   236,   238,   239,   245,
     246,   249,   250,   256,   258,   262,   264,   267,   271,   272,
     279,   280,   287,   291,   294,   295,   301,   303,   307,   312,
     316,   318,   320,   324,   326,   330,   332,   336,   340,   344,
     348,   352,   356,   360,   364,   368,   372,   376,   380,   384,
     388,   392,   396,   398,   404,   406,   410,   412,   416,   418,
     422,   424,   428,   430,   434,   436,   440,   444,   448,   452,
     454,   458,   462,   466,   470,   472,   476,   480,   484,   486,
     490,   494,   496,   500,   504,   508,   511,   514,   516,   519,
     522,   525,   528,   531,   534,   537,   540,   543,   546,   549,
     552,   555,   558,   561,   564,   568,   573,   576,   581,   584,
     589,   592,   594,   598,   600,   604,   609,   611,   612,   617,
     620,   623,   626,   627,   631,   633,   635,   637,   639,   641,
     643,   645,   647,   649,   651,   653,   654,   658,   659,   663,
     668,   670,   672,   676,   677,   679,   681,   683,   684,   689,
     691,   695,   696,   698,   699,   706,   707,   709,   713,   717,
     721,   722,   724,   725,   733,   734,   736,   738,   742,   745,
     748,   750,   752,   754,   756,   757,   766,   767,   769,   773,
     778,   783,   787,   791,   795
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     134,     0,    -1,   135,    -1,    -1,   136,   137,    -1,    -1,
     137,   138,    -1,   137,     1,    65,    -1,   139,    -1,   140,
      -1,    65,    -1,   196,    65,    -1,   148,    -1,   151,    -1,
     142,    -1,   145,    -1,   153,    -1,    81,    65,    -1,    68,
      65,    -1,    70,    65,    -1,   158,    -1,   162,    -1,   171,
      -1,   179,    -1,   185,    -1,   186,    -1,   188,    -1,   190,
      -1,   191,    -1,   194,    -1,    -1,    66,   141,   137,    67,
      -1,    -1,    -1,    88,   143,    55,   196,    63,   144,   138,
      -1,    -1,    -1,    89,   146,   138,    88,    55,   196,    63,
     147,    65,    -1,    -1,    -1,    90,    55,   149,   196,   150,
      63,   138,    -1,    -1,   148,    99,   152,   138,    -1,    87,
      55,   154,    65,   156,    65,   157,    63,   138,    -1,    -1,
      -1,   155,   159,    -1,   196,    -1,    -1,   196,    -1,    -1,
     196,    -1,   159,    65,    -1,    91,   160,    -1,    92,   160,
      -1,   161,    -1,   160,     3,   161,    -1,   131,    -1,   131,
       4,   195,    -1,    -1,    69,   131,   163,   166,   140,    -1,
      -1,    69,   165,   166,   140,    -1,    -1,    55,   170,    63,
      -1,    55,   167,    63,    -1,    55,   168,     3,   170,    63,
      -1,    -1,   168,    -1,   169,    -1,   168,     3,   169,    -1,
     131,    -1,   131,     4,   195,    -1,    40,    -1,   131,    40,
      -1,    -1,    75,   131,    66,   172,   173,    67,    -1,   174,
      -1,   176,    -1,   174,   176,    -1,   176,   174,    -1,    -1,
      97,    55,   131,    63,   175,   140,    -1,    -1,   178,   177,
     140,    -1,    98,    55,    63,    -1,    98,    -1,    -1,    62,
     131,   180,   181,   140,    -1,    -1,    73,   195,    -1,    -1,
      73,   195,     3,   182,   183,    -1,   184,    -1,   183,     3,
     184,    -1,   195,    -1,    80,    65,    -1,    80,   196,    65,
      -1,    -1,    84,    55,   196,    63,   187,   140,    -1,    -1,
     103,    55,   196,    63,   189,   138,    -1,    72,   196,    64,
      -1,    71,    64,    -1,    -1,    96,   192,   138,   193,   138,
      -1,   100,    -1,   100,    55,    63,    -1,   100,    55,   131,
      63,    -1,    95,   196,    65,    -1,   198,    -1,   197,    -1,
     197,    90,   196,    -1,   198,    -1,   197,     3,   198,    -1,
     199,    -1,   199,    29,   198,    -1,   199,     4,   198,    -1,
     199,     5,   198,    -1,   199,     6,   198,    -1,   199,     7,
     198,    -1,   199,     8,   198,    -1,   199,     9,   198,    -1,
     199,    10,   198,    -1,   199,    11,   198,    -1,   199,    12,
     198,    -1,   199,    13,   198,    -1,   199,    14,   198,    -1,
     199,    15,   198,    -1,   199,    18,   198,    -1,   199,    17,
     198,    -1,   199,    16,   198,    -1,   200,    -1,   200,    19,
     199,    64,   199,    -1,   201,    -1,   200,    20,   201,    -1,
     202,    -1,   201,    21,   202,    -1,   203,    -1,   202,    22,
     203,    -1,   204,    -1,   203,    23,   204,    -1,   205,    -1,
     204,    24,   205,    -1,   206,    -1,   205,    25,   206,    -1,
     205,    26,   206,    -1,   205,    27,   206,    -1,   205,    28,
     206,    -1,   207,    -1,   206,    30,   207,    -1,   206,    31,
     207,    -1,   206,    32,   207,    -1,   206,    33,   207,    -1,
     208,    -1,   207,    34,   208,    -1,   207,    35,   208,    -1,
     207,    36,   208,    -1,   209,    -1,   208,    48,   209,    -1,
     208,    49,   209,    -1,   211,    -1,   209,    37,   211,    -1,
     209,    38,   211,    -1,   209,    39,   211,    -1,   210,   211,
      -1,   209,    40,    -1,   212,    -1,    41,   211,    -1,    42,
     211,    -1,    43,   211,    -1,    44,   211,    -1,    45,   219,
      -1,    53,   211,    -1,    52,   211,    -1,   212,    52,    -1,
      46,   211,    -1,    47,   211,    -1,    50,   211,    -1,    51,
     211,    -1,    48,   211,    -1,    49,   211,    -1,    24,   211,
      -1,    40,   211,    -1,   212,    54,   211,    -1,    55,   104,
      63,   211,    -1,   104,   211,    -1,    55,   105,    63,   211,
      -1,   105,   211,    -1,    55,   106,    63,   211,    -1,   106,
     211,    -1,   213,    -1,   213,    86,   212,    -1,   216,    -1,
      55,   196,    63,    -1,   213,    57,   196,    61,    -1,   219,
      -1,    -1,   213,    56,   214,   131,    -1,   213,    44,    -1,
     213,    43,    -1,   213,    41,    -1,    -1,    56,   215,   131,
      -1,   130,    -1,   131,    -1,    58,    -1,    59,    -1,   164,
      -1,    60,    -1,   111,    -1,   222,    -1,   226,    -1,   231,
      -1,   236,    -1,    -1,    11,   217,   132,    -1,    -1,    38,
     218,   132,    -1,   213,    55,   220,    63,    -1,   101,    -1,
     221,    -1,   220,     3,   221,    -1,    -1,    40,    -1,   210,
      -1,   195,    -1,    -1,    57,   223,   224,    61,    -1,   225,
      -1,   224,     3,   225,    -1,    -1,   195,    -1,    -1,    37,
      57,   227,   228,   230,    61,    -1,    -1,   229,    -1,   228,
       3,   229,    -1,   195,     3,   195,    -1,   131,    64,   195,
      -1,    -1,     3,    -1,    -1,    55,    92,    63,    57,   232,
     233,    61,    -1,    -1,   234,    -1,   235,    -1,   234,     3,
     235,    -1,    49,   130,    -1,    48,   130,    -1,   130,    -1,
     111,    -1,   231,    -1,   236,    -1,    -1,    55,    92,    63,
      37,    57,   237,   238,    61,    -1,    -1,   239,    -1,   238,
       3,   239,    -1,   130,     3,    49,   130,    -1,   130,     3,
      48,   130,    -1,   130,     3,   130,    -1,   130,     3,   111,
      -1,   130,     3,   231,    -1,   130,     3,   236,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   216,   216,   221,   221,   227,   229,   230,   237,   238,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     267,   267,   274,   275,   274,   281,   284,   281,   290,   291,
     290,   297,   297,   303,   313,   314,   314,   316,   322,   323,
     328,   329,   334,   338,   339,   346,   347,   352,   354,   360,
     360,   370,   370,   383,   385,   386,   387,   391,   393,   397,
     398,   402,   404,   409,   411,   423,   422,   431,   432,   433,
     434,   438,   438,   449,   449,   458,   459,   465,   465,   472,
     474,   475,   475,   480,   481,   485,   490,   491,   498,   497,
     505,   504,   511,   512,   517,   517,   524,   525,   526,   532,
     537,   541,   542,   547,   548,   553,   554,   555,   556,   557,
     558,   559,   560,   561,   562,   563,   564,   565,   566,   567,
     568,   569,   574,   575,   583,   584,   588,   589,   594,   595,
     599,   600,   604,   605,   609,   610,   611,   612,   613,   617,
     618,   619,   620,   621,   625,   626,   627,   628,   633,   634,
     635,   639,   640,   641,   642,   643,   647,   651,   652,   653,
     654,   655,   656,   657,   658,   659,   660,   661,   662,   663,
     664,   665,   666,   667,   668,   669,   670,   671,   672,   673,
     674,   678,   679,   684,   685,   686,   687,   688,   688,   692,
     693,   694,   695,   695,   703,   705,   708,   709,   710,   711,
     712,   713,   714,   715,   716,   717,   717,   720,   720,   728,
     733,   734,   735,   739,   740,   741,   742,   748,   748,   757,
     758,   763,   764,   769,   769,   779,   781,   782,   787,   788,
     795,   797,   804,   804,   814,   816,   822,   823,   829,   830,
     831,   832,   833,   834,   839,   839,   851,   853,   854,   859,
     860,   861,   862,   863,   864
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
  "T_WITHDOT_PROXY", "T_CONSTVAL", "T_SYMBOL", "T_REGEXP", "$accept",
  "program", "global_list", "$@1", "def_list", "block_or_statement",
  "statement", "block", "$@2", "while", "$@3", "$@4", "do_while", "$@5",
  "$@6", "if", "$@7", "$@8", "if_else", "$@9", "for", "for_first_clause",
  "$@10", "for_second_clause", "for_third_clause", "variable_def",
  "variable_def_inner", "variable_id_list", "variable_id", "func_def",
  "$@11", "func_expr_def", "$@12", "func_decl_arg_opt",
  "func_decl_arg_list", "func_decl_arg_at_least_one", "func_decl_arg",
  "func_decl_arg_collapse", "property_def", "$@13",
  "property_handler_def_list", "property_handler_setter", "$@14",
  "property_handler_getter", "$@15", "property_getter_handler_head",
  "class_def", "$@16", "class_extender", "$@17", "extends_list",
  "extends_name", "return", "switch", "$@18", "with", "$@19", "case",
  "try", "$@20", "catch", "throw", "expr_no_comma", "expr", "comma_expr",
  "assign_expr", "cond_expr", "logical_or_expr", "logical_and_expr",
  "inclusive_or_expr", "exclusive_or_expr", "and_expr", "identical_expr",
  "compare_expr", "shift_expr", "add_sub_expr", "mul_div_expr",
  "mul_div_expr_and_asterisk", "unary_expr", "incontextof_expr",
  "priority_expr", "$@21", "$@22", "factor_expr", "$@23", "$@24",
  "func_call_expr", "call_arg_list", "call_arg", "inline_array", "$@25",
  "array_elm_list", "array_elm", "inline_dic", "$@26", "dic_elm_list",
  "dic_elm", "dic_dummy_elm_opt", "const_inline_array", "$@27",
  "const_array_elm_list_opt", "const_array_elm_list", "const_array_elm",
  "const_inline_dic", "$@28", "const_dic_elm_list", "const_dic_elm", 0
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
     385,   386,   387
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   133,   134,   136,   135,   137,   137,   137,   138,   138,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     141,   140,   143,   144,   142,   146,   147,   145,   149,   150,
     148,   152,   151,   153,   154,   155,   154,   154,   156,   156,
     157,   157,   158,   159,   159,   160,   160,   161,   161,   163,
     162,   165,   164,   166,   166,   166,   166,   167,   167,   168,
     168,   169,   169,   170,   170,   172,   171,   173,   173,   173,
     173,   175,   174,   177,   176,   178,   178,   180,   179,   181,
     181,   182,   181,   183,   183,   184,   185,   185,   187,   186,
     189,   188,   190,   190,   192,   191,   193,   193,   193,   194,
     195,   196,   196,   197,   197,   198,   198,   198,   198,   198,
     198,   198,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   198,   199,   199,   200,   200,   201,   201,   202,   202,
     203,   203,   204,   204,   205,   205,   205,   205,   205,   206,
     206,   206,   206,   206,   207,   207,   207,   207,   208,   208,
     208,   209,   209,   209,   209,   209,   210,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   212,   212,   213,   213,   213,   213,   214,   213,   213,
     213,   213,   215,   213,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   217,   216,   218,   216,   219,
     220,   220,   220,   221,   221,   221,   221,   223,   222,   224,
     224,   225,   225,   227,   226,   228,   228,   228,   229,   229,
     230,   230,   232,   231,   233,   233,   234,   234,   235,   235,
     235,   235,   235,   235,   237,   236,   238,   238,   238,   239,
     239,   239,   239,   239,   239
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     0,     2,     3,     1,     1,
       1,     2,     1,     1,     1,     1,     1,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     4,     0,     0,     7,     0,     0,     9,     0,     0,
       7,     0,     4,     9,     0,     0,     2,     1,     0,     1,
       0,     1,     2,     2,     2,     1,     3,     1,     3,     0,
       5,     0,     4,     0,     3,     3,     5,     0,     1,     1,
       3,     1,     3,     1,     2,     0,     6,     1,     1,     2,
       2,     0,     6,     0,     3,     3,     1,     0,     5,     0,
       2,     0,     5,     1,     3,     1,     2,     3,     0,     6,
       0,     6,     3,     2,     0,     5,     1,     3,     4,     3,
       1,     1,     3,     1,     3,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     5,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     3,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     3,     1,     3,
       3,     1,     3,     3,     3,     2,     2,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     3,     4,     2,     4,     2,     4,
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
       3,     0,     2,     5,     1,     0,     0,   215,     0,     0,
     217,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   202,   227,   206,   207,
     209,     0,    10,    30,     0,    61,     0,     0,     0,     0,
       0,     0,     0,     0,    32,    35,     0,     0,     0,     0,
     104,     0,     0,     0,     0,   210,   204,   205,     6,     8,
       9,    14,    15,    12,    13,    16,    20,     0,    21,   208,
      22,    23,    24,    25,    26,    27,    28,    29,     0,   111,
     113,   115,   132,   134,   136,   138,   140,   142,   144,   149,
     154,   158,     0,   161,   167,   191,   193,   196,   211,   212,
     213,   214,     7,     0,    61,   182,   233,     0,   183,   168,
     169,   170,   171,     0,     0,   172,   176,   177,   180,   181,
     178,   179,   174,   173,     0,     0,     0,     0,     0,     0,
     231,    87,     5,    18,    59,    63,    19,   103,     0,     0,
      96,     0,    17,     0,    45,     0,     0,    38,    57,    53,
      55,    54,     0,     0,     0,   186,   188,   190,    41,    52,
      11,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   166,   165,   175,     0,   201,   200,   199,   223,   197,
       0,     0,   216,   235,   218,     0,     0,     0,     0,   194,
     203,   232,   110,     0,   229,    89,     0,    63,    67,     0,
     102,    75,    97,     0,     0,     0,    47,     0,     0,     0,
       0,     0,   109,     0,     0,     0,   114,   112,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     131,   130,   129,   116,     0,   135,   137,   139,   141,   143,
     145,   146,   147,   148,   150,   151,   152,   153,   155,   156,
     157,   159,   160,   162,   163,   164,   184,   224,   220,   226,
     225,     0,   221,     0,     0,   192,   205,     0,   240,   236,
       0,   242,   185,   187,   189,   231,   228,     0,     0,    31,
       0,    73,    71,     0,    68,    69,     0,    62,     0,    98,
      48,    46,     0,     0,    39,    58,    56,   106,     0,   100,
      42,     0,   223,   219,   198,   195,     0,     0,   241,     0,
     254,   244,   230,    90,    88,    60,     0,    74,    65,     0,
      64,     0,    86,     0,    77,    78,    83,     0,     0,    49,
      33,     0,     0,     0,   105,     0,   133,   222,   239,   238,
     237,   234,   256,     0,     0,     0,   251,   250,   252,     0,
     245,   246,   253,    91,    72,    70,     0,     0,     0,    76,
      79,    80,     0,    99,    50,     0,     0,     0,   107,     0,
     101,     0,     0,   257,   249,   248,   243,     0,     0,    66,
       0,    85,    84,     0,    51,    34,    36,    40,   108,     0,
       0,   255,   247,    92,    93,    95,    81,     0,     0,     0,
       0,   262,   261,   263,   264,   258,     0,     0,    43,    37,
     260,   259,    94,    82
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     3,     5,    58,    59,    60,   132,    61,
     145,   395,    62,   146,   428,    63,   239,   362,    64,   245,
      65,   234,   235,   358,   413,    66,    67,   149,   150,    68,
     227,    69,   135,   229,   313,   314,   315,   316,    70,   318,
     353,   354,   437,   355,   392,   356,    71,   225,   308,   408,
     423,   424,    72,    73,   357,    74,   365,    75,    76,   153,
     328,    77,   221,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,   293,   129,    96,   103,   107,    97,   291,   292,    98,
     130,   223,   224,    99,   213,   298,   299,   339,   100,   341,
     379,   380,   381,   101,   372,   402,   403
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -330
static const yytype_int16 yypact[] =
{
    -330,    33,  -330,  -330,  -330,   302,     7,  -330,  1252,    36,
    -330,  1252,  1252,  1252,  1252,  1252,   136,  1252,  1252,  1252,
    1252,  1252,  1252,  1252,  1252,   590,  -330,  -330,  -330,  -330,
    -330,   -32,  -330,  -330,    45,   -24,    61,    71,  1252,    21,
     686,    80,    98,   103,  -330,  -330,   104,    32,    32,  1252,
    -330,   110,  1252,  1252,  1252,  -330,  -330,  -330,  -330,  -330,
    -330,  -330,  -330,    67,  -330,  -330,  -330,   107,  -330,  -330,
    -330,  -330,  -330,  -330,  -330,  -330,  -330,  -330,   112,     1,
    -330,   656,    57,   149,   153,   160,   165,    77,    37,    -6,
      34,    75,  1252,  -330,   -31,    65,  -330,  -330,  -330,  -330,
    -330,  -330,  -330,    58,  -330,  -330,  -330,    66,  -330,  -330,
    -330,  -330,  -330,   729,    87,   105,  -330,  -330,  -330,  -330,
    -330,  -330,  -330,  -330,   134,   825,   921,  1017,   137,    68,
    1252,  -330,  -330,  -330,  -330,   146,  -330,  -330,   138,   140,
    -330,   139,  -330,  1252,  1060,   155,   494,  -330,   207,   210,
    -330,   210,   150,   494,  1252,  -330,  -330,  -330,  -330,  -330,
    -330,  1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,
    1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,
    1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,
    1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,  1252,
    1252,  -330,  -330,  -330,  1252,  -330,  -330,  -330,  1156,  -330,
    1252,   136,  -330,  1295,  -330,    14,  1252,  1252,  1252,  -330,
    -330,  -330,  -330,    19,  -330,   145,   398,   146,   -34,   154,
    -330,  -330,  -330,   156,   157,    25,  -330,  1252,   135,  1252,
    1252,    32,  -330,   124,   162,   494,  -330,  -330,  -330,  -330,
    -330,  -330,  -330,  -330,  -330,  -330,  -330,  -330,  -330,  -330,
    -330,  -330,  -330,  -330,   163,   149,   153,   160,   165,    77,
      37,    37,    37,    37,    -6,    -6,    -6,    -6,    34,    34,
      34,    75,    75,  -330,  -330,  -330,  -330,  1252,  -330,  -330,
    1252,    11,  -330,    97,   168,  -330,   166,   228,   229,  -330,
     176,  -330,  -330,  -330,  -330,  1252,  -330,  1252,   154,  -330,
     154,  -330,    12,   171,   232,  -330,   173,  -330,    26,  -330,
    1252,  -330,   174,   183,  -330,  -330,  -330,   184,   494,  -330,
    -330,  1252,  1391,  -330,  -330,  -330,  1252,  1252,  1295,   182,
    -330,   -38,  -330,   241,  -330,  -330,  1252,  -330,  -330,   -34,
    -330,   190,   193,   185,   151,   158,  -330,   154,   191,  -330,
    -330,  1252,   187,   -56,  -330,   494,  -330,  -330,  -330,  -330,
    -330,  -330,   127,   131,   132,   172,  -330,  -330,  -330,   202,
     262,  -330,  -330,  -330,  -330,  -330,   205,   141,   206,  -330,
    -330,  -330,   154,  -330,  1252,   494,   208,   494,  -330,   211,
    -330,   267,    24,  -330,  -330,  -330,  -330,   -38,  1252,  -330,
     212,  -330,  -330,   213,  -330,  -330,  -330,  -330,  -330,   -30,
     127,  -330,  -330,   270,  -330,  -330,  -330,   494,   215,   147,
     148,  -330,  -330,  -330,  -330,  -330,  1252,   154,  -330,  -330,
    -330,  -330,  -330,  -330
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -330,  -330,  -330,  -330,   152,  -144,  -330,  -221,  -330,  -330,
    -330,  -330,  -330,  -330,  -330,  -330,  -330,  -330,  -330,  -330,
    -330,  -330,  -330,  -330,  -330,  -330,    46,   234,    44,  -330,
    -330,  -330,  -330,    59,  -330,  -330,   -62,   -61,  -330,  -330,
    -330,   -66,  -330,   -63,  -330,  -330,  -330,  -330,  -330,  -330,
    -330,  -146,  -330,  -330,  -330,  -330,  -330,  -330,  -330,  -330,
    -330,  -330,  -182,   -25,  -330,  -129,  -174,  -330,   113,   111,
     114,   115,   116,   -47,   -10,    -7,   -64,  -205,    42,    83,
     281,  -330,  -330,  -330,  -330,  -330,   283,  -330,   -28,  -330,
    -330,  -330,    -4,  -330,  -330,  -330,   -33,  -330,  -329,  -330,
    -330,  -330,  -101,  -321,  -330,  -330,  -112
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -197
static const yytype_int16 yytable[] =
{
     128,   222,   238,   290,   161,   264,   311,   398,   317,   243,
     373,   374,   378,   138,   332,   141,   346,   375,   429,   430,
     382,   203,   305,   204,   152,   375,   289,   420,   193,   194,
     195,   297,   246,     4,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     105,   300,   347,   108,   109,   110,   111,   112,   325,   116,
     117,   118,   119,   120,   121,   122,   123,   189,   190,   191,
     192,   301,   102,   376,   333,   399,   179,   180,   378,   222,
     306,   431,   196,   197,   222,   421,   382,   344,   128,   345,
     433,   162,   377,   106,   155,   156,   157,   312,   434,   131,
     432,   330,   185,   186,   187,   188,   205,   134,   206,   207,
     133,   222,   198,   199,   200,   201,    47,    48,   233,   236,
     208,   209,   210,   351,   352,   343,   136,   290,   205,   244,
     206,   207,   281,   282,   202,   137,   393,   247,   270,   271,
     272,   273,   208,   209,   210,   142,  -196,     7,  -196,  -196,
     289,   211,   139,   143,   368,   369,   297,   366,   144,   147,
    -196,  -196,  -196,   148,   384,   154,   158,   155,   156,   157,
     181,   412,   159,     9,    10,   182,   222,   160,   222,   274,
     275,   276,   277,   183,   364,   294,   278,   279,   280,   184,
     212,   113,    26,    27,    28,    29,    30,   215,   214,   220,
     219,   228,   230,   222,   232,   104,   231,   222,   222,   222,
     237,   240,   322,   241,   324,   242,   443,   222,   307,   319,
      33,   400,   320,   323,   327,   329,   425,   331,   334,   335,
     336,   337,   338,   340,   348,   349,   350,   360,   361,   363,
     283,   284,   285,   371,   383,   387,   286,    55,   388,   352,
     397,   415,   389,   417,   425,   351,   394,   401,   302,   303,
     304,   404,   405,   406,   124,   407,    56,    57,   409,   411,
     419,   416,   410,   436,   418,   426,   427,   440,   441,   222,
     439,   321,   151,   438,   226,   326,   310,   385,   386,   391,
     442,   390,   266,   265,   295,   359,   267,   114,   268,   115,
     269,   342,    -4,     6,   367,   370,   422,   222,   435,     0,
       0,     0,     0,     7,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     8,     0,     0,   108,
       0,     0,   202,     0,     0,     0,   396,     0,     0,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
      28,    29,    30,     0,    31,     0,     0,    32,    33,   414,
      34,    35,    36,    37,    38,     0,     0,    39,     0,     0,
       0,     0,    40,    41,     0,     0,    42,     0,     0,    43,
      44,    45,    46,    47,    48,     0,     0,    49,    50,     6,
       0,     0,     0,     0,     0,    51,    52,    53,    54,     7,
       0,     0,     0,    55,     0,     0,     0,     0,     0,     0,
       0,     0,     8,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    56,    57,     0,     9,    10,     0,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,    29,    30,     0,
      31,     0,     0,    32,    33,   309,    34,    35,    36,    37,
      38,     0,     0,    39,     0,     0,     0,     0,    40,    41,
       0,     0,    42,     0,     0,    43,    44,    45,    46,    47,
      48,     0,     0,    49,    50,     0,     0,     0,     0,     0,
       0,    51,    52,    53,    54,     7,     0,     0,     0,    55,
       0,     0,     0,     0,     0,     0,     0,     0,     8,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    56,    57,
       0,     9,    10,     0,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,    28,    29,    30,     0,    31,     0,     0,    32,
      33,     0,    34,    35,    36,    37,    38,     0,     0,    39,
       0,     0,     0,     0,    40,    41,     0,     0,    42,     0,
       0,    43,    44,    45,    46,    47,    48,     0,     0,    49,
      50,     0,     0,     0,     0,     0,     0,    51,    52,    53,
      54,     7,     0,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,     0,     8,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    56,    57,     0,     9,    10,     0,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,    29,
      30,     0,     0,     0,     0,     0,     0,     0,     0,   104,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,     0,     0,     0,     0,     0,
       0,     0,   124,     0,     0,   178,     0,     0,     0,     0,
       0,     0,     0,     0,   125,   126,   127,     7,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
       8,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      56,    57,     0,     9,    10,     0,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       7,    25,    26,    27,    28,    29,    30,     0,     0,     0,
       0,   140,     0,     8,     0,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     9,    10,     0,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,     0,    25,    26,    27,    28,    29,    30,
      52,    53,    54,     0,     0,     0,     0,    55,   104,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    56,    57,     0,     0,
       0,   124,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,    53,    54,     7,     0,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,     0,     8,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    56,
      57,     0,     9,    10,     0,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,     0,
      25,    26,    27,    28,    29,    30,     0,     0,   216,     0,
       0,     0,     0,     0,   104,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    52,
      53,    54,     7,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,     8,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    56,    57,     0,     9,    10,
       0,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,    27,    28,
      29,    30,     0,     0,   217,     0,     0,     0,     0,     0,
     104,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    52,    53,    54,     7,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,     8,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    56,    57,     0,     9,    10,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     7,    25,    26,    27,    28,    29,    30,     0,     0,
     218,     0,     0,     0,     8,     0,   104,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     9,    10,     0,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,    29,
      30,    52,    53,    54,     0,   -44,     0,     0,    55,   104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    56,    57,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    52,    53,    54,     7,     0,     0,
       0,    55,     0,     0,     0,     0,     0,     0,     0,     0,
       8,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      56,    57,     0,     9,    10,     0,   287,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,    28,    29,    30,     0,     0,     0,
       0,     0,     0,     0,     0,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   288,     0,     0,
      52,    53,    54,     7,     0,     0,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,     8,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    56,    57,     0,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     7,    25,    26,    27,
      28,    29,    30,     0,     0,     0,     0,     0,     0,     8,
       0,   104,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     9,    10,     0,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,     0,
      25,    26,    27,    28,    29,    30,    52,    53,    54,     0,
       0,     0,     0,    55,   104,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    56,    57,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    52,
      53,    54,     7,     0,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,     8,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    56,   296,     0,     9,    10,
       0,   287,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,    27,    28,
      29,    30,     0,     0,     0,     0,     0,     0,     0,     0,
     104,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    52,    53,    54,     0,     0,
       0,     0,    55,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    56,    57
};

static const yytype_int16 yycheck[] =
{
      25,   130,   146,   208,     3,   179,    40,    63,   229,   153,
      48,    49,   341,    38,     3,    40,     4,    55,    48,    49,
     341,    52,     3,    54,    49,    55,   208,     3,    34,    35,
      36,   213,   161,     0,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
       8,    37,    40,    11,    12,    13,    14,    15,   240,    17,
      18,    19,    20,    21,    22,    23,    24,    30,    31,    32,
      33,    57,    65,   111,    63,   131,    19,    20,   407,   208,
      61,   111,    48,    49,   213,    61,   407,   308,   113,   310,
     419,    90,   130,    57,    52,    53,    54,   131,   419,   131,
     130,   245,    25,    26,    27,    28,    41,   131,    43,    44,
      65,   240,    37,    38,    39,    40,    91,    92,   143,   144,
      55,    56,    57,    97,    98,   307,    65,   332,    41,   154,
      43,    44,   196,   197,    92,    64,   357,   162,   185,   186,
     187,   188,    55,    56,    57,    65,    41,    11,    43,    44,
     332,    86,   131,    55,   336,   337,   338,   331,    55,    55,
      55,    56,    57,   131,   346,    55,    99,   125,   126,   127,
      21,   392,    65,    37,    38,    22,   305,    65,   307,   189,
     190,   191,   192,    23,   328,   210,   193,   194,   195,    24,
     132,    55,    56,    57,    58,    59,    60,    63,   132,   131,
      63,    55,    64,   332,    65,    69,    66,   336,   337,   338,
      55,     4,   237,     3,   239,    65,   437,   346,    73,    63,
      66,   365,    65,    88,   100,    63,   408,    64,   131,    61,
      64,     3,     3,    57,    63,     3,    63,    63,    55,    55,
     198,   199,   200,    61,     3,    55,   204,   111,    55,    98,
      63,   395,    67,   397,   436,    97,    65,   130,   216,   217,
     218,   130,   130,    61,    92,     3,   130,   131,    63,    63,
       3,    63,   131,     3,    63,    63,    63,   130,   130,   408,
      65,   235,    48,   427,   132,   241,   227,   349,   349,   355,
     436,   354,   181,   180,   211,   320,   182,    16,   183,    16,
     184,   305,     0,     1,   332,   338,   407,   436,   420,    -1,
      -1,    -1,    -1,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,   287,
      -1,    -1,   290,    -1,    -1,    -1,   361,    -1,    -1,    37,
      38,    -1,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    -1,    62,    -1,    -1,    65,    66,   394,
      68,    69,    70,    71,    72,    -1,    -1,    75,    -1,    -1,
      -1,    -1,    80,    81,    -1,    -1,    84,    -1,    -1,    87,
      88,    89,    90,    91,    92,    -1,    -1,    95,    96,     1,
      -1,    -1,    -1,    -1,    -1,   103,   104,   105,   106,    11,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,    37,    38,    -1,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    -1,
      62,    -1,    -1,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    -1,    75,    -1,    -1,    -1,    -1,    80,    81,
      -1,    -1,    84,    -1,    -1,    87,    88,    89,    90,    91,
      92,    -1,    -1,    95,    96,    -1,    -1,    -1,    -1,    -1,
      -1,   103,   104,   105,   106,    11,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,    37,    38,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    -1,    55,
      56,    57,    58,    59,    60,    -1,    62,    -1,    -1,    65,
      66,    -1,    68,    69,    70,    71,    72,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    80,    81,    -1,    -1,    84,    -1,
      -1,    87,    88,    89,    90,    91,    92,    -1,    -1,    95,
      96,    -1,    -1,    -1,    -1,    -1,    -1,   103,   104,   105,
     106,    11,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    92,    -1,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,   106,    11,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,    37,    38,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      11,    55,    56,    57,    58,    59,    60,    -1,    -1,    -1,
      -1,    65,    -1,    24,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    -1,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    60,
     104,   105,   106,    -1,    -1,    -1,    -1,   111,    69,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    -1,
      -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,   105,   106,    11,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,    37,    38,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    56,    57,    58,    59,    60,    -1,    -1,    63,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
     105,   106,    11,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    37,    38,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,   105,   106,    11,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   130,   131,    -1,    37,    38,    -1,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    11,    55,    56,    57,    58,    59,    60,    -1,    -1,
      63,    -1,    -1,    -1,    24,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    -1,    55,    56,    57,    58,    59,
      60,   104,   105,   106,    -1,    65,    -1,    -1,   111,    69,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,   105,   106,    11,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,    37,    38,    -1,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      -1,    55,    56,    57,    58,    59,    60,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,
     104,   105,   106,    11,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    37,
      38,    -1,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    11,    55,    56,    57,
      58,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    37,    38,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    -1,
      55,    56,    57,    58,    59,    60,   104,   105,   106,    -1,
      -1,    -1,    -1,   111,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
     105,   106,    11,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    37,    38,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    -1,    55,    56,    57,    58,
      59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,   105,   106,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   130,   131
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   134,   135,   136,     0,   137,     1,    11,    24,    37,
      38,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    55,    56,    57,    58,    59,
      60,    62,    65,    66,    68,    69,    70,    71,    72,    75,
      80,    81,    84,    87,    88,    89,    90,    91,    92,    95,
      96,   103,   104,   105,   106,   111,   130,   131,   138,   139,
     140,   142,   145,   148,   151,   153,   158,   159,   162,   164,
     171,   179,   185,   186,   188,   190,   191,   194,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   216,   219,   222,   226,
     231,   236,    65,   217,    69,   211,    57,   218,   211,   211,
     211,   211,   211,    55,   213,   219,   211,   211,   211,   211,
     211,   211,   211,   211,    92,   104,   105,   106,   196,   215,
     223,   131,   141,    65,   131,   165,    65,    64,   196,   131,
      65,   196,    65,    55,    55,   143,   146,    55,   131,   160,
     161,   160,   196,   192,    55,   211,   211,   211,    99,    65,
      65,     3,    90,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    29,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    30,
      31,    32,    33,    34,    35,    36,    48,    49,    37,    38,
      39,    40,   211,    52,    54,    41,    43,    44,    55,    56,
      57,    86,   132,   227,   132,    63,    63,    63,    63,    63,
     131,   195,   198,   224,   225,   180,   137,   163,    55,   166,
      64,    66,    65,   196,   154,   155,   196,    55,   138,   149,
       4,     3,    65,   138,   196,   152,   198,   196,   198,   198,
     198,   198,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   198,   198,   198,   199,   201,   202,   203,   204,   205,
     206,   206,   206,   206,   207,   207,   207,   207,   208,   208,
     208,   209,   209,   211,   211,   211,   211,    40,   101,   195,
     210,   220,   221,   214,   196,   212,   131,   195,   228,   229,
      37,    57,   211,   211,   211,     3,    61,    73,   181,    67,
     166,    40,   131,   167,   168,   169,   170,   140,   172,    63,
      65,   159,   196,    88,   196,   195,   161,   100,   193,    63,
     138,    64,     3,    63,   131,    61,    64,     3,     3,   230,
      57,   232,   225,   195,   140,   140,     4,    40,    63,     3,
      63,    97,    98,   173,   174,   176,   178,   187,   156,   196,
      63,    55,   150,    55,   138,   189,   199,   221,   195,   195,
     229,    61,   237,    48,    49,    55,   111,   130,   231,   233,
     234,   235,   236,     3,   195,   169,   170,    55,    55,    67,
     176,   174,   177,   140,    65,   144,   196,    63,    63,   131,
     138,   130,   238,   239,   130,   130,    61,     3,   182,    63,
     131,    63,   140,   157,   196,   138,    63,   138,    63,     3,
       3,    61,   235,   183,   184,   195,    63,    63,   147,    48,
      49,   111,   130,   231,   236,   239,     3,   175,   138,    65,
     130,   130,   184,   140
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
# if YYLTYPE_IS_TRIVIAL
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
/*
  switch (yytype)
    {

      default:
	break;
    }*/
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

/* Line 1455 of yacc.c  */
#line 221 "syntax/tjs.y"
    { sb->PushContextStack(TJS_W("global"),
												ctTopLevel); ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 223 "syntax/tjs.y"
    { sb->PopContextStack(); ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 230 "syntax/tjs.y"
    { if(sb->CompileErrorCount>20)
												YYABORT;
											  else yyerrok; ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 244 "syntax/tjs.y"
    { cc->CreateExprCode((yyvsp[(1) - (2)].np)); ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 250 "syntax/tjs.y"
    { cc->DoBreak(); ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 251 "syntax/tjs.y"
    { cc->DoContinue(); ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 252 "syntax/tjs.y"
    { cc->DoDebugger(); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 267 "syntax/tjs.y"
    { cc->EnterBlock(); ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 269 "syntax/tjs.y"
    { cc->ExitBlock(); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 274 "syntax/tjs.y"
    { cc->EnterWhileCode(false); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 275 "syntax/tjs.y"
    { cc->CreateWhileExprCode((yyvsp[(4) - (5)].np), false); ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 276 "syntax/tjs.y"
    { cc->ExitWhileCode(false); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 281 "syntax/tjs.y"
    { cc->EnterWhileCode(true); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 284 "syntax/tjs.y"
    { cc->CreateWhileExprCode((yyvsp[(6) - (7)].np), true); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 285 "syntax/tjs.y"
    { cc->ExitWhileCode(true); ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 290 "syntax/tjs.y"
    { cc->EnterIfCode(); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 291 "syntax/tjs.y"
    { cc->CreateIfExprCode((yyvsp[(4) - (4)].np)); ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 292 "syntax/tjs.y"
    { cc->ExitIfCode(); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 297 "syntax/tjs.y"
    { cc->EnterElseCode(); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 298 "syntax/tjs.y"
    { cc->ExitElseCode(); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 307 "syntax/tjs.y"
    { cc->ExitForCode(); ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 313 "syntax/tjs.y"
    { cc->EnterForCode(false); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 314 "syntax/tjs.y"
    { cc->EnterForCode(true); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 316 "syntax/tjs.y"
    { cc->EnterForCode(false);
											  cc->CreateExprCode((yyvsp[(1) - (1)].np)); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 322 "syntax/tjs.y"
    { cc->CreateForExprCode(NULL); ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 323 "syntax/tjs.y"
    { cc->CreateForExprCode((yyvsp[(1) - (1)].np)); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 328 "syntax/tjs.y"
    { cc->SetForThirdExprCode(NULL); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 329 "syntax/tjs.y"
    { cc->SetForThirdExprCode((yyvsp[(1) - (1)].np)); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 352 "syntax/tjs.y"
    { cc->AddLocalVariable(
												lx->GetString((yyvsp[(1) - (1)].num))); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 354 "syntax/tjs.y"
    { cc->InitLocalVariable(
											  lx->GetString((yyvsp[(1) - (3)].num)), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 360 "syntax/tjs.y"
    { sb->PushContextStack(
												lx->GetString((yyvsp[(2) - (2)].num)),
											  ctFunction);
											  cc->EnterBlock();;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 365 "syntax/tjs.y"
    { cc->ExitBlock(); sb->PopContextStack(); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 370 "syntax/tjs.y"
    { sb->PushContextStack(
												TJS_W("(anonymous)"),
											  ctExprFunction);
											  cc->EnterBlock(); ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 375 "syntax/tjs.y"
    { cc->ExitBlock();
											  tTJSVariant v(cc);
											  sb->PopContextStack();
											  (yyval.np) = cc->MakeNP0(T_CONSTVAL);
											  (yyval.np)->SetValue(v); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 402 "syntax/tjs.y"
    { cc->AddFunctionDeclArg(
												lx->GetString((yyvsp[(1) - (1)].num)), NULL); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 404 "syntax/tjs.y"
    { cc->AddFunctionDeclArg(
												lx->GetString((yyvsp[(1) - (3)].num)), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 409 "syntax/tjs.y"
    { cc->AddFunctionDeclArgCollapse(
												NULL); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 411 "syntax/tjs.y"
    { cc->AddFunctionDeclArgCollapse(
												lx->GetString((yyvsp[(1) - (2)].num))); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 423 "syntax/tjs.y"
    { sb->PushContextStack(
												lx->GetString((yyvsp[(2) - (3)].num)),
												ctProperty); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 427 "syntax/tjs.y"
    { sb->PopContextStack(); ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 438 "syntax/tjs.y"
    { sb->PushContextStack(
												TJS_W("(setter)"),
												ctPropertySetter);
											  cc->EnterBlock();
											  cc->SetPropertyDeclArg(
												lx->GetString((yyvsp[(3) - (4)].num))); ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 444 "syntax/tjs.y"
    { cc->ExitBlock();
											  sb->PopContextStack(); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 449 "syntax/tjs.y"
    { sb->PushContextStack(
												TJS_W("(getter)"),
												ctPropertyGetter);
											  cc->EnterBlock(); ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 453 "syntax/tjs.y"
    { cc->ExitBlock();
											  sb->PopContextStack(); ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 465 "syntax/tjs.y"
    { sb->PushContextStack(
												lx->GetString((yyvsp[(2) - (2)].num)),
												ctClass); ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 469 "syntax/tjs.y"
    { sb->PopContextStack(); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 474 "syntax/tjs.y"
    { cc->CreateExtendsExprCode((yyvsp[(2) - (2)].np), true); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 475 "syntax/tjs.y"
    { cc->CreateExtendsExprCode((yyvsp[(2) - (3)].np), false); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 485 "syntax/tjs.y"
    { cc->CreateExtendsExprCode((yyvsp[(1) - (1)].np), false); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 490 "syntax/tjs.y"
    { cc->ReturnFromFunc(NULL); ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 491 "syntax/tjs.y"
    { cc->ReturnFromFunc((yyvsp[(2) - (3)].np)); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 498 "syntax/tjs.y"
    { cc->EnterSwitchCode((yyvsp[(3) - (4)].np)); ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 499 "syntax/tjs.y"
    { cc->ExitSwitchCode(); ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 505 "syntax/tjs.y"
    { cc->EnterWithCode((yyvsp[(3) - (4)].np)); ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 506 "syntax/tjs.y"
    { cc->ExitWithCode(); ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 511 "syntax/tjs.y"
    { cc->ProcessCaseCode((yyvsp[(2) - (3)].np)); ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 512 "syntax/tjs.y"
    { cc->ProcessCaseCode(NULL); ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 517 "syntax/tjs.y"
    { cc->EnterTryCode(); ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 520 "syntax/tjs.y"
    { cc->ExitTryCode(); ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 524 "syntax/tjs.y"
    { cc->EnterCatchCode(NULL); ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 525 "syntax/tjs.y"
    { cc->EnterCatchCode(NULL); ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 526 "syntax/tjs.y"
    { cc->EnterCatchCode(
												lx->GetString((yyvsp[(3) - (4)].num))); ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 532 "syntax/tjs.y"
    { cc->ProcessThrowCode((yyvsp[(2) - (3)].np)); ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 537 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 541 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 542 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_IF, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 547 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 548 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_COMMA, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 553 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 554 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_SWAP, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 555 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_EQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 556 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_AMPERSANDEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 557 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_VERTLINEEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 558 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_CHEVRONEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 559 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_MINUSEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 560 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PLUSEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 561 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PERCENTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 562 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_SLASHEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 563 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_BACKSLASHEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 564 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_ASTERISKEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 565 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALOREQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 566 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALANDEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 567 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RARITHSHIFTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 568 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LARITHSHIFTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 569 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RBITSHIFTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 574 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 577 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP3(T_QUESTION, (yyvsp[(1) - (5)].np), (yyvsp[(3) - (5)].np), (yyvsp[(5) - (5)].np)); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 583 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 584 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALOR, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 588 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 590 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LOGICALAND, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 594 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 595 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_VERTLINE, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 599 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 600 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_CHEVRON, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 604 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 605 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_AMPERSAND, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 609 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 610 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_NOTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 611 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_EQUALEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 612 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_DISCNOTEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 613 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_DISCEQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 617 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 618 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 619 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_GT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 620 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LTOREQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 621 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_GTOREQUAL, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 625 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 626 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RARITHSHIFT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 627 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LARITHSHIFT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 628 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_RBITSHIFT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 633 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 634 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PLUS, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 635 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_MINUS, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 639 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 640 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_PERCENT, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 641 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_SLASH, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 642 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_BACKSLASH, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 643 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_ASTERISK, (yyvsp[(1) - (2)].np), (yyvsp[(2) - (2)].np)); ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 647 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (2)].np); ;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 651 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 652 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EXCRAMATION, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 653 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_TILDE, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 654 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_DECREMENT, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 655 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INCREMENT, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 656 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(2) - (2)].np); (yyval.np)->SetOpecode(T_NEW); ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 657 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INVALIDATE, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 658 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ISVALID, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 659 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ISVALID, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 660 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_DELETE, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 661 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_TYPEOF, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 662 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_SHARP, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 663 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_DOLLAR, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 664 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_UPLUS, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 665 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_UMINUS, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 666 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_IGNOREPROP, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 667 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_PROPACCESS, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 668 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_INSTANCEOF, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 669 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INT, (yyvsp[(4) - (4)].np)); ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 670 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_INT, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 671 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_REAL, (yyvsp[(4) - (4)].np)); ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 672 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_REAL, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 673 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_STRING, (yyvsp[(4) - (4)].np)); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 674 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_STRING, (yyvsp[(2) - (2)].np)); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 678 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 680 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_INCONTEXTOF, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 684 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 685 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(2) - (3)].np); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 686 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LBRACKET, (yyvsp[(1) - (4)].np), (yyvsp[(3) - (4)].np)); ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 687 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 688 "syntax/tjs.y"
    { lx->SetNextIsBareWord(); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 689 "syntax/tjs.y"
    { tTJSExprNode * node = cc->MakeNP0(T_CONSTVAL);
												  node->SetValue(lx->GetValue((yyvsp[(4) - (4)].num)));
												  (yyval.np) = cc->MakeNP2(T_DOT, (yyvsp[(1) - (4)].np), node); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 692 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_POSTINCREMENT, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 693 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_POSTDECREMENT, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 694 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EVAL, (yyvsp[(1) - (2)].np)); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 695 "syntax/tjs.y"
    { lx->SetNextIsBareWord(); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 696 "syntax/tjs.y"
    { tTJSExprNode * node = cc->MakeNP0(T_CONSTVAL);
												  node->SetValue(lx->GetValue((yyvsp[(3) - (3)].num)));
												  (yyval.np) = cc->MakeNP1(T_WITHDOT, node); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 703 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_CONSTVAL);
												  (yyval.np)->SetValue(lx->GetValue((yyvsp[(1) - (1)].num))); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 705 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_SYMBOL);
												  (yyval.np)->SetValue(tTJSVariant(
													lx->GetString((yyvsp[(1) - (1)].num)))); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 708 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_THIS); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 709 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_SUPER); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 710 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 711 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_GLOBAL); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 712 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_VOID); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 713 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 714 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 715 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 716 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 717 "syntax/tjs.y"
    { lx->SetStartOfRegExp(); ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 718 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_REGEXP);
												  (yyval.np)->SetValue(lx->GetValue((yyvsp[(3) - (3)].num))); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 720 "syntax/tjs.y"
    { lx->SetStartOfRegExp(); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 721 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_REGEXP);
												  (yyval.np)->SetValue(lx->GetValue((yyvsp[(3) - (3)].num))); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 728 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_LPARENTHESIS, (yyvsp[(1) - (4)].np), (yyvsp[(3) - (4)].np)); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 733 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP0(T_OMIT); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 734 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ARG, (yyvsp[(1) - (1)].np)); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 735 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_ARG, (yyvsp[(3) - (3)].np), (yyvsp[(1) - (3)].np)); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 739 "syntax/tjs.y"
    { (yyval.np) = NULL; ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 740 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EXPANDARG, NULL); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 741 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_EXPANDARG, (yyvsp[(1) - (1)].np)); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 742 "syntax/tjs.y"
    { (yyval.np) = (yyvsp[(1) - (1)].np); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 748 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_INLINEARRAY);
										  cc->PushCurrentNode(node); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 752 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 757 "syntax/tjs.y"
    { cn->Add((yyvsp[(1) - (1)].np)); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 758 "syntax/tjs.y"
    { cn->Add((yyvsp[(3) - (3)].np)); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 763 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ARRAYARG, NULL); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 764 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP1(T_ARRAYARG, (yyvsp[(1) - (1)].np)); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 769 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_INLINEDIC);
										  cc->PushCurrentNode(node); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 774 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 781 "syntax/tjs.y"
    { cn->Add((yyvsp[(1) - (1)].np)); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 782 "syntax/tjs.y"
    { cn->Add((yyvsp[(3) - (3)].np)); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 787 "syntax/tjs.y"
    { (yyval.np) = cc->MakeNP2(T_DICELM, (yyvsp[(1) - (3)].np), (yyvsp[(3) - (3)].np)); ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 788 "syntax/tjs.y"
    { tTJSVariant val(lx->GetString((yyvsp[(1) - (3)].num)));
										  tTJSExprNode *node0 = cc->MakeNP0(T_CONSTVAL);
										  node0->SetValue(val);
										  (yyval.np) = cc->MakeNP2(T_DICELM, node0, (yyvsp[(3) - (3)].np)); ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 804 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_CONSTVAL);
										  iTJSDispatch2 * dsp = TJSCreateArrayObject();
										  node->SetValue(tTJSVariant(dsp, dsp));
										  dsp->Release();
										  cc->PushCurrentNode(node); ;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 811 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 829 "syntax/tjs.y"
    { cn->AddArrayElement(- lx->GetValue((yyvsp[(2) - (2)].num))); ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 830 "syntax/tjs.y"
    { cn->AddArrayElement(+ lx->GetValue((yyvsp[(2) - (2)].num))); ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 831 "syntax/tjs.y"
    { cn->AddArrayElement(lx->GetValue((yyvsp[(1) - (1)].num))); ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 832 "syntax/tjs.y"
    { cn->AddArrayElement(tTJSVariant());  ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 833 "syntax/tjs.y"
    { cn->AddArrayElement((yyvsp[(1) - (1)].np)->GetValue()); ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 834 "syntax/tjs.y"
    { cn->AddArrayElement((yyvsp[(1) - (1)].np)->GetValue()); ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 839 "syntax/tjs.y"
    { tTJSExprNode *node =
										  cc->MakeNP0(T_CONSTVAL);
										  iTJSDispatch2 * dsp = TJSCreateDictionaryObject();
										  node->SetValue(tTJSVariant(dsp, dsp));
										  dsp->Release();
										  cc->PushCurrentNode(node); ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 846 "syntax/tjs.y"
    { (yyval.np) = cn; cc->PopCurrentNode(); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 859 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (4)].num)), - lx->GetValue((yyvsp[(4) - (4)].num))); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 860 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (4)].num)), + lx->GetValue((yyvsp[(4) - (4)].num))); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 861 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), lx->GetValue((yyvsp[(3) - (3)].num))); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 862 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), tTJSVariant()); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 863 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), (yyvsp[(3) - (3)].np)->GetValue()); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 864 "syntax/tjs.y"
    { cn->AddDictionaryElement(lx->GetValue((yyvsp[(1) - (3)].num)), (yyvsp[(3) - (3)].np)->GetValue()); ;}
    break;



/* Line 1455 of yacc.c  */
#line 3719 "tjs.tab.cpp"
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



/* Line 1675 of yacc.c  */
#line 869 "syntax/tjs.y"



}
