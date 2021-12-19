#ifndef __TPSHAREDDARA_H_f2b87e623d0d_
#define __TPSHAREDDARA_H_f2b87e623d0d_

#include <mutex>
#include <condition_variable>
#include <map>
#include <memory>
#include <functional>
#include <tuple>
#include <vector>
#include <cassert>
#include "kingpin/Buffer.h"

using namespace std;

namespace kingpin {

class TPSharedData {
public:
    TPSharedData() = default;
    TPSharedData(const TPSharedData &) = delete;
    TPSharedData& operator=(const TPSharedData &) = delete;
    virtual ~TPSharedData() {}

    // Config Param
    int _max_client_per_thr = 2048;
    int _ep_timeout = 1;
};

class ServerTPSharedData : public TPSharedData {
public:
    virtual ~ServerTPSharedData() {}
    int _port;
    // DONOT use this lock again unless you know what you're doing
    mutex _listenfd_lock;
    int _listenfd;

    // Config Param
    int _listen_num = 1024;
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
    map<_t_host, vector<string>, _HostCompare> _pool;

    // Config Param
    int _batch = 1;
    int _ep_timeout_conn = 1;

    void raw_add(string host, int port, string init) {
        pair<string, int> target = make_pair<string &&, int &&>(move(host), move(port));
        _pool[target].emplace_back(init);
    }

    void add(string host, int port, string init) {
        {
            unique_lock<mutex> _tl(this->_pool_lock);
            raw_add(host, port, init);
        }
        _cv.notify_all();
    }

    tuple<string, int, string> raw_get() {
        assert(this->_pool.size() > 0);
        auto iter = _pool.begin();
        string host = iter->first.first;
        int port = iter->first.second;
        string init = iter->second.back();
        iter->second.pop_back();
        if (iter->second.size() == 0) { _pool.erase(iter->first); }
        return make_tuple(move(host), move(port), move(init));
    }

    tuple<string, int, string> get() {
        unique_lock<mutex> _tl(this->_pool_lock);
        return move(raw_get());
    }
};

}

#endif
