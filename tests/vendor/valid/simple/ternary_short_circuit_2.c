//expect 2
//tags: simple, conditional, short-circuit
int main() {
    int a = 0;
    int b = 0;
    a ? (b = 1) : (b = 2);
    return b;
}