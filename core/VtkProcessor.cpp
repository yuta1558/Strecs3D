#include "VtkProcessor.h"
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <QColor>
#include <algorithm>

VtkProcessor::VtkProcessor(const std::string& vtuFileName): vtuFileName(vtuFileName) {
    // renderWindow->AddRenderer(renderer);
    // renderWindowInteractor->SetRenderWindow(renderWindow);
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

    minStress = stressRange[0];
    maxStress = stressRange[1];
    return true;
}

vtkSmartPointer<vtkPolyData> VtkProcessor::extractRegionInRange(double lowerBound, double upperBound){

    vtkSmartPointer<vtkClipDataSet> clip_min = vtkSmartPointer<vtkClipDataSet>::New();
    clip_min->SetInputData(vtuData);
    clip_min->SetValue(lowerBound);
    clip_min->SetInsideOut(false);  // min_val より大きい領域を保持
    clip_min->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "von Mises Stress");
    clip_min->Update();

    // 3. クリップフィルタ2: max_val 以下の領域を抽出
    vtkSmartPointer<vtkClipDataSet> clip_max = vtkSmartPointer<vtkClipDataSet>::New();
    clip_max->SetInputConnection(clip_min->GetOutputPort());
    clip_max->SetValue(upperBound);
    clip_max->SetInsideOut(true);  // max_val より小さい領域を保持
    clip_max->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "von Mises Stress");
    clip_max->Update();

    // 最終的な結果: min_val と max_val の間の値を持つ領域
    vtkSmartPointer<vtkUnstructuredGrid> ug_range = clip_max->GetOutput();  
    vtkSmartPointer<vtkGeometryFilter> geometry_filter = vtkSmartPointer<vtkGeometryFilter>::New();
    geometry_filter->SetInputData(ug_range);    
    geometry_filter->Update();
    vtkSmartPointer<vtkPolyData> poly_data = geometry_filter->GetOutput();
    
    return poly_data;
}   

std::vector<vtkSmartPointer<vtkPolyData>> VtkProcessor::divideMesh() {
    std::vector<vtkSmartPointer<vtkPolyData>> dividedPolyData;

    for (int i = 0; i < isoSurfaceNum - 1; ++i) {
        double minValue = stressValues[i];
        double maxValue = stressValues[i + 1];
        std::cout << "Extracting cells in range: " << minValue << " -> " << maxValue << std::endl;
        vtkSmartPointer<vtkPolyData> currentPolyData = this->extractRegionInRange(minValue, maxValue);
        
        dividedPolyData.push_back(currentPolyData);
    }

    return dividedPolyData;
}

void VtkProcessor::clearPreviousData(){
    stressValues.clear();
    dividedMeshes.clear();
}

void VtkProcessor::prepareStressValues(const std::vector<double>& thresholds) {
    stressValues.clear();
    for (double v : thresholds) {
        stressValues.push_back(static_cast<float>(v));
    }
    isoSurfaceNum = stressValues.size();
    std::cout << "isoSurfaceNum: " << isoSurfaceNum << std::endl;
}

void VtkProcessor::savePolyDataAsSTL(vtkPolyData* polyData, const std::string& fileName) {
    std::filesystem::path currentPath = std::filesystem::current_path();
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
    
    vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();

    // 出力先を設定 (string()でstd::stringに変換、c_str()でconst char*に変換)
    writer->SetFileName(outputFilePath.string().c_str());
    writer->SetInputData(polyData);
    writer->SetFileTypeToBinary();

    if (!writer->Write()) {
        std::cerr << "Error: Failed to write STL file: " << outputFilePath << std::endl;
    }
}

vtkSmartPointer<vtkActor> VtkProcessor::getVtuActor(const std::string& fileName){
    // VTKファイルの読み込み
    vtkSmartPointer<vtkXMLUnstructuredGridReader> reader =
    vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->Update();

    // 読み込んだデータセットを取得
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid =
    vtuData = reader->GetOutput();
    if (!unstructuredGrid){
    std::cerr << "Error: Unable to read the VTK file." << std::endl;
        return nullptr;
    }
    if (!vtuData) {
        std::cerr << "Error: Unable to read the VTK file." << std::endl;
        return nullptr;
    }

    // "von Mises Stress" をアクティブスカラーとして設定
    vtkPointData* pointData = unstructuredGrid->GetPointData();
    if (!pointData){
    std::cerr << "Error: No point data found in the file." << std::endl;
        return nullptr;
    }
    pointData->SetActiveScalars("von Mises Stress");

    // ストレスのレンジを取得
    double stressRange[2];
    unstructuredGrid->GetScalarRange(stressRange);
    minStress = stressRange[0];
    maxStress = stressRange[1];

    // LookupTableの作成（ColorManagerで指定された色のグラデーション）
    vtkSmartPointer<vtkLookupTable> lookupTable =
    vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetNumberOfTableValues(256);
    lookupTable->SetRange(stressRange);
    
    // ColorManagerの色を使用
    lookupTable->SetTableRange(stressRange);
    lookupTable->SetHueRange(0.0, 0.0); // Hueは使用しない
    lookupTable->SetSaturationRange(0.0, 0.0); // Saturationは使用しない
    lookupTable->SetValueRange(1.0, 1.0); // 明度を1に固定
    lookupTable->SetAlphaRange(1.0, 1.0); // 透明度を1に固定
    
    // ColorManagerの色を取得
    QColor lowColor = ColorManager::LOW_COLOR;
    QColor middleColor = ColorManager::MIDDLE_COLOR;
    QColor highColor = ColorManager::HIGH_COLOR;
    
    // カラーテーブルを手動で設定
    for (int i = 0; i < 256; i++) {
        double t = static_cast<double>(i) / 255.0;
        double r, g, b;
        
        if (t < 0.5) {
            // LOW_COLORからMIDDLE_COLORへのグラデーション
            t = t * 2.0; // 0.0-0.5 を 0.0-1.0 に変換
            r = lowColor.redF() + (middleColor.redF() - lowColor.redF()) * t;
            g = lowColor.greenF() + (middleColor.greenF() - lowColor.greenF()) * t;
            b = lowColor.blueF() + (middleColor.blueF() - lowColor.blueF()) * t;
        } else {
            // MIDDLE_COLORからHIGH_COLORへのグラデーション
            t = (t - 0.5) * 2.0; // 0.5-1.0 を 0.0-1.0 に変換
            r = middleColor.redF() + (highColor.redF() - middleColor.redF()) * t;
            g = middleColor.greenF() + (highColor.greenF() - middleColor.greenF()) * t;
            b = middleColor.blueF() + (highColor.blueF() - middleColor.blueF()) * t;
        }
        
        lookupTable->SetTableValue(i, r, g, b, 1.0);
    }
    
    lookupTable->Build();

    // メンバー変数に保存
    currentLookupTable = lookupTable;

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
    actor->GetProperty()->SetOpacity(1.0); // 不透明に設定
    actor->GetProperty()->SetEdgeVisibility(1); // エッジを表示
    actor->GetProperty()->SetEdgeColor(0.1, 0.1, 0.1); // エッジの色を黒に設定
    actor->GetProperty()->SetLineWidth(1.0); // エッジの線の太さを設定

    return actor;
}


vtkSmartPointer<vtkActor> VtkProcessor::getStlActor(const std::string& fileName){

    // STLファイルの読み込み
    vtkSmartPointer<vtkSTLReader> reader =
        vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->Update();

    // 読み込んだデータセットを取得
    vtkSmartPointer<vtkPolyData> polyData = reader->GetOutput();
    if (!polyData)
    {
        std::cerr << "Error: Unable to read the STL file." << std::endl;
        return nullptr;
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
    actor->GetProperty()->SetEdgeVisibility(1); // エッジを表示
    actor->GetProperty()->SetEdgeColor(0.1, 0.1, 0.1); // エッジの色を黒に設定
    actor->GetProperty()->SetLineWidth(1.0); // エッジの線の太さを設定
    return actor;
}

vtkSmartPointer<vtkActor> VtkProcessor::getColoredStlActor(const std::string& fileName, double r, double g, double b) {
    // STLファイルの読み込み
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->Update();

    // 読み込んだデータセットを取得
    vtkSmartPointer<vtkPolyData> polyData = reader->GetOutput();
    if (!polyData) {
        std::cerr << "Error: Unable to read the STL file." << std::endl;
        return nullptr;
    }

    // Mapperの作成
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    mapper->ScalarVisibilityOff(); // STLファイルは通常スカラー値を持たないため

    // Actorの作成
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(r, g, b); // 指定された色を設定
    actor->GetProperty()->SetOpacity(1.0); // 透明度100%（完全不透明）
    return actor;
}

// DensitySliderと同じ色計算ロジックを使用する関数
QColor getGradientColorByStress(double t) {
    // グラデーションストップの定義（DensitySliderと同じ）
    static const struct GradStop {
        double pos;
        QColor color;
    } gradStops[] = {
        {0.0, ColorManager::HIGH_COLOR},   // 赤
        {0.5, ColorManager::MIDDLE_COLOR}, // 白
        {1.0, ColorManager::LOW_COLOR}     // 青
    };
    
    if (t <= gradStops[0].pos) return gradStops[0].color;
    if (t >= gradStops[2].pos) return gradStops[2].color;
    for (int i = 0; i < 2; ++i) {
        if (t >= gradStops[i].pos && t <= gradStops[i+1].pos) {
            double localT = (t - gradStops[i].pos) / (gradStops[i+1].pos - gradStops[i].pos);
            QColor c1 = gradStops[i].color;
            QColor c2 = gradStops[i+1].color;
            int r = c1.red()   + (c2.red()   - c1.red())   * localT;
            int g = c1.green() + (c2.green() - c1.green()) * localT;
            int b = c1.blue()  + (c2.blue()  - c1.blue())  * localT;
            return QColor(r, g, b);
        }
    }
    return QColor(); // fallback
}

vtkSmartPointer<vtkActor> VtkProcessor::getColoredStlActorByStress(const std::string& fileName, double stressValue, double minStress, double maxStress) {
    // STLファイルの読み込み
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->Update();

    // 読み込んだデータセットを取得
    vtkSmartPointer<vtkPolyData> polyData = reader->GetOutput();
    if (!polyData) {
        std::cerr << "Error: Unable to read the STL file." << std::endl;
        return nullptr;
    }

    // ストレス値を0.0〜1.0に正規化（DensitySliderと同じ計算）
    // 高いストレス値をt=0.0（赤）、低いストレス値をt=1.0（青）にする
    double t = (maxStress - stressValue) / (maxStress - minStress);
    t = std::clamp(t, 0.0, 1.0); // 範囲を制限
    
    // DensitySliderと同じ色計算を使用
    QColor regionColor = getGradientColorByStress(t);
    
    // Mapperの作成
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    mapper->ScalarVisibilityOff(); // STLファイルは通常スカラー値を持たないため

    // Actorの作成
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(regionColor.redF(), regionColor.greenF(), regionColor.blueF()); // 計算された色を設定
    actor->GetProperty()->SetEdgeVisibility(1); // エッジを表示
    actor->GetProperty()->SetEdgeColor(0.1, 0.1, 0.1); // エッジの色を黒に設定
    actor->GetProperty()->SetLineWidth(1.0); // エッジの線の太さを設定
    return actor;
}

void VtkProcessor::saveDividedMeshes(const std::vector<vtkSmartPointer<vtkPolyData>>& dividedMeshes)
{
    const auto& stressValues = this->getStressValues();
    
    for (size_t i = 0; i < dividedMeshes.size(); ++i) {
        float minValue = stressValues[i];
        float maxValue = stressValues[i + 1];
        std::string fileName = generateMeshFileName(i + 1, minValue, maxValue);
        this->savePolyDataAsSTL(dividedMeshes[i], fileName);
    }
}

std::string VtkProcessor::generateMeshFileName(int index,
    float minValue,
    float maxValue) const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "dividedMesh"
    << std::setw(2) << std::setfill('0') << index << "_"
    << minValue << "_"
    << maxValue << ".stl";

    return oss.str();
}
