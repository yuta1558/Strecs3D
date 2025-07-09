#include "mainwindow.h"
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , appController(std::make_unique<ApplicationController>())
{
    setWindowTitle("Strecs3D");
    ui = std::make_unique<MainWindowUI>(this);
    setCentralWidget(ui->getCentralWidget());
    resize(1600, 900);

    // ボタン接続
    connect(ui->getOpenStlButton(), &QPushButton::clicked, this, &MainWindow::openSTLFile);
    connect(ui->getOpenVtkButton(), &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(ui->getProcessButton(), &QPushButton::clicked, this, &MainWindow::processFiles);
    connect(ui->getExport3mfButton(), &QPushButton::clicked, this, &MainWindow::export3mfFile);
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
    
    if (appController->openVtkFile(vtkFile, ui.get())) {
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
    
    if (appController->openStlFile(stlFile, ui.get())) {
        logMessage("STL file loaded successfully");
    } else {
        logMessage("Failed to load STL file");
    }
}

void MainWindow::processFiles()
{
    logMessage("Starting file processing...");
    
    if (appController->processFiles(ui.get(), this)) {
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
