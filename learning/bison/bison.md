Concepts
========

Languages and Context-Free Grammars
-----------------------------------
Parse a language, it must be described by a *context-free grammar* - you specify one or more syntactic *groupings* and give *rules* for constructing them from their parts.
E.g. for C, one kind of grouping is called an 'expression', rule, "An expression can be made of a minus sign and another expression", "An expression can be an integer". Rules are often recursive, but there must be at least one rule which leads out of the recursion

Backus-Naur Form or "BNF": The most common formal system for presenting such rules for humans to read

Subclasses of context-free grammars: LR, LALR, LELR...

Parsers for LR(1) grammars are *deterministic*: the next grammar rule to apply at any point in the input is uniquely determined by the preceding input and a fixed, finite portion (called a *lookahead*) of the remaining input.
A context-free grammar can be *ambiguous*: there are multiple ways to apply the grammar rules to get the same inputs.
GLR parsing (Generalized LR)

*symbol*: each kind of syntactic unit or grouping.
*nonterminal symbols*: those which are built by grouping smaller constructs according to grammatical rules
*terminal symbols* or *token types*: those which can't be subdivided
We call a piece of input corresponding to a single terminal symbol a *token*, and a piece corresponding to a single nonterminal symbol a *grouping*.

Grammar in Bison
----------------
A nonterminal symbol in the formal grammar is represented in Bison input as an identifier. By convention, it should be in lower case, such as `expr`, `stmt` or `declaration`.

The Bison representation for a terminal symbol is also called a *token type*.

GLR Parsers
-----------
which of two possible reductions applies, or whether to apply a reduction or read more of the input and apply a reduction later in the input. *reduce/reduce conflicts*, and *shift/reduce* conflicts

• Simple GLR Parsers:	  	Using GLR parsers on unambiguous grammars.
• Merging GLR Parses:	  	Using GLR parsers to resolve ambiguities.
• GLR Semantic Actions:	  	Considerations for semantic values and deferred actions.
• Semantic Predicates:	  	Controlling a parse with arbitrary computations.
• Compiler Requirements:	GLR parsers require a modern C compiler.

Locations
---------
Each token has a semantic value. In a similar fashion, each token has an associated location, but the type of locations is the same for all tokens and groupings.


Bison Output
------------
Give it a Bison grammar file as input, the most important output is a C source file that implements a parser for the language described by the grammar. This parser is called a *Bison parser*, and this file is called a Bison *parser implementation file*.

The job of the Bison parser is to group tokens into groupings according to the grammar rules — for example, to build identifiers and operators into expressions.

The tokens come from a function called the *lexical analyzer* that you must supply in some fashion (such as by writing it in C)

Bison parser implementation file is C code which defines a function named `yyparse` which implements that grammar.

Aside from the token type names and the symbols in the actions you write, all symbols defined in the Bison parser implementation file itself begin with `yy` or `YY`.  This includes interface functions such as the lexical analyzer function `yylex`, the error reporting function `yyerror` and the parser function `yyparse` itself.

Stages in Using Bison
---------------------
1. Formally specify the grammar in a form recognized by Bison
2. Write a lexical analyzer to process input and pass tokens to the parser.
3. Write a controlling function that calls the Bison-produced parser.
4. Write error-reporting routines.

To turn this source code as written into a runnable program, you must follow these steps:
1. Run Bison on the grammar to produce the parser.
2. Compile the code output by Bison, as well as any other source files.
3. Link the object files to produce the finished product.


Overall Layout of a Bison Grammar
---------------------------------

The general form of a Bison grammar file is as follows:
    %{
    Prologue
    %}

    Bison declarations

    %%
    Grammar rules
    %%
    Epilogue

The prologue may define types and variables used in the actions.

The Bison declarations declare the names of the terminal and nonterminal symbols, and may also describe operator precedence and the data types of semantic values of various symbols.

The grammar rules define how to construct each nonterminal symbol from its parts.


Example
=======
