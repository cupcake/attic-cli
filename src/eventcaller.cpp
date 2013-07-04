#include "eventcaller.h"

#include <boost/filesystem.hpp>

EventCaller::EventCaller(EventQueue* eq) {
    thread_ = NULL;
    running_ = false;
    eq_ = eq;
}

EventCaller::~EventCaller() {}

void EventCaller::Initialize() {
    if(!thread_) {
        thread_ = new boost::thread(&EventCaller::Run, this);
        set_running(true);
    }
}

void EventCaller::Shutdown() {
    if(thread_) {
        set_running(false);
        thread_->join();
        delete thread_;
        thread_ = NULL;
    }
}

void EventCaller::RetrieveEvents() {
    EventQueue::EventList queue;
    eq_->CopyAndClear(queue);

    EventQueue::EventList::iterator itr = queue.begin();
    for(;itr!=queue.end();itr++)
        event_queue_.push_back(*itr);
}

void EventCaller::Run() {
    timer_.start();
    while(running()) {
        // Retrieve Events
        RetrieveEvents();
        // Process Events
        ProcessEventQueue();
        //Update transfermap
        UpdateTransferMap();

        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }
    timer_.stop();
}

void EventCaller::UpdateTransferMap() {
    boost::timer::cpu_times time = timer_.elapsed();                     
    boost::timer::nanosecond_type const elapsed(time.user);

    std::cout<<" transfer map size : " << transfer_map_.size() << std::endl;
    TransferMap::iterator itr = transfer_map_.begin();
    for(;itr!=transfer_map_.end();itr++)
        itr->second.Update(elapsed);
}
void EventCaller::ProcessEventQueue() {
    // copy everything out of event queue
    std::deque<WatchEvent>::iterator itr = event_queue_.begin();
    for(;itr!=event_queue_.end(); itr++) { 
        ProcessEvent(*itr);
        std::deque<WatchEvent>::iterator hold = (itr-1);
        event_queue_.erase(itr);
        itr = hold;
    }
}

void EventCaller::ProcessEvent(const WatchEvent& event) {
    std::cout<<" ProcessEvent ********************************************" << std::endl;
    if (event.event()->mask & IN_ACCESS) {
        std::cout<<" IN_ACCESS " << std::endl;
        if (event.event()->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event.event()->name << std::endl;
        else std::cout<<"\t (file) :" << event.event()->name << std::endl;
    }
    if (event.event()->mask & IN_ATTRIB) {
        std::cout<<" IN_ATTRIB " << std::endl;
        if (event.event()->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event.event()->name << std::endl;
        else std::cout<<"\t (file) :" << event.event()->name << std::endl;    
    }
    if (event.event()->mask & IN_CLOSE_WRITE) {
        std::cout<<" IN_CLOSE_WRITE " << std::endl;
        if (event.event()->mask & IN_ISDIR) {
            std::cout<<"\t (dir) : " << event.event()->name << std::endl;
        }
        else {
            std::cout<<"\t (file) :" << event.event()->name << std::endl;    
            std::string filepath = event.directory();
            filepath += std::string("/") + std::string(event.event()->name);
            TransferMap::iterator itr = transfer_map_.find(filepath);
            if(itr != transfer_map_.end())
                itr->second.Reset();
        }
    }
    if (event.event()->mask & IN_CLOSE_NOWRITE) {
        std::cout<<" IN_CLOSE_NOWRITE " << std::endl;
        if (event.event()->mask & IN_ISDIR) {
            std::cout<<"\t (dir) : " << event.event()->name << std::endl;
        }
        else {
            std::cout<<"\t (file) :" << event.event()->name << std::endl;    
            std::string filepath = event.directory();
            filepath += std::string("/") + std::string(event.event()->name);
            TransferMap::iterator itr = transfer_map_.find(filepath);
            if(itr != transfer_map_.end())
                itr->second.Reset();
        }
    }
    if (event.event()->mask & IN_CREATE) {
        std::cout<<" IN_CREATE " << std::endl;
        if (event.event()->mask & IN_ISDIR) {
            std::cout<<"\t (dir) : " << event.event()->name << std::endl;   
            // create directory path
            std::string dir = event.directory();
            dir += std::string("/") + std::string(event.event()->name);
            std::cout<<" new dir : " << dir << std::endl;
            // add to watch
            boost::filesystem::path root(dir);
            if(boost::filesystem::exists(root)) {
                //WatchDirectoryDirectly(dir);
            }
        }
        else {
            std::string filepath = event.directory();
            filepath += std::string("/") + std::string(event.event()->name);
            std::cout<<" filepath : "<< filepath << std::endl;
            FileMarker marker(filepath);
            transfer_map_[filepath] = marker;
        }
    }
    if (event.event()->mask & IN_DELETE) {
        std::cout<<" IN_DELETE " << std::endl;
        if (event.event()->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event.event()->name << std::endl;
        else std::cout<<"\t (file) :" << event.event()->name << std::endl;    
    }
    if (event.event()->mask & IN_DELETE_SELF) {
        std::cout<<" IN_DELETE_SELF " << std::endl;
        if (event.event()->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event.event()->name << std::endl;
        else std::cout<<"\t (file) :" << event.event()->name << std::endl;    
    }
    if (event.event()->mask & IN_MODIFY) {
        std::cout<<" IN_MODIFY " << std::endl;
        if (event.event()->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event.event()->name << std::endl;
        else std::cout<<"\t (file) :" << event.event()->name << std::endl;    
    }
    if (event.event()->mask & IN_MOVE_SELF) {
        std::cout<<" IN_MOVE_SELF " << std::endl;
        if (event.event()->mask & IN_ISDIR) std::cout<<"\t (dir) : " << event.event()->name << std::endl;
        else std::cout<<"\t (file) :" << event.event()->name << std::endl;    
    }
    if (event.event()->mask & IN_MOVED_FROM) {
        // remove from watching
        std::cout<<" IN_MOVED_FROM " << std::endl;
        if (event.event()->mask & IN_ISDIR) {
            std::cout<<"\t (dir) : " << event.event()->name << std::endl;
            std::string dir = event.directory();
            dir += std::string("/") + std::string(event.event()->name);
            rename_map_[event.event()->cookie].from_dir = dir;
            //UnwatchDirectory(wd);
        }
        else {
            std::cout<<"\t (file) :" << event.event()->name << std::endl;    
            std::cout<<" cookie : " << event.event()->cookie << std::endl;
        }
    }
    if (event.event()->mask & IN_MOVED_TO) {
        // add to watching
        std::cout<<" IN_MOVED_TO " << std::endl;
        if (event.event()->mask & IN_ISDIR) {
            std::cout<<"\t (dir) : " << event.event()->name << std::endl;
            std::string dir = event.directory();
            dir += std::string("/") + std::string(event.event()->name);
            rename_map_[event.event()->cookie].to_dir = dir;
            std::cout<<" move event complete ... " << std::endl;
            std::cout<<"\t moved dir from : " << rename_map_[event.event()->cookie].from_dir << std::endl;
            std::cout<<"\t moved dir to : " << rename_map_[event.event()->cookie].to_dir << std::endl;
            //WatchDirectoryDirectly(dir);
        }
        else {
            std::cout<<"\t (file) :" << event.event()->name << std::endl;    
            std::cout<<" cookie : " << event.event()->cookie << std::endl;
        }
        // remove from rename_map_
    }

    if (event.event()->mask & IN_OPEN) {
        std::cout<<" IN_OPEN " << std::endl;
        if (event.event()->mask & IN_ISDIR) {
            std::cout<<"\t (dir) : " << event.event()->name << std::endl;
        }
        else {
            std::cout<<"\t (file) :" << event.event()->name << std::endl;    
            std::string filepath = event.directory();
            filepath += std::string("/") + std::string(event.event()->name);
            TransferMap::iterator itr = transfer_map_.find(filepath);
            if(itr != transfer_map_.end())
                itr->second.Reset();
        }
    }
    std::cout<<" *********************************************************" << std::endl;
}
void EventCaller::set_running(bool r) { 
    r_mtx_.lock();
    running_ = r;
    r_mtx_.unlock();
}

bool EventCaller::running() {
    r_mtx_.lock();
    bool r = running_;
    r_mtx_.unlock();
    return r;
}

