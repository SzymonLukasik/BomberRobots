#ifndef __INBUFFER_H__
#define __INBUFFER_H__

#include <boost/asio.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <string>
#include <exception>
#include <variant>
#include <limits>
#include <list>
#include <map>

#include <exception>
#include <iostream>

#include "utils.hpp"

using boost::asio::ip::udp;


class InBuffer {
public:
    void read_data(void *dest, size_t n) {
        if (size - read < n) {
            std::cerr << size << ' ' << read << ' ' << n << '\n';
            throw std::runtime_error("Not enough data.");
        }

        memcpy(dest, data + read, n);
        read += n;
        if (read == size) {
            read = 0;
            size = 0;
        }
    }

    char *get_data() {
        return data;
    }

    size_t get_size() {
        return size;
    }

private:
    static const size_t CAPACITY = 65536;
    size_t size = 0;
    size_t read = 0;
    char data [CAPACITY];

    friend InBuffer &operator>>(udp::socket &socket, InBuffer &buff) {
        udp::endpoint remote_endpoint;
        size_t n_received = socket.receive_from(boost::asio::buffer(buff.data + buff.size, InBuffer::CAPACITY - buff.size), remote_endpoint);
        buff.size += n_received;
        std::cout << "RECEIVED: " << n_received << " from " << remote_endpoint << '\n' << "SIZE: " << buff.size << '\n';
        return buff;
    } 
};

template <supported_integral T>
T network_to_host(const T &val) {
    if (sizeof(T) == 1) {
        return val;
    } else if (sizeof(T) == 2) {
        return boost::asio::detail::socket_ops::network_to_host_short(val);
    } else if (sizeof(T) == 4) {
        return boost::asio::detail::socket_ops::network_to_host_long(val);
    }
}

template <supported_integral T>
InBuffer &operator>>(InBuffer &buff, T &val) {
    buff.read_data(&val, sizeof(val));
    val = host_to_network(val);
    return buff;
}

InBuffer &operator>>(InBuffer &buff, std::string &str) {
    uint8_t str_size;
    buff >> str_size;

    str.resize((size_t) str_size);
    buff.read_data(str.data(), str.size());
    return buff;
}

template <class V, std::size_t I = 0>
V variant_from_index(uint8_t index) {
    if constexpr(I >= std::variant_size_v<V>)
        throw std::runtime_error("Structure type index " + std::to_string(I + index) + " out of bounds");
    else
        return index == 0
            ? V{std::in_place_index<I>}
            : variant_from_index<V, I + 1>(index - 1);
}

template <class... Ts>
InBuffer &operator>>(InBuffer &buff, std::variant<Ts...> &variant) {
        using variant_t = std::variant<Ts...>;
        uint8_t index;
        buff >> index;
        variant = variant_from_index<variant_t>(index);
        std::visit([&buff](auto &val){ buff >> val; }, variant);
        
        return buff;
}

template <typename U, typename V>
InBuffer &operator>>(InBuffer &buff, std::pair<U, V> &pair) {
    buff >> pair.first >> pair.second;
    return buff;
}

template <typename T>
InBuffer &operator>>(InBuffer &buff, std::list<T> &list) {
    uint32_t list_size;
    buff >> list_size;
    list = std::list<T>(list_size);
    for (const T &val : list) {
        buff >> val;
    }

    return buff;
}

template <typename U, typename V>
InBuffer &operator>>(InBuffer &buff, std::map<U, V> &map) {
    uint32_t map_size;
    buff >> map_size;
    map.clear();
    while (map_size--) {
        U key;
        V value;
        buff >> key >> value;
        map[key] = value;
    }

    return buff;
}

#endif // __InBuffer_H__