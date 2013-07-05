#include "watchevent.h"

#include <iostream>
#include <string.h>

WatchEvent::WatchEvent(inotify_event* event,
                       const std::string& directory) {
    // Copy event locally
    if(event) {
        //event_ = *event;
        memcpy(&event_, event, sizeof(struct inotify_event));
    }
    std::cout<<" setting watch event dir : " << directory << std::endl;
    directory_.append(directory.c_str(), directory.size());
    std::cout<<" setting watch event dir : " << directory_ << std::endl;
    is_file_ = false;
}

WatchEvent::~WatchEvent() {}


