#pragma once

#include <QObject>
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
struct ObjectInfo {
    vtkSmartPointer<vtkActor> actor;
    std::string filename;
    bool visible;
    double opacity;
};

class MainWindowUI;
class VtkProcessor;

class VisualizationManager : public QObject {
    Q_OBJECT
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

    // displayObjectsにObjectInfoを追加するメソッド
    void registerObject(const ObjectInfo& objInfo);

    void renderRegisteredObjects();

    // visible, opacityの値をセットするメソッド
    void setObjectVisible(const std::string& filename, bool visible);
    void setObjectOpacity(const std::string& filename, double opacity);

private:
    MainWindowUI* ui_; //  UIポインタを保持
    std::vector<ObjectInfo> objectList; // 3Dオブジェクト情報リスト
    void calculateColor(double normalizedPos, double& r, double& g, double& b);
    std::vector<std::pair<std::filesystem::path, int>> sortStlFiles(const std::filesystem::path& tempDir);
}; 