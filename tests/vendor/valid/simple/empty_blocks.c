//expect 30
//tags: simple, scoping
int main() {
    int ten = 10;
    {}
    int twenty = 10 * 2;
    {{}}
    return ten + twenty;
}