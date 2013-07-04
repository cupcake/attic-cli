#ifndef WATCHEVENT_H_
#define WATCHEVENT_H_
#pragma once

#include <string>
#include <sys/inotify.h>

class WatchEvent {
public:
    WatchEvent(inotify_event* event,
               const std::string& directory);
    ~WatchEvent();

    const inotify_event* event() const { return &event_; }
    const std::string& directory() const { return directory_; }
private:
    std::string directory_;
    inotify_event event_;
    bool is_file_;
};


#endif

