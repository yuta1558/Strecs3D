#include "VisualizationManager.h"
#include "../UI/mainwindowui.h"
#include "VtkProcessor.h"
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

VisualizationManager::VisualizationManager(MainWindowUI* ui) : ui_(ui) {}

VisualizationManager::~VisualizationManager() = default;

void VisualizationManager::displayVtkFile(const std::string& vtkFile, VtkProcessor* vtkProcessor) {
    if (!ui_ || !vtkProcessor) return;
    
    auto importActor = vtkProcessor->getVtuActor(vtkFile);
    ui_->getRenderer()->AddActor(importActor);
    // ObjectInfoを登録
    ObjectInfo objInfo{importActor, vtkFile, true, 1.0};
    registerObject(objInfo);
    setupScalarBar(vtkProcessor);
    resetCamera();
    renderRegisteredObjects();
}

void VisualizationManager::displayStlFile(const std::string& stlFile, VtkProcessor* vtkProcessor) {
    if (!ui_ || !vtkProcessor) return;
    
    auto importActor = vtkProcessor->getStlActor(stlFile);
    ui_->getRenderer()->AddActor(importActor);
    // ObjectInfoを登録
    ObjectInfo objInfo{importActor, stlFile, true, 1.0};
    registerObject(objInfo);
    resetCamera();
    renderRegisteredObjects();
}

void VisualizationManager::loadAndDisplayTempStlFiles(VtkProcessor* vtkProcessor, QWidget* parent) {
    try {
        std::filesystem::path tempDir = ".temp/div";
        if (!std::filesystem::exists(tempDir)) {
            throw std::runtime_error(".temp directory does not exist");
        }
        
        
        auto stlFiles = sortStlFiles(tempDir);
        if (stlFiles.empty()) {
            throw std::runtime_error("No valid STL files found");
        }
        
        // ストレス範囲を取得
        double minStress = vtkProcessor->getMinStress();
        double maxStress = vtkProcessor->getMaxStress();
        
        for (const auto& [path, number] : stlFiles) {
            // ファイル名からストレス値を抽出
            std::string filename = path.filename().string();
            std::regex stressPattern(R"(^dividedMesh\d+_([0-9.]+)_([0-9.]+)\.stl$)");
            std::smatch match;
            
            if (std::regex_search(filename, match, stressPattern)) {
                double stressMin = std::stod(match[1].str());
                double stressMax = std::stod(match[2].str());
                // 領域の中心のストレス値を使用
                double stressValue = (stressMin + stressMax) / 2.0;
                
                auto actor = vtkProcessor->getColoredStlActorByStress(path.string(), stressValue, minStress, maxStress);
                if (actor) {
                    ui_->getRenderer()->AddActor(actor);
                    // ObjectInfoを登録
                    ObjectInfo objInfo{actor, path.string(), true, 1.0};
                    registerObject(objInfo);
                }
            } else {
                // ファイル名からストレス値が抽出できない場合は、従来の方法を使用
                double r, g, b;
                double normalizedPos = static_cast<double>(number) / stlFiles.size();
                calculateColor(normalizedPos, r, g, b);
                
                auto actor = vtkProcessor->getColoredStlActor(path.string(), r, g, b);
                if (actor) {
                    ui_->getRenderer()->AddActor(actor);
                    // ObjectInfoを登録
                    ObjectInfo objInfo{actor, path.string(), true, 1.0};
                    registerObject(objInfo);
                }
            }
        }
        
        resetCamera();
        renderRegisteredObjects();
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading STL files: " << e.what() << std::endl;
        if (parent) {
            QMessageBox::critical(parent, "Error", QString("Failed to load STL files: ") + e.what());
        }
    }
}

void VisualizationManager::setupScalarBar(VtkProcessor* vtkProcessor) {
    if (!ui_ || !vtkProcessor) return;
    
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
        ui_->getRenderer()->AddActor2D(scalarBar);
    }
}

void VisualizationManager::clearRenderer() {
    if (ui_ && ui_->getRenderer()) {
        ui_->getRenderer()->RemoveAllViewProps();
    }
}

void VisualizationManager::resetCamera() {
    if (ui_ && ui_->getRenderer()) {
        ui_->getRenderer()->ResetCamera();
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

void VisualizationManager::registerObject(const ObjectInfo& objInfo) {
    objectList.push_back(objInfo);
} 

void VisualizationManager::setObjectVisible(const std::string& filename, bool visible) {
    for (auto& obj : objectList) {
        // 完全なパスとファイル名の両方をチェック
        if (obj.filename == filename || 
            obj.filename.find(filename) != std::string::npos ||
            filename.find(obj.filename) != std::string::npos) {
            obj.visible = visible;
            if (obj.actor) {
                obj.actor->SetVisibility(visible ? 1 : 0);
            }
            renderRegisteredObjects();
            return;
        }
    }
}

void VisualizationManager::setObjectOpacity(const std::string& filename, double opacity) {
    for (auto& obj : objectList) {
        // 完全なパスとファイル名の両方をチェック
        if (obj.filename == filename || 
            obj.filename.find(filename) != std::string::npos ||
            filename.find(obj.filename) != std::string::npos) {
            obj.opacity = opacity;
            if (obj.actor) {
                obj.actor->GetProperty()->SetOpacity(opacity);
            }
            renderRegisteredObjects();
            return;
        }
    }
}

void VisualizationManager::renderRegisteredObjects() {
    if (!ui_ || !ui_->getRenderer()) return;
    clearRenderer();
    // すべてのObjectInfoのactorをレンダラーに追加
    for (const auto& obj : objectList) {
        if (obj.visible && obj.actor) {
            obj.actor->SetVisibility(1);
            obj.actor->GetProperty()->SetOpacity(obj.opacity);
            ui_->getRenderer()->AddActor(obj.actor);
        }
    }
    if (ui_->getVtkWidget() && ui_->getVtkWidget()->renderWindow()) {
        ui_->getVtkWidget()->renderWindow()->Render();
    }
} 