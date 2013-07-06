#ifndef APPLICATION_H_
#define APPLICATION_H_
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <string.h>

#include "watcher.h"
#include "eventqueue.h"

class Application {
    typedef std::vector<std::string> split;                                    

    int StartLibrary();
    int StopLibrary();
    bool RegisterApp(const std::string& entity_url, const std::string& device_name);
    bool RequestAuthorizationDetails(const std::string& entity_url, const std::string& auth_code);
    bool RegisterPass(const std::string& passphrase);
    bool EnterPass(const std::string& passphrase);

    int CreateLocalFolder(const std::string& folderpath);
    int DeleteLocalFolder(const std::string& folderpath);
    int UploadFile(const std::string& filepath);
    int UploadPublicFile(const std::string& filepath);
    int RemoveLocalFile(const std::string& filepath);
    int RenameLocalFile(const std::string& old_filepath, const std::string& new_filepath);
    int RenameLocalFolder(const std::string& old_folderpath, const std::string& new_folderpath);
    int StartPolling(void);

    void ProcessCommand(split& s);
    void DisplayHelp();

    void SplitString(const std::string& s, char delim, split& out);
    int GetCanonicalPath(const std::string& path, std::string& out);

    int DownloadAtVersion(const std::string& post_id, 
                          const std::string& version,
                          const std::string& folderpath);

    void UploadFolder(const std::string& folderpath);
    void ScanDirectory(const std::string& folderpath, std::vector<std::string>& paths_out);

    bool PathExists(const std::string& path);

public:
    Application();
    ~Application();

    void Run(const std::string& entity);
    bool running() { return running_; }
private:
    std::string entityurl_;
    bool running_;
    bool libstarted_;
    EventQueue eq_;

    FileWatcher* file_watcher_;
    Watcher* watcher_;

};

#endif

