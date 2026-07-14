// PlotWidget.cpp — Vẽ đồ thị BER bằng QPainter

#include "PlotWidget.h"
#include <QPainter>
#include <QPen>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>
#include <limits>

PlotWidget::PlotWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(400, 280);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void PlotWidget::setTitle(const QString& title)    { m_title  = title;  update(); }
void PlotWidget::setXLabel(const QString& label)   { m_xLabel = label;  update(); }
void PlotWidget::setYLabel(const QString& label)   { m_yLabel = label;  update(); }
void PlotWidget::setLogScaleY(bool enable)         { m_logY   = enable; update(); }

void PlotWidget::addCurve(const std::vector<double>& xData,
                           const std::vector<double>& yData,
                           const QColor& color,
                           const QString& label) {
    m_curves.push_back({xData, yData, color, label});
    update();
}

void PlotWidget::clearCurves() {
    m_curves.clear();
    update();
}

double PlotWidget::mapX(double val, double xMin, double xMax, int plotW) const {
    if (std::abs(xMax - xMin) < 1e-12) return 0;
    return (val - xMin) / (xMax - xMin) * plotW;
}

double PlotWidget::mapY(double val, double yMin, double yMax, int plotH) const {
    if (std::abs(yMax - yMin) < 1e-12) return plotH;
    // Y trục đảo ngược (giá trị nhỏ ở dưới)
    return plotH - (val - yMin) / (yMax - yMin) * plotH;
}

void PlotWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int W = width();
    const int H = height();
    const int plotX = MARGIN_LEFT;
    const int plotY = MARGIN_TOP;
    const int plotW = W - MARGIN_LEFT - MARGIN_RIGHT;
    const int plotH = H - MARGIN_TOP - MARGIN_BOTTOM;

    if (plotW <= 0 || plotH <= 0) return;

    // ── Nền trắng ──────────────────────────────────────────────
    p.fillRect(rect(), Qt::white);

    // ── Tiêu đề ────────────────────────────────────────────────
    p.setPen(Qt::black);
    QFont titleFont = p.font();
    titleFont.setBold(true);
    titleFont.setPointSize(10);
    p.setFont(titleFont);
    p.drawText(QRect(0, 4, W, MARGIN_TOP - 4), Qt::AlignHCenter | Qt::AlignVCenter, m_title);

    // ── Tính min/max dữ liệu (chỉ từ dữ liệu thực tế) ─────────
    double xDataMin = 0, xDataMax = 10;  // Giá trị dữ liệu gốc (không padding)
    double yMin = -7, yMax = 0;

    if (!m_curves.empty()) {
        xDataMin = std::numeric_limits<double>::max();
        xDataMax = std::numeric_limits<double>::lowest();
        yMin     = std::numeric_limits<double>::max();
        yMax     = std::numeric_limits<double>::lowest();

        for (const auto& c : m_curves) {
            for (double x : c.xData) {
                xDataMin = std::min(xDataMin, x);
                xDataMax = std::max(xDataMax, x);
            }
            for (double y : c.yData) {
                double yPlot = m_logY ? (y > 0 ? std::log10(y) : -10.0) : y;
                yMin = std::min(yMin, yPlot);
                yMax = std::max(yMax, yPlot);
            }
        }
        // Chỉ thêm padding cho trục Y (trục X dùng đúng dữ liệu)
        double yPad = (yMax - yMin) * 0.12;
        if (yPad < 0.3) yPad = 0.3;
        yMin -= yPad;
        yMax += yPad;
    }

    // Trục X: Xây dựng danh sách tick là các số nguyên trong dải [xDataMin, xDataMax]
    // Thêm một chút margin trái/phải để điểm đầu/cuối không chạm mép
    const double xMarginFrac = 0.06;  // 6% tổng chiều rộng làm margin
    double xRange = (xDataMax > xDataMin) ? (xDataMax - xDataMin) : 1.0;
    double xMin = xDataMin - xRange * xMarginFrac;
    double xMax = xDataMax + xRange * xMarginFrac;

    // Tạo danh sách X tick: tất cả số nguyên trong [xDataMin, xDataMax]
    std::vector<double> xTicks;
    {
        int iMin = static_cast<int>(std::floor(xDataMin));
        int iMax = static_cast<int>(std::ceil(xDataMax));
        // Nếu quá nhiều ticks (>15), dùng bước 2
        int step = 1;
        if ((iMax - iMin) > 14) step = 2;
        for (int t = iMin; t <= iMax; t += step) xTicks.push_back(static_cast<double>(t));
    }

    // ── Font trục ──────────────────────────────────────────────
    QFont axisFont = p.font();
    axisFont.setBold(false);
    axisFont.setPointSize(8);
    p.setFont(axisFont);

    const int NUM_Y_TICKS = 6;

    // ── Grid dọc (theo X ticks thực tế) ───────────────────────
    p.setPen(QPen(QColor(220, 220, 220), 1, Qt::DashLine));
    for (double xt : xTicks) {
        int px = plotX + static_cast<int>(mapX(xt, xMin, xMax, plotW));
        if (px >= plotX && px <= plotX + plotW)
            p.drawLine(px, plotY, px, plotY + plotH);
    }

    // Grid ngang (theo Y ticks)
    for (int i = 0; i <= NUM_Y_TICKS; ++i) {
        double y = yMin + i * (yMax - yMin) / NUM_Y_TICKS;
        int py = plotY + static_cast<int>(mapY(y, yMin, yMax, plotH));
        p.drawLine(plotX, py, plotX + plotW, py);
    }

    // ── Khung trục ────────────────────────────────────────────
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(plotX, plotY, plotW, plotH);

    // ── Nhãn trục X (số nguyên sạch) ─────────────────────────
    p.setPen(Qt::black);
    for (double xt : xTicks) {
        int px = plotX + static_cast<int>(mapX(xt, xMin, xMax, plotW));
        if (px < plotX - 5 || px > plotX + plotW + 5) continue;
        // Hiển thị số nguyên nếu là nguyên, ngược lại 1 chữ số thập phân
        bool isInt = (std::abs(xt - std::round(xt)) < 0.001);
        QString lbl = isInt ? QString::number(static_cast<int>(std::round(xt)))
                            : QString::number(xt, 'f', 1);
        p.drawText(QRect(px - 25, plotY + plotH + 5, 50, 18),
                   Qt::AlignHCenter, lbl);
    }

    // ── Nhãn trục Y ──────────────────────────────────────────
    for (int i = 0; i <= NUM_Y_TICKS; ++i) {
        double y = yMin + i * (yMax - yMin) / NUM_Y_TICKS;
        int py = plotY + static_cast<int>(mapY(y, yMin, yMax, plotH));
        QString lbl = m_logY
            ? QString("10^%1").arg(static_cast<int>(std::round(y)))
            : QString::number(y, 'f', 3);
        p.drawText(QRect(0, py - 9, MARGIN_LEFT - 5, 18),
                   Qt::AlignRight | Qt::AlignVCenter, lbl);
    }

    // Nhãn tên trục
    QFont labelFont = axisFont;
    labelFont.setBold(true);
    p.setFont(labelFont);
    p.drawText(QRect(plotX, plotY + plotH + 25, plotW, 20),
               Qt::AlignHCenter, m_xLabel);

    // Xoay nhãn trục Y
    p.save();
    p.translate(12, plotY + plotH / 2);
    p.rotate(-90);
    p.drawText(QRect(-plotH / 2, -14, plotH, 16),
               Qt::AlignHCenter, m_yLabel);
    p.restore();

    // ── Vẽ các đường dữ liệu ──────────────────────────────────
    for (const auto& curve : m_curves) {
        if (curve.xData.empty()) continue;

        p.setPen(QPen(curve.color, 2));
        QPolygonF poly;

        for (size_t i = 0; i < curve.xData.size(); ++i) {
            double xv = curve.xData[i];
            double yv = curve.yData[i];
            double yPlot = m_logY ? (yv > 0 ? std::log10(yv) : yMin) : yv;

            int px = plotX + static_cast<int>(mapX(xv, xMin, xMax, plotW));
            int py = plotY + static_cast<int>(mapY(yPlot, yMin, yMax, plotH));

            poly << QPointF(px, py);

            // Vẽ marker nhỏ tại mỗi điểm
            p.drawEllipse(QPointF(px, py), 3, 3);
        }
        p.drawPolyline(poly);
    }

    // ── Legend ─────────────────────────────────────────────────
    if (!m_curves.empty()) {
        const int legendX = plotX + plotW - 170;
        int legendY = plotY + 10;

        p.setFont(axisFont);
        for (const auto& curve : m_curves) {
            p.setPen(QPen(curve.color, 2));
            p.drawLine(legendX, legendY + 6, legendX + 25, legendY + 6);
            p.setPen(Qt::black);
            p.drawText(legendX + 30, legendY + 12, curve.label);
            legendY += 20;
        }
    }
}
