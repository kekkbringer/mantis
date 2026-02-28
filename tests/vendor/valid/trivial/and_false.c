//expect 0
//tags: trivial, binary, logical
int main() {
    return (10 && 0) + (0 && 4) + (0 && 0);
}