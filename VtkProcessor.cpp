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

bool VtkProcessor:: LoadAndPrepareData() {
    // VTKファイルの読み込み
    vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    reader->SetFileName(vtuFileName.c_str());
    reader->Update();
    // データセットを取得
    vtuData = reader->GetOutput();
    if (!vtuData) {
        std::cerr << "Error: Unable to read the VTK file." << std::endl;
        return false;
    }

    vtkPointData* pointData = vtuData->GetPointData();
    pointData->SetActiveScalars("von Mises Stress");
    vtuData->GetScalarRange(stressRange);
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

bool VtkProcessor::calcAverageStress() {
    // (1) まず、PointDataから"von Mises Stress"を読み込む
    std::string arrayName = "von Mises Stress";
    vtkDataArray* pointDataArray = vtuData->GetPointData()->GetArray(arrayName.c_str());
    if(!pointDataArray) {
        std::cerr << "PointData array not found: " << arrayName << std::endl;
        return false;
    }

    vtkIdType numCells = vtuData->GetNumberOfCells();
    // (2) Cellの数だけ要素数を持つ新しい配列を作成 (Cell Data 用)
    auto avgStressArray = vtkSmartPointer<vtkDoubleArray>::New();
    avgStressArray->SetName("AverageVonMisesStress");        // 新しい配列の名前
    avgStressArray->SetNumberOfComponents(1);               // スカラーなので1成分
    avgStressArray->SetNumberOfTuples(numCells);            // セル数だけタプルを用意

    // (3) 各セルごとに平均値を計算し、新配列に書き込む
    for(vtkIdType cid = 0; cid < numCells; ++cid) {
        vtkCell* cell = vtuData->GetCell(cid);
        vtkIdList* pidList = cell->GetPointIds();
        vtkIdType npts = pidList->GetNumberOfIds();

        double sum = 0.0;
        for(vtkIdType i = 0; i < npts; ++i) {
            vtkIdType pid = pidList->GetId(i);
            double val = pointDataArray->GetComponent(pid, 0);
            sum += val;
        }
        double avgCell = (npts > 0) ? sum / static_cast<double>(npts) : 0.0;

        // (4) 新しい配列(avgStressArray)の cid 番目にセット
        avgStressArray->SetValue(cid, avgCell);

        // 確認用の出力
        // std::cout << "[PointData] Cell " << cid
        //           << " average von Mises stress: " << avgCell << std::endl;
    }

    // (5) 生成した配列を CellData に追加
    vtuData->GetCellData()->AddArray(avgStressArray);

    return true;
}


vtkSmartPointer<vtkPolyData> VtkProcessor::extractCellsInRegion(double lowerBound, double upperBound){

    // --- vtkThresholdフィルタの設定 ---
    vtkSmartPointer<vtkThreshold> thresholdFilter = vtkSmartPointer<vtkThreshold>::New();
    thresholdFilter->SetInputData(vtuData);
    // "AverageVonMisesStress"がセルデータとして登録されていることを指定する
    thresholdFilter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, "AverageVonMisesStress");
    
    // --- ThresholdFunctionの設定 ---
    // vtkThreshold::THRESHOLD_BETWEEN を指定して、下限と上限の両方でフィルタリングする
    thresholdFilter->SetThresholdFunction(vtkThreshold::THRESHOLD_BETWEEN);
    thresholdFilter->SetLowerThreshold(lowerBound);
    thresholdFilter->SetUpperThreshold(upperBound);
    
    thresholdFilter->Update();

    // --- 抽出されたセルはvtkThresholdの出力（vtkUnstructuredGrid）に格納される ---
    vtkSmartPointer<vtkUnstructuredGrid> extractedCells = thresholdFilter->GetOutput();

    // --- 必要に応じて、vtkDataSetSurfaceFilterを用いてvtkPolyDataに変換（例：可視化用） ---
    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surfaceFilter->SetInputData(extractedCells);
    surfaceFilter->Update();

    vtkSmartPointer<vtkPolyData> combinedMesh = surfaceFilter->GetOutput();
    return combinedMesh;
}

std::vector<vtkSmartPointer<vtkPolyData>> VtkProcessor::divideMesh() {
    std::vector<vtkSmartPointer<vtkPolyData>> dividedPolyData;

    // stressValues[i] ～ stressValues[i+1] の各範囲ごとにフィルターを適用
    for (int i = 0; i < isoSurfaceNum - 1; ++i) {
        double minValue = stressValues[i];
        double maxValue = stressValues[i + 1];
        std::cout << "Extracting cells in range: " << minValue << " -> " << maxValue << std::endl;

        // 指定範囲のセルを抽出
        vtkSmartPointer<vtkPolyData> currentPolyData = this->extractCellsInRegion(minValue, maxValue);
        
        if (!currentPolyData || currentPolyData->GetNumberOfCells() == 0) {
            std::cout << "No cells extracted for range: " << minValue << " -> " << maxValue << std::endl;
            continue;
        }

        // 体積計算用のフィルター
        auto massProperties = vtkSmartPointer<vtkMassProperties>::New();
        massProperties->SetInputData(currentPolyData);
        massProperties->Update();
        
        double currentVolume = massProperties->GetVolume();
        
        // 体積が基準を超えるまで次の範囲と結合
        while (currentVolume < volumeThreshold && i < isoSurfaceNum - 2) {
            i++;
            double nextMin = stressValues[i];
            double nextMax = stressValues[i + 1];
            
            std::cout << "Combining with next range: " << nextMin << " -> " << nextMax << std::endl;
            
            // 次の範囲のセルを抽出
            auto nextPolyData = this->extractCellsInRegion(nextMin, nextMax);
            
            if (!nextPolyData || nextPolyData->GetNumberOfCells() == 0) {
                std::cout << "No cells in next range to combine" << std::endl;
                continue;
            }

            // セルを結合
            auto appendFilter = vtkSmartPointer<vtkAppendFilter>::New();
            appendFilter->AddInputData(currentPolyData);
            appendFilter->AddInputData(nextPolyData);
            appendFilter->Update();

            // 結合結果をPolyDataに変換
            auto surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
            surfaceFilter->SetInputConnection(appendFilter->GetOutputPort());
            surfaceFilter->Update();
            
            currentPolyData = surfaceFilter->GetOutput();
            
            // 新しい体積を計算
            massProperties->SetInputData(currentPolyData);
            massProperties->Update();
            currentVolume = massProperties->GetVolume();
        }

        if (currentPolyData && currentPolyData->GetNumberOfCells() > 0) {
            dividedPolyData.push_back(currentPolyData);
            std::cout << "Added polyData with volume: " << currentVolume << std::endl;
        }
    }

    return dividedPolyData;
}

void VtkProcessor::prepareStressValues(){
    // 20000: 応力の刻み幅
    for (float v = minStress; v <= maxStress; v +=20000.0f) {
        stressValues.push_back(v);
    }
    isoSurfaceNum = stressValues.size();
    std::cout<< "isoSurfaceNum: " << isoSurfaceNum << std::endl;
}

bool VtkProcessor::generateIsoSurface(){
    for (int i = 0; i < isoSurfaceNum; ++i) {
        // 等値面の生成
        double isoValue = stressValues[i];
        auto singleContourFilter = vtkSmartPointer<vtkContourFilter>::New();
        singleContourFilter->SetInputData(vtuData);
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


void VtkProcessor:: stressDisplay(){
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(vtuData);
    mapper->SetLookupTable(lookupTable);
    mapper->SetScalarRange(stressRange);
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

void VtkProcessor:: startRnederAndInteraction(){
    // レンダリングとインタラクションの開始
    renderWindow->Render();
    renderWindowInteractor->Start();
}

void VtkProcessor::savePolyDataAsSTL(vtkPolyData* polyData, const std::string& fileName) {
    // カレントディレクトリを取得
    std::filesystem::path currentPath = std::filesystem::current_path();
    // カレントディレクトリ/.temp/iso のパスを生成
    std::filesystem::path tempDirPath = currentPath / ".temp/div";
    // .tempディレクトリが存在しなければ作成
    if (!std::filesystem::exists(tempDirPath)) {
        try {
            std::filesystem::create_directories(tempDirPath);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Failed to create directory: " << e.what() << std::endl;
            return; // 作成に失敗した場合は処理を中断
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
    // STLファイルをバイナリ形式で出力するように設定
    writer->SetFileTypeToBinary();

    // 書き出し実行
    if (!writer->Write()) {
        // 書き込みに失敗した場合のエラーハンドリング
        std::cerr << "Error: Failed to write STL file: " << outputFilePath << std::endl;
    }
}
