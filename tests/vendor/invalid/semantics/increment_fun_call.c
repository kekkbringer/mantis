//tags: functions, lvalue
int x();

int main() {
    // a function call is not an lvalue, so we can't increment it
    ++x();
}