#pragma once

#include <QWidget>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>
#include <vtkPointData.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkSTLReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkCamera.h>
#include <iostream>
#include <regex>
#include <algorithm>
#include <filesystem>
#include <vtkActor.h>

// 3Dオブジェクトの表示情報を保持する構造体
struct DisplayObjectInfo {
    vtkSmartPointer<vtkActor> actor;
    std::string filename;
    bool visible;
    double opacity;
};

class MainWindowUI;
class VtkProcessor;

class VisualizationManager {
public:
    VisualizationManager(MainWindowUI* ui);
    ~VisualizationManager();

    // ファイル表示
    void displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor);
    void displayStlFile(const std::string& stlFile, VtkProcessor* vtkProcessor);
    void loadAndDisplayTempStlFiles(VtkProcessor* vtkProcessor, QWidget* parent = nullptr);

    // スカラーバー設定
    void setupScalarBar(VtkProcessor* vtkProcessor);

    // レンダラー操作
    void clearRenderer();
    void resetCamera();

private:
    MainWindowUI* ui_; //  UIポインタを保持
    std::vector<DisplayObjectInfo> displayObjects; // 3Dオブジェクト情報リスト
    void calculateColor(double normalizedPos, double& r, double& g, double& b);
    std::vector<std::pair<std::filesystem::path, int>> sortStlFiles(const std::filesystem::path& tempDir);
}; 