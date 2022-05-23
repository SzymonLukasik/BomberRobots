#include <boost/asio.hpp>
#include "../messages/gui.hpp"
#include "../parse_args.hpp"
#include "../game.hpp"
#include "../buffers/outbuffer.hpp"

using boost::asio::ip::udp;

template <typename T>
void write_to_socket(udp::socket &socket, T &val) {
    // boost::system::error_code error;
    OutBuffer buffer;
    buffer << val; 
    socket.send(boost::asio::buffer(buffer.get_data(), buffer.get_size()));
}

void write_draw_message_to_socket(udp::socket &socket, DrawMessage variant) {
    write_to_socket(socket, variant);
}

void test_gui_serialization(EndPoint &gui_endpoint) {
    try { 
        boost::asio::io_context io_context;
        udp::resolver resolver(io_context);
        udp::endpoint endpoint = *resolver.resolve(gui_endpoint.get_ip(), gui_endpoint.get_port()).begin();
        udp::socket socket(io_context);
        socket.open(endpoint.protocol());
        socket.connect(endpoint);

        write_draw_message_to_socket(socket, Lobby("server_name", 13, 5, 10, 100, 3, 40, {
            {0, Player("Szymon", "[::1]:33840")},
            {1, Player("Kuba", "[::1]:33840")},
            {2, Player("Marysia", "[::1]:33840")}
        }));

        write_draw_message_to_socket(socket, Game("server_name", 3, 5, 200, 40, {
            {0, Player("Szymon", "[::1]:33840")},
            {1, Player("Kuba", "[::1]:33840")},
            {2, Player("Marysia", "[::1]:33840")}
        }, {
            {0, Position(0, 0)},
            {1, Position(1, 1)},
            {2, Position(2, 2)}
        }, {
           Position(0, 0),
           Position(0, 1),
           Position(1, 0) 
        }, {
            Bomb(Position(0, 0), 30),
            Bomb(Position(1, 1), 20),
            Bomb(Position(2, 2), 123)
        }, {
            Position(0, 0),
            Position(0, 1),
            Position(0, 2)
        }, {
            {0, 10},
            {1, 20},
            {2, 30}
        }));

    } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    }
}