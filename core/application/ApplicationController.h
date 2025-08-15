#pragma once

#include <memory>
#include <string>
#include <QString>
#include <QWidget>
#include <QMessageBox>
#include <QObject>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include "../processing/ProcessPipeline.h"
#include "../visualization/VisualizationManager.h"
#include "../export/ExportManager.h"
#include "../interfaces/IUserInterface.h"

class MainWindowUI;

class ApplicationController : public QObject {
    Q_OBJECT
public:
    ApplicationController(QObject* parent = nullptr);
    ~ApplicationController() = default;
    
    // 初期化
    void initializeVisualizationManager(MainWindowUI* mainWindowUI);

    // ファイル操作
    bool openVtkFile(const std::string& vtkFile, IUserInterface* ui);
    bool openStlFile(const std::string& stlFile, IUserInterface* ui);
    
    // メイン処理
    bool processFiles(IUserInterface* ui, QWidget* parent);
    
    // エクスポート
    bool export3mfFile(QWidget* parent);
    
    // 可視化
    void loadAndDisplayTempStlFiles(IUserInterface* ui, QWidget* parent);
    
    // 状態管理
    void setVtkFile(const std::string& vtkFile) { this->vtkFile = vtkFile; }
    void setStlFile(const std::string& stlFile) { this->stlFile = stlFile; }
    std::string getVtkFile() const { return vtkFile; }
    std::string getStlFile() const { return stlFile; }
    QString getCurrentStlFilename() const { return currentStlFilename; }
    void setCurrentStlFilename(const QString& filename) { currentStlFilename = filename; }
    
    // ゲッター
    ProcessPipeline* getFileProcessor() { return fileProcessor.get(); }
    VisualizationManager* getVisualizationManager() { return visualizationManager.get(); }
    ExportManager* getExportManager() { return exportManager.get(); }

private:
    std::string vtkFile;
    std::string stlFile;
    QString currentStlFilename;
    
    std::unique_ptr<ProcessPipeline> fileProcessor;
    std::unique_ptr<VisualizationManager> visualizationManager;
    std::unique_ptr<ExportManager> exportManager;
    
    // ヘルパーメソッド
    bool validateFiles(QWidget* parent);
    std::vector<double> getStressThresholds(IUserInterface* ui);
    std::vector<StressDensityMapping> getStressDensityMappings(IUserInterface* ui);
    QString getCurrentMode(IUserInterface* ui);
    
    // ファイル処理のヘルパーメソッド
    bool initializeVtkProcessor(IUserInterface* ui, QWidget* parent);
    bool processMeshDivision(QWidget* parent);
    bool process3mfGeneration(IUserInterface* ui, QWidget* parent);
    void cleanupTempFiles();
    void showSuccessMessage(QWidget* parent);
    void handleProcessingError(const std::exception& e, QWidget* parent);
    void resetDividedMeshWidgets(IUserInterface* ui);

signals:
    // ファイル名設定シグナル
    void vtkFileNameChanged(const QString& fileName);
    void stlFileNameChanged(const QString& fileName);
    void dividedMeshFileNameChanged(int meshIndex, const QString& fileName);
    
    // 表示状態制御シグナル
    void vtkVisibilityChanged(bool visible);
    void stlVisibilityChanged(bool visible);
    void dividedMeshVisibilityChanged(int meshIndex, bool visible);
    
    // 不透明度制御シグナル
    void vtkOpacityChanged(double opacity);
    void stlOpacityChanged(double opacity);
    void dividedMeshOpacityChanged(int meshIndex, double opacity);
    
    // ストレス範囲設定シグナル
    void stressRangeChanged(double minStress, double maxStress);
    
    // メッセージ表示シグナル
    void showWarningMessage(const QString& title, const QString& message);
    void showCriticalMessage(const QString& title, const QString& message);
    void showInfoMessage(const QString& title, const QString& message);
}; 