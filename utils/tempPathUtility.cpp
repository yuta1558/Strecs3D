#include "tempPathUtility.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>

QString TempPathUtility::getApplicationDir() {
    // アプリケーションの実行ファイルのディレクトリを取得
    QString appDir = QCoreApplication::applicationDirPath();
    
    // .appバンドルの場合、Contents/MacOSディレクトリから
    // アプリケーションバンドルのルートディレクトリに移動
    if (appDir.contains(".app/Contents/MacOS")) {
        // .app/Contents/MacOSの部分を削除して、.appディレクトリの親ディレクトリを取得
        int macosIndex = appDir.indexOf(".app/Contents/MacOS");
        appDir = appDir.left(macosIndex);
        // .appディレクトリの親ディレクトリ（プロジェクトのルートディレクトリ）を取得
        appDir = appDir.left(appDir.lastIndexOf("/"));
    }
    
    return appDir;
}

QString TempPathUtility::getTempDir() {
    QString appDir = getApplicationDir();
    return appDir + "/.temp";
}

QString TempPathUtility::getTempSubDir(const QString& subDir) {
    QString tempDir = getTempDir();
    return tempDir + "/" + subDir;
}

QString TempPathUtility::getTempFilePath(const QString& relativePath) {
    QString tempDir = getTempDir();
    return tempDir + "/" + relativePath;
}

std::filesystem::path TempPathUtility::getTempDirPath() {
    return std::filesystem::path(getTempDir().toStdString());
}

std::filesystem::path TempPathUtility::getTempSubDirPath(const std::string& subDir) {
    return std::filesystem::path(getTempSubDir(QString::fromStdString(subDir)).toStdString());
}

std::filesystem::path TempPathUtility::getTempFilePathPath(const std::string& relativePath) {
    return std::filesystem::path(getTempFilePath(QString::fromStdString(relativePath)).toStdString());
} 