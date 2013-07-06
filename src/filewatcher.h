#ifndef FILEWATCHER_H_
#define FILEWATCHER_H_

#include <deque>
#include <boost/timer/timer.hpp>
#include <boost/thread/thread.hpp>

#include "filemarker.h"
/* This watcher keeps track of file transfers, and calls into the lib when
 * they are supposedly complete
 */

class FileWatcher {
    void Run();
    void ProcessFileQueue();
    void Update();
public:
    FileWatcher();
    ~FileWatcher();

    void Initialize();
    void Shutdown();

    void SetMarkerOpen(const std::string& filepath);
    void SetMarkerClosed(const std::string& filepath);
    void ResetMarker(const std::string& filepath);
    void PushBack(const FileMarker& fm);

    void set_running(bool r);
    bool running();
private:
    typedef std::map<std::string, FileMarker>  TransferMap;
    boost::mutex tm_mtx_;
    TransferMap transfer_map_; // queue for large file transfers

    boost::mutex fq_mtx_;
    std::deque<FileMarker> file_queue_;

    boost::mutex r_mtx_;
    bool running_;

    boost::thread* thread_;
    boost::timer::cpu_timer timer_; 
};

#endif

