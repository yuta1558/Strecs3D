#pragma once

#include <QString>
#include <vector>

struct StressDensityMapping;

class IUserInterface {
public:
    virtual ~IUserInterface() = default;
    
    // ファイル名設定
    virtual void setVtkFileName(const QString& fileName) = 0;
    virtual void setStlFileName(const QString& fileName) = 0;
    
    // 表示状態制御
    virtual void setVtkVisibilityState(bool visible) = 0;
    virtual void setStlVisibilityState(bool visible) = 0;
    virtual void setVtkOpacity(double opacity) = 0;
    virtual void setStlOpacity(double opacity) = 0;
    
    // 分割メッシュ制御
    virtual void setDividedMeshVisibility(int meshIndex, bool visible) = 0;
    virtual void setDividedMeshOpacity(int meshIndex, double opacity) = 0;
    virtual void setDividedMeshFileName(int meshIndex, const QString& fileName) = 0;
    
    // データ取得
    virtual std::vector<double> getStressThresholds() const = 0;
    virtual std::vector<StressDensityMapping> getStressDensityMappings() const = 0;
    virtual QString getCurrentMode() const = 0;
    
    // ストレス範囲設定
    virtual void setStressRange(double minStress, double maxStress) = 0;
};