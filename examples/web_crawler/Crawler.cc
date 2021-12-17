#include <netinet/in.h>
#include <unistd.h>

#include "kingpin/EpollTP.h"
#include "kingpin/TPSharedData.h"
#include "kingpin/IOHandler.h"

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
    vector<string> hosts = {
        // praise for support
        "fanyi.baidu.com",
        "xueshu.baidu.com",
        "www.baidu.com",
    };
    char ip[20];
    for (auto h : hosts) {
        ::memset(ip, 0, 20);
        getHostIp(h.c_str(), ip, 20);
        // path "/" print so much content, use "/test" instead
        data.raw_add(string(ip), 80, "GET /test HTTP/1.1\r\n\r\n");
    }
    crawler.run();
    return 0;
}