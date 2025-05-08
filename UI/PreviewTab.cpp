#include "PreviewTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

PreviewTab::PreviewTab(MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , mainWindow(mainWindow)
{
    setupUI();
}

void PreviewTab::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    modeComboBox = new QComboBox(this);
    modeComboBox->addItem("cura");
    modeComboBox->addItem("bambu");
    
    topLayout->addWidget(modeComboBox);
    topLayout->addStretch();
    
    layout->addLayout(topLayout);
    layout->addStretch();
} 