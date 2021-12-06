#ifndef __TPSharedData_H_
#define __TPSharedData_H_

#include <mutex>
#include <unordered_map>
#include <memory>

#include "kingpin/Buffer.h"

using namespace std;

class TPSharedData {
public:
    unordered_map<int, shared_ptr<Buffer> > _rbh;
    unordered_map<int, shared_ptr<Buffer> > _wbh;
    mutex _rbm;
    mutex _wbm;
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
