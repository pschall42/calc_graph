#include "orxSolver.h"

// Internal defined
#define orxSOLVER_KZ_CONFIG_KEEP_IN_CACHE            "KeepInCache"

#define orxSOLVER_KZ_LITERAL_INFINITY                "infinity"
#define orxSOLVER_KZ_LITERAL_PI                      "pi"
#define orxSOLVER_KZ_LITERAL_EULER                   "e"
#define orxSOLVER_KZ_LITERAL_PHI                     "phi"

#define orxSOLVER_KZ_FUNC_ABS                        "ABS"
#define orxSOLVER_KZ_FUNC_ROUND                      "ROUND"
#define orxSOLVER_KZ_FUNC_CEIL                       "CEIL"
#define orxSOLVER_KZ_FUNC_FLOOR                      "FLOOR"
#define orxSOLVER_KZ_FUNC_TRUNCATE                   "TRUNCATE"
#define orxSOLVER_KZ_FUNC_CLAMP                      "CLAMP"
#define orxSOLVER_KZ_FUNC_ROOT                       "ROOT"

#define orxSOLVER_KZ_FUNC_LOG                        "LOG"
#define orxSOLVER_KZ_FUNC_LN                         "LN"

#define orxSOLVER_KZ_FUNC_SIN                        "SIN"
#define orxSOLVER_KZ_FUNC_COS                        "COS"
#define orxSOLVER_KZ_FUNC_TAN                        "TAN"
#define orxSOLVER_KZ_FUNC_ARCSIN                     "ARCSIN"
#define orxSOLVER_KZ_FUNC_ARCCOS                     "ARCCOS"
#define orxSOLVER_KZ_FUNC_ARCTAN                     "ARCTAN"
#define orxSOLVER_KZ_FUNC_SINH                       "SINH"
#define orxSOLVER_KZ_FUNC_COSH                       "COSH"
#define orxSOLVER_KZ_FUNC_TANH                       "TANH"
#define orxSOLVER_KZ_FUNC_ARCSINH                    "ARCSINH"
#define orxSOLVER_KZ_FUNC_ARCCOSH                    "ARCCOSH"
#define orxSOLVER_KZ_FUNC_ARCTANH                    "ARCTANH"

#define orxSOLVER_KZ_FUNC_LIST_MAP                   "MAP"
#define orxSOLVER_KZ_FUNC_LIST_SUM                   "SUM"
#define orxSOLVER_KZ_FUNC_LIST_MULT                  "MULT"
#define orxSOLVER_KZ_FUNC_LIST_MAX                   "MAX"
#define orxSOLVER_KZ_FUNC_LIST_MIN                   "MIN"
#define orxSOLVER_KZ_FUNC_LIST_MEAN                  "MEAN"
#define orxSOLVER_KZ_FUNC_LIST_AVG                   "AVG"
#define orxSOLVER_KZ_FUNC_LIST_MEDIAN                "MEDIAN"
#define orxSOLVER_KZ_FUNC_LIST_MODE                  "MODE"
#define orxSOLVER_KZ_FUNC_LIST_RANGE                 "RANGE"

#define orxSOLVER_KZ_FUNC_VARIADIC_MAX               "MAX"
#define orxSOLVER_KZ_FUNC_VARIADIC_MIN               "MIN"
#define orxSOLVER_KZ_FUNC_VARIADIC_MEAN              "MEAN"
#define orxSOLVER_KZ_FUNC_VARIADIC_AVG               "AVG"
#define orxSOLVER_KZ_FUNC_VARIADIC_MEDIAN            "MEDIAN"
#define orxSOLVER_KZ_FUNC_VARIADIC_MODE              "MODE"
#define orxSOLVER_KZ_FUNC_VARIADIC_RANGE             "RANGE"

#define orxSOLVER_KC_GROUP_OPEN                      '('
#define orxSOLVER_KC_GROUP_CLOSE                     ')'
#define orxSOLVER_KC_OP_PLUS                         '+'
#define orxSOLVER_KC_OP_MINUS                        '-'
#define orxSOLVER_KC_OP_MULT                         '*'
#define orxSOLVER_KC_OP_DIV                          '/'
#define orxSOLVER_KC_OP_EXP                          '^'
#define orxSOLVER_KC_OP_LIST_PROPERTY                '.'

#define orxSOLVER_KC_ARG_SEPERATOR                   ','
#define orxSOLVER_KC_OPT_ARG_SPECIFIER               ':'

#define orxSOLVER_KU32_TOPOLOGY_SIZE                 4

////////////////////////////////////////////////////////////////////////////////////////////////
/*                         Solver                          */



/*                       End Solver                        */
////////////////////////////////////////////////////////////////////////////////////////////////
/*                         Scanner                         */

typedef enum {
    TOKEN_NULL = 0, // Dummy token so we don't have to do complex orxNULL checks when reading from the identifiers table

    // Single-character tokens (1 - 11)
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_COMMA, TOKEN_COLON, TOKEN_DOT,
    TOKEN_MINUS, TOKEN_PLUS, TOKEN_SLASH, TOKEN_STAR,
    TOKEN_PERCENT, TOKEN_CARAT,

    // Literals (12 - 17)
    TOKEN_IDENTIFIER, TOKEN_NUMBER,
    TOKEN_INFINITY, TOKEN_PI, TOKEN_EULER, TOKEN_PHI,

    // General functions (18 - 24)
    TOKEN_ROUND, TOKEN_CEIL, TOKEN_FLOOR, TOKEN_TRUNCATE,
    TOKEN_ABS, TOKEN_CLAMP, TOKEN_ROOT,

    // Logarithms (25 - 26)
    TOKEN_LOG, TOKEN_LN,

    // Trigonometry (27 - 38)
    TOKEN_SIN, TOKEN_COS, TOKEN_TAN,
    TOKEN_ARCSIN, TOKEN_ARCCOS, TOKEN_ARCTAN,
    TOKEN_SINH, TOKEN_COSH, TOKEN_TANH,
    TOKEN_ARCSINH, TOKEN_ARCCOSH, TOKEN_ARCTANH,

    // List operators (39 - 47)
    TOKEN_MAP, TOKEN_SUM, TOKEN_MULT, TOKEN_MAX, TOKEN_MIN,
    TOKEN_MEAN, TOKEN_MEDIAN, TOKEN_MODE, TOKEN_RANGE,

    // Endings (48 - 49)
    TOKEN_EOL, TOKEN_EOF,

    // Error
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

static orxSTATUS initKeywords();
static orxSTATUS freeKeywords();
static void initScanner(Scanner* scanner, const char* source, int line);
static orxBOOL scanner_isAlpha(char c);
static orxBOOL scanner_isDigit(char c);
static orxBOOL scanner_isAtEnd(Scanner* scanner);
static char scanner_advance(Scanner* scanner);
static char scanner_peek(Scanner* scanner);
static char scanner_peekNext(Scanner* scanner);
static orxBOOL scanner_match(Scanner* scanner, char expected);
static Token scanner_makeToken(Scanner* scanner, TokenType type);
static Token scanner_errorToken(Scanner* scanner, const char* message);
static void scanner_skipWhitespace(Scanner* scanner);
static TokenType scanner_checkKeyword(Scanner* scanner, int start, int length, const char* rest, TokenType type);
static TokenType scanner_identifierType(Scanner* scanner);
static Token scanner_identifier(Scanner* scanner);
static Token scanner_number(Scanner* scanner);
//static Token scanner_string(Scanner* scanner);
Token scanner_scanToken(Scanner* scanner);

/* Move to own init/exit functions (like orxString_Init and orxString_Exit) */

typedef struct __orxSOLVER_PARSER_STATIC_t {
    orxBANK *pstTokenBank;      /**< Bank where tokens are stored : 4 */
    orxHASHTABLE* keywordTable; /**< Hashtable mapping strings to tokens : 8 */
} orxSOLVER_PARSER_STATIC;

static orxSOLVER_PARSER_STATIC sstParser;

//static orxHASHTABLE *keywordTable = orxHashTable_Create((orxU32)31, orxHASHTABLE_KU32_FLAG_NOT_EXPANDABLE, orxMEMORY_TYPE_MAIN);
//static orxHASHTABLE *keywordTable = orxHashTable_Create((orxU32)31, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

static TokenType* parser_makeStoredTokenType(TokenType type) {
    //TokenType* ptr = malloc(sizeof(TokenType));
    TokenType* ptr = (TokenType *)orxBank_Allocate(sstParser.pstTokenBank);
    *ptr = type;
    return ptr;
}

static orxSTATUS initKeywords() {
    orxSTATUS keywords_initialized = orxSTATUS_SUCCESS;
#define RUN_INIT_STAGE(stage)   ((keywords_initialized == orxSTATUS_SUCCESS && (stage) == orxSTATUS_SUCCESS) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE)
#define MAP_TOKEN(key, value)   RUN_INIT_STAGE(orxHashTable_Add(sstParser.keywordTable, orxString_GetID((key)), (void*)parser_makeStoredTokenType(value) ))
#define TOKEN_COUNT             35
    //sstParser.pstTokenBank = orxBank_Create(TOKEN_COUNT, sizeof(TokenType) * TOKEN_COUNT, orxBANK_KU32_FLAG_NOT_EXPANDABLE, orxMEMORY_TYPE_MAIN);
    sstParser.pstTokenBank = orxBank_Create(TOKEN_COUNT, sizeof(TokenType) * TOKEN_COUNT, orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    sstParser.keywordTable = orxHashTable_Create(TOKEN_COUNT, orxHASHTABLE_KU32_FLAG_NOT_EXPANDABLE, orxMEMORY_TYPE_MAIN);

    // Literals
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_LITERAL_INFINITY, TOKEN_INFINITY);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_LITERAL_PI,       TOKEN_PI);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_LITERAL_EULER,    TOKEN_EULER);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_LITERAL_PHI,      TOKEN_PHI);

    // Generic functions
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_ROUND,       TOKEN_ROUND);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_CEIL,        TOKEN_CEIL);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_FLOOR,       TOKEN_FLOOR);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_TRUNCATE,    TOKEN_TRUNCATE);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_ABS,         TOKEN_ABS);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_CLAMP,       TOKEN_CLAMP);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_ROOT,        TOKEN_ROOT);

    // Logarithms
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LOG,         TOKEN_LOG);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LN,          TOKEN_LN);

    // Trigonometry
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_SIN,         TOKEN_SIN);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_COS,         TOKEN_COS);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_TAN,         TOKEN_TAN);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_ARCSIN,      TOKEN_ARCSIN);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_ARCCOS,      TOKEN_ARCCOS);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_ARCTAN,      TOKEN_ARCTAN);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_SINH,        TOKEN_SINH);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_COSH,        TOKEN_COSH);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_TANH,        TOKEN_TANH);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_ARCSINH,     TOKEN_ARCSINH);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_ARCCOSH,     TOKEN_ARCCOSH);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_ARCTANH,     TOKEN_ARCTANH);

    // List/Variadic functions
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_MAP,    TOKEN_MAP);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_SUM,    TOKEN_SUM);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_MULT,   TOKEN_MULT);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_MAX,    TOKEN_MAX);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_MIN,    TOKEN_MIN);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_MEAN,   TOKEN_MEAN);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_AVG,    TOKEN_MEAN);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_MEDIAN, TOKEN_MEDIAN);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_MODE,   TOKEN_MODE);
    keywords_initialized = MAP_TOKEN(orxSOLVER_KZ_FUNC_LIST_RANGE,  TOKEN_RANGE);

    keywords_initialized = RUN_INIT_STAGE(orxHashTable_Optimize(sstParser.keywordTable));

    if (keywords_initialized == orxSTATUS_FAILURE) {
        // TODO: cleanup hashtable here, need to free all allocated pointers, clear and delete the hashtable
        freeKeywords();
    }

    return keywords_initialized;
#undef TOKEN_COUNT
#undef MAP_TOKEN
#undef RUN_INIT_STAGE
}
static orxSTATUS freeKeywords() {
    orxU64    u64Key;
    orxHANDLE hIterator;
    TokenType* type;

    /* For all keys in the table */
    /*for(hIterator = orxHashTable_GetNext(sstParser.keywordTable, orxHANDLE_UNDEFINED, &u64Key, (void **)&type);
        hIterator != orxHANDLE_UNDEFINED;
        hIterator = orxHashTable_GetNext(sstParser.keywordTable, hIterator, &u64Key, (void **)&type))
    {
        // Deletes it's token type //
        free(type);
    }*/

    // Clear and delete the table
    orxHashTable_Clear(sstParser.keywordTable);
    orxBank_Clear(sstParser.pstTokenBank);
    orxHashTable_Delete(sstParser.keywordTable);
    orxBank_Delete(sstParser.pstTokenBank);

    return orxSTATUS_SUCCESS;
}

/* End Move to own init/exit functions (like orxString_Init and orxString_Exit) */

void initScanner(Scanner* scanner, const char* source, int line) {
    scanner->start = source;
    scanner->current = source;
    scanner->line = (line == orxNULL ? 1 : line);
}

// Character checking
static orxBOOL scanner_isAlpha(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') ? orxTRUE : orxFALSE;
}
static orxBOOL scanner_isDigit(char c) {
    return (c >= '0' && c <= '9') ? orxTRUE : orxFALSE;
}
static orxBOOL scanner_isAtEnd(Scanner* scanner) {
    return (*scanner->current == '\0') ? orxTRUE : orxFALSE;
}

// Advancement and character matching
static char scanner_advance(Scanner* scanner) {
    scanner->current++;
    return scanner->current[-1];
}
static orxBOOL scanner_match(Scanner* scanner, char expected) {
    if (scanner_isAtEnd(scanner) == orxTRUE) { return orxFALSE; }
    if (*scanner->current != expected) { return orxFALSE; }

    scanner->current++;
    return orxTRUE;
}

// Lookahead
static char scanner_peek(Scanner* scanner) {
    return *scanner->current;
}
static char scanner_peekNext(Scanner* scanner) {
    if (scanner_isAtEnd(scanner) == orxTRUE) { return '\0'; }
    return scanner->current[1];
}

// Tokens
static Token scanner_makeToken(Scanner* scanner, TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;

    return token;
}
static Token scanner_errorToken(Scanner* scanner, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner->line;

    return token;
}

// Ignore characters
static void scanner_skipWhitespace(Scanner* scanner) {
    for (;;) {
        char c = scanner_peek(scanner);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                scanner_advance(scanner);
                break;
            case '\n':
                scanner->line++;
                scanner_advance(scanner);
                break;
            case '/':
                if (scanner_peekNext(scanner) == '/') {
                    while (scanner_peek(scanner) != '\n' && scanner_isAtEnd(scanner) != orxTRUE) {scanner_advance(scanner);}
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

// Token analysis
static TokenType scanner_checkKeyword(Scanner* scanner, int start, int length, const char* rest, TokenType type) {
    if (scanner->current - scanner->start == start + length && memcmp(scanner->start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}
static TokenType scanner_identifierType(Scanner* scanner) {
    /*
        Get the string id avoiding the orx internal string hashtable, since we're comparing it
        to already hashed values.
    */
    orxU32 token_length = scanner->current - scanner->start;
    orxSTRINGID token_string_id = orxString_NToCRC((orxSTRING)scanner->start, token_length);
    //orxBOOL root_prefix = (token_length > 4) &&

    TokenType* ttype_ptr = (TokenType*) orxHashTable_Get(sstParser.keywordTable, token_string_id);

    if (ttype_ptr == orxNULL) {
        // Check for special cases, such as ROOT<number>
        // TODO: Implement
        return TOKEN_IDENTIFIER;
    } else {
        // Perform a string comparison to make sure there were no hash table collisions
        const orxSTRING keyword = orxString_GetFromID(token_string_id);

        orxCHAR token_string[token_length+1];
        // initialize the buffer so we don't wind up with random garbage in the variable name
        /*for (int i = 0; i < token_length+1; i++){
            token_string[i] = '\0';
        }
        orxString_NCopy(token_string, scanner->start, token_length);*/
        orxString_NPrint(token_string, token_length+1, "%s", scanner->start);
        if (orxString_Compare(keyword, token_string) == 0) {
            return *ttype_ptr;
        }
        //orxString_Delete(token_string);
    }

    return TOKEN_IDENTIFIER;
}

static Token scanner_identifier(Scanner* scanner) {
    while (scanner_isAlpha(scanner_peek(scanner)) == orxTRUE || scanner_isDigit(scanner_peek(scanner)) == orxTRUE) {scanner_advance(scanner);}

    return scanner_makeToken(scanner, scanner_identifierType(scanner));
}
static Token scanner_number(Scanner* scanner) {
    while (scanner_isDigit(scanner_peek(scanner)) == orxTRUE) {scanner_advance(scanner);}

    // Look for a fractional part.
    if (scanner_peek(scanner) == '.' && scanner_isDigit(scanner_peekNext(scanner)) == orxTRUE) {
        // Consume the "."
        scanner_advance(scanner);

        while(scanner_isDigit(scanner_peek(scanner)) == orxTRUE) {scanner_advance(scanner);}
    }

    return scanner_makeToken(scanner, TOKEN_NUMBER);
}
/*static Token scanner_string(Scanner* scanner) {
    while (scanner_peek(scanner) != '"' && scanner_isAtEnd(scanner) != orxTRUE) {
        if (scanner_peek(scanner) == '\n') {scanner->line++;}
        scanner_advance(scanner);
    }

    if (scanner_isAtEnd(scanner) == orxTRUE) {return scanner_errorToken(scanner, "Unterminated string.");}

    // The closing quote.
    scanner_advance(scanner);
    return scanner_makeToken(scanner, TOKEN_STRING);
}*/
Token scanner_scanToken(Scanner* scanner) {
    scanner_skipWhitespace(scanner);

    scanner->start = scanner->current;

    if (scanner_isAtEnd(scanner) == orxTRUE) return scanner_makeToken(scanner, TOKEN_EOF);

    char c = scanner_advance(scanner);
    if (scanner_isAlpha(c) == orxTRUE) {return scanner_identifier(scanner);}
    if (scanner_isDigit(c) == orxTRUE) {return scanner_number(scanner);}

    switch (c) {
        case '(': return scanner_makeToken(scanner, TOKEN_LEFT_PAREN);
        case ')': return scanner_makeToken(scanner, TOKEN_RIGHT_PAREN);
        case ',': return scanner_makeToken(scanner, TOKEN_COMMA);
        case ':': return scanner_makeToken(scanner, TOKEN_COLON);
        case '.': return scanner_makeToken(scanner, TOKEN_DOT);
        case '-': return scanner_makeToken(scanner, TOKEN_MINUS);
        case '+': return scanner_makeToken(scanner, TOKEN_PLUS);
        case '/': return scanner_makeToken(scanner, TOKEN_SLASH);
        case '*': return scanner_makeToken(scanner, TOKEN_STAR);
        case '^': return scanner_makeToken(scanner, TOKEN_CARAT);
        case '%': return scanner_makeToken(scanner, TOKEN_PERCENT);
    }

    return scanner_errorToken(scanner, "Unexpected character.");
}


/*                       End Scanner                       */
////////////////////////////////////////////////////////////////////////////////////////////////
/*                         Parser                         */

typedef enum {
    OP_CONSTANT,      // loads constant from constants array for use
    OP_INFINITY,      // infinity literal
    OP_POP,           // pops top value off of the stack and forgets it
    OP_GET_VARIABLE,  // read value of variable
    OP_SET_VARIABLE,  // set value of variable
    OP_GET_GLOBAL,    // read value of global
    OP_DEFINE_GLOBAL, // global variable declaration
    OP_SET_GLOBAL,    // write value to global
    OP_ADD,           // binary +
    OP_SUBTRACT,      // binary -
    OP_MULTIPLY,      // binary *
    OP_DIVIDE,        // binary /
    OP_MODULUS,       // binary %
    OP_EXPONENT,      // binary ^
    OP_NEGATE,        // unary -
    OP_ROUND,         // round
    OP_CEILING,       // ceiling
    OP_FLOOR,         // floor
    OP_TRUNCATE,      // truncate
    OP_ABS,           // absolute value
    OP_CLAMP,         // clamp
    OP_ROOT,          // root<value>
    OP_LOG,           // log<value>
    OP_LN,            // ln
    OP_SIN,           // sin
    OP_COS,           // cos
    OP_TAN,           // tan
    OP_ARCSIN,        // arcsin
    OP_ARCCOS,        // arccos
    OP_ARCTAN,        // arctan
    OP_SINH,          // sinh
    OP_COSH,          // cosh
    OP_TANH,          // tanh
    OP_ARCSINH,       // arcsinh
    OP_ARCCOSH,       // arccosh
    OP_ARCTANH,       // arctanh

    OP_MAP,
    OP_SUM,
    OP_MULT,
    OP_MAX,
    OP_MIN,
    OP_MEDIAN,
    OP_MODE,
    OP_RANGE,

    OP_ERROR,

    OP_RETURN         // return from current function
} OpCode;

typedef struct {
    Token current;
    Token previous;
    orxBOOL hadError;
    orxBOOL panicMode;
    Scanner* scanner;
} Parser;

typedef enum {
    PREC_NONE = 0,
    PREC_ASSIGNMENT, // =
    PREC_TERM,       // + -
    PREC_FACTOR,     // * / %
    PREC_EXPONENT,   // ^
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;
typedef enum {
    ASSOC_RIGHT = 0,
    ASSOC_LEFT = 1,
    ASSOC_NONE = 1
} Associativity;

// need to change this so that it creates a dataflow graph instead of a syntax tree

typedef struct _orxSolver_syntaxNode_t SyntaxNode;

typedef void (*ParseFn)();
typedef SyntaxNode* (*PrefixFn)(Parser* parser);
typedef SyntaxNode* (*InfixFn)(Parser* parser, SyntaxNode* left);

typedef struct {
    //ParseFn prefix;
    //ParseFn infix;
    PrefixFn prefix;
    InfixFn infix;
    Precedence precedence;
    Associativity associativity;
} ParseRule;

typedef union {
    orxDOUBLE number;
    orxSTRINGID variable_id;
    const orxSTRING error;
} NodeValue;

typedef enum {
    ARG_NUMBER,
    ARG_VARIABLE,
    ARG_EXPRESSION,
} ArgumentType;

typedef struct {
    ArgumentType type;
    union {
        orxDOUBLE number;
        orxSTRINGID variable_id;
        SyntaxNode* expression;
    } value;
} Argument;

typedef struct {
    orxSTRING key;
    ArgumentType type;
    union {
        orxDOUBLE number;
        orxSTRINGID variable_id;
        SyntaxNode* expression;
    } value;
} Option;

struct _orxSolver_syntaxNode_t {
    OpCode op;
    NodeValue value;

    Argument* arg; // for handling things like ROOT4(expression) and LOG2(expression)
    Option* options; // for handling default, missing, ignore_missing, etc.

    SyntaxNode* left;
    SyntaxNode* right;
};

/*static void parser_declaration(Parser* parser);
static void parser_varDeclaration(Parser* parser);
static void parser_namedVariable(Token name, bool canAssign);*/


static SyntaxNode* orxSolverParser_ErrorAt(Parser* parser, Token* token, const orxSTRING message); // done
static SyntaxNode* orxSolverParser_Error(Parser* parser, const orxSTRING message); // done
static SyntaxNode* orxSolverParser_ErrorAtCurrent(Parser* parser, const orxSTRING message); // done

static void orxSolverParser_Advance(Parser* parser); // done
static void orxSolverParser_Consume(Parser* parser, TokenType type, orxSTRING message); // done
static orxBOOL orxSolverParser_Check(Parser* parser, TokenType type); // done
static orxBOOL orxSolverParser_Match(Parser* parser, TokenType type); // done
static ParseRule* orxSolverParser_GetRule(TokenType type); // done
static SyntaxNode* orxSolverParser_ParsePrecedence(Parser* parser, Precedence precedence); // done
static SyntaxNode* orxSolverParser_Expression(Parser* parser); // done
static SyntaxNode* orxSolverParser_Binary(Parser* parser, SyntaxNode* left); // done
static SyntaxNode* orxSolverParser_Unary(Parser* parser); // done
static SyntaxNode* orxSolverParser_Grouping(Parser* parser); // done
static SyntaxNode* orxSolverParser_Variable(Parser* parser); // done-ish
static SyntaxNode* orxSolverParser_Literal(Parser* parser); // done
static SyntaxNode* orxSolverParser_Number(Parser* parser);  // done
static SyntaxNode* orxSolverParser_NewSyntaxNode(); // done
static void orxSolverParser_DeleteSyntaxNode(SyntaxNode* node); // done
static SyntaxNode* orxSolverParser_NumberSyntaxNode(OpCode operator, orxDOUBLE value); // done
static SyntaxNode* orxSolverParser_VariableSyntaxNode(OpCode operator, orxSTRINGID variable_id); // done
static SyntaxNode* orxSolverParser_ErrorSyntaxNode(const orxSTRING message); // done
static SyntaxNode* orxSolverParser_UnarySyntaxNode(OpCode operator, SyntaxNode* right); // done
static SyntaxNode* orxSolverParser_BinarySyntaxNode(OpCode operator, SyntaxNode* left, SyntaxNode* right); // done
static SyntaxNode* orxSolverParser_AddSyntaxNodeArgument(SyntaxNode* node, Argument* argument); // done
static SyntaxNode* orxSolverParser_AddSyntaxNodeOption(SyntaxNode* node, Option* option); // done

static orxSTRING orxSolverParser_VariableIdToString(orxSTRINGID variable_id) {
    const orxSTRING variable = orxString_GetFromID(variable_id);
    int length = orxString_GetLength(variable);
    //orxLOG("From ID:\tstring id: %d, length: %d, string: '%s'", variable_id, length, variable);
    orxSTRING result = orxString_Duplicate(variable);
    return result;
}

static orxSTRING orxSolverParser_NumberToString(orxDOUBLE number) {
    orxCHAR buffer[24];
    orxString_Print(buffer, "%lf", number);
    orxSTRING result = orxString_Duplicate(buffer);
    return result;
}

static orxSTRING parser_concatenateStrings(orxSTRING separator, int argc, ...) {
    va_list argv;
    int length = 0;
    orxSTRING current;
    orxSTRING result = "";

    va_start(argv, argc);
    for (int i = 0; i < argc; i++) {
        current = va_arg(argv, orxSTRING);
        length += orxString_GetLength(current);
        orxCHAR concatenated[length];
        /*orxLOG("");
        orxLOG("accumulated: '%s'", result);
        orxLOG("current: '%s'", current);
        orxLOG("i: '%d'", i);*/
        if (i == 0) {
            //orxLOG("predicted: '%s'", current);
            orxString_Print(concatenated, "%s", current);
        } else {
            //orxLOG("predicted: '%s%s%s'", result, separator, current);
            orxString_Print(concatenated, "%s%s%s", result, separator, current);
        }
        //orxLOG("concatenated: '%s'", concatenated);
        //orxLOG("correct: '%s%s%s'", result, separator, current);
        result = orxString_Duplicate(concatenated);
    }

    return result;
}

static orxSTRING parser_treeToString(SyntaxNode* root) {
    //orxLOG("root->op: %d", root->op);
    switch(root->op) {
        case OP_NEGATE:       return parser_concatenateStrings(" ", 3, "(-",  parser_treeToString(root->right), ")"); break;
        case OP_ADD:          return parser_concatenateStrings(" ", 4, "(+", parser_treeToString(root->left),  parser_treeToString(root->right), ")"); break;
        case OP_SUBTRACT:     return parser_concatenateStrings(" ", 4, "(-", parser_treeToString(root->left),  parser_treeToString(root->right), ")"); break;
        case OP_MULTIPLY:     return parser_concatenateStrings(" ", 4, "(*", parser_treeToString(root->left),  parser_treeToString(root->right), ")"); break;
        case OP_DIVIDE:       return parser_concatenateStrings(" ", 4, "(/", parser_treeToString(root->left),  parser_treeToString(root->right), ")"); break;
        case OP_MODULUS:      return parser_concatenateStrings(" ", 4, "(%%", parser_treeToString(root->left),  parser_treeToString(root->right), ")"); break;
        case OP_EXPONENT:     return parser_concatenateStrings(" ", 4, "(^", parser_treeToString(root->left),  parser_treeToString(root->right), ")"); break;
        case OP_CONSTANT:     return orxSolverParser_NumberToString(root->value.number); break;
        case OP_GET_VARIABLE: return orxSolverParser_VariableIdToString(root->value.variable_id); break;

        case OP_ERROR: return parser_concatenateStrings(": ", 2, "Error", root->value.error); break;
        //case OP_CONSTANT:     return parser_treeToString(root->value.number); break;
        //case OP_GET_VARIABLE: return parser_concatenateStrings("var ", parser_treeToString(root->value.variable_id)); break;
    }
}

SyntaxNode* orxParser_Parse(const orxSTRING equation, int line) {
    initKeywords();

    Scanner scanner;
    initScanner(&scanner, equation, line);

    Parser parser;
    parser.hadError = orxFALSE;
    parser.panicMode = orxFALSE;
    parser.scanner = &scanner;

    orxSolverParser_Advance(&parser);
    SyntaxNode* root = orxSolverParser_Expression(&parser);

    freeKeywords();
    return root;
}

void parse_and_print_tree(const orxSTRING equation, int line) {
    // TODO: implement initial parse and tree printing code

    initKeywords();

    Scanner scanner;
    initScanner(&scanner, equation, line);

    Parser parser;
    parser.hadError = orxFALSE;
    parser.panicMode = orxFALSE;
    parser.scanner = &scanner;

    orxSolverParser_Advance(&parser);
    SyntaxNode* root = orxSolverParser_Expression(&parser);

    orxLOG(parser_treeToString(root));

    // clean up the tree
    orxSolverParser_DeleteSyntaxNode(root);
    freeKeywords();
}

void parser_parseConfigSection(const orxSTRING section) {
    orxLOG("Section: %s\torigin: %s", section, orxConfig_GetOrigin(section));
    if (orxConfig_PushSection(section) == orxSTATUS_SUCCESS) {
        orxU32 num_keys = orxConfig_GetKeyCount();
        for (orxU32 i=0; i < num_keys; i++) {
            const orxSTRING key = orxConfig_GetKey(i);
            if (orxConfig_IsList(key)) {
                orxU32 list_size = orxConfig_GetListCount(key);
                for (orxU32 j=0; j < (list_size); j++) {
                    orxLOG("%s[%d]: \"%s\"", key, j, orxConfig_GetListString(key, j));
                }
            } else if (orxConfig_IsRandomValue(key)) {
                orxLOG("%s: RANDOM", key);
            } else {
                orxLOG("%s: \"%s\"", key, orxConfig_GetString(key));
                parse_and_print_tree(orxConfig_GetString(key), 5);
            }
        }
        orxConfig_PopSection();
    }
}

SyntaxNode* orxSolverParser_Parse(const orxSTRING equation, int line) {
    initKeywords();

    Scanner scanner;
    initScanner(&scanner, equation, line);

    Parser parser;
    parser.hadError = orxFALSE;
    parser.panicMode = orxFALSE;
    parser.scanner = &scanner;

    orxSolverParser_Advance(&parser);
    SyntaxNode* root = orxSolverParser_Expression(&parser);

    freeKeywords();
    return root;
}

orxHASHTABLE* orxSolver_ParseConfigSection(const orxSTRING section) {
    orxLOG("Section: %s\torigin: %s", section, orxConfig_GetOrigin(section));
    orxHASHTABLE *pstEquations = orxNULL;
    if (orxConfig_PushSection(section) == orxSTATUS_SUCCESS) {
        orxU32 num_keys = orxConfig_GetKeyCount();
        pstEquations = orxHashTable_Create(num_keys, orxHASHTABLE_KU32_FLAG_NOT_EXPANDABLE, orxMEMORY_TYPE_MAIN);
        if (pstEquations != orxNULL) {
            for (orxU32 i=0; i < num_keys; i++) {
                const orxSTRING key = orxConfig_GetKey(i);
                if (orxConfig_IsList(key)) {
                    orxU32 list_size = orxConfig_GetListCount(key);
                    for (orxU32 j=0; j < (list_size); j++) {
                        //orxLOG("%s[%d]: \"%s\"", key, j, orxConfig_GetListString(key, j));
                    }
                } else if (orxConfig_IsRandomValue(key)) {
                    //orxLOG("%s: RANDOM", key);
                } else {
                    SyntaxNode* pst_equation_root = orxSolverParser_Parse(orxConfig_GetString(key), i);
                    orxHashTable_Add(pstEquations, orxString_GetID(key), (void*)pst_equation_root);
                }
            }
        }
        orxConfig_PopSection();
    }
    return pstEquations;
}



// TODO: Implement parser_builtin or parser_function for all the functions
static SyntaxNode* test_fail(Parser* parser) {
    return orxSolverParser_ErrorSyntaxNode("BLAH");
}

// Grammar lookup table
#define ORX_FN_GROUPING    orxSolverParser_Grouping
#define ORX_FN_BINARY      orxSolverParser_Binary
#define ORX_FN_UNARY       orxSolverParser_Unary
#define ORX_FN_VARIABLE    orxSolverParser_Variable
#define ORX_FN_LITERAL     orxSolverParser_Literal
#define ORX_FN_NUMBER      orxSolverParser_Number
static ParseRule orxSolverParser_GrammarRules[] = {
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_NULL
    {ORX_FN_GROUPING, orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_LEFT_PAREN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_RIGHT_PAREN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_COMMA
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_COLON
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_DOT
    {ORX_FN_UNARY,    ORX_FN_BINARY, PREC_TERM,     ASSOC_LEFT},    // TOKEN_MINUS
    {orxNULL,         ORX_FN_BINARY, PREC_TERM,     ASSOC_LEFT},    // TOKEN_PLUS
    {orxNULL,         ORX_FN_BINARY, PREC_FACTOR,   ASSOC_LEFT},    // TOKEN_SLASH
    {orxNULL,         ORX_FN_BINARY, PREC_FACTOR,   ASSOC_LEFT},    // TOKEN_STAR
    {orxNULL,         ORX_FN_BINARY, PREC_FACTOR,   ASSOC_LEFT},    // TOKEN_PERCENT
    {orxNULL,         ORX_FN_BINARY, PREC_EXPONENT, ASSOC_RIGHT},   // TOKEN_CARAT
    {ORX_FN_VARIABLE, orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_IDENTIFIER
    {ORX_FN_NUMBER,   orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ORX_FN_NUMBER
    {ORX_FN_LITERAL,  orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_INFINITY
    {ORX_FN_LITERAL,  orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_PI
    {ORX_FN_LITERAL,  orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_EULER
    {ORX_FN_LITERAL,  orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_PHI
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ROUND
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_CEIL
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_FLOOR
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_TRUNCATE
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ABS
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_CLAMP
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ROOT
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_LOG
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_LN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_SIN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_COS
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_TAN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ARCSIN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ARCCOS
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ARCTAN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_SINH
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_COSH
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_TANH
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ARCSINH
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ARCCOSH
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ARCTANH
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_MAP
    {test_fail,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_SUM
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_MULT
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_MAX
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_MIN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_MEAN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_MEDIAN
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_MODE
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_RANGE
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_ERROR
    {orxNULL,         orxNULL,       PREC_NONE,     ASSOC_LEFT},    // TOKEN_EOF
};
#undef ORX_FN_NUMBER
#undef ORX_FN_LITERAL
#undef ORX_FN_VARIABLE
#undef ORX_FN_UNARY
#undef ORX_FN_BINARY
#undef ORX_FN_GROUPING


// Errors

static SyntaxNode* orxSolverParser_ErrorAt(Parser* parser, Token* token, const orxSTRING message) {
    if (parser->panicMode == orxTRUE) {return orxSolverParser_ErrorSyntaxNode("Previous error detected");}
    parser->panicMode = orxTRUE;

    //orxSTRING fullErrorMessage = "Error on line %d at '%.*s': %s";
    orxSTRING fullErrorMessage;

    // TODO: fix so that it properly formats these strings!!!
    /*if (token->type == TOKEN_EOF) {
        fullErrorMessage = ("Error on line %d at end of file: %s", token->line, message);
    } else if (token->type == TOKEN_ERROR) {
        fullErrorMessage = ("Error on line %d: %s", token->line, message);
    } else {
        fullErrorMessage = ("Error on line %d at '%.*s': %s", token->line, token->length, token_start, message);
    }*/

    /*
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at ''%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    */

    parser->hadError = orxTRUE;
    return orxSolverParser_ErrorSyntaxNode(message);

    //return orxSolverParser_ErrorSyntaxNode(fullErrorMessage);
}

static SyntaxNode* orxSolverParser_Error(Parser* parser, const orxSTRING message) {
    return orxSolverParser_ErrorAt(parser, &parser->previous, message);
}

static SyntaxNode* orxSolverParser_ErrorAtCurrent(Parser* parser, const orxSTRING message) {
    return orxSolverParser_ErrorAt(parser, &parser->current, message);
}

// Advancement

static void orxSolverParser_Advance(Parser* parser) {
    parser->previous = parser->current;
    for (;;) {
        parser->current = scanner_scanToken(parser->scanner);
        if (parser->current.type != TOKEN_ERROR) { break; }

        orxSolverParser_ErrorAtCurrent(parser, parser->current.start);
    }
}

static void orxSolverParser_Consume(Parser* parser, TokenType type, orxSTRING message) {
    if (parser->current.type == type) {
        orxSolverParser_Advance(parser);
        return;
    }

    orxSolverParser_ErrorAtCurrent(parser, message);
}

// Token Matching

static orxBOOL orxSolverParser_Check(Parser* parser, TokenType type) {
    return (parser->current.type == type) ? orxTRUE : orxFALSE;
}

static orxBOOL orxSolverParser_Match(Parser* parser, TokenType type) {
    if(orxFALSE == orxSolverParser_Check(parser, type)) { return orxFALSE; }
    orxSolverParser_Advance(parser);
    return orxTRUE;
}

// Parse rule lookup

static ParseRule* orxSolverParser_GetRule(TokenType type) {
    return &orxSolverParser_GrammarRules[type];
}

// Grammar nodes

static SyntaxNode* orxSolverParser_ParsePrecedence(Parser* parser, Precedence precedence) {
    orxSolverParser_Advance(parser);
    PrefixFn prefixRule = orxSolverParser_GetRule(parser->previous.type)->prefix;
    if (prefixRule == NULL) {
        return orxSolverParser_Error(parser, "Expect expression");
    }

    SyntaxNode* expr = prefixRule(parser);

    while (precedence <= orxSolverParser_GetRule(parser->current.type)->precedence) {
        orxSolverParser_Advance(parser);
        InfixFn infixRule = orxSolverParser_GetRule(parser->previous.type)->infix;
        expr = infixRule(parser, expr);
    }

    return expr;
}


static SyntaxNode* orxSolverParser_Expression(Parser* parser) {
    return orxSolverParser_ParsePrecedence(parser, PREC_ASSIGNMENT);
}

static SyntaxNode* orxSolverParser_Grouping(Parser* parser) {
    SyntaxNode* expr = orxSolverParser_Expression(parser);
    orxSolverParser_Consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expression");
    return expr;
}

static SyntaxNode* orxSolverParser_Unary(Parser* parser) {
    // remember the operator
    TokenType op_type = parser->previous.type;

    // parse operand
    SyntaxNode* operand = orxSolverParser_ParsePrecedence(parser, PREC_UNARY);

    // unary operator node
    switch(op_type) {
        case TOKEN_MINUS: return orxSolverParser_UnarySyntaxNode(OP_NEGATE, operand); break;
        default:          return orxSolverParser_ErrorAtCurrent(parser, "Expected '-'"); // unreachable
    }
}

static SyntaxNode* orxSolverParser_Binary(Parser* parser, SyntaxNode* left) {
    // remember the operator
    TokenType op_type = parser->previous.type;

    // parse operand
    ParseRule* rule = orxSolverParser_GetRule(op_type);
    SyntaxNode* right = orxSolverParser_ParsePrecedence(parser, (Precedence)(rule->precedence + rule->associativity) );

    // binary operator node
    switch (op_type) {
        case TOKEN_PLUS:    return orxSolverParser_BinarySyntaxNode(OP_ADD,      left, right); break;
        case TOKEN_MINUS:   return orxSolverParser_BinarySyntaxNode(OP_SUBTRACT, left, right); break;
        case TOKEN_STAR:    return orxSolverParser_BinarySyntaxNode(OP_MULTIPLY, left, right); break;
        case TOKEN_SLASH:   return orxSolverParser_BinarySyntaxNode(OP_DIVIDE,   left, right); break;
        case TOKEN_PERCENT: return orxSolverParser_BinarySyntaxNode(OP_MODULUS,  left, right); break;
        case TOKEN_CARAT:   return orxSolverParser_BinarySyntaxNode(OP_EXPONENT, left, right); break;
        default:            return orxSolverParser_ErrorAtCurrent(parser, "Expected one of '+', '-', '*', '/', '%%', or '^'"); // unreachable
    }
}

// Terminal nodes

static SyntaxNode* orxSolverParser_Number(Parser* parser) {
    orxCHAR *pcEnd;
    orxSTATUS eResult;

    orxDOUBLE number = strtod(parser->previous.start, &pcEnd);

    /* Valid conversion? */
    if ((pcEnd != parser->previous.start) && (parser->previous.start != orxCHAR_NULL) && (pcEnd == &parser->previous.start[parser->previous.length])) {
        eResult = orxSTATUS_SUCCESS;
    } else {
        eResult = orxSTATUS_FAILURE;
    }
    if (eResult == orxSTATUS_SUCCESS) {
        return orxSolverParser_NumberSyntaxNode(OP_CONSTANT, number);
    }
    return orxSolverParser_Error(parser, "Expect number");

    /*
    orxDOUBLE number;
    orxSTRING numberstr;
    orxString_NCopy(numberstr, parser->previous.start, parser->previous.length);
    orxSTATUS result = orxString_ToFloat(numberstr, &number, orxNULL);
    if (eResult == orxSTATUS_SUCCESS) {
        return orxSolverParser_NumberSyntaxNode(OP_CONSTANT, number);
    }
    return orxSolverParser_Error();
    */
}

static SyntaxNode* orxSolverParser_Literal(Parser* parser) {
    if (parser->previous.type == TOKEN_INFINITY) {
        return orxSolverParser_NumberSyntaxNode(OP_CONSTANT, INFINITY);
    } else if (parser->previous.type == TOKEN_PI) {
        return orxSolverParser_NumberSyntaxNode(OP_CONSTANT, M_PI);
    } else if (parser->previous.type == TOKEN_EULER) {
        return orxSolverParser_NumberSyntaxNode(OP_CONSTANT, M_E);
    } else if (parser->previous.type == TOKEN_PHI) {
        return orxSolverParser_NumberSyntaxNode(OP_CONSTANT, M_PHI);
    } else {
        return orxSolverParser_ErrorAtCurrent(parser, "Unknown literal"); // unreachable
    }
}

// TODO: make this actually work with variables
static SyntaxNode* orxSolverParser_Variable(Parser* parser) {
    orxCHAR buffer[parser->previous.length+1];
    // initialize the buffer so we don't wind up with random garbage in the variable name
    /*for (int i = 0; i < parser->previous.length+1; i++){
        buffer[i] = '\0';
    }
    orxString_NCopy(buffer, parser->previous.start, parser->previous.length);*/
    orxString_NPrint(buffer, parser->previous.length+1, "%s", parser->previous.start);
    orxSTRING varstr = orxString_Duplicate(buffer);


    //orxLOG("To ID:\tstring id: %d, length: %d, string: '%s'", orxString_GetID(varstr), orxString_GetLength(varstr), varstr);
    return orxSolverParser_VariableSyntaxNode(OP_GET_VARIABLE, orxString_GetID(varstr));


    /*orxSTRING varstr;
    orxString_NCopy(varstr, parser->previous.start, parser->previous.length);
    return orxSolverParser_VariableSyntaxNode(OP_GET_VARIABLE, orxString_GetID(varstr));*/
}

// SyntaxNode creation

static SyntaxNode* orxSolverParser_NewSyntaxNode() {
    SyntaxNode* node = (SyntaxNode*) malloc(sizeof(SyntaxNode));
    node->op      = orxNULL;
    node->arg     = orxNULL;
    node->options = orxNULL;
    node->left    = orxNULL;
    node->right   = orxNULL;
    return node;
}

static void orxSolverParser_DeleteSyntaxNodeArguments(Argument* arg) {
    // TODO: Implement once node arguments exist
}

static void orxSolverParser_DeleteSyntaxNodeOptions(Option* options) {
    // TODO: Implement once node options exist
}

static void orxSolverParser_DeleteSyntaxNode(SyntaxNode* node) {
    if (node != orxNULL) {
        if (node->arg != orxNULL) {
            orxSolverParser_DeleteSyntaxNodeArguments(node->arg);
        }
        if (node->options != orxNULL) {
            orxSolverParser_DeleteSyntaxNodeOptions(node->options);
        }
        if (node->left != orxNULL) {
            orxSolverParser_DeleteSyntaxNode(node->left);
        }
        if (node->right != orxNULL) {
            orxSolverParser_DeleteSyntaxNode(node->right);
        }
        free(node);
    }
}

static SyntaxNode* orxSolverParser_NumberSyntaxNode(OpCode operator, orxDOUBLE value) {
    SyntaxNode* node = orxSolverParser_NewSyntaxNode();
    node->op = operator;
    node->value.number = value;
    return node;
}

static SyntaxNode* orxSolverParser_VariableSyntaxNode(OpCode operator, orxSTRINGID variable_id) {
    SyntaxNode* node = orxSolverParser_NewSyntaxNode();
    node->op = operator;
    node->value.variable_id = variable_id;
    return node;
}

static SyntaxNode* orxSolverParser_ErrorSyntaxNode(const orxSTRING message) {
    SyntaxNode* node = orxSolverParser_NewSyntaxNode();
    node->op = OP_ERROR;
    node->value.error = message;
    return node;
}

static SyntaxNode* orxSolverParser_UnarySyntaxNode(OpCode operator, SyntaxNode* right) {
    SyntaxNode* node = orxSolverParser_NewSyntaxNode();
    node->op = operator;
    node->right = right;
    return node;
}

static SyntaxNode* orxSolverParser_BinarySyntaxNode(OpCode operator, SyntaxNode* left, SyntaxNode* right) {
    SyntaxNode* node = orxSolverParser_NewSyntaxNode();
    node->op = operator;
    node->left = left;
    node->right = right;
    return node;
}

// SyntaxNode extras (arguments and options) TODO: figure out array pointer stuff

static SyntaxNode* orxSolverParser_AddSyntaxNodeArgument(SyntaxNode* node, Argument* argument) {
    node->arg = argument;
    return node;
}

static SyntaxNode* orxSolverParser_AddSyntaxNodeOption(SyntaxNode* node, Option* option) {
    node->options = option;
    return node;
}

/*                       End Parser                       */
////////////////////////////////////////////////////////////////////////////////////////////////
/*                    Topological Sort                    */

// TODO: Define to be an expandable array of expandable arrays
  // instead, just use orxHASHTABLEs, they already exist, you would need to use void* anyway
  // for a generic implementation, and the index is just the key used. The count - 1 would then
  // be the last element, so it would work the exact same just without keying string ids to it
//typedef struct {} orxSolverTopology;

// TODO: Define to be an expandable array
//typedef struct {} orxSolverTopologyTraversed;

typedef struct {
    orxBANK                      *pstBank;
    orxHASHTABLE                 *pstHashtable;
} orxSOLVER_TOPOLOGY_TRAVERSED;
typedef struct {
    orxBANK                      *pstBank;
    orxHASHTABLE                 *pstHashtable;
} orxSOLVER_TOPOLOGY_DEPTHS;
typedef struct {
    orxBANK                      *pstBank;
    orxSOLVER_TOPOLOGY_TRAVERSED *pstTraversed;
    orxSOLVER_TOPOLOGY_DEPTHS    *pstDepths;
    orxHASHTABLE                 *pstStrata;
    orxBOOL                      hasError;
    orxSTRING                    errorMessage;
} orxSOLVER_TOPOLOGY;


/** @name Topology Traversed structure creation/destruction.
 * @{ */
/** Create a new traversed structure and return it.
 * @return Returns the traversed pointer or orxNULL if failed.
 */
static orxSOLVER_TOPOLOGY_TRAVERSED* orxSolverTopologyTraversed_Create() {
    orxSOLVER_TOPOLOGY_TRAVERSED *pstTraversed;

    /* Allocate memory */
    pstTraversed = (orxSOLVER_TOPOLOGY_TRAVERSED *)orxMemory_Allocate(sizeof(orxSOLVER_TOPOLOGY_TRAVERSED), orxMEMORY_TYPE_MAIN);

    /* Clean values */
    orxMemory_Zero(pstTraversed, sizeof(orxSOLVER_TOPOLOGY_TRAVERSED));

    /* Create bank */
    pstTraversed->pstBank = orxBank_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, sizeof(orxBOOL), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Create hashtable */
    pstTraversed->pstHashtable = orxHashTable_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Done! */
    return pstTraversed;
}

/** Delete a traversed structure
 * @param[in] _pstTraversed    Traversed structure to delete.
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxSTATUS orxSolverTopologyTraversed_Delete(orxSOLVER_TOPOLOGY_TRAVERSED *_pstTraversed) {
    /* Checks */
    orxASSERT(_pstTraversed != orxNULL);

    orxSTATUS eResult = orxHashTable_Delete(_pstTraversed->pstHashtable);
    orxBank_Delete(_pstTraversed->pstBank);
    return eResult;
}

/** Mark a variable as traversed.
 * @param[in] _pstTraversed    Traversed
 * @param[in] _u32ID           Variable id to add.
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxSTATUS orxSolverTopologyTraversed_Add(orxSOLVER_TOPOLOGY_TRAVERSED *_pstTraversed, orxSTRINGID _u32ID) {
    orxSTATUS eResult;

    /* Checks */
    orxASSERT(_pstTraversed != orxNULL);

    orxBOOL* ptrBool = (orxBOOL *)orxBank_Allocate(_pstTraversed->pstBank);
    *ptrBool = orxTRUE;
    eResult = orxHashTable_Add(_pstTraversed->pstHashtable, _u32ID, (void*)ptrBool);
    if (eResult == orxSTATUS_FAILURE) {
        orxBank_Free(_pstTraversed->pstBank, ptrBool);
    }
    return eResult;
}

/** Checks if a variable has been traversed.
 * @param[in] _pstTraversed    Traversed
 * @param[in] _u32ID           Variable id to check.
 * @return orxTRUE/orxFALSE
 */
static orxBOOL orxSolverTopologyTraversed_VariableTraversed(orxSOLVER_TOPOLOGY_TRAVERSED *_pstTraversed, orxSTRINGID _u32ID) {
    orxBOOL bResult = orxFALSE;

    /* Checks */
    orxASSERT(_pstTraversed != orxNULL);

    orxBOOL *ptrBool = (orxBOOL *)orxHashTable_Get(_pstTraversed->pstHashtable, _u32ID);

    if (ptrBool != orxNULL) {
        bResult = *ptrBool;
    }

    return bResult;
}

/** Clear a traversed structure.
 * @param[in] _pstTraversed      Travesed structure to clear.
 * @return    orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxSolverTopologyTraversed_Clear(orxSOLVER_TOPOLOGY_TRAVERSED *_pstTraversed) {
    /* Checks */
    orxASSERT(_pstTraversed != orxNULL);

    orxBank_Clear(_pstTraversed->pstBank);
    return orxHashTable_Clear(_pstTraversed->pstHashtable);
}

/** @name Topology Depths structure creation/destruction.
 * @{ */
/** Create a new depths structure and return it.
 * @return Returns the depths pointer or orxNULL if failed.
 */
static orxSOLVER_TOPOLOGY_DEPTHS* orxSolverTopologyDepths_Create() {
    orxSOLVER_TOPOLOGY_DEPTHS *pstDepths;

    /* Allocate memory */
    pstDepths = (orxSOLVER_TOPOLOGY_DEPTHS *)orxMemory_Allocate(sizeof(orxSOLVER_TOPOLOGY_DEPTHS), orxMEMORY_TYPE_MAIN);

    /* Clean values */
    orxMemory_Zero(pstDepths, sizeof(orxSOLVER_TOPOLOGY_DEPTHS));

    /* Create bank */
    pstDepths->pstBank = orxBank_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, sizeof(orxU32), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Create hashtable */
    pstDepths->pstHashtable = orxHashTable_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Done! */
    return pstDepths;
}

/** Delete a depths structure
 * @param[in] _pstDepths    Depths structure to delete.
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxSTATUS orxSolverTopologyDepths_Delete(orxSOLVER_TOPOLOGY_DEPTHS *_pstDepths) {
    /* Checks */
    orxASSERT(_pstDepths != orxNULL);

    orxSTATUS eResult = orxHashTable_Delete(_pstDepths->pstHashtable);
    orxBank_Delete(_pstDepths->pstBank);
    return eResult;
}

/** Record the depth of a variable.
 * @param[in] _pstDepths       Depths
 * @param[in] _u32ID           Variable id to add.
 * @param[in] _u32Depth        Calculated topological depth of the variable.
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxSTATUS orxSolverTopologyDepths_Add(orxSOLVER_TOPOLOGY_DEPTHS *_pstDepths, orxSTRINGID _u32ID, orxU32 _u32Depth) {
    orxSTATUS eResult;

    /* Checks */
    orxASSERT(_pstDepths != orxNULL);

    orxU32* ptrU32 = (orxU32 *)orxBank_Allocate(_pstDepths->pstBank);
    *ptrU32 = _u32Depth;
    eResult = orxHashTable_Add(_pstDepths->pstHashtable, _u32ID, (void*)ptrU32);
    if (eResult == orxSTATUS_FAILURE) {
        orxBank_Free(_pstDepths->pstBank, ptrU32);
    }
    return eResult;
}

/** Checks if a variable has an assigned depth.
 * @param[in] _pstDepths       Depths
 * @param[in] _u32ID           Variable id to check.
 * @return orxTRUE/orxFALSE
 */
static orxBOOL orxSolverTopologyDepths_HasKey(orxSOLVER_TOPOLOGY_DEPTHS *_pstDepths, orxSTRINGID _u32ID) {
    orxBOOL bResult = orxFALSE;

    /* Checks */
    orxASSERT(_pstDepths != orxNULL);

    if (orxHashTable_Get(_pstDepths->pstHashtable, _u32ID) != orxNULL) {
        bResult = orxTRUE;
    }

    return bResult;
}

/** Gets a variable's depth.
 * @param[in]  _pstDepths       Depths
 * @param[in]  _u32ID           Variable id to check.
 * @param[out] _pu32Depth       Pointer to write depth to.
 * @return orxTRUE/orxFALSE
 */
static orxSTATUS orxSolverTopologyDepths_Get(orxSOLVER_TOPOLOGY_DEPTHS *_pstDepths, orxSTRINGID _u32ID, orxU32* _pu32Depth) {
    orxSTATUS eResult = orxSTATUS_FAILURE;

    /* Checks */
    orxASSERT(_pstDepths != orxNULL);

    orxU32* pu32Depth = (orxU32 *)orxHashTable_Get(_pstDepths->pstHashtable, _u32ID);

    if (pu32Depth != orxNULL) {
        eResult = orxSTATUS_SUCCESS;
        *_pu32Depth = *pu32Depth;
    }


    return eResult;
}

/** Clear a depths structure.
 * @param[in] _pstDepths      Depths structure to clear.
 * @return    orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxSolverTopologyDepths_Clear(orxSOLVER_TOPOLOGY_DEPTHS *_pstDepths) {
    /* Checks */
    orxASSERT(_pstDepths != orxNULL);

    orxBank_Clear(_pstDepths->pstBank);
    return orxHashTable_Clear(_pstDepths->pstHashtable);
}

/** @name Topology creation/destruction.
 * @{ */
/** Create a new topology and return it.
 * @return Returns the topology pointer or orxNULL if failed.
 */
static orxSOLVER_TOPOLOGY* orxSolverTopology_Create() {
    orxSOLVER_TOPOLOGY *pstTopology;

    /* Allocate memory */
    pstTopology = (orxSOLVER_TOPOLOGY *)orxMemory_Allocate(sizeof(orxSOLVER_TOPOLOGY), orxMEMORY_TYPE_MAIN);

    /* Clean values */
    orxMemory_Zero(pstTopology, sizeof(orxSOLVER_TOPOLOGY));

    /* Create bank */
    pstTopology->pstBank = orxBank_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, sizeof(orxSTRINGID), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Create traversed */
    pstTopology->pstTraversed = orxSolverTopologyTraversed_Create();

    /* Create depths */
    pstTopology->pstDepths = orxSolverTopologyDepths_Create();

    /* Create strata */
    pstTopology->pstStrata = orxHashTable_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Initialize error handling */
    pstTopology->hasError = orxFALSE;
    pstTopology->errorMessage = orxNULL;

    /* Done! */
    return pstTopology;
}

/** Delete a topology
 * @param[in] _pstTopology    Topology to delete.
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxSTATUS orxSolverTopology_Delete(orxSOLVER_TOPOLOGY *_pstTopology) {
    /* Checks */
    orxASSERT(_pstTopology != orxNULL);

    /* Delete traversed */
    orxSolverTopologyTraversed_Delete(_pstTopology->pstTraversed);

    /* Delete depths */
    orxSolverTopologyDepths_Delete(_pstTopology->pstDepths);

    /* Delete strata */
    orxHashTable_Delete(_pstTopology->pstStrata);

    /* Delete error message */
    if(_pstTopology->hasError) {
        orxString_Delete(_pstTopology->errorMessage);
    }

    /* Done! */
    return orxSTATUS_SUCCESS;
}

/** Add variable to topology at depth
 * @param[in] _pstTopology     Topology
 * @param[in] _u32ID           Variable id to add.
 * @param[in] _u32Depth        Calculated topological depth of the variable.
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxSTATUS orxSolverTopology_Add(orxSOLVER_TOPOLOGY *_pstTopology, orxSTRINGID _u32ID, orxU32 _u32Depth) {
    orxSTATUS eResult;

    /* Checks */
    orxASSERT(_pstTopology != orxNULL);

    // create stratum at depth if not present
    orxHASHTABLE *stratum;
    stratum = (orxHASHTABLE *)orxHashTable_Get(_pstTopology->pstStrata, _u32Depth);
    if (stratum == orxNULL) {
        stratum = orxHashTable_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
        orxHashTable_Add(_pstTopology->pstStrata, _u32Depth, (void*) stratum);
    }

    // add variable_id to the stratum
    orxSTRINGID* pU32ID = (orxSTRINGID *)orxBank_Allocate(_pstTopology->pstBank);
    *pU32ID = _u32ID;
    eResult = orxHashTable_Add(stratum, orxHashTable_GetCount(stratum), (void*)pU32ID);

    // add to the depths cache
    if (eResult = orxSTATUS_SUCCESS) {
        eResult = orxSolverTopologyDepths_Add(_pstTopology->pstDepths, _u32ID, _u32Depth);
    }

    // free the slot on failure
    if (eResult == orxSTATUS_FAILURE) {
        orxBank_Free(_pstTopology->pstBank, pU32ID);
    }

    /* Done! */
    return eResult;
}

// TODO: change/update to use the orxSOLVER_TOPOLOGY structure


static orxU32 orxSolver_topologicalSort(orxHASHTABLE* equation_trees, orxSTRINGID current_variable_id, SyntaxNode* current_node, orxSOLVER_TOPOLOGY* topology);

static orxU32 orxSolver_maxDepth(orxHASHTABLE* equation_trees, SyntaxNode* current_node, orxSOLVER_TOPOLOGY* topology) {
    orxU32 left_depth = 0;
    orxU32 right_depth = 0;
    orxU32 value_depth = 0;

    if (current_node->left != orxNULL) {
        left_depth = orxSolver_maxDepth(equation_trees, current_node->left, topology);
    }
    if (current_node->right != orxNULL) {
        right_depth = orxSolver_maxDepth(equation_trees, current_node->right, topology);
    }
    if (current_node->op == OP_GET_VARIABLE) {
        orxSTRINGID variable_id = current_node->value.variable_id;
        orxU32 variable_depth;

        if (orxSolverTopologyDepths_Get(topology->pstDepths, variable_id, &variable_depth) == orxSTATUS_SUCCESS) {
            return variable_depth + 1;
        }
        SyntaxNode *variable_tree = (SyntaxNode *) orxHashTable_Get(equation_trees, variable_id);

        // variable is an input if variable_tree is orxNULL
        if (variable_tree == orxNULL) {
            // Set input variable at depth 0
            orxSolverTopology_Add(topology, variable_id, 0);

            return 1;
        }
        value_depth = orxSolver_topologicalSort(equation_trees, variable_id, variable_tree, topology) + 1;
    }

    return orxMAX(orxMAX(left_depth, right_depth), value_depth);
}

static orxU32 orxSolver_topologicalSort(orxHASHTABLE* equation_trees, orxSTRINGID current_variable_id, SyntaxNode* current_node, orxSOLVER_TOPOLOGY* topology) {
    if (orxSolverTopologyTraversed_VariableTraversed(topology->pstTraversed, current_variable_id) == orxTRUE) {
        // Loop detected, set error
    }
    // Set topology->pstTraversed[current_variable_id] to orxTRUE
    orxSolverTopologyTraversed_Add(topology->pstTraversed, current_variable_id);
    orxU32 depth;

    if (orxSolverTopologyDepths_Get(topology->pstDepths, current_variable_id, &depth) == orxSTATUS_SUCCESS) {
        return depth;
    }
    depth = orxSolver_maxDepth(equation_trees, current_node, topology);

    // Set depth of current_variable
    orxSolverTopology_Add(topology, current_variable_id, depth);

    return depth;
}

static orxSOLVER_TOPOLOGY* orxSolver_getTopology(orxHASHTABLE* equation_trees) {
    // initialize variable_depths and topology
    /*orxHASHTABLE *variable_depths = orxHashTable_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    orxHASHTABLE *topology = orxHashTable_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    orxHASHTABLE *traversed = orxHashTable_Create(orxSOLVER_KU32_TOPOLOGY_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);*/

    orxSOLVER_TOPOLOGY *topology = orxSolverTopology_Create();

    orxU64    current_variable_id;
    orxHANDLE hIterator;
    SyntaxNode* current_node;

    // For all keys in the table //
    for(hIterator = orxHashTable_GetNext(equation_trees, orxHANDLE_UNDEFINED, &current_variable_id, (void **)&current_node);
        hIterator != orxHANDLE_UNDEFINED;
        hIterator = orxHashTable_GetNext(equation_trees, hIterator, &current_variable_id, (void **)&current_node))
    {
        // Do a topological sort on the node //
        //orxSolver_topologicalSort(equation_trees, current_variable_id, current_node, variable_depths, topology, traversed);
        orxSolver_topologicalSort(equation_trees, current_variable_id, current_node, topology);

        // Clean traversed table //
        /*orxSolver_topologyClearBooleansFromHashtable(traversed);
        orxHashTable_Clear(traversed);*/
        orxSolverTopologyTraversed_Clear(topology->pstTraversed);
    }

    // Cleanup internal hashtables //
    /*orxSolver_topologyClearU32FromHashtable(variable_depths);
    orxHashTable_Delete(variable_depths);
    orxHashTable_Delete(traversed);*/

    return topology;
}

void orxSolver_printConfigByTopology(const orxSTRING section) {
    orxHASHTABLE *equations = orxSolver_ParseConfigSection(section);
    orxSOLVER_TOPOLOGY *topology = orxSolver_getTopology(equations);

    orxU32 strata_count = orxHashTable_GetCount(topology->pstStrata);

    for (orxU32 i = 0; i < strata_count; i++) {
        orxHASHTABLE *stratum = (orxHASHTABLE *)orxHashTable_Get(topology->pstStrata, i);
        orxU32 variable_count = orxHashTable_GetCount(stratum);
        for (orxU32 j = 0; j < variable_count; j++) {
            orxSTRINGID *variable = (orxSTRINGID *)orxHashTable_Get(stratum, j);
            SyntaxNode* variable_root = (SyntaxNode *)orxHashTable_Get(equations, *variable);
            orxLOG("%s = %s", orxString_GetFromID(*variable), parser_treeToString(variable_root));

            // clean up the tree
            orxSolverParser_DeleteSyntaxNode(variable_root);
        }
    }
    orxSolverTopology_Delete(topology);
    orxHashTable_Delete(equations);
}


//static orxSTRINGID** orxSolver_topologicalSort()

/*                  End Topological Sort                  */
////////////////////////////////////////////////////////////////////////////////////////////////
/*                        Compiler                        */

/*typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_EXPONENT,   // ^
    PREC_UNARY,      // -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;


static Chunk* currentChunk();
static void errorAt(Token* token, const char* message);
static void error(const char* message);
static void errorAtCurrent(const char* message);
static void advance();
static void consume(TokenType type, const char* message);
static bool check(TokenType type);
static bool match(TokenType type);
static void emitByte(uint8_t byte);
static void emitBytes(uint8_t byte1, uint8_t byte2);
static void emitReturn();
static uint8_t makeConstant(Value value);
static void emitConstant(Value value);
static void endCompiler();
static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
static uint8_t identifierConstant(Token* name);
static uint8_t parseVariable(const char* errorMessage);
static void defineVariable(uint8_t global);
static void binary(bool canAssign);
static void literal(bool canAssign);
static void grouping(bool canAssign);
static void number(bool canAssign);
static void string(bool canAssign);
static void namedVariable(Token name, bool canAssign);
static void variable(bool canAssign);
static void unary(bool canAssign);
static void varDeclaration();
static void expressionStatement();
static void printStatement();
static void synchronize();
bool compile(const char* source, Chunk* chunk);

*/

/*
typedef struct _bytecode_array {
    orxU32 capacity;
    orxU32 size;
    orxU32 *array;
};

typedef _bytecode_array orxSolverBytecode;
typedef _bytecode_array orxSolverConstants;

*/

/*                      End Compiler                      */
////////////////////////////////////////////////////////////////////////////////////////////////
/*                        Debugger                        */

/*
void disassembleChunk(Chunk* chunk, const char* name);
static int constantInstruction(const char* name, Chunk* chunk, int offset);
static int simpleInstruction(const char* name, int offset);
int disassembleInstruction(Chunk* chunk, int offset);
*/

/*                      End Debugger                      */
////////////////////////////////////////////////////////////////////////////////////////////////
/*                           VM                           */

/*
typedef struct {
    Chunk* chunk; // chunk currently being executed
    uint8_t* ip;  // instruction pointer
    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;

    Obj* objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

static void resetStack();
static void runtimeError(const char* format, ...);
void initVM();
void freeVM();
void push(Value value);
Value pop();
static Value peek(int distance);
static bool isFalsey(Value value);
static void concatenate();
static InterpretResult run()
InterpretResult interpret(const char* source);
*/




/* TODO: Uncomment when ready, this is the actual VM


typedef struct {
    orxU32 ip;
    orxSOLVER_MODE mode;
    orxBANK *stack;
    orxBANK *pstResultBank;
    orxHASHTABLE *defaults;
    orxHASHTABLE *inputs;
    orxHASHTABLE *outputs;
    orxHASHTABLE *constants;
    orxSolverBytecode* bytecode;
} orxSolverVM;

void orxSolverVM_growStack(orxSolverVM* vm) {
    // expand stack
}

orxU32 orxSolverVM_readByte(orxSolverVM* vm) {
    vm.ip++;
    return vm->bytecode->array[vm.ip - 1];
}

orxSOLVER_VAR orxSolverVM_getConstantValue(orxSolverVM* vm, orxU32 constant_address) {
    return vm->constants->array[constant_address];
}

orxSOLVER_VAR orxSolverVM_getVariableValue(orxSolverVM* vm, orxU32 variable_address) {
    return vm->constants->array[variable_address];
}

void orxSolverVM_StackPush(orxSolverVM* vm, orxSOLVER_VAR *value) {
    orxU32 idx = vm->stack->top;
    if (idx == vm->stack->capacity) {
        orxSolverVM_growStack(vm);
    }
    vm->stack->array[idx] = value;
    vm->stack->top += 1;
}

orxSOLVER_VAR orxSolverVM_StackPop(orxSolverVM* vm) {
    orxU32 idx = vm->stack->top - 1;
    vm->stack->top = idx;
    return vm->stack->array[idx];
}

orxSOLVER_VAR orxSolverVar_Add(orxSOLVER_VAR left, orxSOLVER_VAR right) {
    orxSOLVER_VAR stResult;
    switch(left->eType) {
        case orxSOLVER_VAR_TYPE_DOUBLE: {

            break;
        }
        case orxSOLVER_VAR_TYPE_VECTOR: {
            stResult
            break;
        }
        default: {
            // error, can't add strings or booleans
            break;
        }
    }
    if(left.eType == right.eType) {
        if(left.eType == orxSOLVER_VAR_TYPE_DOUBLE) {
            stResult =
        }
    } else {
        // error, can't add different types
    }
}

orxSOLVER_VAR* orxSolverVM_GetVariable(orxSolverVM *_pstVm, orxU64 _u64Key){
    orxSOLVER_VAR *pstValue = orxNULL;
    if (_pstVm->mode == orxSOLVER_MODE_SOLVE) {
        pstValue = (orxSOLVER_VAR *)orxHashTable_Get(_pstVm->outputs, _u64Key);
    }
    if (pstValue == orxNULL) {
        pstValue = (orxSOLVER_VAR *)orxHashTable_Get(_pstVm->inputs, _u64Key);
    }
    if (pstValue == orxNULL) {
        pstValue = (orxSOLVER_VAR *)orxHashTable_Get(_pstVm->defaults, _u64Key);
    }
    return pstValue;
}

orxSTATUS orxSolverVM_SetVariable(orxSolverVM *_pstVm, orxU64 _u64Key, orxSOLVER_VAR *_pstValue){
    return orxHashTable_Add(_pstVm->outputs, _u64Key, (void *)_pstValue);
}



void orxSolverVM_processOperation(orxSolverVM* vm, OpCode opcode) {
    switch(opcode) {
        case OP_CONSTANT: {
            orxU32 constant_address = orxSolverVM_readByte(vm);
            orxSOLVER_VAR *constant_value = orxSolverVM_getConstantValue(vm, constant_address);
            orxSolverVM_StackPush(vm, constant_value);
            break;
        }
        case OP_GET_VARIABLE: {
            orxU32 variable_address = orxSolverVM_readByte(vm);
            orxSOLVER_VAR *variable_value = orxSolverVM_GetVariable(vm, variable_address);
            orxSolverVM_StackPush(vm, variable_value);
            break;
        }
        case OP_SET_VARIABLE: {
            orxU32 variable_address = orxSolverVM_readByte(vm);
            orxSOLVER_VAR *variable_value = orxSolverVM_StackPop(vm);
            orxSolverVM_SetVariable(vm, variable_address, variable_value);
            break;
        }
        // binary +
        case OP_ADD: {
            orxSOLVER_VAR *right_value = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *left_value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, left_value + right_value);
            break;
        }
        // binary -
        case OP_SUBTRACT: {
            orxSOLVER_VAR *right_value = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *left_value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, left_value - right_value);
            break;
        }
        // binary *
        case OP_MULTIPLY: {
            orxSOLVER_VAR *right_value = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *left_value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, left_value * right_value);
            break;
        }
        // binary /
        case OP_DIVIDE: {
            orxSOLVER_VAR *right_value = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *left_value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, left_value / right_value);
            break;
        }
        // binary %
        case OP_MODULUS: {
            orxSOLVER_VAR *right_value = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *left_value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, fmod(left_value, right_value));
            break;
        }
        // binary ^
        case OP_EXPONENT: {
            orxSOLVER_VAR *right_value = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *left_value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, pow(left_value, right_value));
            break;
        }
        // unary -
        case OP_NEGATE: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, -value);
            break;
        }
        // round
        case OP_ROUND: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, orxMath_Round(value));
            break;
        }
        // ceiling
        case OP_CEILING: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, orxMath_Ceil(value));
            break;
        }
        // floor
        case OP_FLOOR: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, orxMath_Floor(value));
            break;
        }
        // truncate
        case OP_TRUNCATE: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, trunc(value));
            break;
        }
        // absolute value
        case OP_ABS: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            if (value < 0) {
                value = -value;
            }
            orxSolverVM_StackPush(vm, value);
            break;
        }
        // clamp
        case OP_CLAMP: {
            orxSOLVER_VAR *max_value = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *min_value = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, orxCLAMP(value, min_value, max_value));
            break;
        }
        // root<value>
        case OP_ROOT: {
            orxSOLVER_VAR *root = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, pow(value, (1.0 / root) ));
            break;
        }
        // log<value>
        case OP_LOG: {
            orxSOLVER_VAR *base = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSOLVER_VAR *log_value = (base == M_E) ? log(value)   :
                                  (base == 10)  ? log10(value) :
                                  (log10(value) / log10(base));
            orxSolverVM_StackPush(vm, log_value);
            break;
        }
        // ln
        case OP_LN: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, log(value));
            break;
        }
        // sin
        case OP_SIN: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, sin(value));
            break;
        }
        // cos
        case OP_COS: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, cos(value));
            break;
        }
        // tan
        case OP_TAN: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, tan(value));
            break;
        }
        // arcsin
        case OP_ARCSIN: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, asin(value));
            break;
        }
        // arccos
        case OP_ARCCOS: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, acos(value));
            break;
        }
        // arctan
        case OP_ARCTAN: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, atan2(value));
            break;
        }
        // sinh
        case OP_SINH: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, sinh(value));
            break;
        }
        // cosh
        case OP_COSH: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, cosh(value));
            break;
        }
        // tanh
        case OP_TANH: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, tanh(value));
            break;
        }
        // arcsinh
        case OP_ARCSINH: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, asinh(value));
            break;
        }
        // arccosh
        case OP_ARCCOSH: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, acosh(value));
            break;
        }
        // arctanh
        case OP_ARCTANH: {
            orxSOLVER_VAR *value = orxSolverVM_StackPop(vm);
            orxSolverVM_StackPush(vm, atanh(value));
            break;
        }
    }
}
/**/


/*                         End VM                         */
////////////////////////////////////////////////////////////////////////////////////////////////
