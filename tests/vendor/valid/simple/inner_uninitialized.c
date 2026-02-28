//expect 4
//tags: simple, scoping
int main() {
    int x = 4;
    {
        int x;
    }
    return x;
}