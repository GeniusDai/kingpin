#ifndef __ThreadSharedData_H_
#define __ThreadSharedData_H_

#include <mutex>
#include <unordered_set>

using namespace std;

class ThreadSharedData {
public:
    virtual ~ThreadSharedData() {}
};

class ThreadSharedDataServer : public ThreadSharedData {
public:
    // DONOT use this lock again unless you know what you're doing
    mutex _listenfd_lock;
    int _listenfd;
    virtual ~ThreadSharedDataServer() {}
};

class ThreadSharedDataClient : public ThreadSharedData {
public:
    mutex _m;
    unordered_set<int> _connfds;
    virtual ~ThreadSharedDataClient() {}
};

#endif
