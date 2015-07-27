#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static int max_line_size=0;
// http://stackoverflow.com/questions/1465909/c-expression-evaluator

char *eval(char *expr, int *res)
{
    enum STATE { LEFT, OP1, MID, OP2, RIGHT } state=LEFT;
    enum { AND, OR } op;
    int mid=0, tmp=0, neg=0;

    for( ; ; expr++, state=(STATE)(state+1), neg=0 ){
	for( ;; expr++ ) {
	    if( *expr == '!') neg = !neg;
	    else if( *expr != ' ') break;
	}
	if( *expr == '0'){
	    tmp  =  neg; 
	} else if(*expr == '1'){
	    tmp  = !neg; 
	} else if(*expr == '&'){
	    
	    op = AND; expr+=1;
	} else if(*expr == '|'){
	    op = OR; expr+=1;
	} else if(*expr == '('){
	    expr = eval(expr+1, &tmp);
	    if(neg) tmp=!tmp;
	} else if(*expr == '\0' || *expr == ')'){
	    if(state == OP2)
		*res |= mid;
	    return expr;
	}

	if( state == LEFT){
	    *res  = tmp;
	} else if(state == MID && op == OR) {
	    mid  = tmp;
	} else if(state == MID && op == AND){
	    *res &= tmp; state = LEFT; 
	} else if(state == OP2 && op == OR){
	    *res |= mid; state = OP1;
	} else if(state == RIGHT){
	    mid &= tmp; state = MID;
	}
    }
}

void test(const char *expr, int exprval) {
    int result;
    eval((char *)expr, &result);
    printf("expression:    %-*s expect:%d result: %i %s\n", max_line_size, expr, exprval, result,
	   result==exprval?"\033[32mOK\033[0m":"\033[31mFAILED\033[0m");
}

void trim(char **ptr)
{
    char *str = *ptr;
    size_t len = strlen(str);
    size_t i,j;
    for(i=0,j=len-1; i<len && j>=0; ++i,--j) {
	if (isprint(str[i]) &&isprint(j)) 
	    break;
	if (iscntrl(str[i])) str[i]=0;
	if (iscntrl(str[j])) str[j]=0;
    }    
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
