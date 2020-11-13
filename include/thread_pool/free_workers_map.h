#ifndef FREE_WORKERS_MAP_H
#define FREE_WORKERS_MAP_H

#include <map>
#include <mutex>

#include <spdlog/spdlog.h>

namespace tp {
class FreeWorkersMap
{
public:
    FreeWorkersMap() {}

    void setFree(size_t id, bool isFree) {
        std::lock_guard<std::mutex> guard(_workerLock);
        spdlog::debug("Setting worker with id : {}, to status free = {}", id, isFree);
        _freeWorkersMap[id] = isFree;
    }

    bool findFreeWorker(size_t & id) {
        std::lock_guard<std::mutex> guard(_workerLock);
        auto it = std::find_if(_freeWorkersMap.begin(), _freeWorkersMap.end(),
                               [](std::map<size_t, bool>::value_type item)->bool {
                                   return item.second;});
        if (it == _freeWorkersMap.end()) {
            return false;
        } else {
            id = (*it).first;
            return true;
        }
    }

private:
    std::map<size_t, bool> _freeWorkersMap;
    std::mutex _workerLock;

};
}

#endif // FREE_WORKERS_MAP_H
