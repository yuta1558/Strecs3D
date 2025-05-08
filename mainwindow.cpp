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

    // 中央ウィジェット＋レイアウト
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // ─── タブウィジェット ─────────────────────────────
    QTabWidget* tabWidget = new QTabWidget(centralWidget);
    // 角丸＆ダークテーマ風のスタイル
    tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #444;
            border-radius: 8px;
            background: #2e2e2e;
        }
        QTabBar::tab {
            background: #3a3a3a;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            padding: 6px 12px;
            min-width: 80px;
        }
        QTabBar::tab:selected {
            background:rgb(79, 76, 119);
        }
    )");

    // 各タブページ（中身はあとでレイアウトを作る）
    QWidget* importPage  = new QWidget();
    QWidget* settingsPage = new QWidget();
    QWidget* previewPage = new QWidget();

    tabWidget->addTab(importPage,  tr("Import"));
    tabWidget->addTab(settingsPage, tr("Settings"));
    tabWidget->addTab(previewPage, tr("Preview"));

    mainLayout->addWidget(tabWidget);
    // ────────────────────────────────────────────────

    // ─── Import タブのレイアウト ───────────────────────
    QVBoxLayout* importLayout = new QVBoxLayout(importPage);
    QPushButton* openStlButton = new QPushButton("Open STL File", importPage);
    QPushButton* openVtkButton = new QPushButton("Open VTK File", importPage);
    importLayout->addWidget(openStlButton);
    importLayout->addWidget(openVtkButton);

    importVtkWidget = new QVTKOpenGLNativeWidget(importPage);
    importLayout->addWidget(importVtkWidget);

    importRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    importVtkWidget->setRenderWindow(importRenderWindow);
    importRenderer = vtkSmartPointer<vtkRenderer>::New();
    importRenderWindow->AddRenderer(importRenderer);
    // ────────────────────────────────────────────────

    // ─── settings タブのレイアウト ──────────────────────
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsPage);
    modeComboBox = new QComboBox(settingsPage);
    modeComboBox->addItem("cura");
    modeComboBox->addItem("bambu");
    QPushButton* processButton = new QPushButton("Process", settingsPage);
    settingsLayout->addWidget(modeComboBox);
    settingsLayout->addWidget(processButton);

    settingsVtkWidget = new QVTKOpenGLNativeWidget(settingsPage);
    settingsLayout->addWidget(settingsVtkWidget);

    settingsRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    settingsVtkWidget->setRenderWindow(settingsRenderWindow);
    settingsRenderer = vtkSmartPointer<vtkRenderer>::New();
    settingsRenderWindow->AddRenderer(settingsRenderer);
    // ────────────────────────────────────────────────

    // ─── Preview タブのレイアウト ──────────────────────
    QVBoxLayout* previewLayout = new QVBoxLayout(previewPage);
    // Preview タブ用の独自のウィジェットを配置
    // previewVtkWidget = new QVTKOpenGLNativeWidget(previewPage);
    // previewLayout->addWidget(previewVtkWidget);
    // ────────────────────────────────────────────────

    setCentralWidget(centralWidget);

    // VTK のセットアップ
    // renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    // importVtkWidget->setRenderWindow(renderWindow);
    // settingsVtkWidget->setRenderWindow(renderWindow);
    // renderer = vtkSmartPointer<vtkRenderer>::New();
    // renderWindow->AddRenderer(renderer);

    // シグナル／スロット接続
    connect(openStlButton,  &QPushButton::clicked, this, &MainWindow::openSTLFile);
    connect(openVtkButton,  &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(processButton,  &QPushButton::clicked, this, &MainWindow::processFiles);

    // 全体のスタイル＆サイズ
    resize(1600, 900);
    importVtkWidget->setStyleSheet("background-color: #1a1a1a;");
    settingsVtkWidget->setStyleSheet("background-color: #1a1a1a;");
    this->setStyleSheet("background-color: #1a1a1a;");

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

    importRenderer->RemoveAllViewProps();
    auto importActor = vtkProcessor->getVtuActor(vtkFile);
    importRenderer->AddActor(importActor);
    importRenderer->ResetCamera();

    settingsRenderer->RemoveAllViewProps();
    auto settingsActor = vtkProcessor->getVtuActor(vtkFile);
    settingsRenderer->AddActor(settingsActor);
    settingsRenderer->ResetCamera();

    importVtkWidget->renderWindow()->Render();
    settingsVtkWidget->renderWindow()->Render();
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
    importRenderer->RemoveAllViewProps();
    auto importActor = vtkProcessor->getStlActor(stlFile);
    importRenderer->AddActor(importActor);
    importRenderer->ResetCamera();

    settingsRenderer->RemoveAllViewProps();
    auto settingsActor = vtkProcessor->getStlActor(stlFile);
    settingsRenderer->AddActor(settingsActor);
    settingsRenderer->ResetCamera();

    importVtkWidget->renderWindow()->Render();
    settingsVtkWidget->renderWindow()->Render();
}