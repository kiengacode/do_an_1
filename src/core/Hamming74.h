#pragma once
// Hamming74.h — Mã hóa và giải mã Hamming (7,4) Systematic

#include "Types.h"

// =============================================================
// Hamming74 — Encoder & Decoder
//
// Quy ước codeword (systematic form):
//   Position:  1   2   3   4   5   6   7
//   Content:  p1  p2  d1  p3  d2  d3  d4
//
// Công thức parity (even parity):
//   p1 = d1 ⊕ d2 ⊕ d4   (kiểm tra vị trí 1,3,5,7)
//   p2 = d1 ⊕ d3 ⊕ d4   (kiểm tra vị trí 2,3,6,7)
//   p3 = d2 ⊕ d3 ⊕ d4   (kiểm tra vị trí 4,5,6,7)
//
// Syndrome (từ parity check matrix H):
//   s1 = r1 ⊕ r3 ⊕ r5 ⊕ r7
//   s2 = r2 ⊕ r3 ⊕ r6 ⊕ r7
//   s3 = r4 ⊕ r5 ⊕ r6 ⊕ r7
//   syndrome = s1 + 2*s2 + 4*s3  → chỉ ra vị trí lỗi (1-indexed)
// =============================================================

class Hamming74 {
public:
    // --- Encoder ---

    // Mã hóa 4 bit thông tin thành 7 bit codeword
    static std::array<Bit, 7> encodeBlock(const std::array<Bit, 4>& data);

    // Mã hóa toàn bộ chuỗi bit (phải là bội số của 4)
    static BitVector encode(const BitVector& dataBits);


    // --- Decoder ---

    // Giải mã 1 codeword 7-bit, trả về thông tin chi tiết
    static HammingDecodeResult decodeBlock(const std::array<Bit, 7>& received);

    // Giải mã toàn bộ chuỗi bit (phải là bội số của 7)
    // Trả về chuỗi Information Bits đã giải mã
    static BitVector decode(const BitVector& receivedBits);


    // --- Utility ---

    // Tính syndrome của một codeword 7-bit
    // Trả về giá trị 0–7 (0 = không lỗi, 1–7 = vị trí lỗi)
    static int calcSyndrome(const std::array<Bit, 7>& cw);
};
