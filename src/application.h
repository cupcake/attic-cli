#ifndef APPLICATION_H_
#define APPLICATION_H_
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <string.h>

class Application {
    typedef std::vector<std::string> split;                                    

    int StartLibrary();
    int StopLibrary();
    bool RegisterApp(const std::string& entity_url, const std::string& device_name);
    bool RequestAuthorizationDetails(const std::string& entity_url, const std::string& auth_code);
    bool RegisterPass(const std::string& passphrase);
    bool EnterPass(const std::string& passphrase);

    int UploadFile(const std::string& filepath);
    int RemoveFile(const std::string& filepath);
    int RenameFile(const std::string& old_filepath, const std::string& new_filename);
    int RenameFolder(const std::string& old_folderpath, const std::string& new_foldername);
    int StartPolling(void);

    void ProcessCommand(split& s);
    void DisplayHelp();

    void SplitString(const std::string &s, char delim, split &out);
    int GetCanonicalPath(const std::string& path, std::string& out);

public:
    Application();
    ~Application();

    void Run(const std::string& entity);
    bool running() { return running_; }
private:
    std::string entityurl_;
    bool running_;
    bool libstarted_;
};

#endif

