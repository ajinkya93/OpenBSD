/* A Bison parser, made from m2-exp.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	INT	257
# define	HEX	258
# define	ERROR	259
# define	UINT	260
# define	M2_TRUE	261
# define	M2_FALSE	262
# define	CHAR	263
# define	FLOAT	264
# define	STRING	265
# define	NAME	266
# define	BLOCKNAME	267
# define	IDENT	268
# define	VARNAME	269
# define	TYPENAME	270
# define	SIZE	271
# define	CAP	272
# define	ORD	273
# define	HIGH	274
# define	ABS	275
# define	MIN_FUNC	276
# define	MAX_FUNC	277
# define	FLOAT_FUNC	278
# define	VAL	279
# define	CHR	280
# define	ODD	281
# define	TRUNC	282
# define	INC	283
# define	DEC	284
# define	INCL	285
# define	EXCL	286
# define	COLONCOLON	287
# define	INTERNAL_VAR	288
# define	ABOVE_COMMA	289
# define	ASSIGN	290
# define	LEQ	291
# define	GEQ	292
# define	NOTEQUAL	293
# define	IN	294
# define	OROR	295
# define	LOGICAL_AND	296
# define	DIV	297
# define	MOD	298
# define	UNARY	299
# define	DOT	300
# define	NOT	301
# define	QID	302

#line 41 "m2-exp.y"


#include "defs.h"
#include "gdb_string.h"
#include "expression.h"
#include "language.h"
#include "value.h"
#include "parser-defs.h"
#include "m2-lang.h"
#include "bfd.h" /* Required by objfiles.h.  */
#include "symfile.h" /* Required by objfiles.h.  */
#include "objfiles.h" /* For have_full_symbols and have_partial_symbols */
#include "block.h"

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
   as well as gratuitiously global symbol names, so we can have multiple
   yacc generated parsers in gdb.  Note that these are only the variables
   produced by yacc.  If other parser generators (bison, byacc, etc) produce
   additional global names that conflict at link time, then those parser
   generators need to be fixed instead of adding those names to this list. */

#define	yymaxdepth m2_maxdepth
#define	yyparse	m2_parse
#define	yylex	m2_lex
#define	yyerror	m2_error
#define	yylval	m2_lval
#define	yychar	m2_char
#define	yydebug	m2_debug
#define	yypact	m2_pact
#define	yyr1	m2_r1
#define	yyr2	m2_r2
#define	yydef	m2_def
#define	yychk	m2_chk
#define	yypgo	m2_pgo
#define	yyact	m2_act
#define	yyexca	m2_exca
#define	yyerrflag m2_errflag
#define	yynerrs	m2_nerrs
#define	yyps	m2_ps
#define	yypv	m2_pv
#define	yys	m2_s
#define	yy_yys	m2_yys
#define	yystate	m2_state
#define	yytmp	m2_tmp
#define	yyv	m2_v
#define	yy_yyv	m2_yyv
#define	yyval	m2_val
#define	yylloc	m2_lloc
#define	yyreds	m2_reds		/* With YYDEBUG defined */
#define	yytoks	m2_toks		/* With YYDEBUG defined */
#define yyname	m2_name		/* With YYDEBUG defined */
#define yyrule	m2_rule		/* With YYDEBUG defined */
#define yylhs	m2_yylhs
#define yylen	m2_yylen
#define yydefred m2_yydefred
#define yydgoto	m2_yydgoto
#define yysindex m2_yysindex
#define yyrindex m2_yyrindex
#define yygindex m2_yygindex
#define yytable	 m2_yytable
#define yycheck	 m2_yycheck

#ifndef YYDEBUG
#define	YYDEBUG 1		/* Default to yydebug support */
#endif

#define YYFPRINTF parser_fprintf

int yyparse (void);

static int yylex (void);

void yyerror (char *);

#if 0
static char *make_qualname (char *, char *);
#endif

static int parse_number (int);

/* The sign of the number being parsed. */
static int number_sign = 1;

/* The block that the module specified by the qualifer on an identifer is
   contained in, */
#if 0
static struct block *modblock=0;
#endif


#line 136 "m2-exp.y"
#ifndef YYSTYPE
typedef union
  {
    LONGEST lval;
    ULONGEST ulval;
    DOUBLEST dval;
    struct symbol *sym;
    struct type *tval;
    struct stoken sval;
    int voidval;
    struct block *bval;
    enum exp_opcode opcode;
    struct internalvar *ivar;

    struct type **tvec;
    int *ivec;
  } yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		181
#define	YYFLAG		-32768
#define	YYNTBASE	68

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 302 ? yytranslate[x] : 82)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    44,     2,     2,    48,     2,
      60,    64,    52,    50,    35,    51,     2,    53,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      38,    42,    39,     2,    49,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    59,     2,    67,    57,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    65,     2,    66,    62,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    36,
      37,    40,    41,    43,    45,    46,    47,    54,    55,    56,
      58,    61,    63
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     2,     4,     6,     9,    10,    14,    17,    20,
      22,    24,    29,    34,    39,    44,    49,    54,    59,    66,
      71,    76,    81,    84,    89,    96,   101,   108,   112,   114,
     118,   125,   132,   136,   141,   142,   148,   149,   155,   156,
     158,   162,   164,   168,   173,   178,   182,   186,   190,   194,
     198,   202,   206,   210,   214,   218,   222,   226,   230,   234,
     238,   242,   246,   250,   252,   254,   256,   258,   260,   262,
     264,   269,   271,   273,   275,   279,   281,   283,   287,   289
};
static const short yyrhs[] =
{
      70,     0,    69,     0,    81,     0,    70,    57,     0,     0,
      51,    71,    70,     0,    50,    70,     0,    72,    70,     0,
      61,     0,    62,     0,    18,    60,    70,    64,     0,    19,
      60,    70,    64,     0,    21,    60,    70,    64,     0,    20,
      60,    70,    64,     0,    22,    60,    81,    64,     0,    23,
      60,    81,    64,     0,    24,    60,    70,    64,     0,    25,
      60,    81,    35,    70,    64,     0,    26,    60,    70,    64,
       0,    27,    60,    70,    64,     0,    28,    60,    70,    64,
       0,    17,    70,     0,    29,    60,    70,    64,     0,    29,
      60,    70,    35,    70,    64,     0,    30,    60,    70,    64,
       0,    30,    60,    70,    35,    70,    64,     0,    70,    58,
      12,     0,    73,     0,    70,    45,    73,     0,    31,    60,
      70,    35,    70,    64,     0,    32,    60,    70,    35,    70,
      64,     0,    65,    76,    66,     0,    81,    65,    76,    66,
       0,     0,    70,    59,    74,    77,    67,     0,     0,    70,
      60,    75,    76,    64,     0,     0,    70,     0,    76,    35,
      70,     0,    70,     0,    77,    35,    70,     0,    65,    81,
      66,    70,     0,    81,    60,    70,    64,     0,    60,    70,
      64,     0,    70,    49,    70,     0,    70,    52,    70,     0,
      70,    53,    70,     0,    70,    54,    70,     0,    70,    55,
      70,     0,    70,    50,    70,     0,    70,    51,    70,     0,
      70,    42,    70,     0,    70,    43,    70,     0,    70,    44,
      70,     0,    70,    40,    70,     0,    70,    41,    70,     0,
      70,    38,    70,     0,    70,    39,    70,     0,    70,    47,
      70,     0,    70,    46,    70,     0,    70,    37,    70,     0,
       7,     0,     8,     0,     3,     0,     6,     0,     9,     0,
      10,     0,    80,     0,    17,    60,    81,    64,     0,    11,
       0,    79,     0,    13,     0,    78,    33,    13,     0,    79,
       0,    34,     0,    78,    33,    12,     0,    12,     0,    16,
       0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   205,   206,   209,   218,   222,   222,   229,   233,   237,
     238,   241,   245,   249,   253,   257,   263,   269,   273,   279,
     283,   287,   291,   296,   300,   306,   310,   316,   322,   325,
     329,   333,   337,   339,   345,   345,   356,   356,   366,   369,
     373,   378,   383,   388,   394,   400,   408,   412,   416,   420,
     424,   428,   432,   436,   440,   442,   446,   450,   454,   458,
     462,   466,   470,   477,   483,   489,   496,   505,   513,   520,
     523,   530,   537,   541,   550,   562,   570,   574,   590,   641
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "INT", "HEX", "ERROR", "UINT", "M2_TRUE", 
  "M2_FALSE", "CHAR", "FLOAT", "STRING", "NAME", "BLOCKNAME", "IDENT", 
  "VARNAME", "TYPENAME", "SIZE", "CAP", "ORD", "HIGH", "ABS", "MIN_FUNC", 
  "MAX_FUNC", "FLOAT_FUNC", "VAL", "CHR", "ODD", "TRUNC", "INC", "DEC", 
  "INCL", "EXCL", "COLONCOLON", "INTERNAL_VAR", "','", "ABOVE_COMMA", 
  "ASSIGN", "'<'", "'>'", "LEQ", "GEQ", "'='", "NOTEQUAL", "'#'", "IN", 
  "OROR", "LOGICAL_AND", "'&'", "'@'", "'+'", "'-'", "'*'", "'/'", "DIV", 
  "MOD", "UNARY", "'^'", "DOT", "'['", "'('", "NOT", "'~'", "QID", "')'", 
  "'{'", "'}'", "']'", "start", "type_exp", "exp", "@1", "not_exp", "set", 
  "@2", "@3", "arglist", "non_empty_arglist", "block", "fblock", 
  "variable", "type", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    68,    68,    69,    70,    71,    70,    70,    70,    72,
      72,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    73,    73,    74,    70,    75,    70,    76,    76,
      76,    77,    77,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    78,    79,    79,    80,    80,    80,    80,    81
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     1,     1,     1,     2,     0,     3,     2,     2,     1,
       1,     4,     4,     4,     4,     4,     4,     4,     6,     4,
       4,     4,     2,     4,     6,     4,     6,     3,     1,     3,
       6,     6,     3,     4,     0,     5,     0,     5,     0,     1,
       3,     1,     3,     4,     4,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     1,     1,     3,     1,     1,     3,     1,     1
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       0,    65,    66,    63,    64,    67,    68,    71,    78,    73,
      79,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    76,     0,     5,
       0,     9,    10,    38,     2,     1,     0,    28,     0,    75,
      69,     3,     0,    22,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       7,     0,     0,    39,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     4,     0,    34,    36,     8,     0,
       0,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     6,    45,
       0,    32,     0,    62,    58,    59,    56,    57,    53,    54,
      55,    38,    29,     0,    61,    60,    46,    51,    52,    47,
      48,    49,    50,    27,     0,    38,    77,    74,     0,     0,
      70,    11,    12,    14,    13,    15,    16,    17,     0,    19,
      20,    21,     0,    23,     0,    25,     0,     0,    40,    43,
      41,     0,     0,    44,    33,     0,     0,     0,     0,     0,
       0,    35,    37,    18,    24,    26,    30,    31,    42,     0,
       0,     0
};

static const short yydefgoto[] =
{
     179,    34,    63,    61,    36,    37,   134,   135,    64,   161,
      38,    39,    40,    44
};

static const short yypact[] =
{
     155,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,   215,   -27,   -22,   -20,   -19,    14,    24,    26,    27,
      28,    29,    31,    32,    33,    35,    36,-32768,   155,-32768,
     155,-32768,-32768,   155,-32768,   742,   155,-32768,    -6,    -4,
  -32768,   -34,   155,     5,   -34,   155,   155,   155,   155,    44,
      44,   155,    44,   155,   155,   155,   155,   155,   155,   155,
       5,   155,   272,   742,   -31,   -41,   155,   155,   155,   155,
     155,   155,   155,   155,   -15,   155,   155,   155,   155,   155,
     155,   155,   155,   155,-32768,    85,-32768,-32768,     5,    -5,
     155,   155,   -21,   300,   328,   356,   384,    34,    39,   412,
      64,   440,   468,   496,    78,   244,   692,   718,     5,-32768,
     155,-32768,   155,   766,   -37,   -37,   -37,   -37,   -37,   -37,
     -37,   155,-32768,    40,   141,   201,   777,   786,   786,     5,
       5,     5,     5,-32768,   155,   155,-32768,-32768,   524,   -29,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   155,-32768,
  -32768,-32768,   155,-32768,   155,-32768,   155,   155,   742,     5,
     742,   -33,   -32,-32768,-32768,   552,   580,   608,   636,   664,
     155,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   742,   100,
     106,-32768
};

static const short yypgoto[] =
{
  -32768,-32768,     0,-32768,-32768,    37,-32768,-32768,   -86,-32768,
  -32768,-32768,-32768,    52
};


#define	YYLAST		846


static const short yytable[] =
{
      35,    10,   170,   110,   110,   139,   110,   136,   137,    75,
      76,    43,    77,    78,    79,    80,    81,    82,    83,    90,
      84,    85,    86,    87,    91,   112,    90,    89,    60,   -72,
      62,    91,   172,    45,   171,   111,    88,   164,    46,    90,
      47,    48,    62,   140,    91,    93,    94,    95,    96,   162,
     121,    99,    41,   101,   102,   103,   104,   105,   106,   107,
      10,   108,    84,    85,    86,    87,   113,   114,   115,   116,
     117,   118,   119,   120,    49,   124,   125,   126,   127,   128,
     129,   130,   131,   132,    50,    65,    51,    52,    53,    54,
     138,    55,    56,    57,    92,    58,    59,   133,   145,   148,
     180,    97,    98,   146,   100,    91,   181,     0,     0,     0,
     158,   122,   159,   152,     0,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,   123,    77,    78,    79,
      80,    81,    82,    83,   160,    84,    85,    86,    87,     0,
       0,     0,   153,     0,     0,     0,     0,     0,   165,     0,
       0,     0,   166,     0,   167,     0,   168,   169,     1,     0,
       0,     2,     3,     4,     5,     6,     7,     8,     9,     0,
     178,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    76,    27,
      77,    78,    79,    80,    81,    82,    83,     0,    84,    85,
      86,    87,     0,     0,     0,    28,    29,     0,     0,     0,
       0,     0,     0,     0,     0,    30,    31,    32,     1,     0,
      33,     2,     3,     4,     5,     6,     7,     8,     9,     0,
       0,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,     0,    27,
      77,    78,    79,    80,    81,    82,    83,     0,    84,    85,
      86,    87,     0,     0,     0,    28,    29,     0,     0,     0,
       0,     0,     0,     0,     0,    42,    31,    32,     0,   154,
      33,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,     0,    77,    78,    79,    80,    81,    82,    83,
       0,    84,    85,    86,    87,     0,     0,     0,   155,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
       0,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,     0,     0,     0,   109,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,     0,    77,
      78,    79,    80,    81,    82,    83,     0,    84,    85,    86,
      87,     0,     0,     0,   141,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,     0,    77,    78,    79,
      80,    81,    82,    83,     0,    84,    85,    86,    87,     0,
       0,     0,   142,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,     0,    77,    78,    79,    80,    81,
      82,    83,     0,    84,    85,    86,    87,     0,     0,     0,
     143,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,     0,    77,    78,    79,    80,    81,    82,    83,
       0,    84,    85,    86,    87,     0,     0,     0,   144,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
       0,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,     0,     0,     0,   147,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,     0,    77,
      78,    79,    80,    81,    82,    83,     0,    84,    85,    86,
      87,     0,     0,     0,   149,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,     0,    77,    78,    79,
      80,    81,    82,    83,     0,    84,    85,    86,    87,     0,
       0,     0,   150,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,     0,    77,    78,    79,    80,    81,
      82,    83,     0,    84,    85,    86,    87,     0,     0,     0,
     151,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,     0,    77,    78,    79,    80,    81,    82,    83,
       0,    84,    85,    86,    87,     0,     0,     0,   163,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
       0,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,     0,     0,     0,   173,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,     0,    77,
      78,    79,    80,    81,    82,    83,     0,    84,    85,    86,
      87,     0,     0,     0,   174,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,     0,    77,    78,    79,
      80,    81,    82,    83,     0,    84,    85,    86,    87,     0,
       0,     0,   175,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,     0,    77,    78,    79,    80,    81,
      82,    83,     0,    84,    85,    86,    87,     0,     0,     0,
     176,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,     0,    77,    78,    79,    80,    81,    82,    83,
       0,    84,    85,    86,    87,     0,     0,   156,   177,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
       0,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,   157,     0,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,     0,    77,    78,    79,
      80,    81,    82,    83,     0,    84,    85,    86,    87,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
       0,    77,    78,    79,    80,    81,    82,    83,     0,    84,
      85,    86,    87,-32768,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,     0,    77,    78,    79,    80,    81,
      82,    83,     0,    84,    85,    86,    87,    78,    79,    80,
      81,    82,    83,     0,    84,    85,    86,    87,    80,    81,
      82,    83,     0,    84,    85,    86,    87
};

static const short yycheck[] =
{
       0,    16,    35,    35,    35,    91,    35,    12,    13,    46,
      47,    11,    49,    50,    51,    52,    53,    54,    55,    60,
      57,    58,    59,    60,    65,    66,    60,    33,    28,    33,
      30,    65,    64,    60,    67,    66,    36,    66,    60,    60,
      60,    60,    42,    64,    65,    45,    46,    47,    48,   135,
      65,    51,     0,    53,    54,    55,    56,    57,    58,    59,
      16,    61,    57,    58,    59,    60,    66,    67,    68,    69,
      70,    71,    72,    73,    60,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    60,    33,    60,    60,    60,    60,
      90,    60,    60,    60,    42,    60,    60,    12,    64,    35,
       0,    49,    50,    64,    52,    65,     0,    -1,    -1,    -1,
     110,    74,   112,    35,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    74,    49,    50,    51,
      52,    53,    54,    55,   134,    57,    58,    59,    60,    -1,
      -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,   152,    -1,   154,    -1,   156,   157,     3,    -1,
      -1,     6,     7,     8,     9,    10,    11,    12,    13,    -1,
     170,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    47,    34,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    58,
      59,    60,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,     3,    -1,
      65,     6,     7,     8,     9,    10,    11,    12,    13,    -1,
      -1,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    58,
      59,    60,    -1,    -1,    -1,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    -1,    35,
      65,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    58,    59,    60,    -1,    -1,    -1,    64,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      58,    59,    60,    -1,    -1,    -1,    64,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    58,    59,
      60,    -1,    -1,    -1,    64,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    58,    59,    60,    -1,
      -1,    -1,    64,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    58,    59,    60,    -1,    -1,    -1,
      64,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    58,    59,    60,    -1,    -1,    -1,    64,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      58,    59,    60,    -1,    -1,    -1,    64,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    58,    59,
      60,    -1,    -1,    -1,    64,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    58,    59,    60,    -1,
      -1,    -1,    64,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    58,    59,    60,    -1,    -1,    -1,
      64,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    58,    59,    60,    -1,    -1,    -1,    64,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      58,    59,    60,    -1,    -1,    -1,    64,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    58,    59,
      60,    -1,    -1,    -1,    64,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    58,    59,    60,    -1,
      -1,    -1,    64,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    58,    59,    60,    -1,    -1,    -1,
      64,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    50,    51,    52,    53,    54,    55,
      -1,    57,    58,    59,    60,    -1,    -1,    35,    64,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      58,    59,    60,    35,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    58,    59,    60,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      58,    59,    60,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    50,    51,    52,    53,
      54,    55,    -1,    57,    58,    59,    60,    50,    51,    52,
      53,    54,    55,    -1,    57,    58,    59,    60,    52,    53,
      54,    55,    -1,    57,    58,    59,    60
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

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

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or xmalloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC xmalloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
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
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


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
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to xreallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

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
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to xreallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 3:
#line 210 "m2-exp.y"
{ write_exp_elt_opcode(OP_TYPE);
		  write_exp_elt_type(yyvsp[0].tval);
		  write_exp_elt_opcode(OP_TYPE);
		}
    break;
case 4:
#line 219 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_IND); }
    break;
case 5:
#line 223 "m2-exp.y"
{ number_sign = -1; }
    break;
case 6:
#line 225 "m2-exp.y"
{ number_sign = 1;
			  write_exp_elt_opcode (UNOP_NEG); }
    break;
case 7:
#line 230 "m2-exp.y"
{ write_exp_elt_opcode(UNOP_PLUS); }
    break;
case 8:
#line 234 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_LOGICAL_NOT); }
    break;
case 11:
#line 242 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_CAP); }
    break;
case 12:
#line 246 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_ORD); }
    break;
case 13:
#line 250 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_ABS); }
    break;
case 14:
#line 254 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_HIGH); }
    break;
case 15:
#line 258 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_MIN);
			  write_exp_elt_type (yyvsp[-1].tval);
			  write_exp_elt_opcode (UNOP_MIN); }
    break;
case 16:
#line 264 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_MAX);
			  write_exp_elt_type (yyvsp[-1].tval);
			  write_exp_elt_opcode (UNOP_MIN); }
    break;
case 17:
#line 270 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_FLOAT); }
    break;
case 18:
#line 274 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_VAL);
			  write_exp_elt_type (yyvsp[-3].tval);
			  write_exp_elt_opcode (BINOP_VAL); }
    break;
case 19:
#line 280 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_CHR); }
    break;
case 20:
#line 284 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_ODD); }
    break;
case 21:
#line 288 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_TRUNC); }
    break;
case 22:
#line 292 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_SIZEOF); }
    break;
case 23:
#line 297 "m2-exp.y"
{ write_exp_elt_opcode(UNOP_PREINCREMENT); }
    break;
case 24:
#line 301 "m2-exp.y"
{ write_exp_elt_opcode(BINOP_ASSIGN_MODIFY);
			  write_exp_elt_opcode(BINOP_ADD);
			  write_exp_elt_opcode(BINOP_ASSIGN_MODIFY); }
    break;
case 25:
#line 307 "m2-exp.y"
{ write_exp_elt_opcode(UNOP_PREDECREMENT);}
    break;
case 26:
#line 311 "m2-exp.y"
{ write_exp_elt_opcode(BINOP_ASSIGN_MODIFY);
			  write_exp_elt_opcode(BINOP_SUB);
			  write_exp_elt_opcode(BINOP_ASSIGN_MODIFY); }
    break;
case 27:
#line 317 "m2-exp.y"
{ write_exp_elt_opcode (STRUCTOP_STRUCT);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_STRUCT); }
    break;
case 29:
#line 326 "m2-exp.y"
{ error("Sets are not implemented.");}
    break;
case 30:
#line 330 "m2-exp.y"
{ error("Sets are not implemented.");}
    break;
case 31:
#line 334 "m2-exp.y"
{ error("Sets are not implemented.");}
    break;
case 32:
#line 338 "m2-exp.y"
{ error("Sets are not implemented.");}
    break;
case 33:
#line 340 "m2-exp.y"
{ error("Sets are not implemented.");}
    break;
case 34:
#line 349 "m2-exp.y"
{ start_arglist(); }
    break;
case 35:
#line 351 "m2-exp.y"
{ write_exp_elt_opcode (MULTI_SUBSCRIPT);
			  write_exp_elt_longcst ((LONGEST) end_arglist());
			  write_exp_elt_opcode (MULTI_SUBSCRIPT); }
    break;
case 36:
#line 359 "m2-exp.y"
{ start_arglist (); }
    break;
case 37:
#line 361 "m2-exp.y"
{ write_exp_elt_opcode (OP_FUNCALL);
			  write_exp_elt_longcst ((LONGEST) end_arglist ());
			  write_exp_elt_opcode (OP_FUNCALL); }
    break;
case 39:
#line 370 "m2-exp.y"
{ arglist_len = 1; }
    break;
case 40:
#line 374 "m2-exp.y"
{ arglist_len++; }
    break;
case 41:
#line 379 "m2-exp.y"
{ arglist_len = 1; }
    break;
case 42:
#line 384 "m2-exp.y"
{ arglist_len++; }
    break;
case 43:
#line 389 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_MEMVAL);
			  write_exp_elt_type (yyvsp[-2].tval);
			  write_exp_elt_opcode (UNOP_MEMVAL); }
    break;
case 44:
#line 395 "m2-exp.y"
{ write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (yyvsp[-3].tval);
			  write_exp_elt_opcode (UNOP_CAST); }
    break;
case 45:
#line 401 "m2-exp.y"
{ }
    break;
case 46:
#line 409 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_REPEAT); }
    break;
case 47:
#line 413 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_MUL); }
    break;
case 48:
#line 417 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_DIV); }
    break;
case 49:
#line 421 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_INTDIV); }
    break;
case 50:
#line 425 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_REM); }
    break;
case 51:
#line 429 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_ADD); }
    break;
case 52:
#line 433 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_SUB); }
    break;
case 53:
#line 437 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_EQUAL); }
    break;
case 54:
#line 441 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_NOTEQUAL); }
    break;
case 55:
#line 443 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_NOTEQUAL); }
    break;
case 56:
#line 447 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_LEQ); }
    break;
case 57:
#line 451 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_GEQ); }
    break;
case 58:
#line 455 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_LESS); }
    break;
case 59:
#line 459 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_GTR); }
    break;
case 60:
#line 463 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_LOGICAL_AND); }
    break;
case 61:
#line 467 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_LOGICAL_OR); }
    break;
case 62:
#line 471 "m2-exp.y"
{ write_exp_elt_opcode (BINOP_ASSIGN); }
    break;
case 63:
#line 478 "m2-exp.y"
{ write_exp_elt_opcode (OP_BOOL);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].ulval);
			  write_exp_elt_opcode (OP_BOOL); }
    break;
case 64:
#line 484 "m2-exp.y"
{ write_exp_elt_opcode (OP_BOOL);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].ulval);
			  write_exp_elt_opcode (OP_BOOL); }
    break;
case 65:
#line 490 "m2-exp.y"
{ write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (builtin_type_m2_int);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].lval);
			  write_exp_elt_opcode (OP_LONG); }
    break;
case 66:
#line 497 "m2-exp.y"
{
			  write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (builtin_type_m2_card);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].ulval);
			  write_exp_elt_opcode (OP_LONG);
			}
    break;
case 67:
#line 506 "m2-exp.y"
{ write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (builtin_type_m2_char);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].ulval);
			  write_exp_elt_opcode (OP_LONG); }
    break;
case 68:
#line 514 "m2-exp.y"
{ write_exp_elt_opcode (OP_DOUBLE);
			  write_exp_elt_type (builtin_type_m2_real);
			  write_exp_elt_dblcst (yyvsp[0].dval);
			  write_exp_elt_opcode (OP_DOUBLE); }
    break;
case 70:
#line 524 "m2-exp.y"
{ write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (builtin_type_int);
			  write_exp_elt_longcst ((LONGEST) TYPE_LENGTH (yyvsp[-1].tval));
			  write_exp_elt_opcode (OP_LONG); }
    break;
case 71:
#line 531 "m2-exp.y"
{ write_exp_elt_opcode (OP_M2_STRING);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (OP_M2_STRING); }
    break;
case 72:
#line 538 "m2-exp.y"
{ yyval.bval = SYMBOL_BLOCK_VALUE(yyvsp[0].sym); }
    break;
case 73:
#line 542 "m2-exp.y"
{ struct symbol *sym
			    = lookup_symbol (copy_name (yyvsp[0].sval), expression_context_block,
					     VAR_DOMAIN, 0, NULL);
			  yyval.sym = sym;}
    break;
case 74:
#line 551 "m2-exp.y"
{ struct symbol *tem
			    = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					     VAR_DOMAIN, 0, NULL);
			  if (!tem || SYMBOL_CLASS (tem) != LOC_BLOCK)
			    error ("No function \"%s\" in specified context.",
				   copy_name (yyvsp[0].sval));
			  yyval.sym = tem;
			}
    break;
case 75:
#line 563 "m2-exp.y"
{ write_exp_elt_opcode(OP_VAR_VALUE);
			  write_exp_elt_block (NULL);
			  write_exp_elt_sym (yyvsp[0].sym);
			  write_exp_elt_opcode (OP_VAR_VALUE); }
    break;
case 77:
#line 575 "m2-exp.y"
{ struct symbol *sym;
			  sym = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					       VAR_DOMAIN, 0, NULL);
			  if (sym == 0)
			    error ("No symbol \"%s\" in specified context.",
				   copy_name (yyvsp[0].sval));

			  write_exp_elt_opcode (OP_VAR_VALUE);
			  /* block_found is set by lookup_symbol.  */
			  write_exp_elt_block (block_found);
			  write_exp_elt_sym (sym);
			  write_exp_elt_opcode (OP_VAR_VALUE); }
    break;
case 78:
#line 591 "m2-exp.y"
{ struct symbol *sym;
			  int is_a_field_of_this;

 			  sym = lookup_symbol (copy_name (yyvsp[0].sval),
					       expression_context_block,
					       VAR_DOMAIN,
					       &is_a_field_of_this,
					       NULL);
			  if (sym)
			    {
			      if (symbol_read_needs_frame (sym))
				{
				  if (innermost_block == 0 ||
				      contained_in (block_found, 
						    innermost_block))
				    innermost_block = block_found;
				}

			      write_exp_elt_opcode (OP_VAR_VALUE);
			      /* We want to use the selected frame, not
				 another more inner frame which happens to
				 be in the same block.  */
			      write_exp_elt_block (NULL);
			      write_exp_elt_sym (sym);
			      write_exp_elt_opcode (OP_VAR_VALUE);
			    }
			  else
			    {
			      struct minimal_symbol *msymbol;
			      char *arg = copy_name (yyvsp[0].sval);

			      msymbol =
				lookup_minimal_symbol (arg, NULL, NULL);
			      if (msymbol != NULL)
				{
				  write_exp_msymbol
				    (msymbol,
				     lookup_function_type (builtin_type_int),
				     builtin_type_int);
				}
			      else if (!have_full_symbols () && !have_partial_symbols ())
				error ("No symbol table is loaded.  Use the \"symbol-file\" command.");
			      else
				error ("No symbol \"%s\" in current context.",
				       copy_name (yyvsp[0].sval));
			    }
			}
    break;
case 79:
#line 642 "m2-exp.y"
{ yyval.tval = lookup_typename (copy_name (yyvsp[0].sval),
						expression_context_block, 0); }
    break;
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

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

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 647 "m2-exp.y"


#if 0  /* FIXME! */
int
overflow(a,b)
   long a,b;
{
   return (MAX_OF_TYPE(builtin_type_m2_int) - b) < a;
}

int
uoverflow(a,b)
   unsigned long a,b;
{
   return (MAX_OF_TYPE(builtin_type_m2_card) - b) < a;
}
#endif /* FIXME */

/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/*** Needs some error checking for the float case ***/

static int
parse_number (olen)
     int olen;
{
  char *p = lexptr;
  LONGEST n = 0;
  LONGEST prevn = 0;
  int c,i,ischar=0;
  int base = input_radix;
  int len = olen;
  int unsigned_p = number_sign == 1 ? 1 : 0;

  if(p[len-1] == 'H')
  {
     base = 16;
     len--;
  }
  else if(p[len-1] == 'C' || p[len-1] == 'B')
  {
     base = 8;
     ischar = p[len-1] == 'C';
     len--;
  }

  /* Scan the number */
  for (c = 0; c < len; c++)
  {
    if (p[c] == '.' && base == 10)
      {
	/* It's a float since it contains a point.  */
	yylval.dval = atof (p);
	lexptr += len;
	return FLOAT;
      }
    if (p[c] == '.' && base != 10)
       error("Floating point numbers must be base 10.");
    if (base == 10 && (p[c] < '0' || p[c] > '9'))
       error("Invalid digit \'%c\' in number.",p[c]);
 }

  while (len-- > 0)
    {
      c = *p++;
      n *= base;
      if( base == 8 && (c == '8' || c == '9'))
	 error("Invalid digit \'%c\' in octal number.",c);
      if (c >= '0' && c <= '9')
	i = c - '0';
      else
	{
	  if (base == 16 && c >= 'A' && c <= 'F')
	    i = c - 'A' + 10;
	  else
	     return ERROR;
	}
      n+=i;
      if(i >= base)
	 return ERROR;
      if(!unsigned_p && number_sign == 1 && (prevn >= n))
	 unsigned_p=1;		/* Try something unsigned */
      /* Don't do the range check if n==i and i==0, since that special
	 case will give an overflow error. */
      if(RANGE_CHECK && n!=i && i)
      {
	 if((unsigned_p && (unsigned)prevn >= (unsigned)n) ||
	    ((!unsigned_p && number_sign==-1) && -prevn <= -n))
	    range_error("Overflow on numeric constant.");
      }
	 prevn=n;
    }

  lexptr = p;
  if(*p == 'B' || *p == 'C' || *p == 'H')
     lexptr++;			/* Advance past B,C or H */

  if (ischar)
  {
     yylval.ulval = n;
     return CHAR;
  }
  else if ( unsigned_p && number_sign == 1)
  {
     yylval.ulval = n;
     return UINT;
  }
  else if((unsigned_p && (n<0))) {
     range_error("Overflow on numeric constant -- number too large.");
     /* But, this can return if range_check == range_warn.  */
  }
  yylval.lval = n;
  return INT;
}


/* Some tokens */

static struct
{
   char name[2];
   int token;
} tokentab2[] =
{
    { {'<', '>'},    NOTEQUAL 	},
    { {':', '='},    ASSIGN	},
    { {'<', '='},    LEQ	},
    { {'>', '='},    GEQ	},
    { {':', ':'},    COLONCOLON },

};

/* Some specific keywords */

struct keyword {
   char keyw[10];
   int token;
};

static struct keyword keytab[] =
{
    {"OR" ,   OROR	 },
    {"IN",    IN         },/* Note space after IN */
    {"AND",   LOGICAL_AND},
    {"ABS",   ABS	 },
    {"CHR",   CHR	 },
    {"DEC",   DEC	 },
    {"NOT",   NOT	 },
    {"DIV",   DIV    	 },
    {"INC",   INC	 },
    {"MAX",   MAX_FUNC	 },
    {"MIN",   MIN_FUNC	 },
    {"MOD",   MOD	 },
    {"ODD",   ODD	 },
    {"CAP",   CAP	 },
    {"ORD",   ORD	 },
    {"VAL",   VAL	 },
    {"EXCL",  EXCL	 },
    {"HIGH",  HIGH       },
    {"INCL",  INCL	 },
    {"SIZE",  SIZE       },
    {"FLOAT", FLOAT_FUNC },
    {"TRUNC", TRUNC	 },
};


/* Read one token, getting characters through lexptr.  */

/* This is where we will check to make sure that the language and the operators used are
   compatible  */

static int
yylex ()
{
  int c;
  int namelen;
  int i;
  char *tokstart;
  char quote;

 retry:

  prev_lexptr = lexptr;

  tokstart = lexptr;


  /* See if it is a special token of length 2 */
  for( i = 0 ; i < (int) (sizeof tokentab2 / sizeof tokentab2[0]) ; i++)
     if(DEPRECATED_STREQN(tokentab2[i].name, tokstart, 2))
     {
	lexptr += 2;
	return tokentab2[i].token;
     }

  switch (c = *tokstart)
    {
    case 0:
      return 0;

    case ' ':
    case '\t':
    case '\n':
      lexptr++;
      goto retry;

    case '(':
      paren_depth++;
      lexptr++;
      return c;

    case ')':
      if (paren_depth == 0)
	return 0;
      paren_depth--;
      lexptr++;
      return c;

    case ',':
      if (comma_terminates && paren_depth == 0)
	return 0;
      lexptr++;
      return c;

    case '.':
      /* Might be a floating point number.  */
      if (lexptr[1] >= '0' && lexptr[1] <= '9')
	break;			/* Falls into number code.  */
      else
      {
	 lexptr++;
	 return DOT;
      }

/* These are character tokens that appear as-is in the YACC grammar */
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
    case '<':
    case '>':
    case '[':
    case ']':
    case '=':
    case '{':
    case '}':
    case '#':
    case '@':
    case '~':
    case '&':
      lexptr++;
      return c;

    case '\'' :
    case '"':
      quote = c;
      for (namelen = 1; (c = tokstart[namelen]) != quote && c != '\0'; namelen++)
	if (c == '\\')
	  {
	    c = tokstart[++namelen];
	    if (c >= '0' && c <= '9')
	      {
		c = tokstart[++namelen];
		if (c >= '0' && c <= '9')
		  c = tokstart[++namelen];
	      }
	  }
      if(c != quote)
	 error("Unterminated string or character constant.");
      yylval.sval.ptr = tokstart + 1;
      yylval.sval.length = namelen - 1;
      lexptr += namelen + 1;

      if(namelen == 2)  	/* Single character */
      {
	   yylval.ulval = tokstart[1];
	   return CHAR;
      }
      else
	 return STRING;
    }

  /* Is it a number?  */
  /* Note:  We have already dealt with the case of the token '.'.
     See case '.' above.  */
  if ((c >= '0' && c <= '9'))
    {
      /* It's a number.  */
      int got_dot = 0, got_e = 0;
      char *p = tokstart;
      int toktype;

      for (++p ;; ++p)
	{
	  if (!got_e && (*p == 'e' || *p == 'E'))
	    got_dot = got_e = 1;
	  else if (!got_dot && *p == '.')
	    got_dot = 1;
	  else if (got_e && (p[-1] == 'e' || p[-1] == 'E')
		   && (*p == '-' || *p == '+'))
	    /* This is the sign of the exponent, not the end of the
	       number.  */
	    continue;
	  else if ((*p < '0' || *p > '9') &&
		   (*p < 'A' || *p > 'F') &&
		   (*p != 'H'))  /* Modula-2 hexadecimal number */
	    break;
	}
	toktype = parse_number (p - tokstart);
        if (toktype == ERROR)
	  {
	    char *err_copy = (char *) alloca (p - tokstart + 1);

	    memcpy (err_copy, tokstart, p - tokstart);
	    err_copy[p - tokstart] = 0;
	    error ("Invalid number \"%s\".", err_copy);
	  }
	lexptr = p;
	return toktype;
    }

  if (!(c == '_' || c == '$'
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
    /* We must have come across a bad character (e.g. ';').  */
    error ("Invalid character '%c' in expression.", c);

  /* It's a name.  See how long it is.  */
  namelen = 0;
  for (c = tokstart[namelen];
       (c == '_' || c == '$' || (c >= '0' && c <= '9')
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
       c = tokstart[++namelen])
    ;

  /* The token "if" terminates the expression and is NOT
     removed from the input stream.  */
  if (namelen == 2 && tokstart[0] == 'i' && tokstart[1] == 'f')
    {
      return 0;
    }

  lexptr += namelen;

  /*  Lookup special keywords */
  for(i = 0 ; i < (int) (sizeof(keytab) / sizeof(keytab[0])) ; i++)
     if(namelen == strlen(keytab[i].keyw) && DEPRECATED_STREQN(tokstart,keytab[i].keyw,namelen))
	   return keytab[i].token;

  yylval.sval.ptr = tokstart;
  yylval.sval.length = namelen;

  if (*tokstart == '$')
    {
      write_dollar_variable (yylval.sval);
      return INTERNAL_VAR;
    }

  /* Use token-type BLOCKNAME for symbols that happen to be defined as
     functions.  If this is not so, then ...
     Use token-type TYPENAME for symbols that happen to be defined
     currently as names of types; NAME for other symbols.
     The caller is not constrained to care about the distinction.  */
 {


    char *tmp = copy_name (yylval.sval);
    struct symbol *sym;

    if (lookup_partial_symtab (tmp))
      return BLOCKNAME;
    sym = lookup_symbol (tmp, expression_context_block,
			 VAR_DOMAIN, 0, NULL);
    if (sym && SYMBOL_CLASS (sym) == LOC_BLOCK)
      return BLOCKNAME;
    if (lookup_typename (copy_name (yylval.sval), expression_context_block, 1))
      return TYPENAME;

    if(sym)
    {
       switch(sym->aclass)
       {
       case LOC_STATIC:
       case LOC_REGISTER:
       case LOC_ARG:
       case LOC_REF_ARG:
       case LOC_REGPARM:
       case LOC_REGPARM_ADDR:
       case LOC_LOCAL:
       case LOC_LOCAL_ARG:
       case LOC_BASEREG:
       case LOC_BASEREG_ARG:
       case LOC_CONST:
       case LOC_CONST_BYTES:
       case LOC_OPTIMIZED_OUT:
       case LOC_COMPUTED:
       case LOC_COMPUTED_ARG:
	  return NAME;

       case LOC_TYPEDEF:
	  return TYPENAME;

       case LOC_BLOCK:
	  return BLOCKNAME;

       case LOC_UNDEF:
	  error("internal:  Undefined class in m2lex()");

       case LOC_LABEL:
       case LOC_UNRESOLVED:
	  error("internal:  Unforseen case in m2lex()");

       default:
	  error ("unhandled token in m2lex()");
	  break;
       }
    }
    else
    {
       /* Built-in BOOLEAN type.  This is sort of a hack. */
       if(DEPRECATED_STREQN(tokstart,"TRUE",4))
       {
	  yylval.ulval = 1;
	  return M2_TRUE;
       }
       else if(DEPRECATED_STREQN(tokstart,"FALSE",5))
       {
	  yylval.ulval = 0;
	  return M2_FALSE;
       }
    }

    /* Must be another type of name... */
    return NAME;
 }
}

#if 0		/* Unused */
static char *
make_qualname(mod,ident)
   char *mod, *ident;
{
   char *new = xmalloc(strlen(mod)+strlen(ident)+2);

   strcpy(new,mod);
   strcat(new,".");
   strcat(new,ident);
   return new;
}
#endif  /* 0 */

void
yyerror (msg)
     char *msg;
{
  if (prev_lexptr)
    lexptr = prev_lexptr;

  error ("A %s in expression, near `%s'.", (msg ? msg : "error"), lexptr);
}
