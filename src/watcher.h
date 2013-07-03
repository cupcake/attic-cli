#ifndef WATCHER_H_
#define WATCHER_H_
#pragma once

#include <map>
#include <deque>
#include <string>
#include <boost/thread/thread.hpp>

#include "watchevent.h"

class Watcher {
    struct watch_target {
        std::string directory;
        int fd;
    };

    void ProcessEventDebug(const inotify_event* event);
    void ReadEventBuffer(int fd);
    void ProcessEventBuffer();
    void ProcessEvents();
    void Run();
public:
    Watcher();
    ~Watcher();

    void Initialize();
    void Shutdown();
    bool WatchDirectory(const std::string& dir);

    void set_running(bool r);
    bool running();

    bool is_init() { return is_init_; }
private:
    bool is_init_;

    boost::mutex r_mtx_;
    bool running_;
    boost::thread*   worker_;

    boost::mutex dir_mtx_;
    typedef std::map<std::string, watch_target> DirectoryMap;
    DirectoryMap directories_; // directories to watch

    boost::mutex fd_mtx_;
    int fd_; // file descriptor
};

#endif

