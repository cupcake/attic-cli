#include "watcher.h"

#include <iostream>
#include <boost/filesystem.hpp>

#include "libcaller.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

Watcher::Watcher(FileWatcher* fw) {
    worker_ = NULL;
    is_init_ = false;
    fd_ = 0;
    fw_ = fw;
}

Watcher::~Watcher() {}

void Watcher::Initialize() {
    // Spin off background thread
    if(!worker_) {
        set_running(true);
        worker_ = new boost::thread(&Watcher::Run, this);
        is_init_ = true;
        fd_ = inotify_init();
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

    LibCaller::instance()->Shutdown();
}

void Watcher::Run() {
    timer_.start();
    while(running()) {
        // Check pending dirs
        CheckPendingDirectories();
        // Read buffer
        ReadEventBuffer(); // This is blocking
       
        // sleep for a bit
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }
    timer_.stop();
    std::cout<<" watcher finishing " << std::endl;
}


void Watcher::CheckPendingDirectories() {
    pdir_mtx_.lock();
    if(pending_directories_.size()) {
        DirectoryMap::iterator itr = pending_directories_.begin();
        for(;itr!=pending_directories_.end(); itr++) {
            // copy over watch targets
            directories_[itr->first] = itr->second;
            ScanDirectory(itr->second.directory);
        }
        pending_directories_.clear();
    }
    pdir_mtx_.unlock();
}

// Do this to see if we need to watch any new directories
void Watcher::ProcessEvent(const inotify_event* event, const int wd) {
    std::ostringstream oss;
    oss <<" ProcessEvent ********************************************" << std::endl;

    /*
    if (event->mask & IN_ACCESS) {
        oss <<" IN_ACCESS " << std::endl;
        if (event->mask & IN_ISDIR) oss<<"\t (dir) : " << event->name << std::endl;
        else oss <<"\t (file) :" << event->name << std::endl;
    }
    if (event->mask & IN_ATTRIB) {
        oss <<" IN_ATTRIB " << std::endl;
        if (event->mask & IN_ISDIR) oss<<"\t (dir) : " << event->name << std::endl;
        else oss <<"\t (file) :" << event->name << std::endl;    
    }
    */
    if (event->mask & IN_CLOSE_WRITE) {
        oss <<" IN_CLOSE_WRITE " << std::endl;
        if (event->mask & IN_ISDIR) {
     //       oss<<"\t (dir) : " << event->name << std::endl;
        }
        else {
      //      oss<<"\t (file) :" << event->name << std::endl;    
            std::string filepath = directories_[wd].directory;
            filepath += std::string("/") + std::string(event->name);
            fw_->SetMarkerClosed(filepath);
        }
    }
    if (event->mask & IN_CLOSE_NOWRITE) {
        oss<<" IN_CLOSE_NOWRITE " << std::endl;
        if (event->mask & IN_ISDIR) {
        //    oss<<"\t (dir) : " << event->name << std::endl;
        }
        else {
            //oss<<"\t (file) :" << event->name << std::endl;    
            std::string filepath = directories_[wd].directory;
            filepath += std::string("/") + std::string(event->name);
            //oss<<" filepath : " << filepath << std::endl;
            fw_->SetMarkerClosed(filepath);
        }
    }
    if (event->mask & IN_CREATE) {
        //oss<<" IN_CREATE " << std::endl;
        if (event->mask & IN_ISDIR) {
            // create directory path
            std::string dir = directories_[wd].directory;
            dir += std::string("/") + std::string(event->name);
            // add to watch
            boost::filesystem::path root(dir);
            if(boost::filesystem::exists(root)) {
                WatchDirectoryDirectly(dir);
                // call into lib TEMPORARY
                oss << " Create Folder : " << dir << std::endl;
                LibCaller::instance()->LibCreateFolder(dir.c_str());
            }
            
        }
        else {
            std::string filepath = directories_[wd].directory;
            filepath += std::string("/") + std::string(event->name);
            oss<<" filepath : "<< filepath << std::endl;
            if(boost::filesystem::exists(filepath))
                fw_->SetMarkerOpen(filepath);
        }
    }
    if (event->mask & IN_DELETE) {
        oss<<" IN_DELETE " << std::endl;
        if (event->mask & IN_ISDIR) oss<<"\t (dir) : " << event->name << std::endl;
        else oss<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_DELETE_SELF) {
        oss<<" IN_DELETE_SELF " << std::endl;
        if (event->mask & IN_ISDIR) oss<<"\t (dir) : " << event->name << std::endl;
        else oss<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_MODIFY) {
        oss<<" IN_MODIFY " << std::endl;
        if (event->mask & IN_ISDIR) oss<<"\t (dir) : " << event->name << std::endl;
        else oss<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_MOVE_SELF) {
        oss<<" IN_MOVE_SELF " << std::endl;
        if (event->mask & IN_ISDIR) oss<<"\t (dir) : " << event->name << std::endl;
        else oss<<"\t (file) :" << event->name << std::endl;    
    }
    if (event->mask & IN_MOVED_FROM) {
        // remove from watching
        //oss<<" IN_MOVED_FROM " << std::endl;
        if (event->mask & IN_ISDIR) {
            //oss<<"\t (dir) : " << event->name << std::endl;
            std::string dir = directories_[wd].directory;
            dir += std::string("/") + std::string(event->name);
            rename_map_[event->cookie].from_dir = dir;
            //UnwatchDirectory(wd);
        }
        else {
            //oss<<"\t (file) :" << event->name << std::endl;    
            //oss<<" cookie : " << event->cookie << std::endl;

            //oss<<"\t (dir) : " << event->name << std::endl;
            std::string dir = directories_[wd].directory;
            dir += std::string("/") + std::string(event->name);
            rename_map_[event->cookie].from_dir = dir;
 
        }
    }
    if (event->mask & IN_MOVED_TO) {
        // add to watching
        //oss<<" IN_MOVED_TO " << std::endl;
        if (event->mask & IN_ISDIR) {
            //oss<<"\t (dir) : " << event->name << std::endl;
            std::string dir = directories_[wd].directory;
            dir += std::string("/") + std::string(event->name);
            rename_map_[event->cookie].to_dir = dir;
            //oss<<" move event complete ... " << std::endl;
            //oss<<"\t moved dir from : " << rename_map_[event->cookie].from_dir << std::endl;
            //oss<<"\t moved dir to : " << rename_map_[event->cookie].to_dir << std::endl;
            WatchDirectoryDirectly(dir);

            // call into lib TEMPRORAY
            if(!rename_map_[event->cookie].from_dir.empty() && 
               !rename_map_[event->cookie].to_dir.empty()) {
                LibCaller::instance()->LibRenameFolder(rename_map_[event->cookie].from_dir.c_str(),
                                          rename_map_[event->cookie].to_dir.c_str()); 
                
            }
            else if(!rename_map_[event->cookie].from_dir.empty() && 
                    !rename_map_[event->cookie].to_dir.empty()) {
                LibCaller::instance()->LibCreateFolder(rename_map_[event->cookie].to_dir.c_str());
            }
        }
        else {
            /*
            oss<<"\t (file) :" << event->name << std::endl;    
            oss<<" cookie : " << event->cookie << std::endl;
            oss<<"\t (dir) : " << event->name << std::endl;
            */
            std::string dir = directories_[wd].directory;
            dir += std::string("/") + std::string(event->name);
            rename_map_[event->cookie].to_dir = dir;
            /*
            oss<<" move event complete ... " << std::endl;
            oss<<"\t moved dir from : " << rename_map_[event->cookie].from_dir << std::endl;
            oss<<"\t moved dir to : " << rename_map_[event->cookie].to_dir << std::endl;
            */

            // call into lib TEMPRORAY
            if(!rename_map_[event->cookie].from_dir.empty() && 
               !rename_map_[event->cookie].to_dir.empty()) {
                LibCaller::instance()->LibRenameFile(rename_map_[event->cookie].from_dir.c_str(),
                                        rename_map_[event->cookie].to_dir.c_str()); 
                fw_->SetMarkerClosed(rename_map_[event->cookie].from_dir.c_str());
            }
            else if(!rename_map_[event->cookie].from_dir.empty() && 
                    !rename_map_[event->cookie].to_dir.empty()) {
                LibCaller::instance()->LibUploadFile(rename_map_[event->cookie].to_dir.c_str());
                fw_->SetMarkerClosed(rename_map_[event->cookie].to_dir.c_str());
            }

 
        }
        // remove from rename_map_
    }

    if (event->mask & IN_OPEN) {
        //oss<<" IN_OPEN " << std::endl;
        if (event->mask & IN_ISDIR) {
            //oss<<"\t (dir) : " << event->name << std::endl;
        }
        else {
            /*
            oss<<"\t (file) :" << event->name << std::endl;    
            oss<<"\t (wd) : " << wd << std::endl;
            */
            std::string filepath = directories_[wd].directory;
            filepath += std::string("/") + std::string(event->name);
            //oss<<" filepath : " << filepath << std::endl;
            // make sure file exists
            /*
            if(boost::filesystem::exists(filepath))
                fw_->SetMarkerOpen(filepath);
                */
        }
    }
    oss<<" *********************************************************" << std::endl;
}

void Watcher::ReadEventBuffer() {
    if(fd_ > 0) {
        char buffer[EVENT_BUF_LEN];
        std::cout<<" reading event buffer for fd : " << fd_ << std::endl;
        int len = read( fd_, buffer, EVENT_BUF_LEN );  // this read is blocking
        if(len  > -1) {
            int i = 0;
            while(i < len) {
                inotify_event *event = reinterpret_cast<inotify_event*>(&buffer[i]);
                if (event->len) {
                    ProcessEvent(event, event->wd);
                }
                i += EVENT_SIZE + event->len;
            }
            std::cout<<" done reading event buffer " << std::endl;
        }
        else {
            std::cout<<" read error " << std::endl;
        }
    }
    else {
        std::cout<< " fd is : " << fd_ << std::endl;
    }
}

bool Watcher::UnwatchDirectory(const int wd) {
    bool ret = false;
    if(inotify_rm_watch(fd_, wd) == 0) {
        ret = true;
    }
    else {
        // error
        // check errno
    }
    return ret;
}

bool Watcher::WatchDirectoryDirectly(const std::string& dir) {
    bool ret = false;

    if(fd_ > -1) {
        std::cout<<" adding " << dir << " to watch " << std::endl;
        int wd = inotify_add_watch(fd_, dir.c_str(), IN_ALL_EVENTS);
        std::cout<<" FD : " << fd_ << std::endl;
        std::cout<<" WD : " << wd << std::endl;
        if(fd_ > -1) {
            // success
            watch_target wt;
            wt.fd = fd_; // file descriptor shared amongst all watch targets
            wt.wd = wd; // watch descriptor
            wt.directory = dir;
            directories_[wd] = wt;
            ret = true;
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

bool Watcher::WatchDirectory(const std::string& dir) {
    bool ret = false;
    //int fd_ = inotify_init();
    if(fd_ > -1) {
        std::cout<<" adding " << dir << " to watch " << std::endl;
        int wd = inotify_add_watch(fd_, dir.c_str(), IN_ALL_EVENTS);
        std::cout<<" FD : " << fd_ << std::endl;
        std::cout<<" WD : " << wd << std::endl;
        if(fd_ > -1) {
            // success
            watch_target wt;
            wt.fd = fd_;
            wt.wd = wd;
            wt.directory = dir;

            pdir_mtx_.lock();
            pending_directories_[wd] = wt;
            pdir_mtx_.unlock();
            std::cout<<" \t success " << std::endl;
            ret = true;
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

void Watcher::ScanDirectory(const std::string& folderpath) {
    boost::filesystem::path root(folderpath);
    if(boost::filesystem::exists(root)){
        boost::filesystem::directory_iterator itr(root);
        for(;itr != boost::filesystem::directory_iterator(); itr++ ){
            if(itr->status().type() == boost::filesystem::directory_file){
                WatchDirectoryDirectly(itr->path().string());
                ScanDirectory(itr->path().string());
            }
        }
    }
}

