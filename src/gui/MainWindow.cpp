// MainWindow.cpp — Cài đặt giao diện chính

#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>
#include <QScrollArea>
#include <QFont>
#include <cmath>

// ─────────────────────────────────────────────────────────────
// Constructor & Destructor
// ─────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    buildUi();
    setWindowTitle("Hamming (7,4) AWGN Simulator");
    resize(1100, 720);
}

MainWindow::~MainWindow() {
    if (m_worker && m_worker->isRunning()) {
        m_worker->terminate();
        m_worker->wait();
    }
}

// ─────────────────────────────────────────────────────────────
// Xây dựng UI
// ─────────────────────────────────────────────────────────────
void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QVBoxLayout(central);

    auto* tabs = new QTabWidget(central);
    mainLayout->addWidget(tabs);

    // Tab 1 — Simulation
    auto* simTab = new QWidget();
    tabs->addTab(simTab, "📊 Simulation");
    buildSimTab(simTab);

    // Tab 2 — Error Analysis
    auto* errTab = new QWidget();
    tabs->addTab(errTab, "🔍 Error Analysis");
    buildErrorTab(errTab);

    // Tab 3 — Log
    auto* logTab = new QWidget();
    tabs->addTab(logTab, "📝 Log");
    buildLogTab(logTab);
}

// ─────────────────────────────────────────────────────────────
// Tab 1 — Simulation
// ─────────────────────────────────────────────────────────────
void MainWindow::buildSimTab(QWidget* tab) {
    auto* layout = new QHBoxLayout(tab);

    // === Panel trái: Tham số đầu vào ===
    auto* leftPanel = new QWidget();
    leftPanel->setFixedWidth(240);
    auto* leftLayout = new QVBoxLayout(leftPanel);

    auto* grpParam = new QGroupBox("Simulation Parameters");
    auto* formParam = new QFormLayout(grpParam);

    m_numBits = new QSpinBox();
    m_numBits->setRange(1000, 10000000);
    m_numBits->setValue(100000);
    m_numBits->setSingleStep(10000);
    m_numBits->setToolTip("Số Information Bits (sẽ làm tròn lên bội số 4)");

    m_ebN0Min = new QDoubleSpinBox();
    m_ebN0Min->setRange(-5.0, 20.0);
    m_ebN0Min->setValue(0.0);
    m_ebN0Min->setSingleStep(0.5);

    m_ebN0Max = new QDoubleSpinBox();
    m_ebN0Max->setRange(-5.0, 20.0);
    m_ebN0Max->setValue(10.0);
    m_ebN0Max->setSingleStep(0.5);

    m_ebN0Step = new QDoubleSpinBox();
    m_ebN0Step->setRange(0.1, 5.0);
    m_ebN0Step->setValue(1.0);
    m_ebN0Step->setSingleStep(0.5);

    m_seed = new QSpinBox();
    m_seed->setRange(0, 999999);
    m_seed->setValue(42);

    formParam->addRow("Info Bits:", m_numBits);
    formParam->addRow("Eb/N0 Min (dB):", m_ebN0Min);
    formParam->addRow("Eb/N0 Max (dB):", m_ebN0Max);
    formParam->addRow("Step (dB):", m_ebN0Step);
    formParam->addRow("Random Seed:", m_seed);

    leftLayout->addWidget(grpParam);

    // Nút Run / Cancel
    m_btnRun    = new QPushButton("▶  Run Simulation");
    m_btnCancel = new QPushButton("⏹  Cancel");
    m_btnCancel->setEnabled(false);

    m_btnRun->setStyleSheet("QPushButton { background: #2196F3; color: white; font-weight:bold; padding: 6px; border-radius: 4px; }"
                            "QPushButton:hover { background: #1976D2; }");
    m_btnCancel->setStyleSheet("QPushButton { background: #f44336; color: white; font-weight:bold; padding: 6px; border-radius: 4px; }"
                               "QPushButton:disabled { background: #ccc; color: #888; }");

    leftLayout->addWidget(m_btnRun);
    leftLayout->addWidget(m_btnCancel);

    m_progress = new QProgressBar();
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    leftLayout->addWidget(m_progress);

    leftLayout->addStretch();

    // Kết luận
    auto* grpConclusion = new QGroupBox("Auto Conclusion");
    auto* concLayout = new QVBoxLayout(grpConclusion);
    m_lblConclusion = new QLabel("Run simulation to see conclusions.");
    m_lblConclusion->setWordWrap(true);
    m_lblConclusion->setStyleSheet("QLabel { padding: 4px; }");
    concLayout->addWidget(m_lblConclusion);
    leftLayout->addWidget(grpConclusion);

    layout->addWidget(leftPanel);

    // === Panel phải: Kết quả ===
    auto* rightPanel = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightPanel);

    // Bảng BER
    auto* grpTable = new QGroupBox("BER Results Table");
    auto* tableLayout = new QVBoxLayout(grpTable);

    m_table = new QTableWidget();
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "Eb/N0 (dB)", "Sigma", "BER (Uncoded)", "BER (Hamming)", 
        "Err Uncoded", "Err Hamming", "Info Bits"
    });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setMaximumHeight(180);
    tableLayout->addWidget(m_table);
    rightLayout->addWidget(grpTable);

    // Đồ thị
    auto* grpPlot = new QGroupBox("BER vs Eb/N0 Plot");
    auto* plotLayout = new QVBoxLayout(grpPlot);
    m_plot = new PlotWidget();
    m_plot->setTitle("BER vs Eb/N0");
    m_plot->setXLabel("Eb/N0 (dB)");
    m_plot->setYLabel("BER (log scale)");
    m_plot->setLogScaleY(true);
    plotLayout->addWidget(m_plot);
    rightLayout->addWidget(grpPlot);

    layout->addWidget(rightPanel, 1);

    // ── Connections ────────────────────────────────────────────
    connect(m_btnRun,    &QPushButton::clicked, this, &MainWindow::onRunClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
}

// ─────────────────────────────────────────────────────────────
// Tab 2 — Error Analysis
// ─────────────────────────────────────────────────────────────
void MainWindow::buildErrorTab(QWidget* tab) {
    auto* layout = new QHBoxLayout(tab);

    // === Panel cấu hình Flip ===
    auto* ctrlPanel = new QWidget();
    ctrlPanel->setFixedWidth(250);
    auto* ctrlLayout = new QVBoxLayout(ctrlPanel);

    // Single Flip
    auto* grpSingle = new QGroupBox("Single Bit Flip Test");
    auto* singleLayout = new QFormLayout(grpSingle);

    m_chkSingle = new QCheckBox("Enable Single Bit Flip");
    m_spnSinglePos = new QSpinBox();
    m_spnSinglePos->setRange(1, 7);
    m_spnSinglePos->setValue(1);
    m_spnSinglePos->setEnabled(false);

    connect(m_chkSingle, &QCheckBox::toggled, m_spnSinglePos, &QSpinBox::setEnabled);

    singleLayout->addRow(m_chkSingle);
    singleLayout->addRow("Bit Position (1–7):", m_spnSinglePos);
    ctrlLayout->addWidget(grpSingle);

    // Double Flip
    auto* grpDouble = new QGroupBox("Double Bit Flip Test");
    auto* doubleLayout = new QFormLayout(grpDouble);

    m_chkDouble = new QCheckBox("Enable Double Bit Flip");
    m_spnDoublePos1 = new QSpinBox();
    m_spnDoublePos1->setRange(1, 7);
    m_spnDoublePos1->setValue(1);
    m_spnDoublePos1->setEnabled(false);

    m_spnDoublePos2 = new QSpinBox();
    m_spnDoublePos2->setRange(1, 7);
    m_spnDoublePos2->setValue(2);
    m_spnDoublePos2->setEnabled(false);

    connect(m_chkDouble, &QCheckBox::toggled, m_spnDoublePos1, &QSpinBox::setEnabled);
    connect(m_chkDouble, &QCheckBox::toggled, m_spnDoublePos2, &QSpinBox::setEnabled);

    doubleLayout->addRow(m_chkDouble);
    doubleLayout->addRow("Bit Position 1 (1–7):", m_spnDoublePos1);
    doubleLayout->addRow("Bit Position 2 (1–7):", m_spnDoublePos2);
    ctrlLayout->addWidget(grpDouble);

    auto* noteLabel = new QLabel(
        "<b>Note:</b> Flip is applied on the <b>first codeword</b> only, "
        "after BPSK demodulation and before Hamming decode. "
        "The codeword used is from an ideal channel (no AWGN) for clarity."
    );
    noteLabel->setWordWrap(true);
    noteLabel->setStyleSheet("QLabel { color: #555; font-size: 9pt; padding: 4px; }");
    ctrlLayout->addWidget(noteLabel);
    ctrlLayout->addStretch();
    layout->addWidget(ctrlPanel);

    // === Panel kết quả Flip ===
    auto* grpResult = new QGroupBox("Flip Result Detail");
    auto* resultLayout = new QVBoxLayout(grpResult);

    m_errorOutput = new QTextEdit();
    m_errorOutput->setReadOnly(true);
    m_errorOutput->setFont(QFont("Courier New", 9));
    m_errorOutput->setPlaceholderText(
        "Run simulation with Single/Double Flip enabled to see detailed results here."
    );
    resultLayout->addWidget(m_errorOutput);
    layout->addWidget(grpResult, 1);
}

// ─────────────────────────────────────────────────────────────
// Tab 3 — Log
// ─────────────────────────────────────────────────────────────
void MainWindow::buildLogTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    m_logOutput = new QTextEdit();
    m_logOutput->setReadOnly(true);
    m_logOutput->setFont(QFont("Courier New", 9));
    m_logOutput->setPlaceholderText("Simulation log will appear here...");
    layout->addWidget(m_logOutput);
}

// ─────────────────────────────────────────────────────────────
// Slot: Nhấn Run
// ─────────────────────────────────────────────────────────────
void MainWindow::onRunClicked() {
    if (m_ebN0Min->value() > m_ebN0Max->value()) {
        QMessageBox::warning(this, "Invalid Input", "Eb/N0 Min must be <= Eb/N0 Max.");
        return;
    }

    // Xây dựng SimConfig từ UI
    SimConfig cfg;
    cfg.numInfoBits     = m_numBits->value();
    cfg.ebN0Min         = m_ebN0Min->value();
    cfg.ebN0Max         = m_ebN0Max->value();
    cfg.ebN0Step        = m_ebN0Step->value();
    cfg.seed            = static_cast<unsigned>(m_seed->value());
    cfg.enableSingleFlip = m_chkSingle->isChecked();
    cfg.flipPos1        = m_spnSinglePos->value();
    cfg.enableDoubleFlip = m_chkDouble->isChecked();
    cfg.flipPos2a       = m_spnDoublePos1->value();
    cfg.flipPos2b       = m_spnDoublePos2->value();

    // Reset UI
    m_table->setRowCount(0);
    m_plot->clearCurves();
    m_logOutput->clear();
    m_errorOutput->clear();
    m_lblConclusion->setText("Running...");
    m_progress->setValue(0);
    setRunning(true);

    // Khởi động worker thread
    if (m_worker) {
        if (m_worker->isRunning()) { m_worker->terminate(); m_worker->wait(); }
        delete m_worker;
    }
    m_worker = new SimWorker(cfg, this);
    connect(m_worker, &SimWorker::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(m_worker, &SimWorker::logEmitted,      this, &MainWindow::onLogEmitted);
    connect(m_worker, &SimWorker::finished,        this, &MainWindow::onSimFinished);
    m_worker->start();
}

// ─────────────────────────────────────────────────────────────
// Slot: Hủy mô phỏng
// ─────────────────────────────────────────────────────────────
void MainWindow::onCancelClicked() {
    if (m_worker && m_worker->isRunning()) {
        m_worker->terminate();
        m_worker->wait();
        m_logOutput->append("[Cancelled by user]");
        setRunning(false);
    }
}

void MainWindow::onProgressUpdated(int pct) { m_progress->setValue(pct); }

void MainWindow::onLogEmitted(const QString& msg) {
    m_logOutput->append(msg);
}

// ─────────────────────────────────────────────────────────────
// Slot: Mô phỏng hoàn thành
// ─────────────────────────────────────────────────────────────
void MainWindow::onSimFinished(SimResult result) {
    setRunning(false);
    m_progress->setValue(100);
    fillTable(result);
    fillPlot(result);
    fillErrorTab(result);
    generateConclusion(result);
}

// ─────────────────────────────────────────────────────────────
// Điền bảng kết quả BER
// ─────────────────────────────────────────────────────────────
void MainWindow::fillTable(const SimResult& result) {
    m_table->setRowCount(static_cast<int>(result.points.size()));

    for (int row = 0; row < static_cast<int>(result.points.size()); ++row) {
        const auto& pt = result.points[row];
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(pt.ebN0Db, 'f', 1)));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(pt.sigma, 'f', 4)));
        m_table->setItem(row, 2, new QTableWidgetItem(
            pt.berUncoded > 0 ? QString::number(pt.berUncoded, 'e', 3) : "0"));
        m_table->setItem(row, 3, new QTableWidgetItem(
            pt.berHamming > 0 ? QString::number(pt.berHamming, 'e', 3) : "0"));
        m_table->setItem(row, 4, new QTableWidgetItem(QString::number(pt.errorsUncoded)));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::number(pt.errorsHamming)));
        m_table->setItem(row, 6, new QTableWidgetItem(QString::number(pt.totalInfoBits)));

        // Tô màu nếu Hamming tốt hơn
        if (pt.berHamming < pt.berUncoded) {
            m_table->item(row, 3)->setBackground(QColor(200, 255, 200));
        } else if (pt.berHamming > pt.berUncoded) {
            m_table->item(row, 3)->setBackground(QColor(255, 220, 220));
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Vẽ đồ thị
// ─────────────────────────────────────────────────────────────
void MainWindow::fillPlot(const SimResult& result) {
    m_plot->clearCurves();

    std::vector<double> xData;
    std::vector<double> yUncoded, yHamming;

    for (const auto& pt : result.points) {
        xData.push_back(pt.ebN0Db);
        yUncoded.push_back(pt.berUncoded  > 0 ? pt.berUncoded  : 1e-10);
        yHamming.push_back(pt.berHamming  > 0 ? pt.berHamming  : 1e-10);
    }

    m_plot->addCurve(xData, yUncoded, Qt::blue,  "Uncoded (BPSK)");
    m_plot->addCurve(xData, yHamming, Qt::red,   "Hamming (7,4)");
}

// ─────────────────────────────────────────────────────────────
// Điền tab Error Analysis
// ─────────────────────────────────────────────────────────────
void MainWindow::fillErrorTab(const SimResult& result) {
    QString out;

    auto printCW = [](const std::array<Bit, 7>& cw) -> QString {
        QString s = "[";
        for (int i = 0; i < 7; ++i) s += QString::number(cw[i]) + (i < 6 ? " " : "");
        return s + "]";
    };

    auto printData = [](const std::array<Bit, 4>& d) -> QString {
        QString s = "[";
        for (int i = 0; i < 4; ++i) s += QString::number(d[i]) + (i < 3 ? " " : "");
        return s + "]";
    };

    // ─── Single Flip ───────────────────────────────────────────
    if (result.singleFlip.applied) {
        out += "═══════════════════════════════════════\n";
        out += "  SINGLE BIT FLIP TEST (Position " + QString::number(result.singleFlip.flipPos1) + ")\n";
        out += "═══════════════════════════════════════\n\n";
        out += "Original Codeword:   " + printCW(result.singleFlip.originalCW) + "\n";
        out += "Codeword after flip: " + printCW(result.singleFlip.cwAfterFlip) + "\n\n";
        out += "Syndrome value   : " + QString::number(result.singleFlip.syndrome) + "\n";
        out += "Detected err pos : " + QString::number(result.singleFlip.detectedErrPos)
                                     + (result.singleFlip.detectedErrPos == 0 ? " (No error)" : "") + "\n";
        out += "Corrected CW     : " + printCW(result.singleFlip.cwCorrected) + "\n";
        out += "Decoded Data     : " + printData(result.singleFlip.decodedBits) + "\n";
        out += "Correction result: " + QString(result.singleFlip.correctionCorrect ? "✅ SUCCESS" : "❌ FAIL") + "\n";
        out += "\nConclusion: " + QString::fromStdString(result.singleFlip.conclusion) + "\n\n";
    } else {
        out += "[Single Flip Test not enabled or not run.]\n\n";
    }

    // ─── Double Flip ───────────────────────────────────────────
    if (result.doubleFlip.applied) {
        out += "═══════════════════════════════════════\n";
        out += "  DOUBLE BIT FLIP TEST (Positions "
             + QString::number(result.doubleFlip.flipPos1) + " & "
             + QString::number(result.doubleFlip.flipPos2) + ")\n";
        out += "═══════════════════════════════════════\n\n";
        out += "Original Codeword   : " + printCW(result.doubleFlip.originalCW) + "\n";
        out += "Codeword after flip : " + printCW(result.doubleFlip.cwAfterFlip) + "\n\n";
        out += "Syndrome value   : " + QString::number(result.doubleFlip.syndrome) + "\n";
        out += "Detected err pos : " + QString::number(result.doubleFlip.detectedErrPos)
                                     + (result.doubleFlip.detectedErrPos == 0 ? " (No error detected!)" : "") + "\n";
        out += "Corrected CW     : " + printCW(result.doubleFlip.cwCorrected) + "\n";
        out += "Decoded Data     : " + printData(result.doubleFlip.decodedBits) + "\n";
        out += "Correction result: " + QString(result.doubleFlip.correctionCorrect ? "✅ (Correct by chance)" : "❌ INCORRECT") + "\n";
        out += "\nConclusion: " + QString::fromStdString(result.doubleFlip.conclusion) + "\n";
    } else {
        out += "[Double Flip Test not enabled or not run.]\n";
    }

    m_errorOutput->setPlainText(out);
}

// ─────────────────────────────────────────────────────────────
// Tự động đưa ra kết luận
// ─────────────────────────────────────────────────────────────
void MainWindow::generateConclusion(const SimResult& result) {
    if (result.points.empty()) {
        m_lblConclusion->setText("No data.");
        return;
    }

    int hammingBetter = 0, hammingWorse = 0;
    for (const auto& pt : result.points) {
        if (pt.berHamming < pt.berUncoded)   ++hammingBetter;
        else if (pt.berHamming > pt.berUncoded) ++hammingWorse;
    }

    // Lấy điểm cuối cùng có xuất hiện lỗi Uncoded để so sánh
    const SimPointResult* evalPt = nullptr;
    for (auto it = result.points.rbegin(); it != result.points.rend(); ++it) {
        if (it->berUncoded > 0) {
            evalPt = &(*it);
            break;
        }
    }
    
    // Nếu điểm cuối cùng mà uncoded = 0, lấy điểm thực sự cuối cùng
    if (!evalPt) evalPt = &result.points.back();

    QString text;
    // Đánh giá dựa trên khả năng cải thiện ở Eb/N0 cao
    if (evalPt->berHamming < evalPt->berUncoded || hammingBetter > 0) {
        text = QString(
            "✅ Hamming (7,4) shows improvement at higher Eb/N0.\n"
            "At Eb/N0 = %1 dB: BER reduced from %2 → %3.\n"
            "(Note: At low Eb/N0, coded BER is often worse due to 'coding penalty').\n"
            "Code rate = 4/7 (requires 75% more bandwidth)."
        ).arg(evalPt->ebN0Db, 0, 'f', 1)
         .arg(evalPt->berUncoded, 0, 'e', 2)
         .arg(evalPt->berHamming, 0, 'e', 2);
    } else {
        text = QString(
            "⚠️ At current Eb/N0 range, Hamming (7,4) does not outperform uncoded.\n"
            "Try higher Eb/N0 (e.g. 6–12 dB) for clear improvement.\n"
            "Code rate = 4/7."
        );
    }
    m_lblConclusion->setText(text);
}

// ─────────────────────────────────────────────────────────────
// Bật/tắt UI khi đang chạy
// ─────────────────────────────────────────────────────────────
void MainWindow::setRunning(bool running) {
    m_btnRun->setEnabled(!running);
    m_btnCancel->setEnabled(running);
    m_numBits->setEnabled(!running);
    m_ebN0Min->setEnabled(!running);
    m_ebN0Max->setEnabled(!running);
    m_ebN0Step->setEnabled(!running);
    m_seed->setEnabled(!running);
}
