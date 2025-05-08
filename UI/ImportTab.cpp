#include "ImportTab.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>

ImportTab::ImportTab(MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , mainWindow(mainWindow)
{
    setupUI();
    setupConnections();
}

void ImportTab::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    QVBoxLayout* leftpaneLayout = new QVBoxLayout();
    QPushButton* openStlButton = new QPushButton("Open STL File", this);
    QPushButton* openVtkButton = new QPushButton("Open VTK File", this);
    leftpaneLayout->addWidget(openStlButton);
    leftpaneLayout->addWidget(openVtkButton);

    // Set size policy for left pane
    QWidget* leftPaneWidget = new QWidget(this);
    leftPaneWidget->setLayout(leftpaneLayout);
    leftPaneWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    leftPaneWidget->setMaximumWidth(300); // Set maximum width to prevent too wide left pane

    vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(leftPaneWidget, 1); // Stretch factor of 1
    layout->addWidget(vtkWidget, 3);      // Stretch factor of 3

    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    vtkWidget->setStyleSheet("background-color: #1a1a1a;");
}

void ImportTab::setupConnections()
{
    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button->text() == "Open STL File") {
            connect(button, &QPushButton::clicked, mainWindow, &MainWindow::openSTLFile);
        } else if (button->text() == "Open VTK File") {
            connect(button, &QPushButton::clicked, mainWindow, &MainWindow::openVTKFile);
        }
    }
} 