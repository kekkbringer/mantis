//expect 21
//tags: trivial, binary, bitwise, precedence
int main(void) {
    return 80 >> 2 | 1 ^ 5 & 7 << 1;
}