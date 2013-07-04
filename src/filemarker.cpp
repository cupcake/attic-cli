#include "filemarker.h"

#include <iostream>
#include <boost/timer/timer.hpp>

static boost::timer::nanosecond_type const limit(3 * 100000000LL); // 3 second in nano seconds

FileMarker::FileMarker(){
    state_ = START;
}

FileMarker::FileMarker(const std::string& path) {
    path_ = path;
    state_ = START;

}
FileMarker::~FileMarker() {}

void FileMarker::Reset() {
    elapsed_ = 0;
}

void FileMarker::Update(long long dt) {
    elapsed_ += dt;

    std::cout<<" update elapsed : " << elapsed_ << std::endl;
    if(elapsed_ > limit) {
        std::cout<<" FILE TRANSFER DONE : " << path_ << std::endl;
        // set close state
        state_ = DONE;
    }
}

