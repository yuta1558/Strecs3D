#ifndef CGALPROCESSOR_H
#define CGALPROCESSOR_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/IO/STL.h>


namespace PMP = CGAL::Polygon_mesh_processing;
using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Mesh   = CGAL::Surface_mesh<Kernel::Point_3>;


class CGALProcessor{
    private:
        std::vector<std::string> stlFiles;
        std::vector<Mesh> isoMeshes;
        std::vector<Mesh> isoMeshesRev;
        int isoSurfaceNum;
        Mesh outlineMesh;

    public:
        bool getFileNames();
        bool readStl(const std::string& filename, Mesh& mesh);
        bool writeStl(const std::string& filename, const Mesh& mesh);
        bool prepareMeshes(const std::string& outlineFileName);
        bool divideMeshes();
        void printMeshInfo(const Mesh& mesh);

};

#endif // CGALPROCESSOR_H