#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

typedef int64_t i64;

int tests_passed = 0;

void supplement_assert(i64 condition) {
    if(condition) {
        putchar('.');
        tests_passed++;
    } else {
        printf("\nassertion failed!\n");
        fflush(stdout);
        assert(condition);
    }
}

void supplement_print_stats() {
    printf("Passed %d tests!\n", tests_passed);
}

void supplement_print_int(i64 v) {
    printf("%ld", v);
}

void supplement_print_nl() {
    puts("");
}

void supplement_print_char(i64 c) {
    printf("%c", (int) c);
}

