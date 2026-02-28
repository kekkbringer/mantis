//expect 0
//tags: simple, short-circuit
int main() {
    int a = 0;
    1 || (a = 1);
    return a;
}