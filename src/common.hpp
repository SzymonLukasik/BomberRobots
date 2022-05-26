#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
#include <iostream>
#include <variant>
#include <type_traits>
#include <iostream>

// helper type for the visitor
template<class... Ts> struct visitors : Ts... { using Ts::operator()...; };

// explicit deduction guide (not needed as of C++20)
template<class... Ts> visitors(Ts...) -> visitors<Ts...>;


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


template <typename T>
std::ostream &operator<<(std::ostream &stream, const std::set<T> &set) {
    stream << " [ ";
    for (const T &val : set) {
        stream << val << ", ";
    }
    stream << " ] ";
    return stream;
}


template <typename U, typename V>
std::ostream &operator<<(std::ostream &stream, const std::map<U, V> &map) {
    stream << " { (map_size = " << map.size() << ") ";
    for (const std::pair<U, V> &key_val : map) {
        stream << key_val << ", ";
    }
    stream << " } ";
    return stream;
}

#endif // __COMMON_H__