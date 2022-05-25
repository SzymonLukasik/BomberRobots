#ifndef __UTILS_H__
#define __UTILS_H__

#include <variant>
#include <type_traits>
#include <iostream>

template <typename U, typename V>
std::ostream &operator<<(std::ostream &stream, const std::pair<U, V> &pair) {
    stream << pair.first << ": " << pair.second;
    return stream;
}

template <typename T>
std::ostream &operator<<(std::ostream &stream, const std::list<T> &list) {
    stream << " [ ";
    for (const T &val : list) {
        stream << val << ", ";
    }
    stream << " ] ";
    return stream;
}

template <typename U, typename V>
std::ostream &operator<<(std::ostream &stream, const std::map<U, V> &map) {
    stream << " { ";
    for (const std::pair<U, V> &key_val : map) {
        stream << key_val << ", ";
    }
    stream << " } ";
    return stream;
}

#endif // __UTILS_H__