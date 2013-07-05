#include "watchevent.h"

#include <iostream>
#include <string.h>

WatchEvent::WatchEvent(inotify_event* event,
                       const std::string& directory) {
    /*
     * struct inotify_event {
     *   int      wd;       // Watch descriptor 
     *   uint32_t mask;     // Mask of events 
         uint32_t cookie;   // Unique cookie associating related  events (for rename(2)) 
         uint32_t len;      // Size of name field 
         char     name[];   // Optional null-terminated name
        };
     */

    event_.wd = event->wd;
    event_.mask = event->mask;
    event_.cookie = event->cookie;
    event_.len = event->len;
    strcpy(event_.name, event->name);

    std::cout <<" copied name : " << event_.name << std::endl;

    // Copy event locally
    //memcpy(&event_, event, sizeof(inotify_event));
    std::cout<<" setting watch event dir : " << directory << std::endl;
    directory_.append(directory.c_str(), directory.size());
    std::cout<<" setting watch event dir : " << directory_ << std::endl;
    is_file_ = false;
}

WatchEvent::WatchEvent(const WatchEvent& rhs) {
    directory_ = rhs.directory_;
    is_file_ = rhs.is_file_;

    event_.wd = rhs.event_.wd;
    event_.mask = rhs.event_.mask;
    event_.cookie = rhs.event_.cookie;
    event_.len = rhs.event_.len;
    strcpy(event_.name, rhs.event_.name);
}

WatchEvent::~WatchEvent() {}


