// Bpsk.cpp — Cài đặt điều chế/giải điều chế BPSK

#include "Bpsk.h"

// Điều chế: Bit 1 → +1.0, Bit 0 → -1.0
std::vector<double> Bpsk::modulate(const BitVector& inputBits) {
    std::vector<double> txSymbols;
    txSymbols.reserve(inputBits.size());

    for (Bit b : inputBits) {
        // Ánh xạ: 1 → +1.0, 0 → -1.0
        txSymbols.push_back((b == 1) ? 1.0 : -1.0);
    }
    return txSymbols;
}

// Giải điều chế Hard Decision: r > 0 → 1, r <= 0 → 0
BitVector Bpsk::demodulate(const std::vector<double>& rxSymbols) {
    BitVector demodBits;
    demodBits.reserve(rxSymbols.size());

    for (double r : rxSymbols) {
        demodBits.push_back((r > 0.0) ? 1 : 0);
    }
    return demodBits;
}
