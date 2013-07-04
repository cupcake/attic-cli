#ifndef FILEMARKER_H_
#define FILEMARKER_H_
#pragma once

#include <string>

class FileMarker {
    enum state {
        START = 0,
        IN_PROGRESS,
        DONE
    };

public:
    FileMarker();
    FileMarker(const std::string& path);
    ~FileMarker();
    void Update(long long dt);
    void Reset();
    bool IsDone() { if(state_ == DONE) return true; return false; }
private:
    state state_;
    long long elapsed_;
    std::string path_;
};

#endif

