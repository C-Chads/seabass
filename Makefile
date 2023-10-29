CC= gcc
CFLAGS= -O3 -s -std=gnu99 -march=native -flto -fwrapv -DUSE_REALPATH
CFLAGS_DBG= -Og -g -fwrapv -fsanitize=address,undefined,leak -std=gnu99 -DCOMPILER_CLEANS_UP -DUSE_PARSER_ECHO -DUSE_REALPATH
CFLAGS_CLEAN= -O3 -s -fwrapv -flto -std=gnu99 -DCOMPILER_CLEANS_UP -DUSE_REALPATH
CFLAGS_PURE= -O3 -fwrapv -std=gnu99 -DCOMPILER_CLEANS_UP
CFLAGS_CBAS= -O3 -fwrapv -s -lpthread


cbas_dirty:
	$(CC) $(CFLAGS) ctok.c parser.c data.c constexpr.c metaprogramming.c code_validator.c astexec.c astdump.c reflection_library.c -o cbas_dirty -lm -g
	
all: cbas_tcc cbas_clean cbas_dirty cbas_dbg cbas_pure

all_not_tcc: cbas_clean cbas_dirty cbas_dbg cbas_pure
	
cbas_clean:
	$(CC) $(CFLAGS_CLEAN) ctok.c parser.c data.c constexpr.c metaprogramming.c code_validator.c astexec.c astdump.c reflection_library.c -o cbas_clean -lm -g

cbas_dbg:
	$(CC) $(CFLAGS_DBG) ctok.c parser.c data.c constexpr.c metaprogramming.c code_validator.c astexec.c astdump.c reflection_library.c -o cbas_dbg -lm -g

cbas_tcc:
	tcc -s ctok.c parser.c data.c constexpr.c metaprogramming.c code_validator.c astexec.c astdump.c reflection_library.c -o cbas_tcc -lm
	
cbas_pure:
		$(CC) $(CFLAGS_PURE) ctok.c parser.c data.c constexpr.c metaprogramming.c code_validator.c astexec.c astdump.c reflection_library.c -o cbas_pure -lm -g
	

install: all_not_tcc
	cp ./cbas_dirty /usr/local/bin/
	cp ./cbas_dbg /usr/local/bin/
	cp ./cbas_clean /usr/local/bin/cbas
	cp ./cbas_pure /usr/local/bin/

install_all: all
	cp ./cbas_dirty /usr/local/bin/
	cp ./cbas_dbg /usr/local/bin/
	cp ./cbas_clean /usr/local/bin/
	cp ./cbas_tcc /usr/local/bin/
	cp ./cbas_pure /usr/local/bin/
	cp ./cbas_clean /usr/local/bin/cbas


vm_test: cbas_clean
	./cbas_clean tests/vm_test.txt

fib_test: cbas_clean
	./cbas_clean tests/fib_test.txt 20
	./cbas_clean tests/fib_test.txt 20
	
dev: clean uninstall cbas_dbg
	cp ./cbas_dbg /usr/local/bin/
	
q: clean install

toc_test:
	cd library && \
	cbas toc.cbas && \
	mv auto_out.c ../
	$(CC) -m32 $(CFLAGS_CBAS) auto_out.c library/companion.c -o toc_test

	
toc_gnuc_test:
	cd library && \
	cbas toc_gnuc.cbas && \
	mv auto_out.c ../
	$(CC) -m32 $(CFLAGS_CBAS) -lpthread auto_out.c library/companion_gnuc.c -o toc_test

fib:
	cd library && \
	cbas toc_fib_example.cbas && \
	mv auto_out.c ../
	$(CC) $(CFLAGS_CBAS) auto_out.c library/companion.c -o fib
	

uninstall:
	rm -f /usr/local/bin/cbas_dirty
	rm -f /usr/local/bin/cbas_clean
	rm -f /usr/local/bin/cbas_dbg
	rm -f /usr/local/bin/cbas_tcc
	rm -f /usr/local/bin/cbas_pure
	rm -f /usr/local/bin/cbas

push: clean
	git add .
	git commit -m "make push"
	git push

clean:
	rm -f *.exe *.out *.bin *.o cbas cbas_dirty cbas_pure cbas_dbg cbas_clean cbas_tcc auto_out.c toc_test fib
