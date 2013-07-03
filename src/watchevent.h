#ifndef WATCHEVENT_H_
#define WATCHEVENT_H_
#pragma once

#include <sys/inotify.h>
class WatchEvent {
public:
    WatchEvent(inotify_event* event);
    ~WatchEvent();

    inotify_event* event() { return &event_; }
private:
    inotify_event event_;
};


#endif

