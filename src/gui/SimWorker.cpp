// SimWorker.cpp — Cài đặt QThread wrapper cho Simulator

#include "SimWorker.h"
#include "core/Simulator.h"

SimWorker::SimWorker(const SimConfig& cfg, QObject* parent)
    : QThread(parent), m_cfg(cfg) {}

void SimWorker::run() {
    // Chạy Simulator và forward callback lên GUI qua signal
    SimResult result = Simulator::run(
        m_cfg,
        [this](int pct) { emit progressUpdated(pct); },
        [this](const QString& msg) { emit logEmitted(msg); }
    );
    emit finished(result);
}
