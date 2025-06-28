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

class MainWindowUI;
class VtkProcessor;

class VisualizationManager {
public:
    VisualizationManager();
    ~VisualizationManager();

    // ファイル表示
    void displayVtkFile(const std::string& vtkFile, MainWindowUI* ui, VtkProcessor* vtkProcessor);
    void displayStlFile(const std::string& stlFile, MainWindowUI* ui, VtkProcessor* vtkProcessor);
    void loadAndDisplayTempStlFiles(MainWindowUI* ui, VtkProcessor* vtkProcessor, QWidget* parent = nullptr);

    // スカラーバー設定
    void setupScalarBar(MainWindowUI* ui, VtkProcessor* vtkProcessor);

    // レンダラー操作
    void clearRenderer(MainWindowUI* ui);
    void resetCamera(MainWindowUI* ui);

private:
    // ヘルパーメソッド
    void calculateColor(double normalizedPos, double& r, double& g, double& b);
    std::vector<std::pair<std::filesystem::path, int>> sortStlFiles(const std::filesystem::path& tempDir);
}; 