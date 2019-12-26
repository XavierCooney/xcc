#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

typedef int64_t i64;


void supplement_assert(i64 condition) {
    if(condition) {
        putchar('.');
    } else {
        assert(condition);
    }
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
