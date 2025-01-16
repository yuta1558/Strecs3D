#include "VtkProcessor.h"
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



int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <filename.vtu> <outer_boundary.stl>" << std::endl;
        return EXIT_FAILURE;
    }
    VtkProcessor vtkProcessor(argv[1]);
    vtkProcessor.showInfo();

    if(!vtkProcessor.LoadAndPrepareData(argv[1], vtkProcessor.vtuData, vtkProcessor.lookupTable, vtkProcessor.stressRange)){
        std::cerr << "Error: LoadAndPrepareData failed." << std::endl;
        return EXIT_FAILURE;
    }

    vtkProcessor.stressDisplay(vtkProcessor.vtuData, vtkProcessor.lookupTable, vtkProcessor.stressRange, vtkProcessor.renderer);
    vtkProcessor.prepareStressValues(vtkProcessor.minStress, vtkProcessor.maxStress);
    vtkProcessor.generateIsoSurface(vtkProcessor.vtuData, vtkProcessor.stressValues);

    //vtkProcessor.deleteSmallIsosurface(vtkProcessor.isoSurfaces, 5000); // 面積が5000未満の等値面を削除

    //TODO:isoSurface[0]にはメッシュデータがないので、１からにしている。deleteSmallIsosurfaceを修正して、これも省くようにする。
    for (int i = 1; i < vtkProcessor.isoSurfaceNum; ++i) {
        auto expandedSurface = vtkProcessor.scalePolyData(vtkProcessor.isoSurfaces[i], 1.01);
        auto smoothedSurface = vtkProcessor.makePolyDataSmooth(expandedSurface);
        auto reversedSurface = vtkProcessor.reversePolyDataOrientation(smoothedSurface);
        vtkProcessor.polyDataDisplay(smoothedSurface, vtkProcessor.renderer);
        vtkProcessor.savePolyDataAsSTL(smoothedSurface, std::to_string(i) + ".stl");
    }

    vtkProcessor.startRnederAndInteraction(vtkProcessor.renderWindow, vtkProcessor.renderWindowInteractor);
} 