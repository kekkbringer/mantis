//expect 1
//tags: functions
int foo() {
    return 2;
}

int main() {
    int x = 3;
    x -= foo();
    return x;
}