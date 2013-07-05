#ifndef EVENTQUEUE_H_
#define EVENTQUEUE_H_
#pragma once

#include <deque>
#include <boost/thread/thread.hpp>
#include "watchevent.h"

class EventQueue {
public:
    typedef std::deque<WatchEvent> EventList;
    EventQueue() {}
    ~EventQueue() {}

    void PushBack(const WatchEvent event) {
        eq_mtx_.lock();
        event_queue_.push_back(event);
        eq_mtx_.unlock();
    }

    void CopyAndClear(EventList& out) {
        eq_mtx_.lock();
        EventList::iterator itr = event_queue_.begin();
        for(;itr!= event_queue_.end(); itr++) {
            out.push_back(*itr);
        }
        event_queue_.clear();
        eq_mtx_.unlock();
    }


private:
    boost::mutex eq_mtx_;
    EventList event_queue_;

};

#endif

