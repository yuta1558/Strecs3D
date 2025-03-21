#include "mainwindow.h"
#include "VtkProcessor.h"
#include "lib3mfProcessor.h"
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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("von Mises Stress Viewer");

    // セントラルウィジェットとレイアウトの作成
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // 「Open VTK File」ボタンの作成
    QPushButton* openVtkButton = new QPushButton("Open VTK File", centralWidget);
    layout->addWidget(openVtkButton);
    
    QPushButton* openStlButton = new QPushButton("Open STL File", centralWidget);
    layout->addWidget(openStlButton);

    // QVTKOpenGLNativeWidgetの作成
    vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    layout->addWidget(vtkWidget);

    QPushButton* processButton = new QPushButton("Process", centralWidget);
    layout->addWidget(processButton);

    setCentralWidget(centralWidget);

    // VTKのレンダリングウィンドウとレンダラーの作成
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);

    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // ボタンのシグナルとスロットの接続
    connect(openVtkButton, &QPushButton::clicked, this, &MainWindow::openVTKFile);
    connect(openStlButton, &QPushButton::clicked, this, &MainWindow::openSTLFile);
    connect(processButton, &QPushButton::clicked, this, &MainWindow::processFiles);

    resize(800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::openVTKFile()
{
    // ファイルダイアログの表示
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open VTK File",
                                                    "",
                                                    "VTK Files (*.vtu)");
    if (fileName.isEmpty())
        return; // ファイルが選択されなかった場合は何もしない
    vtkFile = fileName.toStdString();
    // 既存の描画オブジェクトをクリア
    renderer->RemoveAllViewProps();

    // VTKファイルの読み込み
    vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
        vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();

    // 読み込んだデータセットを取得
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid =
        reader->GetOutput();
    if (!unstructuredGrid)
    {
        std::cerr << "Error: Unable to read the VTK file." << std::endl;
        return;
    }

    // "von Mises Stress" をアクティブスカラーとして設定
    vtkPointData* pointData = unstructuredGrid->GetPointData();
    if (!pointData)
    {
        std::cerr << "Error: No point data found in the file." << std::endl;
        return;
    }
    pointData->SetActiveScalars("von Mises Stress");

    // ストレスのレンジを取得
    double stressRange[2];
    unstructuredGrid->GetScalarRange(stressRange);

    // LookupTableの作成（青から赤へのグラデーション）
    vtkSmartPointer<vtkLookupTable> lookupTable =
        vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetNumberOfTableValues(256);
    lookupTable->SetRange(stressRange);
    lookupTable->SetHueRange(0.6667, 0.0); // 青から赤へ
    lookupTable->Build();

    // Mapperの作成
    vtkSmartPointer<vtkDataSetMapper> mapper =
        vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(unstructuredGrid);
    mapper->SetLookupTable(lookupTable);
    mapper->SetScalarRange(stressRange);
    mapper->ScalarVisibilityOn();

    // Actorの作成
    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetOpacity(0.8); // 透明度80%

    // Actorをレンダラーに追加し、カメラをリセット
    renderer->AddActor(actor);
    renderer->ResetCamera();

    // レンダリングの更新
    vtkWidget->renderWindow()->Render();
}


void MainWindow::openSTLFile()
{
    // ファイルダイアログの表示
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open STL File",
                                                    "",
                                                    "STL Files (*.stl)");
    if (fileName.isEmpty())
        return; // ファイルが選択されなかった場合は何もしない
    stlFile = fileName.toStdString();
    // 既存の描画オブジェクトをクリア
    renderer->RemoveAllViewProps();

    // STLファイルの読み込み
    vtkSmartPointer<vtkSTLReader> reader =
        vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();

    // 読み込んだデータセットを取得
    vtkSmartPointer<vtkPolyData> polyData = reader->GetOutput();
    if (!polyData)
    {
        std::cerr << "Error: Unable to read the STL file." << std::endl;
        return;
    }

    // Mapperの作成
    vtkSmartPointer<vtkPolyDataMapper> mapper =
        vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    mapper->ScalarVisibilityOff(); // STLファイルは通常スカラー値を持たないため

    // Actorの作成
    vtkSmartPointer<vtkActor> actor =
        vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetOpacity(0.8); // 透明度80%

    // Actorをレンダラーに追加し、カメラをリセット
    renderer->AddActor(actor);
    renderer->ResetCamera();

    // レンダリングの更新
    vtkWidget->renderWindow()->Render();
}

void MainWindow::processFiles()
{
    try {
        // 1. VTKファイルの初期化とデータ準備
        if (!initializeVtkProcessor()) {
            throw std::runtime_error("VTK processor initialization failed");
        }

        // 2. メッシュ分割処理
        auto dividedMeshes = processMeshDivision();
        if (dividedMeshes.empty()) {
            throw std::runtime_error("No meshes generated during division");
        }

        // 3. 分割メッシュの保存
        saveDividedMeshes(dividedMeshes);

        // 4. 3MFファイルの生成
        if (!process3mfFile()) {
            throw std::runtime_error("3MF file processing failed");
        }

        QMessageBox::information(this, "Success", "Files processed successfully");
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing files: " << e.what() << std::endl;
        QMessageBox::critical(this, "Error", QString("Failed to process files: ") + e.what());
    }
}

bool MainWindow::process3mfFile()
{
    try {
        // 1. 3MFプロセッサの初期化
        Lib3mfProcessor lib3mfProcessor;

        // 2. 分割メッシュの読み込み
        if (!lib3mfProcessor.getMeshes()) {
            throw std::runtime_error("Failed to load divided meshes. Check if meshes exist and are valid.");
        }

        // 3. 元のSTLファイルの読み込み
        if (!lib3mfProcessor.setStl(stlFile)) {
            throw std::runtime_error("Failed to load STL file: " + stlFile);
        }

        // 4. メタデータの設定
        if (!lib3mfProcessor.setMetaData()) {
            throw std::runtime_error("Failed to set required metadata");
        }

        // 5. 3Dオブジェクトの構築
        if (!lib3mfProcessor.assembleObjects()) {
            throw std::runtime_error("Failed to assemble 3D objects");
        }

        // 6. 出力ディレクトリの準備
        const std::string outputDir = "result";
        const std::string outputPath = outputDir + "/result.3mf";
        
        // 7. 3MFファイルの保存
        if (!lib3mfProcessor.save3mf(outputPath)) {
            throw std::runtime_error("Failed to save 3MF file to: " + outputPath);
        }

        std::cout << "Successfully saved 3MF file: " << outputPath << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "3MF Processing Error: " << e.what() << std::endl;
        QMessageBox::critical(this, 
                            "3MF Processing Error", 
                            QString::fromStdString("Failed to process 3MF file:\n" + std::string(e.what())));
        return false;
    }
}

bool MainWindow::initializeVtkProcessor()
{
    if (vtkFile.empty()) {
        QMessageBox::warning(this, "Warning", "No VTK file selected");
        return false;
    }

    vtkProcessor = std::make_unique<VtkProcessor>(vtkFile);
    vtkProcessor->showInfo();
    vtkProcessor->LoadAndPrepareData();
    vtkProcessor->calcAverageStress();
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

void MainWindow::saveDividedMeshes(const std::vector<vtkSmartPointer<vtkPolyData>>& dividedMeshes)
{
    const auto& stressValues = vtkProcessor->getStressValues();
    
    for (size_t i = 0; i < dividedMeshes.size(); ++i) {
        float minValue = stressValues[i];
        float maxValue = stressValues[i + 1];
        std::string fileName = generateMeshFileName(i + 1, minValue, maxValue);
        vtkProcessor->savePolyDataAsSTL(dividedMeshes[i], fileName);
    }
}

std::string MainWindow::generateMeshFileName(int index, float minValue, float maxValue) const
{
    return "dividedMesh" + std::to_string(index) + "_" + 
           std::to_string(minValue) + "_" + 
           std::to_string(maxValue) + ".stl";
}
