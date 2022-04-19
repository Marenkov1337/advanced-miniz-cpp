#pragma once

#include "zip_file.hpp"
#include <optional>
#include <filesystem>
namespace fs = std::filesystem;

namespace zip_functions
{
	//Create folder on given path (folderName)
    void CreateFolder(std::string folderName)
    {
        fs::create_directory(folderName);
    }
	
	//Replace all symbols (what) to another (with) in given string (in) and return result
    std::string ReplaceAll(std::string const& in, std::string const& what, std::string const& with)
    {
        std::string out = in;
        for (std::string::size_type pos{};
            out.npos != (pos = out.find(what.data(), pos, what.length()));
            pos += with.length()) {
            out.replace(pos, what.length(), with.data(), with.length());
        }
        return out;
    }
	
	//Create an archive with given name (newArchiveName) to all files in folder (directoryToArchPath). If succeeded, returns std::nullopt, otherwise error message
    std::optional<std::string> CreateArchiveForDirectoryFiles(std::string const& directoryToArchPath, std::string const& newArchiveName)
    {
        if (directoryToArchPath.empty())
            return "directory to arch path is empty";
        if (newArchiveName.empty())
            return "archive name is empty";

        miniz_cpp::zip_file file;
        auto normzlizedFolderPath = ReplaceAll(directoryToArchPath, "\\", "/");
        for (const auto& entry : fs::recursive_directory_iterator(directoryToArchPath))
        {
            try
            {
                auto filePath = entry.path().string();
                filePath = ReplaceAll(filePath, "\\", "/");
                if (entry.is_directory())
                    filePath += "/";

                std::cout << filePath << std::endl;
                file.write(filePath, ReplaceAll(filePath, normzlizedFolderPath, ""));
            }
            catch (std::runtime_error e)
            {
                return e.what();
            }
        }
        try
        {
            file.save(newArchiveName);
        }
        catch (std::runtime_error e)
        {
            return e.what();
        }
        return std::nullopt;
    }
	
	//Extract an archive (archiveFileName_) to given folder (folderToExtractPath_). If succeeded, returns std::nullopt, otherwise error message
    std::optional<std::string> ExtractArchiveRecursively(std::string const& archiveFileName_, std::string const& folderToExtractPath_)
    {       

        std::string archiveFileName = archiveFileName_;
        std::string folderToExtractPath = folderToExtractPath_;
        if (folderToExtractPath.empty())
            return "folder to extract is empty";
        if (archiveFileName.empty())
            return "archive file name is empty";

        if (folderToExtractPath.at(folderToExtractPath.size() - 1) != '/')
            folderToExtractPath += "/";

        miniz_cpp::zip_file file;
        try
        {
            file.load(archiveFileName);
        }
        catch (std::runtime_error ex)
        {
            return ex.what();
        }

        std::vector<miniz_cpp::zip_info> allFiles;
        try
        {
             allFiles = file.infolist();
        }
        catch (std::runtime_error ex)
        {
            return ex.what();
        }

        for (auto archFile : allFiles)
        {
            auto archFileName = archFile.filename;
            if (archFileName.empty())
                continue;


            auto split = miniz_cpp::detail::split_path(archFileName);
            std::string currentFolderPart = "";
            for (int i = 0; i < split.size() - 1; i++)
            {
                if (split[i].empty())
                    break;
                currentFolderPart += "/" + split[i];
                CreateFolder(folderToExtractPath + currentFolderPart);
            }

            if (archFileName.at(archFileName.size() - 1) == '/')
            {
                CreateFolder(folderToExtractPath + archFileName);
                continue;
            }
            try
            {
                file.extract(archFile, folderToExtractPath);
            }
            catch (std::runtime_error ex)
            {
                return ex.what();
            }
        }

        return std::nullopt;
    }
}
