#ifndef FILEMARKER_H_
#define FILEMARKER_H_
#pragma once

#include <string>

class FileMarker {
    enum state {
        START = 0,
        IS_OPEN,
        IS_CLOSED,
        DONE
    };

public:
    FileMarker();
    FileMarker(const std::string& path);
    ~FileMarker();
    void Update(long dt);
    void SetOpen() { state_ = IS_OPEN; ResetElapsed(); }
    void SetClosed() { state_ = IS_CLOSED; ResetElapsed(); }
    void ResetElapsed();
    bool IsDone() { if(state_ == DONE) return true; return false; }

    const std::string& path() const { return path_; }
private:
    state state_;
    long long elapsed_;
    std::string path_;
};

#endif

