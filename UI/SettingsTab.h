#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include <QWidget>
#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>

class MainWindow;

class SettingsTab : public QWidget {
    Q_OBJECT
public:
    explicit SettingsTab(MainWindow* mainWindow, QWidget* parent = nullptr);
    QVTKOpenGLNativeWidget* getVtkWidget() const { return vtkWidget; }
    vtkRenderer* getRenderer() const { return renderer; }
    QComboBox* getModeComboBox() const { return modeComboBox; }

private:
    void setupUI();
    void setupConnections();

    MainWindow* mainWindow;
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    QComboBox* modeComboBox;
};

#endif // SETTINGSTAB_H 