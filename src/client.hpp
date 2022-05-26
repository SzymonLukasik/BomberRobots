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
#include "buffers/buffers.hpp"

using boost::asio::ip::udp;
using boost::asio::ip::tcp;

class Client {
public:
    Client(std::string player_name, EndPoint server_endpoint, EndPoint gui_endpoint,
           uint16_t port)
    : player_name(player_name), io_context(), 
      server_buffer(io_context, server_endpoint), gui_buffer(io_context, port, gui_endpoint) {        
        connect_to_server();
    }

    void run() {
        std::thread t1([this](){ this->listen_to_gui(); });
        std::thread t2([this](){ this->listen_to_server(); });
        t1.join();
        t2.join();
    }

private:
    std::string player_name;
    boost::asio::io_context io_context;
    TCPBuffer server_buffer;
    UDPBuffer gui_buffer;

    mutable std::shared_mutex game_state_mutex;
    DrawMessage game_state;
    bool observer = true;

    void connect_to_server() {
        ServerMessage message;
        server_buffer >> message;
        if (!std::holds_alternative<Hello>(message)) {
            throw std::runtime_error("Wrong hello message.");
        }
        game_state = Lobby(std::move(std::get<Hello>(message)));
        send_state_to_gui();
    }

    bool is_in_lobby() {
        std::shared_lock lock(game_state_mutex);
        return std::holds_alternative<Lobby>(game_state);
    }

    void send_state_to_gui() {
        gui_buffer << game_state;
        gui_buffer.send();
    }

    void respond_to_server_in_lobby(ServerMessage &message) {
        std::visit(visitors {
            [this](AcceptedPlayer &accepted_player) {
                std::unique_lock lock(game_state_mutex);
                if (accepted_player.get_player().get_name() == player_name)
                    observer = false;
                std::get<Lobby>(game_state).add_player(std::move(accepted_player));
                send_state_to_gui();
            },
            [this](GameStarted &game_started) {
                std::unique_lock lock(game_state_mutex);
                game_state = std::get<Lobby>(game_state).start_game(std::move(game_started));
            },
            [](auto){ throw std::runtime_error("Unexpected server message in lobby."); }
        }, message);
    }

    void respond_to_server_during_game(ServerMessage &message) {
        std::visit(visitors {
            [this](Turn &turn) {
                std::unique_lock lock(game_state_mutex);
                Game &game = std::get<Game>(game_state);
                game.process_turn(std::move(turn));
                send_state_to_gui();
                game.next_turn();
            }, 
            [this]([[maybe_unused]] GameEnded &game_ended) {
                std::unique_lock lock(game_state_mutex);
                game_state = std::get<Game>(game_state).end_game();
                send_state_to_gui();
            },
            [](auto){ throw std::runtime_error("Unexpected server message during a game."); }
        }, message);
    }

    void listen_to_server() {
        for (;;) {
            ServerMessage message;
            try { server_buffer >> message; } 
            catch (...) { exit(1); }
            if (is_in_lobby()) {
                respond_to_server_in_lobby(message);
            } else {
                respond_to_server_during_game(message);
            }
        }
    }

    void join_game() {
        server_buffer << ClientMessage(Join(player_name));
        server_buffer.send();
    }

    void send_move_to_server(InputMessage &input_message) {
        ClientMessage client_message;
        std::visit([&client_message](auto move){ client_message = move; }, input_message);
        server_buffer << client_message;
        server_buffer.send();
    }

    void listen_to_gui() {
        InputMessage input_message;
        for (;;) {
            try {
                gui_buffer >> input_message;
                if (is_in_lobby()) {
                    join_game();
                } else if (!observer) {
                    send_move_to_server(input_message);
                }
            } catch (...) {}
        }
    }
};

#endif // __CLIENT_H__