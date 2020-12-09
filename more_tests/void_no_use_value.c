// EXPECT rc 1
// EXPECT stderr Type mismatch!
// EXPECT stderr Got: i0, but needed: i64
void f() {
    // hmmm
    int a = 3 + 4;
}

int g() {
    int x = f();
}