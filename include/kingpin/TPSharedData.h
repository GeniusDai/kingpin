#ifndef __TPSHAREDDARA_H_f2b87e623d0d_
#define __TPSHAREDDARA_H_f2b87e623d0d_

#include <mutex>
#include <condition_variable>
#include <map>
#include <memory>
#include <functional>

#include "kingpin/Buffer.h"

using namespace std;

namespace kingpin {

class TPSharedData {
public:
    TPSharedData() = default;
    TPSharedData(const TPSharedData &) = delete;
    TPSharedData& operator=(const TPSharedData &) = delete;
    virtual ~TPSharedData() {}
};

class ServerTPSharedData : public TPSharedData {
public:
    virtual ~ServerTPSharedData() {}
    int _port;
    // DONOT use this lock again unless you know what you're doing
    mutex _listenfd_lock;
    int _listenfd;
};

class ClientTPSharedData : public TPSharedData {
public:
    virtual ~ClientTPSharedData() {}
    using _t_host = pair<string, int>;
    struct _HostCompare {
        size_t _hash(const _t_host &s) const {
            return hash<string>()(s.first) * 100 + hash<int>()(s.second);
        }
        bool operator()(const _t_host &lhs, const _t_host &rhs) const {
            return _hash(lhs) < _hash(rhs);
        }
    };

    // DONOT use this lock again unless you know what you're doing
    mutex _pool_lock;
    condition_variable _cv;
    multimap<_t_host, string, _HostCompare> _pool;

    void add(string host, int port, string init) {
        {
            unique_lock<mutex> _tl(this->_pool_lock);
            raw_add(host, port, init);
        }
        _cv.notify_one();
    }

    void raw_add(string host, int port, string init) {
        _pool.emplace(make_pair<string &&, int &&>(move(host), move(port)), init);
    }
};

}

#endif
