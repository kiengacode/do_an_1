#pragma once
// PlotWidget.h — Widget tự vẽ đồ thị BER bằng QPainter

#include <QWidget>
#include <QString>
#include <QColor>
#include <vector>
#include <utility>

// =============================================================
// PlotWidget — Vẽ đồ thị BER vs Eb/N0
//
// Hỗ trợ nhiều đường (curve) trên cùng một widget.
// Trục Y: logarithmic (log10 BER)
// Trục X: tuyến tính (Eb/N0 dB)
// Không dùng Qt Charts hay thư viện ngoài.
// =============================================================
struct PlotCurve {
    std::vector<double> xData;
    std::vector<double> yData;
    QColor              color;
    QString             label;
};

class PlotWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlotWidget(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setXLabel(const QString& label);
    void setYLabel(const QString& label);
    void setLogScaleY(bool enable);  // true = log10, false = tuyến tính

    // Thêm một đường dữ liệu mới
    void addCurve(const std::vector<double>& xData,
                  const std::vector<double>& yData,
                  const QColor& color,
                  const QString& label);

    // Xóa tất cả dữ liệu
    void clearCurves();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString              m_title;
    QString              m_xLabel;
    QString              m_yLabel;
    bool                 m_logY = true;
    std::vector<PlotCurve> m_curves;

    // Vùng vẽ (tính sau khi biết kích thước widget)
    static constexpr int MARGIN_LEFT   = 65;
    static constexpr int MARGIN_RIGHT  = 20;
    static constexpr int MARGIN_TOP    = 35;
    static constexpr int MARGIN_BOTTOM = 50;

    // Helper: map giá trị thực → pixel
    double mapX(double val, double xMin, double xMax, int plotW) const;
    double mapY(double val, double yMin, double yMax, int plotH) const;
};
