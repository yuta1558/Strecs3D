#include "ApplicationController.h"
#include "../UI/mainwindowui.h"
#include "../utils/fileUtility.h"
#include "VtkProcessor.h"
#include <QDir>
#include <QFileInfo>
#include <iostream>
#include <stdexcept>

ApplicationController::ApplicationController()
    : fileProcessor(std::make_unique<ProcessPipeline>())
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
    
    visualizationManager->loadAndDisplayTempStlFiles(ui, fileProcessor->getVtkProcessor().get(), parent);
}

void ApplicationController::cleanupTempFiles()
{
    std::filesystem::path tempFiledir = ".temp";
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