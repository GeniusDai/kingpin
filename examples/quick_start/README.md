# Quick Start:

**Build a Simple Server**

In this guide, we will build a server that echo "Hello, kingpin!" to the client and then close the connection. Just several codes.

### Step 1. Define your own IOHandler

Implement a IOHandler, this could tell the thread what to do when handling a connection.

```
template<typename _Data>
class SimpleHandler : public IOHandlerForServer<_Data> {
public:
    SimpleHandler(_Data *d) : IOHandlerForServer<_Data>(d) {}
    void onConnect(int conn) {
        const char *str = "Hello, kingpin!";
        ::write(conn, str, strlen(str));
        ::close(conn);
    }
};
```

The template _Data refers to the data shared between threads. This template is essential for the IOHandler.

### Step 2. Define you own TPSharedData

Define the data shared between threads. Since on EXTRA DATA(except the listening socket and mutex) to be shared in this simple demo, we just derive from the base class.

```
class SharedData : public ServerTPSharedData {

};
```

### Step 3. Init the data and run the server

Use 8 threads to handle the connnections, and listen in 8888 port.

```
int main() {
    SharedData data;
    EpollTPServer<SimpleHandler, SharedData> server(8, 8888, &data);
    server.run();
    return 0;
}
```

Since as long as the server alives, the thread pool alives, so the data shared between TP shall have the same life cycle as the server. Usually we place them in the main function's stack.

### Step 4. Connect to server

Use netcat to connect.

```
$ nc localhost 8888
Hello, kingpin!

Ncat: Broken pipe.
```

# Tutorial

**Override 5 functions for IOHandlerForServer:**

* void onEpollLoop() : Epoll loop starts

* void onConnect(int conn) : Socket accepted

* void onMessage(int conn) : Message arrives, has been read automatically to read buffer

* void onWriteComplete(int conn) : Async write finished

* void onPassivelyClosed(int conn) : Oppo closed socket or crash

**Override 6 functions for IOHandlerForClient:**

* void onEpollLoop() : Epoll loop starts

* void onConnect(int conn) : Connection established

* void onConnectFailed(int conn) : Connect to host failed

* void onMessage(int conn) : Message arrives, has been read automatically to read buffer

* void onWriteComplete(int conn) : Async write finished

* void onPassivelyClosed(int conn) : Oppo closed socket or crash
