//expect 1
//tags: simple, loop
int main() {
    int a = 10;
    while ((a = 1))
        break;
    return a;
}