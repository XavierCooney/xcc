all: test_out


.PHONY: clean
clean:
	rm xcc test_out assembly.S

xcc: xcc.c
	gcc -Wall -Werror -ggdb xcc.c -o xcc

assembly.S: xcc test_1.c
	./xcc < test_1.c > assembly.S

test_out: assembly.S
	gcc assembly.S -o test_out