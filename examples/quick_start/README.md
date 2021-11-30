# Quick Start:

**Build a Simple Server**

In this guide, we will build a server that echo "Hello, kingpin!" to the client and then close the connection. Just several codes.

### Step 1. Define your own IOHandler

First we will implement a IOHandler, this could tell the thread what to do when handling a connection.

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

Here the template _Data refers to the data shared between threads. This template is essential for the IOHandler.

### Step 2. Define you own TPSharedData

Next we define the data shared between threads. Since we didn't got any EXTRA DATA(except the listening socket and mutex) to shared in this simple demo, we just derive from the base class.

```
class SharedData : public ServerTPSharedData {

};
```

### Step 3. Init the data and run the server

Here we use 8 threads to handle the connnections, and listen in 8888 port.

```
int main() {
    SharedData data;
    EpollTPServer<SimpleHandler, SharedData> server(8, 8888, &data);
    server.run();
    return 0;
}
```

Since as long as the server alives, the thread pool alives, so the data shared between TP shall have the same life cycle as the server. Usually we place both of them in the main's stack.

### Step 4. Test by client

Use netcat to connect.

```
$ nc localhost 8888
Hello, kingpin!

Ncat: Broken pipe.
```
