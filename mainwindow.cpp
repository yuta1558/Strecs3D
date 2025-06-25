#include "mainwindow.h"
#include "VtkProcessor.h"
#include "lib3mfProcessor.h"
#include "utils/fileUtility.h"
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <vtkCamera.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkLookupTable.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkSTLReader.h>
#include <vtkPolyDataMapper.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("von Mises Stress Viewer");
    ui = std::make_unique<MainWindowUI>(this);
    setCentralWidget(ui->getCentralWidget());
    resize(1600, 900);

    vtkProcessor = std::make_unique<VtkProcessor>(vtkFile);
    setupCameraCallbacks();
    setupMessageConsoles();
}

void MainWindow::CameraCallback::Execute(vtkObject* caller, unsigned long, void*)
{
    vtkRenderer* renderer = static_cast<vtkRenderer*>(caller);
    if (window && renderer) {
        if (isImport) {
            window->syncCameras(renderer, window->ui->getSettingsTab()->getRenderer());
        } else {
            window->syncCameras(renderer, window->ui->getImportTab()->getRenderer());
        }
    }
}

void MainWindow::setupCameraCallbacks()
{
    importCameraCallback = vtkSmartPointer<CameraCallback>::New();
    importCameraCallback->window = this;
    importCameraCallback->isImport = true;
    ui->getImportTab()->getRenderer()->AddObserver(vtkCommand::ModifiedEvent, importCameraCallback);

    settingsCameraCallback = vtkSmartPointer<CameraCallback>::New();
    settingsCameraCallback->window = this;
    settingsCameraCallback->isImport = false;
    ui->getSettingsTab()->getRenderer()->AddObserver(vtkCommand::ModifiedEvent, settingsCameraCallback);
}

void MainWindow::setupMessageConsoles()
{
    // Connect message console signals
    connect(ui->getImportTab()->getMessageConsole(), &MessageConsole::messageAdded,
            this, &MainWindow::syncMessageConsoles);
    connect(ui->getSettingsTab()->getMessageConsole(), &MessageConsole::messageAdded,
            this, &MainWindow::syncMessageConsoles);
    connect(ui->getPreviewTab()->getMessageConsole(), &MessageConsole::messageAdded,
            this, &MainWindow::syncMessageConsoles);
}

void MainWindow::syncMessageConsoles(const QString& message)
{
    // Get the sender console
    MessageConsole* sender = qobject_cast<MessageConsole*>(QObject::sender());
    if (!sender) return;

    // Block signals temporarily to prevent infinite recursion
    if (ui->getImportTab() && ui->getImportTab()->getMessageConsole() && sender != ui->getImportTab()->getMessageConsole()) {
        ui->getImportTab()->getMessageConsole()->blockSignals(true);
        ui->getImportTab()->getMessageConsole()->appendMessage(message);
        ui->getImportTab()->getMessageConsole()->blockSignals(false);
    }

    if (ui->getSettingsTab() && ui->getSettingsTab()->getMessageConsole() && sender != ui->getSettingsTab()->getMessageConsole()) {
        ui->getSettingsTab()->getMessageConsole()->blockSignals(true);
        ui->getSettingsTab()->getMessageConsole()->appendMessage(message);
        ui->getSettingsTab()->getMessageConsole()->blockSignals(false);
    }

    if (ui->getPreviewTab() && ui->getPreviewTab()->getMessageConsole() && sender != ui->getPreviewTab()->getMessageConsole()) {
        ui->getPreviewTab()->getMessageConsole()->blockSignals(true);
        ui->getPreviewTab()->getMessageConsole()->appendMessage(message);
        ui->getPreviewTab()->getMessageConsole()->blockSignals(false);
    }
}

void MainWindow::logMessage(const QString& message)
{
    if (ui && ui->getImportTab() && ui->getImportTab()->getMessageConsole()) {
        ui->getImportTab()->getMessageConsole()->appendMessage(message);
    }
}

MainWindow::~MainWindow()
{
    if (ui->getImportTab()->getRenderer() && importCameraCallback) {
        ui->getImportTab()->getRenderer()->RemoveObserver(importCameraCallback);
    }
    if (ui->getSettingsTab()->getRenderer() && settingsCameraCallback) {
        ui->getSettingsTab()->getRenderer()->RemoveObserver(settingsCameraCallback);
    }
}

void MainWindow::syncCameras(vtkRenderer* source, vtkRenderer* dest)
{
    if (!source || !dest) return;
    
    vtkCamera* sourceCam = source->GetActiveCamera();
    vtkCamera* destCam = dest->GetActiveCamera();
    
    // カメラパラメータをコピー
    destCam->SetPosition(sourceCam->GetPosition());
    destCam->SetFocalPoint(sourceCam->GetFocalPoint());
    destCam->SetViewUp(sourceCam->GetViewUp());
    destCam->SetViewAngle(sourceCam->GetViewAngle());
    destCam->SetClippingRange(sourceCam->GetClippingRange());
    
    dest->ResetCameraClippingRange();
    dest->GetRenderWindow()->Render();
}

void MainWindow::processFiles()
{
    try {
        if (!initializeVtkProcessor()) {
            throw std::runtime_error("VTK processor initialization failed");
        }

        auto dividedMeshes = processMeshDivision();
        if (dividedMeshes.empty()) {
            throw std::runtime_error("No meshes generated during division");
        }

        vtkProcessor->saveDividedMeshes(dividedMeshes);

        if (!process3mfFile()) {
            throw std::runtime_error("3MF file processing failed");
        }

        // Display STL files first
        loadAndDisplayTempStlFiles();

        // Then clear the temporary directory
        std::filesystem::path tempFiledir = ".temp";
        FileUtility::clearDirectoryContents(tempFiledir);

        QMessageBox::information(this, "Success", "Files processed successfully");
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing files: " << e.what() << std::endl;
        QMessageBox::critical(this, "Error", QString("Failed to process files: ") + e.what());
    }
}

bool MainWindow::initializeVtkProcessor()
{
    vtkProcessor->clearPreviousData();
    if (vtkFile.empty()) {
        QMessageBox::warning(this, "Warning", "No VTK file selected");
        return false;
    }
    if (stlFile.empty()) {
        QMessageBox::warning(this, "Warning", "No STL file selected");
        return false;
    }

    vtkProcessor->showInfo();
    vtkProcessor->prepareStressValues();
    
    return true;
}


std::vector<vtkSmartPointer<vtkPolyData>> MainWindow::processMeshDivision()
{
    if (!vtkProcessor) {
        throw std::runtime_error("VtkProcessor not initialized");
    }

    auto dividedMeshes = vtkProcessor->divideMesh();
    if (dividedMeshes.empty()) {
        throw std::runtime_error("No meshes generated");
    }

    return dividedMeshes;
}

bool MainWindow::process3mfFile()
{
    try {
        Lib3mfProcessor lib3mfProcessor;
        
        if (!loadInputFiles(lib3mfProcessor)) {
            throw std::runtime_error("Failed to load input files");
        }

        QString currentMode = ui->getSettingsTab()->getModeComboBox()->currentText();
        if (!processByMode(lib3mfProcessor, currentMode)) {
            throw std::runtime_error("Failed to process in " + currentMode.toStdString() + " mode");
        }

        return true;
    }
    catch (const std::exception& e) {
        handle3mfError(e);
        return false;
    }
}

bool MainWindow::loadInputFiles(Lib3mfProcessor& processor)
{
    if (!processor.getMeshes()) {
        throw std::runtime_error("Failed to load divided meshes");
    }

    if (!processor.setStl(stlFile)) {
        throw std::runtime_error("Failed to load STL file: " + stlFile);
    }

    return true;
}

bool MainWindow::processByMode(Lib3mfProcessor& processor, const QString& mode)
{
    if (mode == "cura") {
        return processCuraMode(processor);
    } else if (mode == "bambu") {
        return processBambuMode(processor);
    }
    throw std::runtime_error("Unknown mode: " + mode.toStdString());
}

bool MainWindow::processCuraMode(Lib3mfProcessor& processor)
{
    std::cout << "Processing in Cura mode" << std::endl;
    
    if (!processor.setMetaData(vtkProcessor->getMaxStress())) {
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

bool MainWindow::processBambuMode(Lib3mfProcessor& processor)
{
    std::cout << "Processing in Bambu mode" << std::endl;
    
    processor.setMetaDataBambu(vtkProcessor->getMaxStress());
    
    const std::string tempFile = ".temp/result.3mf";
    if (!processor.save3mf(tempFile)) {
        throw std::runtime_error("Failed to save temporary 3MF file");
    }

    return processBambuZipFiles();
}

bool MainWindow::processBambuZipFiles()
{
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

void MainWindow::handle3mfError(const std::exception& e)
{
    std::cerr << "3MF Processing Error: " << e.what() << std::endl;
    QMessageBox::critical(this, 
                         "3MF Processing Error", 
                         QString::fromStdString("Failed to process 3MF file:\n" + std::string(e.what())));
}

void MainWindow::loadAndDisplayTempStlFiles()
{
    try {
        std::filesystem::path tempDir = ".temp/div";
        if (!std::filesystem::exists(tempDir)) {
            throw std::runtime_error(".temp directory does not exist");
        }

        // Clear existing actors from the preview tab
        ui->getPreviewTab()->getRenderer()->RemoveAllViewProps();

        // Get all STL files and extract their numbers
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

        // Sort by the extracted number
        std::sort(stlFiles.begin(), stlFiles.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        if (stlFiles.empty()) {
            throw std::runtime_error("No valid STL files found");
        }

        // Get min and max numbers for color interpolation
        int minNumber = stlFiles.front().second;
        int maxNumber = stlFiles.back().second;
        double range = maxNumber - minNumber;

        // Display each STL file with appropriate color
        for (const auto& [path, number] : stlFiles) {
            double r, g, b;
            
            // Calculate normalized position (0 to 1) in the range
            double normalizedPos = (number - minNumber) / range;
            
            if (normalizedPos <= 0.5) {
                // Blue to White (0 to 0.5)
                double t = normalizedPos * 2.0; // Scale to 0-1
                r = t;
                g = t;
                b = 1.0;
            } else {
                // White to Red (0.5 to 1)
                double t = (normalizedPos - 0.5) * 2.0; // Scale to 0-1
                r = 1.0;
                g = 1.0 - t;
                b = 1.0 - t;
            }

            auto actor = vtkProcessor->getColoredStlActor(path.string(), r, g, b);
            if (actor) {
                ui->getPreviewTab()->getRenderer()->AddActor(actor);
            }
        }

        // Reset camera and render
        ui->getPreviewTab()->getRenderer()->ResetCamera();
        ui->getPreviewTab()->getVtkWidget()->renderWindow()->Render();
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading STL files: " << e.what() << std::endl;
        QMessageBox::critical(this, "Error", QString("Failed to load STL files: ") + e.what());
    }
}

void MainWindow::openVTKFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open VTK File",
                                                    "",
                                                    "VTK Files (*.vtu)");
    if (fileName.isEmpty())
        return;
    vtkFile = fileName.toStdString();
    
    logMessage("Open VTK File: " + fileName);
    ui->getImportTab()->getRenderer()->RemoveAllViewProps();
    auto importActor = vtkProcessor->getVtuActor(vtkFile);
    ui->getImportTab()->getRenderer()->AddActor(importActor);
    auto lookupTable = vtkProcessor->getCurrentLookupTable();
    if (lookupTable)
    {
        vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
        scalarBar->SetLookupTable(lookupTable);
        scalarBar->SetTitle("von Mises Stress");
        scalarBar->GetLabelTextProperty()->SetColor(1,1,1);
        scalarBar->GetTitleTextProperty()->SetColor(1,1,1);
        scalarBar->SetNumberOfLabels(5);
        scalarBar->SetOrientationToHorizontal();
        scalarBar->SetWidth(0.5);
        scalarBar->SetHeight(0.05);
        scalarBar->SetPosition(0.5, 0.05);
        ui->getImportTab()->getRenderer()->AddActor2D(scalarBar);
    }
    ui->getImportTab()->getRenderer()->ResetCamera();

    ui->getSettingsTab()->getRenderer()->RemoveAllViewProps();
    auto settingsActor = vtkProcessor->getVtuActor(vtkFile);
    ui->getSettingsTab()->getRenderer()->AddActor(settingsActor);
    lookupTable = vtkProcessor->getCurrentLookupTable();
    if (lookupTable)
    {
        vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
        scalarBar->SetLookupTable(lookupTable);
        scalarBar->SetTitle("von Mises Stress");
        scalarBar->GetLabelTextProperty()->SetColor(1,1,1);
        scalarBar->GetTitleTextProperty()->SetColor(1,1,1);
        scalarBar->SetNumberOfLabels(5);
        scalarBar->SetOrientationToHorizontal();
        scalarBar->SetWidth(0.5);
        scalarBar->SetHeight(0.05);
        scalarBar->SetPosition(0.5, 0.05);
        ui->getSettingsTab()->getRenderer()->AddActor2D(scalarBar);
    }
    ui->getSettingsTab()->getRenderer()->ResetCamera();

    // Stress範囲をスライダーに反映
    auto slider = ui->getSettingsTab()->getRangeSlider();
    if (slider) {
        slider->setStressRange(vtkProcessor->getMinStress(), vtkProcessor->getMaxStress());
    }

    syncCameras(ui->getImportTab()->getRenderer(), ui->getSettingsTab()->getRenderer());

    ui->getImportTab()->getVtkWidget()->renderWindow()->Render();
    ui->getSettingsTab()->getVtkWidget()->renderWindow()->Render();
}

void MainWindow::openSTLFile()
{
    QString filename = QFileDialog::getOpenFileName(this,
        "Open STL File",
        QDir::homePath(),
        "STL Files (*.stl)");

    if (filename.isEmpty()) {
        return;
    }

    currentStlFilename = filename;
    stlFile = filename.toStdString();
    logMessage("Open STL File: " +  filename );
    ui->getImportTab()->getRenderer()->RemoveAllViewProps();
    auto importActor = vtkProcessor->getStlActor(stlFile);
    ui->getImportTab()->getRenderer()->AddActor(importActor);
    ui->getImportTab()->getRenderer()->ResetCamera();

    ui->getSettingsTab()->getRenderer()->RemoveAllViewProps();
    auto settingsActor = vtkProcessor->getStlActor(stlFile);
    ui->getSettingsTab()->getRenderer()->AddActor(settingsActor);
    ui->getSettingsTab()->getRenderer()->ResetCamera();

    syncCameras(ui->getImportTab()->getRenderer(), ui->getSettingsTab()->getRenderer());

    ui->getImportTab()->getVtkWidget()->renderWindow()->Render();
    ui->getSettingsTab()->getVtkWidget()->renderWindow()->Render();
}

QString MainWindow::getCurrentMode() const
{
    return ui->getSettingsTab()->getModeComboBox()->currentText();
}
