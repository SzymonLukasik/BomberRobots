#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <boost/asio.hpp>

#include "buffer_utils.hpp"
#include "inbuffer.hpp"
#include "outbuffer.hpp"
#include "../parse_args.hpp"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;


class Buffer {
public:
    Buffer(socket_t &&socket) : socket(std::move(socket)), in_buffer(this->socket), out_buffer(this->socket) {}

    void send() {
        out_buffer.send();
    }

    socket_t &get_socket() { return socket; }

    void get_endpoint(udp::endpoint udp_endpoint) { out_buffer.get_endpoint(udp_endpoint);
    }

private:
    socket_t socket;
    InBuffer in_buffer;
    OutBuffer out_buffer;

    template <typename T>
    friend Buffer &operator<<(Buffer &buff, const T &val) {
        buff.out_buffer << val;
        return buff;
    }

    template <typename T>
    friend Buffer &operator>>(Buffer &buff, T &val) {
        buff.in_buffer >> val;
        return buff;
    }
};

#endif // __BUFFER_H__