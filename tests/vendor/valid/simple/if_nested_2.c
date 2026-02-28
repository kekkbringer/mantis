//expect 2
//tags: simple, if
int main() {
    int a = 0;
    int b = 1;
    if (a)
        b = 1;
    else if (~b)
        b = 2;
    return b;
}