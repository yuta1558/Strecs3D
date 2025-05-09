#include "PreviewTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

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
    QPushButton* openStlButton = new QPushButton("export 3mf", this);
    
    modeComboBox = new QComboBox(this);
    modeComboBox->addItem("cura");
    modeComboBox->addItem("bambu");
    
    leftpaneLayout->addWidget(modeComboBox);
    leftpaneLayout->addWidget(openStlButton);
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
} 