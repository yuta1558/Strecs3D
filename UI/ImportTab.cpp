#include "ImportTab.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QPushButton>

ImportTab::ImportTab(MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
    , mainWindow(mainWindow)
{
    setupUI();
    setupConnections();
}

void ImportTab::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QPushButton* openStlButton = new QPushButton("Open STL File", this);
    QPushButton* openVtkButton = new QPushButton("Open VTK File", this);
    layout->addWidget(openStlButton);
    layout->addWidget(openVtkButton);

    vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(vtkWidget);

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