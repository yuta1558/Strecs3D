#ifndef TEMPPATHUTILITY_H
#define TEMPPATHUTILITY_H

#include <QString>
#include <QDir>
#include <filesystem>

class TempPathUtility {
public:
    /**
     * @brief アプリケーションの実行ファイルのディレクトリを取得
     * @return 実行ファイルのディレクトリパス
     */
    static QString getApplicationDir();
    
    /**
     * @brief .tempディレクトリの絶対パスを取得
     * @return .tempディレクトリの絶対パス
     */
    static QString getTempDir();
    
    /**
     * @brief .tempディレクトリ内の指定されたサブディレクトリの絶対パスを取得
     * @param subDir サブディレクトリ名（例: "div", "result", "3mf"）
     * @return サブディレクトリの絶対パス
     */
    static QString getTempSubDir(const QString& subDir);
    
    /**
     * @brief .tempディレクトリ内の指定されたファイルの絶対パスを取得
     * @param relativePath .tempディレクトリからの相対パス（例: "result/result.3mf"）
     * @return ファイルの絶対パス
     */
    static QString getTempFilePath(const QString& relativePath);
    
    /**
     * @brief std::filesystem::path版のgetTempDir
     * @return .tempディレクトリのstd::filesystem::path
     */
    static std::filesystem::path getTempDirPath();
    
    /**
     * @brief std::filesystem::path版のgetTempSubDir
     * @param subDir サブディレクトリ名
     * @return サブディレクトリのstd::filesystem::path
     */
    static std::filesystem::path getTempSubDirPath(const std::string& subDir);
    
    /**
     * @brief std::filesystem::path版のgetTempFilePath
     * @param relativePath .tempディレクトリからの相対パス
     * @return ファイルのstd::filesystem::path
     */
    static std::filesystem::path getTempFilePathPath(const std::string& relativePath);
};

#endif // TEMPPATHUTILITY_H 