#include "kingpin/EpollTP.h"
#include "kingpin/TPSharedData.h"
#include "kingpin/IOHandler.h"
#include <netinet/in.h>
#include <unistd.h>

using namespace std;
using namespace kingpin;

template<typename _Data>
class CrawlerHandler : public IOHandlerForClient<_Data> {
public:
    CrawlerHandler(_Data *d) : IOHandlerForClient<_Data>(d) {}
    void onMessage(int conn) {
        INFO << "Message of socket " << conn << ":\n"
            << this->_rbh[conn]->_buffer << END;
    }
};

int main() {
    ClientTPSharedData data;
    EpollTPClient<CrawlerHandler, ClientTPSharedData> crawler(2, &data);
    vector<string> hosts = { "fanyi.baidu.com", "xueshu.baidu.com", "www.baidu.com" };
    for (auto &h : hosts) {
        string ip = getHostIp(h.c_str());
        const char *request = "GET /TEST HTTP/1.1\r\n\r\n";
        data.raw_add(ip, 80, request);
    }
    crawler.run();
    return 0;
}