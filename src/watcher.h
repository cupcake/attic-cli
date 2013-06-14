#ifndef WATCHER_H_
#define WATCHER_H_
#pragma once

#include <map>
#include <deque>
#include <string>
#include <boost/thread/thread.hpp>

class WatcherDelegate {
    struct watch_target {
        std::string directory;
        int fd;
    };


public:
    WatcherDelegate(){
        running_ = false;
    }
    ~WatcherDelegate(){}

    void ReadEventBuffer();

    bool PushBackDirectory(const std::string& dir);
    void set_running(bool r);
    bool running();

private:
    boost::mutex r_mtx_;
    bool running_;

    boost::mutex dir_mtx_;
    std::map<std::string, watch_target> directories_;

    boost::mutex fd_mtx_;
    int fd_;
};

class Watcher {
public:
    Watcher();
    ~Watcher();

    void Initialize();
    void Shutdown();
    void WatchFolder(const std::string& folderpath);

    bool is_init() { return is_init_; }
private:
    bool is_init_;
    WatcherDelegate* delegate_;
    boost::thread*   worker_;
};

#endif

