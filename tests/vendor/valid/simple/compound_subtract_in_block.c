//expect 1
//tags: simple, scoping
int main() {
    int a = 5;
    if (a > 4) {
        a -= 4;
        int a = 5;
        if (a > 4) {
            a -= 4;
        }
    }
    return a;
}