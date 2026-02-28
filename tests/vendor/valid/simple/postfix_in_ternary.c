//expect 9
//tags: simple, conditional
int main() {
    int x = 10;
    x - 10 ? 0 : x--;
    return x;
}