//expect 1
//tags: simple, goto
int main() {
    goto _foo_1_;  // a label may include numbers and underscores
    return 0;
_foo_1_:
    return 1;
}