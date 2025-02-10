#include "lib3mfProcessor.h"

#include <iostream>
#include <filesystem>
#include <regex>
#include <map>



// bool Lib3mfProcessor::getMeshes(){
//     std::string directoryPath = "./.temp/iso";
//     std::regex filePattern(R"(dividedMesh(\d+)_([0-9]+\.[0-9]+)_([0-9]+\.[0-9]+)\.stl)");
//     std::map<std::string, FileInfo> fileInfoMap;

//     try {
//         // ディレクトリ内の全ファイルを走査
//         for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
//             if (entry.is_regular_file()) {
//                 // ファイル名のみ取得
//                 std::string filename = entry.path().filename().string();
//                 std::smatch match;

//                 // 正規表現でファイル名をパース
//                 if (std::regex_match(filename, match, filePattern)) {
//                     FileInfo info;
//                     info.id = std::stoi(match[1].str());
//                     info.minStress = std::stod(match[2].str());
//                     info.maxStress = std::stod(match[3].str());

//                     // 辞書に登録
//                     fileInfoMap[filename] = info;
//                 }
//             }
//         }
//     } catch (const std::filesystem::filesystem_error& e) {
//         std::cerr << "ファイルシステムエラー: " << e.what() << "\n";
//     }

    
//     for (const auto& kv : fileInfoMap) {
//         std::string stlFileName = directoryPath + "/" + kv.first;
//         getStl(stlFileName);
//         std::cout << "File Name: " << kv.first << "\n"
//             << "ID: " << kv.second.id << "\n"
//             << "minStress: " << kv.second.minStress << "\n"
//             << "maxStress: " << kv.second.maxStress << "\n"
//             << "------------------------\n";
//     }
//     return 0;
// }
bool Lib3mfProcessor::getMeshes(){
    std::string directoryPath = "./.temp/iso";
    try {
        // ディレクトリ内の全ファイルを走査
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                std::cout<< "filename: " << filename << std::endl;
                getStl(directoryPath + "/" + filename);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "ファイルシステムエラー: " << e.what() << "\n";
    }
    return 0;
}


bool Lib3mfProcessor::getStl(const std::string stlFileName){

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
    for (; meshIterator->MoveNext(); ) {
        Lib3MF::PMeshObject currentMesh = meshIterator->GetCurrentMeshObject();
        PMetaDataGroup metadataGroup = currentMesh->GetMetaDataGroup();
        auto name = currentMesh->GetName();
        size_t dot_pos = name.find_last_of('.');
        std::cout << "Object name: " << name << std::endl; // ファイル名が出力される

        std::regex filePattern(R"(dividedMesh(\d+)_([0-9]+\.[0-9]+)_([0-9]+\.[0-9]+)\.stl)");
        std::smatch match;
        float minStress = 0.0;
        float maxStress = 0.0;
        if (std::regex_match(name, match, filePattern)) {
            std::string id_str = match[1].str();
            std::string minStress_str = match[2].str();
            std::string maxStress_str = match[3].str();
            std::cout << "ID: " << id_str << std::endl;
            std::cout << "minStress: " << minStress_str << std::endl;
            std::cout << "maxStress: " << maxStress_str << std::endl;
            minStress = std::stof(minStress_str);
            maxStress = std::stof(maxStress_str);
        }
        //応力の平均値から密度を計算
        float aveStress = (minStress + maxStress) / 2;
        int density =    aveStress/300000 * 100;
        std::cout << "density: " << density << std::endl;
        std::string density_str = std::to_string(density);
        std::string cura_uri = "http://software.ultimaker.com/xml/cura/3mf/2015/10";
        // メタデータの追加
        metadataGroup->AddMetaData(cura_uri, "drop_to_buildplate", "True", "xs:boolean", false);
        metadataGroup->AddMetaData(cura_uri, "infill_sparse_density", density_str, "xs:integer", false);

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