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
#include "ModeComboBox.h"
#include "ObjectDisplayOptionsWidget.h"

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
    ModeComboBox* getModeComboBox() const { return modeComboBox; }
    DensitySlider* getRangeSlider() const { return rangeSlider; }
    MessageConsole* getMessageConsole() const { return messageConsole; }
    ObjectDisplayOptionsWidget* getObjectDisplayOptionsWidget() const { return objectDisplayOptionsWidget; }
    ObjectDisplayOptionsWidget* getVtkDisplayOptionsWidget() const { return vtkDisplayOptionsWidget; }
    
    // Divided mesh display widgets
    ObjectDisplayOptionsWidget* getDividedMeshWidget1() const { return dividedMeshWidget1; }
    ObjectDisplayOptionsWidget* getDividedMeshWidget2() const { return dividedMeshWidget2; }
    ObjectDisplayOptionsWidget* getDividedMeshWidget3() const { return dividedMeshWidget3; }
    ObjectDisplayOptionsWidget* getDividedMeshWidget4() const { return dividedMeshWidget4; }

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
    ModeComboBox* modeComboBox;
    DensitySlider* rangeSlider;
    MessageConsole* messageConsole;
    ObjectDisplayOptionsWidget* objectDisplayOptionsWidget;
    ObjectDisplayOptionsWidget* vtkDisplayOptionsWidget;
    
    // Divided mesh display widgets
    ObjectDisplayOptionsWidget* dividedMeshWidget1;
    ObjectDisplayOptionsWidget* dividedMeshWidget2;
    ObjectDisplayOptionsWidget* dividedMeshWidget3;
    ObjectDisplayOptionsWidget* dividedMeshWidget4;
};

#endif // MAINWINDOWUI_H 