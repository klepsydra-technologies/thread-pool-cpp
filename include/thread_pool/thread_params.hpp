#pragma once

#include <atomic>
#include <thread>
#include <string>
#include <vector>

#if defined(__unix__) || defined(__rtems__)
#include <pthread.h>
#endif

namespace tp
{

class ThreadParams
{
public:
    ThreadParams();

    explicit ThreadParams(const std::string& name, const std::vector<int>& cpuset);

    std::string& getName();
    std::vector<int>& getCpuAffinity();

private:
    std::string name_;

    std::vector<int> cpuset_;
};

/// Implementation
inline ThreadParams::ThreadParams()
    : name_("")
    , cpuset_()
{}

inline ThreadParams::ThreadParams(const std::string& name, const std::vector<int>& cpuset)
    : name_(name)
    , cpuset_(cpuset)
{}

inline std::string& ThreadParams::getName() {
    return name_;
}

inline std::vector<int>& ThreadParams::getCpuAffinity() {
    return cpuset_;
}
}
