#pragma once

#include <QObject>
#include <QMessageBox>
#include "../interfaces/IUserInterface.h"
#include "../../UI/mainwindowui.h"

class ObjectDisplayOptionsWidget;

class MainWindowUIAdapter : public IUserInterface {
    Q_OBJECT
public:
    explicit MainWindowUIAdapter(MainWindowUI* ui, QObject* parent = nullptr);
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
    
    // メッセージ表示
    void showWarningMessage(const QString& title, const QString& message) override;
    void showCriticalMessage(const QString& title, const QString& message) override;
    void showInfoMessage(const QString& title, const QString& message) override;
    
    // ファイル選択・保存ダイアログ
    bool showFileValidationError() override;
    bool showProcessingError(const QString& errorMessage) override;
    void showProcessingSuccess() override;
    
    // Adapter specific method
    MainWindowUI* getMainWindowUI() const { return ui; }

private:
    ObjectDisplayOptionsWidget* getDividedMeshWidget(int meshIndex) const;
    MainWindowUI* ui;
};