//tags: simple, switch
int main() {
    int a = 3;
    switch(a + 1) {
        case 0:
            a = 1;
        default: continue;
    }
    return a;
}