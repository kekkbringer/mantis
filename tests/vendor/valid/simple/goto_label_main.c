//expect 0
//tags: simple, goto
int main() {
    // it's legal to use main as both a function name and label
    goto main;
    return 5;
main:
    return 0;
}