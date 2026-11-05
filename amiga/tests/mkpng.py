#!/usr/bin/env python3
"""mkpng.py: writes rgb_pngs.h for rgb_filter.c: synthetic 8-bit RGB PNGs
(colour type 2, no ancillary chunks), every row stored with one PNG filter
(0 None, 1 Sub, 2 Up, 3 Average, 4 Paeth), in sizes with rows of 90 bytes
(30 wide, as Starfighter's gfx/missileBoat1.png) and 96 bytes (32 wide)."""
import struct, zlib

def paeth(a, b, c):
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    return a if pa <= pb and pa <= pc else b if pb <= pc else c

def png(w, h, ftype):
    rows = [bytes(((x * 37 + y * 11) ^ (x * y)) & 255 for x in range(w) for _ in range(3)) for y in range(h)]
    rows = [bytes((v + i % 3 * 85) & 255 for i, v in enumerate(r)) for r in rows]
    bpp, out, prev = 3, b"", bytes(w * 3)
    for r in rows:
        f = bytearray()
        for i, v in enumerate(r):
            a = r[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            pred = [0, a, b, (a + b) // 2, paeth(a, b, c)][ftype]
            f.append((v - pred) & 255)
        out += bytes([ftype]) + bytes(f)
        prev = r
    chunk = lambda t, d: struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d) & 0xffffffff)
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(out, 9)) + chunk(b"IEND", b""))

items = [(w, 31, f) for w in (30, 32) for f in range(5)]
lines = ["/* Made by mkpng.py: synthetic 8-bit RGB PNGs, one filter per picture. */"]
for w, h, f in items:
    d = png(w, h, f)
    lines.append("static const unsigned char png_%d_%d[] = {%s};" % (w, f, ",".join(map(str, d))))
lines.append("static const struct { int w, h, filter; const unsigned char *data; int size; } pngs[] = {")
lines += ["\t{%d, %d, %d, png_%d_%d, sizeof png_%d_%d}," % (w, h, f, w, f, w, f) for w, h, f in items]
lines.append("};")
open("rgb_pngs.h", "w").write("\n".join(lines) + "\n")
