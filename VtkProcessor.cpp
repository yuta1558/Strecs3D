#include "VtkProcessor.h"
#include <filesystem>
#include <iostream>

VtkProcessor::VtkProcessor(const std::string& vtuFileName): vtuFileName(vtuFileName) {
    renderWindow->AddRenderer(renderer);
    renderWindowInteractor->SetRenderWindow(renderWindow);
}

void VtkProcessor::showInfo(){
    std::cout << "VTK file: " << vtuFileName << std::endl;
}


bool VtkProcessor:: LoadAndPrepareData(const char* filename, vtkSmartPointer<vtkUnstructuredGrid>& data, vtkSmartPointer<vtkLookupTable>& lookupTable, double stressRange[2]) {
    // VTKファイルの読み込み
    vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    reader->SetFileName(filename);
    reader->Update();
    // データセットを取得
    data = reader->GetOutput();
    if (!data) {
        std::cerr << "Error: Unable to read the VTK file." << std::endl;
        return false;
    }

    vtkPointData* pointData = data->GetPointData();
    pointData->SetActiveScalars("von Mises Stress");
    data->GetScalarRange(stressRange);
    // LookupTableの作成
    lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetNumberOfTableValues(256);
    lookupTable->SetRange(stressRange);
    lookupTable->SetHueRange(0.6667, 0.0); // 青から赤へ
    lookupTable->Build();

    minStress = stressRange[0];
    maxStress = stressRange[1];
    return true;
}


void VtkProcessor::prepareStressValues(float minStress,float maxStress){
    // 20000: 応力の刻み幅
    for (float v = minStress; v <= maxStress; v += 20000.0f) {
        stressValues.push_back(v);
    }
    isoSurfaceNum = stressValues.size();
}

bool VtkProcessor::generateIsoSurface(vtkSmartPointer<vtkUnstructuredGrid> data, std::vector<float> stressValues){
    for (int i = 0; i < isoSurfaceNum; ++i) {
        // 等値面の生成
        double isoValue = stressValues[i];
        auto singleContourFilter = vtkSmartPointer<vtkContourFilter>::New();
        singleContourFilter->SetInputData(data);
        singleContourFilter->SetValue(0, isoValue);
        singleContourFilter->Update();
        auto singleIsoSurface = vtkSmartPointer<vtkPolyData>::New();
        singleIsoSurface->DeepCopy(singleContourFilter->GetOutput());
        isoSurfaces.push_back(singleIsoSurface);
    }
    return true;
}


void VtkProcessor::deleteSmallIsosurface(std::vector<vtkSmartPointer<vtkPolyData>> isoSurfaces, double threshold){
    isoSurfaces.erase(
            std::remove_if(
                isoSurfaces.begin(),
                isoSurfaces.end(),
                [threshold](const vtkSmartPointer<vtkPolyData>& polyData) {
                    // 面積を計算
                    vtkSmartPointer<vtkMassProperties> massProperties =
                        vtkSmartPointer<vtkMassProperties>::New();
                    massProperties->SetInputData(polyData);
                    massProperties->Update();
                    double area = massProperties->GetSurfaceArea();
                    // “面積が threshold 未満かどうか”を判定
                    return (area < threshold);
                }
            ),
            isoSurfaces.end()
        );
    isoSurfaceNum = isoSurfaces.size();
}


vtkSmartPointer<vtkPolyData> VtkProcessor::scalePolyData(vtkSmartPointer<vtkPolyData> polyData, double scaleFactor){
    std::array<double, 3> center = VtkProcessor:: ComputeMeshCenter(polyData);
    std::cout << "Center: " << center[0] << ", " << center[1] << ", " << center[2] << std::endl;

    //原点中心に移動
    auto transformToOrigin = vtkSmartPointer<vtkTransform>::New();
    transformToOrigin->Translate(-center[0], -center[1], -center[2]); // x, y, z軸方向に移動
    auto transformFilterToOrigin = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilterToOrigin->SetInputData(polyData);
    transformFilterToOrigin->SetTransform(transformToOrigin);
    transformFilterToOrigin->Update();
    auto originPolyData = transformFilterToOrigin->GetOutput();

    //原点中心で少し拡大
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Scale(scaleFactor, scaleFactor, scaleFactor);
    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData(originPolyData);
    transformFilter->SetTransform(transform);
    transformFilter->Update();
    auto expandedSurfaceOrigin = transformFilter->GetOutput();

    //元の位置に戻す
    auto transformBack = vtkSmartPointer<vtkTransform>::New();
    transformBack->Translate(center[0], center[1], center[2]); // x, y, z軸方向に移動
    auto transformFilterBack = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilterBack->SetInputData(expandedSurfaceOrigin);
    transformFilterBack->SetTransform(transformBack);
    transformFilterBack->Update();
    auto expandedSurface = transformFilterBack->GetOutput();

    return expandedSurface;
}


// vtkPolyDataの中心点を計算する関数
std::array<double, 3> VtkProcessor:: ComputeMeshCenter(vtkSmartPointer<vtkPolyData> polyData)
{
    std::array<double, 3> center = {0.0, 0.0, 0.0};
    vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
    if (!points){
        throw std::runtime_error("No points in the input mesh.");
    }

    vtkIdType numberOfPoints = points->GetNumberOfPoints();
    if (numberOfPoints == 0){
        throw std::runtime_error("Mesh has no points.");
    }
    for (vtkIdType i = 0; i < numberOfPoints; ++i){
        double point[3];
        points->GetPoint(i, point);
        center[0] += point[0];
        center[1] += point[1];
        center[2] += point[2];
    }
    // 平均を計算
    center[0] /= numberOfPoints;
    center[1] /= numberOfPoints;
    center[2] /= numberOfPoints;

    return center;
}


vtkSmartPointer<vtkPolyData> VtkProcessor::makePolyDataSmooth(vtkSmartPointer<vtkPolyData> polyData){
    auto smoother = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    smoother->SetInputData(polyData);
    smoother->SetNumberOfIterations(20); // スムージングの繰り返し回数
    smoother->SetRelaxationFactor(0.1);
    smoother->FeatureEdgeSmoothingOff();
    smoother->BoundarySmoothingOn();
    smoother->Update();

    auto expandedSurface = smoother->GetOutput();
    return expandedSurface;
}


vtkSmartPointer<vtkPolyData> VtkProcessor::reversePolyDataOrientation(vtkSmartPointer<vtkPolyData> PolyData)
{
    // vtkReverseSenseフィルタのインスタンスを生成
    vtkSmartPointer<vtkReverseSense> reverseSenseFilter = vtkSmartPointer<vtkReverseSense>::New();
    reverseSenseFilter->SetInputData(PolyData);
    // セルおよび法線方向の反転を有効化
    reverseSenseFilter->ReverseCellsOn();
    reverseSenseFilter->ReverseNormalsOn();
    // フィルタを実行
    reverseSenseFilter->Update();

    // 結果をDeepCopyして返す（元データに影響を与えないため）
    vtkSmartPointer<vtkPolyData> reversedPolyData = vtkSmartPointer<vtkPolyData>::New();
    reversedPolyData->DeepCopy(reverseSenseFilter->GetOutput());

    return reversedPolyData;
}


void VtkProcessor:: stressDisplay(vtkSmartPointer<vtkUnstructuredGrid> data, vtkSmartPointer<vtkLookupTable> lookupTable, double scalarRange[2], vtkSmartPointer<vtkRenderer> renderer){
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(data);
    mapper->SetLookupTable(lookupTable);
    mapper->SetScalarRange(scalarRange);
    mapper->ScalarVisibilityOn();
    // データアクターの作成
    vtkSmartPointer<vtkActor> dataActor = vtkSmartPointer<vtkActor>::New();
    dataActor->SetMapper(mapper);
    dataActor->GetProperty()->SetOpacity(0.5); // 透明度50%
    renderer->AddActor(dataActor);
}

void VtkProcessor::polyDataDisplay(vtkSmartPointer<vtkPolyData> polyData, vtkSmartPointer<vtkRenderer> renderer){
    // ポリデータマッパーを作成
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    // アクターを作成
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetOpacity(0.5); // 透明度30%
    actor->GetProperty()->SetColor(0.8, 0.8, 0.8); // グレー色
    // アクターをレンダラーに追加
    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.2, 0.4); // 背景色を設定（RGB）
}

void VtkProcessor:: startRnederAndInteraction(vtkSmartPointer<vtkRenderWindow> renderWindow, vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor){
    // レンダリングとインタラクションの開始
    renderWindow->Render();
    renderWindowInteractor->Start();
}

void VtkProcessor::savePolyDataAsSTL(vtkPolyData* polyData, const std::string& fileName){
    // カレントディレクトリを取得
    std::filesystem::path currentPath = std::filesystem::current_path();
    // カレントディレクトリ/.temp のパスを生成
    std::filesystem::path tempDirPath = currentPath / ".temp";
    // .tempディレクトリが存在しなければ作成
    if (!std::filesystem::exists(tempDirPath)) {
        try {
            std::filesystem::create_directories(tempDirPath);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Failed to create directory: " << e.what() << std::endl;
            return; // 作成に失敗したら処理を中断するなどのハンドリングをする
        }
    }
    // 出力ファイルのフルパスを組み立てる
    std::filesystem::path outputFilePath = tempDirPath / fileName;
    // vtkSTLWriter のインスタンス生成
    vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();

    // 出力先を設定 (string()でstd::stringに変換、c_str()でconst char*に変換)
    writer->SetFileName(outputFilePath.string().c_str());
    // 書き込むvtkPolyDataを設定
    writer->SetInputData(polyData);

    // 書き出し実行
    if (!writer->Write()){
        // 書き込みに失敗した場合のエラーハンドリング
        std::cerr << "Error: Failed to write STL file: " << outputFilePath << std::endl;
    }
}