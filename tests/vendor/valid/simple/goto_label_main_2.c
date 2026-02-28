//expect 1
//tags: simple, goto
int main() {
    goto _main;
    return 0;
    _main:
        return 1;
}