#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#include <boost/asio.hpp>

#include "buffers_utils.hpp"
#include "inbuffers.hpp"
#include "outbuffers.hpp"
#include "../parse_args.hpp"
#include "../utils.hpp"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;


class UDPBuffer {
public:
    UDPBuffer(boost::asio::io_context &io_context, uint16_t port, EndPoint _endpoint)
    : socket(io_context, udp::endpoint(udp::v6(), port)),
      endpoint(*udp::resolver(io_context).resolve(_endpoint.get_ip(), _endpoint.get_port()).begin()),
      in_buffer(socket, endpoint), out_buffer(socket, endpoint) {}

    void send() {
        out_buffer.send();
    }

protected:
    udp::socket socket;
    udp::endpoint endpoint;
    UDPInBuffer in_buffer;
    UDPOutBuffer out_buffer;

    template <typename T>
    friend UDPBuffer &operator<<(UDPBuffer &buff, const T &val) {
        buff.out_buffer << val;
        return buff;
    }

    template <typename T>
    friend UDPBuffer &operator>>(UDPBuffer &buff, T &val) {
        buff.in_buffer >> val;
        return buff;
    }
};


class TCPBuffer {
public:
    TCPBuffer(boost::asio::io_context &io_context, EndPoint endpoint)
    : socket(io_context), in_buffer(socket), out_buffer(socket) {
        tcp::resolver tcp_resolver(io_context);
        tcp::resolver::results_type tcp_endpoints = tcp_resolver.resolve(endpoint.get_ip(), endpoint.get_port());        
        socket.open(tcp::v6());
        socket.set_option(tcp::no_delay(true));
        boost::asio::connect(socket, tcp_endpoints);
    }

    void send() {
        out_buffer.send();
    }

protected:
    tcp::socket socket;
    tcp::endpoint endpoint;
    TCPInBuffer in_buffer;
    TCPOutBuffer out_buffer;

    template <typename T>
    friend TCPBuffer &operator<<(TCPBuffer &buff, const T &val) {
        buff.out_buffer << val;
        return buff;
    }

    template <typename T>
    friend TCPBuffer &operator>>(TCPBuffer &buff, T &val) {
        buff.in_buffer >> val;
        return buff;
    }
};

#endif // __BUFFERS_H__