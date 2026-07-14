// Simulator.cpp — Cài đặt pipeline mô phỏng đầy đủ

#include "Simulator.h"
#include "Hamming74.h"
#include "Bpsk.h"
#include "AwgnChannel.h"
#include "BerEvaluator.h"
#include <random>
#include <cmath>

// ─────────────────────────────────────────────────────────────
// Hàm nội bộ: Sinh chuỗi bit ngẫu nhiên (bội số của 4)
// ─────────────────────────────────────────────────────────────
static BitVector generateBits(int count, unsigned int seed) {
    // Làm tròn lên bội số 4
    if (count % 4 != 0) count += (4 - count % 4);

    std::mt19937 rng(seed);
    std::bernoulli_distribution dist(0.5);

    BitVector bits(count);
    for (auto& b : bits) b = dist(rng) ? 1 : 0;
    return bits;
}

// ─────────────────────────────────────────────────────────────
// Hàm phụ: Thực hiện Manual Flip và trả về thông tin chi tiết
// ─────────────────────────────────────────────────────────────
ManualFlipResult Simulator::doManualFlip(
    const std::array<Bit, 7>& originalCW,
    const std::array<Bit, 7>& receivedCW,
    const std::array<Bit, 4>& originalData,
    int flipPos1,
    int flipPos2)
{
    ManualFlipResult res;
    res.applied     = true;
    res.flipPos1    = flipPos1;
    res.flipPos2    = flipPos2;
    res.originalCW  = originalCW;

    // Bắt đầu từ receivedCW (có thể đã có lỗi từ AWGN)
    std::array<Bit, 7> flipped = receivedCW;

    // Flip vị trí 1 (bắt buộc)
    if (flipPos1 >= 1 && flipPos1 <= 7) {
        flipped[flipPos1 - 1] ^= 1;
    }

    // Flip vị trí 2 (chỉ cho double flip)
    if (flipPos2 >= 1 && flipPos2 <= 7 && flipPos2 != flipPos1) {
        flipped[flipPos2 - 1] ^= 1;
    }

    res.cwAfterFlip = flipped;

    // Chạy Decoder Hamming trên codeword đã flip
    auto decRes = Hamming74::decodeBlock(flipped);
    res.syndrome       = decRes.syndrome;
    res.detectedErrPos = decRes.errorPosition;
    res.cwCorrected    = decRes.correctedCW;
    res.decodedBits    = decRes.decodedBits;

    // Kiểm tra xem kết quả giải mã có khớp với dữ liệu gốc không
    res.correctionCorrect = (res.decodedBits == originalData);

    // Tạo kết luận bằng text
    if (flipPos2 == 0) {
        // Single flip
        if (res.correctionCorrect) {
            res.conclusion = "Single-bit error: Hamming detected at position "
                           + std::to_string(res.detectedErrPos)
                           + ". Correction SUCCESSFUL.";
        } else {
            res.conclusion = "Single-bit error: Hamming detected at position "
                           + std::to_string(res.detectedErrPos)
                           + ". Correction FAILED (AWGN may have introduced additional errors).";
        }
    } else {
        // Double flip
        res.conclusion = "Double-bit error: Syndrome = "
                       + std::to_string(res.syndrome)
                       + " (detected pos "
                       + std::to_string(res.detectedErrPos)
                       + "). Hamming (7,4) CANNOT guarantee correction of double errors.";
        if (!res.correctionCorrect) {
            res.conclusion += " Data is INCORRECT after decode.";
        } else {
            res.conclusion += " (Data happened to be correct by coincidence.)";
        }
    }

    return res;
}

// ─────────────────────────────────────────────────────────────
// Hàm chính: Chạy toàn bộ mô phỏng
// ─────────────────────────────────────────────────────────────
SimResult Simulator::run(const SimConfig& cfg, ProgressCb progressCb, LogCb logCb) {
    SimResult result;

    auto log = [&](const QString& msg) {
        result.logs.push_back(msg);
        if (logCb) logCb(msg);
    };

    // ── Bước 1: Sinh Information Bits ──────────────────────────
    log("Generating random information bits...");
    BitVector dataBits = generateBits(cfg.numInfoBits, cfg.seed);
    const int numInfoBits = static_cast<int>(dataBits.size());
    log(QString("Total information bits: %1").arg(numInfoBits));

    // ── Bước 2: Hamming encode (dùng cho cả BER và Manual Flip) ─
    log("Encoding with Hamming (7,4)...");
    BitVector codedBits = Hamming74::encode(dataBits);
    log(QString("Coded bits (codewords): %1").arg(codedBits.size()));

    // Lưu lại codeword đầu tiên để dùng cho Manual Flip
    std::array<Bit, 7> firstCW = {};
    std::array<Bit, 4> firstData = {};
    if (codedBits.size() >= 7) {
        for (int i = 0; i < 7; ++i) firstCW[i]   = codedBits[i];
        for (int i = 0; i < 4; ++i) firstData[i] = dataBits[i];
    }

    // ── Bước 3: Duyệt từng mức Eb/N0 ─────────────────────────
    // Đếm tổng số bước để tính tiến trình
    int numSteps = 0;
    for (double snr = cfg.ebN0Min; snr <= cfg.ebN0Max + 1e-9; snr += cfg.ebN0Step)
        ++numSteps;
    if (numSteps == 0) numSteps = 1;

    int stepIdx = 0;
    for (double ebN0Db = cfg.ebN0Min; ebN0Db <= cfg.ebN0Max + 1e-9; ebN0Db += cfg.ebN0Step) {
        SimPointResult pt;
        pt.ebN0Db      = ebN0Db;
        pt.totalInfoBits = numInfoBits;

        // Tính sigma — cả hai hệ thống dùng cùng mức nhiễu (R=1)
        // Quy ước: Eb/N0 trục X = năng lượng per channel bit (coded bit)
        // Hệ thống Hamming cải thiện BER thuần túy nhờ khả năng sửa lỗi của decoder
        const double ebN0Lin = std::pow(10.0, ebN0Db / 10.0);
        pt.sigma = std::sqrt(1.0 / (2.0 * ebN0Lin));  // sigma chung cho cả hai nhánh

        // Seed khác nhau cho uncoded/coded, khác nhau theo step
        const unsigned int seedUncoded = cfg.seed + static_cast<unsigned>(stepIdx) * 2u;
        const unsigned int seedCoded   = cfg.seed + static_cast<unsigned>(stepIdx) * 2u + 1u;

        // ── Uncoded Branch ──────────────────────────────────────
        {
            // Modulate
            auto txUncoded = Bpsk::modulate(dataBits);
            // AWGN với R = 1
            auto rxUncoded = AwgnChannel::addNoise(txUncoded, ebN0Db, 1.0, seedUncoded);
            // Demodulate
            auto rxBits = Bpsk::demodulate(rxUncoded);
            // BER
            pt.errorsUncoded = BerEvaluator::countErrors(dataBits, rxBits);
            pt.berUncoded    = BerEvaluator::calcBer(pt.errorsUncoded, numInfoBits);
        }

        // ── Hamming Branch ──────────────────────────────────────
        {
            // Modulate (coded bits)
            auto txCoded = Bpsk::modulate(codedBits);
            // AWGN: dùng R = 1.0 (cùng mức nhiễu với uncoded)
            // → so sánh công bằng trên cùng channel SNR per bit
            auto rxCoded = AwgnChannel::addNoise(txCoded, ebN0Db, 1.0, seedCoded);
            // Demodulate
            BitVector demodBits = Bpsk::demodulate(rxCoded);
            // Hamming decode → lấy recovered info bits
            BitVector recoveredBits = Hamming74::decode(demodBits);
            // BER (so với dataBits gốc)
            pt.errorsHamming = BerEvaluator::countErrors(dataBits, recoveredBits);
            pt.berHamming    = BerEvaluator::calcBer(pt.errorsHamming, numInfoBits);
        }

        result.points.push_back(pt);
        log(QString("Eb/N0 = %1 dB | BER_uncoded = %2 | BER_hamming = %3")
            .arg(ebN0Db, 5, 'f', 1)
            .arg(pt.berUncoded,  10, 'e', 4)
            .arg(pt.berHamming,  10, 'e', 4));

        ++stepIdx;
        if (progressCb) progressCb(stepIdx * 100 / numSteps);
    }

    // ── Bước 4: Manual Flip (trên codeword đầu tiên) ──────────
    // Dùng seed cố định để codeword nhận được là xác định
    // (dùng Eb/N0 trung bình để minh họa)
    const double midEbN0 = (cfg.ebN0Min + cfg.ebN0Max) / 2.0;
    std::array<Bit, 7> firstCWReceived = firstCW; // Giả sử không có AWGN trên demo flip

    // Thực ra ta dùng kênh lý tưởng (không noise) cho codeword đầu tiên
    // để kết quả minh họa được sạch và rõ ràng
    (void)midEbN0;

    if (cfg.enableSingleFlip && firstCW[0] + firstCW[1] + firstCW[2] + firstCW[3]
                              + firstCW[4] + firstCW[5] + firstCW[6] >= 0) {
        log("Running single bit flip test on first codeword...");
        result.singleFlip = doManualFlip(
            firstCW, firstCWReceived, firstData,
            cfg.flipPos1, 0
        );
        log(QString::fromStdString("Single flip result: " + result.singleFlip.conclusion));
    }

    if (cfg.enableDoubleFlip && firstCW[0] + firstCW[1] + firstCW[2] + firstCW[3]
                              + firstCW[4] + firstCW[5] + firstCW[6] >= 0) {
        log("Running double bit flip test on first codeword...");
        result.doubleFlip = doManualFlip(
            firstCW, firstCWReceived, firstData,
            cfg.flipPos2a, cfg.flipPos2b
        );
        log(QString::fromStdString("Double flip result: " + result.doubleFlip.conclusion));
    }

    log("Simulation complete.");
    if (progressCb) progressCb(100);
    return result;
}
