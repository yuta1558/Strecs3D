#include "cgalProcessor.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
namespace fs = std::filesystem;

bool CGALProcessor::getFileNames(){
// 読み込み対象のディレクトリ (例: ".temp")
    fs::path directoryPath{".temp/iso"};

    // (数値, ファイルパス) をセットで一時的に保持するベクタ
    std::vector<std::pair<int, fs::path>> stlFilesTemp;

    // ディレクトリ内を走査
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        // 通常のファイルのみを対象とし、拡張子が「.stl」か確認
        if (entry.is_regular_file() && entry.path().extension() == ".stl") {
            // 拡張子を除いた部分(= stem)を取得 (例: "10.stl" → "10")
            std::string stem = entry.path().stem().string();

            // 数値に変換（例外や数値以外が混在する場合のエラーチェックは省略）
            int num = std::stoi(stem);

            // (数値, パス) のペアを一時ベクタに格納
            stlFilesTemp.push_back({ num, entry.path() });
        }
    }

    // 数値を基準に昇順ソート
    std::sort(stlFilesTemp.begin(), stlFilesTemp.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        }
    );

    // ソート後の結果をパス文字列として格納するベクタ
    //std::vector<std::string> stlFiles;
    CGALProcessor::stlFiles.reserve(stlFilesTemp.size());
    for (const auto& [num, path] : stlFilesTemp) {
        CGALProcessor::stlFiles.push_back(path.string());
    }

    // 確認用：結果の一覧を表示
    for (const auto& file : CGALProcessor::stlFiles) {
        std::cout << file << std::endl;
    }

    return 0;
}


bool CGALProcessor::prepareMeshes(const std::string& outlineFileName){
    for (int i = 0; i < this->stlFiles.size(); ++i) {
        Mesh isoMesh;
        if(!this->readStl(this->stlFiles[i], isoMesh)) return 1;
        this->isoMeshes.push_back(isoMesh);
        PMP::reverse_face_orientations(isoMesh); 
        this->isoMeshesRev.push_back(isoMesh);  //反転したmeshをベクトルに追加
    }
    this->isoSurfaceNum = this->isoMeshes.size();

    if(!this->readStl(outlineFileName, this->outlineMesh)) return 1;
    
    return 0;
}

bool CGALProcessor::divideMeshes(){
    for (int i = 0; i < isoSurfaceNum+1; ++i) {
        //差分演算
        Mesh diff_mesh;
        Mesh outlineMeshCopy = this->outlineMesh;
        bool success;
        if(i==0){
            success = PMP::corefine_and_compute_difference(outlineMeshCopy,isoMeshes[i], diff_mesh);
        } 
        else if(i==isoSurfaceNum){
            success = PMP::corefine_and_compute_difference(outlineMeshCopy, isoMeshesRev[i-1], diff_mesh);
        }
        else{
            Mesh diffMesh1;
            PMP::corefine_and_compute_difference(outlineMeshCopy, isoMeshesRev[i-1], diffMesh1);
            success = PMP::corefine_and_compute_difference(diffMesh1, isoMeshes[i], diff_mesh);
        }
        if(!success) {
            std::cerr << "Error: corefine_and_compute_difference failed." << std::endl;
            return 1;
        }
        std::string stl_output_file = ".temp/div/" + std::to_string(i) + ".stl";
        // 書き出し
        if(!writeStl(stl_output_file, diff_mesh)) {
            std::cerr << "Error: failed to write STL to " << stl_output_file << std::endl;
            return 1;
        }
    }
    return 0;
}

bool CGALProcessor::readStl(const std::string& filename, Mesh& mesh)
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
bool CGALProcessor::writeStl(const std::string& filename, const Mesh& mesh)
{
    if(!CGAL::IO::write_STL(filename, mesh)) {
        std::cerr << "Error: failed to write STL file " << filename << std::endl;
        return false;
    }

    return true;
}

void CGALProcessor::printMeshInfo(const Mesh& mesh)
{
    std::cout << "Number of vertices   : " << mesh.number_of_vertices()   << std::endl;
    std::cout << "Number of edges      : " << mesh.number_of_edges()      << std::endl;
    std::cout << "Number of halfedges  : " << mesh.number_of_halfedges()  << std::endl;
    std::cout << "Number of faces      : " << mesh.number_of_faces()      << std::endl;
}