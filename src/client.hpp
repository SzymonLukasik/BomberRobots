#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <string>
#include <iostream>
#include <variant>
#include <atomic>
#include <shared_mutex>

#include "utils.hpp"
#include "messages/gui.hpp"
#include "messages/server.hpp"
#include "buffers/buffer.hpp"
#include "tests/utils.hpp"

using boost::asio::ip::udp;
using boost::asio::ip::tcp;


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
        } else if (std::is_same_v<variant_t, Event>) {
            stream << "Event"; 
        } else {
            stream << "UnknownVariant";
        }

        stream << " { ";
        std::visit([&stream](auto const &value){ stream << value; }, variant);
        stream << " } ";
        return stream;
}

class Client {
public:
    Client(std::string player_name, EndPoint server_endpoint, EndPoint gui_endpoint,
           uint16_t port)
    : player_name(player_name), io_context(), 
      tcp_buffer(tcp::socket(io_context)), 
      udp_buffer(udp::socket(io_context, udp::endpoint(udp::v6(), port))) {        
        tcp::socket &tcp_socket = std::get<tcp::socket>(tcp_buffer.get_socket());
        tcp::resolver tcp_resolver(io_context);
        tcp::resolver::results_type tcp_endpoints = tcp_resolver.resolve(server_endpoint.get_ip(), server_endpoint.get_port());        
        tcp_socket.open(tcp::v6());
        tcp_socket.set_option(tcp::no_delay(true));
        boost::asio::connect(tcp_socket, tcp_endpoints);

        // udp::socket &udp_socket = std::get<udp::socket>(udp_buffer.get_socket());
        udp::resolver udp_resolver(io_context);
        udp::endpoint udp_endpoint = *udp_resolver.resolve(gui_endpoint.get_ip(), gui_endpoint.get_port()).begin();
        // udp_socket.connect(udp_endpoint);
        udp_buffer.get_endpoint(udp_endpoint);

        connect_to_server();
    }

    void connect_to_server() {
        ServerMessage message;
        tcp_buffer >> message;
        if (!std::holds_alternative<Hello>(message)) {
            throw std::runtime_error("Wrong hello message.");
        }
        game_state = Lobby(std::move(std::get<Hello>(message)));
        send_state_to_gui();
    }

    void run() {
        std::thread t1([this](){ this->listen_to_gui(); });
        std::thread t2([this](){ this->listen_to_server(); });
        t1.join();
        t2.join();
    }

private:
    bool is_in_lobby() {
        std::shared_lock lock(game_state_mutex);
        return std::holds_alternative<Lobby>(game_state);
    }

    void send_state_to_gui() {
        udp_buffer << game_state;
        udp_buffer.send();
    }

    void listen_to_server() {
        ServerMessage message;
        for (;;) {
            try {
                tcp_buffer >> message;
            } catch (...) {
                exit(1);
            }
            if (is_in_lobby()) {
                if (std::holds_alternative<AcceptedPlayer>(message)) {
                    std::cout << "ACCEPTED_PLAYER\n";
                    std::unique_lock lock(game_state_mutex);
                    AcceptedPlayer accepted_player = std::get<AcceptedPlayer>(message);
                    std::get<Lobby>(game_state).add_player(accepted_player);
                    send_state_to_gui();
                    if (accepted_player.get_player().get_name() == player_name) {
                        observer = false;
                    }
                } else if (std::holds_alternative<GameStarted>(message)) {
                    std::cout << "GAME_STARTED\n";
                    std::unique_lock lock(game_state_mutex);
                    game_state = std::get<Lobby>(game_state).start_game(
                        std::move(std::get<GameStarted>(message)));
                } else {
                    throw std::runtime_error("Unexpected server message in lobby.");
                }
            } else {
                if (std::holds_alternative<Turn>(message)) {
                    std::cout << "TURN\n";
                    std::unique_lock lock(game_state_mutex);
                    Game &game = std::get<Game>(game_state);
                    game.process_turn(std::get<Turn>(message));
                    std::cout << game << '\n';
                    send_state_to_gui();
                    game.next_turn();
                } else if (std::holds_alternative<GameEnded>(message)) {
                    std::cout << "GAME_ENDED\n";
                    std::unique_lock lock(game_state_mutex);
                    game_state = std::get<Game>(game_state).end_game();
                    send_state_to_gui();
                } else {
                    throw std::runtime_error("Unexpected server message during a game.");
                }
            }
        }
    }

    void listen_to_gui() {
        InputMessage input_message;
        ClientMessage client_message;
        for (;;) {
            try {
                udp_buffer >> input_message;
                std::cout << input_message << '\n';
                if (is_in_lobby()) {
                    tcp_buffer << ClientMessage(Join(player_name));
                    tcp_buffer.send();
                } else if (!observer) {
                    std::visit([&client_message](auto move){ client_message = move; }, input_message);
                    tcp_buffer << client_message;
                    tcp_buffer.send();
                }
            } catch (...) {
                std::cout << "RECEIVED INVALID\n";
            }
        }
    }

    std::string player_name;
    boost::asio::io_context io_context;
    Buffer tcp_buffer;
    Buffer udp_buffer;

    mutable std::shared_mutex game_state_mutex;
    DrawMessage game_state;
    bool observer = true;
};

#endif // __CLIENT_H__