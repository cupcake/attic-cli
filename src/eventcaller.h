#ifndef EVENTCALLER_H_
#define EVENTCALLER_H_
#pragma once

#include <deque>
#include <boost/timer/timer.hpp>
#include <boost/thread/thread.hpp>
#include "watchevent.h"
#include "eventqueue.h"
#include "filemarker.h"

class EventCaller {
    struct move_event {
        std::string from_dir;
        std::string to_dir;
    };

    void Run();
    void RetrieveEvents();
    void ProcessEventQueue();
    void ProcessEvent(const WatchEvent& event);
    void UpdateTransferMap();
public:
    EventCaller(EventQueue* eq);
    ~EventCaller();

    void Initialize();
    void Shutdown();

    void set_running(bool r);
    bool running();

private:
    // filepath, file marker
    typedef std::map<std::string, FileMarker>  TransferMap;
    // cookie, move event
    typedef std::map<int, move_event> RenameMap;

    RenameMap rename_map_;
    TransferMap transfer_map_; // queue for large file transfers

    std::deque<WatchEvent> event_queue_;

    boost::mutex r_mtx_;
    bool running_;
    boost::thread* thread_;
    boost::timer::cpu_timer timer_; 

    EventQueue* eq_;
};

#endif


