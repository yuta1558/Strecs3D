#include "mainwindow.h"
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkLookupTable.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkUnstructuredGrid.h>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("von Mises Stress Viewer");

    // セントラルウィジェットとレイアウトの作成
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // 「Open VTK File」ボタンの作成
    QPushButton* openButton = new QPushButton("Open VTK File", centralWidget);
    layout->addWidget(openButton);

    // QVTKOpenGLNativeWidgetの作成
    vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    layout->addWidget(vtkWidget);

    setCentralWidget(centralWidget);

    // VTKのレンダリングウィンドウとレンダラーの作成
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);

    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // ボタンのシグナルとスロットの接続
    connect(openButton, &QPushButton::clicked, this, &MainWindow::openVTKFile);

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
