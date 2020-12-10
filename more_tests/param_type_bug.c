// Perhaps the compiler's first regression test
// I must have missed it from before, but the compiler
// wasn't properly setting the stack_variables.type_id
// for parameters
// EXPECT rc 0

int f() {
    int *a = (int*) 3; // fill stack_variables table with different type_id
}
int g(int b) {
    int c = b; // this would fail
}