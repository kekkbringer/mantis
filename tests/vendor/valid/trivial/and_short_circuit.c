//expect 0
//tags: trivial, binary, logical, short-circuit
int main(void) {
    return 0 && (1 / 0);
}