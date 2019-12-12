all: test_out


.PHONY: clean
clean:
	rm xcc test_out assembly.S

.PHONY: dbg
dbg: xcc test_1.c
	gdb ./xcc

xcc: xcc.c
	gcc -Wall -Werror -ggdb xcc.c -o xcc

assembly.S: xcc test_1.c
	./xcc < test_1.c > assembly_temp.S
	mv assembly_temp.S assembly.S

.PHONY: lex
lex: xcc test_1.c
	./xcc t < test_1.c

test_out: assembly.S
	gcc assembly.S -o test_out