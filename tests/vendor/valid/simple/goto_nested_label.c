//expect 5
//tags: simple, goto
int main() {
    goto labelB;

    labelA:
        labelB:
            return 5;
    return 0;
}