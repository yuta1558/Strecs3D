#include "VtkProcessor.h"
#include "cgalProcessor.h"
#include "lib3mfProcessor.h"

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
    lib3mfProcessor.save3mf("result/merged_data.3mf");

    return EXIT_SUCCESS;
} 