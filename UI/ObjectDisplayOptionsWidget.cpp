#include "ObjectDisplayOptionsWidget.h"
#include <QHBoxLayout>

ObjectDisplayOptionsWidget::ObjectDisplayOptionsWidget(const QString& fileName, QWidget* parent)
    : QWidget(parent), visibleState(true)
{
    fileNameLabel = new QLabel(fileName, this);
    visibilityButton = new QPushButton("表示", this);
    opacitySlider = new QSlider(Qt::Horizontal, this);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(100);
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(fileNameLabel);
    layout->addWidget(visibilityButton);
    layout->addWidget(opacitySlider);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    connect(visibilityButton, &QPushButton::clicked, this, [this]() {
        visibleState = !visibleState;
        updateVisibilityButton();
        emit visibilityToggled(visibleState);
    });
    connect(opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        emit opacityChanged(value);
    });
    updateVisibilityButton();
}

void ObjectDisplayOptionsWidget::setFileName(const QString& fileName) {
    fileNameLabel->setText(fileName);
}

void ObjectDisplayOptionsWidget::setVisibleState(bool visible) {
    visibleState = visible;
    updateVisibilityButton();
}

void ObjectDisplayOptionsWidget::setOpacity(int opacity) {
    opacitySlider->setValue(opacity);
}

bool ObjectDisplayOptionsWidget::isVisibleState() const {
    return visibleState;
}

int ObjectDisplayOptionsWidget::opacityValue() const {
    return opacitySlider->value();
}

void ObjectDisplayOptionsWidget::updateVisibilityButton() {
    visibilityButton->setText(visibleState ? "表示" : "非表示");
} 