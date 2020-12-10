// Another regression test
// The stack variable test was checking for one more that
// it should've, meaning stale entries were being checked
// EXPECT rc 1
// EXPECT stderr Variable `x` not found
int f() {
    if(1 > 0) {
        int x = 4;
    } else {
        x;
    }
}