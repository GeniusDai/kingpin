**Kingpin is a C++ network programming framework based on TCP/IP + epoll + pthread, aims to implement a library for the high concurrent servers and crawlers. Inspired by *nginx* / *muduo*.**

# Features

* Per epoll per thread, one socket handled by one thread

* Server: Multi-threads competing for mutex to register read event for listening socket

* Client: Multi-threads competing for mutex to get fd from connection pool

* Avoid excessive wrapper of classes and functions to get thing complecated

# Repository Contents

Guide to header files:

* IOHandler.h: Virtual base class got a epoll fd, derived class shall implement run function for thread and handler functions for IO.

* IOHandlerServer.h: Virtual base class inherited from IOHandler, register read event for listening socket and wait for read/write IO.

* IOHandlerClient.h: Virtual base class inherited from IOHandler, compete for connection pool and wait for read/write IO.

* ThreadShareData.h: Data shared among threads.

* EpollTP.h: Thread pool, initialized by multi-thread shared data.

# Todo

* Thread safe logger

* Crawler related

**Please notice kingpin is still in development!**
