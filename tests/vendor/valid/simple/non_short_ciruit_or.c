//expect 1
//tags: simple, short-circuit, binary, logical, variables
int main() {
    int a = 0;
    0 || (a = 1);
    return a;
}