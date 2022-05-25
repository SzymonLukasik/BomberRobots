#ifndef __VARIANT_UTILS_H__
#define __VARIANT_UTILS_H__

#include <variant>

// helper type for the visitor
template<class... Ts> struct visitors : Ts... { using Ts::operator()...; };

// explicit deduction guide (not needed as of C++20)
template<class... Ts> visitors(Ts...) -> visitors<Ts...>;

#endif // __VARIANT_UTILS_H__