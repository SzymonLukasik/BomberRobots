#ifndef __OUTBUFFER_H__
#define __OUTBUFFER_H__

#include <boost/asio/detail/socket_ops.hpp>
#include <string>
#include <exception>
#include <variant>
#include <limits>
#include <list>
#include <map>

#include <iostream>

#include "utils.hpp"

class OutBuffer {
public:
    void write_data(const void *src, size_t n) {
        if (size + n > CAPACITY) {
            throw std::runtime_error("Not enough capacity.");
        }

        memcpy(data + size, src, n);
        size += n;
    }

    const char *get_data() {
        return data;
    }

    size_t get_size() {
        return size;
    }
private:
    static const size_t CAPACITY = 65536;
    size_t size = 0;
    char data [CAPACITY];

    friend OutBuffer &operator<<(tcp::socket &socket, OutBuffer &buff) {
        boost::system::error_code error;
        socket.send(boost::asio::buffer(buffer.data, buffer.size), boost::asio::basic_stream_socket::message_flags(0));
        buff.size += n_received;
        std::cout << "RECEIVED: " << n_received << " from " << remote_endpoint << '\n' << "SIZE: " << buff.size << '\n';
        return buff;
    }
};

template <supported_integral T>
T host_to_network(const T &val) {
    if (sizeof(T) == 1) {
        return val;
    } else if (sizeof(T) == 2) {
        return boost::asio::detail::socket_ops::host_to_network_short(val);
    } else if (sizeof(T) == 4) {
        return boost::asio::detail::socket_ops::host_to_network_long(val);
    }
}

template <supported_integral T>
OutBuffer &operator<<(OutBuffer &buff, const T &val) {
    T network_val = host_to_network(val);
    buff.write_data(&network_val, sizeof(network_val));
    return buff;
}

OutBuffer &operator<<(OutBuffer &buff, const std::string &val) {
    if (val.size() > std::numeric_limits<uint8_t>::max()) {
        throw std::runtime_error("String too long.");
    }

    buff << (uint8_t) val.size();
    buff.write_data(val.c_str(), val.size());
    return buff;
}

template <class... Ts>
OutBuffer &operator<<(OutBuffer &buff, const std::variant<Ts...> &variant) {
        uint8_t index = (uint8_t) variant.index();
        std::visit([&buff, index](auto const &value){ buff << index << value; }, variant);
        return buff;
}

template <typename U, typename V>
OutBuffer &operator<<(OutBuffer &buff, const std::pair<U, V> &pair) {
    buff << pair.first << pair.second;
    return buff;
}

template <typename T>
OutBuffer &operator<<(OutBuffer &buff, const std::list<T> &list) {
    if (list.size() > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("List too long.");
    }

    buff << static_cast<uint32_t>(list.size());
    for (const T &val : list) {
        buff << val;
    }

    return buff;
}

template <typename U, typename V>
OutBuffer &operator<<(OutBuffer &buff, const std::map<U, V> &map) {
    if (map.size() > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("Map too long.");
    }

    buff << static_cast<uint32_t>(map.size());
    for (const std::pair<U, V> &key_val : map) {
        buff << key_val;
    }
    return buff;
}

#endif // __OUTBUFFER_H__