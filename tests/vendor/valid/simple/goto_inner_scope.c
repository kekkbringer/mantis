//expect 1
//tags: simple, scoping, goto
int main() {
    int x = 5;
    goto inner;
    {
        int x = 0;
        inner:
        x = 1;
        return x;
    }
}