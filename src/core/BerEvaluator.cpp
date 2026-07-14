// BerEvaluator.cpp — Đếm lỗi bit và tính BER

#include "BerEvaluator.h"
#include <algorithm>

// Đếm số vị trí mà hai chuỗi bit có giá trị khác nhau
int BerEvaluator::countErrors(const BitVector& original, const BitVector& received) {
    int errors = 0;
    const size_t len = std::min(original.size(), received.size());

    for (size_t i = 0; i < len; ++i) {
        if (original[i] != received[i]) {
            ++errors;
        }
    }
    return errors;
}

// Tính BER = errors / totalBits
// Trả về 0.0 nếu totalBits == 0 để tránh chia cho 0
double BerEvaluator::calcBer(int numErrors, int totalBits) {
    if (totalBits <= 0) return 0.0;
    return static_cast<double>(numErrors) / static_cast<double>(totalBits);
}
