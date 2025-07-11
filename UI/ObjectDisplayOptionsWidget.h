#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>

class ObjectDisplayOptionsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ObjectDisplayOptionsWidget(const QString& fileName, QWidget* parent = nullptr);
    void setFileName(const QString& fileName);
    void setVisibleState(bool visible);
    void setOpacity(double opacity);
    bool isVisibleState() const;
    double opacityValue() const;

signals:
    void visibilityToggled(bool visible);
    void opacityChanged(double value);

private:
    QLabel* fileNameLabel;
    QCheckBox* visibilityButton;
    QSlider* opacitySlider;
    bool visibleState;
    void updateVisibilityButton();
}; 