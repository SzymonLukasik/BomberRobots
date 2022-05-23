#include <variant>
#include <type_traits>
#include <iostream>

#include "../messages/gui.hpp"
#include "../messages/server.hpp"

template <class T, class... Ts>
std::ostream &operator<<(std::ostream &stream, const std::variant<T, Ts...> &variant) {
        using variant_t = std::variant<T, Ts...>;

        if (std::is_same_v<variant_t, ClientMessage>) {
            stream << "ClientMessage";
        } else if (std::is_same_v<variant_t, ServerMessage>) {
            stream << "ServerMessage";
        } else if (std::is_same_v<variant_t, DrawMessage>) {
            stream << "DrawMessage";
        } else if (std::is_same_v<variant_t, InputMessage>) {
            stream << "InputMessage";
        } else {
            stream << "UnknownVariant";
        }

        stream << " { ";
        std::visit([&stream](auto const &value){ stream << value; }, variant);
        stream << " } ";
        return stream;
}