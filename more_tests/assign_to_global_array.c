// EXPECT stderr cannot directly assign to global array
// EXPECT rc 1
int array[10];
int f() {
    array = (int* ) 7;
}