#pragma once
// SimConfig.h — Cấu hình mô phỏng

// =============================================================
// SimConfig — Tất cả tham số đầu vào cho một lần chạy mô phỏng
// =============================================================
struct SimConfig {
    // --- Thông số chính ---
    int          numInfoBits  = 100000;  // Số Information Bits (phải là bội số 4)
    double       ebN0Min      = 0.0;     // Eb/N0 nhỏ nhất (dB)
    double       ebN0Max      = 10.0;    // Eb/N0 lớn nhất (dB)
    double       ebN0Step     = 1.0;     // Bước tăng Eb/N0 (dB)
    unsigned int seed         = 42;      // Seed sinh ngẫu nhiên (để tái lập kết quả)

    // --- Chức năng Manual Single Bit Flip ---
    // Áp dụng trên codeword đầu tiên, sau giải điều chế, trước giải mã
    bool enableSingleFlip = false;
    int  flipPos1         = 1;  // Vị trí bit cần flip trong codeword (1–7)

    // --- Chức năng Manual Double Bit Flip ---
    // Áp dụng đồng thời hai vị trí trên codeword đầu tiên
    bool enableDoubleFlip = false;
    int  flipPos2a        = 1;  // Vị trí bit flip thứ nhất (1–7)
    int  flipPos2b        = 2;  // Vị trí bit flip thứ hai  (1–7, khác flipPos2a)
};
