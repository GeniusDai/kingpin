#ifndef __ThreadSharedData_H_
#define __ThreadSharedData_H_

#include <mutex>
#include <unordered_set>

using namespace std;

class ThreadSharedData {
public:
    virtual ~ThreadSharedData() {}
};

class ServerTPSharedData : public ThreadSharedData {
public:
    // DONOT use this lock again unless you know what you're doing
    mutex _listenfd_lock;
    int _listenfd;
    virtual ~ServerTPSharedData() {}
};

class ClientTPSharedData : public ThreadSharedData {
public:
    virtual ~ClientTPSharedData() {}
};

#endif
