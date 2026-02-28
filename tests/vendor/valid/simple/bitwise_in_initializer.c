//expect 11
//tags: simple, compound
int main() {
    int a = 15;
    int b = a ^ 5;  // 10
    return 1 | b;   // 11
}