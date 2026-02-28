//expect 1
//tags: simple, increment
int main() {
    int a = 2;
    int b = 3 + a++;
    int c = 4 + ++b;
    return (a == 3 && b == 6 && c == 10);
}