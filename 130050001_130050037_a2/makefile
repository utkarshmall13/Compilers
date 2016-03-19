all: clean compiler runcompiler

compiler:    Scanner.ih Scanner.h Scannerbase.h lex.cc parse.cc main.cc Parser.h Parserbase.h Parser.ih
	@g++   --std=c++0x lex.cc parse.cc main.cc -o parser
	@echo "Parser compiled successfully"

runcompiler: parser
	@./parser < test.c >ast.out
	@echo "========================================================================================================"
	@echo "\nAbstract Syntax Tree"
	@cat ast.out
	@echo "========================================================================================================"
	@echo "\nSymbol Table"
	@cat symtab.out
	@echo "========================================================================================================"
	@echo "\nStruct Table"
	@cat structtab.out
	@echo "========================================================================================================"

Scanner.ih: lex.l
	@if [ ! -e Scanner.ih ]; then \
		flexc++ lex.l;\
                sed -i '/include/a #include "Parserbase.h"' Scanner.ih; \
	fi

parse.cc Parser.h Parserbase.h Parser.ih: parse.y
	@bisonc++  --construction parse.y > /dev/null;
	@bash sedscript


.PHONY: clean
clean:
	@$(RM) Parser.ih Parser.h Parserbase.h parse.cc Scanner.ih Scanner.h Scannerbase.h lex.cc a.out graph.ps test.ast *.out *~

distclean: clean
	@$(RM) parse.y.output parser
