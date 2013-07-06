#ifndef WATCHER_H_
#define WATCHER_H_
#pragma once

#include <map>
#include <deque>
#include <string>
#include <boost/timer/timer.hpp>
#include <boost/thread/thread.hpp>

#include "watchevent.h"
#include "filemarker.h"
#include "filewatcher.h"

class Watcher {
    struct watch_target {
        std::string directory;
        int fd;
        int wd;
    };

    struct move_event {
        std::string from_dir;
        std::string to_dir;
    };

    void CheckPendingDirectories();
    void ProcessEvent(const inotify_event* event, const int wd);
    void ReadEventBuffer();
    void ProcessEvents();
    void Run();

    bool WatchDirectoryDirectly(const std::string& dir);
    bool UnwatchDirectory(const int wd);

public:
    Watcher(FileWatcher* fw);
    ~Watcher();

    void Initialize();
    void Shutdown();
    bool WatchDirectory(const std::string& dir);

    void set_running(bool r);
    bool running();

    bool is_init() { return is_init_; }
private:
    // watch descriptor, watch target
    typedef std::map<int, watch_target> DirectoryMap;
    // cookie, move event
    typedef std::map<int, move_event> RenameMap;
    // filepath, file marker

    bool is_init_;

    boost::mutex r_mtx_;
    bool running_;
    boost::thread*   worker_;

    boost::mutex pdir_mtx_;
    DirectoryMap pending_directories_; // directories recently appended
    DirectoryMap directories_; // directories to watch

    RenameMap rename_map_;

    boost::mutex fd_mtx_;
    int fd_; // file descriptor, one file descriptor can be associated with many watch descriptors
    
    boost::timer::cpu_timer timer_; 

    FileWatcher* fw_;
};

#endif

