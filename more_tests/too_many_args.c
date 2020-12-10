// EXPECT rc 1
// EXPECT stderr Parse error
// EXPECT stderr too many arguments in function call
int f(int x, int y);
int g() {
    f(3, 4, 5);
}