#ifndef __TPSharedData_H_
#define __TPSharedData_H_

#include <mutex>
#include <unordered_set>

using namespace std;

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
    virtual ~ClientTPSharedData() {}
};

#endif
