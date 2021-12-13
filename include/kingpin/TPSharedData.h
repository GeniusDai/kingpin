#ifndef __TPSHAREDDARA_H_f2b87e623d0d_
#define __TPSHAREDDARA_H_f2b87e623d0d_

#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>

#include "kingpin/Buffer.h"

using namespace std;

namespace kingpin {

class TPSharedData {
public:
    virtual ~TPSharedData() {}
};

class ServerTPSharedData : public TPSharedData {
public:
    // DONOT use this lock again unless you know what you're doing
    mutex _listenfd_lock;
    int _listenfd;

    virtual ~ServerTPSharedData() {}
};

class ClientTPSharedData : public TPSharedData {
public:
    struct _Hash {
        size_t operator()(const pair<string, int> &elem) const {
            return hash<string>()(elem.first) + hash<int>()(elem.second);
        }
    };

    // DONOT use this lock again unless you know what you're doing
    mutex _pool_lock;
    condition_variable _cv;
    unordered_multimap<pair<string, int>, string, _Hash> _pool;

    virtual ~ClientTPSharedData() {}
};

}

#endif
