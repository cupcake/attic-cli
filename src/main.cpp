#include <iostream>
#include <string>
#include <string.h>

#include "libattic.h"

bool RegisterApp(const std::string& entity_url, const std::string& device_name) {
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

    if(!status) {
        std::cout<< "AUTHORIZATION URL : " << GetAuthorizationURL() << std::endl;
        std::cout<< " go to url to finish app registration, and return with auth code" << std::endl;
        return true;
    }

    return false;
}

bool RequestAuthorizationDetails(const std::string& entity_url, const std::string& auth_code) { 
    bool ret = RequestUserAuthorizationDetails(entity_url.c_str(), 
                                               auth_code.c_str(), 
                                               "./config");

    return ret;
}

bool RegisterPassphrase(const std::string& passphrase) {
    int status = RegisterPassphrase(passphrase.c_str());
    if(!status)
        return true;
    return false;
}

bool EnterPassphrase(const std::string& passphrase) {
    int status = EnterPassphrase(passphrase.c_str());
    if(!status)
        return true;
    return false;
}


int main(int argc, char** argv) {
    if(argc > 1) {

    }

    return 0;
}
