// EXPECT rc 1
// EXPECT stderr Type mismatch!
// EXPECT stderr Got: i64*, but needed: i64
int f() {
    int *a = (int*) 3;
    int *b = (int*) 5;
    a + b;
}