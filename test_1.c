// test comment
int putchar(int c);

int supplement_assert(int condition);
int supplement_print_int(int v);
int supplement_print_nl();
int supplement_print_char(int c);

int fib(int n);

int hello_world() {
    putchar(72);
    putchar(101);
    putchar(108);
    putchar(108);
    putchar(111);
    putchar(44);
    putchar(32);
    putchar(87);
    putchar(111);
    putchar(114);
    putchar(108);
    putchar(100);
    putchar(33);
    putchar(10);
}

int do_basic_arithmetic_test() {
    supplement_assert(1 == 1);
    supplement_assert(1 != 2);
    supplement_assert(1 + 1 == 2);
    supplement_assert(1 - 1 == 0);
    supplement_assert(-1 == 0 - 1);
    supplement_assert(4 - 3 == 1);
    supplement_assert(3 - 4 == -1);
    supplement_assert(3 - 4 == -1);

    supplement_assert(3 * 4 == 12);
    supplement_assert(4 * 3 == 12);

    supplement_assert(13 / 3 == 4);
    supplement_assert(0 / 3 == 0);

    supplement_assert(!0);
    supplement_assert(!!3);
    supplement_assert(!!3 == 1);

    supplement_assert(7 <= 8);
    supplement_assert(7 < 8);
    supplement_assert(8 <= 8);
    supplement_assert(!(8 < 8));
    supplement_assert(!(9 < 8));
    supplement_assert(!(9 <= 8));

    supplement_assert(3 > 0 - 7);
    supplement_assert(3 > -7);
    supplement_assert(3 >= 0 - 7);
    supplement_assert(3 >= -7);

    supplement_assert(8 >= 7);
    supplement_assert(8 > 7);
    supplement_assert(8 >= 8);
    supplement_assert(!(8 > 8));
    supplement_assert(!(8 > 9));
    supplement_assert(!(8 >= 9));

    supplement_assert(-4 > -10);
    supplement_assert(-4 >= -4);
}

int do_var_test() {
    int a = 3;
    supplement_assert(a);
    supplement_assert(a - 1);
    supplement_assert(a - 2);
    supplement_assert(a - 4);
    supplement_assert(a - 4);
    a = a + 2;
    supplement_assert(a == 5);
    supplement_assert(a != 3);

    int b = 0;
    supplement_assert(b == 0);
    supplement_assert(!b);

    int c = 12;
    supplement_assert(c == 12);
    supplement_assert(b == 0);
    supplement_assert(a == 5);

    supplement_assert(b == 0);
    supplement_assert(a == 5);
    supplement_assert(c == 12);
}


int fib(int n) {
    if(n <= 1) {
        return n;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

int do_while_test_1() {
    int total = 3;
    int i = 10;
    while(i < 17) {
        total = total + i;
        i = i + 3;
    }
    supplement_assert(total == 42); // 3 + 10 + 13 + 16
}

int do_while_test_2() {
    // check case of 0 loops
    int foo = 1;
    while(2 > 3) {
        foo = 2;
    }
    supplement_assert(foo == 1);
}

int identifier_test() {
    int _ = 0;
    int a = 1;
    int ab = 2;
    int c7 = 3;

    supplement_assert(_ == 0);
    supplement_assert(a == 1);
    supplement_assert(ab == 2);
    supplement_assert(c7 == 3);
}

int argument_passing_test_inner(int a, int b, int c, int d, int e) {
    supplement_assert(a == 1);
    supplement_assert(b == 2);
    supplement_assert(c == 3);
    supplement_assert(d == 4);
    supplement_assert(e == 5);
}

int argument_passing_test() {
    argument_passing_test_inner(1, 2, 3, 4, 5);
}

int do_test() {
    do_basic_arithmetic_test();
    do_var_test();
    supplement_assert(fib(21) == 10946);
    do_while_test_1();
    do_while_test_2();
    identifier_test();
    supplement_print_nl();
}

int main() {
    hello_world();
    do_test();
    return 0;
}