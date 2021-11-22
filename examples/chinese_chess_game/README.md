# Chinese Chess Game

About
-----

Implemented by *C++*

Pre-requirement
---------------

* Linux OS: For epoll

* g++ >= 4.8.5

* make >= 3.82

Build and Run
-------------

To build the project:

    % make build

To start the Server:

    % make run-server

To start the Client:

    % make run-client

Default:

* TCP Port: "8889"

* Server IP: "127.0.0.1"

Screenshot
----------

![image](https://github.com/GeniusDai/ChineseChessGame/raw/main/pictures/PlayChess.png)

TODO
----

High Priority:

* Data is not multi-thread safe

* Implement thread safe logger

* Exception handler

Low Priority:

* Validity of chess be accessed

* Complie source code to dynamic libs

* TCP/IP be encrypted by SSL/TLS

* Implement GUI by QT

* Add Unittest by gtest
