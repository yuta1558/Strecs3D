#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include "VtkProcessor.h"
#include "lib3mfProcessor.h"
#include "UI/mainwindowui.h"
#include "UI/MessageConsole.h"
#include <QString>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    bool initializeVtkProcessor();
    std::vector<vtkSmartPointer<vtkPolyData>> processMeshDivision();
    QString getCurrentMode() const;
    QString getCurrentStlFilename() const { return currentStlFilename; }
    void logMessage(const QString& message);

public slots:
    void openVTKFile();
    void openSTLFile();
    void processFiles();
    void loadAndDisplayTempStlFiles();

private slots:
    bool process3mfFile();
    void syncMessageConsoles(const QString& message);

private:
    class CameraCallback : public vtkCommand {
    public:
        static CameraCallback* New() { return new CameraCallback; }
        virtual void Execute(vtkObject* caller, unsigned long, void*);
        MainWindow* window;
        bool isImport;
    };

    void syncCameras(vtkRenderer* source, vtkRenderer* dest);
    void setupCameraCallbacks();
    void setupMessageConsoles();

    std::unique_ptr<VtkProcessor> vtkProcessor;
    std::unique_ptr<MainWindowUI> ui;
    vtkSmartPointer<CameraCallback> importCameraCallback;
    vtkSmartPointer<CameraCallback> settingsCameraCallback;

    QVTKOpenGLNativeWidget* importVtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> importRenderWindow;
    vtkSmartPointer<vtkRenderer> importRenderer;

    QVTKOpenGLNativeWidget* settingsVtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> settingsRenderWindow;
    vtkSmartPointer<vtkRenderer> settingsRenderer;

    std::string vtkFile;
    std::string stlFile;
    QComboBox* modeComboBox;

    QString currentStlFilename;

    bool loadInputFiles(class Lib3mfProcessor& processor);
    bool processByMode(class Lib3mfProcessor& processor, const QString& mode);
    bool processCuraMode(class Lib3mfProcessor& processor);
    bool processBambuMode(class Lib3mfProcessor& processor);
    bool processBambuZipFiles();
    void handle3mfError(const std::exception& e);
};

#endif // MAINWINDOW_H
