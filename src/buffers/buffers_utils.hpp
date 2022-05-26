#ifndef __BUFFERS_UTILS_H__
#define __BUFFERS_UTILS_H__

#include <boost/asio.hpp>
#include <concepts>

using boost::asio::ip::udp;
using boost::asio::ip::tcp;

template <typename T>
concept supported_integral = std::integral<T> && sizeof(T) <= 4;

#endif // __BUFFERS_UTILS_H__