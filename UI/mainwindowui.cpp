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

    setupImportTab();
    setupSettingsTab();
    setupPreviewTab();
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

    importPage = new QWidget();
    settingsPage = new QWidget();
    previewPage = new QWidget();

    tabWidget->addTab(importPage, QObject::tr("Import"));
    tabWidget->addTab(settingsPage, QObject::tr("Settings"));
    tabWidget->addTab(previewPage, QObject::tr("Preview"));
}

void MainWindowUI::setupImportTab()
{
    QVBoxLayout* importLayout = new QVBoxLayout(importPage);
    QPushButton* openStlButton = new QPushButton("Open STL File", importPage);
    QPushButton* openVtkButton = new QPushButton("Open VTK File", importPage);
    importLayout->addWidget(openStlButton);
    importLayout->addWidget(openVtkButton);

    importVtkWidget = new QVTKOpenGLNativeWidget(importPage);
    importLayout->addWidget(importVtkWidget);

    importRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    importVtkWidget->setRenderWindow(importRenderWindow);
    importRenderer = vtkSmartPointer<vtkRenderer>::New();
    importRenderWindow->AddRenderer(importRenderer);

    QObject::connect(openStlButton, &QPushButton::clicked, mainWindow, &MainWindow::openSTLFile);
    QObject::connect(openVtkButton, &QPushButton::clicked, mainWindow, &MainWindow::openVTKFile);
}

void MainWindowUI::setupSettingsTab()
{
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsPage);
    modeComboBox = new QComboBox(settingsPage);
    modeComboBox->addItem("cura");
    modeComboBox->addItem("bambu");
    QPushButton* processButton = new QPushButton("Process", settingsPage);
    settingsLayout->addWidget(modeComboBox);
    settingsLayout->addWidget(processButton);

    settingsVtkWidget = new QVTKOpenGLNativeWidget(settingsPage);
    settingsLayout->addWidget(settingsVtkWidget);

    settingsRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    settingsVtkWidget->setRenderWindow(settingsRenderWindow);
    settingsRenderer = vtkSmartPointer<vtkRenderer>::New();
    settingsRenderWindow->AddRenderer(settingsRenderer);

    QObject::connect(processButton, &QPushButton::clicked, mainWindow, &MainWindow::processFiles);
}

void MainWindowUI::setupPreviewTab()
{
    QVBoxLayout* previewLayout = new QVBoxLayout(previewPage);
    // Preview tab implementation will be added here when needed
}

void MainWindowUI::setupStyle()
{
    importVtkWidget->setStyleSheet("background-color: #1a1a1a;");
    settingsVtkWidget->setStyleSheet("background-color: #1a1a1a;");
    mainWindow->setStyleSheet("background-color: #1a1a1a;");
} 