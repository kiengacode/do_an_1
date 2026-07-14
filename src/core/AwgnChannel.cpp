// AwgnChannel.cpp — Sinh nhiễu Gaussian và cộng vào tín hiệu

#include "AwgnChannel.h"
#include <cmath>
#include <random>

std::vector<double> AwgnChannel::addNoise(
    const std::vector<double>& txSymbols,
    double ebN0Db,
    double codeRate,
    unsigned int seed)
{
    // Bước 1: Chuyển Eb/N0 từ dB sang giá trị tuyến tính
    const double ebN0Linear = std::pow(10.0, ebN0Db / 10.0);

    // Bước 2: Tính sigma (độ lệch chuẩn nhiễu)
    // sigma = sqrt(1 / (2 * R * EbNoLinear))
    const double sigma = std::sqrt(1.0 / (2.0 * codeRate * ebN0Linear));

    // Bước 3: Tạo bộ sinh số ngẫu nhiên và phân phối Gaussian
    std::mt19937 rng(seed);
    std::normal_distribution<double> noiseDist(0.0, sigma);

    // Bước 4: Cộng nhiễu vào từng ký hiệu: r = s + n
    std::vector<double> rxSymbols;
    rxSymbols.reserve(txSymbols.size());

    for (double s : txSymbols) {
        rxSymbols.push_back(s + noiseDist(rng));
    }

    return rxSymbols;
}
