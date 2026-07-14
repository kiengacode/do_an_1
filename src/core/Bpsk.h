#pragma once
// Bpsk.h — Điều chế và giải điều chế BPSK

#include "Types.h"
#include <vector>

// =============================================================
// Bpsk — Binary Phase Shift Keying
//
// Quy ước điều chế:
//   Bit 0 → +1.0
//   Bit 1 → -1.0
//
// Giải điều chế (Hard Decision):
//   r >= 0 → Bit 0
//   r <  0 → Bit 1
// =============================================================
class Bpsk {
public:
    // Điều chế: chuyển chuỗi bit thành chuỗi ký hiệu +1.0/-1.0
    // Tham số: inputBits — chuỗi bit cần điều chế
    static std::vector<double> modulate(const BitVector& inputBits);

    // Giải điều chế Hard Decision: chuyển tín hiệu nhận về chuỗi bit
    // Tham số: rxSymbols — chuỗi tín hiệu sau kênh truyền
    static BitVector demodulate(const std::vector<double>& rxSymbols);
};
