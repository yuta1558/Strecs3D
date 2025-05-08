#include "mainwindowui.h"
#include "../mainwindow.h"
#include <QVBoxLayout>

MainWindowUI::MainWindowUI(MainWindow* mainWindow)
    : mainWindow(mainWindow)
{
    setupUI();
}

void MainWindowUI::setupUI()
{
    centralWidget = new QWidget(mainWindow);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    setupTabWidget();
    mainLayout->addWidget(tabWidget);

    setupStyle();
}

void MainWindowUI::setupTabWidget()
{
    tabWidget = new QTabWidget(centralWidget);
    tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #444;
            border-radius: 8px;
            background: #2e2e2e;
        }
        QTabBar::tab {
            background: #3a3a3a;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            padding: 6px 12px;
            min-width: 80px;
        }
        QTabBar::tab:selected {
            background:rgb(79, 76, 119);
        }
    )");

    importTab = new ImportTab(mainWindow);
    settingsTab = new SettingsTab(mainWindow);
    previewTab = new PreviewTab(mainWindow);

    tabWidget->addTab(importTab, QObject::tr("Import"));
    tabWidget->addTab(settingsTab, QObject::tr("Settings"));
    tabWidget->addTab(previewTab, QObject::tr("Preview"));
}

void MainWindowUI::setupStyle()
{
    mainWindow->setStyleSheet("background-color: #1a1a1a;");
} 