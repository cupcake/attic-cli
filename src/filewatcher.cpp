#include "filewatcher.h"


#include "libcaller.h"

static boost::timer::nanosecond_type const limit(1 * 100000000LL); 
FileWatcher::FileWatcher() {
    thread_ = NULL;
    running_ = false;
}

FileWatcher::~FileWatcher() {

}

void FileWatcher::Initialize() {
    if(!thread_) {
        thread_ = new boost::thread(&FileWatcher::Run, this);
        set_running(true);
    }

}

void FileWatcher::Shutdown() {
    if(thread_) {
        set_running(false);
        thread_->join();
        delete thread_;
        thread_ = NULL;
    }

}

void FileWatcher::Run() {
    timer_.start();
    while(running()) {
        ProcessFileQueue();
        Update();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }
    timer_.stop();
}

void FileWatcher::Update() {
    boost::timer::cpu_times time = timer_.elapsed();                     
    //boost::timer::nanosecond_type const elapsed(time.user);
    boost::timer::nanosecond_type const elapsed(time.system + time.user);

    tm_mtx_.lock();
    TransferMap::iterator itr = transfer_map_.begin();
    for(;itr!=transfer_map_.end();itr++) {
        if(itr->second.IsDone()) {
            /*
            TransferMap::iterator hold = itr; 
            if(hold != transfer_map_.begin())
                hold--;
            transfer_map_.erase(itr);
            if(hold == itr)
                itr = transfer_map_.begin();
            else
                itr = hold;
                */
            // call into lib
           
            // remove from map
            std::cout<<" Pushing file : " << itr->second.path() << std::endl;
            LibCaller::instance()->LibUploadFile(itr->second.path());
            transfer_map_.erase(itr);
        }
        else {
            itr->second.Update(elapsed);
        }
    }
    tm_mtx_.unlock();

    if(elapsed > limit) {
        timer_.stop();
        timer_.start();
    }
}

void FileWatcher::ProcessFileQueue() {
    // copy all markers from queue into transfermap
    tm_mtx_.lock();
    fq_mtx_.lock();
    std::deque<FileMarker>::iterator itr = file_queue_.begin();
    for(;itr!= file_queue_.end(); itr++)
        transfer_map_[(*itr).path()] = *itr;
    file_queue_.clear();
    fq_mtx_.unlock();
    tm_mtx_.unlock();
}

void FileWatcher::ResetMarker(const std::string& filepath) {
    tm_mtx_.lock();
    TransferMap::iterator itr = transfer_map_.find(filepath);
    if(itr != transfer_map_.end())
        itr->second.ResetElapsed();                                 
    tm_mtx_.unlock();
}

void FileWatcher::SetMarkerOpen(const std::string& filepath) {
    tm_mtx_.lock();
    TransferMap::iterator itr = transfer_map_.find(filepath);
    if(itr != transfer_map_.end())
        itr->second.SetOpen();
    else {
        FileMarker marker(filepath); 
        transfer_map_[filepath] = marker;
        itr = transfer_map_.find(filepath);
        if(itr != transfer_map_.end()) { 
            itr->second.SetOpen();
        }
    }
    tm_mtx_.unlock();
}

void FileWatcher::SetMarkerClosed(const std::string& filepath) {
    tm_mtx_.lock();
    TransferMap::iterator itr = transfer_map_.find(filepath);
    if(itr != transfer_map_.end())
        itr->second.SetClosed();
    else {
        itr = transfer_map_.begin();
        for(;itr!= transfer_map_.end(); itr++) 
            std::cout<< "\t" << itr->first << std::endl;
    }
    tm_mtx_.unlock();
}

void FileWatcher::PushBack(const FileMarker& fm) {
    fq_mtx_.lock();
    file_queue_.push_back(fm);
    fq_mtx_.unlock();
}

void FileWatcher::set_running(bool r) { 
    r_mtx_.lock();
    running_ = r;
    r_mtx_.unlock();
}

bool FileWatcher::running() {
    r_mtx_.lock();
    bool r = running_;
    r_mtx_.unlock();
    return r;
}

