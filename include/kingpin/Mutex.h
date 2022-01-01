#include <mutex>
#include <pthread.h>
#include <unordered_set>
#include <cassert>
#include "kingpin/Utils.h"

using namespace std;

namespace kingpin {

class RWLock final {
    pid_t _wr_owner;
#ifdef _KINGPIN_DEBUG_
    unordered_set<pid_t> _rd_owner;
    mutex _debug_lock;
#endif
public:
    enum LOCK_TYPE {
        READ,
        WRITE
    };
    shared_ptr<pthread_rwlock_t> __rw_lock;
    pthread_rwlock_t *_rw_lock;

    LOCK_TYPE _default_lock = LOCK_TYPE::READ;

    RWLock() {
        __rw_lock = unique_ptr<pthread_rwlock_t> (new pthread_rwlock_t);
        _rw_lock = __rw_lock.get();
        if (::pthread_rwlock_init(_rw_lock, nullptr) != 0)
            { fatalError("syscall pthread_rwlock_init error"); }
    }
    RWLock(const RWLock &) = delete;
    RWLock &operator=(const RWLock &) = delete;
    ~RWLock() {
        if (::pthread_rwlock_destroy(_rw_lock) < 0)
            { fatalError("syscall pthread_rwlock_destroy error"); }
    }

    void lock() {
        if (LOCK_TYPE::READ == _default_lock) { rd_lock(); }
        else { wr_lock(); }
    }
    void unlock() {
        if (_wr_owner == gettid()) { _wr_owner = 0; }
#ifdef _KINGPIN_DEBUG_
        {
            unique_lock<mutex> lock(_debug_lock);
            _rd_owner.erase(gettid());
        }
#endif
        if (::pthread_rwlock_unlock(_rw_lock) != 0)
            { fatalError("syscall pthread_rwlock_unlock error"); }
    }
    void rd_lock() {
        if (_wr_owner == gettid()) { fatalError("cannot rd_lock() when already wr_lock()"); }
        if (::pthread_rwlock_rdlock(_rw_lock) != 0)
            { fatalError("syscall pthread_rwlock_rdlock error"); }
        assert(_wr_owner == 0);
#ifdef _KINGPIN_DEBUG_
        {
            unique_lock<mutex> lock(_debug_lock);
            _rd_owner.insert(gettid());
        }
#endif
    }
    void wr_lock() {
        if (::pthread_rwlock_wrlock(_rw_lock) != 0)
            { fatalError("syscall pthread_rwlock_wrlock error"); }
#ifdef _KINGPIN_DEBUG_
        assert(_rd_owner.empty());
#endif
        _wr_owner = gettid();
    }
};

class WRLockGuard {
    RWLock *_lock;
public:
    WRLockGuard(RWLock &m) { _lock = &m; _lock->wr_lock(); }
    WRLockGuard(const WRLockGuard &) = delete;
    WRLockGuard &operator=(const WRLockGuard &) = delete;
    ~WRLockGuard() { _lock->unlock(); }
    void lock() { _lock->wr_lock(); }
    void unlock() { _lock->unlock(); }
};

class RDLockGuard {
    RWLock *_lock;
public:
    RDLockGuard(RWLock &m) { _lock = &m; _lock->rd_lock(); }
    RDLockGuard(const RDLockGuard &) = delete;
    RDLockGuard &operator=(const RDLockGuard &) = delete;
    ~RDLockGuard() { _lock->unlock(); }
    void lock() { _lock->rd_lock(); }
    void unlock() { _lock->unlock(); }
};

class RecursiveLock {
    pthread_mutex_t _lock;
    pid_t _owner;
    int _count;
public:
    RecursiveLock() {
        pthread_mutexattr_t attr;
        if (::pthread_mutexattr_init(&attr) != 0) {
            fatalError("syscall pthread_mutexattr_init error");
        }
        if (::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
            fatalError("syscall pthread_mutexattr_settype error");
        }
        if (::pthread_mutex_init(&_lock, &attr) != 0) {
            fatalError("syscall pthread_mutex_init error");
        }
        if (::pthread_mutexattr_destroy(&attr) != 0) {
            fatalError("syscall pthread_mutexattr_destroy error");
        }
    }
    RecursiveLock(const RecursiveLock &) = delete;
    RecursiveLock &operator=(const RecursiveLock &) = delete;
    ~RecursiveLock() {
        if (::pthread_mutex_destroy(&_lock) != 0) {
            fatalError("syscall pthread_mutex_destory error");
        }
    }
    void lock() {
        if (::pthread_mutex_lock(&_lock) != 0) {
            fatalError("syscall pthread_mutex_lock error");
        }
        _owner = gettid();
        _count++;
    }
    void unlock() {
        if (--_count == 0) { _owner = 0; }
        if (::pthread_mutex_unlock(&_lock) != 0) {
            fatalError("syscall pthread_mutex_unlock error");
        }
    }
};


}