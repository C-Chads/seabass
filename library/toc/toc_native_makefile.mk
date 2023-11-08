

# TEMPLATE MAKEFILE FOR SEABASS PROJECTS COMPILING TO NATIVE C

all:
	cbas NAME.cbas
	cc -O3 -march=native -fwrapv -s auto_aout.c -o NAME -lpthread -lm
	
