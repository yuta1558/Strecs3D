#include "ExportManager.h"
#include "../utils/tempPathUtility.h"

const QString ExportManager::FILE_FILTER = "3MF Files (*.3mf)";

ExportManager::ExportManager() = default;

ExportManager::~ExportManager() = default;

bool ExportManager::export3mfFile(const std::string& stlFile, QWidget* parent) {
    QString sourcePath = TempPathUtility::getTempFilePath("result/result.3mf");
    if (!check3mfFileExists(sourcePath)) {
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
    
    // 既存のファイルがある場合は削除
    if (QFile::exists(savePath)) {
        if (!QFile::remove(savePath)) {
            if (parent) {
                QMessageBox::critical(parent, "Error", "Failed to overwrite existing file.");
            }
            return false;
        }
    }
    
    if (QFile::copy(sourcePath, savePath)) {
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

bool ExportManager::check3mfFileExists(const QString& sourcePath) const {
    return QFile::exists(sourcePath);
}

QString ExportManager::generateDefaultFileName(const std::string& stlFile) const {
    return QFileInfo(QString::fromStdString(stlFile)).baseName() + ".3mf";
} 