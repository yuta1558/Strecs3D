#include "VisualizationManager.h"
#include "../UI/mainwindowui.h"
#include "../VtkProcessor.h"
#include <QMessageBox>
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

VisualizationManager::VisualizationManager() = default;

VisualizationManager::~VisualizationManager() = default;

void VisualizationManager::displayVtkFile(const std::string& vtkFile, MainWindowUI* ui, VtkProcessor* vtkProcessor) {
    if (!ui || !vtkProcessor) return;
    
    clearRenderer(ui);
    auto importActor = vtkProcessor->getVtuActor(vtkFile);
    ui->getRenderer()->AddActor(importActor);
    setupScalarBar(ui, vtkProcessor);
    resetCamera(ui);
    ui->getVtkWidget()->renderWindow()->Render();
}

void VisualizationManager::displayStlFile(const std::string& stlFile, MainWindowUI* ui, VtkProcessor* vtkProcessor) {
    if (!ui || !vtkProcessor) return;
    
    clearRenderer(ui);
    auto importActor = vtkProcessor->getStlActor(stlFile);
    ui->getRenderer()->AddActor(importActor);
    resetCamera(ui);
    ui->getVtkWidget()->renderWindow()->Render();
}

void VisualizationManager::loadAndDisplayTempStlFiles(MainWindowUI* ui, VtkProcessor* vtkProcessor, QWidget* parent) {
    try {
        std::filesystem::path tempDir = ".temp/div";
        if (!std::filesystem::exists(tempDir)) {
            throw std::runtime_error(".temp directory does not exist");
        }
        
        clearRenderer(ui);
        
        auto stlFiles = sortStlFiles(tempDir);
        if (stlFiles.empty()) {
            throw std::runtime_error("No valid STL files found");
        }
        
        int minNumber = stlFiles.front().second;
        int maxNumber = stlFiles.back().second;
        double range = maxNumber - minNumber;
        
        for (const auto& [path, number] : stlFiles) {
            double r, g, b;
            double normalizedPos = (number - minNumber) / range;
            calculateColor(normalizedPos, r, g, b);
            
            auto actor = vtkProcessor->getColoredStlActor(path.string(), r, g, b);
            if (actor) {
                ui->getRenderer()->AddActor(actor);
            }
        }
        
        resetCamera(ui);
        ui->getVtkWidget()->renderWindow()->Render();
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading STL files: " << e.what() << std::endl;
        if (parent) {
            QMessageBox::critical(parent, "Error", QString("Failed to load STL files: ") + e.what());
        }
    }
}

void VisualizationManager::setupScalarBar(MainWindowUI* ui, VtkProcessor* vtkProcessor) {
    if (!ui || !vtkProcessor) return;
    
    auto lookupTable = vtkProcessor->getCurrentLookupTable();
    if (lookupTable) {
        vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
        scalarBar->SetLookupTable(lookupTable);
        scalarBar->SetTitle("von Mises Stress");
        scalarBar->GetLabelTextProperty()->SetColor(1, 1, 1);
        scalarBar->GetTitleTextProperty()->SetColor(1, 1, 1);
        scalarBar->SetNumberOfLabels(5);
        scalarBar->SetOrientationToHorizontal();
        scalarBar->SetWidth(0.5);
        scalarBar->SetHeight(0.05);
        scalarBar->SetPosition(0.5, 0.05);
        ui->getRenderer()->AddActor2D(scalarBar);
    }
}

void VisualizationManager::clearRenderer(MainWindowUI* ui) {
    if (ui && ui->getRenderer()) {
        ui->getRenderer()->RemoveAllViewProps();
    }
}

void VisualizationManager::resetCamera(MainWindowUI* ui) {
    if (ui && ui->getRenderer()) {
        ui->getRenderer()->ResetCamera();
    }
}

void VisualizationManager::calculateColor(double normalizedPos, double& r, double& g, double& b) {
    if (normalizedPos <= 0.5) {
        double t = normalizedPos * 2.0;
        r = t;
        g = t;
        b = 1.0;
    } else {
        double t = (normalizedPos - 0.5) * 2.0;
        r = 1.0;
        g = 1.0 - t;
        b = 1.0 - t;
    }
}

std::vector<std::pair<std::filesystem::path, int>> VisualizationManager::sortStlFiles(const std::filesystem::path& tempDir) {
    std::vector<std::pair<std::filesystem::path, int>> stlFiles;
    std::regex filePattern(R"(^dividedMesh(\d+)_)");
    
    for (const auto& entry : std::filesystem::directory_iterator(tempDir)) {
        if (entry.path().extension() == ".stl") {
            std::string filename = entry.path().filename().string();
            std::smatch match;
            if (std::regex_search(filename, match, filePattern)) {
                int number = std::stoi(match[1].str());
                stlFiles.push_back({entry.path(), number});
            }
        }
    }
    
    std::sort(stlFiles.begin(), stlFiles.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    return stlFiles;
} 