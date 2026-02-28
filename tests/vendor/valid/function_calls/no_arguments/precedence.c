//expect 0
//tags: functions, precedence
int three(void) {
    return 3;
}

int main(void) {
    /* The function call operator () is higher precedence
     * than unary prefix operators
     */
    return !three();
}