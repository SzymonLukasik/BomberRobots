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
#include "buffers/inbuffer.hpp"
#include "buffers/outbuffer.hpp"
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
    : player_name(player_name), gui_endpoint(gui_endpoint), server_endpoint(server_endpoint),
      port(port), io_context(), udp_socket(io_context, udp::endpoint{udp::v6(), port}),
      tcp_socket(io_context) {
        tcp::resolver tcp_resolver(io_context);
        tcp::resolver::results_type tcp_endpoints = tcp_resolver.resolve(server_endpoint.get_ip(), server_endpoint.get_port());
        boost::asio::connect(tcp_socket, tcp_endpoints);


        boost::asio::io_context io_context;
        udp::resolver udp_resolver(io_context);
        udp::endpoint udp_endpoint = *udp_resolver.resolve(gui_endpoint.get_ip(), gui_endpoint.get_port()).begin();
        udp_socket.connect(udp_endpoint);
    }

    void run() {
        join_server();
        std::cerr << "JOINED SERVER\n";
        if (observer) {
            std::cerr << "OBSERVING\n";
            observe();
        } else {
            std::cerr << "PLAYING\n";
            play();
        }
    }

private:

    void observe() {
        for (;;) {
            ServerMessage message;
            tcp_socket >> tcp_in_buffer >> message;
            if (std::holds_alternative<Turn>(message)) {
                std::unique_lock lock(game_state_mutex);
                Game &game = std::get<Game>(game_state);
                game.process_turn(std::get<Turn>(message));
                send_state_to_gui();
                game.next_turn();
                if (game.ended()) {
                    std::cout << "GAME ENDED1\n";
                    game_state = Lobby();
                    break;
                }
            } else if (std::holds_alternative<GameEnded>(message)) {
                std::cout << "GAME ENDED2\n";
                std::unique_lock lock(game_state_mutex);
                game_state = Lobby();
                break;
            } else {
                throw std::runtime_error("Unexpected server message sent to an observer.");
            }
        }
    }

    void make_move() {
        for (;;) {
            if (is_in_lobby()) {
                std::cout << "ENDED HERE\n";
                break;
            }
            InputMessage input_message;
            udp_socket >> udp_in_buffer;
            udp_in_buffer >> input_message;

            ClientMessage client_message;
            std::visit([&client_message](auto move){ client_message = move; }, input_message);
            tcp_out_buffer << client_message;
            tcp_socket << tcp_out_buffer;
        }
    }

    void play() {
        std::thread t1([this](){ this->observe(); });
        std::thread t2([this](){ this->make_move(); });
        t1.join();
        t2.join();
    }

    bool is_in_lobby() {
        std::shared_lock lock(game_state_mutex);
        return std::holds_alternative<Lobby>(game_state);
    }

    bool read_correct_input() {
        InputMessage input_message;
        try {
            udp_socket >> udp_in_buffer;
            udp_in_buffer >> input_message;
        } catch (std::exception &exception) {
            return false;
        } 
        return true;
    }

    void wait_for_correct_input() {
        while (is_in_lobby() && !read_correct_input()) {
        }
    }

    void send_join() {
        wait_for_correct_input();
        if (is_in_lobby()) {
            tcp_out_buffer << ClientMessage(Join(player_name));
            tcp_socket << tcp_out_buffer;
        }
    };

    void listen_to_server_in_lobby() {
        for (;;) {
            ServerMessage message;
            tcp_socket >> tcp_in_buffer >> message;

            if (std::holds_alternative<AcceptedPlayer>(message)) {
                std::unique_lock lock(game_state_mutex);
                AcceptedPlayer accepted_player = std::get<AcceptedPlayer>(message);
                std::get<Lobby>(game_state).add_player(accepted_player);
                send_state_to_gui();
                if (accepted_player.get_player().get_name() == player_name) {
                    observer = false;
                }
            } else if (std::holds_alternative<GameStarted>(message)) {
                std::unique_lock lock(game_state_mutex);
                game_state = Game(std::get<Lobby>(game_state), std::get<GameStarted>(message));
                break;
            } else {
                throw std::runtime_error("Unexpected server message in lobby.");
            }
        }
    }

    void join_server() {
        ServerMessage message;
        tcp_socket >> tcp_in_buffer >> message;
        if (!std::holds_alternative<Hello>(message)) {
            throw std::runtime_error("Wrong hello message.");
        }
        game_state = Lobby(std::get<Hello>(message));
        send_state_to_gui();
        std::thread t1([this](){ this->send_join(); });
        std::thread t2([this](){ this->listen_to_server_in_lobby(); });
        t1.join();
        t2.join();
    }

    void send_state_to_gui() {
        udp_out_buffer << game_state;
        udp_socket << udp_out_buffer;
    }

    std::string player_name;
    EndPoint gui_endpoint, server_endpoint;
    uint16_t port;

    boost::asio::io_context io_context;
    udp::socket udp_socket;
    tcp::socket tcp_socket;

    InBuffer tcp_in_buffer, udp_in_buffer;
    OutBuffer tcp_out_buffer, udp_out_buffer;

    mutable std::shared_mutex game_state_mutex;
    DrawMessage game_state;
    bool observer = true;
};

#endif // __CLIENT_H__