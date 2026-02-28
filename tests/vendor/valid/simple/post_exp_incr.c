//expect 21
//tags: simple, loop
int main() {
    int product = 1;
    for (int i = 0; i < 10; i++) {
        product = product + 2;
    }
    return product;
}