#ifndef __SERVER_CLIENT_H__
#define __SERVER_CLIENT_H__

#include <string>
#include <map>
#include <list>
#include <variant>

#include "gui.hpp"
#include "event.hpp"
#include "action.hpp"
#include "../buffers/outbuffers.hpp"
#include "../utils.hpp"

class Join {
public:
    Join() = default;

    Join(std::string name) : name(name) {}

private:
    std::string name;

    friend OutBuffer &operator<<(OutBuffer &buff, const Join &join) {
        buff << join.name;
        return buff;
    }
};

using ClientMessage = std::variant<
    Join,
    PlaceBomb,
    PlaceBlock,
    Move
>; 

std::ostream &operator<<(std::ostream &stream, const ClientMessage &client_message) {
    stream << "ClientMessage { ";
    std::visit([&stream](auto const &value){ stream << value; }, client_message);
    stream << " }";
    return stream;
}

class Hello {
public:
    Hello() = default;

    Hello(std::string server_name, players_count_t players_count, 
          Position::coord_t size_x, Position::coord_t size_y, game_length_t game_length,
          Bomb::explosion_rad_t explosion_radius, Bomb::timer_t bomb_timer)
    : server_name(server_name), players_count(players_count), size_x(size_x),
      size_y(size_y),game_length(game_length), explosion_radius(explosion_radius),
      bomb_timer(bomb_timer) {}

    const std::string &get_server_name() const { return server_name; }

    players_count_t get_players_count() const { return players_count; }

    Position::coord_t get_size_x() const { return size_x; }

    Position::coord_t get_size_y() const { return size_y; }

    game_length_t get_game_length() const { return game_length; }

    Bomb::explosion_rad_t get_explosion_radius() const { return explosion_radius; }

    Bomb::timer_t get_bomb_timer() const { return bomb_timer; }

private:
    std::string server_name;
    players_count_t players_count;
    Position::coord_t size_x;
    Position::coord_t size_y;
    game_length_t game_length;
    Bomb::explosion_rad_t explosion_radius;
    Bomb::timer_t bomb_timer;

    friend InBuffer &operator>>(InBuffer &buff, Hello &hello) {
        buff >> hello.server_name >> hello.players_count >> hello.size_x >> hello.size_y
             >> hello.game_length >> hello.explosion_radius >> hello.bomb_timer;
        return buff;
    }

    friend OutBuffer &operator<<(OutBuffer &buff, const Hello &hello) {
        buff << hello.server_name << hello.players_count
             << hello.size_x << hello.size_y << hello.game_length
             << hello.explosion_radius << hello.bomb_timer;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, [[maybe_unused]] const Hello &hello) {
        stream << "Hello { server_name: " << hello.server_name 
               << ", players_count: " << hello.players_count
               << ", size_x: " << hello.size_x 
               << ", size_y: " << hello.size_y 
               << ", game_length: " << hello.game_length
               << ", explosion_radius: " << hello.explosion_radius 
               << ", bomb_timer: " << hello.bomb_timer << " }";
        return stream;
    }
};

class AcceptedPlayer {
public:
    AcceptedPlayer() = default;

    AcceptedPlayer(Player::id_t id, Player player) : id(id), player(player) {}

    Player::id_t get_id() const { return id; }

    const Player &get_player() const { return player; }

private:
    Player::id_t id;
    Player player;

    friend InBuffer &operator>>(InBuffer &buff, AcceptedPlayer &accepted_player) {
        buff >> accepted_player.id >> accepted_player.player;
        return buff;
    }

    friend OutBuffer &operator<<(OutBuffer &buff, const AcceptedPlayer &accepted_player) {
        buff << accepted_player.id << accepted_player.player;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const AcceptedPlayer &accepted_player) {
        stream << "AcceptedPlayer { id: " << accepted_player.id 
               << ", player: " << accepted_player.player << " }";
        return stream;
    }
};

class GameStarted {
public:
    GameStarted() = default;

    GameStarted(std::map<Player::id_t, Player> players) : players(players) {}

    std::map<Player::id_t, Player> &get_players() { return players; }

private:
    std::map<Player::id_t, Player> players;

    friend InBuffer &operator>>(InBuffer &buff, GameStarted &game_started) {
        buff >> game_started.players;
        return buff;
    }

    friend OutBuffer &operator>>(OutBuffer &buff, const GameStarted &game_started) {
        buff << game_started.players;
        return buff;
    }
    
    friend std::ostream &operator<<(std::ostream &stream, const GameStarted &game_started) {
        stream << "GameStarted { players: " << game_started.players << " }";
        return stream;
    }
};

class Turn {
public:
    Turn() = default;

    Turn(game_length_t turn, std::list<Event> events) : turn(turn), events(events) {}

    game_length_t get_turn() { return turn; }

    const std::list<Event> &get_events() { return events; }

private:
    game_length_t turn;
    std::list<Event> events;

    friend InBuffer &operator>>(InBuffer &buff, Turn &turn) {
        buff >> turn.turn >> turn.events;
        return buff;
    }

    friend OutBuffer &operator>>(OutBuffer &buff, const Turn &turn) {
        buff << turn.turn << turn.events;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Turn &turn) {
        stream << "Turn { turn: " << turn.turn << ", events: " << turn.events << " }";
        return stream;
    }
};

class GameEnded {
public:
    GameEnded() = default;

    GameEnded(std::map<Player::id_t, score_t> scores) : scores(scores) {}

private:
    std::map<Player::id_t, score_t> scores;

    friend InBuffer &operator>>(InBuffer &buff, GameEnded &game_ended) {
        buff >> game_ended.scores;
        return buff;
    }

    friend OutBuffer &operator>>(OutBuffer &buff, const GameEnded &game_ended) {
        buff << game_ended.scores;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const GameEnded &game_ended) {
        stream << "GameEnded { scores: " << game_ended.scores << " }";
        return stream;
    }
};

using ServerMessage = std::variant<
    Hello,
    AcceptedPlayer,
    GameStarted,
    Turn,
    GameEnded
>;

std::ostream &operator<<(std::ostream &stream, const ServerMessage &server_message) {
    stream << "ServerMessage { ";
    std::visit([&stream](auto const &value){ stream << value; }, server_message);
    stream << " }";
    return stream;
}

#endif // __SERVER_CLIENT_H__