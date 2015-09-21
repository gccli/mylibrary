#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stack>
#include <vector>
using namespace std;

typedef enum _OPERATOR {
    sentinel,
    lparen,
    rparen,
    plus,
    minus,
    multiply,
    divide,
    eq,           // ==
    ne,           // !=
    lt,           // <
    gt,           // >
    le,           // <=
    ge,           // >=
    op_and,       // &&
    op_or,        // ||
    op_bitwise_and, // &
    op_bitwise_or,  // |
    
} OPERATOR;

typedef struct _VARIABLE {
    char type;
    union {
	char   name[256];
	double dval;
	float  fval;
	long   lval;
    } var;
} VARIABLE;

stack<char>     operators;
stack<VARIABLE> operands;

typedef enum _TokenType {
    TK_ERR=-2,
    TK_EOF=-1,
    TK_INIT=0,
    TK_SP=1, //' '
    TK_LPAREN=2, // '('
    TK_RPAREN=3, // ')'
    TK_LBRACE=4, // '{'
    TK_RBRACE=5, // '}'
    TK_LSQUARE=6,//'['
    TK_RSQUARE=7,//']'
    TK_OP_AND=8,//'&&'
    TK_OP_OR=9,//'||'
    TK_OP_NOT=10,//'!'
    
};


class Token 
{
public:
    Token(const char *filename);
    char *nextword();
    char *next();
    char getchar() {
	char c;
	if (bufoff == buflen)
	    return EOF;
	c = buffer[bufoff++];
	return c;
    }
    char peekchar() {
	if (bufoff == buflen-1)
	    return EOF;
	return buffer[bufoff+1];
    }
    

public:
    unsigned char   type;
    char           *sym;
    size_t          symoff;
    size_t          bufoff;
    char            symbol[128];
    size_t          buflen;
    char           *buffer;

    FILE           *inf; 
};

Token::Token(const char *filename)
{
    type=0;

    sym=symbol;
    symoff=0;
    bufoff=0;
    memset(symbol, 0, sizeof(symbol));

    inf = fopen(filename, "r");
    if (inf) {
	fseek(inf, 0L, SEEK_END);
	buflen=ftell(inf);
	fseek(inf, 0L, SEEK_SET);
	buffer=calloc(buflen+1, 1);
	fread(buffer, buflen, 1, inf);
	fclose(inf);
    }
}

char *Token::next()
{
    memset(symbol, 0, sizeof(symbol));
    symoff=0;

    char ch, nextch;
    unsigned tmptype=0;
    while ((ch = getchar()) != EOF) {
	switch (ch) {
	    case '&':
		nextch=peekchar();
		if (nextch == '&') {
		    tmptype=TK_OP_AND;
		    getchar();
		} else if (nextch == ' ') {
		    return NULL;
		}
		break;
		
	    default:
		break;
	}
    }
}


void trim(char **ptr)
{
    char *str = *ptr;
    size_t len = strlen(str);
    size_t i,j;
    for(i=0,j=len-1; i<len && j>=0;) {
	if (!(isspace(str[i]) && isspace(j)))
	    break;
	if (iscntrl(str[i])) 
	    str[i++]=0;
	if (iscntrl(str[j]))
	    str[j--]=0;
    }    
}

void test(const char *expr, int exprval) {
    int result;
    eval((char *)expr, &result);
    printf("expression:    %-*s expect:%d result: %i %s\n", max_line_size, expr, exprval, result,
	   result==exprval?"\033[32mOK\033[0m":"\033[31mFAILED\033[0m");
}


int main(int argc, char *argv[])
{
    ssize_t len;
    size_t  sz=0;
    char *line = NULL;
    FILE *fpin = NULL;
    if (argc > 1 && access(argv[1], F_OK)==0) {
	fpin = fopen(argv[1], "r");
	while((len = getline(&line, &sz, fpin)) > 0)
	    if (max_line_size < len)
		max_line_size = len;
	rewind(fpin);
    } else {
	fpin = stdin;
    }

    char *expr, *token;
    int expect;
    
    while((len = getline(&line, &sz, fpin)) > 0) {
	trim(&line);
	if (strlen(line) == 0 || line[0] == ';' || strchr(line, ';') == NULL) {
	    continue;
	}
	expr = line;
	token = strtok(line, ";");
	token = strtok(NULL, ";");
	if (token == NULL) {
	    printf("ignore\n");
	    continue;
	}
	trim(&expr);
	trim(&token);
	expect = atoi(token);
	test(line, expect);
    }
    if (line) free(line);

    return 0;
}
