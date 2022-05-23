#ifndef __CLIENT_GUI_H__
#define __CLIENT_GUI_H__

#include <string>
#include <map>

#include "action.hpp"
#include "../game.hpp"
#include "../buffers/outbuffer.hpp"

class Lobby {
public:
    Lobby(std::string server_name, players_count_t players_count,
          Position::coord_t size_x, Position::coord_t size_y, game_length_t game_length,
          Bomb::explosion_rad_t explosion_radius, Bomb::timer_t bomb_timer,
          std::map<Player::id_t, Player> players)
    : server_name(server_name), players_count(players_count), size_x(size_x), size_y(size_y),
      game_length(game_length), explosion_radius(explosion_radius), bomb_timer(bomb_timer), players(players) {}
private:
    std::string server_name;
    players_count_t players_count;
    Position::coord_t size_x;
    Position::coord_t size_y;
    game_length_t game_length;
    Bomb::explosion_rad_t explosion_radius;
    Bomb::timer_t bomb_timer;
    std::map<Player::id_t, Player> players;

    friend OutBuffer &operator<<(OutBuffer &buff, const Lobby &lobby) {
        buff << lobby.server_name << lobby.players_count
             << lobby.size_x << lobby.size_y << lobby.game_length
             << lobby.explosion_radius << lobby.bomb_timer << lobby.players;
        return buff;
    }
};

class Game {
public:
    Game(std::string server_name, Position::coord_t size_x, Position::coord_t size_y,
         game_length_t game_length, game_length_t turn, std::map<Player::id_t, Player> players,
         std::map<Player::id_t, Position> player_positions, std::list<Position> blocks, 
         std::list<Bomb> bombs, std::list<Position> explosions, std::map<Player::id_t, score_t> scores)
    : server_name(server_name), size_x(size_x), size_y(size_y), game_length(game_length), turn(turn),
      players(players), player_positions(player_positions), blocks(blocks), bombs(bombs),
      explosions(explosions), scores(scores) {}
private:
    std::string server_name;
    Position::coord_t size_x;
    Position::coord_t size_y;
    game_length_t game_length;
    game_length_t turn;
    std::map<Player::id_t, Player> players;
    std::map<Player::id_t, Position> player_positions;
    std::list<Position> blocks;
    std::list<Bomb> bombs;
    std::list<Position> explosions;
    std::map<Player::id_t, score_t> scores;

    friend OutBuffer &operator<<(OutBuffer &buff, const Game &game) {
        buff << game.server_name << game.size_x << game.size_y << game.game_length
             << game.turn << game.players << game.player_positions << game.blocks 
             << game.bombs << game.explosions << game.scores;
        return buff;
    }
};

using DrawMessage = std::variant<
    Lobby,
    Game
>;

using InputMessage = std::variant<
    PlaceBomb,
    PlaceBlock,
    Move
>;

#endif // __CLIENT_GUI_H__