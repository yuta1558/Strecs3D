#include "ApplicationController.h"
#include "MainWindowUIAdapter.h"
#include "../../UI/mainwindowui.h"
#include "../../UI/widgets/DensitySlider.h"
#include "../../utils/fileUtility.h"
#include "../../utils/tempPathUtility.h"
#include "../processing/VtkProcessor.h"
#include <QDir>
#include <QFileInfo>
#include <iostream>
#include <stdexcept>

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
    , fileProcessor(std::make_unique<ProcessPipeline>())
    , visualizationManager(nullptr)
    , exportManager(std::make_unique<ExportManager>())
{
}

void ApplicationController::initializeVisualizationManager(MainWindowUI* mainWindowUI)
{
    visualizationManager = std::make_unique<VisualizationManager>(mainWindowUI);
}

bool ApplicationController::openVtkFile(const std::string& vtkFile, IUserInterface* ui)
{
    if (!ui) return false;
    
    setVtkFile(vtkFile);
    
    // VTK用ObjectDisplayOptionsWidgetのファイル名を更新
    emit vtkFileNameChanged(QString::fromStdString(vtkFile));
    emit vtkOpacityChanged(1.0);
    
    // --- 追加: STLを非表示にし、チェックボックスもオフ ---
    emit stlVisibilityChanged(false);
    emit stlOpacityChanged(1.0);
    if (visualizationManager) {
        visualizationManager->hideAllStlObjects();
    }
    // --- ここまで追加 ---
    try {
        if (visualizationManager) {
            visualizationManager->displayVtkFile(vtkFile, fileProcessor->getVtkProcessor().get());
        }
        
        // ストレス範囲をスライダーに設定
        if (fileProcessor->getVtkProcessor()) {
            emit stressRangeChanged(
                fileProcessor->getVtkProcessor()->getMinStress(),
                fileProcessor->getVtkProcessor()->getMaxStress()
            );
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening VTK file: " << e.what() << std::endl;
        return false;
    }
}

bool ApplicationController::openStlFile(const std::string& stlFile, IUserInterface* ui)
{
    if (!ui) return false;
    
    setStlFile(stlFile);
    setCurrentStlFilename(QString::fromStdString(stlFile));
    
    // ObjectDisplayOptionsWidgetのファイル名を更新
    emit stlFileNameChanged(QString::fromStdString(stlFile));
    
    try {
        if (visualizationManager) {
            visualizationManager->displayStlFile(stlFile, fileProcessor->getVtkProcessor().get());
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening STL file: " << e.what() << std::endl;
        return false;
    }
}

bool ApplicationController::processFiles(IUserInterface* ui, QWidget* parent)
{
    try {
        // Step 1: Validate input files
        if (!validateFiles(parent)) {
            return false;
        }
        
        // Step 2: Initialize VTK processor with stress thresholds
        if (!initializeVtkProcessor(ui, parent)) {
            return false;
        }
        
        // Step 3: Process mesh division
        if (!processMeshDivision(parent)) {
            return false;
        }
        
        // Step 4: Process 3MF file generation
        if (!process3mfGeneration(ui, parent)) {
            return false;
        }
        
        // Step 5: Load and display temporary STL files
        loadAndDisplayTempStlFiles(ui, parent);
        
        // Step 6: Cleanup temporary files
        cleanupTempFiles();
        
        // Step 7: Show success message
        showSuccessMessage(parent);
        
        return true;
    }
    catch (const std::exception& e) {
        handleProcessingError(e, parent);
        return false;
    }
}

bool ApplicationController::validateFiles(QWidget* parent)
{
    if (vtkFile.empty()) {
        emit showWarningMessage("Warning", "No VTK file selected");
        return false;
    }
    if (stlFile.empty()) {
        emit showWarningMessage("Warning", "No STL file selected");
        return false;
    }
    return true;
}

bool ApplicationController::initializeVtkProcessor(IUserInterface* ui, QWidget* parent)
{
    auto thresholds = getStressThresholds(ui);
    if (!fileProcessor->initializeVtkProcessor(vtkFile, stlFile, thresholds, parent)) {
        emit showCriticalMessage("Error", "Failed to initialize VTK processor");
        return false;
    }
    return true;
}

bool ApplicationController::processMeshDivision(QWidget* parent)
{
    auto dividedMeshes = fileProcessor->processMeshDivision();
    if (dividedMeshes.empty()) {
        emit showCriticalMessage("Error", "No meshes generated during division");
        return false;
    }
    
    fileProcessor->getVtkProcessor()->saveDividedMeshes(dividedMeshes);
    return true;
}

bool ApplicationController::process3mfGeneration(IUserInterface* ui, QWidget* parent)
{
    auto mappings = getStressDensityMappings(ui);
    auto currentMode = getCurrentMode(ui);
    double maxStress = fileProcessor->getMaxStress();
    
    if (!fileProcessor->process3mfFile(currentMode.toStdString(), mappings, maxStress, parent)) {
        emit showCriticalMessage("Error", "Failed to process 3MF file");
        return false;
    }
    return true;
}

void ApplicationController::loadAndDisplayTempStlFiles(IUserInterface* ui, QWidget* parent)
{
    if (!ui || !fileProcessor->getVtkProcessor()) return;
    
    // 分割STL Actorを削除
    if (visualizationManager) {
        visualizationManager->removeDividedStlActors();
    }
    // 分割されたメッシュウィジェットをリセット
    resetDividedMeshWidgets(ui);
    
    // --- 追加: VTKを非表示にし、チェックボックスもオフ ---
    emit vtkVisibilityChanged(false);
    emit vtkOpacityChanged(1.0);
    if (visualizationManager) {
        visualizationManager->hideVtkObject();
    }
    // 分割STLウィジェットのチェックボックスをオン
    emit dividedMeshVisibilityChanged(0, true);
    emit dividedMeshOpacityChanged(0, 1.0);
    emit dividedMeshVisibilityChanged(1, true);
    emit dividedMeshOpacityChanged(1, 1.0);
    emit dividedMeshVisibilityChanged(2, true);
    emit dividedMeshOpacityChanged(2, 1.0);
    emit dividedMeshVisibilityChanged(3, true);
    emit dividedMeshOpacityChanged(3, 1.0);
    // --- ここまで追加 ---
    if (visualizationManager) {
        visualizationManager->showTempDividedStl(fileProcessor->getVtkProcessor().get(), parent);
    }
}

void ApplicationController::cleanupTempFiles()
{
    std::filesystem::path tempFiledir = TempPathUtility::getTempSubDirPath("div");
    FileUtility::clearDirectoryContents(tempFiledir);
}

void ApplicationController::showSuccessMessage(QWidget* parent)
{
    emit showInfoMessage("Success", "Files processed successfully");
}

void ApplicationController::handleProcessingError(const std::exception& e, QWidget* parent)
{
    std::cerr << "Error processing files: " << e.what() << std::endl;
    emit showCriticalMessage("Error", QString("Failed to process files: ") + e.what());
}

bool ApplicationController::export3mfFile(QWidget* parent)
{
    return exportManager->export3mfFile(stlFile, parent);
}

std::vector<double> ApplicationController::getStressThresholds(IUserInterface* ui)
{
    if (!ui) return {};
    
    return ui->getStressThresholds();
}

std::vector<StressDensityMapping> ApplicationController::getStressDensityMappings(IUserInterface* ui)
{
    if (!ui) return {};
    
    return ui->getStressDensityMappings();
}

QString ApplicationController::getCurrentMode(IUserInterface* ui)
{
    if (!ui) return "cura";
    
    return ui->getCurrentMode();
}

void ApplicationController::resetDividedMeshWidgets(IUserInterface* ui)
{
    if (!ui) return;
    
    // 分割されたメッシュウィジェットをリセット
    emit dividedMeshFileNameChanged(0, "Divided Mesh 1");
    emit dividedMeshOpacityChanged(0, 1.0);
    emit dividedMeshFileNameChanged(1, "Divided Mesh 2");
    emit dividedMeshOpacityChanged(1, 1.0);
    emit dividedMeshFileNameChanged(2, "Divided Mesh 3");
    emit dividedMeshOpacityChanged(2, 1.0);
    emit dividedMeshFileNameChanged(3, "Divided Mesh 4");
    emit dividedMeshOpacityChanged(3, 1.0);
} 