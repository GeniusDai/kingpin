**Kingpin is a C++ network programming framework based on TCP/IP + epoll + pthread, aims to implement a library for the high concurrent servers and clients. Inspired by *nginx* / *muduo*.**

# Features

* Per epoll per thread, one socket handled by one thread

* Thread pool and epoll for both server's and client's concurrency

* High performance server using multi-threads competing for mutex to register read event for listening socket

* High performance client using multi-threads competing for mutex to get file descriptor from connection pool

* High performance asynchronous logger using backend thread to print debug info with timestamp and tid

* Avoid excessive wrapper of classes and functions to get thing complecated

# Repository Contents

Guide to kingpin header files.

core:

* IOHandler.h: Virtual base class got a epoll fd, derived class shall implement run function for thread and handler functions for IO.

* IOHandlerServer.h: Virtual base class inherited from IOHandler, register read event for listening socket and wait for read/write IO.

* IOHandlerClient.h: Virtual base class inherited from IOHandler, compete for connection pool and wait for read/write IO.

* ThreadShareData.h: Data shared among threads.

* EpollTP.h: Thread pool, initialized by multi-thread shared data.

* Logger.h: Multi-thread safe logger, use backend thread, will end elegantly in the destruction.

# Todo

* Web crawler for demo
