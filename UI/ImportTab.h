#ifndef IMPORTTAB_H
#define IMPORTTAB_H

#include <QWidget>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include "MessageConsole.h"

class MainWindow;

class ImportTab : public QWidget {
    Q_OBJECT
public:
    explicit ImportTab(MainWindow* mainWindow, QWidget* parent = nullptr);
    QVTKOpenGLNativeWidget* getVtkWidget() const { return vtkWidget; }
    vtkRenderer* getRenderer() const { return renderer; }
    MessageConsole* getMessageConsole() const { return messageConsole; }

private:
    void setupUI();
    void setupConnections();

    MainWindow* mainWindow;
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    MessageConsole* messageConsole;
};

#endif // IMPORTTAB_H 