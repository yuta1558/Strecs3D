#include "PreviewTab.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QFileInfo>

PreviewTab::PreviewTab(MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , mainWindow(mainWindow)
{
    setupUI();
}

void PreviewTab::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QVBoxLayout* leftpaneLayout = new QVBoxLayout();
    QPushButton* export3mfButton = new QPushButton("export 3mf", this);
    
    leftpaneLayout->addWidget(export3mfButton);
    leftpaneLayout->addStretch();
    
    QWidget* leftPaneWidget = new QWidget(this);
    leftPaneWidget->setLayout(leftpaneLayout);
    leftPaneWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    leftPaneWidget->setMaximumWidth(300); // Set maximum width to prevent too wide left pane
    leftPaneWidget->setStyleSheet("QWidget { background-color: #2D2D2D; border-radius: 10px; }");

    vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(leftPaneWidget, 1); // Stretch factor of 1
    layout->addWidget(vtkWidget, 3);      // Stretch factor of 3

    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);
    
    // Set background color to dark gray (RGB values from 0 to 1)
    renderer->SetBackground(0.1, 0.1, 0.1);  // Dark gray color

    // Connect export button
    connect(export3mfButton, &QPushButton::clicked, this, &PreviewTab::export3MF);
}

QString PreviewTab::getDefaultExportFilename() const
{
    QString stlFilename = mainWindow->getCurrentStlFilename();
    if (stlFilename.isEmpty()) {
        return "exported_model.3mf";
    }

    QFileInfo fileInfo(stlFilename);
    return fileInfo.baseName() + ".3mf";
}

void PreviewTab::export3MF()
{
    QString sourcePath = QDir::currentPath() + "/result/result.3mf";
    if (!QFile::exists(sourcePath)) {
        QMessageBox::warning(this, "Error", "No 3MF file found in result directory.");
        return;
    }

    QString defaultName = getDefaultExportFilename();
    QString savePath = QFileDialog::getSaveFileName(this,
        "Save 3MF File",
        QDir::homePath() + "/" + defaultName,
        "3MF Files (*.3mf)");

    if (savePath.isEmpty()) {
        return;
    }

    if (!savePath.endsWith(".3mf", Qt::CaseInsensitive)) {
        savePath += ".3mf";
    }

    if (QFile::copy(sourcePath, savePath)) {
        QMessageBox::information(this, "Success", "3MF file exported successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to export 3MF file.");
    }
} 