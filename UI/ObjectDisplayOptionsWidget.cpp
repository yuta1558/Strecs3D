#include "ObjectDisplayOptionsWidget.h"
#include <QHBoxLayout>
#include <QFileInfo>

ObjectDisplayOptionsWidget::ObjectDisplayOptionsWidget(const QString& fileName, QWidget* parent)
    : QWidget(parent), visibleState(true), fileName(fileName)
{
    QString displayName = fileName.isEmpty() ? "No file selected" : QFileInfo(fileName).fileName();
    fileNameLabel = new QLabel(displayName, this);
    visibilityButton = new QCheckBox("表示", this);
    visibilityButton->setChecked(true);
    opacitySlider = new QSlider(Qt::Horizontal, this);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(100);
    opacitySlider->setMinimumWidth(200); // スライダーの幅を大きく
    opacitySlider->setFixedHeight(40);   // スライダーの高さを大きく
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(fileNameLabel);
    layout->addWidget(visibilityButton);
    layout->addWidget(opacitySlider);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    connect(visibilityButton, &QCheckBox::toggled, this, [this](bool checked) {
        visibleState = checked;
        updateVisibilityButton();
        emit visibilityToggled(visibleState);
    });
    connect(opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        emit opacityChanged(value / 100.0); // 0-1の小数でemit
    });
    updateVisibilityButton();
}

void ObjectDisplayOptionsWidget::setFileName(const QString& fileName) {
    this->fileName = fileName;
    QString displayName = fileName.isEmpty() ? "No file selected" : QFileInfo(fileName).fileName();
    fileNameLabel->setText(displayName);
}

QString ObjectDisplayOptionsWidget::getFileName() const {
    return fileName;
}

void ObjectDisplayOptionsWidget::setVisibleState(bool visible) {
    visibleState = visible;
    updateVisibilityButton();
}

void ObjectDisplayOptionsWidget::setOpacity(double opacity) {
    int sliderValue = static_cast<int>(opacity * 100.0 + 0.5);
    opacitySlider->setValue(sliderValue);
}

bool ObjectDisplayOptionsWidget::isVisibleState() const {
    return visibleState;
}

double ObjectDisplayOptionsWidget::opacityValue() const {
    return opacitySlider->value() / 100.0;
}

void ObjectDisplayOptionsWidget::updateVisibilityButton() {
    visibilityButton->setText(visibleState ? "表示" : "非表示");
    visibilityButton->setChecked(visibleState);
} 