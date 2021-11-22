**Kingpin is a C++ network programming framework based on TCP/IP + epoll + pthread, aims to implement a library for the high concurrent servers and crawlers. Inspired by *nginx* / *muduo*.**

# Features

* Per epoll per thread, one socket handled by one thread

* Multi-threads competing for mutex to register read event for listening socket

# Repository Contents

Guide to header files:

* IOHandler.h: Virtual base class got a epoll fd, derived class shall implement IO run function and handler functions.

* IOHandlerServer.h: Virtual base class inherited from IOHandler, implemented run function, will register read event for listening socket.

* IOHandlerClient.h: Virtual base class inherited from IOHandler, implemented run function, no listening socket.

* ThreadShareData.h: Data shared among threads.

* EpollTP.h: Thread poll, thread will use derived class of IOHandler.

* EpollTPServer.h: Implement listening socket and transmit to derived class of ThreadShareData.

* EpollTPClient.h: No essential data for transmitting to derived class of ThreadShareData.

# Todo

* Thread safe logger

* Crawler related

**Please notice kingpin is still in development!**
