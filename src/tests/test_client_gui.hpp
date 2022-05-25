#include <boost/asio.hpp>
#include "../messages/gui.hpp"
#include "../parse_args.hpp"
#include "../game.hpp"
#include "../buffers/buffer.hpp"

using boost::asio::ip::udp;


void test_gui_serialization(EndPoint &gui_endpoint) {
    try { 
        boost::asio::io_context io_context;
        udp::resolver resolver(io_context);
        udp::endpoint endpoint = *resolver.resolve(gui_endpoint.get_ip(), gui_endpoint.get_port()).begin();
        udp::socket socket(io_context);
        socket.open(endpoint.protocol());
        socket.connect(endpoint);


        Buffer buffer(std::move(socket));
        Hello game_settings = Hello("server_name", 13, 5, 10, 100, 3, 40);
        Lobby lobby = Lobby(game_settings);
        lobby.add_player(AcceptedPlayer(0, Player("Szymon", "[::1]:33840")));
        lobby.add_player(AcceptedPlayer(1, Player("Kuba", "[::1]:33840")));
        lobby.add_player(AcceptedPlayer(2, Player("Marysia", "[::1]:33840")));

        DrawMessage message = lobby;
        buffer << message;
        buffer.send();

        message = Game(game_settings, 0, {
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
            });
        buffer << message;
        buffer.send();
    } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    }
}