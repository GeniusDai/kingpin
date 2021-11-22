#ifndef __ThreadShareData_H_
#define __ThreadShareData_H_

#include <mutex>
#include <unordered_set>

using namespace std;

class ThreadShareData {};

class ThreadShareDataServer : public ThreadShareData {
public:
    mutex _m;
    int _listenfd;
};

class ThreadSahreDataClient : public ThreadShareData {
public:
    mutex _m;
    unordered_set<int> _connfds;
};

#endif
