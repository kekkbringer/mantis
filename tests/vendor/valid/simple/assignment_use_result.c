//expect 1
//tags: simple
int main() {
    int x = 1;
    int y = x += 3;
    return (x == 4 && y == 4);
}