#include "PreviewTab.h"
#include <QVBoxLayout>

PreviewTab::PreviewTab(MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , mainWindow(mainWindow)
{
    setupUI();
}

void PreviewTab::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    // Preview tab implementation will be added here when needed
} 