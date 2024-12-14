#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>
#include <vtkTextProperty.h>
#include <vtkPointData.h>
#include <vtkContourFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkOpenGLRenderer.h>
#include <vtkClipPolyData.h>
#include <vtkPlane.h>
#include <vtkSTLWriter.h>
#include <vtkClipDataSet.h>
#include <vtkGeometryFilter.h>
#include <vtkAppendFilter.h>


#include <vtkThreshold.h>
#include <vtkDoubleArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkPolyDataWriter.h>
#include <vtkWarpVector.h>
#include <vtkPolyDataNormals.h>
#include <vtkCleanPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkReverseSense.h>

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <limits> // for std::numeric_limits


#include <iostream>

// 関数宣言
bool LoadAndPrepareData(const char* filename, vtkSmartPointer<vtkUnstructuredGrid>& data, vtkSmartPointer<vtkLookupTable>& lookupTable, double scalarRange[2]);
vtkSmartPointer<vtkPolyData> ReadSTL(const std::string& file_path);
void StlDisplay(vtkSmartPointer<vtkPolyData> polyData, vtkSmartPointer<vtkRenderer> renderer);
void StartRnederAndInteraction(vtkSmartPointer<vtkRenderWindow> renderWindow, vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor);
void ContourDisplay(vtkSmartPointer<vtkUnstructuredGrid> data, vtkSmartPointer<vtkContourFilter> contourFilter, vtkSmartPointer<vtkLookupTable> lookupTable, double scalarRange[2],vtkSmartPointer<vtkRenderer> renderer);
void StressDisplay(vtkSmartPointer<vtkUnstructuredGrid> data, vtkSmartPointer<vtkLookupTable> lookupTable, double scalarRange[2], vtkSmartPointer<vtkRenderer> renderer);
void SavePolyDataAsSTL(vtkPolyData* polyData, const std::string& fileName);
vtkSmartPointer<vtkPolyData> ScalePolyData(
    vtkSmartPointer<vtkPolyData> inputPolyData, 
    double scaleFactor);
vtkSmartPointer<vtkPolyData> ReversePolyDataOrientation(vtkSmartPointer<vtkPolyData> inputPolyData);
vtkSmartPointer<vtkPolyData> getDifferenceData(
    vtkSmartPointer<vtkPolyData> minuend, vtkSmartPointer<vtkPolyData> subtrahend);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <filename.vtu> <outer_boundary.stl>" << std::endl;
        return EXIT_FAILURE;
    }
    
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    auto renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    auto stldata = ReadSTL(argv[2]);
    //StlDisplay(stldata, renderer);

    vtkSmartPointer<vtkUnstructuredGrid> data;
    vtkSmartPointer<vtkLookupTable> lookupTable;
    double scalarRange[2];
    if (!LoadAndPrepareData(argv[1], data, lookupTable, scalarRange)) {
        std::cerr << "Failed to load and prepare the VTK data." << std::endl;
        return EXIT_FAILURE;
    }
    

    auto contourFilter = vtkSmartPointer<vtkContourFilter>::New();
    contourFilter->SetInputData(data);
    int numContours = 3; // 等値面の数を増加
    contourFilter->GenerateValues(numContours, scalarRange);

    contourFilter->SetInputData(data);
    contourFilter->Update();
    auto isoSurface = contourFilter->GetOutput();
    
    auto expandedSurface = ScalePolyData(isoSurface, 1.2);
    auto expandedSurfaceRev = ReversePolyDataOrientation(expandedSurface);

    StressDisplay(data, lookupTable, scalarRange, renderer);
    StlDisplay(isoSurface, renderer);

    auto resultPolyData = getDifferenceData(stldata, expandedSurface);
    auto resultPolyData2 = getDifferenceData(stldata, expandedSurfaceRev);

    std::string fileName = "result.stl";
    std::string fileName2 ="result_rev.stl";

    SavePolyDataAsSTL(resultPolyData, fileName);
    SavePolyDataAsSTL(resultPolyData2, fileName2);
    StartRnederAndInteraction(renderWindow, renderWindowInteractor);

}


bool LoadAndPrepareData(const char* filename, vtkSmartPointer<vtkUnstructuredGrid>& data, vtkSmartPointer<vtkLookupTable>& lookupTable, double scalarRange[2]) {
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

    std::cout<<"range[0]"<<scalarRange[0]<<"range[1]"<<scalarRange[1]<<std::endl;
    // スカラー範囲を取得
    data->GetScalarRange(scalarRange);
    std::cout<<"range[0]"<<scalarRange[0]<<"range[1]"<<scalarRange[1]<<std::endl;

    // LookupTableの作成
    lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetNumberOfTableValues(256);
    lookupTable->SetRange(scalarRange);
    lookupTable->SetHueRange(0.6667, 0.0); // 青から赤へ
    lookupTable->Build();

    return true;
}


void StressDisplay(vtkSmartPointer<vtkUnstructuredGrid> data, vtkSmartPointer<vtkLookupTable> lookupTable, double scalarRange[2], vtkSmartPointer<vtkRenderer> renderer)
{
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



void StartRnederAndInteraction(vtkSmartPointer<vtkRenderWindow> renderWindow, vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor){
    // レンダリングとインタラクションの開始
    renderWindow->Render();
    renderWindowInteractor->Start();
}


// STLファイルを読み込んでvtkPolyDataオブジェクトを返す関数
vtkSmartPointer<vtkPolyData> ReadSTL(const std::string& file_path) {
    // STLリーダーを作成
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(file_path.c_str());
    // 読み込みを実行
    reader->Update();
    // ポリゴンデータを取得
    return reader->GetOutput();
}



void StlDisplay(vtkSmartPointer<vtkPolyData> polyData, vtkSmartPointer<vtkRenderer> renderer)
{
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


void SavePolyDataAsSTL(vtkPolyData* polyData, const std::string& fileName)
{
    // vtkSmartPointerを用いてインスタンス生成
    vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
    
    // 出力ファイル名の設定
    writer->SetFileName(fileName.c_str());
    
    // 書き込むvtkPolyDataを設定
    writer->SetInputData(polyData);
    
    // 書き出し実行
    if(!writer->Write())
    {
        // 書き込みに失敗した場合のエラーハンドリング
        std::cerr << "Error: Failed to write STL file: " << fileName << std::endl;
    }
}


vtkSmartPointer<vtkPolyData> ScalePolyData(
    vtkSmartPointer<vtkPolyData> inputPolyData, 
    double scaleFactor)
{
    // Transform を作成して拡大係数を設定
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->Scale(scaleFactor, scaleFactor, scaleFactor);

    // TransformPolyDataFilter を使用して変換を適用
    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData(inputPolyData);
    transformFilter->SetTransform(transform);
    transformFilter->Update();

    // 拡大された PolyData を返す
    return transformFilter->GetOutput();
}


vtkSmartPointer<vtkPolyData> ReversePolyDataOrientation(vtkSmartPointer<vtkPolyData> inputPolyData)
{
    // vtkReverseSenseフィルタのインスタンスを生成
    vtkSmartPointer<vtkReverseSense> reverseSenseFilter = vtkSmartPointer<vtkReverseSense>::New();
    reverseSenseFilter->SetInputData(inputPolyData);

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

vtkSmartPointer<vtkPolyData> getDifferenceData(
    vtkSmartPointer<vtkPolyData> minuend, vtkSmartPointer<vtkPolyData> subtrahend)
{
    auto booleanFilter1 = vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
    booleanFilter1->SetOperationToDifference();
    booleanFilter1->SetInputData(0, minuend);
    booleanFilter1->SetInputData(1, subtrahend);
    booleanFilter1->Update();
    auto outputData = vtkSmartPointer<vtkPolyData>::New();
    outputData->DeepCopy(booleanFilter1->GetOutput());
    return outputData;
}