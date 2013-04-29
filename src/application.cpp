#include "application.h"

#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>
#include "libattic.h"


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

int Application::DeleteFile(const std::string& filepath) {
    int status = -1;
    std::string canonical;
    status = GetCanonicalPath(filepath, canonical);
    if(!status) {
        std::cout<<" deleting file : " << canonical << std::endl;
        status = DeleteFile(canonical.c_str());
    }
    return status;
}

int Application::RenameFile(const std::string& old_filepath, const std::string& new_filename) {
    int status = -1;

    status = RenameFile(old_filepath.c_str(), new_filename.c_str());
    return status;
}

int Application::RenameFolder(const std::string& old_folderpath, const std::string& new_foldername) {
    int status = -1;

    status = RenameFolder(old_folderpath.c_str(), new_foldername.c_str()); 
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

