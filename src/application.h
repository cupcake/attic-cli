#ifndef APPLICATION_H_
#define APPLICATION_H_
#pragma once

#include <iostream>
#include <string.h>

class Application {
public:
    Application() {
        running_ = false;
    }

    ~Application() {}

    void Run() {
        running_ = true;
        char buffer[4000] = {'\0'};
        while(running_) {
            memset(buffer, '\0', 4000);
            std::cout <<" >> ";
            std::cin.getline(buffer, 4000);

            std::cout<<" output : " << buffer << std::endl;
        }
    }

    bool running() { return running_; }

private:
    bool running_;
};

#endif

