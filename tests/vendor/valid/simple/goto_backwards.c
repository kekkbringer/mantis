//expect 5
//tags: simple, goto
int main() {
    if (0)
    label:
        return 5;
    goto label;
    return 0;
}