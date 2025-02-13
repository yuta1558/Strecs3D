#include "VtkProcessor.h"
#include "cgalProcessor.h"
#include "lib3mfProcessor.h"
#include "utils/zipUtility.h"


#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <limits> 
#include <iostream>


int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <filename.vtu> <outer_boundary.stl>" << std::endl;
        return EXIT_FAILURE;
    }
    VtkProcessor vtkProcessor(argv[1]);
    vtkProcessor.showInfo();

    if(!vtkProcessor.LoadAndPrepareData()){
        std::cerr << "Error: LoadAndPrepareData failed." << std::endl;
        return EXIT_FAILURE;
    }

    if(!vtkProcessor.calcAverageStress()){
        std::cerr << "Error: CalcAverageStress failed." << std::endl;
        return EXIT_FAILURE;
    }

    vtkProcessor.prepareStressValues();
    auto dividedMeshes =  vtkProcessor.divideMesh();

    for (int i = 0; i < dividedMeshes.size(); ++i) {
        float minValue = vtkProcessor.getStressValues()[i];
        float maxValue = vtkProcessor.getStressValues()[i + 1];
        std::string fileName = "dividedMesh" + std::to_string(i+1) + "_" + std::to_string(minValue) + "_" + std::to_string(maxValue) + ".stl";
        vtkProcessor.savePolyDataAsSTL(dividedMeshes[i], fileName);
    }
    int dividedMeshNum = dividedMeshes.size();
    Lib3mfProcessor lib3mfProcessor;
    lib3mfProcessor.getMeshes();
    lib3mfProcessor.setStl(argv[2]);
    lib3mfProcessor.setMetaData();
    lib3mfProcessor.assembleObjects();
    lib3mfProcessor.save3mf(".temp/3mf/result.3mf");

    std::string extractDirectory = ".temp/3mf"; // 解凍先のディレクトリ
    std::string zipFile = ".temp/3mf/result.3mf"; // 解凍するZIPファイルのパス            

    std::string directoryToZip = ".temp/3mf"; // 圧縮するディレクトリのパス
    std::string outputZip = "result/test.3mf";
    
    // ZIPファイルを解凍
    if (ZipUtility::unzipFile(zipFile, extractDirectory)) {
        std::cout << "Zip extraction successed" << std::endl;
    } else {
        std::cerr << "Zip extraction failed" << std::endl;
    }
    // ディレクトリをZIP化
    if (ZipUtility::zipDirectory(directoryToZip, outputZip)) {
        std::cout << "Zip compression successed " << std::endl;
    } else {
        std::cerr << "Zip compression failed" << std::endl;
    }

    return EXIT_SUCCESS;
} 