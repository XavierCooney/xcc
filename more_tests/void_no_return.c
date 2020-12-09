// EXPECT rc 1
// EXPECT stderr Type mismatch!
// EXPECT stderr Got: i64, but needed: i0
void f() {
    return 3;
}