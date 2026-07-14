// Hamming74.cpp — Cài đặt Hamming (7,4) Encoder/Decoder

#include "Hamming74.h"
#include <stdexcept>

// -------------------------------------------------------------
// Mã hóa 1 block 4 bit → 7 bit codeword
// data[0]=d1, data[1]=d2, data[2]=d3, data[3]=d4
// Codeword: [p1 p2 d1 p3 d2 d3 d4]
// -------------------------------------------------------------
std::array<Bit, 7> Hamming74::encodeBlock(const std::array<Bit, 4>& data) {
    const Bit d1 = data[0], d2 = data[1], d3 = data[2], d4 = data[3];

    // Tính các bit parity theo công thức even parity
    const Bit p1 = d1 ^ d2 ^ d4;  // kiểm tra pos 1,3,5,7
    const Bit p2 = d1 ^ d3 ^ d4;  // kiểm tra pos 2,3,6,7
    const Bit p3 = d2 ^ d3 ^ d4;  // kiểm tra pos 4,5,6,7

    // Sắp xếp codeword theo thứ tự vị trí 1→7
    return {p1, p2, d1, p3, d2, d3, d4};
}

// -------------------------------------------------------------
// Mã hóa toàn bộ chuỗi bit (dataBits.size() phải là bội số 4)
// -------------------------------------------------------------
BitVector Hamming74::encode(const BitVector& dataBits) {
    if (dataBits.size() % 4 != 0) {
        throw std::invalid_argument("dataBits size must be multiple of 4");
    }

    BitVector coded;
    coded.reserve(dataBits.size() / 4 * 7);

    for (size_t i = 0; i < dataBits.size(); i += 4) {
        std::array<Bit, 4> block = {dataBits[i], dataBits[i+1],
                                    dataBits[i+2], dataBits[i+3]};
        auto cw = encodeBlock(block);
        for (Bit b : cw) coded.push_back(b);
    }
    return coded;
}

// -------------------------------------------------------------
// Tính syndrome từ codeword 7-bit
// Dựa trên Parity Check Matrix H của Hamming (7,4)
//
// H transpose (vị trí 1→7 tương ứng với cột):
//   pos1: [1,0,0] → s1 bị ảnh hưởng
//   pos2: [0,1,0] → s2 bị ảnh hưởng
//   pos3: [1,1,0] → s1, s2
//   pos4: [0,0,1] → s3
//   pos5: [1,0,1] → s1, s3
//   pos6: [0,1,1] → s2, s3
//   pos7: [1,1,1] → s1, s2, s3
//
// syndrome = s1*1 + s2*2 + s3*4
// -------------------------------------------------------------
int Hamming74::calcSyndrome(const std::array<Bit, 7>& cw) {
    // cw[0]=r1, cw[1]=r2, ..., cw[6]=r7
    const int s1 = cw[0] ^ cw[2] ^ cw[4] ^ cw[6]; // pos 1,3,5,7
    const int s2 = cw[1] ^ cw[2] ^ cw[5] ^ cw[6]; // pos 2,3,6,7
    const int s3 = cw[3] ^ cw[4] ^ cw[5] ^ cw[6]; // pos 4,5,6,7

    return s1 * 1 + s2 * 2 + s3 * 4;  // Giá trị 0–7
}

// -------------------------------------------------------------
// Giải mã 1 codeword 7-bit
// -------------------------------------------------------------
HammingDecodeResult Hamming74::decodeBlock(const std::array<Bit, 7>& received) {
    HammingDecodeResult res;
    res.receivedCW   = received;
    res.correctedCW  = received;  // Bắt đầu từ received, sẽ sửa nếu có lỗi

    // Bước 1: Tính syndrome
    res.syndrome = calcSyndrome(received);

    // Bước 2: Xác định và sửa lỗi
    res.errorPosition = res.syndrome;  // syndrome = 0 → không lỗi
                                       // syndrome = 1–7 → vị trí lỗi

    if (res.syndrome != 0 && res.syndrome <= 7) {
        // Flip bit tại vị trí lỗi (1-indexed → 0-indexed)
        res.correctedCW[res.syndrome - 1] ^= 1;
        res.corrected = true;
    }

    // Bước 3: Trích xuất 4 bit thông tin từ codeword đã sửa
    // Codeword: [p1 p2 d1 p3 d2 d3 d4] → index [0 1 2 3 4 5 6]
    res.decodedBits = {
        res.correctedCW[2],  // d1 tại vị trí 3
        res.correctedCW[4],  // d2 tại vị trí 5
        res.correctedCW[5],  // d3 tại vị trí 6
        res.correctedCW[6]   // d4 tại vị trí 7
    };

    return res;
}

// -------------------------------------------------------------
// Giải mã toàn bộ chuỗi bit (receivedBits.size() phải là bội số 7)
// -------------------------------------------------------------
BitVector Hamming74::decode(const BitVector& receivedBits) {
    if (receivedBits.size() % 7 != 0) {
        throw std::invalid_argument("receivedBits size must be multiple of 7");
    }

    BitVector info;
    info.reserve(receivedBits.size() / 7 * 4);

    for (size_t i = 0; i < receivedBits.size(); i += 7) {
        std::array<Bit, 7> cw = {receivedBits[i],   receivedBits[i+1],
                                  receivedBits[i+2], receivedBits[i+3],
                                  receivedBits[i+4], receivedBits[i+5],
                                  receivedBits[i+6]};
        auto res = decodeBlock(cw);
        for (Bit b : res.decodedBits) info.push_back(b);
    }
    return info;
}
