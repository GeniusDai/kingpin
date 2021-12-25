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

int main() {
    TPSharedDataForClient data;
    EpollTPClient<CrawlerHandler, TPSharedDataForClient> crawler(2, &data);
    vector<string> hosts = { "www.taobao.com", "www.bytedance.com", "www.baidu.com" };
    for (auto &h : hosts) {
        string ip = getHostIp(h.c_str());
        const char *request = "GET /TEST HTTP/1.1\r\n\r\n";
        data.raw_add(ip, 80, request);
    }
    crawler.run();
    return 0;
}