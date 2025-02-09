#ifndef VTKPROCESSOR_H
#define VTKPROCESSOR_H

#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>
#include <vtkTextProperty.h>
#include <vtkPointData.h>
#include <vtkContourFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkOpenGLRenderer.h>
#include <vtkClipPolyData.h>
#include <vtkPlane.h>
#include <vtkSTLWriter.h>
#include <vtkClipDataSet.h>
#include <vtkGeometryFilter.h>
#include <vtkAppendFilter.h>
#include <vtkThreshold.h>
#include <vtkDoubleArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkPolyDataWriter.h>
#include <vtkWarpVector.h>
#include <vtkPolyDataNormals.h>
#include <vtkCleanPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkReverseSense.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkFillHolesFilter.h>
#include <vtkMassProperties.h>

#include <string>

class VtkProcessor{

private:
    std::string vtuFileName;
    vtkSmartPointer<vtkUnstructuredGrid> vtuData;
    vtkSmartPointer<vtkLookupTable> lookupTable;
    double stressRange[2];
    float minStress;
    float maxStress;
    std::vector<float> stressValues;
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    std::array<double, 3>  ComputeMeshCenter(vtkSmartPointer<vtkPolyData> polyData);
    int isoSurfaceNum;
    std::vector<vtkSmartPointer<vtkPolyData>> isoSurfaces;


public:
    VtkProcessor(const std::string& vtuFileName);
    void showInfo();
    bool LoadAndPrepareData();
    void stressDisplay();
    void prepareStressValues();
    bool generateIsoSurface();

    vtkSmartPointer<vtkRenderer> getRenderer() const { return renderer; }
    vtkSmartPointer<vtkRenderWindow> getRenderWindow() const { return renderWindow; }
    vtkSmartPointer<vtkRenderWindowInteractor> getRenderWindowInteractor() const { return renderWindowInteractor; }
    
    void deleteSmallIsosurface(std::vector<vtkSmartPointer<vtkPolyData>> isoSurfaces, double threshold);
    
    int getIsoSurfaceNum() const { return isoSurfaceNum; }
    const std::vector<vtkSmartPointer<vtkPolyData>>& getIsoSurfaces() const {
        return isoSurfaces;
    }

    vtkSmartPointer<vtkPolyData> scalePolyData(vtkSmartPointer<vtkPolyData> polyData, double scaleFactor);
    vtkSmartPointer<vtkPolyData> makePolyDataSmooth(vtkSmartPointer<vtkPolyData> polyData);
    vtkSmartPointer<vtkPolyData> reversePolyDataOrientation(vtkSmartPointer<vtkPolyData> polyData);
    void polyDataDisplay(vtkSmartPointer<vtkPolyData> polyData, vtkSmartPointer<vtkRenderer> renderer);

    void startRnederAndInteraction();
    vtkSmartPointer<vtkPolyData> ReadSTL(const std::string& file_path);
    void stlDisplay(vtkSmartPointer<vtkPolyData> polyData);
    void savePolyDataAsSTL(vtkPolyData* polyData, const std::string& fileName);

};


#endif