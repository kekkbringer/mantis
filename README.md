# Mantis

A small C compiler written from scratch.

`mantis` parses C source code, performs semantic analysis, lowers it to an intermediate representation, generates x86 assembly, and produces native executables. Currently, preprocessing, assembling and linking is done by calling GCC.

---

## Features

 - Recursive descent parser
 - Abstract Syntax Tree (AST) construction
 - Semantic analysis with symbol table
 - Lowering to a three-address code (TAC) intermediate representation
 - x86 code generation
 - Automated test suite (Release + Debug with sanitizers)
 - Continuous Integration on Linux

---

## Building

### Requirements

 - CMake
 - GCC or Clang (need to support C++23)
 - Python3 (for the test runner)

### Build
 Use these commands from within the root directory to build `mantis`.

 ```bash
 cmake -S . -B build
 cmake --build build
 ```

 By specifying `-DCMAKE_BUILD_TYPE=Debug`, the debug version can be build.

---

## Running

Compile a C file:

 ```bash
 mantis input.c
 ```

This produces a native executable from the given C source file. As mentioned above, a working GCC installation must be present and found in the $PATH, so that `mantis` can use the required GCC utilities.

---

## Testing

The project includes an automated test suite driven by CTest.

To run the tests:

 ```bash
 cd build
 ctest --output-on-failure
 ```

CI runs tests in:
 - Release mode
 - Debug mode with AddressSanitizer, UndefinedBehaviourSanitizer, and LeakSanitizer

---

## Project Status

`mantis` is under active development.
The focus is correctness, clean architecture, and well-tested compiler infrastructure.
