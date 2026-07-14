# HammingAwgnQt — Mô phỏng Hamming (7,4) trên kênh AWGN

## Giới thiệu
Chương trình mô phỏng hệ thống truyền thông số: **Information Bits → Hamming (7,4) → BPSK → AWGN → BPSK Hard Decision → Hamming Decode → BER Evaluation**

## Cấu trúc thư mục
```
src/
  core/
    Types.h            — Kiểu dữ liệu chung
    Hamming74.h/.cpp   — Encoder + Decoder + Syndrome (không hard-code)
    Bpsk.h/.cpp        — Điều chế/giải điều chế BPSK
    AwgnChannel.h/.cpp — Kênh nhiễu Gaussian (std::normal_distribution)
    BerEvaluator.h/.cpp— Đếm lỗi, tính BER
    SimConfig.h        — Cấu hình mô phỏng
    SimResult.h        — Kết quả mô phỏng
    Simulator.h/.cpp   — Pipeline orchestrator
  gui/
    PlotWidget.h/.cpp  — Đồ thị BER vẽ bằng QPainter
    SimWorker.h/.cpp   — QThread wrapper
    MainWindow.h/.cpp  — Cửa sổ chính (3 tab)
  main.cpp
CMakeLists.txt
```

## Hướng dẫn Build (Qt Creator)
1. Mở Qt Creator → **File → Open File or Project**
2. Chọn file `CMakeLists.txt`
3. Chọn Kit (Desktop Qt 6.x MinGW hoặc MSVC 64-bit)
4. Nhấn **Configure Project**
5. Nhấn **Ctrl+R** để Build & Run

## Hướng dẫn sử dụng

### Tab 1 — Simulation
- Nhập **Info Bits**, **Eb/N0 range**, **Seed** → nhấn **Run Simulation**
- Bảng BER và đồ thị sẽ tự động hiển thị
- Ô xanh lá = Hamming tốt hơn uncoded, ô đỏ = ngược lại

### Tab 2 — Error Analysis
- **Single Bit Flip**: bật checkbox, chọn vị trí bit (1–7), chạy lại simulation
- **Double Bit Flip**: bật checkbox, chọn 2 vị trí khác nhau, chạy lại simulation
- Flip được áp dụng trên **codeword đầu tiên** (kênh lý tưởng, không AWGN) để minh họa syndrome rõ ràng

### Tab 3 — Log
- Toàn bộ log text của quá trình mô phỏng

## Lý thuyết
- **Hamming (7,4) Systematic**: `[p1 p2 d1 p3 d2 d3 d4]`
- **Syndrome**: `s1 + 2*s2 + 4*s3` → chỉ ra vị trí lỗi (không hard-code)
- **AWGN sigma**: `σ = √(1/(2·R·EbN0))` với R=4/7 cho Hamming, R=1 cho uncoded
- **BER**: tính trên Information Bits sau giải mã, không tính trên codeword
