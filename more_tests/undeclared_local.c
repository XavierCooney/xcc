// EXPECT rc 1
// EXPECT stderr Variable `b` not found!
int f() {
    int a = 3;
    b;
}