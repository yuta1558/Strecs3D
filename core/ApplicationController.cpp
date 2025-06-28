#include "ApplicationController.h"
#include "../UI/mainwindowui.h"
#include "../utils/fileUtility.h"
#include "VtkProcessor.h"
#include <QDir>
#include <QFileInfo>
#include <iostream>
#include <stdexcept>

ApplicationController::ApplicationController()
    : fileProcessor(std::make_unique<FileProcessor>())
    , visualizationManager(std::make_unique<VisualizationManager>())
    , exportManager(std::make_unique<ExportManager>())
{
}

bool ApplicationController::openVtkFile(const std::string& vtkFile, MainWindowUI* ui)
{
    if (!ui) return false;
    
    setVtkFile(vtkFile);
    
    try {
        visualizationManager->displayVtkFile(vtkFile, ui, fileProcessor->getVtkProcessor().get());
        
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
    
    try {
        visualizationManager->displayStlFile(stlFile, ui, fileProcessor->getVtkProcessor().get());
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
        if (!validateFiles(parent)) {
            return false;
        }
        
        auto thresholds = getStressThresholds(ui);
        if (!fileProcessor->initializeVtkProcessor(vtkFile, stlFile, thresholds, parent)) {
            throw std::runtime_error("VTK processor initialization failed");
        }
        
        auto dividedMeshes = fileProcessor->processMeshDivision();
        if (dividedMeshes.empty()) {
            throw std::runtime_error("No meshes generated during division");
        }
        
        fileProcessor->getVtkProcessor()->saveDividedMeshes(dividedMeshes);
        
        auto mappings = getStressDensityMappings(ui);
        auto currentMode = getCurrentMode(ui);
        double maxStress = fileProcessor->getMaxStress();
        
        if (!fileProcessor->process3mfFile(currentMode.toStdString(), mappings, maxStress, parent)) {
            throw std::runtime_error("3MF file processing failed");
        }
        
        loadAndDisplayTempStlFiles(ui, parent);
        
        // 一時ファイルのクリーンアップ
        std::filesystem::path tempFiledir = ".temp";
        FileUtility::clearDirectoryContents(tempFiledir);
        
        if (parent) {
            QMessageBox::information(parent, "Success", "Files processed successfully");
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing files: " << e.what() << std::endl;
        if (parent) {
            QMessageBox::critical(parent, "Error", QString("Failed to process files: ") + e.what());
        }
        return false;
    }
}

bool ApplicationController::export3mfFile(QWidget* parent)
{
    return exportManager->export3mfFile(stlFile, parent);
}

void ApplicationController::loadAndDisplayTempStlFiles(MainWindowUI* ui, QWidget* parent)
{
    if (!ui || !fileProcessor->getVtkProcessor()) return;
    
    visualizationManager->loadAndDisplayTempStlFiles(ui, fileProcessor->getVtkProcessor().get(), parent);
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