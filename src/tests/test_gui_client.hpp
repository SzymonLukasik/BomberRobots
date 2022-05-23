#include <boost/asio.hpp>
#include <variant>

#include "../messages/gui.hpp"
#include "../parse_args.hpp"
#include "../game.hpp"
#include "../buffers/inbuffer.hpp"

#include "utils.hpp"

using boost::asio::ip::udp;

void test_gui_deserialization(uint16_t &port) {
    try { 
        boost::asio::io_context io_context;
        udp::socket socket(io_context, udp::endpoint{udp::v6(), port});

        InBuffer buffer;
        InputMessage message;
        for (;;) {
            socket >> buffer >> message;
            std::cout << message << '\n';
        }

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}