#ifndef __UTILS_H__
#define __UTILS_H__

#include <concepts>

template <typename T>
concept supported_integral = std::integral<T> && sizeof(T) <= 4;

// class SerializationError : public std::runtime_error {
// public:
//     explicit SerializationError(const std::string &what_arg);
//     explicit SerializationError(const char* what_arg);
// };

#endif // __UTILS_H__