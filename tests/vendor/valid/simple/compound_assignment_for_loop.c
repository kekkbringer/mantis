//expect 1
//tags: simple, loop
int main() {
    int i = 1;
    for (i *= -1; i >= -100; i -=3)
        ;
    return (i == -103);
}