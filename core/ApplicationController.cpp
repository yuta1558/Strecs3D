#include "ApplicationController.h"
#include "MainWindowUIAdapter.h"
#include "../UI/mainwindowui.h"
#include "../UI/DensitySlider.h"
#include "../utils/fileUtility.h"
#include "../utils/tempPathUtility.h"
#include "VtkProcessor.h"
#include <QDir>
#include <QFileInfo>
#include <iostream>
#include <stdexcept>

ApplicationController::ApplicationController(IUserInterface* ui)
    : fileProcessor(std::make_unique<ProcessPipeline>())
    , visualizationManager(std::make_unique<VisualizationManager>(dynamic_cast<MainWindowUIAdapter*>(ui)->getMainWindowUI()))
    , exportManager(std::make_unique<ExportManager>())
{
}

bool ApplicationController::openVtkFile(const std::string& vtkFile, IUserInterface* ui)
{
    if (!ui) return false;
    
    setVtkFile(vtkFile);
    
    // VTK用ObjectDisplayOptionsWidgetのファイル名を更新
    ui->setVtkFileName(QString::fromStdString(vtkFile));
    ui->setVtkOpacity(1.0);
    
    // --- 追加: STLを非表示にし、チェックボックスもオフ ---
    ui->setStlVisibilityState(false);
    ui->setStlOpacity(1.0);
    visualizationManager->hideAllStlObjects();
    // --- ここまで追加 ---
    try {
        visualizationManager->displayVtkFile(vtkFile, fileProcessor->getVtkProcessor().get());
        
        // ストレス範囲をスライダーに設定
        if (fileProcessor->getVtkProcessor()) {
            ui->setStressRange(
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
    ui->setStlFileName(QString::fromStdString(stlFile));
    
    try {
        visualizationManager->displayStlFile(stlFile, fileProcessor->getVtkProcessor().get());
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
        if (parent) {
            QMessageBox::warning(parent, "Warning", "No VTK file selected");
        }
        return false;
    }
    if (stlFile.empty()) {
        if (parent) {
            QMessageBox::warning(parent, "Warning", "No STL file selected");
        }
        return false;
    }
    return true;
}

bool ApplicationController::initializeVtkProcessor(IUserInterface* ui, QWidget* parent)
{
    auto thresholds = getStressThresholds(ui);
    if (!fileProcessor->initializeVtkProcessor(vtkFile, stlFile, thresholds, parent)) {
        if (parent) {
            QMessageBox::critical(parent, "Error", "Failed to initialize VTK processor");
        }
        return false;
    }
    return true;
}

bool ApplicationController::processMeshDivision(QWidget* parent)
{
    auto dividedMeshes = fileProcessor->processMeshDivision();
    if (dividedMeshes.empty()) {
        if (parent) {
            QMessageBox::critical(parent, "Error", "No meshes generated during division");
        }
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
        if (parent) {
            QMessageBox::critical(parent, "Error", "Failed to process 3MF file");
        }
        return false;
    }
    return true;
}

void ApplicationController::loadAndDisplayTempStlFiles(IUserInterface* ui, QWidget* parent)
{
    if (!ui || !fileProcessor->getVtkProcessor()) return;
    
    // 分割STL Actorを削除
    visualizationManager->removeDividedStlActors();
    // 分割されたメッシュウィジェットをリセット
    resetDividedMeshWidgets(ui);
    
    // --- 追加: VTKを非表示にし、チェックボックスもオフ ---
    ui->setVtkVisibilityState(false);
    ui->setVtkOpacity(1.0);
    visualizationManager->hideVtkObject();
    // 分割STLウィジェットのチェックボックスをオン
    ui->setDividedMeshVisibility(0, true);
    ui->setDividedMeshOpacity(0, 1.0);
    ui->setDividedMeshVisibility(1, true);
    ui->setDividedMeshOpacity(1, 1.0);
    ui->setDividedMeshVisibility(2, true);
    ui->setDividedMeshOpacity(2, 1.0);
    ui->setDividedMeshVisibility(3, true);
    ui->setDividedMeshOpacity(3, 1.0);
    // --- ここまで追加 ---
    visualizationManager->showTempDividedStl(fileProcessor->getVtkProcessor().get(), parent);
}

void ApplicationController::cleanupTempFiles()
{
    std::filesystem::path tempFiledir = TempPathUtility::getTempSubDirPath("div");
    FileUtility::clearDirectoryContents(tempFiledir);
}

void ApplicationController::showSuccessMessage(QWidget* parent)
{
    if (parent) {
        QMessageBox::information(parent, "Success", "Files processed successfully");
    }
}

void ApplicationController::handleProcessingError(const std::exception& e, QWidget* parent)
{
    std::cerr << "Error processing files: " << e.what() << std::endl;
    if (parent) {
        QMessageBox::critical(parent, "Error", QString("Failed to process files: ") + e.what());
    }
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
    ui->setDividedMeshFileName(0, "Divided Mesh 1");
    ui->setDividedMeshOpacity(0, 1.0);
    ui->setDividedMeshFileName(1, "Divided Mesh 2");
    ui->setDividedMeshOpacity(1, 1.0);
    ui->setDividedMeshFileName(2, "Divided Mesh 3");
    ui->setDividedMeshOpacity(2, 1.0);
    ui->setDividedMeshFileName(3, "Divided Mesh 4");
    ui->setDividedMeshOpacity(3, 1.0);
} 