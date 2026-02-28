//expect 4
//tags: simple, precedence
int main() {
    int a = 1;
    int b = 0;
    a = 3 * (b = a);
    return a + b;
}