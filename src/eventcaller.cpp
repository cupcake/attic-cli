#include "eventcaller.h"

EventCaller::EventCaller() {
    thread_ = NULL;
    running_ = false;
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

void EventCaller::Run() {
    while(running()) {

    }
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

