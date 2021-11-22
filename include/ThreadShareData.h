#ifndef __ThreadShareData_H_
#define __ThreadShareData_H_

#include <mutex>

using namespace std;

class ThreadShareData {};

class ThreadShareDataServer : public ThreadShareData {
public:
    mutex _m;
    int _listenfd;
};


#endif
