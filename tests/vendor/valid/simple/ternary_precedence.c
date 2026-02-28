//expect 20
//tags: simple, conditional, precedence
int main() {
    int a = 10;
    return a || 0 ? 20 : 0; // test that || is higher precedence than ?
}