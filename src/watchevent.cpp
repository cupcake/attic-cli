#include "watchevent.h"

WatchEvent::WatchEvent(inotify_event* event,
                       const std::string& directory) {
    // Copy event locally
    if(event) {
        event_ = *event;
        
    }
    directory_ = directory;
    is_file_ = false;
}

WatchEvent::~WatchEvent() {}


