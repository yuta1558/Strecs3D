#include "lib3mfProcessor.h"

#include <iostream>
#include <filesystem>
#include <regex>
#include <vector>
#include <algorithm>
#include <map>

namespace fs = std::filesystem;


bool Lib3mfProcessor::getMeshes(){
    std::string directoryPath = "./.temp/div";
    try {
        std::vector<fs::directory_entry> files;

        // ディレクトリ内のエントリを走査し、通常ファイルの場合のみ vector に追加
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                files.push_back(entry);
            }
        }

        // ファイル名で昇順にソート
        std::sort(files.begin(), files.end(),
            [](const fs::directory_entry& a, const fs::directory_entry& b) {
                return a.path().filename().string() < b.path().filename().string();
            }
        );

        // ソート済みのファイルを処理
        for (const auto& entry : files) {
            std::string filename = entry.path().filename().string();
            std::cout << "filename: " << filename << std::endl;
            // ここで setStl 関数を呼び出す（directoryPath と filename を結合）
            setStl(directoryPath + "/" + filename);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "file system error: " << e.what() << "\n";
    }
    return 0;
}


bool Lib3mfProcessor::setStl(const std::string stlFileName){

    // Import Model from File
    std::cout << "reading " << stlFileName << "..." << std::endl;
    try{
        reader->ReadFromFile(stlFileName);
    } catch (Lib3MF::ELib3MFException &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    auto meshIterator = model->GetMeshObjects();
    // メッシュの総数を取得
    size_t meshCount = meshIterator->Count();
    std::cout << "Mesh count: " << meshCount << std::endl;
    //メッシュが存在しない場合はエラーメッセージを出力して終了
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

    //有効なメッシュIDが取得できた場合
    if (lastMeshID != 0) {
        // メッシュIDを使用してメッシュオブジェクトを取得
        auto lastMesh = model->GetMeshObjectByID(lastMeshID);
        std::filesystem::path pathObj(stlFileName);
        std::string fileName = pathObj.filename().string();
        lastMesh->SetName(fileName); // メッシュの名前を設定
    } else {
        // メッシュIDの取得に失敗した場合はエラーメッセージを出力
        std::cerr << "Failed to set name for the last mesh from file: " << stlFileName << std::endl;
    } 
    return 0;
}

bool Lib3mfProcessor::setMetaData(){
    auto meshIterator = model->GetMeshObjects();
    std::regex filePattern(R"(dividedMesh(\d+)_([0-9]+\.[0-9]+)_([0-9]+\.[0-9]+)\.stl)");
    for (; meshIterator->MoveNext(); ) {
        Lib3MF::PMeshObject currentMesh = meshIterator->GetCurrentMeshObject();
        auto name = currentMesh->GetName();
        std::map<std::string, FileInfo> fileInfoMap;
        std::smatch match;
        if (std::regex_match(name, match, filePattern)) {
            std::string id_str = match[1].str();
            std::string minStress_str = match[2].str();
            std::string maxStress_str = match[3].str();
            std::cout << "Object name: " << name << std::endl;
            std::cout << "ID: " << id_str << std::endl;
            std::cout << "minStress: " << minStress_str << std::endl;
            std::cout << "maxStress: " << maxStress_str << std::endl;
            std::cout << "----------------------" << std::endl;  
            FileInfo fileInfo;
            fileInfo.id = std::stoi(id_str);
            fileInfo.name = name;
            fileInfo.minStress = std::stod(minStress_str);
            fileInfo.maxStress = std::stod(maxStress_str);
            fileInfoMap[name] = fileInfo;
            setMetaDataForInfillMesh(currentMesh, fileInfoMap[name]);
        }
        else{
            std::cerr << "Process Outline mesh" << std::endl;
            setMetaDataForOutlineMesh(currentMesh);
        }
    }
    return 0;
}


bool Lib3mfProcessor::setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo){
    std::string cura_uri = "http://software.ultimaker.com/xml/cura/3mf/2015/10";
    PMetaDataGroup metadataGroup = Mesh->GetMetaDataGroup();
    double aveStress = (fileInfo.minStress + fileInfo.maxStress) / 2;
    int density = aveStress / 300000 * 100;
    std::string density_str = std::to_string(density);
    metadataGroup->AddMetaData(cura_uri, "drop_to_buildplate", "False", "xs:boolean", false);
    metadataGroup->AddMetaData(cura_uri, "infill_mesh", "True", "xs:boolean", false);
    metadataGroup->AddMetaData(cura_uri, "infill_sparse_density", density_str, "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "wall_line_count", "0","xs:integer", false );
    metadataGroup->AddMetaData(cura_uri, "wall_thickness", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "bottom_layers", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "bottom_thickness", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "top_bottom_thickness", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "top_layers", "0", "xs:integer", false);
    metadataGroup->AddMetaData(cura_uri, "top_thickness", "0", "xs:integer", false); 
    return 0;
}

bool Lib3mfProcessor::setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh){
    std::string cura_uri = "http://software.ultimaker.com/xml/cura/3mf/2015/10";
    PMetaDataGroup metadataGroup = Mesh->GetMetaDataGroup();
    metadataGroup->AddMetaData(cura_uri, "drop_to_buildplate", "True", "xs:boolean", false);
    return 0;
}

bool Lib3mfProcessor::assembleObjects(){
    sTransform identityTransform;
    auto transform = lib3mf_getidentitytransform(&identityTransform);
    auto mergedObject = model->AddComponentsObject();
    auto meshIterator = model->GetMeshObjects();
    int meshCount = meshIterator->Count();
    for (int i = 0; i < meshCount; i++) {
        meshIterator->MoveNext();
        auto currentMesh = meshIterator->GetCurrentMeshObject();
        mergedObject->AddComponent(currentMesh.get(), identityTransform);
    }
    auto metadataGroup = mergedObject->GetMetaDataGroup();
    mergedObject->SetName("Group #1");
    std::string cura_uri = "http://software.ultimaker.com/xml/cura/3mf/2015/10";
    metadataGroup->AddMetaData(cura_uri, "drop_to_buildplate", "True", "xs:boolean", false);
    metadataGroup->AddMetaData(cura_uri, "print_order", "1", "xs:integer", false);

    //buildオブジェクトのすべてのメッシュを削除して、mergedオブジェクトを追加
    auto buildItemIterator = model->GetBuildItems();
    for (int i = 0; i < buildItemIterator->Count(); i++) {
        buildItemIterator->MoveNext();
        auto buildItem = buildItemIterator->GetCurrent();
        model->RemoveBuildItem(buildItem);
    }
    
    model->AddBuildItem(mergedObject.get(), identityTransform);
    return 0;
}


bool Lib3mfProcessor::save3mf(const std::string outputFilename){
    PWriter writer = model->QueryWriter("3mf");
    std::cout << "Writing " << outputFilename << "..." << std::endl;
    writer->WriteToFile(outputFilename);
    std::cout << "Done" << std::endl;
    return 0;
}