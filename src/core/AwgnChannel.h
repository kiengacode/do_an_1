#pragma once
// AwgnChannel.h — Mô hình kênh truyền AWGN

#include <vector>

// =============================================================
// AwgnChannel — Additive White Gaussian Noise Channel
//
// Mô hình kênh: r = s + n
//   s: tín hiệu BPSK phát đi
//   n: nhiễu Gaussian trắng, n ~ N(0, sigma^2)
//
// Công thức tính sigma:
//   EbNoLinear = 10^(ebN0Db / 10)
//   sigma = sqrt(1 / (2 * R * EbNoLinear))
//   Trong đó:
//     R = 1       → hệ thống không mã hóa
//     R = 4.0/7.0 → hệ thống Hamming (7,4)
// =============================================================
class AwgnChannel {
public:
    // Thêm nhiễu AWGN vào chuỗi ký hiệu BPSK
    // txSymbols : chuỗi ký hiệu phát (+1.0 hoặc -1.0)
    // ebN0Db    : Eb/N0 theo đơn vị dB
    // codeRate  : Code rate R (1.0 hoặc 4.0/7.0)
    // seed      : Seed cho bộ sinh số ngẫu nhiên (để tái lập kết quả)
    static std::vector<double> addNoise(
        const std::vector<double>& txSymbols,
        double ebN0Db,
        double codeRate,
        unsigned int seed
    );
};
