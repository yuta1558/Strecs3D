#include "DensitySlider.h"
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>

// グラデーションストップの定義
static const struct GradStop {
    double pos;
    QColor color;
} gradStops[] = {
    {0.0, QColor("#FF4D00")}, // 赤
    {0.5, QColor("#ffffff")}, // 白
    {1.0, QColor("#004ca1")}  // 青
};

// 指定した位置（0.0〜1.0）でグラデーション色を線形補間で取得
QColor getGradientColor(double t) {
    if (t <= gradStops[0].pos) return gradStops[0].color;
    if (t >= gradStops[2].pos) return gradStops[2].color;
    for (int i = 0; i < 2; ++i) {
        if (t >= gradStops[i].pos && t <= gradStops[i+1].pos) {
            double localT = (t - gradStops[i].pos) / (gradStops[i+1].pos - gradStops[i].pos);
            QColor c1 = gradStops[i].color;
            QColor c2 = gradStops[i+1].color;
            int r = c1.red()   + (c2.red()   - c1.red())   * localT;
            int g = c1.green() + (c2.green() - c1.green()) * localT;
            int b = c1.blue()  + (c2.blue()  - c1.blue())  * localT;
            return QColor(r, g, b);
        }
    }
    return QColor(); // fallback
}

CustomRangeSlider::CustomRangeSlider(QWidget* parent)
    : QWidget(parent)
{
    // 初期ハンドル位置（均等分割）
    m_handles = {50, 100, 150};
    setMinimumWidth(60);
    setMinimumHeight(220);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}

QSize CustomRangeSlider::minimumSizeHint() const {
    return QSize(60, 220);
}

QSize CustomRangeSlider::sizeHint() const {
    return QSize(60, 300);
}

std::vector<int> CustomRangeSlider::handlePositions() const {
    return m_handles;
}

void CustomRangeSlider::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int w = width();
    int h = height();
    int x = w / 2;
    int left = x - 15;
    int right = x + 15;
    int top = m_margin;
    int bottom = h - m_margin;

    // グラデーション長方形（スライダの左側に表示）
    int gradWidth = 30;
    int gradLeft = left - gradWidth - 30; // スライダから10px離す
    int gradRight = gradLeft + gradWidth;
    QLinearGradient gradient(gradLeft, top, gradLeft, bottom);
    gradient.setColorAt(0.0, QColor("#FF4D00")); // 赤
    gradient.setColorAt(0.5, QColor("#ffffff")); // 白
    gradient.setColorAt(1.0, QColor("#004ca1")); // 青
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawRect(gradLeft, top, gradWidth, bottom - top);

    // 長方形
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(60, 60, 60));
    painter.drawRect(left, top, right - left, bottom - top);

    // 領域の色
    std::vector<int> positions = {top};
    for (int y : m_handles) positions.push_back(y);
    positions.push_back(bottom);
    for (int i = 0; i < 4; ++i) {
        // 領域の中心Y座標
        int yCenter = (positions[i] + positions[i+1]) / 2;
        // グラデーション範囲で正規化
        double t = (double)(yCenter - top) / (bottom - top);
        QColor regionColor = getGradientColor(t);
        regionColor.setAlpha(190); // 完全不透明
        painter.setBrush(regionColor);
        painter.drawRect(left, positions[i], right - left, positions[i+1] - positions[i]);
    }

    // ハンドル
    painter.setBrush(QColor(220,220,220));
    for (int y : m_handles) {
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(gradLeft, y, right, y);
        painter.setPen(Qt::NoPen);
    }
}

int CustomRangeSlider::handleAtPosition(const QPoint& pos) const {
    int w = width();
    int x = w / 2;
    int left = x - 15;
    int gradWidth = 30;
    int gradLeft = left - gradWidth - 30;
    int right = x + 15;
    int tolerance = 6; // ピクセル単位の許容範囲
    for (int i = 0; i < (int)m_handles.size(); ++i) {
        int y = m_handles[i];
        // 線の範囲内かどうか
        if (pos.x() >= gradLeft && pos.x() <= right && std::abs(pos.y() - y) <= tolerance)
            return i;
    }
    return -1;
}

void CustomRangeSlider::mousePressEvent(QMouseEvent* event) {
    m_draggedHandle = handleAtPosition(event->pos());
}

void CustomRangeSlider::mouseMoveEvent(QMouseEvent* event) {
    if (m_draggedHandle >= 0) {
        int y = std::clamp(event->pos().y(), m_margin, height() - m_margin);
        // ハンドル間の最小距離を保つ
        if (m_draggedHandle > 0)
            y = std::max(y, m_handles[m_draggedHandle-1] + m_minDistance);
        if (m_draggedHandle < (int)m_handles.size()-1)
            y = std::min(y, m_handles[m_draggedHandle+1] - m_minDistance);
        m_handles[m_draggedHandle] = y;
        clampHandles();
        update();
        emit handlePositionsChanged(m_handles);
    }
}

void CustomRangeSlider::mouseReleaseEvent(QMouseEvent*) {
    m_draggedHandle = -1;
}

void CustomRangeSlider::clampHandles() {
    int top = m_margin;
    int bottom = height() - m_margin;
    for (int i = 0; i < (int)m_handles.size(); ++i) {
        if (i == 0)
            m_handles[i] = std::clamp(m_handles[i], top, m_handles[i+1] - m_minDistance);
        else if (i == (int)m_handles.size()-1)
            m_handles[i] = std::clamp(m_handles[i], m_handles[i-1] + m_minDistance, bottom);
        else
            m_handles[i] = std::clamp(m_handles[i], m_handles[i-1] + m_minDistance, m_handles[i+1] - m_minDistance);
    }
} 