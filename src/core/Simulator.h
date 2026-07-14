#pragma once
// Simulator.h — Pipeline orchestrator chính

#include "SimConfig.h"
#include "SimResult.h"
#include <functional>
#include <QString>

// =============================================================
// Simulator — Điều phối toàn bộ pipeline mô phỏng
//
// Pipeline tại mỗi mức Eb/N0:
//
//  Uncoded branch:
//    dataBits → BPSK mod → AWGN(R=1) → BPSK demod → BER
//
//  Hamming branch:
//    dataBits → Hamming encode → BPSK mod → AWGN(R=4/7)
//            → BPSK demod → [optional flip] → Hamming decode → BER
// =============================================================
class Simulator {
public:
    using ProgressCb = std::function<void(int percent)>;   // Callback cập nhật tiến trình
    using LogCb      = std::function<void(const QString&)>; // Callback ghi log

    // Chạy toàn bộ mô phỏng và trả về kết quả
    static SimResult run(
        const SimConfig& cfg,
        ProgressCb       progressCb = nullptr,
        LogCb            logCb      = nullptr
    );

private:
    // Xử lý Manual Flip và trả về thông tin chi tiết
    static ManualFlipResult doManualFlip(
        const std::array<Bit, 7>& originalCW, // Codeword gốc (sau encode)
        const std::array<Bit, 7>& receivedCW, // Codeword sau giải điều chế (có thể đã có lỗi AWGN)
        const std::array<Bit, 4>& originalData, // 4 bit dữ liệu gốc để kiểm tra
        int flipPos1,
        int flipPos2 = 0  // 0 = single flip
    );
};
