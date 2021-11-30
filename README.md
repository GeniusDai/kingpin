**Kingpin is a C++ network programming framework based on TCP/IP + epoll + pthread, aims to implement a library for the high concurrent servers and clients. Inspired by *nginx* / *muduo*.**

# Features

* Per epoll per thread and one connected socket handled only by one thread

* Thread pool and IO multiplexing for both server's and client's concurrency

* High performance server, multi-threads competing for mutex to register read event for listening socket and handle connected socket

* High performance client using multi-threads init and handle connected socket

* High performance asynchronous logger using backend thread to print debug info with timestamp and tid

* Avoid excessive wrapper of classes and functions to get things complecated

# Design Overview

![image](https://github.com/GeniusDai/kingpin/raw/dev/pictures/kingpin.001.png)

### EpollTPServer:

    1. Trying to get mutex from thread shared data, if it's locked, it won't block.

    2. If got the mutex, IO thread register EPOLLIN for the listening socket.

    3. Wait for epoll events.

    4. Handle the connected sockets.

    5. Get a new connected socket by accept syscall.

    6. Remove EPOLLIN for the listening socket.

    7. Release the mutex.

    8. Handle the new connected socket.

### EpollTPClient:

    1. Init the connected sockets by connect syscall.

    2. Register the connected sockets to epoll.

    3. Wait for the epoll events.

    4. Handle the connected sockets.

![image](https://github.com/GeniusDai/kingpin/raw/dev/pictures/kingpin.002.png)

### Async Logger:

    1. Get mutex for the log buffer, if it's locked, it will block.

    2. Write log to the buffer.

    3. Release the mutex.

    4. Notify the corresponding backend thread.

    5. Backend thread being waked up by the condition variable.

    6. Backend thread trying to get the mutex, if it's locked, sleep until next time being notified.

    7. If get mutex, read from the buffer.

    8. Write buffer to the corresponding fd.

    9. Release the mutex.

# Quick Start

Please refer to: [Quick Start](https://github.com/GeniusDai/kingpin/tree/dev/examples/quick_start)

# Prerequirement

* Linux >= 3.10.0

* make >= 3.82

* g++ >= 4.8.5

* c++11 or higher

* link posix thread lib

# Header Files

* IOHandler.h: Virtual base class got a epoll fd, derived class shall implement handler functions for IO.

* TPSharedData.h: Data shared among threads.

* EpollTP.h: Thread pool, initialized by IOHandler and TPSharedData.

* Logger.h: Multi-thread safe logger, using backend thread for asynchronous output, line buffered.

* Buffer.h: IO Buffer pre-allocated on heap, support nonblock socket and disk fd. NOT thread safe.

* Utils.h: Some utility functions.

# Examples

* [Chinese Chess Game](https://github.com/GeniusDai/kingpin/tree/dev/examples/chinese_chess_game): Chess game server and client, also implement a high concurrency test client.

* [Big File Transfer](https://github.com/GeniusDai/kingpin/tree/dev/examples/big_file_transfer): Big file transfer server and client, IO and exceptions are both well handled.
