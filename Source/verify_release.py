#!/usr/bin/env python3
"""Static release checks for the FreeHeliports native x64 plugin."""

from __future__ import annotations

import struct
import sys
from pathlib import Path


def fail(message: str) -> None:
    raise SystemExit(f"FAILED: {message}")


def main() -> None:
    path = Path(sys.argv[1] if len(sys.argv) > 1 else "free_heliports.dll")
    data = path.read_bytes()
    if data[:2] != b"MZ": fail("missing MZ signature")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0": fail("missing PE signature")

    coff = pe + 4
    machine, section_count, _, _, _, optional_size, characteristics = struct.unpack_from(
        "<HHIIIHH", data, coff
    )
    if machine != 0x8664: fail("not x86-64")
    if not characteristics & 0x2000: fail("not marked as a DLL")

    optional = coff + 20
    if struct.unpack_from("<H", data, optional)[0] != 0x20B: fail("not PE32+")
    checksum = struct.unpack_from("<I", data, optional + 0x40)[0]
    dll_flags = struct.unpack_from("<H", data, optional + 0x46)[0]
    if checksum == 0: fail("zero PE checksum")
    required_flags = 0x20 | 0x40 | 0x100
    if dll_flags & required_flags != required_flags:
        fail("ASLR/high-entropy/NX flags are incomplete")

    directory = optional + 0x70
    directories = [struct.unpack_from("<II", data, directory + i * 8) for i in range(16)]
    for index, label in ((0, "exports"), (1, "imports"), (2, "resources"),
                         (3, "exception/unwind"), (5, "relocations")):
        if directories[index] == (0, 0): fail(f"missing {label} directory")
    if directories[14] != (0, 0): fail("unexpected CLR payload")

    section_table = optional + optional_size
    for index in range(section_count):
        offset = section_table + index * 40
        name = data[offset:offset + 8].rstrip(b"\0").decode("ascii", "replace")
        flags = struct.unpack_from("<I", data, offset + 36)[0]
        writable = bool(flags & 0x80000000)
        executable = bool(flags & 0x20000000)
        if writable and executable: fail(f"section {name} is writable and executable")

    for required in (b"TsmPluginApiVersion\0", b"TsmPluginInit\0",
                     b"FreeHeliports\0", b"1.0.0\0"):
        if required not in data: fail(f"missing release marker {required!r}")
    if b"RSDS" in data or b".pdb" in data.lower(): fail("embedded PDB/debug path")
    if b"0.1.2-test" in data: fail("test-version marker remains")

    print(f"PASS: {path.name} is a hardened native x64 FreeHeliports v1.0.0 DLL")


if __name__ == "__main__":
    main()
