#include "ExportManager.h"

const QString ExportManager::SOURCE_PATH = QDir::currentPath() + "/result/result.3mf";
const QString ExportManager::FILE_FILTER = "3MF Files (*.3mf)";

ExportManager::ExportManager() = default;

ExportManager::~ExportManager() = default;

bool ExportManager::export3mfFile(const std::string& stlFile, QWidget* parent) {
    if (!check3mfFileExists()) {
        if (parent) {
            QMessageBox::warning(parent, "Error", "No 3MF file found in result directory.");
        }
        return false;
    }
    
    QString defaultName = generateDefaultFileName(stlFile);
    QString savePath = QFileDialog::getSaveFileName(parent,
        "Save 3MF File",
        QDir::homePath() + "/" + defaultName,
        FILE_FILTER);
        
    if (savePath.isEmpty()) {
        return false;
    }
    
    if (!savePath.endsWith(".3mf", Qt::CaseInsensitive)) {
        savePath += ".3mf";
    }
    
    if (QFile::copy(SOURCE_PATH, savePath)) {
        if (parent) {
            QMessageBox::information(parent, "Success", "3MF file exported successfully.");
        }
        return true;
    } else {
        if (parent) {
            QMessageBox::critical(parent, "Error", "Failed to export 3MF file.");
        }
        return false;
    }
}

bool ExportManager::check3mfFileExists() const {
    return QFile::exists(SOURCE_PATH);
}

QString ExportManager::generateDefaultFileName(const std::string& stlFile) const {
    return QFileInfo(QString::fromStdString(stlFile)).baseName() + ".3mf";
} 