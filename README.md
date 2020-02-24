# ERPLAG-Compiler
A compiler for the ERPLAG language.

## Group members:
Anirudh S Chakravarthy	(2017A7PS1195P) <br/>
Hariharan Srikrishnan	(2017A7PS0134P) <br/>
Honnesh Rohmetra		(2016B2A70770P) <br/>
Praveen Ravirathinam	(2017A7PS1174P) <br/>

### Lexer.c
This file contains functions that allows the user to, given an input source file, read the file as a stream of tokens in the language of the given grammar, and its specifications. Apart from tokenising the file, appropiate error messages and the lines at which they occur are displayed wherever wrong tokens are encountered. 

Token recognition and seperation was done using Definite Finite Automata (DFA) concept. On reaching a final state retraction was done using the retract(n) function, where n denotes number of states to retract. The tokens before recognition were stored using an efficient twin buffer system and maintained two indices for tracking current token, namely buffer1, buffer2 and current. Token names were stored as enumerations and were recognised by the getLexeme() function, which used the buffer1, buffer2 and current indices to extract the correct token. Keywords were recognised by the isKeyword() function. Each recognised token is returned as a stucutre consisting of Token ID, line number and its lexeme, whose maximum length can be 25 characters.

If one wants to use only the lexer to tokenise a test text file one can do this using the lexertester.c program. In this program the text file is opened in read mode. The function getStream() is then used to read the contents of the file, then in an iterative fashion getNextToken() is called till ENDMARKER is reached. For each token the token lexeme, line number and Id are displayed.

### lexerDef.h
This file contains the definition of the structure of the token and also consists of list of all tokens stored in an enumeration fashion. <br/>
There is also anohter strucutre definition called VALUE whihc is used while tokenising integers or floats.

### hash.c
This file contains the functions used to create the Lookup hash table for tokens. <br/>
The lookup table is populated, however, in getStream() using keywords and tokens defined in lexer.c

### parserutils.c
This file contains all the utility functions needed for the parsing stage of the compiler. This includes fucntion to create nodes and populate Parse tree along with mechanism to identify tokens from ID and vice versa.

List of non-terminals in an enumeration format, definition of structure of terminal, nonterminal and symbol are defined in parserDef.h <br/>
The structure of grammar and nodes in the grammar structure are also defined here. Each node in the grammar rule has a link to the previous as well as next symbol in the rule. the nature of the symbol in the node ,i.e terminal or non terminal, is also stored at each rhs node. 

### parserDef.h
This file consists of various structure definitions used in the parser. <br/>
The structure of a terminal, non terminal and a symbol are defined here. Symbol was defined using a union of terminal and nonterminal structure.

Grammar structure was also defined here. Grammar was implemented as an array of linked lists with each linked list representing a rule of the grammar. The left hand sides, or head, of each rule was linked using the structure grammer[]. The right hand side of each rule was linked to its corresponding head and stored using a chain of rhsnode structure. 

This file also consists of list of all nonterminals stored in as an enumeration.

### parserutils.c
This file consists of functions and definitions needed for operations on parse tree and stack. The defintion of structures of stack, stack node, tree data and parse tree node are present in parserutils.h. paserutils.c has the definitions for creation of stack, pushing a new node to stack and for popping n elements from stack. It also consists of the function for deepcopy of a node on stack.

The functions for getting a terminal or non terminal name given its ID and vice-versa are also defined here.

### parser.c
This file consists of all the functions for the functionality of the parser.

The grammar is read from a text file and stored in the grammar structure. Using this structure as input, First and Follow Sets for all non terminals can be calcuted using the computeFirstAndFollowSets() function. The parse tree can be initialised using the initializeParseTree() function. The Parse Table can then be created using the createParseTable() function. Now, one can write the Parse Table to a text file using printParseTable() function which takes a file pointer as input.

To parse a program, one has to first use getStream() to read the file, like in lexer.c, then invoke the parseInputSourceCode() function to parse the file and generate the Parse tree for the given program. To write the Parse Tree to a file one must invoke the printParseTree() function. <br/>

A full flow of this can be seen in parsertester.c

