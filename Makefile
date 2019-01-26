# 1. kompajliranje programa
#       make
# 2. brisanje automatski generisanih fajlova
#       make clean
# 3. pokretanje svih testova
#       make test
# 4. pokretanje samo nekih testova
#       make test TEST=test-ok1.mc
#       make test TEST="test-ok1.mc test-ok2.mc"

# Nazivi test fajlova treba da slede sledeću konvenciju:
#   Početak naziva      Značenje
#     test-sanity       korektan miniC program bez novih konstrukcija (provera originalne gramatike)
#     test-ok           korektan miniC program sa novim konstrukcijama
#     test-semerr       nekorektan miniC program sa (jednom) semantičkom greškom
#     test-synerr       nekorektan miniC program sa (jednom) sintaksnom greškom
# Ukoliko se u test fajlu nalazi red koji počinje sa //OPIS:, ta linija će biti ispisana pre samog testiranja.

# bash je potreban da bi radila redirekcija stderr i stdout, kao i zbog PIPESTATUS promenljive
SHELL = /bin/bash
# naziv kompajlera i osnovnih source fajlova
COMP = $(wildcard *.l)
SRC = $(basename $(COMP))
# da li je kompajler sa generisanjem koda?
ifeq ($(SRC),micko)
    ASM = true
    CGENC = codegen.c
    CGENH = codegen.h
endif
# da li je prisutan simulator?
ifeq ($(wildcard ./hipsim),)
    COMPILE_SIM = $(wildcard ./hipsim-src/Makefile)
else
    COMPILE_SIM =
endif
# fajlovi od kojih se sastoji kompajler
COMPILER_BUILD = lex.yy.c $(SRC).tab.c symtab.c $(CGENC)
# fajlovi od kojih zavisi ponovno prevođenje
COMPILER_DEPENDS = $(COMPILER_BUILD) defs.h symtab.h $(CGENH)
# fajlovi koje treba pobrisati da bi ostao samo izvorni kod
COMPILER_CLEAN = lex.yy.c $(SRC).tab.c $(SRC).tab.h $(SRC).output $(SRC) *.?~ *.mc~ .make.out* *.asm Makefile~
# ako treba sprovesti samo neke testove, ovu promenljivu treba postaviti na naziv testa
TEST = ""

# pravila koja ne generišu nove fajlove
.PHONY: clean test

# osnovno pravilo za prevođenje kompajlera
$(SRC): $(COMPILER_DEPENDS)
	@if [ "$(COMPILE_SIM)" != "" ]; then \
		echo -e "\e[01;32mSIMULATOR...\e[00m"; \
		make --silent -C ./hipsim-src/ SIMULATOR_PATH=../; \
	fi
	@echo -e "\e[01;32mGCC...\e[00m"
	@-rm -f $(SRC) .make.out2 2>/dev/null
	@gcc -o $@ $(COMPILER_BUILD) 2>&1 | tee .make.outg; pstat=$${PIPESTATUS[0]}; \
	cat .make.outf .make.outb .make.outg > .make.out 2>/dev/null; \
	out=`grep -Ei "conflict|warning|error" .make.out 2>/dev/null`; \
	if [ "$$out" != "" ]; then \
		cat .make.outf .make.outb; \
		echo -e "\e[01;31m\nThere are errors/warnings in grammar/actions!\n\e[00m" ; \
		rm -f $(SRC) 2>/dev/null ; \
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

archive: clean
	@arc=`pwd`; \
	arc=`basename $$arc`; \
	echo -e "\e[01;32mCreating archive ../$$arc.tar.gz\e[00m"; \
	tar --exclude=*.gz -czf ../$$arc.tar.gz ../$$arc

# testiranje kompajlera, izvršava se samo ako nema grešaka u gramatici
# - u petlji će se kompajleru proslediti svi test*.mc fajlovi
# - ako u test fajlu postoji linija sa tekstom "//OPIS:", ispisaće se na ekranu
# - ukoliko prilikom poziva kompajlera nije bilo grešaka, i ako postoji
#   generisanje koda, ispisaće se i asemblerski izlaz
test: $(SRC)
	@if [ ! -f $(SRC) ]; then exit 1; fi
	@echo -e "\n\e[01;34mRunning compiler with test files...\e[00m"
	@if [ "$$TEST" != "" ]; then all_tests=$$TEST; \
	else all_tests=test*.mc; fi; \
	for test in $$all_tests; do \
		if [[ $$test =~ .*synerr.* ]]; then echo -e "\e[01;35m"; ttyp="syn"; \
		elif [[ $$test =~ .*semerr.* ]]; then echo -e "\e[01;34m"; ttyp="sem"; \
		elif [[ $$test =~ .*sanity.* ]]; then echo -e "\e[01;32m"; ttyp="san"; \
		else echo -e "\e[01;32m"; ttyp="ok"; fi; \
		echo -e "\n\n------------------------\nTesting: $$test"; \
		grep "//OPIS:" $$test; \
		echo -e "------------------------\e[00m"; \
		./$(SRC) < "$$test"; \
		out=$$?; \
		if [ $$ttyp == "ok" ] && [ $$out -ne 0 ]; then \
			echo -e "\e[01;31m\nError reported for '$$test'!\n\e[00m" ; \
		elif [ $$ttyp == "san" ] && [ $$out -ne 0 ]; then \
			echo -e "\e[01;31m\nOriginal miniC grammar is corrupted!\n\e[00m" ; \
		elif [ $$ttyp == "sem" ] && ([ $$out -eq 0 ] || [ $$out -eq 255 ]); then \
			echo -e "\e[01;31m\nSemantic error not reported for '$$test'!\n\e[00m" ; \
		elif [ $$ttyp == "syn" ] && [ $$out -ne 255 ]; then \
			echo -e "\e[01;31m\nSyntax error not reported for '$$test'!\n\e[00m" ; \
		elif [ "$(ASM)" == "true" ] && [ $$out -eq 0 ]; then \
			outname=$$(basename $$test); \
			outname=$${outname%.*}; \
			mv output.asm "$${outname}.asm"; \
			echo -e "output.asm saved as \e[01;34m$${outname}.asm\n\e[00m"; \
			if [ -f $${outname}.asm.ok ]; then \
				diff -w -B "$${outname}.asm" "$${outname}.asm.ok" > /dev/null; \
				if [ $$? -ne 0 ]; then \
					if [[ $$ttyp == "san" ]]; then \
						echo -e "\e[01;31m\nASM code generation for original miniC is changed!"; \
					else \
						echo -e "\e[01;31m\nASM code differs from expected!"; \
					fi; \
					echo -e "Run\e[00m fldiff $${outname}.asm $${outname}.asm.ok"; \
				fi; \
			fi; \
		fi; \
	done; \
	echo

