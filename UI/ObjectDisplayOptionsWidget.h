#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>

class ObjectDisplayOptionsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ObjectDisplayOptionsWidget(const QString& fileName, QWidget* parent = nullptr);
    void setFileName(const QString& fileName);
    void setVisibleState(bool visible);
    void setOpacity(int opacity);
    bool isVisibleState() const;
    int opacityValue() const;

signals:
    void visibilityToggled(bool visible);
    void opacityChanged(int value);

private:
    QLabel* fileNameLabel;
    QPushButton* visibilityButton;
    QSlider* opacitySlider;
    bool visibleState;
    void updateVisibilityButton();
}; 