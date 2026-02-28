//expect 1
//tags: simple, increment, decrement
int main() {
    int a = 0;
    int b = 0;
    a++;
    ++a;
    ++a;
    b--;
    --b;
    return (a == 3 && b == -2);
}