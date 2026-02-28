//expect 1
//tags: simple, increment, decrement
int main() {
    int a = 1;
    int b = !a++;
    return (a == 2 && b == 0);
}