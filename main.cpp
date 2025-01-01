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
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkFillHolesFilter.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/IO/STL.h>


#include "lib3mf_implicit.hpp"
using namespace Lib3MF;

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <limits> // for std::numeric_limits
#include <iostream>


namespace PMP = CGAL::Polygon_mesh_processing;
using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Mesh   = CGAL::Surface_mesh<Kernel::Point_3>;

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
std::array<double, 3> ComputeMeshCenter(vtkSmartPointer<vtkPolyData> polyData);
bool read_stl(const std::string& filename, Mesh& mesh);
bool write_stl(const std::string& filename, const Mesh& mesh);

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

    vtkSmartPointer<vtkUnstructuredGrid> data;
    vtkSmartPointer<vtkLookupTable> lookupTable;
    double scalarRange[2];
    if (!LoadAndPrepareData(argv[1], data, lookupTable, scalarRange)) {
        std::cerr << "Failed to load and prepare the VTK data." << std::endl;
        return EXIT_FAILURE;
    }
    
    auto contourFilter = vtkSmartPointer<vtkContourFilter>::New();
    contourFilter->SetInputData(data);
    int isoSurfaceNum =  3; // 等値面の数
    float minStress = scalarRange[0];
    float maxStress = scalarRange[1];
    std::vector<vtkSmartPointer<vtkPolyData>> isoSurfaces;

    for (int i = 1; i < isoSurfaceNum+1; ++i) {
        // 等値面の生成
        double isoValue = minStress + i*(maxStress - minStress)/(isoSurfaceNum+1);
        std::cout<<isoValue<<std::endl;
        auto singleContourFilter = vtkSmartPointer<vtkContourFilter>::New();
        singleContourFilter->SetInputData(data);
        singleContourFilter->SetValue(0, isoValue);
        singleContourFilter->Update();
        auto singleIsoSurface = vtkSmartPointer<vtkPolyData>::New();
        singleIsoSurface->DeepCopy(singleContourFilter->GetOutput());
        isoSurfaces.push_back(singleIsoSurface);
    }
    
    for (int i = 0; i < isoSurfaceNum; ++i) {
        std::array<double, 3> center = ComputeMeshCenter(isoSurfaces[i]);
        std::cout << "Center: " << center[0] << ", " << center[1] << ", " << center[2] << std::endl;

        auto transformToOrigin = vtkSmartPointer<vtkTransform>::New();
        transformToOrigin->Translate(-center[0], -center[1], -center[2]); // x, y, z軸方向に移動
        // vtkTransformPolyDataFilterを使用して変換を適用
        auto transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        transformFilter->SetInputData(isoSurfaces[i]);
        transformFilter->SetTransform(transformToOrigin);
        transformFilter->Update();

        auto originPolyData = transformFilter->GetOutput();

        //原点中心で少し拡大
        auto expandedSurfaceOrigin = ScalePolyData(originPolyData, 1.01);
        //元の位置に戻す
        auto transformBack = vtkSmartPointer<vtkTransform>::New();
        transformBack->Translate(center[0], center[1], center[2]); // x, y, z軸方向に移動

        // vtkTransformPolyDataFilterを使用して変換を適用
        auto transformFilterBack = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        transformFilterBack->SetInputData(expandedSurfaceOrigin);
        transformFilterBack->SetTransform(transformBack);
        transformFilterBack->Update();
        auto expandedSurfaceCombined = transformFilterBack->GetOutput();

        // スムージング処理
        auto smoother = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
        smoother->SetInputData(expandedSurfaceCombined);
        smoother->SetNumberOfIterations(20); // スムージングの繰り返し回数
        smoother->SetRelaxationFactor(0.1);
        smoother->FeatureEdgeSmoothingOff();
        smoother->BoundarySmoothingOn();
        smoother->Update();

        auto expandedSurface = smoother->GetOutput();
        auto expandedSurfaceRev = ReversePolyDataOrientation(expandedSurface);
        
        std::string fileName = "isoSurface" + std::to_string(i) + ".stl";
        SavePolyDataAsSTL(expandedSurfaceRev, fileName);
    }
    StlDisplay(isoSurfaces[0], renderer);

    std::vector<Mesh> isoMeshes;
    std::vector<Mesh> isoMeshesRev;
    for (int i = 0; i < isoSurfaceNum; ++i) {
        Mesh isoMesh;
        if(!read_stl("isoSurface" + std::to_string(i) + ".stl", isoMesh)) return 1;
        isoMeshes.push_back(isoMesh);
        PMP::reverse_face_orientations(isoMesh); 
        isoMeshesRev.push_back(isoMesh);  //反転したmeshをベクトルに追加
    }

    Mesh outlineMesh;
    if(!read_stl(argv[2], outlineMesh)) return 1;
    for (int i = 0; i < isoSurfaceNum+1; ++i) {
        //差分演算
        Mesh diff_mesh;
        Mesh outlineMeshCopy = outlineMesh;
        bool success;
        if(i==0){
            success = PMP::corefine_and_compute_difference(outlineMeshCopy, isoMeshesRev[i], diff_mesh);
        } 
        else if(i==isoSurfaceNum){
            success = PMP::corefine_and_compute_difference(outlineMeshCopy,isoMeshes[i-1], diff_mesh);
        }
        else{
            Mesh diffMesh1;
            PMP::corefine_and_compute_difference(outlineMeshCopy, isoMeshes[i-1], diffMesh1);
            success = PMP::corefine_and_compute_difference(diffMesh1, isoMeshesRev[i], diff_mesh);
        }

        if(!success) {
            std::cerr << "Error: corefine_and_compute_difference failed." << std::endl;
            return 1;
        }
        std::string stl_output_file = std::to_string(i) + ".stl";
        // 書き出し
        if(!write_stl(stl_output_file, diff_mesh)) {
            std::cerr << "Error: failed to write STL to " << stl_output_file << std::endl;
            return 1;
        }
    }
    std::string sReaderName = "stl";
    std::string sWriterName = "3mf";
    PWrapper wrapper = CWrapper::loadLibrary();
    PModel model = wrapper->CreateModel();
    PReader reader = model->QueryReader(sReaderName);


    for (int i = 0; i < isoSurfaceNum+1; ++i) {

        std::string stl_output_file = std::to_string(i) + ".stl";
        std::string output_file = "diff_result" + std::to_string(i) + ".3mf";

        // Import Model from File
        std::cout << "reading " << stl_output_file << "..." << std::endl;
        reader->ReadFromFile(stl_output_file);

        auto meshIterator = model->GetMeshObjects();
        // メッシュの総数を取得
        size_t meshCount = meshIterator->Count();
        // メッシュが存在しない場合はエラーメッセージを出力して終了
        if (meshCount == 0) {
            std::cerr << "No mesh objects found in the model." << std::endl;
        }
        // 最後のメッシュのインデックスを計算
        size_t lastIndex = meshCount - 1;
        size_t currentIndex = 0;      // 現在のインデックスを初期化
        Lib3MF_uint32 lastMeshID = 0; // 最後のメッシュのIDを格納する変数を初期化

        // イテレータを使用してメッシュを順に処理
        while (meshIterator->MoveNext()) {
            // 現在のインデックスが最後のインデックスに達したら
            if (currentIndex == lastIndex) {
                auto currentMesh = meshIterator->GetCurrent();      // 現在のメッシュを取得
                lastMeshID = currentMesh->GetResourceID();          // メッシュのリソースIDを取得
                break;                                              // ループを抜ける
            }
            currentIndex++; // インデックスをインクリメント
        }

        // 有効なメッシュIDが取得できた場合
        if (lastMeshID != 0) {
            // メッシュIDを使用してメッシュオブジェクトを取得
            auto lastMesh = model->GetMeshObjectByID(lastMeshID);
            std::__fs::filesystem::path pathObj(stl_output_file);
            std::string fileName = pathObj.filename().string();
            lastMesh->SetName(fileName); // メッシュの名前を設定
        } else {
            // メッシュIDの取得に失敗した場合はエラーメッセージを出力
            std::cerr << "Failed to set name for the last mesh from file: " << stl_output_file << std::endl;
        } 
    }


     // 各メッシュオブジェクトにメタデータを追加
    auto meshIterator = model->GetMeshObjects();
    for (; meshIterator->MoveNext(); ) {
        Lib3MF::PMeshObject currentMesh = meshIterator->GetCurrentMeshObject();
        PMetaDataGroup metadataGroup = currentMesh->GetMetaDataGroup();
        auto name = currentMesh->GetName();
        size_t dot_pos = name.find_last_of('.');
        // ドットの前の部分を抽出
        std::string number_str = name.substr(0, dot_pos);
        int density = (std::stoi(number_str)+1)*10;
        std::string density_str = std::to_string(density);
        std::cout << "Object name: " << name << std::endl; // ファイル名が出力される
        std::cout<<"density: "<<density_str<<std::endl;
        std::string cura_uri = "http://software.ultimaker.com/xml/cura/3mf/2015/10";
        // メタデータの追加
        metadataGroup->AddMetaData(cura_uri, "drop_to_buildplate", "True", "xs:boolean", false);
        metadataGroup->AddMetaData(cura_uri, "infill_sparse_density", density_str, "xs:integer", false);

        // メッシュオブジェクトに対する追加処理
        std::cout << "Mesh Object UUID: " << currentMesh->GetResourceID() << std::endl;
    }
    std::string outputFilename = "merged_data.3mf";
    PWriter writer = model->QueryWriter("3mf");
    std::cout << "Writing " << outputFilename << "..." << std::endl;
    writer->WriteToFile(outputFilename);
    std::cout << "Done" << std::endl;

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



// vtkPolyDataの中心点を計算する関数
std::array<double, 3> ComputeMeshCenter(vtkSmartPointer<vtkPolyData> polyData)
{
    // 結果を格納する中心点
    std::array<double, 3> center = {0.0, 0.0, 0.0};

    // メッシュのポイントを取得
    vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
    if (!points)
    {
        throw std::runtime_error("No points in the input mesh.");
    }

    // ポイントの個数を取得
    vtkIdType numberOfPoints = points->GetNumberOfPoints();
    if (numberOfPoints == 0)
    {
        throw std::runtime_error("Mesh has no points.");
    }

    // 中心点を計算
    for (vtkIdType i = 0; i < numberOfPoints; ++i)
    {
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

bool read_stl(const std::string& filename, Mesh& mesh)
{
    std::ifstream fin(filename, std::ios::binary);
    if(!fin) {
        std::cerr << "Error: could not open file " << filename << std::endl;
        return false;
    }

    // CGAL::IO::read_STL が使用可能
    if(!CGAL::IO::read_STL(fin, mesh)) {
        std::cerr << "Error: failed to read STL file " << filename << std::endl;
        return false;
    }

    return true;
}

// STL ファイル書き出し用の簡単なラッパ
bool write_stl(const std::string& filename, const Mesh& mesh)
{
    if(!CGAL::IO::write_STL(filename, mesh)) {
        std::cerr << "Error: failed to write STL file " << filename << std::endl;
        return false;
    }

    return true;
}