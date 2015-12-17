%option noyywrap
%x IFILE
%{
  struct bufstack {
  struct bufstack *prev; /* previous entry */
  YY_BUFFER_STATE bs;    /* saved buffer */
  int lineno;            /* saved line number */
  int deep;              /* current file level */
  char *filename;        /* name of this file */
  FILE *f;               /* current file */
} *curbs = 0;
char *curfilename; /* name of current input file */
int file_push(char *fn);
int file_pop(void);
%}

%%

^#[ \t]*include[ \t]*[\"<] { BEGIN IFILE; }

<IFILE>[^ \t\n\">]+ {
  { int c;
    while((c = input()) && c != '\n') ;
  }
  yylineno++;
  if(!file_push(yytext))
   yyterminate(); /* no such file */
  BEGIN INITIAL;
}


<IFILE>.|\n {
  fprintf(stderr, "%4d bad include line\n", yylineno);
  yyterminate();
}


<<EOF>> { if(!file_pop()) yyterminate(); }


^.  { fprintf(yyout, "%4d %s", yylineno, yytext); }
^\n { fprintf(yyout, "%4d %s", yylineno++, yytext); }
\n  { ECHO; yylineno++; }
.   { ECHO; }

%%

int main(int argc, char **argv)
{
  if(argc < 2) {
    fprintf(stderr, "need filename\n");
    return 1;
  }
  if(file_push(argv[1]))
    yylex();

  return 0;
}

int file_push(char *fn) {

  FILE *f = fopen(fn, "r");
  if (!f) {
    printf("Ignore \"%s\"- %s\n", fn, strerror(errno));
    return 1;
  }

  struct bufstack *bs = calloc(1, sizeof(struct bufstack));

  // save state
  if(curbs)curbs->lineno = yylineno;

  bs->prev = curbs;
  // set up current entry
  bs->bs = yy_create_buffer(f, YY_BUF_SIZE);
  bs->f = f;
  bs->filename = fn;
  if (bs->prev) bs->deep = bs->prev->deep + 1;
  else bs->deep = 1;
  yy_switch_to_buffer(bs->bs);
  curbs = bs;
  yylineno = 1;
  curfilename = fn;
  printf("(%d) PUSH \"%s\"\n", bs->deep, fn);

  return 1;
}

int file_pop(void)
{
  struct bufstack *bs = curbs;
  struct bufstack *prevbs;
  if(!bs) return 0;

  printf("(%d) POP  \"%s\"\n", bs->deep, bs->filename);

  fclose(bs->f);
  yy_delete_buffer(bs->bs);
  // switch back to previous
  prevbs = bs->prev;
  free(bs);
  if(!prevbs) return 0;
  yy_switch_to_buffer(prevbs->bs);
  curbs = prevbs;
  yylineno = curbs->lineno;
  curfilename = curbs->filename;

  return 1;
}
