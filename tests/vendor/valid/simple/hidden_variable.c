//expect 1
//tags: simple, scoping
int main() {
    int a = 2;
    {
        int a = 1;
        return a;
    }
}