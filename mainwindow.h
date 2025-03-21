#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include "VtkProcessor.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    bool initializeVtkProcessor();
    std::vector<vtkSmartPointer<vtkPolyData>> processMeshDivision();
    void saveDividedMeshes(const std::vector<vtkSmartPointer<vtkPolyData>>& dividedMeshes);
    std::string generateMeshFileName(int index, float minValue, float maxValue)const;

private slots:
    void openVTKFile();
    void openSTLFile();
    void processFiles();
    bool process3mfFile();

private:
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    std::string vtkFile;
    std::string stlFile;
    std::unique_ptr<VtkProcessor> vtkProcessor;
};

#endif // MAINWINDOW_H
