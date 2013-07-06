#include "libcaller.h"

#include <boost/filesystem.hpp>
#include "libattic.h"

static boost::mutex inst_mtx_;
LibCaller* LibCaller::instance_ = 0;

LibCaller::LibCaller() {
    ignore_map_[".DS_Store"] = true;
}

LibCaller::~LibCaller() {}

LibCaller* LibCaller::instance() {
    boost::mutex::scoped_lock lock(inst_mtx_);
    if(!instance_) 
        instance_ = new LibCaller();
    LibCaller* hold = instance_;
    return hold;
}

void LibCaller::Shutdown() {
    inst_mtx_.lock();
    if(instance_) {
        delete instance_;
        instance_ = NULL;
    }
    inst_mtx_.unlock();
}

void LibCaller::LibUploadFile(const std::string& filepath) {
    // Check ignore map
    if(!IsHiddenFile(filepath) && !IsIgnored(filepath)) {
        // Make sure file exists
        if(boost::filesystem::exists(filepath)) {
            // Upload
            PushFile(filepath.c_str());
        }
    }
}

void LibCaller::LibRenameFile(const std::string& old_path, const std::string& new_path) {
    if(!IsHiddenFile(new_path))
        RenameFile(old_path.c_str(), new_path.c_str());
}

void LibCaller::LibCreateFolder(const std::string& folderpath) {
    if(boost::filesystem::exists(folderpath))
        CreateFolder(folderpath.c_str());
}

void LibCaller::LibRenameFolder(const std::string& old_path, const std::string& new_path) {
    RenameFolder(old_path.c_str(), new_path.c_str());
}

void LibCaller::AddIgnore(const std::string& str) {
    ig_mtx_.lock();
    ignore_map_[str] = true;
    ig_mtx_.unlock();
}

void LibCaller::RemoveIgnore(const std::string& str) {
    ig_mtx_.lock();
    std::map<std::string, bool>::iterator itr = ignore_map_.find(str);
    if(itr!=ignore_map_.end()) 
        ignore_map_.erase(itr);
    ig_mtx_.unlock();
}

bool LibCaller::IsIgnored(const std::string& str) {
    bool ret = false;
    std::string hold;
    hold.append(str.c_str(), str.size());
    ig_mtx_.lock();
    std::map<std::string, bool>::iterator itr = ignore_map_.begin();
    for(;itr!=ignore_map_.end();itr++) {
        if(hold.rfind(itr->first) != std::string::npos) {
            ret = true;
            break;
        }
    }
    ig_mtx_.unlock();
    return ret;
}

bool LibCaller::IsHiddenFile(const std::string& path) {
    bool ret = true;
    size_t pos = path.rfind("/");
    if(pos != std::string::npos) {
        if(path[pos+1] != '.')
            ret = false;
    }
    return ret;
}

