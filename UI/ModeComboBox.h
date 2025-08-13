#pragma once
#include <QComboBox>
#include <QLineEdit>
#include <QColor>
#include "../core/Mode.h"

class ModeComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit ModeComboBox(QWidget* parent = nullptr);
    SliceMode currentMode() const;
protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
private:
    bool m_isHovered = false;
    bool m_isPressed = false;
    QColor m_currentColor;
}; 