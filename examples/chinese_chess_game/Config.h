#ifndef _CONFIG_H_cbda4c090b9d_
#define _CONFIG_H_cbda4c090b9d_

class Config {
public:
    static const int _port;
    static const int _concurrency_num;
    static const char *const _ip;
    static const char *const _init_msg_red;
    static const char *const _init_msg_black;
    static const char *const _end_msg;
    virtual ~Config() = 0;
};

const int Config::_port = 8889;
const int Config::_concurrency_num = 20000;
const char *const Config::_ip = "127.0.0.1";
const char *const Config::_init_msg_red = "0 0 0 0\n";
const char *const Config::_init_msg_black = "1 1 1 1\n";
const char *const Config::_end_msg = "2 2 2 2\n";


#endif