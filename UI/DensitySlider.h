#pragma once
#include <QWidget>
#include <vector>

class CustomRangeSlider : public QWidget {
    Q_OBJECT
public:
    explicit CustomRangeSlider(QWidget* parent = nullptr);
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    std::vector<int> handlePositions() const;

signals:
    void handlePositionsChanged(const std::vector<int>& positions);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    std::vector<int> m_handles; // 3つのハンドル位置（Y座標）
    int m_draggedHandle = -1;
    int m_handleRadius = 8;
    int m_margin = 20;
    int m_minDistance = 20; // ハンドル間の最小距離
    int handleAtPosition(const QPoint& pos) const;
    void clampHandles();
}; 