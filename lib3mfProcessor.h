#ifndef LIB3MFPROCESSOR_H
#define LIB3MFPROCESSOR_H

#include "lib3mf_implicit.hpp"
using namespace Lib3MF;

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
    public:
        bool getMeshes();
        bool setStl(const std::string stlFileName);
        bool setMetaData();
        bool save3mf(const std::string outputFilename);
        bool setMetaDataForInfillMesh(Lib3MF::PMeshObject Mesh, FileInfo fileInfo);
        bool setMetaDataForOutlineMesh(Lib3MF::PMeshObject Mesh);
        bool assembleObjects();
};



#endif