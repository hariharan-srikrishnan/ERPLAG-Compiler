# ERPLAG-Compiler
#### Group members:
Anirudh S Chakravarthy	(2017A7PS1195P) <br/>
Hariharan Srikrishnan	(2017A7PS0134P) <br/>
Honnesh Rohmetra		(2016B2A70770P) <br/>
Praveen Ravirathinam	(2017A7PS1174P) <br/>

This repository consists of the various files needed to implement a compiler for the ERPLAG language. To execute the program one can use the make file which generates an executable which can accept command line arguments.<br/>
Below are descriptions of the functionality of the files present in the repository.

### Lexer.c
This file contains functions that allows the user to, given an input source file, read the file as a stream of tokens in the language of the given grammar and its specifications. Apart from tokenising the file, appropriate error messages and the lines at which they occur are displayed wherever wrong tokens are encountered. 

Token recognition and separation was done using a Deterministic Finite Automata (DFA). On reaching a final state retraction was performed using the retract(n) function, where n denotes number of characters to retract. The tokens before recognition were stored using an efficient twin buffer system and maintained three indices for tracking current token, namely buffer1, buffer2 and current. Token names were stored as enumerations and were recognized by the getLexeme() function, which used the buffer1, buffer2 and current indices to extract the correct token. Keywords were recognized by the isKeyword() function. Each recognized token is returned as a structure consisting of Token ID, line number and its lexeme, whose maximum length can be 20 characters.

If one wants to use only the lexer to tokenise a test textfile one can do this using the lexertester.c program. In this program the text file is opened in read mode. The function getStream() is then used to read the contents of the file, then in an iterative fashion getNextToken() is called till ENDMARKER is reached. For each token the token lexeme, line number and Id are displayed.

### lexerDef.h
This file contains the data definition of the structure of the token and consists of list of all tokens stored in an enumerated fashion. 

### hash.c
This file contains the functions used to create the lookup hash table for keywords. The lookup table is populated, however, in getStream() using keywords defined in lexer.c

### parserutils.h
This file contains all the utility functions needed for parsing in the compiler. This includes function to create nodes and populate the parse tree along with mechanism to identify tokens from ID and vice versa. List of non-terminals in an enumeration format, definition of structure of terminal, nonterminal and symbol are defined in parserDef.h.

The structure of grammar and nodes in the grammar structure are also defined here. Each node in the grammar rule is doubly linked, i.e. it has a link to the previous as well as next symbol in the rule. The nature of the symbol in the node, i.e terminal or non terminal, is also stored at each node. The definition of structures of stack, stack node, tree data and parse tree node are also present in parserutils.h.

### parserDef.h
This file consists of various structure definitions used in the parser. The structure of a terminal, nonterminal and a symbol are defined here. Symbol is defined using a union of terminal and nonterminal structure.

Grammar structure was also defined here. Grammar was implemented as an array of linked lists with each linked list representing a rule of the grammar. The left hand side, or head, of each rule is stored under grammar[ruleNo]. The right hand side of each rule was linked to its corresponding head and stored using a linked list of rhsnode structures. 

This file also consists of list of all nonterminals stored in as an enumeration.

### parserutils.c
This file consists of functions and definitions needed for operations on parse tree and stack. parserutils.c has the definitions for creation of stack, pushing a new node to stack and for popping n elements from stack. It also consists of the function for performing a deep copy of a node on stack.

The functions for getting a terminal or non terminal name given its ID and vice-versa are also defined here.

### parser.c
This file consists of all the functions for the functionality of the parser.

The grammar is read from a text file and stored in the grammar structure. Using this structure as input, First and Follow Sets for all non terminals can be automatically calculated using the computeFirstAndFollowSets() function. The parse tree can be initialized using the initializeParseTree() function. The Parse Table for the given grammar can then be created using the createParseTable() function. Now, one can write the Parse Table to a text file using printParseTable() function which takes a file pointer as input.

To parse a program, one has to first use getStream() to read the file, like in lexer.c, then invoke the parseInputSourceCode() function to parse the file and generate the Parse tree for the given program. To write the Parse Tree to a file one must invoke the printParseTree() function passing the text file name as input. <br/>
A full flow of this can be seen in parsertester.c

### grammar.txt
This file contains the grammar of the compiler which we use to parse the program. The format one has to use to enter a rule in the text file is that all symbols for the rule are to be in order and separated by a space with the left hand side nonterminal for each rule being the first word in every line. One must keep in mind that if a grammar rule has OR it must be split into multiple rules, i.e multiple entries in the text file. Ensure to add all added symbols to their respective enumerations.

#### An example:

**Non Terminals** :	A,B,C
**Terminals** :		d,e,f

**Rules of given grammar**:	A --> dBef
							B --> f|BCe
							C --> dB

**Entry in text file is**: 
	A d B e f
	B f
	B B C e
	C d B
