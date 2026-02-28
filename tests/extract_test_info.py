#!/usr/bin/env python3
import sys
import re

def extract_tags_and_expect(path):
    expect = None
    tags = []
    with open(path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if expect is None:
                m_expect = re.match(r"//\s*expect\s+(-?\d+)", line)
                if m_expect:
                    expect = m_expect.group(1)
            if not tags:
                m_tags = re.match(r"//\s*tags\s*:\s*(.*)", line)
                if m_tags:
                    tags = [t.strip() for t in m_tags.group(1).split(",") if t.strip()]
            if expect and tags:
                break
    print(expect, *tags)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(1)
    extract_tags_and_expect(sys.argv[1])