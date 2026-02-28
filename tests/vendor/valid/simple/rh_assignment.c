//expect 1
//tags: simple, conditional
int main() {
    int flag = 1;
    int a = 0;
    flag ? a = 1 : (a = 0);
    return a;
}