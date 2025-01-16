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

    vtkSmartPointer<vtkPolyData> ScalePolyData(vtkSmartPointer<vtkPolyData> inputPolyData, double scaleFactor);
    vtkSmartPointer<vtkPolyData> ReversePolyDataOrientation(vtkSmartPointer<vtkPolyData> inputPolyData);
    vtkSmartPointer<vtkPolyData> getDifferenceData(vtkSmartPointer<vtkPolyData> minuend, vtkSmartPointer<vtkPolyData> subtrahend);
    std::array<double, 3>  ComputeMeshCenter(vtkSmartPointer<vtkPolyData> polyData);

public:
    
    std::string vtuFileName;
    VtkProcessor(const std::string& vtuFileName);
    void showInfo();
    vtkSmartPointer<vtkUnstructuredGrid> vtuData;
    vtkSmartPointer<vtkLookupTable> lookupTable;
    double stressRange[2];
    bool LoadAndPrepareData(const char* filename, vtkSmartPointer<vtkUnstructuredGrid>& data, vtkSmartPointer<vtkLookupTable>& lookupTable, double scalarRange[2]);
    void setRenderer(vtkSmartPointer<vtkRenderer> renderer);
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    
    float minStress;
    float maxStress;
    int isoSurfaceNum;
    std::vector<float> stressValues;
    void prepareStressValues(float minSterss,float maxStress);
    
    std::vector<vtkSmartPointer<vtkPolyData>> isoSurfaces;
    bool generateIsoSurface(vtkSmartPointer<vtkUnstructuredGrid> data, std::vector<float> stressValues);

    void deleteSmallIsosurface(std::vector<vtkSmartPointer<vtkPolyData>> isoSurfaces, double threshold);
    vtkSmartPointer<vtkPolyData> scalePolyData(vtkSmartPointer<vtkPolyData> polyData, double scaleFactor);
    vtkSmartPointer<vtkPolyData> makePolyDataSmooth(vtkSmartPointer<vtkPolyData> polyData);
    vtkSmartPointer<vtkPolyData> reversePolyDataOrientation(vtkSmartPointer<vtkPolyData> polyData);

    
    void polyDataDisplay(vtkSmartPointer<vtkPolyData> polyData, vtkSmartPointer<vtkRenderer> renderer);

    void stressDisplay(vtkSmartPointer<vtkUnstructuredGrid> data, vtkSmartPointer<vtkLookupTable> lookupTable, double scalarRange[2], vtkSmartPointer<vtkRenderer> renderer);
    void startRnederAndInteraction(vtkSmartPointer<vtkRenderWindow> renderWindow, vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor);
    vtkSmartPointer<vtkPolyData> ReadSTL(const std::string& file_path);
    void stlDisplay(vtkSmartPointer<vtkPolyData> polyData);
    void savePolyDataAsSTL(vtkPolyData* polyData, const std::string& fileName);

};


#endif