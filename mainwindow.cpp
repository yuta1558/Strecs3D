#include "mainwindow.h"
#include "VtkProcessor.h"
#include "lib3mfProcessor.h"
#include "utils/fileUtility.h"
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMessageBox>
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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("von Mises Stress Viewer");

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    QPushButton* openStlButton = new QPushButton("Open STL File", centralWidget);
    layout->addWidget(openStlButton);

    QPushButton* openVtkButton = new QPushButton("Open VTK File", centralWidget);
    layout->addWidget(openVtkButton);
    
    vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    layout->addWidget(vtkWidget);


    modeComboBox = new QComboBox(centralWidget);
    modeComboBox->addItem("cura");
    modeComboBox->addItem("bambu");
    layout->addWidget(modeComboBox);

    QPushButton* processButton = new QPushButton("Process", centralWidget);
    layout->addWidget(processButton);

    setCentralWidget(centralWidget);

    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);

    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    connect(openVtkButton, &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(openStlButton, &QPushButton::clicked, this, &MainWindow::openSTLFile);
    connect(processButton, &QPushButton::clicked, this, &MainWindow::processFiles);

    resize(800, 600);
    vtkProcessor = std::make_unique<VtkProcessor>(vtkFile);
}

MainWindow::~MainWindow()
{
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

        QString currentMode = modeComboBox->currentText();
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



void MainWindow::openVTKFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open VTK File",
                                                    "",
                                                    "VTK Files (*.vtu)");
    if (fileName.isEmpty())
        return; // ファイルが選択されなかった場合は何もしない
    vtkFile = fileName.toStdString();
    renderer->RemoveAllViewProps();

    auto actor = vtkProcessor->getVtuActor(vtkFile);
    renderer->AddActor(actor);
    renderer->ResetCamera();

    vtkWidget->renderWindow()->Render();
}


void MainWindow::openSTLFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open STL File",
                                                    "",
                                                    "STL Files (*.stl)");
    if (fileName.isEmpty())
        return; // ファイルが選択されなかった場合は何もしない
    stlFile = fileName.toStdString();
    renderer->RemoveAllViewProps();

    auto actor = vtkProcessor->getStlActor(stlFile);
    renderer->AddActor(actor);
    renderer->ResetCamera();

    vtkWidget->renderWindow()->Render();
}