#include "watcher.h"

#include <iostream>


#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


WatchEvent::WatchEvent(inotify_event* event) {
    // Copy event locally
    if(event) {
        event_ = *event;
        
    }
}

WatchEvent::~WatchEvent() {}

Watcher::Watcher() {
    worker_ = NULL;
    is_init_ = false;
}
Watcher::~Watcher() {}

void Watcher::Initialize() {
    // Spin off background thread
    if(!worker_) {
        worker_ = new boost::thread(&Watcher::Run, this);
        set_running(true);
        is_init_ = true;
    }
}

void Watcher::Shutdown() {
    // Join thread
    if(worker_) {
        set_running(false);
        worker_->join();
        delete worker_;
        worker_ = NULL;
        is_init_ = false;
    }
}

void Watcher::Run() {
    while(running()) {
        // Read buffer
        ProcessEventBuffer();
        // Process events
       
        // sleep for a bit
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }
}

void Watcher::ProcessEventBuffer() {
    dir_mtx_.lock();
    DirectoryMap::iterator itr = directories_.begin();
    for(;itr!= directories_.end(); itr++)
        ReadEventBuffer(itr->second.fd);
    dir_mtx_.unlock();
}

void Watcher::ProcessEventDebug(const inotify_event* event) {
    std::cout<<" ProcessEvent ********************************************" << std::endl;
    if (event->mask & IN_ACCESS) {
        std::cout<<" IN_ACCESS " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;
    }
    if (event->mask & IN_ATTRIB) {
        std::cout<<" IN_ATTRIB " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_CLOSE_WRITE) {
        std::cout<<" IN_CLOSE_WRITE " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_CLOSE_NOWRITE) {
        std::cout<<" IN_CLOSE_NOWRITE " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_CREATE) {
        std::cout<<" IN_CREATE " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_DELETE) {
        std::cout<<" IN_DELETE " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_DELETE_SELF) {
        std::cout<<" IN_DELETE_SELF " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_MODIFY) {
        std::cout<<" IN_MODIFY " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_MOVE_SELF) {
        std::cout<<" IN_MOVE_SELF " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_MOVED_FROM) {
        std::cout<<" IN_MOVED_FROM " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_MOVED_TO) {
        std::cout<<" IN_MOVED_TO " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_OPEN) {
        std::cout<<" IN_OPEN " << std::endl;
        if (event->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event->name << std::endl;
        else std::cout<<"\t (file) :" << event->name << std::endl;    
    }
    std::cout<<" *********************************************************" << std::endl;
}

void Watcher::ReadEventBuffer(int fd) {
    if(fd > 0) {
        char buffer[EVENT_BUF_LEN];
        int len = read( fd, buffer, EVENT_BUF_LEN ); 
        if(len  > -1) {
            int i = 0;
            while(i < len) {
                inotify_event *event = reinterpret_cast<inotify_event*>(&buffer[i]);
                if (event->len) {
                    ProcessEventDebug(event);
                }
                i += EVENT_SIZE + event->len;
            }
        }
        else {
            std::cout<<" read error " << std::endl;
        }
    }
}

bool Watcher::WatchDirectory(const std::string& dir) {
    bool ret = false;
    int fd = inotify_init();
    if(fd > -1) {
        std::cout<<" adding " << dir << " to watch " << std::endl;
        inotify_add_watch(fd, dir.c_str(), IN_ALL_EVENTS);
        std::cout<<" FD : " << fd << std::endl;
        if(fd > -1) {
            // success
            watch_target wt;
            wt.fd = fd;
            wt.directory = dir;

            dir_mtx_.lock();
            directories_[dir] = wt;
            dir_mtx_.unlock();
        }
        else {
            // error
            // check errno
        }
    }
    else { 
        std::cout<<" error adding to watch " << std::endl;
    }

    return ret;
}

void Watcher::set_running(bool r) { 
    r_mtx_.lock();
    running_ = r;
    r_mtx_.unlock();
}

bool Watcher::running() {
    r_mtx_.lock();
    bool r = running_;
    r_mtx_.unlock();
    return r;
}

