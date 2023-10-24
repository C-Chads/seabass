CC= gcc
CFLAGS= -O3 -s -std=gnu99 -march=native -flto
CFLAGS_DBG= -Og -g -std=gnu99 -DCOMPILER_CLEANS_UP -DUSE_PARSER_ECHO
CFLAGS_CLEAN= -O3 -std=gnu99 -DCOMPILER_CLEANS_UP

all: cbas_tcc cbas_cleanup cbas_dirtyfast cbas_debug

cbas_dirtyfast:
	$(CC) $(CFLAGS) ctok.c parser.c data.c constexpr.c metaprogramming.c code_validator.c astexec.c astdump.c reflection_library.c -o cbas -lm -g
	
cbas_cleanup:
	$(CC) $(CFLAGS_CLEAN) ctok.c parser.c data.c constexpr.c metaprogramming.c code_validator.c astexec.c astdump.c reflection_library.c -o cbas_clean -lm -g

cbas_debug:
	$(CC) $(CFLAGS_DBG) ctok.c parser.c data.c constexpr.c metaprogramming.c code_validator.c astexec.c astdump.c reflection_library.c -o cbas_dbg -lm -g
cbas_tcc:
	tcc -s ctok.c parser.c data.c constexpr.c metaprogramming.c code_validator.c astexec.c astdump.c reflection_library.c -o cbas_tcc -lm
	

install: all
	cp ./cbas /usr/local/bin/
	cp ./cbas_dbg /usr/local/bin/
	cp ./cbas_clean /usr/local/bin/
	cp ./cbas_tcc /usr/local/bin/

vm_test: cbas
	./cbas tests/vm_test.txt

fib_test: cbas
	./cbas tests/fib_test.txt 20
	./cbas tests/fib_test.txt 20
	
dev: clean uninstall cbas_debug
	cp ./cbas_dbg /usr/local/bin/
	
q: clean install

	

uninstall:
	rm -f /usr/local/bin/cbas
	rm -f /usr/local/bin/cbas_clean
	rm -f /usr/local/bin/cbas_dbg
	rm -f /usr/local/bin/cbas_tcc

push: clean
	git add .
	git commit -m "make push"
	git push

clean:
	rm -f *.exe *.out *.bin *.o cbas cbas_dbg cbas_clean cbas_tcc auto_out.c
