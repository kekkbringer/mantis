//expect 1
//tags: simple, conditional
int main() {
    int x = 10;
    int y = 0;
    y = (x = 5) ? x : 2;
    return (x == 5 && y == 5);
}