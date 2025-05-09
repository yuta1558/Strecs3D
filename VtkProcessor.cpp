#include "VtkProcessor.h"
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>

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

void VtkProcessor::prepareStressValues(){
    int divisionNum = 5;
    double stressInterval = (maxStress - minStress) / divisionNum;
    for (int i=0; i<=divisionNum; ++i){
        stressValues.push_back(minStress + i*stressInterval);
    }
    isoSurfaceNum = stressValues.size();
    std::cout<< "isoSurfaceNum: " << isoSurfaceNum << std::endl;
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
    actor->GetProperty()->SetOpacity(0.8); // 透明度80%
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
    actor->GetProperty()->SetOpacity(0.8); // 透明度80%
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
