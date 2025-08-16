#include "MainWindowUIAdapter.h"
#include "../../UI/widgets/DensitySlider.h"

MainWindowUIAdapter::MainWindowUIAdapter(MainWindowUI* ui, QObject* parent) 
    : IUserInterface(parent), ui(ui)
{
}

void MainWindowUIAdapter::setVtkFileName(const QString& fileName)
{
    if (!ui) return;
    auto vtkDisplayWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkDisplayWidget) {
        vtkDisplayWidget->setFileName(fileName);
    }
}

void MainWindowUIAdapter::setStlFileName(const QString& fileName)
{
    if (!ui) return;
    auto stlDisplayWidget = ui->getObjectDisplayOptionsWidget();
    if (stlDisplayWidget) {
        stlDisplayWidget->setFileName(fileName);
    }
}

void MainWindowUIAdapter::setVtkVisibilityState(bool visible)
{
    if (!ui) return;
    auto vtkWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkWidget) {
        vtkWidget->setVisibleState(visible);
    }
}

void MainWindowUIAdapter::setStlVisibilityState(bool visible)
{
    if (!ui) return;
    auto stlWidget = ui->getObjectDisplayOptionsWidget();
    if (stlWidget) {
        stlWidget->setVisibleState(visible);
    }
}

void MainWindowUIAdapter::setVtkOpacity(double opacity)
{
    if (!ui) return;
    auto vtkWidget = ui->getVtkDisplayOptionsWidget();
    if (vtkWidget) {
        vtkWidget->setOpacity(opacity);
    }
}

void MainWindowUIAdapter::setStlOpacity(double opacity)
{
    if (!ui) return;
    auto stlWidget = ui->getObjectDisplayOptionsWidget();
    if (stlWidget) {
        stlWidget->setOpacity(opacity);
    }
}

void MainWindowUIAdapter::setDividedMeshVisibility(int meshIndex, bool visible)
{
    if (!ui) return;
    ObjectDisplayOptionsWidget* widget = nullptr;
    switch (meshIndex) {
        case 0: widget = ui->getDividedMeshWidget1(); break;
        case 1: widget = ui->getDividedMeshWidget2(); break;
        case 2: widget = ui->getDividedMeshWidget3(); break;
        case 3: widget = ui->getDividedMeshWidget4(); break;
        default: return;
    }
    if (widget) {
        widget->setVisibleState(visible);
    }
}

void MainWindowUIAdapter::setDividedMeshOpacity(int meshIndex, double opacity)
{
    if (!ui) return;
    ObjectDisplayOptionsWidget* widget = nullptr;
    switch (meshIndex) {
        case 0: widget = ui->getDividedMeshWidget1(); break;
        case 1: widget = ui->getDividedMeshWidget2(); break;
        case 2: widget = ui->getDividedMeshWidget3(); break;
        case 3: widget = ui->getDividedMeshWidget4(); break;
        default: return;
    }
    if (widget) {
        widget->setOpacity(opacity);
    }
}

void MainWindowUIAdapter::setDividedMeshFileName(int meshIndex, const QString& fileName)
{
    if (!ui) return;
    ObjectDisplayOptionsWidget* widget = nullptr;
    switch (meshIndex) {
        case 0: widget = ui->getDividedMeshWidget1(); break;
        case 1: widget = ui->getDividedMeshWidget2(); break;
        case 2: widget = ui->getDividedMeshWidget3(); break;
        case 3: widget = ui->getDividedMeshWidget4(); break;
        default: return;
    }
    if (widget) {
        widget->setFileName(fileName);
    }
}

std::vector<double> MainWindowUIAdapter::getStressThresholds() const
{
    if (!ui) return {};
    auto slider = ui->getRangeSlider();
    if (slider) {
        return slider->stressThresholds();
    }
    return {};
}

std::vector<StressDensityMapping> MainWindowUIAdapter::getStressDensityMappings() const
{
    if (!ui) return {};
    auto slider = ui->getRangeSlider();
    if (slider) {
        return slider->stressDensityMappings();
    }
    return {};
}

QString MainWindowUIAdapter::getCurrentMode() const
{
    if (!ui) return "cura";
    auto comboBox = ui->getModeComboBox();
    if (comboBox) {
        return comboBox->currentText();
    }
    return "cura";
}

void MainWindowUIAdapter::setStressRange(double minStress, double maxStress)
{
    if (!ui) return;
    auto slider = ui->getRangeSlider();
    if (slider) {
        slider->setStressRange(minStress, maxStress);
    }
}

void MainWindowUIAdapter::showWarningMessage(const QString& title, const QString& message)
{
    if (ui) {
        QMessageBox::warning(qobject_cast<QWidget*>(ui), title, message);
    }
}

void MainWindowUIAdapter::showCriticalMessage(const QString& title, const QString& message)
{
    if (ui) {
        QMessageBox::critical(qobject_cast<QWidget*>(ui), title, message);
    }
}

void MainWindowUIAdapter::showInfoMessage(const QString& title, const QString& message)
{
    if (ui) {
        QMessageBox::information(qobject_cast<QWidget*>(ui), title, message);
    }
}

bool MainWindowUIAdapter::showFileValidationError()
{
    if (ui) {
        QMessageBox::warning(qobject_cast<QWidget*>(ui), "Validation Error", "File validation failed");
        return false;
    }
    return false;
}

bool MainWindowUIAdapter::showProcessingError(const QString& errorMessage)
{
    if (ui) {
        QMessageBox::critical(qobject_cast<QWidget*>(ui), "Processing Error", errorMessage);
        return false;
    }
    return false;
}

void MainWindowUIAdapter::showProcessingSuccess()
{
    if (ui) {
        QMessageBox::information(qobject_cast<QWidget*>(ui), "Success", "Processing completed successfully");
    }
}