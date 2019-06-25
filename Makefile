# Bash is needed for redirection and PIPESTATUS variable
SHELL = /bin/bash
# Basename of source files
COMP = $(wildcard *.l)
SRC = $(basename $(COMP))
# Source files
COMPILER_BUILD = main.c lex.yy.c $(SRC).tab.c $(SRC).c symtab.c lsp.c io.c
# Compile dependencies
COMPILER_DEPENDS = $(COMPILER_BUILD) $(SRC).h defs.h symtab.h lsp.h io.h err_codes.h
# Temporary files
COMPILER_CLEAN = lex.yy.c $(SRC).tab.c $(SRC).tab.h $(SRC).output $(SRC)-lsp *.?~ *.mc~ .make.out* *.asm Makefile~
# cJSON library
CJSON = `pkg-config --cflags --libs libcjson`

.PHONY: clean

$(SRC)-lsp: $(COMPILER_DEPENDS)
	@echo -e "\e[01;32mGCC...\e[00m"
	@-rm -f $(SRC)-lsp .make.out2 2>/dev/null
	@gcc -o $@ $(COMPILER_BUILD) $(CJSON) 2>&1 | tee .make.outg; pstat=$${PIPESTATUS[0]}; \
	cat .make.outf .make.outb .make.outg > .make.out 2>/dev/null; \
	out=`grep -Ei "conflict|warning|error" .make.out 2>/dev/null`; \
	if [ "$$out" != "" ]; then \
		cat .make.outf .make.outb; \
		echo -e "\e[01;31m\nThere are errors/warnings in grammar/actions!\n\e[00m" ; \
		rm -f $(SRC)-lsp 2>/dev/null ; \
		exit 1; \
	fi; \
	exit $$pstat

lex.yy.c: $(SRC).l $(SRC).tab.c
	@echo -e "\e[01;32mFLEX...\e[00m"
	@flex $< 2>&1 | tee .make.outf; exit $${PIPESTATUS[0]}

$(SRC).tab.c: $(SRC).y
	@echo -e "\e[01;32mBISON...\e[00m"
	@bison -d -v $< 2>&1 | tee .make.outb; exit $${PIPESTATUS[0]}

clean:
	@echo -e "\e[01;32mDeleting temporary files...\e[00m"
	@rm -f $(COMPILER_CLEAN)
