//expect 1
//tags: trivial, binary, logical, precedence
int main() {
    return (0 == 0 && 3 == 2 + 1 > 1) + 1;
}