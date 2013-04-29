#include <iostream>
#include <string>
#include <string.h>

#include "application.h"

int main(int argc, char** argv) {
    std::cout<< " arg count : " << argc << std::endl;
    if(argc > 1) {
        std::string cmd = argv[1];
        std::cout<<" input : " << argv[1] << std::endl;

        if(cmd == "help") {
            std::cout<<"usage ./attic-cli <entityurl>" << std::endl;
        }
        else {
            Application app;
            app.Run(cmd);
        }
    }

    return 0;
}
