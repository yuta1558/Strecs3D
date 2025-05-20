#ifndef PREVIEWTAB_H
#define PREVIEWTAB_H

#include <QWidget>
#include <QPushButton>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include "MessageConsole.h"

class MainWindow;

class PreviewTab : public QWidget {
    Q_OBJECT
public:
    explicit PreviewTab(MainWindow* mainWindow, QWidget* parent = nullptr);
    QVTKOpenGLNativeWidget* getVtkWidget() const { return vtkWidget; }
    vtkRenderer* getRenderer() const { return renderer; }
    MessageConsole* getMessageConsole() const { return messageConsole; }

private slots:
    void export3MF();

private:
    void setupUI();
    QString getDefaultExportFilename() const;

    MainWindow* mainWindow;
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    QPushButton* export3MFButton;
    MessageConsole* messageConsole;
};

#endif // PREVIEWTAB_H 