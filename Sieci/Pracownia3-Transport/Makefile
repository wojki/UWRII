transport: prac3.o Cbuffer.o
	gcc -std=c99 -Wall -Wextra -o transport prac3.o Cbuffer.o

prac3.o : prac3.c

Cbuffer.o : Cbuffer.h Cbuffer.c

clean : 
	rm -f prac3.o Cbuffer.o

distclean : 
	rm -f transport prac3.o Cbuffer.o
