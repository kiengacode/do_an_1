#pragma once
// Types.h — Kiểu dữ liệu dùng chung trong toàn bộ dự án

#include <cstdint>
#include <vector>
#include <array>
#include <string>

// ---------------------------------------------------------
// Kiểu cơ bản
// ---------------------------------------------------------
using Bit       = uint8_t;           // Giá trị 0 hoặc 1
using BitVector = std::vector<Bit>;  // Chuỗi bit

// ---------------------------------------------------------
// Kết quả giải mã một codeword 7-bit
// ---------------------------------------------------------
struct HammingDecodeResult {
    std::array<Bit, 7> receivedCW;       // Codeword nhận được (trước sửa)
    std::array<Bit, 7> correctedCW;      // Codeword sau khi sửa
    std::array<Bit, 4> decodedBits;      // 4 bit thông tin sau giải mã
    int  syndrome       = 0;             // Giá trị syndrome (0–7)
    int  errorPosition  = 0;             // Vị trí lỗi 1-indexed (0 = không lỗi)
    bool corrected      = false;         // true nếu đã flip bit sửa lỗi
};

// ---------------------------------------------------------
// Kết quả tại một điểm Eb/N0
// ---------------------------------------------------------
struct SimPointResult {
    double ebN0Db         = 0.0;
    double sigma          = 0.0;  // Độ lệch chuẩn nhiễu (hệ Hamming)
    double berUncoded     = 0.0;  // BER hệ thống không mã hóa
    double berHamming     = 0.0;  // BER hệ thống Hamming sau giải mã
    int    errorsUncoded  = 0;
    int    errorsHamming  = 0;
    int    totalInfoBits  = 0;    // Tổng số Information Bits đã so sánh
};

// ---------------------------------------------------------
// Kết quả kiểm tra Manual Flip (single hoặc double)
// ---------------------------------------------------------
struct ManualFlipResult {
    bool applied = false;  // true nếu đã thực hiện flip

    std::array<Bit, 7> originalCW   = {};  // Codeword gốc (sau encode)
    std::array<Bit, 7> cwAfterFlip  = {};  // Codeword sau khi flip
    std::array<Bit, 7> cwCorrected  = {};  // Codeword sau khi Decoder sửa
    std::array<Bit, 4> decodedBits  = {};  // 4 bit thông tin thu được

    int syndrome          = 0;
    int detectedErrPos    = 0;   // Vị trí Decoder phát hiện (0 = không phát hiện)

    int  flipPos1         = 0;   // Vị trí flip thứ nhất (1-indexed, 1–7)
    int  flipPos2         = 0;   // Vị trí flip thứ hai (dành cho double flip)

    bool correctionCorrect = false;  // Sửa đúng dữ liệu gốc hay không

    std::string conclusion;  // Kết luận bằng text
};
