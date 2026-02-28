//expect 3
//tags: functions
int main(void) {
    int f(void);
    int f(void);
    return f();
}

int f(void) {
    return 3;
}