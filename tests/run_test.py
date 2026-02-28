#!/usr/bin/env python3
import sys
import os
import re
import subprocess

def parse_test_file(path):
    """
    Parses <path> for:
      //expect <number>
      //output "<string>"
      //tags: tag1, tag2, ...
      //mantisargs: arg1 arg2 ...
    Returns (expect:int|None, expected_output:str|None, tags:List[str], mantis_args:List[str])
    """
    expect = None
    expected_output = None
    tags = []
    mantis_args = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()

            m = re.match(r"//\s*expect\s+(-?\d+)", line)
            if m:
                expect = int(m.group(1))
                continue

            m = re.match(r'//\s*output\s+"(.*)"', line)
            if m:
                expected_output = m.group(1)
                continue

            m = re.match(r"//\s*tags\s*:\s*(.*)", line)
            if m:
                tags = [t.strip() for t in m.group(1).split(",") if t.strip()]
                continue

            m = re.match(r"//\s*mantisargs\s*:\s*(.*)", line)
            if m:
                # split by whitespace into individual args
                mantis_args = m.group(1).split()
                continue

    return expect, expected_output, tags, mantis_args

def run_test(test_file, mantis_executable):
    expect, expected_output, tags, mantis_args = parse_test_file(test_file)

    # detect invalid tests by directory name
    is_invalid = "/invalid/" in test_file.replace("\\", "/")

    # valid tests must have an expected result
    if (not is_invalid) and (expect is None):
        print(f"[ERROR] no //expect in {test_file}")
        return 2

    # compile the test
    comp_cmd = [mantis_executable] + [os.path.basename(test_file)] + mantis_args
    comp = subprocess.run(comp_cmd,
                          cwd=os.path.dirname(test_file),
                          text=True)
    if is_invalid:
        if comp.returncode != 0:
            print(f"PASS (compile failure as expected): {test_file}")
            return 0
        else:
            print(f"FAIL (unexpected compile success): {test_file}")
            return 1

    # must compile successfully
    if comp.returncode != 0:
        print(f"FAIL (compiler error): {test_file}")
        return 1

    # run the produced binary
    bin_path = os.path.splitext(test_file)[0]
    if not os.path.exists(bin_path):
        print(f"FAIL (binary not found): {bin_path}")
        return 1

    run = subprocess.run([bin_path],
                         cwd=os.path.dirname(bin_path),
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE,
                         text=True)

    # clean up binary
    try:
        os.remove(bin_path)
    except OSError:
        pass

    # check return code
    if run.returncode != expect:
        print(f"FAIL: {test_file} (tags: {', '.join(tags)})")
        print(f"  expected exit code {expect}, got {run.returncode}")
        print(run.stdout, run.stderr, sep="\n")
        return 1

    # check stdout if specified
    if expected_output is not None:
        # decode escape sequences like '\n' into actual characters
        expected_output = expected_output.encode('raw_unicode_escape').decode("unicode_escape")
        actual_output = run.stdout
        if actual_output != expected_output:
            print(f"FAIL: {test_file} (tags: {', '.join(tags)})")
            print(f"  expected output: '{expected_output}'")
            print(f"  actual output:   '{actual_output}'")
            return 1

    print(f"PASS: {test_file} (tags: {', '.join(tags)})")
    return 0

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: run_test.py <test_file.c> <mantis_executable>")
        sys.exit(2)
    sys.exit(run_test(sys.argv[1], sys.argv[2]))
