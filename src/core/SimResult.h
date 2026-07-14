#pragma once
// SimResult.h — Kết quả trả về sau khi chạy mô phỏng

#include "Types.h"
#include <vector>
#include <QString>

// =============================================================
// SimResult — Kết quả đầy đủ của một lần chạy mô phỏng
// =============================================================
struct SimResult {
    // Kết quả BER tại từng điểm Eb/N0
    std::vector<SimPointResult> points;

    // Kết quả kiểm tra lỗi đơn (single flip)
    ManualFlipResult singleFlip;

    // Kết quả kiểm tra lỗi kép (double flip)
    ManualFlipResult doubleFlip;

    // Log text để hiển thị lên giao diện
    std::vector<QString> logs;
};
