#ifndef LIBCALLER_H_
#define LIBCALLER_H_
#pragma once

#include <map>
#include <string>
#include <boost/thread/thread.hpp>

class LibCaller {
    LibCaller();
    LibCaller(const LibCaller& rhs){}
    LibCaller operator=(const LibCaller& rhs) { return *this; }

    bool IsIgnored(const std::string& str);
    bool IsHiddenFile(const std::string& path);
public:
    ~LibCaller();

    static LibCaller* instance();
    void Shutdown();

    void LibUploadFile(const std::string& filepath);
    void LibRenameFile(const std::string& old_path, const std::string& new_path);
    void LibCreateFolder(const std::string& folderpath);
    void LibRenameFolder(const std::string& old_path, const std::string& new_path);

    void AddIgnore(const std::string& str);
    void RemoveIgnore(const std::string& str);

private:
    boost::mutex ig_mtx_;
    std::map<std::string, bool> ignore_map_;

    static LibCaller* instance_;
};

#endif

