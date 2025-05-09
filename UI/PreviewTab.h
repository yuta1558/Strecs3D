#ifndef PREVIEWTAB_H
#define PREVIEWTAB_H

#include <QWidget>
#include <QComboBox>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>

class MainWindow;

class PreviewTab : public QWidget {
    Q_OBJECT
public:
    explicit PreviewTab(MainWindow* mainWindow, QWidget* parent = nullptr);
    QComboBox* getModeComboBox() const { return modeComboBox; }
    QVTKOpenGLNativeWidget* getVtkWidget() const { return vtkWidget; }
    vtkRenderer* getRenderer() const { return renderer; }

private:
    void setupUI();

    MainWindow* mainWindow;
    QComboBox* modeComboBox;
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
};

#endif // PREVIEWTAB_H 