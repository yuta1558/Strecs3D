#ifndef LIB3MFPROCESSOR_H
#define LIB3MFPROCESSOR_H

#include "lib3mf_implicit.hpp"
using namespace Lib3MF;

#include "../../utils/xmlConverter.h"
#include <vector>
#include "../../UI/widgets/DensitySlider.h" // For StressDensityMapping

struct FileInfo {
    int id;
    std::string name;
    double minStress;
    double maxStress;
};

class Lib3mfProcessor{
    private:
        PWrapper wrapper = CWrapper::loadLibrary();
        PModel model = wrapper->CreateModel();
        PReader reader = model->QueryReader("stl"); 

        xmlconverter::Config config;
        xmlconverter::Object object;
    public:
        bool getMeshes();
        bool setStl(const std::string stlFileName);
        bool setMetaData(double maxStress);
        bool setMetaData(double maxStress, const std::vector<StressDensityMapping>& mappings);
        bool save3mf(const std::string outputFilename);
        bool setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress);
        bool setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings);
        bool setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh);
        bool assembleObjects();

        bool setMetaDataBambu(double maxStress);
        bool setMetaDataBambu(double maxStress, const std::vector<StressDensityMapping>& mappings);
        bool setMetaDataForInfillMeshBambu(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress);
        bool setMetaDataForInfillMeshBambu(Lib3MF::PMeshObject Mesh, FileInfo fileInfo, double maxStress, const std::vector<StressDensityMapping>& mappings);
        bool setMetaDataForOutlineMeshBambu(Lib3MF::PMeshObject Mesh);

        bool setObjectDataBambu(int meshCount);
        bool setPlateDataBambu(int meshCount);
        bool setAssembleDataBambu(int meshCount);
        bool setupBuildObjects();
        bool exportConfig();

};



#endif