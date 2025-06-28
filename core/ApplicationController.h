#pragma once

#include <memory>
#include <string>
#include <QString>
#include <QWidget>
#include <QMessageBox>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include "FileProcessor.h"
#include "VisualizationManager.h"
#include "ExportManager.h"
#include "../UI/DensitySlider.h"

class MainWindowUI;

class ApplicationController {
public:
    ApplicationController();
    ~ApplicationController() = default;

    // ファイル操作
    bool openVtkFile(const std::string& vtkFile, MainWindowUI* ui);
    bool openStlFile(const std::string& stlFile, MainWindowUI* ui);
    
    // メイン処理
    bool processFiles(MainWindowUI* ui, QWidget* parent);
    
    // エクスポート
    bool export3mfFile(QWidget* parent);
    
    // 可視化
    void loadAndDisplayTempStlFiles(MainWindowUI* ui, QWidget* parent);
    
    // 状態管理
    void setVtkFile(const std::string& vtkFile) { this->vtkFile = vtkFile; }
    void setStlFile(const std::string& stlFile) { this->stlFile = stlFile; }
    std::string getVtkFile() const { return vtkFile; }
    std::string getStlFile() const { return stlFile; }
    QString getCurrentStlFilename() const { return currentStlFilename; }
    void setCurrentStlFilename(const QString& filename) { currentStlFilename = filename; }
    
    // ゲッター
    FileProcessor* getFileProcessor() { return fileProcessor.get(); }
    VisualizationManager* getVisualizationManager() { return visualizationManager.get(); }
    ExportManager* getExportManager() { return exportManager.get(); }

private:
    std::string vtkFile;
    std::string stlFile;
    QString currentStlFilename;
    
    std::unique_ptr<FileProcessor> fileProcessor;
    std::unique_ptr<VisualizationManager> visualizationManager;
    std::unique_ptr<ExportManager> exportManager;
    
    // ヘルパーメソッド
    bool validateFiles(QWidget* parent);
    std::vector<double> getStressThresholds(MainWindowUI* ui);
    std::vector<StressDensityMapping> getStressDensityMappings(MainWindowUI* ui);
    QString getCurrentMode(MainWindowUI* ui);
}; 