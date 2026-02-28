//expect 1
//tags: simple, compound
int main() {
    int a = 11;
    int b = 12;
    a &= 0 || b;  // a = 1
    b ^= a || 1;  // b = 13

    int c = 14;
    c |= a || b;  // c = 15

    int d = 16;
    d >>= c || d;  // d = 8

    int e = 18;
    e <<= c || d; // e = 36
    return (a == 1 && b == 13 && c == 15 && d == 8 && e == 36);
}