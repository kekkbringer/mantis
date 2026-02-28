//expect 3
//tags: trivial, binary, logical
int main() {
    return (4 || 0) + (0 || 3) + (5 || 5);
}