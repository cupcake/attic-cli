#ifndef EVENTCALLER_H_
#define EVENTCALLER_H_
#pragma once

#include <deque>
#include <boost/thread/thread.hpp>
#include "watchevent.h"

class EventCaller {
    void Run();
public:
    EventCaller();
    ~EventCaller();

    void Initialize();
    void Shutdown();

    void set_running(bool r);
    bool running();
private:
    boost::mutex eq_mtx_;
    std::deque<WatchEvent> event_queue_;

    boost::mutex r_mtx_;
    bool running_;
    boost::thread* thread_;
};

#endif


