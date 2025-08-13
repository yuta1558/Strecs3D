#include "ProcessPipeline.h"
#include "VtkProcessor.h"
#include "lib3mfProcessor.h"
#include "../utils/fileUtility.h"
#include "../utils/tempPathUtility.h"
#include <QMessageBox>
#include <iostream>
#include <stdexcept>
#include <vtkPolyData.h>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

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
    
    // VtkProcessorにファイル名を設定し、データを読み込む
    vtkProcessor->setVtuFileName(vtkFile);
    if (!vtkProcessor->LoadAndPrepareData()) {
        if (parent) {
            QMessageBox::critical(parent, "Error", "Failed to load VTK file: " + QString::fromStdString(vtkFile));
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

bool ProcessPipeline::process3mfFile(SliceMode mode, const std::vector<StressDensityMapping>& mappings,
                                  double maxStress, QWidget* parent) {
    try {
        Lib3mfProcessor lib3mfProcessor;
        if (!loadInputFiles(lib3mfProcessor, stlFile)) {
            throw std::runtime_error("Failed to load input files");
        }
        if (!processByMode(lib3mfProcessor, mode, mappings, maxStress)) {
            throw std::runtime_error("Failed to process in selected mode");
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

bool ProcessPipeline::processByMode(Lib3mfProcessor& processor, SliceMode mode,
                                 const std::vector<StressDensityMapping>& mappings, double maxStress) {
    switch (mode) {
    case SliceMode::Cura:
        return processCuraMode(processor, mappings, maxStress);
    case SliceMode::Bambu:
        return processBambuMode(processor, maxStress, mappings);
    case SliceMode::Prusa:
        return processPrusaMode(processor, mappings, maxStress);
    }
    throw std::runtime_error("Unknown mode");
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
    const std::string outputPath = TempPathUtility::getTempFilePath("result/result.3mf").toStdString();
    if (!processor.save3mf(outputPath)) {
        throw std::runtime_error("Failed to save 3MF file");
    }
    std::cout << "Successfully saved 3MF file: " << outputPath << std::endl;
    return true;
}

bool ProcessPipeline::processBambuMode(Lib3mfProcessor& processor, double maxStress, const std::vector<StressDensityMapping>& mappings) {
    std::cout << "Processing in Bambu mode" << std::endl;
    processor.setMetaDataBambu(maxStress, mappings);
    const std::string tempFile = TempPathUtility::getTempFilePath("result.3mf").toStdString();
    if (!processor.save3mf(tempFile)) {
        throw std::runtime_error("Failed to save temporary 3MF file");
    }
    return processBambuZipFiles();
}

bool ProcessPipeline::processPrusaMode(Lib3mfProcessor& processor, const std::vector<StressDensityMapping>& mappings,
                          double maxStress) {
    std::cout << "Processing in Prusa mode" << std::endl;
    auto zones = loadPrusaProfile();
    processor.setMeshNamesPrusa(mappings, zones);
    if (!processor.assembleObjects()) {
        throw std::runtime_error("Failed to assemble objects");
    }
    const std::string outputPath = TempPathUtility::getTempFilePath("result/result.3mf").toStdString();
    if (!processor.save3mf(outputPath)) {
        throw std::runtime_error("Failed to save 3MF file");
    }
    return true;
}

std::vector<std::pair<std::string, int>> ProcessPipeline::loadPrusaProfile() {
    QString path = "profiles/prusa.json";
    QFile file(path);
    QByteArray data;
    if (file.open(QIODevice::ReadOnly)) {
        data = file.readAll();
        file.close();
    } else {
        data = QByteArray(R"({
  "zones": {
    "HIGH": { "fill_density": "50%", "fill_pattern": "gyroid" },
    "MID":  { "fill_density": "25%", "fill_pattern": "gyroid" },
    "LOW":  { "fill_density": "15%", "fill_pattern": "grid" }
  }
})");
    }
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject zonesObj = doc.object().value("zones").toObject();
    std::vector<std::pair<std::string, int>> result;
    for (auto it = zonesObj.begin(); it != zonesObj.end(); ++it) {
        QString name = it.key();
        QJsonObject zone = it.value().toObject();
        QString densityStr = zone.value("fill_density").toString();
        densityStr.remove('%');
        int density = densityStr.toInt();
        result.emplace_back(name.toStdString(), density);
    }
    return result;
}

bool ProcessPipeline::processBambuZipFiles() {
    const std::string extractDir = TempPathUtility::getTempSubDirPath("3mf").string();
    const std::string zipFile = TempPathUtility::getTempFilePath("result.3mf").toStdString();
    const std::string outputFile = TempPathUtility::getTempFilePath("result/result.3mf").toStdString();
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