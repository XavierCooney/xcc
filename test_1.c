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

int do_test() {
    do_basic_arithmetic_test();
    do_var_test();
    supplement_assert(fib(21) == 10946);
    supplement_print_nl();
}

int fib(int n) {
    if(n <= 1) {
        return n;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

int main() {
    hello_world();
    do_test();
    return 0;
}