#pragma once

#include "../interfaces/IUserInterface.h"
#include "../../UI/mainwindowui.h"

class MainWindowUIAdapter : public IUserInterface {
public:
    explicit MainWindowUIAdapter(MainWindowUI* ui);
    ~MainWindowUIAdapter() = default;
    
    // IUserInterface implementation
    void setVtkFileName(const QString& fileName) override;
    void setStlFileName(const QString& fileName) override;
    void setVtkVisibilityState(bool visible) override;
    void setStlVisibilityState(bool visible) override;
    void setVtkOpacity(double opacity) override;
    void setStlOpacity(double opacity) override;
    void setDividedMeshVisibility(int meshIndex, bool visible) override;
    void setDividedMeshOpacity(int meshIndex, double opacity) override;
    void setDividedMeshFileName(int meshIndex, const QString& fileName) override;
    std::vector<double> getStressThresholds() const override;
    std::vector<StressDensityMapping> getStressDensityMappings() const override;
    QString getCurrentMode() const override;
    void setStressRange(double minStress, double maxStress) override;
    
    // Adapter specific method
    MainWindowUI* getMainWindowUI() const { return ui; }

private:
    MainWindowUI* ui;
};