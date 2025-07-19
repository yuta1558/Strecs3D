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
#include <optional>

// 3Dオブジェクトの表示情報を保持する構造体
struct ObjectInfo {
    vtkSmartPointer<vtkActor> actor;
    std::string filename;
    bool visible;
    double opacity;
};

class MainWindowUI;
class VtkProcessor;
class ObjectDisplayOptionsWidget;

class VisualizationManager : public QObject {
    Q_OBJECT
public:
    VisualizationManager(MainWindowUI* ui);
    ~VisualizationManager();

    // ファイル表示
    void displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor);
    void displayStlFile(const std::string& stlFile, VtkProcessor* vtkProcessor);
    void showTempDividedStl(VtkProcessor* vtkProcessor, QWidget* parent = nullptr);

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

    void removeDividedStlActors();

private:
    MainWindowUI* ui_; //  UIポインタを保持
    std::vector<ObjectInfo> objectList; // 3Dオブジェクト情報リスト
    void calculateColor(double normalizedPos, double& r, double& g, double& b);
    std::vector<std::pair<std::filesystem::path, int>> sortStlFiles(const std::filesystem::path& tempDir);
    
    // リファクタリングで追加されたメソッド
    std::vector<std::pair<std::filesystem::path, int>> fetchDividedStlFiles();
    std::vector<ObjectDisplayOptionsWidget*> fetchMeshDisplayWidgets();
    void showDividedStlFiles(
        const std::vector<std::pair<std::filesystem::path, int>>& stlFiles,
        VtkProcessor* vtkProcessor,
        double minStress,
        double maxStress,
        const std::vector<ObjectDisplayOptionsWidget*>& widgets);
    std::optional<std::pair<double, double>> parseStressRange(const std::string& filename);
    void showStlWithStress(
        const std::filesystem::path& path,
        const std::string& filename,
        const std::pair<double, double>& stressValues,
        double minStress,
        double maxStress,
        VtkProcessor* vtkProcessor,
        const std::vector<ObjectDisplayOptionsWidget*>& widgets,
        int& widgetIndex);
    void showStlWithColor(
        const std::filesystem::path& path,
        const std::string& filename,
        int number,
        size_t totalFiles,
        VtkProcessor* vtkProcessor,
        const std::vector<ObjectDisplayOptionsWidget*>& widgets,
        int& widgetIndex);
    void addActorToRenderer(vtkSmartPointer<vtkActor> actor, const std::string& filePath);
    void updateWidgetAndConnectSignals(
        const std::vector<ObjectDisplayOptionsWidget*>& widgets,
        int& widgetIndex,
        const std::string& filename,
        const std::string& filePath);
    void connectWidgetSignals(ObjectDisplayOptionsWidget* widget, const std::string& filePath);
    void handleStlFileLoadError(const std::exception& e, QWidget* parent);
}; 