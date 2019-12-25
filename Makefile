all: out


.PHONY: clean
clean:
	rm xcc out assembly.S supplement.o

.PHONY: dbg
dbg: xcc test_1.c
	gdb ./xcc

xcc: xcc.c
	gcc -Wall -Werror -ggdb xcc.c -o xcc -std=gnu11

assembly.S: xcc test_1.c
	./xcc < test_1.c > assembly_temp.S
	mv assembly_temp.S assembly.S

.PHONY: lex
lex: xcc test_1.c
	./xcc t < test_1.c

supplement.o: supplement.c
	gcc supplement.c -ggdb -O0 -Wall -Werror -c -o supplement.o

out: assembly.S supplement.o
	gcc assembly.S supplement.o -o out