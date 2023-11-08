

# TEMPLATE MAKEFILE FOR SEABASS PROJECTS COMPILING TO NATIVE C

all:
	cbas main.cbas
	cc -O3 -march=native -fwrapv -s auto_out.c -o main -lpthread -lm

clean:
	rm -f main main.exe auto_out.c
