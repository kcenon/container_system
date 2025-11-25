#!/usr/bin/env python3
"""
Generate seed corpus for fuzzing.

Creates valid and edge-case serialized data to seed the fuzzer
for better coverage of the deserialization code.
"""

import os
import struct
from pathlib import Path


def generate_value_corpus(output_dir: Path):
    """Generate seed corpus for value deserialization."""
    output_dir.mkdir(parents=True, exist_ok=True)

    # Valid value types (based on value_types enum)
    # 0 = null_value
    # 1 = bool_value
    # 2 = short_value (int16)
    # 3 = ushort_value (uint16)
    # 4 = int_value (int32)
    # 5 = uint_value (uint32)
    # 6 = llong_value (int64)
    # 7 = ullong_value (uint64)
    # 8 = float_value
    # 9 = double_value
    # 10 = bytes_value
    # 11 = string_value
    # 12 = container_value
    # 13 = array_value

    def make_value(name: bytes, type_byte: int, data: bytes) -> bytes:
        """Create a serialized value."""
        name_len = struct.pack("<I", len(name))
        return name_len + name + bytes([type_byte]) + data

    # Null value
    with open(output_dir / "null_value", "wb") as f:
        f.write(make_value(b"null_test", 0, b""))

    # Bool values
    with open(output_dir / "bool_true", "wb") as f:
        f.write(make_value(b"bool_true", 1, bytes([1])))

    with open(output_dir / "bool_false", "wb") as f:
        f.write(make_value(b"bool_false", 1, bytes([0])))

    # Int16 value
    with open(output_dir / "int16_value", "wb") as f:
        f.write(make_value(b"short", 2, struct.pack("<h", -12345)))

    # Uint16 value
    with open(output_dir / "uint16_value", "wb") as f:
        f.write(make_value(b"ushort", 3, struct.pack("<H", 65000)))

    # Int32 value
    with open(output_dir / "int32_value", "wb") as f:
        f.write(make_value(b"int", 4, struct.pack("<i", -123456789)))

    # Uint32 value
    with open(output_dir / "uint32_value", "wb") as f:
        f.write(make_value(b"uint", 5, struct.pack("<I", 0xDEADBEEF)))

    # Int64 value
    with open(output_dir / "int64_value", "wb") as f:
        f.write(make_value(b"llong", 6, struct.pack("<q", -9223372036854775807)))

    # Uint64 value
    with open(output_dir / "uint64_value", "wb") as f:
        f.write(make_value(b"ullong", 7, struct.pack("<Q", 0xDEADBEEFCAFEBABE)))

    # Float value
    with open(output_dir / "float_value", "wb") as f:
        f.write(make_value(b"float", 8, struct.pack("<f", 3.14159)))

    # Double value
    with open(output_dir / "double_value", "wb") as f:
        f.write(make_value(b"double", 9, struct.pack("<d", 2.718281828459045)))

    # String value
    test_string = b"Hello, World!"
    string_data = struct.pack("<I", len(test_string)) + test_string
    with open(output_dir / "string_value", "wb") as f:
        f.write(make_value(b"string", 11, string_data))

    # Bytes value
    test_bytes = bytes(range(256))
    bytes_data = struct.pack("<I", len(test_bytes)) + test_bytes
    with open(output_dir / "bytes_value", "wb") as f:
        f.write(make_value(b"bytes", 10, bytes_data))

    # Edge cases
    with open(output_dir / "empty", "wb") as f:
        f.write(b"")

    with open(output_dir / "minimal", "wb") as f:
        f.write(b"\x00\x00\x00\x00")

    with open(output_dir / "truncated_name_len", "wb") as f:
        f.write(b"\xFF\xFF")  # Incomplete length

    with open(output_dir / "huge_name_len", "wb") as f:
        f.write(struct.pack("<I", 0xFFFFFFFF) + b"x")

    with open(output_dir / "invalid_type", "wb") as f:
        f.write(make_value(b"test", 255, b"\x00\x00\x00\x00"))

    with open(output_dir / "zero_name", "wb") as f:
        f.write(make_value(b"", 4, struct.pack("<i", 42)))

    print(f"Generated {len(list(output_dir.iterdir()))} seed files in {output_dir}")


def generate_container_corpus(output_dir: Path):
    """Generate seed corpus for container deserialization."""
    output_dir.mkdir(parents=True, exist_ok=True)

    def make_value(name: bytes, type_byte: int, data: bytes) -> bytes:
        """Create a serialized value."""
        name_len = struct.pack("<I", len(name))
        return name_len + name + bytes([type_byte]) + data

    def make_container(values: list) -> bytes:
        """Create a serialized container."""
        count = struct.pack("<I", len(values))
        return count + b"".join(values)

    # Empty container
    with open(output_dir / "empty_container", "wb") as f:
        f.write(make_container([]))

    # Single value container
    with open(output_dir / "single_value", "wb") as f:
        value = make_value(b"test", 4, struct.pack("<i", 42))
        f.write(make_container([value]))

    # Multiple values container
    with open(output_dir / "multiple_values", "wb") as f:
        values = [
            make_value(b"int_val", 4, struct.pack("<i", 123)),
            make_value(b"bool_val", 1, bytes([1])),
            make_value(b"str_val", 11, struct.pack("<I", 4) + b"test"),
        ]
        f.write(make_container(values))

    # Large count (edge case)
    with open(output_dir / "huge_count", "wb") as f:
        f.write(struct.pack("<I", 0xFFFFFFFF))

    # Corrupted container
    with open(output_dir / "corrupted", "wb") as f:
        f.write(struct.pack("<I", 5) + b"\x00" * 10)  # Claims 5 values but data is garbage

    # Edge cases
    with open(output_dir / "empty", "wb") as f:
        f.write(b"")

    with open(output_dir / "partial_count", "wb") as f:
        f.write(b"\x01\x00")  # Incomplete count

    print(f"Generated {len(list(output_dir.iterdir()))} seed files in {output_dir}")


def main():
    base_dir = Path("corpus")

    generate_value_corpus(base_dir / "deserialize")
    generate_container_corpus(base_dir / "container")

    print("\nCorpus generation complete!")
    print(f"Total files: {sum(1 for _ in base_dir.rglob('*') if _.is_file())}")


if __name__ == "__main__":
    main()
