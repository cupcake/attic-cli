#include "filemarker.h"

#include <iostream>
#include <boost/timer/timer.hpp>

static boost::timer::nanosecond_type const limit(3 * 100000000LL); // 3 second in nano seconds

FileMarker::FileMarker(){
    state_ = START;
    elapsed_ = 0;
}

FileMarker::FileMarker(const std::string& path) {
    path_ = path;
    state_ = START;
    elapsed_ = 0;
}
FileMarker::~FileMarker() {}

void FileMarker::ResetElapsed() {
    std::cout<<" reseting marker for file : " << path_ << std::endl;
    elapsed_ = 0;
}

void FileMarker::Update(long dt) {
    elapsed_ += dt;
    std::cout<<" update elapsed : " << elapsed_ << std::endl;
    if(elapsed_ > limit && (state_ == IS_CLOSED)) {
        std::cout<<" FILE TRANSFER DONE : " << path_ << std::endl;
        // set close state
        state_ = DONE;
        std::cout<<" am i done ? : " << IsDone() << std::endl;
    }
    else {
        std::cout<<" CURRENT STATE : " << state_ << std::endl;
        ResetElapsed();
    }
}

