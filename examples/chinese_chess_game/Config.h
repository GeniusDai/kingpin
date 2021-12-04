#ifndef _CONFIG_H_cbda4c090b9d_
#define _CONFIG_H_cbda4c090b9d_

class Config {
public:
    static const int _port;
    static const char *const _ip;
    static const char *const _init_msg;
    virtual ~Config() {}
};

const int Config::_port = 8889;
const char *const Config::_ip = "127.0.0.1";
const char *const Config::_init_msg = "0 0 0 0\n";

#endif