#ifndef __BUFFER_UTILS_H__
#define __BUFFER_UTILS_H__

#include <boost/asio.hpp>
#include <concepts>

using boost::asio::ip::udp;
using boost::asio::ip::tcp;

using socket_t = std::variant<
    udp::socket,
    tcp::socket
>;

template <typename T>
concept supported_integral = std::integral<T> && sizeof(T) <= 4;

// class SerializationError : public std::runtime_error {
// public:
//     explicit SerializationError(const std::string &what_arg);
//     explicit SerializationError(const char* what_arg);
// };

#endif // __BUFFER_UTILS_H__