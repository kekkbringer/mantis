//expect 255
//tags: trivial, binary, bitwise, implementation-defined
//right bitshift with negative value is implementation defined, mantis uses sign extension (like GCC)
int main() {
    return -5 >> 30;
}