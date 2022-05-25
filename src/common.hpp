#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
#include <iostream>

class EndPoint {
public:
    EndPoint() : ip(), port() {}

    EndPoint(std::string str) {
        size_t pos = str.find_last_of(":");
        ip = str.substr(0, pos);
        port = str.substr(pos + 1);
    }

    friend std::ostream &operator<<(std::ostream &stream, const EndPoint &endpoint) {
        stream << "ip: " << endpoint.ip << ", port: " << endpoint.port;
        return stream; 
    }

    std::string get_ip() {
        return ip;
    }

    std::string get_port() {
        return port;
    }

private:
    std::string ip;
    std::string port;
};

#endif // __COMMON_H__