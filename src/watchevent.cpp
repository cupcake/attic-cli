#include "watchevent.h"

WatchEvent::WatchEvent(inotify_event* event) {
    // Copy event locally
    if(event) {
        event_ = *event;
        
    }
}

WatchEvent::~WatchEvent() {}


