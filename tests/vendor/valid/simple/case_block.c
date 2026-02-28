//expect 1
//tags: simple, switch
int main() {
    int a = 4;
    int b = 0;
    switch(2) {
        // the substatement of a case statement may be a compound statement
        case 2: {
            int a = 8;
            b = a;
        }
    }
    return (a == 4 && b == 8);
}