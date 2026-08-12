#!/usr/bin/env python3
"""Static release checks for the FreeHeliports native x64 plugin."""

from __future__ import annotations

import struct
import sys
from pathlib import Path


def fail(message: str) -> None:
    raise SystemExit(f"FAILED: {message}")


def pe_checksum(data: bytes, checksum_offset: int) -> int:
    buf = bytearray(data)
    buf[checksum_offset:checksum_offset + 4] = b"\0\0\0\0"
    padded = bytes(buf) + b"\0" * ((4 - len(buf) % 4) % 4)
    checksum = 0
    for offset in range(0, len(padded), 4):
        dword = struct.unpack_from("<I", padded, offset)[0]
        checksum = (checksum & 0xFFFFFFFF) + dword + (checksum >> 32)
        if checksum > 0xFFFFFFFF:
            checksum = (checksum & 0xFFFFFFFF) + (checksum >> 32)
    checksum = (checksum & 0xFFFF) + (checksum >> 16)
    checksum += checksum >> 16
    return ((checksum & 0xFFFF) + len(buf)) & 0xFFFFFFFF


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
    if checksum != pe_checksum(data, optional + 0x40): fail("invalid PE checksum")
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
    sections = []
    for index in range(section_count):
        offset = section_table + index * 40
        name = data[offset:offset + 8].rstrip(b"\0").decode("ascii", "replace")
        virtual_size, virtual_address, raw_size, raw_pointer = struct.unpack_from(
            "<IIII", data, offset + 8
        )
        sections.append((virtual_address, raw_pointer, max(virtual_size, raw_size)))
        flags = struct.unpack_from("<I", data, offset + 36)[0]
        writable = bool(flags & 0x80000000)
        executable = bool(flags & 0x20000000)
        if writable and executable: fail(f"section {name} is writable and executable")

    def rva_to_file(rva: int) -> int:
        for section_rva, section_raw, section_size in sections:
            if section_rva <= rva < section_rva + section_size:
                return section_raw + rva - section_rva
        fail(f"unmapped RVA 0x{rva:X}")

    # Windows IMAGE_IMPORT_BY_NAME records are WORD-aligned. Reject malformed
    # layouts before Windows reports STATUS_INVALID_IMAGE_FORMAT (0xC000007B).
    import_rva, _ = directories[1]
    descriptor = rva_to_file(import_rva)
    imports = set()
    while True:
        original_thunk, timestamp, forwarder, dll_name, first_thunk = struct.unpack_from(
            "<IIIII", data, descriptor
        )
        if not any((original_thunk, timestamp, forwarder, dll_name, first_thunk)):
            break
        thunk_rva = original_thunk or first_thunk
        thunk = rva_to_file(thunk_rva)
        while True:
            value = struct.unpack_from("<Q", data, thunk)[0]
            if value == 0:
                break
            if not value & (1 << 63):
                name_rva = value & 0x7FFFFFFF_FFFFFFFF
                if name_rva & 1: fail(f"misaligned IMAGE_IMPORT_BY_NAME RVA 0x{name_rva:X}")
                name_offset = rva_to_file(name_rva) + 2
                end = data.find(b"\0", name_offset)
                if end < 0: fail("unterminated import name")
                imports.add(data[name_offset:end])
            thunk += 8
        descriptor += 20
    required_imports = {b"VirtualProtect", b"FlushInstructionCache", b"GetCurrentProcess"}
    if imports != required_imports: fail(f"unexpected imports: {sorted(imports)!r}")

    for required in (b"TsmPluginApiVersion\0", b"TsmPluginInit\0",
                     b"FreeHeliports\0", b"1.1.0\0"):
        if required not in data: fail(f"missing release marker {required!r}")
    if b"RSDS" in data or b".pdb" in data.lower(): fail("embedded PDB/debug path")
    if bytes.fromhex("48 8B C4 44 88 48 20 F3 0F 11 50 18 48 89 50 10") not in data:
        fail("missing audited WRSR 1.1.1.9 prologue marker")
    if b"\x70\x90\x15\x00" not in data: fail("missing WRSR 1.1.1.9 hook RVA")
    print(f"PASS: {path.name} is a hardened native x64 FreeHeliports v1.1.0 DLL")


if __name__ == "__main__":
    main()
