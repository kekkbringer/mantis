//expect 0
//tags: trivial, binary, logical, short-circuit
int main() {
    return 0 || 0 && (1 / 0);
}