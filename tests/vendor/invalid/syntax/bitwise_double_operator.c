//tags: trivial
int main() {
    return 1 | | 2; // this is "| |" not "||" and should result in an error
}