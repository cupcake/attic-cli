#include "application.h"

#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>
#include "libattic.h"

static void HistoryCB(int a, const char* b, int c, int d) {
    std::cout<<" FILE HISTORY : " << std::endl;
    std::cout<<" a : " << a << std::endl;
    std::cout<<" b : " << b << std::endl;
    std::cout<<" c : " << c << std::endl;
    std::cout<<" d : " << d << std::endl;
}

static void ErrorCB(int a, int b, const char* c) {
    std::cout<<" ERROR CAUGHT IN HANDLER : " << std::endl;
    std::cout<<" a : " << a << std::endl;
    std::cout<<" b : " << b << std::endl;
    std::cout<<" c : " << c << std::endl;
}

Application::Application() {
    running_ = false;
    libstarted_ = false;
}

Application::~Application() {}

bool Application::RegisterApp(const std::string& entity_url, 
                              const std::string& device_name) {
    char* buf = new char[entity_url.size()+1];
    memset(buf, '\0', entity_url.size()+1);
    memcpy(buf, entity_url.c_str(), entity_url.size());

    std::string name("attic-cli");
    name += " ";
    name += device_name;

    int status = RegisterAtticApp(entity_url.c_str(),
                                  name.c_str(),
                                  "Attic command line client",
                                  "http://www.tent.is",
                                  "",
                                  entity_url.c_str(),
                                  "./config");

    if(buf) {
        delete[] buf;
        buf = NULL;
    }

    if(!status) {
        std::cout<< "AUTHORIZATION URL : " << GetAuthorizationURL() << std::endl;
        std::cout<< " go to url to finish app registration, and return with auth code" << std::endl;
        return true;
    }

    return false;
}

bool Application::RequestAuthorizationDetails(const std::string& entity_url, 
                                              const std::string& auth_code) {

    std::string canonical;
    GetCanonicalPath("./config", canonical);
    bool ret = RequestUserAuthorizationDetails(entity_url.c_str(), 
                                               auth_code.c_str(), 
                                               canonical.c_str());

    return ret;

}

bool Application::RegisterPass(const std::string& passphrase) {
    int status = -1;
    if(libstarted_) {
        std::cout<<" registering passphrase : " << passphrase << std::endl;
        status = RegisterPassphrase(passphrase.c_str());
        if(!status)
            return true;
    }
    else {
        std::cout<<" libattic not started ... " << std::endl;
    }
    return false;
}

bool Application::EnterPass(const std::string& passphrase) {
    int status = -1;
    if(libstarted_) {
        status = EnterPassphrase(passphrase.c_str());
        if(!status)
            return true;
    }
    else {
        std::cout<<" libattic not started ... " << std::endl;
    }
    return false;
}

void Application::Run(const std::string& entity) {
    SetConfigValue("working_dir", "./data");
    SetConfigValue("config_dir", "./config");
    SetConfigValue("temp_dir", "./data/temp");
    SetConfigValue("trash_path", "./data/temp"); // test trash dir
    SetConfigValue("entity_url", entity.c_str());
    entityurl_= entity;

    running_ = true;
    char buffer[4000] = {'\0'};
    while(running_) {
        memset(buffer, '\0', 4000);
        std::cout <<" >> ";
        std::cin.getline(buffer, 4000);
        split s;
        SplitString(buffer, ' ', s);
        ProcessCommand(s);
        std::cin.clear();
    }
}

void Application::ProcessCommand(split& s) { 
    if(s.size() > 0) {
        std::string toplevel = s[0];
        if(toplevel == "registerapp") {
            if(s.size() > 1) {
                int result = RegisterApp(entityurl_, s[1]);
                std::cout<<" result : " << result << std::endl;
            }
            else
                std::cout<<" usage : registerapp <device_name> " << std::endl;
        }
        else if(toplevel == "requestauthcode") {
            if(s.size() > 1) {
                int result = RequestAuthorizationDetails(entityurl_, s[1]);
                std::cout<<" result : " << result << std::endl;
            }
            else
                std::cout<< " usage : requestauthcode <auth_code> " <<std::endl;
        }
        else if(toplevel == "registerpass") {
            if(s.size() > 1) {
                bool result = RegisterPass(s[1]);
                std::cout<<" register passphrase : ";
                if(result) std::cout<< "OK" << std::endl;
                else std::cout<<"FAIL"<<std::endl;
            }
            else {
                std::cout<<" usage : registerpass <passphrase> " << std::endl;
            }
        }
        else if(toplevel == "enterpass") {
            if(s.size() > 1) {
                bool result = EnterPass(s[1]);
                std::cout<<" enter passphrase : ";
                if(result) std::cout<< "OK" << std::endl;
                else std::cout<<"FAIL"<<std::endl;
            }
            else {
                std::cout<<" usage : enterpass <passphrase " << std::endl;
            }
        }
        else if (toplevel == "start") {
            int result = StartLibrary();
            std::cout<<" result : " << result <<std::endl;
        }
        else if(toplevel == "stop") {
            int result = StopLibrary();
            std::cout<<" result : " << result <<std::endl;
        }
        else if(toplevel == "poll") {
            int result = StartPolling();
            std::cout<<" result : " << result <<std::endl;
        }
        else if(toplevel == "help") {
            DisplayHelp();
        }
        else if(toplevel == "upload") {
            if(s.size() > 1) {
                int result = UploadFile(s[1]);
                std::cout<<" result : " << result <<std::endl;
            }
            else {
                std::cout<<" usage : upload <path to file> " << std::endl;
            }
        }
        else if(toplevel == "upload_folder") {
            if(s.size() > 1) {
                UploadFolder(s[1]);
            }
            else {
                std::cout<<" usage : upload <path to folder> " << std::endl;
            }
        }
        else if(toplevel == "history") {
            if(s.size() > 1) {
                std::string canonical;
                int status = GetCanonicalPath(s[1], canonical);
                int result = GetFileHistory(canonical.c_str(), HistoryCB);
                std::cout<<" result : " << result <<std::endl;
            }
            else {
                std::cout<<" usage : history <path to file> " << std::endl;
            }

        }
        else if(toplevel == "delete") {
            if(s.size() > 1) {
                int result = RemoveLocalFile(s[1]);
                std::cout<<" result : " << result <<std::endl;
            }
            else {
                std::cout<<" usage : delete <path to file> " << std::endl;
            }
        }
        else if(toplevel == "rename"){
            if(s.size() > 2) {
                int result = RenameLocalFile(s[1], s[2]);
                std::cout<<" result : " << result <<std::endl;
            }
            else {
                std::cout<<" usage : rename <path/to/file> <path/to/file>" << std::endl;
            }
        }
        else if(toplevel == "rename_folder") {
            if(s.size() > 2) {
                int result = RenameLocalFolder(s[1], s[2]);
                std::cout<<" result : " << result <<std::endl;
            }
            else {
                std::cout<<" usage : rename <path/to/folder> <path/to/folder>" << std::endl;
            }
        }
        else if (toplevel == "create_folder") {
            if(s.size() > 1) {
                int result = CreateLocalFolder(s[1]);
                std::cout<<" result : " << result <<std::endl;
            }
            else {
                std::cout<<" usage : create_folder <path/to/folder>" << std::endl;
            }
        }
        else if (toplevel == "delete_folder") {
            if(s.size() > 1) {
                int result = DeleteLocalFolder(s[1]);
                std::cout<<" result : " << result <<std::endl;
            }
            else {
                std::cout<<" usage : delete_folder <path/to/folder>" << std::endl;
            }
        }
        else if(toplevel == "exit") {
            running_ = false;
            if(libstarted_) {
                int result = StopLibrary();
                std::cout<<" stopping libattic ... " << result << std::endl;
            }
            std::cout<< "exiting ... " << std::endl;
        }
        else {
            std::cout<<" unknown command " << std::endl;
        }
    }
}

void Application::DisplayHelp() {
    std::string help;
    help += " registerapp - register application with server \n";
    help += " requestauthcode - trade in authcode for auth token \n";
    help += " start - start app \n";
    help += " stop - stop app \n";
    help += " registerpass - register a passphrase \n";
    help += " enterpass - enter passphrase \n";
    help += " poll - start polling \n";
    help += " upload - upload file \n";
    help += " upload_folder - upload a folder \n";
    help += " exit - shutdown lib and exit \n";
    help += " help - displays help \n";
        
    std::cout<< help << std::endl;
}

int Application::StartLibrary() {
    int status = 0;
    RegisterForErrorNotify(ErrorCB);  
    status = InitLibAttic();
    if(!status) libstarted_ = true;
    return status;
}

int Application::StopLibrary() {
    int status = 0;
    status = ShutdownLibAttic(NULL);
    if(!status) libstarted_ = false;
    return status;
}

void Application::SplitString(const std::string &s, char delim, split &out) {    
    std::stringstream ss(s);                                               
    std::string item;
                                                                                       
    while(std::getline(ss, item, delim)) {                                 
        out.push_back(item);                                               
    }
}                                                                          

int Application::UploadFile(const std::string& filepath) {
    int status = -1;
    // Get absolute path first
    std::string canonical;
    status = GetCanonicalPath(filepath, canonical);
    if(!status) {
        std::cout<<" uploading file : " << canonical << std::endl;
        status = PushFile(canonical.c_str());                                       
    }
    return status;
}

int Application::RemoveLocalFile(const std::string& filepath) {
    int status = -1;
    std::string canonical;
    status = GetCanonicalPath(filepath, canonical);
    if(!status) {
        std::cout<<" deleting file : " << canonical << std::endl;
        status = DeleteFile(canonical.c_str());
    }
    return status;
}

int Application::RenameLocalFile(const std::string& old_filepath, const std::string& new_filepath) {
    int status = -1;
    std::string canonical;
    status = GetCanonicalPath(old_filepath, canonical);
    std::cout<<" CANONICAL : " << canonical << std::endl;
    size_t pos = canonical.rfind("/");
    std::string local_filepath;
    if(pos != std::string::npos) {
        size_t pos_2 = new_filepath.rfind("/");
        std::string filename = new_filepath.substr(pos_2);
        std::cout<< "filename : " << filename << std::endl;

        local_filepath = canonical.substr(0, pos);
        std::cout<<" old filepath : " << local_filepath << std::endl; 
        local_filepath += filename; 

        std::cout<<" Local filepath : " << local_filepath << std::endl;
        status = RenameFile(canonical.c_str(), local_filepath.c_str());

    }

    return status;
}
int Application::CreateLocalFolder(const std::string& folderpath) {
    std::string canonical;
    int status = GetCanonicalPath("./data", canonical);
    if(!canonical.empty()) {
        canonical += "/" + folderpath;
        std::cout<<" folderpath : " << canonical << std::endl;
        status = CreateFolder(canonical.c_str());
    }
    return status;
}

int Application::DeleteLocalFolder(const std::string& folderpath) {
    std::string canonical;
    int status = GetCanonicalPath("./data", canonical);
    if(!canonical.empty()) {
        canonical += "/" + folderpath;
        std::cout<<" folderpath : " << canonical << std::endl;
        status = DeleteFolder(canonical.c_str());
    }
    return status;
}

int Application::RenameLocalFolder(const std::string& old_folderpath, 
                                   const std::string& new_folderpath) {
    int status = -1;
    std::string canonical;
    status = GetCanonicalPath(old_folderpath, canonical);
    std::cout<<" CANONICAL : " << canonical << std::endl;
    size_t pos = canonical.rfind("/");
    std::string local_folderpath;
    if(pos != std::string::npos) {
        std::string filename;
        size_t pos_2 = new_folderpath.rfind("/");
        if(pos_2 != std::string::npos)
            filename = new_folderpath.substr(pos_2);
        else
            filename = new_folderpath;

        std::cout<< "filename : " << filename << std::endl;

        local_folderpath = canonical.substr(0, pos);
        std::cout<<" old filepath : " << local_folderpath << std::endl; 
        local_folderpath += "/";
        local_folderpath += new_folderpath;

        std::cout<<" Local folderpath : " << local_folderpath << std::endl;
        status = RenameFolder(canonical.c_str(), local_folderpath.c_str()); 
    }
    return status;
}

int Application::StartPolling(void) {
    int status = -1;

    status =  PollFiles();
    return status;
}

int Application::GetCanonicalPath(const std::string& path, std::string& out) {
    int status = -1;
    boost::filesystem::path root(path.c_str());

    if(boost::filesystem::exists(root)){
        boost::system::error_code error;
        boost::filesystem::path can = boost::filesystem::canonical(root, error);

        if(!error) {
            out = can.string();
            status = 0;
        }
    }
    return status;
}

void Application::UploadFolder(const std::string& folderpath) {
    std::vector<std::string> files;
    ScanDirectory(folderpath, files);
    std::vector<std::string>::iterator itr = files.begin();
    for(;itr != files.end(); itr++) {
        int result = UploadFile(*itr);
        std::cout<<" uploading : " << *itr << " result : " << result << std::endl;
    }
}

void Application::ScanDirectory(const std::string& folderpath, std::vector<std::string>& paths_out) {
    boost::filesystem::path root(folderpath);
    if(boost::filesystem::exists(root)){
        boost::filesystem::directory_iterator itr(root);
        for(;itr != boost::filesystem::directory_iterator(); itr++ ){
            if(itr->status().type() == boost::filesystem::regular_file){
                paths_out.push_back(itr->path().string());
            }
            else if(itr->status().type() == boost::filesystem::directory_file) {
                ScanDirectory(itr->path().string(), paths_out);
            }
        }
    }
}




