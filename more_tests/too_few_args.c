// EXPECT rc 1
// EXPECT stderr Parse error
// EXPECT stderr too few arguments in func call
int f(int x, int y);
int g() {
    f(3);
}