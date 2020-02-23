# ERPLAG-Compiler
A compiler for the ERPLAG language.

## Group members:
Anirudh S Chakravarthy	(2017A7PS1195P)
Hariharan Srikrishnan	(2017A7PS0134P)
Honnesh Rohmetra		(2016B2A70770P)
Praveen Ravirathinam	(2017A7PS1174P)

## Lexer.c
This file contains functions that allows the user to, given an input source file, read the file as a stream of tokens in the language of the given grammar, and its specifications. Apart from tokenising the file, appropiate error messages and the lines at which they occur are displayed wherever wrong tokens are encountered. 

Token recognition and seperation was done using Definite Finite Automata (DFA) concept. On reaching a final state retraction was done using the retract(n) function, where n denotes number of states to retract. The tokens before recognition were stored using an efficient twin buffer system and maintained two indices for tracking current token, namely buffer1, buffer2 and current. Token names were stored as enumerations and were recognised by the getLexeme() function, which used the buffer1, buffer2 and current indices to extract the correct token. Keywords were recognised by the isKeyword() function. Each recognised token is returned as a stucutre consisting of Token ID, line number and its lexeme, whose maximum length can be 25 characters.

If one wants to use only the lexer to tokenise a test text file one can do this using the lexertester.c program. In this program the text file is opened in read mode. The function getStream() is then used to read the contents of the file, then in an iterative fashion getNextToken() is called till ENDMARKER is reached. For each token the token lexeme, line number and Id are displayed.

## hash.c
This file contains the functions used to create the Lookup hash table for tokens. 
The lookup table is populated, however, in getStream() using keywords and tokens defined in lexer.c

## parserutils.c
This file contains all the utility functions needed for the parsing stage of the compiler. This includes fucntion to create nodes and populate Parse tree along with mechanism to identify tokens from ID and vice versa.

List of non-terminals in an enumeration format, definition of structure of terminal, nonterminal and symbol are defined in parserDef.h
The structure of grammar and nodes in the grammar structure are also defined here. Each node in the grammar rule has a link to the previous as well as next symbol in the rule. the nature of the symbol in the node ,i.e terminal or non terminal, is also stored at each rhs node. 

