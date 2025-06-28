#include "ProcessPipeline.h"
#include "VtkProcessor.h"
#include "lib3mfProcessor.h"
#include "../utils/fileUtility.h"
#include <QMessageBox>
#include <iostream>
#include <stdexcept>
#include <vtkPolyData.h>

ProcessPipeline::ProcessPipeline() {
    vtkProcessor = std::make_unique<VtkProcessor>("");
}

ProcessPipeline::~ProcessPipeline() = default;

bool ProcessPipeline::initializeVtkProcessor(const std::string& vtkFile, const std::string& stlFile, 
                                          const std::vector<double>& thresholds, QWidget* parent) {
    this->vtkFile = vtkFile;
    this->stlFile = stlFile;
    
    vtkProcessor->clearPreviousData();
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
    vtkProcessor->showInfo();
    vtkProcessor->prepareStressValues(thresholds);
    return true;
}

std::vector<vtkSmartPointer<vtkPolyData>> ProcessPipeline::processMeshDivision() {
    if (!vtkProcessor) {
        throw std::runtime_error("VtkProcessor not initialized");
    }
    auto dividedMeshes = vtkProcessor->divideMesh();
    if (dividedMeshes.empty()) {
        throw std::runtime_error("No meshes generated");
    }
    return dividedMeshes;
}

bool ProcessPipeline::process3mfFile(const std::string& mode, const std::vector<StressDensityMapping>& mappings, 
                                  double maxStress, QWidget* parent) {
    try {
        Lib3mfProcessor lib3mfProcessor;
        if (!loadInputFiles(lib3mfProcessor, stlFile)) {
            throw std::runtime_error("Failed to load input files");
        }
        QString currentMode = QString::fromStdString(mode);
        if (!processByMode(lib3mfProcessor, currentMode, mappings, maxStress)) {
            throw std::runtime_error("Failed to process in " + mode + " mode");
        }
        return true;
    }
    catch (const std::exception& e) {
        handle3mfError(e, parent);
        return false;
    }
}

bool ProcessPipeline::loadInputFiles(Lib3mfProcessor& processor, const std::string& stlFile) {
    if (!processor.getMeshes()) {
        throw std::runtime_error("Failed to load divided meshes");
    }
    if (!processor.setStl(stlFile)) {
        throw std::runtime_error("Failed to load STL file: " + stlFile);
    }
    return true;
}

bool ProcessPipeline::processByMode(Lib3mfProcessor& processor, const QString& mode, 
                                 const std::vector<StressDensityMapping>& mappings, double maxStress) {
    if (mode == "cura") {
        return processCuraMode(processor, mappings, maxStress);
    } else if (mode == "bambu") {
        return processBambuMode(processor, maxStress);
    }
    throw std::runtime_error("Unknown mode: " + mode.toStdString());
}

bool ProcessPipeline::processCuraMode(Lib3mfProcessor& processor, const std::vector<StressDensityMapping>& mappings, 
                                   double maxStress) {
    std::cout << "Processing in Cura mode" << std::endl;
    if (!processor.setMetaData(maxStress, mappings)) {
        throw std::runtime_error("Failed to set metadata");
    }
    if (!processor.assembleObjects()) {
        throw std::runtime_error("Failed to assemble objects");
    }
    const std::string outputPath = "result/result.3mf";
    if (!processor.save3mf(outputPath)) {
        throw std::runtime_error("Failed to save 3MF file");
    }
    std::cout << "Successfully saved 3MF file: " << outputPath << std::endl;
    return true;
}

bool ProcessPipeline::processBambuMode(Lib3mfProcessor& processor, double maxStress) {
    std::cout << "Processing in Bambu mode" << std::endl;
    processor.setMetaDataBambu(maxStress);
    const std::string tempFile = ".temp/result.3mf";
    if (!processor.save3mf(tempFile)) {
        throw std::runtime_error("Failed to save temporary 3MF file");
    }
    return processBambuZipFiles();
}

bool ProcessPipeline::processBambuZipFiles() {
    const std::string extractDir = ".temp/3mf";
    const std::string zipFile = ".temp/result.3mf";
    const std::string outputFile = "result/result.3mf";
    if (!FileUtility::unzipFile(zipFile, extractDir)) {
        throw std::runtime_error("Failed to extract ZIP file");
    }
    if (!FileUtility::zipDirectory(extractDir, outputFile)) {
        throw std::runtime_error("Failed to create output ZIP file");
    }
    std::cout << "Successfully processed Bambu mode files" << std::endl;
    return true;
}

void ProcessPipeline::handle3mfError(const std::exception& e, QWidget* parent) {
    std::cerr << "3MF Processing Error: " << e.what() << std::endl;
    if (parent) {
        QMessageBox::critical(parent, 
                             "3MF Processing Error", 
                             QString::fromStdString("Failed to process 3MF file:\n" + std::string(e.what())));
    }
}

double ProcessPipeline::getMaxStress() const {
    if (vtkProcessor) {
        return vtkProcessor->getMaxStress();
    }
    return 0.0;
} 