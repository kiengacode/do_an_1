#pragma once
// MainWindow.h — Cửa sổ chính của ứng dụng

#include <QMainWindow>
#include <QTabWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include "PlotWidget.h"
#include "SimWorker.h"

// =============================================================
// MainWindow — Cửa sổ chính, quản lý UI và kết nối với SimWorker
//
// Layout: QTabWidget với 3 tab
//   Tab 1 — Simulation  : tham số, chạy mô phỏng, bảng BER, đồ thị
//   Tab 2 — Error Test  : single/double bit flip + kết quả chi tiết
//   Tab 3 — Log         : toàn bộ log text
// =============================================================
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onRunClicked();
    void onCancelClicked();
    void onProgressUpdated(int pct);
    void onLogEmitted(const QString& msg);
    void onSimFinished(SimResult result);

private:
    void buildUi();
    void buildSimTab(QWidget* tab);
    void buildErrorTab(QWidget* tab);
    void buildLogTab(QWidget* tab);

    void fillTable(const SimResult& result);
    void fillPlot(const SimResult& result);
    void fillErrorTab(const SimResult& result);
    void generateConclusion(const SimResult& result);
    void setRunning(bool running);

    // ── Tab 1: Simulation ──────────────────────────────────────
    QSpinBox*        m_numBits;
    QDoubleSpinBox*  m_ebN0Min;
    QDoubleSpinBox*  m_ebN0Max;
    QDoubleSpinBox*  m_ebN0Step;
    QSpinBox*        m_seed;
    QPushButton*     m_btnRun;
    QPushButton*     m_btnCancel;
    QProgressBar*    m_progress;
    QTableWidget*    m_table;
    PlotWidget*      m_plot;
    QLabel*          m_lblConclusion;

    // ── Tab 2: Error Test ──────────────────────────────────────
    QCheckBox*       m_chkSingle;
    QSpinBox*        m_spnSinglePos;
    QCheckBox*       m_chkDouble;
    QSpinBox*        m_spnDoublePos1;
    QSpinBox*        m_spnDoublePos2;
    QTextEdit*       m_errorOutput;

    // ── Tab 3: Log ─────────────────────────────────────────────
    QTextEdit*       m_logOutput;

    // ── Worker ─────────────────────────────────────────────────
    SimWorker*       m_worker = nullptr;
};
