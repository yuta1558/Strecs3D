#pragma once

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <string>

class ExportManager {
public:
    ExportManager();
    ~ExportManager();

    // 3MFファイルのエクスポート
    bool export3mfFile(const std::string& stlFile, QWidget* parent = nullptr);

private:
    static const QString SOURCE_PATH;
    static const QString FILE_FILTER;
    
    // ヘルパーメソッド
    bool check3mfFileExists() const;
    QString generateDefaultFileName(const std::string& stlFile) const;
}; 