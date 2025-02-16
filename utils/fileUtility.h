#ifndef ZIPUTILITY_H
#define ZIPUTILITY_H

#include <string>
#include <filesystem>

class FileUtility {
public:
    /// @brief 指定されたディレクトリをZIPファイルに圧縮します。
    /// @param directoryPath 圧縮するディレクトリのパス
    /// @param zipFilePath 出力するZIPファイルのパス
    /// @return 圧縮に成功した場合は true、失敗した場合は false
    static bool zipDirectory(const std::string& directoryPath, const std::string& zipFilePath);
    
    /// @brief 指定されたZIPファイルを解凍して指定のディレクトリに展開します。
    /// @param zipFilePath 解凍するZIPファイルのパス
    /// @param extractToDirectory 展開先のディレクトリパス
    /// @return 解凍に成功した場合は true、失敗した場合は false
    static bool unzipFile(const std::string& zipFilePath, const std::string& extractToDirectory);
    static bool clearDirectoryContents(const std::filesystem::path& dir);

};

#endif // ZIPUTILITY_H
