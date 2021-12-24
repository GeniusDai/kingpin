# Quick Start:

## --> Build a Simple Server

In this guide, we will build a server that echo "Hello, kingpin!" to the client and then close the connection. Just several codes.

### Step 1. Define your own IOHandler

Implement a IOHandler, this could tell the thread what to do when handling a connection. We just echo the string and close the connection when write completes.

```
template<typename _Data>
class SimpleHandler : public IOHandlerForServer<_Data> {
public:
    SimpleHandler(_Data *d) : IOHandlerForServer<_Data>(d) {}

    void onConnect(int conn) { this->writeToBuffer(conn, "Hello, kingpin!"); }

    void onWriteComplete(int conn) { ::close(conn); }
};
```

The template _Data refers to the data shared between threads. This template is essential for the IOHandler.

### Step 2. Define you own TPSharedData

Define the data shared between threads. Since no EXTRA DATA to be shared in the simple demo, just derive from the base class (or use the base class).

```
class SharedData : public ServerTPSharedData {};
```

### Step 3. Init the data and run the server

Use 8 threads to handle the connnections, and listen in 8888 port.

```
int main() {
    SharedData data;
    data._port = 8888;
    EpollTPServer<SimpleHandler, SharedData> server(8, &data);
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

## --> Build a Simple Crawler

Now let's use the client to build a crawler just send a simple HTTP request.

### Step 1. Define your own IOHandler

This step is always essential for you thread pool. We just print the page we crawled.

```
template<typename _Data>
class CrawlerHandler : public IOHandlerForClient<_Data> {
public:
    CrawlerHandler(_Data *d) : IOHandlerForClient<_Data>(d) {}
    void _print(int conn) {
        if (0 == this->_rbh[conn]->_offset) return;
        INFO << "Host of socket " << conn << ": " << this->_conn_info[conn].first
                << "\nMessage of socket " << conn << ":\n"
                << this->_rbh[conn]->_buffer << END;
        this->_rbh[conn]->clear();
    }

    void onMessage(int conn) { _print(conn); }

    void onPassivelyClosed(int conn) { _print(conn); }
};
```

### Step 2. Define you own TPSharedData

In this simple crawler, we just use the default structure for client.

### Step 3. Init the data and run the server

Since our client will not just support crawler but also concurrency test. Our connection pool is identified by a tuple (ip, port, init_message). We could use the raw_add to add our data to the pool shared by crawler.Then Use 2 threads to handle the connnections.

```
int main() {
    ClientTPSharedData data;
    EpollTPClient<CrawlerHandler, ClientTPSharedData> crawler(2, &data);
    vector<string> hosts = { "www.taobao.com", "www.bytedance.com", "www.baidu.com" };
    for (auto &h : hosts) {
        string ip = getHostIp(h.c_str());
        const char *request = "GET /TEST HTTP/1.1\r\n\r\n";
        data.raw_add(ip, 80, request);
    }
    crawler.run();
    return 0;
}
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
