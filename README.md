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

Procedure for the EpollTPServer:

1. Trying to get mutex from thread shared data, if it's locked, it won't block.

2. If got the mutex, IO thread register EPOLLIN for the listening socket.

3. Wait for epoll events.

4. Handle the connected sockets.

5. Using the accept syscall for the listening socket to get one connected socket.

6. Register the connected sockets.

7. Release the mutex.

Procedure for the EpollTPClient:

1. Init the connected sockets using the connect syscall.

2. Register the connected sockets.

3. Wait for the epoll events.

4. Handle the connected sockets.

![image](https://github.com/GeniusDai/kingpin/raw/dev/pictures/kingpin.002.png)

Procedure for the async logger:

1. Get mutex for the log buffer, if it's locked, it will block.

2. Write log to the buffer.

3. Release the mutex.

4. Notify the corresponding backend thread.

5. Backend thread being waked up by the condition variable.

6. Backend thread trying to get the mutex, if it's locked, it will sleep until next time being notified.

7. If get mutex, read from the buffer.

8. Write buffer to the corresponding fd.

9. Release the mutex.

# Repository Contents

Guide to kingpin header files:

* IOHandler.h: Virtual base class got a epoll fd, derived class shall implement run function for thread and handler functions for IO.

* ThreadSharedData.h: Data shared among threads.

* EpollTP.h: Thread pool, initialized by multi-thread shared data.

* Logger.h: Multi-thread safe logger, using backend thread for asynchrous output, line buffered.

* Buffer.h: Buffer preallocated on heap, NOT thread safe.

* Utils.h: Some utility functions.

Examples that use kingpin framework:

* [chinese_chess_game](https://github.com/GeniusDai/kingpin/tree/dev/examples/chinese_chess_game)

# Todo List

Demo:

* Web crawler

* Big file download server
