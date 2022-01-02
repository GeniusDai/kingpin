**Kingpin is a C++ network library based on TCP/IP + epoll + pthread for the high concurrent servers and clients. Inspired by *nginx* / *muduo*.**

# Features

* One connected socket handled only by one thread.

* Thread pool and IO multiplexing for both server's and client's concurrency.

* IO threads compete for listening socket or target host pool in server and client.

* Avoid excessive wrapper of classes and functions to get things complicated.

# Quick Start

Please refer to: [Quick Start](https://github.com/GeniusDai/kingpin/tree/dev/examples/quick_start)

# Prerequirement

* Linux: Support epoll, and glibc > 2.15 to avoid pthread_rwlock deadlock bug!

* g++: Support C++ 11.

* googletest / cmake / make.

# Header Files

* IOHandler.h: Both server's and client's IO thread.

* TPSharedData.h: Data shared among thread pool.

* EpollTP.h: Thread pool initialized by IOHandler and TPSharedData.

* AsyncLogger.h: Thread safe logger, using backend thread for asynchronous output, line buffered.

* Buffer.h: IO Buffer pre-allocated on heap, support nonblock socket and disk fd. NOT thread safe.

* Mutex.h: Easy to debug wrapper for read-write lock and recursive lock in libpthread.

* Context.h: Function call timeout wrapper.

* Utils.h: Some utility functions.

# Examples

A wealth of cases:

* [Chinese Chess Game](https://github.com/GeniusDai/kingpin/tree/dev/examples/chinese_chess_game): Chess game server and client, also implements a high concurrency test client.

* [File Transfer](https://github.com/GeniusDai/kingpin/tree/dev/examples/file_transfer): File transfer server and client, IO and exceptions are both well handled.

Build and run:

    $ mkdir build && cd build
    $ cmake .. && make
    $ cd unittest && ctest
