#include "SettingsTab.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>

SettingsTab::SettingsTab(MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , mainWindow(mainWindow)
{
    setupUI();
    setupConnections();
}

void SettingsTab::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    modeComboBox = new QComboBox(this);
    modeComboBox->addItem("cura");
    modeComboBox->addItem("bambu");
    QPushButton* processButton = new QPushButton("Process", this);
    layout->addWidget(modeComboBox);
    layout->addWidget(processButton);

    vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(vtkWidget);

    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    vtkWidget->setStyleSheet("background-color: #1a1a1a;");
}

void SettingsTab::setupConnections()
{
    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button->text() == "Process") {
            connect(button, &QPushButton::clicked, mainWindow, &MainWindow::processFiles);
        }
    }
} 