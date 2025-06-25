#include "SettingsTab.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QComboBox>
#include "DensitySlider.h"

SettingsTab::SettingsTab(MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , mainWindow(mainWindow)
{
    setupUI();
    setupConnections();
}

void SettingsTab::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QVBoxLayout* leftpaneLayout = new QVBoxLayout();
    QPushButton* processButton = new QPushButton("Process", this);
    
    modeComboBox = new QComboBox(this);
    modeComboBox->addItem("cura");
    modeComboBox->addItem("bambu");
    
    // CustomRangeSliderを追加
    CustomRangeSlider* rangeSlider = new CustomRangeSlider(this);
    leftpaneLayout->addWidget(rangeSlider);
    
    // Create message console
    messageConsole = new MessageConsole(this);
    messageConsole->setMinimumHeight(200);
    
    leftpaneLayout->addWidget(modeComboBox);
    leftpaneLayout->addWidget(processButton);
    leftpaneLayout->addWidget(messageConsole);

    // Set size policy for left pane
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

    renderer->SetBackground(0.1, 0.1, 0.1);  // Dark gray color
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