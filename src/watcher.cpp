#include "watcher.h"

#include <iostream>
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

void ThreadFunc(WatcherDelegate* del) {
    std::cout<<" starting up watcher delegate " << std::endl;
    if(del) {
        while(del->running()) {
            del->ReadEventBuffer();
        }
    }
    std::cout<<" watcher delegate ending " << std::endl;
}


void WatcherDelegate::ReadEventBuffer() {
    if(fd_ > 0) {
    char buffer[EVENT_BUF_LEN];
        int len = read( fd_, buffer, EVENT_BUF_LEN ); 
        if(len  > -1) {
            int i = 0;
            while(i < len) {
                inotify_event *event = reinterpret_cast<inotify_event*>(&buffer[i]);
                if (event->len) {

                      if (event->mask & IN_CREATE) {
                          if (event->mask & IN_ISDIR)
                              std::cout<<" Dir created : " << event->name << std::endl;
                          else
                              std::cout<<" file created :" << event->name << std::endl;
                      }
                      else if ( event->mask & IN_DELETE ) {
                          if ( event->mask & IN_ISDIR ) {
                              std::cout<<" Dir deleted : " << event->name << std::endl;
                          }
                          else {
                              std::cout<<" file deleted : " << event->name << std::endl;
                          }
                      }
                }
                i += EVENT_SIZE + event->len;
            }
        }
        else {
            std::cout<<" read error " << std::endl;
        }
    }
}

bool WatcherDelegate::PushBackDirectory(const std::string& dir) {
    bool ret = false;
    fd_mtx_.lock();
    fd_ = inotify_init();
    if(fd_ > -1) {
        std::cout<<" adding " << dir << " to watch " << std::endl;
        inotify_add_watch(fd_, dir.c_str(), IN_ALL_EVENTS);
        std::cout<<" FD : " << fd_ << std::endl;
    }
    else { 
        std::cout<<" error adding to watch " << std::endl;
    }
    fd_mtx_.unlock();

    return ret;
}

void WatcherDelegate::set_running(bool r) { 
    r_mtx_.lock();
    running_ = r;
    r_mtx_.unlock();
}

bool WatcherDelegate::running() {
    r_mtx_.lock();
    bool r = running_;
    r_mtx_.unlock();
    return r;
}


Watcher::Watcher() {
    delegate_ = NULL;
    worker_ = NULL;
    is_init_ = false;
}

Watcher::~Watcher() {
    if(delegate_) {
        delete delegate_;
        delegate_ = NULL;
    }
}

void Watcher::Initialize() {
    // Spin off background thread
    if(!delegate_) {
        delegate_ = new WatcherDelegate();
        worker_ = new boost::thread(ThreadFunc, delegate_);
        delegate_->set_running(true);
        is_init_ = true;
    }
}

void Watcher::Shutdown() {
    // Join thread
    if(worker_) {
        delegate_->set_running(false);
        worker_->join();
        delete worker_;
        worker_ = NULL;
        is_init_ = false;
    }
}

void Watcher::WatchFolder(const std::string& folderpath) {
    std::cout<< " watching folder ... " << folderpath << std::endl;
    delegate_->PushBackDirectory(folderpath);
}
