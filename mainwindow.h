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


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    bool initializeVtkProcessor();
    std::vector<vtkSmartPointer<vtkPolyData>> processMeshDivision();
    //void saveDividedMeshes(const std::vector<vtkSmartPointer<vtkPolyData>>& dividedMeshes);
    //std::string generateMeshFileName(int index, float minValue, float maxValue)const;
    QString getCurrentMode() const;

private slots:
    void openVTKFile();
    void openSTLFile();
    void processFiles();
    bool process3mfFile();

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

    std::unique_ptr<VtkProcessor> vtkProcessor;
    vtkSmartPointer<CameraCallback> importCameraCallback;
    vtkSmartPointer<CameraCallback> settingsCameraCallback;

    QVTKOpenGLNativeWidget* importVtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> importRenderWindow;
    vtkSmartPointer<vtkRenderer> importRenderer;

    QVTKOpenGLNativeWidget* settingsVtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> settingsRenderWindow;
    vtkSmartPointer<vtkRenderer> settingsRenderer;

    // QVTKOpenGLNativeWidget* vtkWidget;
    // QVTKOpenGLNativeWidget* previewVtkWidget;
    // vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    // vtkSmartPointer<vtkRenderer> renderer;
    std::string vtkFile;
    std::string stlFile;
    QComboBox* modeComboBox;

    bool loadInputFiles(class Lib3mfProcessor& processor);
    bool processByMode(class Lib3mfProcessor& processor, const QString& mode);
    bool processCuraMode(class Lib3mfProcessor& processor);
    bool processBambuMode(class Lib3mfProcessor& processor);
    bool processBambuZipFiles();
    void handle3mfError(const std::exception& e);
};

#endif // MAINWINDOW_H
