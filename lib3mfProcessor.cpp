#include "lib3mfProcessor.h"

#include <iostream>
#include <filesystem>

bool Lib3mfProcessor::getMeshes(int divideMeshNum){
    for (int i=0; i<divideMeshNum; ++i){
        std::string stlFileName = ".temp/div/" + std::to_string(i) + ".stl";
        getStl(stlFileName);
    }
    return 0;
}

bool Lib3mfProcessor::getStl(const std::string stlFileName){
    std::string output_file = stlFileName + ".3mf";

    // Import Model from File
    std::cout << "reading " << stlFileName << "..." << std::endl;
    reader->ReadFromFile(stlFileName);

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
    return 0;
}

bool Lib3mfProcessor::save3mf(const std::string outputFilename){
    PWriter writer = model->QueryWriter("3mf");
    std::cout << "Writing " << outputFilename << "..." << std::endl;
    writer->WriteToFile(outputFilename);
    std::cout << "Done" << std::endl;
    return 0;
}