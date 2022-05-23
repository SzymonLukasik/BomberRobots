#ifndef __SERVER_CLIENT_H__
#define __SERVER_CLIENT_H__

#include <string>
#include <map>
#include <list>
#include <variant>

#include "../buffers/outbuffer.hpp"
#include "../game.hpp"
#include "action.hpp"

class Join {
public:
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


class Hello {
public:
    Hello(std::string server_name, players_count_t players_count, 
          Position::coord_t size_x, Position::coord_t size_y, game_length_t game_length,
          Bomb::explosion_rad_t explosion_radius, Bomb::timer_t bomb_timer)
    : server_name(server_name), players_count(players_count), size_x(size_x),
      size_y(size_y),game_length(game_length), explosion_radius(explosion_radius),
      bomb_timer(bomb_timer) {}
private:
    std::string server_name;
    players_count_t players_count;
    Position::coord_t size_x;
    Position::coord_t size_y;
    game_length_t game_length;
    Bomb::explosion_rad_t explosion_radius;
    Bomb::timer_t bomb_timer;
};

class AcceptedPlayer {
public:
    AcceptedPlayer(Player::id_t id, Player player)
    : id(id), player(player) {}
private:
    Player::id_t id;
    Player player;
};

class GameStarted {
public:
    GameStarted(std::map<Player::id_t, Player> players) : players(players) {}
private:
    std::map<Player::id_t, Player> players;
};

class Turn {
public:
    Turn(game_length_t turn, std::list<Event> events) : turn(turn), events(events) {}
private:
    game_length_t turn;
    std::list<Event> events;
};

class GameEnded {
public:
    GameEnded(std::map<Player::id_t, score_t> scores) : scores(scores) {}
private:
    std::map<Player::id_t, score_t> scores;
};

using ServerMessage = std::variant<
    Hello,
    AcceptedPlayer,
    GameStarted,
    Turn,
    GameEnded
>;

#endif // __SERVER_CLIENT_H__