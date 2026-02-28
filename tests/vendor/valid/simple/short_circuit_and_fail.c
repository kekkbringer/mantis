//expect 0
//tags: simple, short-circuit
int main() {
    int a = 0;
    0 && (a = 5);
    return a;
}