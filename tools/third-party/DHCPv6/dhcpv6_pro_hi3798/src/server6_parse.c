/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         server6parse
#define yylex           server6lex
#define yyerror         server6error
#define yylval          server6lval
#define yychar          server6char
#define yydebug         server6debug
#define yynerrs         server6nerrs


/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 32 "server6_parse.y"

#include "config.h"

#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <syslog.h>
#include <netinet/in.h>
#include <net/if.h>

#include "dhcp6.h"
#include "confdata.h"
#include "server6_conf.h"
#include "common.h"
#include "lease.h"
#include "hash.h"

extern int server6lex (void);

extern int num_lines;
extern int sock;
static struct interface *ifnetworklist = NULL;
static struct link_decl *linklist = NULL;
static struct host_decl *hostlist = NULL;
static struct pool_decl *poollist = NULL;

static struct interface *ifnetwork = NULL;
static struct link_decl *link = NULL;
static struct host_decl *host = NULL;
static struct pool_decl *pool = NULL;
static struct scopelist *currentscope = NULL;
static struct scopelist *currentgroup = NULL;
static int allow = 0;

static void cleanup(void);
extern void server6error(char *, ...)
        __attribute__((__format__(__printf__, 1, 2)));

#define ABORT	do { cleanup(); YYABORT; } while (0)

extern int server6_tokenlex(void);


/* Line 268 of yacc.c  */
#line 123 "server6_parse.c"

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
     INTERFACE = 258,
     IFNAME = 259,
     PREFIX = 260,
     LINK = 261,
     RELAY = 262,
     STRING = 263,
     NUMBER = 264,
     SIGNEDNUMBER = 265,
     DECIMAL = 266,
     BOOLEAN = 267,
     IPV6ADDR = 268,
     INFINITY = 269,
     HOST = 270,
     POOL = 271,
     RANGE = 272,
     GROUP = 273,
     LINKLOCAL = 274,
     OPTION = 275,
     ALLOW = 276,
     SEND = 277,
     PREFERENCE = 278,
     RENEWTIME = 279,
     REBINDTIME = 280,
     RAPIDCOMMIT = 281,
     ADDRESS = 282,
     VALIDLIFETIME = 283,
     PREFERLIFETIME = 284,
     UNICAST = 285,
     TEMPIPV6ADDR = 286,
     DNS_SERVERS = 287,
     DUID = 288,
     DUID_ID = 289,
     IAID = 290,
     IAIDINFO = 291,
     INFO_ONLY = 292,
     INFO_REFRESH_TIME = 293,
     TO = 294,
     BAD_TOKEN = 295
   };
#endif
/* Tokens.  */
#define INTERFACE 258
#define IFNAME 259
#define PREFIX 260
#define LINK 261
#define RELAY 262
#define STRING 263
#define NUMBER 264
#define SIGNEDNUMBER 265
#define DECIMAL 266
#define BOOLEAN 267
#define IPV6ADDR 268
#define INFINITY 269
#define HOST 270
#define POOL 271
#define RANGE 272
#define GROUP 273
#define LINKLOCAL 274
#define OPTION 275
#define ALLOW 276
#define SEND 277
#define PREFERENCE 278
#define RENEWTIME 279
#define REBINDTIME 280
#define RAPIDCOMMIT 281
#define ADDRESS 282
#define VALIDLIFETIME 283
#define PREFERLIFETIME 284
#define UNICAST 285
#define TEMPIPV6ADDR 286
#define DNS_SERVERS 287
#define DUID 288
#define DUID_ID 289
#define IAID 290
#define IAIDINFO 291
#define INFO_ONLY 292
#define INFO_REFRESH_TIME 293
#define TO 294
#define BAD_TOKEN 295




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 114 "server6_parse.y"

	unsigned int	num;
	int 	snum;
	char	*str;
	int 	dec;
	int	bool;
	struct in6_addr	addr;
	struct dhcp6_addr *dhcp6addr;



/* Line 293 of yacc.c  */
#line 251 "server6_parse.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 263 "server6_parse.c"

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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

# define YYCOPY_NEEDED 1

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

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
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
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   215

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  45
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  45
/* YYNRULES -- Number of rules.  */
#define YYNRULES  99
/* YYNRULES -- Number of states.  */
#define YYNSTATES  179

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   295

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    44,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    43,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    41,     2,    42,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    21,
      24,    25,    28,    30,    32,    34,    36,    42,    45,    46,
      49,    51,    53,    55,    57,    59,    61,    63,    66,    68,
      74,    80,    82,    83,    86,    88,    90,    92,    94,    96,
      98,   104,   112,   118,   119,   122,   124,   126,   128,   130,
     132,   134,   136,   138,   144,   147,   150,   152,   156,   158,
     160,   166,   171,   176,   178,   182,   185,   187,   189,   191,
     193,   199,   205,   210,   215,   217,   222,   225,   227,   229,
     231,   234,   237,   240,   243,   247,   251,   254,   256,   258,
     260,   262,   264,   268,   272,   276,   280,   284,   286,   288
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      46,     0,    -1,    -1,    46,    47,    -1,    48,    -1,    64,
      -1,    86,    -1,    52,    -1,    49,    41,    50,    42,    43,
      -1,     3,     4,    -1,    -1,    50,    51,    -1,    52,    -1,
      68,    -1,    64,    -1,    86,    -1,    53,    41,    54,    42,
      43,    -1,     6,    89,    -1,    -1,    54,    55,    -1,    58,
      -1,    63,    -1,    62,    -1,    68,    -1,    64,    -1,    86,
      -1,    56,    -1,    56,    57,    -1,    57,    -1,     7,    13,
      44,     9,    43,    -1,    59,    41,    60,    42,    43,    -1,
      16,    -1,    -1,    60,    61,    -1,    68,    -1,    64,    -1,
      63,    -1,    62,    -1,    86,    -1,    56,    -1,     5,    13,
      44,     9,    43,    -1,    17,    13,    39,    13,    44,     9,
      43,    -1,    67,    41,    65,    42,    43,    -1,    -1,    65,
      66,    -1,    68,    -1,    58,    -1,    52,    -1,    63,    -1,
      62,    -1,    48,    -1,    86,    -1,    18,    -1,    69,    41,
      70,    42,    43,    -1,    15,    89,    -1,    70,    71,    -1,
      71,    -1,    33,    34,    43,    -1,    72,    -1,    75,    -1,
      36,    41,    73,    42,    43,    -1,    73,    24,    88,    43,
      -1,    73,    25,    88,    43,    -1,    74,    -1,    35,     9,
      43,    -1,    75,    76,    -1,    76,    -1,    77,    -1,    78,
      -1,    81,    -1,    27,    41,    79,    42,    43,    -1,     5,
      41,    79,    42,    43,    -1,    79,    28,    88,    43,    -1,
      79,    29,    88,    43,    -1,    80,    -1,    13,    44,     9,
      43,    -1,    82,    83,    -1,    22,    -1,    21,    -1,    20,
      -1,    26,    43,    -1,    31,    43,    -1,    30,    43,    -1,
      37,    43,    -1,    38,    88,    43,    -1,    32,    84,    43,
      -1,    84,    85,    -1,    85,    -1,    13,    -1,     8,    -1,
      87,    -1,    81,    -1,    24,    88,    43,    -1,    25,    88,
      43,    -1,    28,    88,    43,    -1,    29,    88,    43,    -1,
      23,     9,    43,    -1,     9,    -1,    14,    -1,     8,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   124,   124,   126,   130,   131,   132,   133,   137,   163,
     202,   204,   208,   209,   210,   211,   215,   233,   268,   270,
     274,   275,   276,   277,   278,   279,   280,   284,   285,   289,
     311,   324,   347,   349,   353,   354,   355,   356,   357,   358,
     362,   405,   500,   510,   512,   516,   517,   518,   519,   520,
     521,   522,   526,   549,   574,   604,   605,   609,   617,   618,
     622,   628,   632,   636,   640,   651,   652,   656,   670,   678,
     682,   690,   698,   702,   706,   713,   733,   737,   745,   754,
     765,   772,   779,   786,   793,   808,   814,   815,   819,   824,
     855,   856,   860,   869,   878,   894,   910,   924,   928,   935
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INTERFACE", "IFNAME", "PREFIX", "LINK",
  "RELAY", "STRING", "NUMBER", "SIGNEDNUMBER", "DECIMAL", "BOOLEAN",
  "IPV6ADDR", "INFINITY", "HOST", "POOL", "RANGE", "GROUP", "LINKLOCAL",
  "OPTION", "ALLOW", "SEND", "PREFERENCE", "RENEWTIME", "REBINDTIME",
  "RAPIDCOMMIT", "ADDRESS", "VALIDLIFETIME", "PREFERLIFETIME", "UNICAST",
  "TEMPIPV6ADDR", "DNS_SERVERS", "DUID", "DUID_ID", "IAID", "IAIDINFO",
  "INFO_ONLY", "INFO_REFRESH_TIME", "TO", "BAD_TOKEN", "'{'", "'}'", "';'",
  "'/'", "$accept", "statements", "networkdef", "ifdef", "ifhead",
  "ifbody", "ifparams", "linkdef", "linkhead", "linkbody", "linkparams",
  "relaylist", "relaypara", "pooldef", "poolhead", "poolbody", "poolparas",
  "prefixdef", "rangedef", "groupdef", "groupbody", "groupparas",
  "grouphead", "hostdef", "hosthead", "hostbody", "hostdecl", "iaiddef",
  "iaidbody", "iaidpara", "hostparas", "hostpara", "hostaddr6",
  "hostprefix6", "addr6para", "v6address", "optiondecl", "optionhead",
  "optionpara", "dns_paras", "dns_para", "confdecl", "paradecl",
  "number_or_infinity", "name", 0
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
     295,   123,   125,    59,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    45,    46,    46,    47,    47,    47,    47,    48,    49,
      50,    50,    51,    51,    51,    51,    52,    53,    54,    54,
      55,    55,    55,    55,    55,    55,    55,    56,    56,    57,
      58,    59,    60,    60,    61,    61,    61,    61,    61,    61,
      62,    63,    64,    65,    65,    66,    66,    66,    66,    66,
      66,    66,    67,    68,    69,    70,    70,    71,    71,    71,
      72,    73,    73,    73,    74,    75,    75,    76,    76,    76,
      77,    78,    79,    79,    79,    80,    81,    82,    82,    82,
      83,    83,    83,    83,    83,    83,    84,    84,    85,    85,
      86,    86,    87,    87,    87,    87,    87,    88,    88,    89
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     5,     2,
       0,     2,     1,     1,     1,     1,     5,     2,     0,     2,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     5,
       5,     1,     0,     2,     1,     1,     1,     1,     1,     1,
       5,     7,     5,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     5,     2,     2,     1,     3,     1,     1,
       5,     4,     4,     1,     3,     2,     1,     1,     1,     1,
       5,     5,     4,     4,     1,     4,     2,     1,     1,     1,
       2,     2,     2,     2,     3,     3,     2,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,     0,    52,    79,    78,    77,     0,
       0,     0,     0,     0,     3,     4,     0,     7,     0,     5,
       0,    91,     0,     6,    90,     9,    99,    17,     0,    97,
      98,     0,     0,     0,     0,    10,    18,    43,     0,     0,
       0,     0,     0,     0,    76,    96,    92,    93,    94,    95,
       0,     0,     0,    80,    82,    81,    89,    88,     0,    87,
      83,     0,     0,     0,    11,    12,    14,    13,     0,    15,
       0,     0,    31,     0,     0,    19,    26,    28,    20,     0,
      22,    21,    24,    23,    25,     0,    50,    47,    46,    49,
      48,    44,    45,    51,    85,    86,    84,    54,     8,     0,
       0,     0,     0,    16,    27,    32,    42,     0,     0,     0,
       0,     0,    56,    58,    59,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    55,    65,
       0,     0,     0,     0,    39,    33,    37,    36,    35,    34,
      38,     0,     0,    74,     0,    57,     0,     0,    63,    53,
      40,    29,     0,    30,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    71,    70,    64,
       0,     0,    60,    41,    75,    72,    73,    61,    62
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    14,    15,    16,    50,    64,    17,    18,    51,
      75,    76,    77,    78,    79,   122,   135,    80,    81,    19,
      52,    91,    20,    67,    68,   111,   112,   113,   147,   148,
     114,   115,   116,   117,   142,   143,    21,    22,    44,    58,
      59,    23,    24,    31,    27
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -65
static const yytype_int16 yypact[] =
{
     -65,   154,   -65,    20,    23,   -65,   -65,   -65,   -65,    35,
       6,     6,     6,     6,   -65,   -65,     5,   -65,     8,   -65,
      12,   -65,    76,   -65,   -65,   -65,   -65,   -65,     4,   -65,
     -65,    14,    19,    26,    30,   -65,   -65,   -65,    34,    52,
      56,    32,    57,     6,   -65,   -65,   -65,   -65,   -65,   -65,
     106,    43,    13,   -65,   -65,   -65,   -65,   -65,     9,   -65,
     -65,    58,    23,    66,   -65,   -65,   -65,   -65,    15,   -65,
     102,   104,   -65,   105,    77,   -65,   115,   -65,   -65,    82,
     -65,   -65,   -65,   -65,   -65,    89,   -65,   -65,   -65,   -65,
     -65,   -65,   -65,   -65,   -65,   -65,   -65,   -65,   -65,   131,
      93,    94,   100,   -65,   -65,   -65,   -65,    92,    99,   107,
     101,    83,   -65,   -65,   141,   -65,   -65,   -65,   -65,   134,
     138,   142,    69,   143,   143,   116,   130,   123,   -65,   -65,
     126,   127,   129,   128,   115,   -65,   -65,   -65,   -65,   -65,
     -65,   136,   -15,   -65,    54,   -65,   172,     1,   -65,   -65,
     -65,   -65,   175,   -65,   176,     6,     6,   144,   145,   146,
       6,     6,   147,   148,   149,   150,   151,   -65,   -65,   -65,
     152,   153,   -65,   -65,   -65,   -65,   -65,   -65,   -65
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -65,   -65,   -65,   155,   -65,   -65,   -65,   -29,   -65,   -65,
     -65,    64,   -64,   156,   -65,   -65,   -65,   -42,   -41,   -44,
     -65,   -65,   -65,   -43,   -65,   -65,    86,   -65,   -65,   -65,
     -65,    84,   -65,   -65,    75,   -65,   -60,   -65,   -65,   -65,
     157,   -47,   -65,   -11,   139
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      32,    33,    34,    69,    84,    93,    66,    82,    83,    92,
      89,    90,   104,   155,   156,    29,     3,    56,    70,     4,
      30,    65,    57,    87,    25,   160,   161,   157,    62,    72,
      73,    26,    61,     6,     7,     8,     9,    10,    11,   118,
      56,    12,    13,   162,    28,    57,    35,    45,    70,    36,
      71,   118,    94,    37,   118,    85,    99,    46,    62,    72,
      73,     5,    47,     6,     7,     8,     9,    10,    11,    48,
     104,    12,    13,    49,    70,   140,    71,    53,   138,   139,
     136,   137,   155,   156,    62,    74,    73,     5,   107,     6,
       7,     8,     9,    10,    11,    54,   158,    12,    13,    55,
      60,    96,    38,     6,     7,     8,    39,    40,    41,    98,
     108,   133,     4,    42,    43,   100,   109,   101,   102,   110,
     103,    62,    71,   105,     5,   127,     6,     7,     8,     9,
      10,    11,   106,   123,    12,    13,   107,   119,   120,   121,
     124,   125,   126,   130,   165,   166,   107,   131,    63,   170,
     171,     6,     7,     8,     2,   132,   141,     3,   108,   145,
       4,     6,     7,     8,   109,   146,   149,   110,   108,   150,
     151,   153,     5,   152,     6,     7,     8,     9,    10,    11,
     154,   159,    12,    13,   163,   164,   134,   167,   168,   169,
     172,   173,   174,   175,   176,   177,   178,   128,   129,   144,
       0,    97,     0,     0,     0,     0,     0,    86,    88,     0,
       0,     0,     0,     0,     0,    95
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-65))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      11,    12,    13,    50,    51,    52,    50,    51,    51,    52,
      52,    52,    76,    28,    29,     9,     3,     8,     5,     6,
      14,    50,    13,    52,     4,    24,    25,    42,    15,    16,
      17,     8,    43,    20,    21,    22,    23,    24,    25,    99,
       8,    28,    29,    42,     9,    13,    41,    43,     5,    41,
       7,   111,    43,    41,   114,    42,    41,    43,    15,    16,
      17,    18,    43,    20,    21,    22,    23,    24,    25,    43,
     134,    28,    29,    43,     5,   122,     7,    43,   122,   122,
     122,   122,    28,    29,    15,    42,    17,    18,     5,    20,
      21,    22,    23,    24,    25,    43,    42,    28,    29,    43,
      43,    43,    26,    20,    21,    22,    30,    31,    32,    43,
      27,    42,     6,    37,    38,    13,    33,    13,    13,    36,
      43,    15,     7,    41,    18,    42,    20,    21,    22,    23,
      24,    25,    43,    41,    28,    29,     5,    44,    44,    39,
      41,    34,    41,     9,   155,   156,     5,     9,    42,   160,
     161,    20,    21,    22,     0,    13,    13,     3,    27,    43,
       6,    20,    21,    22,    33,    35,    43,    36,    27,    43,
      43,    43,    18,    44,    20,    21,    22,    23,    24,    25,
      44,     9,    28,    29,     9,     9,   122,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,   111,   114,   124,
      -1,    62,    -1,    -1,    -1,    -1,    -1,    52,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    58
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    46,     0,     3,     6,    18,    20,    21,    22,    23,
      24,    25,    28,    29,    47,    48,    49,    52,    53,    64,
      67,    81,    82,    86,    87,     4,     8,    89,     9,     9,
      14,    88,    88,    88,    88,    41,    41,    41,    26,    30,
      31,    32,    37,    38,    83,    43,    43,    43,    43,    43,
      50,    54,    65,    43,    43,    43,     8,    13,    84,    85,
      43,    88,    15,    42,    51,    52,    64,    68,    69,    86,
       5,     7,    16,    17,    42,    55,    56,    57,    58,    59,
      62,    63,    64,    68,    86,    42,    48,    52,    58,    62,
      63,    66,    68,    86,    43,    85,    43,    89,    43,    41,
      13,    13,    13,    43,    57,    41,    43,     5,    27,    33,
      36,    70,    71,    72,    75,    76,    77,    78,    81,    44,
      44,    39,    60,    41,    41,    34,    41,    42,    71,    76,
       9,     9,    13,    42,    56,    61,    62,    63,    64,    68,
      86,    13,    79,    80,    79,    43,    35,    73,    74,    43,
      43,    43,    44,    43,    44,    28,    29,    42,    42,     9,
      24,    25,    42,     9,     9,    88,    88,    43,    43,    43,
      88,    88,    43,    43,    43,    43,    43,    43,    43
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


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
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


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


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
  if (yypact_value_is_default (yyn))
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
      if (yytable_value_is_error (yyn))
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
        case 8:

/* Line 1806 of yacc.c  */
#line 138 "server6_parse.y"
    {
		if (linklist) {
			ifnetwork->linklist = linklist;
			linklist = NULL;
		}
		if (hostlist) {
			ifnetwork->hostlist = hostlist;
			hostlist = NULL;
		}
		if (currentgroup) 
			ifnetwork->group = currentgroup->scope;

		dhcpv6_dprintf(LOG_DEBUG, "interface definition for %s is ok", ifnetwork->name);
		ifnetwork->next = ifnetworklist;
		ifnetworklist = ifnetwork;
		ifnetwork = NULL;

		globalgroup->iflist = ifnetworklist;

		/* leave interface scope we know the current scope is not point to NULL*/
		currentscope = pop_double_list(currentscope);
	}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 164 "server6_parse.y"
    {
		struct interface *temp_if = ifnetworklist;
		while (temp_if)
		{
			if (!strcmp(temp_if->name, (yyvsp[(2) - (2)].str)))
			{
				dhcpv6_dprintf(LOG_ERR, "duplicate interface definition for %s",
					temp_if->name);
				ABORT;
			}
			temp_if = temp_if->next;
		}
		ifnetwork = (struct interface *)malloc(sizeof(*ifnetwork));
		if (ifnetwork == NULL) {
			dhcpv6_dprintf(LOG_ERR, "failed to allocate memory");
			ABORT;
		}
		memset(ifnetwork, 0, sizeof(*ifnetwork));
		TAILQ_INIT(&ifnetwork->ifscope.dnslist.addrlist);
		strncpy(ifnetwork->name, (yyvsp[(2) - (2)].str), strlen((yyvsp[(2) - (2)].str))); 
		if (get_linklocal(ifnetwork->name, &ifnetwork->linklocal) < 0) {
			dhcpv6_dprintf(LOG_ERR, "get device %s linklocal failed", ifnetwork->name);
		}
		
		/* check device, if the device is not available,
		 * it is OK, it might be added later
		 * so keep this in the configuration file.
		 */
		if (if_nametoindex(ifnetwork->name) == 0) {
			dhcpv6_dprintf(LOG_ERR, "this device %s doesn't exist.", (yyvsp[(2) - (2)].str));
		}
		/* set up hw_addr, link local, primary ipv6addr */
		/* enter interface scope */
		currentscope = push_double_list(currentscope, &ifnetwork->ifscope);
		if (currentscope == NULL)
			ABORT;
	}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 216 "server6_parse.y"
    {
		if (poollist) {
			link->poollist = poollist;
			poollist = NULL;
		}
		if (currentgroup) 
			link->group = currentgroup->scope;

		link->next = linklist;
		linklist = link;
		link = NULL;
		/* leave iink scope we know the current scope is not point to NULL*/
		currentscope = pop_double_list(currentscope);
	}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 234 "server6_parse.y"
    {
		struct link_decl *temp_sub = linklist;
		/* memory allocation for link */
		link = (struct link_decl *)malloc(sizeof(*link));
		if (link == NULL) {
			dhcpv6_dprintf(LOG_ERR, "failed to allocate memory");
			ABORT;
		}
		memset(link, 0, sizeof(*link));
		TAILQ_INIT(&link->linkscope.dnslist.addrlist);
		while (temp_sub) {
			if (!strcmp(temp_sub->name, (yyvsp[(2) - (2)].str)))
			{
				dhcpv6_dprintf(LOG_ERR, "duplicate link definition for %s", (yyvsp[(2) - (2)].str));
				ABORT;
			}
			temp_sub = temp_sub->next;
		}			
		/* link set */
		strncpy(link->name, (yyvsp[(2) - (2)].str), strlen((yyvsp[(2) - (2)].str)));
		if (ifnetwork)
			link->network = ifnetwork;
		else {
			/* create a ifnetwork for this interface */
		}
		link->relaylist = NULL;
		link->seglist = NULL;
		/* enter link scope */
		currentscope = push_double_list(currentscope, &link->linkscope);
		if (currentscope == NULL)
			ABORT;
	}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 290 "server6_parse.y"
    {
		struct v6addrlist *temprelay;
		if (!link) {
			dhcpv6_dprintf(LOG_ERR, "relay must be defined under link");
			ABORT;
		}
		temprelay = (struct v6addrlist *)malloc(sizeof(*temprelay));
		if (temprelay == NULL) {
			dhcpv6_dprintf(LOG_ERR, "failed to allocate memory");
			ABORT;
		}
		memset(temprelay, 0, sizeof(*temprelay));
		memcpy(&temprelay->v6addr.addr, &(yyvsp[(2) - (5)].addr), sizeof(temprelay->v6addr.addr));
		temprelay->v6addr.plen = (yyvsp[(4) - (5)].num);
		temprelay->next = link->relaylist;
		link->relaylist = temprelay;
		temprelay = NULL;
	}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 312 "server6_parse.y"
    {
		if (currentgroup) 
			pool->group = currentgroup->scope;
		pool->next = poollist;
		poollist = pool;
		pool = NULL;
		/* leave pool scope we know the current scope is not point to NULL*/
		currentscope = pop_double_list(currentscope);
	}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 325 "server6_parse.y"
    {
		if (!link) {
			dhcpv6_dprintf(LOG_ERR, "pooldef must be defined under link");
			ABORT;
		}
		pool = (struct pool_decl *)malloc(sizeof(*pool));
		if (pool == NULL) {
			dhcpv6_dprintf(LOG_ERR, "fail to allocate memory");
			ABORT;
		}
		memset(pool, 0, sizeof(*pool));
		TAILQ_INIT(&pool->poolscope.dnslist.addrlist);
		if (link)
			pool->link = link;
			
		/* enter pool scope */
		currentscope = push_double_list(currentscope, &pool->poolscope);
		if (currentscope == NULL)
			ABORT;
	}
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 363 "server6_parse.y"
    {
		struct v6prefix *v6prefix, *v6prefix0;
		struct v6addr *prefix;
		if (!link) {
			dhcpv6_dprintf(LOG_ERR, "prefix must be defined under link");
			ABORT;
		}
		v6prefix = (struct v6prefix *)malloc(sizeof(*v6prefix));
		if (v6prefix == NULL) {
			dhcpv6_dprintf(LOG_ERR, "failed to allocate memory");
			ABORT;
		}
		memset(v6prefix, 0, sizeof(*v6prefix));
		v6prefix->link = link;
		if (pool)
			v6prefix->pool = pool;
		/* make sure the range ipv6 address within the prefixaddr */
		if ((yyvsp[(4) - (5)].num) > 128 || (yyvsp[(4) - (5)].num) < 0) {
			dhcpv6_dprintf(LOG_ERR, "invalid prefix length in line %d", num_lines);
			ABORT;
		}
		prefix = getprefix(&(yyvsp[(2) - (5)].addr), (yyvsp[(4) - (5)].num));
		for (v6prefix0 = link->prefixlist; v6prefix0; v6prefix0 = v6prefix0->next) {
			if (IN6_ARE_ADDR_EQUAL(prefix, &v6prefix0->prefix.addr) && 
					(yyvsp[(4) - (5)].num) == v6prefix0->prefix.plen)  {
				dhcpv6_dprintf(LOG_ERR, "duplicated prefix defined within same link");
				ABORT;
			}
		}
		/* check the assigned prefix is not reserved pv6 addresses */
		if (IN6_IS_ADDR_RESERVED(prefix)) {
			dhcpv6_dprintf(LOG_ERR, "config reserved prefix");
			ABORT;
		}
		memcpy(&v6prefix->prefix, prefix, sizeof(v6prefix->prefix));
		v6prefix->next = link->prefixlist;
		link->prefixlist = v6prefix;
		free(prefix);
	}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 406 "server6_parse.y"
    {
		struct v6addrseg *seg, *temp_seg;
		struct v6addr *prefix1, *prefix2;
		if (!link) {
			dhcpv6_dprintf(LOG_ERR, "range must be defined under link");
			ABORT;
		}
		seg = (struct v6addrseg *)malloc(sizeof(*seg));
		if (seg == NULL) {
			dhcpv6_dprintf(LOG_ERR, "failed to allocate memory");
			ABORT;
		}
		memset(seg, 0, sizeof(*seg));
		temp_seg = link->seglist;
		seg->link = link;
		if (pool)
			seg->pool = pool;
		/* make sure the range ipv6 address within the prefixaddr */
		if ((yyvsp[(6) - (7)].num) > 128 || (yyvsp[(6) - (7)].num) < 0) {
			dhcpv6_dprintf(LOG_ERR, "invalid prefix length in line %d", num_lines);
			ABORT;
		}
		prefix1 = getprefix(&(yyvsp[(2) - (7)].addr), (yyvsp[(6) - (7)].num));
		prefix2 = getprefix(&(yyvsp[(4) - (7)].addr), (yyvsp[(6) - (7)].num));
		if (!prefix1 || !prefix2) {
			dhcpv6_dprintf(LOG_ERR, "address range defined error");
			ABORT;
		}
		if (ipv6addrcmp(&prefix1->addr, &prefix2->addr)) {
			dhcpv6_dprintf(LOG_ERR, 
				"address range defined doesn't in the same prefix range");
			ABORT;
		}
		if (ipv6addrcmp(&(yyvsp[(2) - (7)].addr), &(yyvsp[(4) - (7)].addr)) < 0) {
			memcpy(&seg->min, &(yyvsp[(2) - (7)].addr), sizeof(seg->min));
			memcpy(&seg->max, &(yyvsp[(4) - (7)].addr), sizeof(seg->max));
		} else {
			memcpy(&seg->max, &(yyvsp[(2) - (7)].addr), sizeof(seg->max));
			memcpy(&seg->min, &(yyvsp[(4) - (7)].addr), sizeof(seg->min));
		}
		/* check the assigned addresses are not reserved ipv6 addresses */
		if (IN6_IS_ADDR_RESERVED(&seg->max) || IN6_IS_ADDR_RESERVED(&seg->max)) {
			dhcpv6_dprintf(LOG_ERR, "config reserved ipv6address");
			ABORT;
		}

		memcpy(&seg->prefix, prefix1, sizeof(seg->prefix));
		memcpy(&seg->free, &seg->min, sizeof(seg->free));
		if (pool)
			seg->pool = pool;
		/* make sure there is no overlap in the rangelist */
		/* the segaddr is sorted by prefix len, thus most specific
		   ipv6 address is going to be assigned.
		 */
		if (!temp_seg) {
			seg->next = NULL;
			seg->prev = NULL;
			link->seglist = seg;
		} else {
			for (; temp_seg; temp_seg = temp_seg->next) { 
				if ( prefix1->plen < temp_seg->prefix.plen) {
					if (temp_seg->next == NULL) {
						temp_seg->next = seg;
						seg->prev = temp_seg;
						seg->next = NULL;
						break;
					}
					continue;
				}
				if (prefix1->plen == temp_seg->prefix.plen) {
	     				if (!(ipv6addrcmp(&seg->min, &temp_seg->max) > 0
					    || ipv6addrcmp(&seg->max, &temp_seg->min) < 0)) {
		   				dhcpv6_dprintf(LOG_ERR, "overlap range addr defined");
		   				ABORT;
					}
				}
				if (temp_seg->prev == NULL) { 
					link->seglist = seg;
					seg->prev = NULL;
				} else {
					temp_seg->prev->next = seg;
					seg->prev = temp_seg->prev;
				}
				temp_seg->prev = seg;
				seg->next = temp_seg;
				break;
			}
		}
		free(prefix1);
		free(prefix2);
	}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 501 "server6_parse.y"
    {
		/* return to prev group scope if any */
		currentgroup = pop_double_list(currentgroup);

		/* leave current group scope */
		currentscope = pop_double_list(currentscope);
	}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 527 "server6_parse.y"
    {
		struct scope *groupscope;
		groupscope = (struct scope *)malloc(sizeof(*groupscope));
		if (groupscope == NULL) {
			dhcpv6_dprintf(LOG_ERR, "group memory allocation failed");
			ABORT;
		}
		memset(groupscope, 0, sizeof(*groupscope));
		TAILQ_INIT(&groupscope->dnslist.addrlist);
		/* set up current group */
		currentgroup = push_double_list(currentgroup, groupscope);
		if (currentgroup == NULL)
			ABORT;

		/* enter group scope  */
		currentscope = push_double_list(currentscope, groupscope);
		if (currentscope == NULL)
			ABORT;
	}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 550 "server6_parse.y"
    {
		struct host_decl *temp_host = hostlist;
		while (temp_host)
		{
			if (temp_host->iaidinfo.iaid == host->iaidinfo.iaid) {
				if (0 == duidcmp(&temp_host->cid, &host->cid)) {
					dhcpv6_dprintf(LOG_ERR, "duplicated host DUID=%s IAID=%u redefined", duidstr(&host->cid), host->iaidinfo.iaid);
					ABORT;
				}
			}
			temp_host = temp_host->next;
		}
		if (currentgroup) 
			host->group = currentgroup->scope;
		host->next = hostlist;
		hostlist = host;
		host = NULL;
		/* leave host scope we know the current scope is not point to NULL*/
		currentscope = pop_double_list(currentscope);
	}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 575 "server6_parse.y"
    {
		struct host_decl *temp_host = hostlist;
		while (temp_host)
		{
			if (!strcmp(temp_host->name, (yyvsp[(2) - (2)].str))) {
				dhcpv6_dprintf(LOG_ERR, "duplicated host %s redefined", (yyvsp[(2) - (2)].str));
				ABORT;
			}
			temp_host = temp_host->next;
		}
		host = (struct host_decl *)malloc(sizeof(*host));
		if (host == NULL) {
			dhcpv6_dprintf(LOG_ERR, "fail to allocate memory");
			ABORT;
		}
		memset(host, 0, sizeof(*host));
		TAILQ_INIT(&host->addrlist);
		TAILQ_INIT(&host->prefixlist);
		TAILQ_INIT(&host->hostscope.dnslist.addrlist);
		host->network = ifnetwork;
		strncpy(host->name, (yyvsp[(2) - (2)].str), strlen((yyvsp[(2) - (2)].str)));
		/* enter host scope */
		currentscope = push_double_list(currentscope, &host->hostscope);
		if (currentscope == NULL)
			ABORT;
	}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 610 "server6_parse.y"
    {
		if (host == NULL) {
			dhcpv6_dprintf(LOG_DEBUG, "duid should be defined under host decl");
			ABORT;
		}
		configure_duid((yyvsp[(2) - (3)].str), &host->cid);
	}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 623 "server6_parse.y"
    {
	}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 629 "server6_parse.y"
    {
		host->iaidinfo.renewtime = (yyvsp[(3) - (4)].num);
	}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 633 "server6_parse.y"
    {
		host->iaidinfo.rebindtime = (yyvsp[(3) - (4)].num);
	}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 641 "server6_parse.y"
    {
		if (host == NULL) {
			dhcpv6_dprintf(LOG_DEBUG, "iaid should be defined under host decl");
			ABORT;
		}
		host->iaidinfo.iaid = (yyvsp[(2) - (3)].num);
	}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 657 "server6_parse.y"
    {
		if (host == NULL) {
			dhcpv6_dprintf(LOG_DEBUG, "address should be defined under host decl");
			ABORT;
		}
		dhcp6_add_listval(&host->addrlist, (yyvsp[(1) - (1)].dhcp6addr), DHCP6_LISTVAL_DHCP6ADDR);
		if (hash_add(host_addr_hash_table, &((yyvsp[(1) - (1)].dhcp6addr)->addr), (yyvsp[(1) - (1)].dhcp6addr)) != 0) {
			dhcpv6_dprintf(LOG_ERR, "%s" "hash add lease failed for %s",
				FNAME, in6addr2str(&((yyvsp[(1) - (1)].dhcp6addr)->addr), 0));
			free((yyvsp[(1) - (1)].dhcp6addr));
			return (-1);
		}
	}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 671 "server6_parse.y"
    {
		if (host == NULL) {
			dhcpv6_dprintf(LOG_DEBUG, "prefix should be defined under host decl");
			ABORT;
		}
		dhcp6_add_listval(&host->prefixlist, (yyvsp[(1) - (1)].dhcp6addr), DHCP6_LISTVAL_DHCP6ADDR);
	}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 683 "server6_parse.y"
    {
		(yyvsp[(3) - (5)].dhcp6addr)->type = IANA;
		(yyval.dhcp6addr) = (yyvsp[(3) - (5)].dhcp6addr);
	}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 691 "server6_parse.y"
    {
		(yyvsp[(3) - (5)].dhcp6addr)->type = IAPD;
		(yyval.dhcp6addr) = (yyvsp[(3) - (5)].dhcp6addr);
	}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 699 "server6_parse.y"
    {
		(yyvsp[(1) - (4)].dhcp6addr)->validlifetime = (yyvsp[(3) - (4)].num);
	}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 703 "server6_parse.y"
    {
		(yyvsp[(1) - (4)].dhcp6addr)->preferlifetime = (yyvsp[(3) - (4)].num);
	}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 707 "server6_parse.y"
    {
		(yyval.dhcp6addr) = (yyvsp[(1) - (1)].dhcp6addr);
	}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 714 "server6_parse.y"
    {
		struct dhcp6_addr *temp;
		temp = (struct dhcp6_addr *)malloc(sizeof(*temp));
		if (temp == NULL) {
			dhcpv6_dprintf(LOG_ERR, "v6addr memory allocation failed");
			ABORT;
		}
		memset(temp, 0, sizeof(*temp));
		memcpy(&temp->addr, &(yyvsp[(1) - (4)].addr), sizeof(temp->addr));
		if ((yyvsp[(3) - (4)].num) > 128 || (yyvsp[(3) - (4)].num) < 0) {
			dhcpv6_dprintf(LOG_ERR, "invalid prefix length in line %d", num_lines);
			ABORT;
		}
		temp->plen = (yyvsp[(3) - (4)].num);
		(yyval.dhcp6addr) = temp;
	}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 738 "server6_parse.y"
    {		
		if (!currentscope) { 
			currentscope = push_double_list(currentscope, &globalgroup->scope);
			if (currentscope == NULL)
				ABORT;
		}
	}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 746 "server6_parse.y"
    {		
		if (!currentscope) {
			currentscope = push_double_list(currentscope, &globalgroup->scope);
			if (currentscope == NULL)
				ABORT;
		}
		allow = 1;
	}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 755 "server6_parse.y"
    {		
		if (!currentscope) {
			currentscope = push_double_list(currentscope, &globalgroup->scope);
			if (currentscope == NULL)
				ABORT;
		}
	}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 766 "server6_parse.y"
    {
		if (allow) 
			currentscope->scope->allow_flags |= DHCIFF_RAPID_COMMIT;
		else 
			currentscope->scope->send_flags |= DHCIFF_RAPID_COMMIT;
	}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 773 "server6_parse.y"
    {
		if (allow) 
			currentscope->scope->allow_flags |= DHCIFF_TEMP_ADDRS;
		else 
			currentscope->scope->send_flags |= DHCIFF_TEMP_ADDRS;
	}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 780 "server6_parse.y"
    {
		if (allow) 
			currentscope->scope->allow_flags |= DHCIFF_UNICAST;
		else 
			currentscope->scope->send_flags |= DHCIFF_UNICAST;
	}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 787 "server6_parse.y"
    {
		if (allow) 
			currentscope->scope->allow_flags |= DHCIFF_INFO_ONLY;
		else
			currentscope->scope->send_flags |= DHCIFF_INFO_ONLY;
	}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 794 "server6_parse.y"
    {
		if (!currentscope) {
			currentscope = push_double_list(currentscope, &globalgroup->scope);
			if (currentscope == NULL)
				ABORT;
		}
		if ((yyvsp[(2) - (3)].num) < IRT_MINIMUM || DHCP6_DURATITION_INFINITE < (yyvsp[(2) - (3)].num)) {
			dhcpv6_dprintf(LOG_ERR, "%s"
				       "bad information refresh time",
				       FNAME);
			ABORT;
		}
		currentscope->scope->irt = (yyvsp[(2) - (3)].num);
	}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 809 "server6_parse.y"
    {
	}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 820 "server6_parse.y"
    {
		dhcp6_add_listval(&currentscope->scope->dnslist.addrlist, &(yyvsp[(1) - (1)].addr),
			DHCP6_LISTVAL_ADDR6);
	}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 825 "server6_parse.y"
    {
		struct domain_list *domainname, *temp;
		int len = 0;
		domainname = (struct domain_list *)malloc(sizeof(*domainname));
		if (domainname == NULL)
			ABORT;
		len = strlen((yyvsp[(1) - (1)].str));
		if (len > MAXDNAME) 
			ABORT;
		strncpy(domainname->name, (yyvsp[(1) - (1)].str), len);
		domainname->name[len] = '\0';
		domainname->next = NULL;
		if (currentscope->scope->dnslist.domainlist == NULL) {
			currentscope->scope->dnslist.domainlist = domainname;
			dhcpv6_dprintf(LOG_DEBUG, "add domain name %s", domainname->name);
		} else {
			for (temp = currentscope->scope->dnslist.domainlist; temp;
			     temp = temp->next) {
				if (temp->next == NULL) {
					dhcpv6_dprintf(LOG_DEBUG, "add domain name %s", 
						domainname->name);
					temp->next = domainname;
					break;
				}
			}
		}
	}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 861 "server6_parse.y"
    {
		if (!currentscope) {
			currentscope = push_double_list(currentscope, &globalgroup->scope);
			if (currentscope == NULL)
				ABORT;
		}
		currentscope->scope->renew_time = (yyvsp[(2) - (3)].num);
	}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 870 "server6_parse.y"
    {	
		if (!currentscope) {
			currentscope = push_double_list(currentscope, &globalgroup->scope);
			if (currentscope == NULL)
				ABORT;
		}
		currentscope->scope->rebind_time = (yyvsp[(2) - (3)].num);
	}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 879 "server6_parse.y"
    {
		if (!currentscope) {
			currentscope = push_double_list(currentscope, &globalgroup->scope);
			if (currentscope == NULL)
				ABORT;
		}
		currentscope->scope->valid_life_time = (yyvsp[(2) - (3)].num);
		if (currentscope->scope->prefer_life_time != 0 && 
		    currentscope->scope->valid_life_time <
		    currentscope->scope->prefer_life_time) {
			dhcpv6_dprintf(LOG_ERR, "%s" 
				"validlifetime is less than preferlifetime", FNAME);
			ABORT;
		}
	}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 895 "server6_parse.y"
    {
		if (!currentscope) {
			currentscope = push_double_list(currentscope, &globalgroup->scope);
			if (currentscope == NULL)
				ABORT;
		}
		currentscope->scope->prefer_life_time = (yyvsp[(2) - (3)].num);
		if (currentscope->scope->valid_life_time != 0 &&
		    currentscope->scope->valid_life_time <
		    currentscope->scope->prefer_life_time) {
			dhcpv6_dprintf(LOG_ERR, "%s" 
				"validlifetime is less than preferlifetime", FNAME);
			ABORT;
		}
	}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 911 "server6_parse.y"
    {
		if (!currentscope) {
			currentscope = push_double_list(currentscope, &globalgroup->scope);
			if (currentscope == NULL)
				ABORT;
		}
		if ((yyvsp[(2) - (3)].num) < 0 || (yyvsp[(2) - (3)].num) > 255)
			dhcpv6_dprintf(LOG_ERR, "%s" "bad server preference number", FNAME);
		currentscope->scope->server_pref = (yyvsp[(2) - (3)].num);
	}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 925 "server6_parse.y"
    {
		(yyval.num) = (yyvsp[(1) - (1)].num); 
	}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 929 "server6_parse.y"
    {
		(yyval.num) = DHCP6_DURATITION_INFINITE;
	}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 936 "server6_parse.y"
    {
		(yyval.str) = (yyvsp[(1) - (1)].str);
	}
    break;



/* Line 1806 of yacc.c  */
#line 2516 "server6_parse.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
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
      if (!yypact_value_is_default (yyn))
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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
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



/* Line 2067 of yacc.c  */
#line 941 "server6_parse.y"



static
void cleanup(void)
{
	/* it is not necessary to free all the pre malloc(), if it fails, 
	 * exit will free them automatically.
	 */
}

