#ifndef MAINWINDOWUI_H
#define MAINWINDOWUI_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "DensitySlider.h"
#include "MessageConsole.h"
#include "Button.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>

class MainWindow;

class MainWindowUI {
public:
    explicit MainWindowUI(MainWindow* mainWindow);
    ~MainWindowUI() = default;

    void setupUI();
    QWidget* getCentralWidget() const { return centralWidget; }
    QVTKOpenGLNativeWidget* getVtkWidget() const { return vtkWidget; }
    vtkSmartPointer<vtkRenderer> getRenderer() const { return renderer; }
    Button* getOpenStlButton() const { return openStlButton; }
    Button* getOpenVtkButton() const { return openVtkButton; }
    Button* getProcessButton() const { return processButton; }
    Button* getExport3mfButton() const { return export3mfButton; }
    QComboBox* getModeComboBox() const { return modeComboBox; }
    DensitySlider* getRangeSlider() const { return rangeSlider; }
    MessageConsole* getMessageConsole() const { return messageConsole; }

private:
    void setupStyle();

    MainWindow* mainWindow;
    QWidget* centralWidget;
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    Button* openStlButton;
    Button* openVtkButton;
    Button* processButton;
    Button* export3mfButton;
    QComboBox* modeComboBox;
    DensitySlider* rangeSlider;
    MessageConsole* messageConsole;
};

#endif // MAINWINDOWUI_H 