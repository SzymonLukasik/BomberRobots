#ifndef __CLIENT_GUI_H__
#define __CLIENT_GUI_H__

#include <string>
#include <map>

#include "action.hpp"
#include "../game.hpp"
#include "../buffers/outbuffer.hpp"
#include "../messages/server.hpp"

class Lobby {
public:
    Lobby(std::string server_name, players_count_t players_count,
          Position::coord_t size_x, Position::coord_t size_y, game_length_t game_length,
          Bomb::explosion_rad_t explosion_radius, Bomb::timer_t bomb_timer,
          std::map<Player::id_t, Player> players)
    : server_name(server_name), players_count(players_count), size_x(size_x), size_y(size_y),
      game_length(game_length), explosion_radius(explosion_radius), bomb_timer(bomb_timer), players(players) {};

    Lobby(Hello &hello)
    : server_name(hello.server_name), players_count(hello.players_count), size_x(hello.size_x), size_y(hello.size_y),
      game_length(hello.game_length), explosion_radius(hello.explosion_radius), bomb_timer(hello.bomb_timer) {};

    Lobby() = default;

    void add_player(AcceptedPlayer &accepted_player) {
        players[accepted_player.id] = accepted_player.player;
    }

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

    friend class Game;
};



// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class Game {
public:
    Game(std::string server_name, Position::coord_t size_x, Position::coord_t size_y,
         game_length_t game_length, game_length_t turn, std::map<Player::id_t, Player> players,
         std::map<Player::id_t, Position> player_positions, std::list<Position> blocks, 
         std::list<Bomb> bombs, std::list<Position> explosions, std::map<Player::id_t, score_t> scores)
    : server_name(server_name), size_x(size_x), size_y(size_y), game_length(game_length), turn(turn),
      players(players), player_positions(player_positions), blocks(blocks), bombs(bombs),
      explosions(explosions), scores(scores) {}

    Game(Lobby lobby, GameStarted game_started)
    : server_name(lobby.server_name), size_x(lobby.size_x), size_y(lobby.size_y), game_length(lobby.game_length),
      turn(0), players(game_started.players), bomb_timer(lobby.bomb_timer) {
          for (const std::pair<Player::id_t, Player> &key_val : game_started.players) {
              scores[key_val.first] = 0;
          }
      }

    bool ended() {
        return (turn == game_length);
    }

    void process_turn(Turn &turn) {
        if (turn.get_turn() != this->turn) {
            throw std::runtime_error("Turn out of order.");
        }

        for (const Event &event : turn.get_events()) {
            std::visit(overloaded {
                [this](const BombPlaced &bomb_placed) { this->add_bomb(bomb_placed); },
                [this](const BombExploded &bomb_exploded) { this->process_bomb_explosion(bomb_exploded);  },
                [this](const PlayerMoved &player_moved) { this->move_player(player_moved); },
                [this](const BlockPlaced &block_placed) { this->place_block(block_placed); },
            }, event);
        }

        build_state();
    }

    void next_turn() {
        turn++;
    }

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

    
    Bomb::timer_t bomb_timer;
    std::map<Bomb::id_t, Bomb> bomb_map;
    std::set<Position> block_positions;

    void add_bomb(const BombPlaced &bomb_placed) {
        bomb_map[bomb_placed.get_id()] = Bomb(bomb_placed.get_position(), bomb_timer);
    }

    void process_bomb_explosion(const BombExploded &bomb_exploded) {
        auto bomb_map_it = bomb_map.find(bomb_exploded.get_id());
        explosions.push_back((bomb_map_it->second).get_position());
        bomb_map.erase(bomb_map_it);
        for (const Player::id_t &player_id : bomb_exploded.get_robots_destroyed()) {
            player_positions.erase(player_positions.find(player_id));
            scores[player_id] += 1;
        }

        for (const Position &position : bomb_exploded.get_blocks_destroyed()) {
            block_positions.erase(block_positions.find(position));
        }
    }

    void move_player(const PlayerMoved &player_moved) {
        player_positions[player_moved.get_id()] = player_moved.get_position();
    }

    void place_block(const BlockPlaced &block_placed) {
        block_positions.insert(block_placed.get_position());
    }

    void build_state() {
        blocks = std::list<Position>();
        for (const Position &pos : block_positions) {
            blocks.push_back(pos);
        }

        bombs = std::list<Bomb>();
        for (const std::pair<Bomb::id_t, Bomb> &key_val : bomb_map) {
            bombs.push_back(key_val.second);
        }
    }


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