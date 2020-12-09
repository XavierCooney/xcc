.PHONY: all
all: out


.PHONY: clean
clean:
	rm xcc out assembly.S supplement.o

.PHONY: dbg
dbg: xcc main_test_suite.c
	gdb ./xcc

xcc: xcc.c
	gcc -fsanitize=undefined -Wall -Werror -ggdb xcc.c -o xcc -std=gnu11

assembly.S: xcc main_test_suite.c
	./xcc < main_test_suite.c > assembly_temp.S
	mv assembly_temp.S assembly.S

.PHONY: lex
lex: xcc main_test_suite.c
	./xcc t < main_test_suite.c

supplement.o: supplement.c
	gcc supplement.c -ggdb -O0 -Wall -Werror -c -o supplement.o

out: assembly.S supplement.o
	gcc assembly.S supplement.o -o out -no-pie

.PHONY: test
test: out
	./out
	python3 test_compile_error.py --no-make