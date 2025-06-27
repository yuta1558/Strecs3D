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

    // ボタン接続
    connect(ui->getOpenStlButton(), &QPushButton::clicked, this, &MainWindow::openSTLFile);
    connect(ui->getOpenVtkButton(), &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(ui->getProcessButton(), &QPushButton::clicked, this, &MainWindow::processFiles);
    connect(ui->getExport3mfButton(), &QPushButton::clicked, [this]() {
        QString sourcePath = QDir::currentPath() + "/result/result.3mf";
        if (!QFile::exists(sourcePath)) {
            QMessageBox::warning(this, "Error", "No 3MF file found in result directory.");
            return;
        }
        QString defaultName = QFileInfo(QString::fromStdString(stlFile)).baseName() + ".3mf";
        QString savePath = QFileDialog::getSaveFileName(this,
            "Save 3MF File",
            QDir::homePath() + "/" + defaultName,
            "3MF Files (*.3mf)");
        if (savePath.isEmpty()) return;
        if (!savePath.endsWith(".3mf", Qt::CaseInsensitive)) savePath += ".3mf";
        if (QFile::copy(sourcePath, savePath)) {
            QMessageBox::information(this, "Success", "3MF file exported successfully.");
        } else {
            QMessageBox::critical(this, "Error", "Failed to export 3MF file.");
        }
    });
}

void MainWindow::logMessage(const QString& message)
{
    if (ui && ui->getMessageConsole()) {
        ui->getMessageConsole()->appendMessage(message);
    }
}

MainWindow::~MainWindow() {}

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
        loadAndDisplayTempStlFiles();
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
    auto slider = ui->getRangeSlider();
    std::vector<double> thresholds;
    if (slider) {
        thresholds = slider->stressThresholds();
    }
    vtkProcessor->prepareStressValues(thresholds);
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
        QString currentMode = ui->getModeComboBox()->currentText();
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
    auto slider = ui->getRangeSlider();
    std::vector<StressDensityMapping> mappings;
    if (slider) {
        mappings = slider->stressDensityMappings();
    }
    if (!processor.setMetaData(vtkProcessor->getMaxStress(), mappings)) {
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
        // 既存のアクターをクリア
        ui->getRenderer()->RemoveAllViewProps();
        // STLファイルを取得し番号でソート
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
        if (stlFiles.empty()) {
            throw std::runtime_error("No valid STL files found");
        }
        int minNumber = stlFiles.front().second;
        int maxNumber = stlFiles.back().second;
        double range = maxNumber - minNumber;
        for (const auto& [path, number] : stlFiles) {
            double r, g, b;
            double normalizedPos = (number - minNumber) / range;
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
            auto actor = vtkProcessor->getColoredStlActor(path.string(), r, g, b);
            if (actor) {
                ui->getRenderer()->AddActor(actor);
            }
        }
        ui->getRenderer()->ResetCamera();
        ui->getVtkWidget()->renderWindow()->Render();
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
    ui->getRenderer()->RemoveAllViewProps();
    auto importActor = vtkProcessor->getVtuActor(vtkFile);
    ui->getRenderer()->AddActor(importActor);
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
        ui->getRenderer()->AddActor2D(scalarBar);
    }
    ui->getRenderer()->ResetCamera();
    auto slider = ui->getRangeSlider();
    if (slider) {
        slider->setStressRange(vtkProcessor->getMinStress(), vtkProcessor->getMaxStress());
    }
    ui->getVtkWidget()->renderWindow()->Render();
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
    ui->getRenderer()->RemoveAllViewProps();
    auto importActor = vtkProcessor->getStlActor(stlFile);
    ui->getRenderer()->AddActor(importActor);
    ui->getRenderer()->ResetCamera();
    ui->getVtkWidget()->renderWindow()->Render();
}

QString MainWindow::getCurrentMode() const
{
    return ui->getModeComboBox()->currentText();
}
