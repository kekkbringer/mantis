//expect 1
//tags: simple, scoping
int main() {
    int a;
    {
        int b = a = 1;
    }
    return a;
}