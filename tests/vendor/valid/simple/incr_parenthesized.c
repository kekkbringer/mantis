//expect 1
//tags: simple, increment, decrement
int main() {
    int a = 1;
    int b = 2;
    int c = -++(a);
    int d = !(b)--;
    return (a == 2 && b == 1 && c == -2 && d == 0);
}