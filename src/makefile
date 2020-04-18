all:
	gcc -g driver.c codegen.c semantics.c typeExtractor.c symboltable.c symboltableutils.c ast.c parser.c parserutils.c lexer.c hash.c -o compiler
	# gcc -g driver.c parser.c parserutils.c lexer.c hash.c -o stage1exe

clean:
	rm -rf compiler compiler.dSYM
	# rm -rf stage1exe stage1exe.dSYM