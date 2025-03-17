#include <iostream>
#include <QApplication>
#include <QMainWindow>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkLookupTable.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkUnstructuredGrid.h>

int main(int argc, char *argv[])
{
    int a = 0;
    QApplication app(argc, argv);

    // QVTKOpenGLNativeWidgetで使用するデフォルトのSurfaceFormatを設定
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    // メインウィンドウの作成
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("von Mises Stress Viewer");

    // セントラルウィジェットとレイアウトの作成
    QWidget* centralWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // 「Open VTK File」ボタンの作成
    QPushButton* openButton = new QPushButton("Open VTK File");
    layout->addWidget(openButton);

    // QVTKOpenGLNativeWidgetの作成
    QVTKOpenGLNativeWidget* vtkWidget = new QVTKOpenGLNativeWidget;
    layout->addWidget(vtkWidget);

    mainWindow.setCentralWidget(centralWidget);

    // VTKのレンダリングウィンドウとレンダラーの作成
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow =
        vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);

    vtkSmartPointer<vtkRenderer> renderer =
        vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // ボタンがクリックされたときの処理
    QObject::connect(openButton, &QPushButton::clicked, [&]() {
        // ファイルダイアログの表示
        QString fileName = QFileDialog::getOpenFileName(&mainWindow,
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
        actor->GetProperty()->SetOpacity(0.8); // 透明度50%

        // Actorをレンダラーに追加し、カメラをリセット
        renderer->AddActor(actor);
        renderer->ResetCamera();

        // レンダリングの更新
        vtkWidget->renderWindow()->Render();
    });

    mainWindow.resize(800, 600);
    mainWindow.show();

    return app.exec();
}
