//tags: functions
/* A function declaration can't have multiple storage class keywords */
static int extern foo(void) {
    return 0;
}

int main() {
    return foo();
}