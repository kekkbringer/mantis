//tags: simple, loop, switch
int main() {
    // make sure our usual analysis of break/continue labels also traverses labeled statements
    label: break;
    return 0;
}