#include "mainwindow.h"
#include "core/application/MainWindowUIAdapter.h"
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Strecs3D");
    ui = std::make_unique<MainWindowUI>(this);
    uiAdapter = std::make_unique<MainWindowUIAdapter>(ui.get());
    appController = std::make_unique<ApplicationController>(uiAdapter.get());
    setCentralWidget(ui->getCentralWidget());
    resize(1600, 900);

    // ボタン接続
    connect(ui->getOpenStlButton(), &QPushButton::clicked, this, &MainWindow::openSTLFile);
    connect(ui->getOpenVtkButton(), &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(ui->getProcessButton(), &QPushButton::clicked, this, &MainWindow::processFiles);
    connect(ui->getExport3mfButton(), &QPushButton::clicked, this, &MainWindow::export3mfFile);
    
    // ObjectDisplayOptionsWidgetのシグナルをVisualizationManagerに接続
    auto objectDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (objectDisplayWidget) {
        connect(objectDisplayWidget, &ObjectDisplayOptionsWidget::visibilityToggled, 
                this, &MainWindow::onObjectVisibilityChanged);
        connect(objectDisplayWidget, &ObjectDisplayOptionsWidget::opacityChanged, 
                this, &MainWindow::onObjectOpacityChanged);
    }

    // VTK用ObjectDisplayOptionsWidgetのシグナルをVisualizationManagerに接続
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkDisplayWidget) {
        connect(vtkDisplayWidget, &ObjectDisplayOptionsWidget::visibilityToggled, 
                this, &MainWindow::onVtkObjectVisibilityChanged);
        connect(vtkDisplayWidget, &ObjectDisplayOptionsWidget::opacityChanged, 
                this, &MainWindow::onVtkObjectOpacityChanged);
    }
}

void MainWindow::logMessage(const QString& message)
{
    if (ui && ui->getMessageConsole()) {
        ui->getMessageConsole()->appendMessage(message);
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::openVTKFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open VTK File",
                                                    "",
                                                    "VTK Files (*.vtu)");
    if (fileName.isEmpty())
        return;
        
    std::string vtkFile = fileName.toStdString();
    logMessage("Open VTK File: " + fileName);
    
    if (appController->openVtkFile(vtkFile, uiAdapter.get())) {
        logMessage("VTK file loaded successfully");
    } else {
        logMessage("Failed to load VTK file");
    }
}

void MainWindow::openSTLFile()
{
    QString filename = QFileDialog::getOpenFileName(this,
        "Open STL File",
        QDir::homePath(),
        "STL Files (*.stl)");
    if (filename.isEmpty()) {
        return;
    }
    
    std::string stlFile = filename.toStdString();
    logMessage("Open STL File: " + filename);
    
    if (appController->openStlFile(stlFile, uiAdapter.get())) {
        logMessage("STL file loaded successfully");
    } else {
        logMessage("Failed to load STL file");
    }
}

void MainWindow::processFiles()
{
    logMessage("Starting file processing...");
    
    if (appController->processFiles(uiAdapter.get(), this)) {
        logMessage("File processing completed successfully");
    } else {
        logMessage("File processing failed");
    }
}

void MainWindow::export3mfFile()
{
    logMessage("Starting 3MF export...");
    
    if (appController->export3mfFile(this)) {
        logMessage("3MF export completed successfully");
    } else {
        logMessage("3MF export failed");
    }
}

QString MainWindow::getCurrentMode() const
{
    return ui->getModeComboBox()->currentText();
}

QString MainWindow::getCurrentStlFilename() const
{
    return appController->getCurrentStlFilename();
}

void MainWindow::onObjectVisibilityChanged(bool visible)
{
    auto objectDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (objectDisplayWidget) {
        QString fileName = objectDisplayWidget->getFileName();
        auto visualizationManager = appController->getVisualizationManager();
        if (visualizationManager) {
            visualizationManager->setObjectVisible(fileName.toStdString(), visible);
        }
    }
}

void MainWindow::onObjectOpacityChanged(double opacity)
{
    auto objectDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (objectDisplayWidget) {
        QString fileName = objectDisplayWidget->getFileName();
        auto visualizationManager = appController->getVisualizationManager();
        if (visualizationManager) {
            visualizationManager->setObjectOpacity(fileName.toStdString(), opacity);
        }
    }
}

void MainWindow::onVtkObjectVisibilityChanged(bool visible)
{
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkDisplayWidget) {
        QString fileName = vtkDisplayWidget->getFileName();
        auto visualizationManager = appController->getVisualizationManager();
        if (visualizationManager) {
            visualizationManager->setObjectVisible(fileName.toStdString(), visible);
        }
    }
}

void MainWindow::onVtkObjectOpacityChanged(double opacity)
{
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkDisplayWidget) {
        QString fileName = vtkDisplayWidget->getFileName();
        auto visualizationManager = appController->getVisualizationManager();
        if (visualizationManager) {
            visualizationManager->setObjectOpacity(fileName.toStdString(), opacity);
        }
    }
}
