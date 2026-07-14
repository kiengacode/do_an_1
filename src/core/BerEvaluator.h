#pragma once
// BerEvaluator.h — Đánh giá tỷ lệ lỗi bit (BER)

#include "Types.h"

// =============================================================
// BerEvaluator — Bit Error Rate Evaluator
//
// BER chỉ được tính trên Information Bits (không phải codeword).
// BER = (Số bit lỗi) / (Tổng số bit so sánh)
// =============================================================
class BerEvaluator {
public:
    // Đếm số bit khác nhau giữa hai chuỗi cùng độ dài
    static int countErrors(const BitVector& original, const BitVector& received);

    // Tính BER từ số lỗi và tổng số bit
    static double calcBer(int numErrors, int totalBits);
};
