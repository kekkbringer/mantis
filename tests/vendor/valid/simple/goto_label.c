//expect 1
//tags: simple, goto
int main() {
    goto label;
    return 0;
label:
    return 1;
}