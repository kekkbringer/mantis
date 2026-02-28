//expect 1
//tags: simple, conditional
int main() {
    int x = 10;
    (x -= 1) ? (x /= 2) : 0;
    return x == 4;
}