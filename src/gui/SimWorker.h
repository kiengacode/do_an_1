#pragma once
// SimWorker.h — QThread wrapper để chạy Simulator ở background

#include <QThread>
#include "core/SimConfig.h"
#include "core/SimResult.h"

// =============================================================
// SimWorker — Chạy Simulator::run() trong background thread
// để giao diện Qt không bị treo khi mô phỏng
// =============================================================
class SimWorker : public QThread {
    Q_OBJECT

public:
    explicit SimWorker(const SimConfig& cfg, QObject* parent = nullptr);

    void run() override;  // Gọi Simulator::run()

signals:
    void progressUpdated(int percent);
    void logEmitted(const QString& msg);
    void finished(SimResult result);

private:
    SimConfig m_cfg;
};
