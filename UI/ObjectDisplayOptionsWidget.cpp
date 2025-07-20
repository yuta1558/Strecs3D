#include "ObjectDisplayOptionsWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>

ObjectDisplayOptionsWidget::ObjectDisplayOptionsWidget(const QString& fileName, QWidget* parent)
    : QWidget(parent), visibleState(true), fileName(fileName)
{
    QString displayName = fileName.isEmpty() ? "No file selected" : QFileInfo(fileName).fileName();
    fileNameLabel = new QLabel(this);
    fileNameLabel->setMaximumWidth(160); // 最大幅を設定
    fileNameLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    fileNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    QFontMetrics metrics(fileNameLabel->font());
    QString elided = metrics.elidedText(displayName, Qt::ElideMiddle, fileNameLabel->maximumWidth());
    fileNameLabel->setText(elided);
    visibilityButton = new CustomCheckBox(this);
    visibilityButton->setChecked(true);
    opacitySlider = new QSlider(Qt::Horizontal, this);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(100);
    // スライダーの幅を自動調整（必要なら最大幅を指定）
    opacitySlider->setMaximumWidth(180); // 例: 最大180pxまで
    opacitySlider->setFixedHeight(32);   // 高さはやや小さめ
    
    // 新しいレイアウト構成
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    // 左側：チェックボックス
    mainLayout->addWidget(visibilityButton, 0, Qt::AlignVCenter);

    // 右側：ファイル名とスライダーを縦に並べる
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(6);
    rightLayout->addWidget(fileNameLabel, 0, Qt::AlignLeft);
    rightLayout->addWidget(opacitySlider);
    mainLayout->addLayout(rightLayout, 1);

    setLayout(mainLayout);

    connect(visibilityButton, &CustomCheckBox::toggled, this, [this](bool checked) {
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
    QFontMetrics metrics(fileNameLabel->font());
    int maxWidth = fileNameLabel->maximumWidth();
    QString elided = metrics.elidedText(displayName, Qt::ElideMiddle, maxWidth);
    fileNameLabel->setText(elided);
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
    visibilityButton->setChecked(visibleState);
} 