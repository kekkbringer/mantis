//expect 1
//tags: simple, conditional, short-circuit
int main() {
    int a = 1;
    int b = 0;
    a ? (b = 1) : (b = 2);
    return b;
}