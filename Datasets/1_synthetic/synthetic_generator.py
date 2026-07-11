import argparse
import csv
import sys
import os


def multiple_sequential_scan(path):
    unique_keys_count = 50_000
    scan_count = 20
    with open(path, "w", encoding="utf-8") as file:
        file.write(f"int {unique_keys_count * scan_count}\n")
        for _ in range(scan_count):
            line = "\n".join(map(str, range(1, 1 + unique_keys_count))) + "\n"
            file.write(line)


def zig_zag_scan(path):
    unique_keys_count = 50_000
    scan_count = 20
    with open(path, "w", encoding="utf-8") as file:
        file.write(f"int {unique_keys_count * scan_count}\n")
        for i in range(scan_count):
            if i % 2 == 0:
                numbers = range(1, 1 + unique_keys_count)
            else:
                numbers = range(unique_keys_count, 0, -1)
            line = "\n".join(map(str, numbers)) + "\n"
            file.write(line)


def main():
    multiple_sequential_scan("multiple_sequential_scan.txt")
    zig_zag_scan("zig_zag_scan.txt")


if __name__ == "__main__":
    main()
