#include "ApplicationController.h"
#include "../UI/mainwindowui.h"
#include "../utils/fileUtility.h"
#include "VtkProcessor.h"
#include <QDir>
#include <QFileInfo>
#include <iostream>
#include <stdexcept>

ApplicationController::ApplicationController(MainWindowUI* ui)
    : fileProcessor(std::make_unique<ProcessPipeline>())
    , visualizationManager(std::make_unique<VisualizationManager>(ui))
    , exportManager(std::make_unique<ExportManager>())
{
}

bool ApplicationController::openVtkFile(const std::string& vtkFile, MainWindowUI* ui)
{
    if (!ui) return false;
    
    setVtkFile(vtkFile);
    
    // VTK用ObjectDisplayOptionsWidgetのファイル名を更新
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkDisplayWidget) {
        vtkDisplayWidget->setFileName(QString::fromStdString(vtkFile));
        vtkDisplayWidget->setOpacity(1.0); // 追加: 不透明
    }
    // --- 追加: STLを非表示にし、チェックボックスもオフ ---
    auto stlWidget = ui->getObjectDisplayOptionsWidget();
    if (stlWidget) {
        stlWidget->setVisibleState(false);
        stlWidget->setOpacity(1.0); // 追加: 不透明
    }
    visualizationManager->hideAllStlObjects();
    // --- ここまで追加 ---
    try {
        visualizationManager->displayVtkFile(vtkFile, fileProcessor->getVtkProcessor().get());
        
        // ストレス範囲をスライダーに設定
        auto slider = ui->getRangeSlider();
        if (slider && fileProcessor->getVtkProcessor()) {
            slider->setStressRange(
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

bool ApplicationController::openStlFile(const std::string& stlFile, MainWindowUI* ui)
{
    if (!ui) return false;
    
    setStlFile(stlFile);
    setCurrentStlFilename(QString::fromStdString(stlFile));
    
    // ObjectDisplayOptionsWidgetのファイル名を更新
    auto objectDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (objectDisplayWidget) {
        objectDisplayWidget->setFileName(QString::fromStdString(stlFile));
    }
    
    try {
        visualizationManager->displayStlFile(stlFile, fileProcessor->getVtkProcessor().get());
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening STL file: " << e.what() << std::endl;
        return false;
    }
}

bool ApplicationController::processFiles(MainWindowUI* ui, QWidget* parent)
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

bool ApplicationController::initializeVtkProcessor(MainWindowUI* ui, QWidget* parent)
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

bool ApplicationController::process3mfGeneration(MainWindowUI* ui, QWidget* parent)
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

void ApplicationController::loadAndDisplayTempStlFiles(MainWindowUI* ui, QWidget* parent)
{
    if (!ui || !fileProcessor->getVtkProcessor()) return;
    
    // 分割STL Actorを削除
    visualizationManager->removeDividedStlActors();
    // 分割されたメッシュウィジェットをリセット
    resetDividedMeshWidgets(ui);
    
    // --- 追加: VTKを非表示にし、チェックボックスもオフ ---
    auto vtkWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkWidget) {
        vtkWidget->setVisibleState(false);
        vtkWidget->setOpacity(1.0); // 追加: 不透明
    }
    visualizationManager->hideVtkObject();
    // 分割STLウィジェットのチェックボックスをオン
    auto d1 = ui->getDividedMeshWidget1();
    auto d2 = ui->getDividedMeshWidget2();
    auto d3 = ui->getDividedMeshWidget3();
    auto d4 = ui->getDividedMeshWidget4();
    if (d1) { d1->setVisibleState(true); d1->setOpacity(1.0); }
    if (d2) { d2->setVisibleState(true); d2->setOpacity(1.0); }
    if (d3) { d3->setVisibleState(true); d3->setOpacity(1.0); }
    if (d4) { d4->setVisibleState(true); d4->setOpacity(1.0); }
    // --- ここまで追加 ---
    visualizationManager->showTempDividedStl(fileProcessor->getVtkProcessor().get(), parent);
}

void ApplicationController::cleanupTempFiles()
{
    std::filesystem::path tempFiledir = ".temp/div";
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

std::vector<double> ApplicationController::getStressThresholds(MainWindowUI* ui)
{
    if (!ui) return {};
    
    auto slider = ui->getRangeSlider();
    if (slider) {
        return slider->stressThresholds();
    }
    return {};
}

std::vector<StressDensityMapping> ApplicationController::getStressDensityMappings(MainWindowUI* ui)
{
    if (!ui) return {};
    
    auto slider = ui->getRangeSlider();
    if (slider) {
        return slider->stressDensityMappings();
    }
    return {};
}

QString ApplicationController::getCurrentMode(MainWindowUI* ui)
{
    if (!ui) return "cura";
    
    auto comboBox = ui->getModeComboBox();
    if (comboBox) {
        return comboBox->currentText();
    }
    return "cura";
}

void ApplicationController::resetDividedMeshWidgets(MainWindowUI* ui)
{
    if (!ui) return;
    
    // 分割されたメッシュウィジェットをリセット
    auto widget1 = ui->getDividedMeshWidget1();
    auto widget2 = ui->getDividedMeshWidget2();
    auto widget3 = ui->getDividedMeshWidget3();
    auto widget4 = ui->getDividedMeshWidget4();
    
    if (widget1) { widget1->setFileName("Divided Mesh 1"); widget1->setOpacity(1.0); }
    if (widget2) { widget2->setFileName("Divided Mesh 2"); widget2->setOpacity(1.0); }
    if (widget3) { widget3->setFileName("Divided Mesh 3"); widget3->setOpacity(1.0); }
    if (widget4) { widget4->setFileName("Divided Mesh 4"); widget4->setOpacity(1.0); }
} 