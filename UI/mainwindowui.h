#ifndef MAINWINDOWUI_H
#define MAINWINDOWUI_H

#include <QWidget>
#include <QTabWidget>
#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <QPushButton>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>

class MainWindow;

class MainWindowUI {
public:
    explicit MainWindowUI(MainWindow* mainWindow);
    ~MainWindowUI() = default;

    void setupUI();
    QWidget* getCentralWidget() const { return centralWidget; }
    QVTKOpenGLNativeWidget* getImportVtkWidget() const { return importVtkWidget; }
    QVTKOpenGLNativeWidget* getSettingsVtkWidget() const { return settingsVtkWidget; }
    QComboBox* getModeComboBox() const { return modeComboBox; }
    vtkRenderer* getImportRenderer() const { return importRenderer; }
    vtkRenderer* getSettingsRenderer() const { return settingsRenderer; }

private:
    void setupTabWidget();
    void setupImportTab();
    void setupSettingsTab();
    void setupPreviewTab();
    void setupStyle();

    MainWindow* mainWindow;
    QWidget* centralWidget;
    QTabWidget* tabWidget;
    QWidget* importPage;
    QWidget* settingsPage;
    QWidget* previewPage;

    QVTKOpenGLNativeWidget* importVtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> importRenderWindow;
    vtkSmartPointer<vtkRenderer> importRenderer;

    QVTKOpenGLNativeWidget* settingsVtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> settingsRenderWindow;
    vtkSmartPointer<vtkRenderer> settingsRenderer;

    QComboBox* modeComboBox;
};

#endif // MAINWINDOWUI_H 