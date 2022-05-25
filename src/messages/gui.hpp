#ifndef __CLIENT_GUI_H__
#define __CLIENT_GUI_H__

#include <string>
#include <map>

#include "action.hpp"
#include "../game.hpp"
#include "../buffers/outbuffer.hpp"
#include "../messages/server.hpp"
#include "../variant_utils.hpp"

class Lobby;

class Game {
public:
    Game(Hello game_settings, game_length_t turn, std::map<Player::id_t, Player> players,
         std::map<Player::id_t, Position> player_positions, std::list<Position> blocks, 
         std::list<Bomb> bombs, std::list<Position> explosions, std::map<Player::id_t, score_t> scores)
    : game_settings(game_settings), turn(turn), players(players), player_positions(player_positions), 
      blocks(blocks), bombs(bombs), explosions(explosions), scores(scores) {}

    Game(Hello &&game_settings, GameStarted &&game_started)
    : game_settings(std::move(game_settings)), turn(0), players(std::move(game_started.players)) {
          for (const std::pair<Player::id_t, Player> &key_val : players) {
              scores[key_val.first] = 0;
          }
      }

    Lobby end_game();

    bool is_ended() {
        return (turn == game_settings.get_game_length());
    }

    void process_turn(Turn &turn) {
        if (turn.get_turn() != this->turn) {
            throw std::runtime_error("Turn out of order.");
        }

        explosions.clear();
        for (const Event &event : turn.get_events()) {
            std::visit(visitors {
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
    Hello game_settings;
    game_length_t turn;
    std::map<Player::id_t, Player> players;
    std::map<Player::id_t, Position> player_positions;
    std::list<Position> blocks;
    std::list<Bomb> bombs;
    std::list<Position> explosions;
    std::map<Player::id_t, score_t> scores;

    std::map<Bomb::id_t, Bomb> bomb_map;
    std::set<Position> block_positions;

    void add_bomb(const BombPlaced &bomb_placed) {
        bomb_map[bomb_placed.get_id()] = Bomb(bomb_placed.get_position(), game_settings.get_bomb_timer());
    }

    bool position_in_map(const Position &position) {
        return ((position.get_x() < game_settings.get_size_x())
            &&  (position.get_y() < game_settings.get_size_y()));
    }

    void mark_explosions_in_direction(Position position, const Direction &direction) {
        position = position.shift(direction);
        if (position_in_map(position)) {
            explosions.push_back(position);
            if (block_positions.find(position) == block_positions.end()) {
                mark_explosions_in_direction(position, direction);
            }
        }
    }

    void process_bomb_explosion(const BombExploded &bomb_exploded) {
        auto bomb_map_it = bomb_map.find(bomb_exploded.get_id());

        Position explosion_position = (bomb_map_it->second).get_position();
        explosions.push_back(explosion_position);
        if (block_positions.find(explosion_position) == block_positions.end()) {
            for (const Direction &direction : {Direction::Up, Direction::Right, Direction::Down, Direction::Left}) {
                mark_explosions_in_direction(explosion_position, direction);
            }
        }
        
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
        blocks.clear();
        for (const Position &pos : block_positions) {
            blocks.push_back(pos);
        }

        bombs.clear();
        for (std::pair<const Bomb::id_t, Bomb> &key_val : bomb_map) {
            key_val.second.decrease_timer();
            bombs.push_back(key_val.second);
        }
    }

    friend OutBuffer &operator<<(OutBuffer &buff, const Game &game) {
        buff << game.game_settings.get_server_name() << game.game_settings.get_size_x() 
             << game.game_settings.get_size_y() << game.game_settings.get_game_length()
             << game.turn << game.players << game.player_positions << game.blocks 
             << game.bombs << game.explosions << game.scores;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Game &game) {
        stream << " Game { game_settings " << game.game_settings << ", turn: " << game.turn
               << ", players: " << game.players << ", player_positions: " << game.player_positions
               << ", blocks: " << game.blocks << ", bombs: " << game.bombs << ", explosions: " << game.explosions
               << ", scores: " << game.scores;
        return stream; 
    }
};

class Lobby {
public:
    Lobby(Hello &game_settings) : game_settings(std::move(game_settings)) {};
    Lobby(Hello &&game_settings) : game_settings(std::move(game_settings)) {};
    Lobby() = default;

    Game start_game(GameStarted &&game_started) {
        return Game(std::move(game_settings), std::move(game_started));
    }

    void add_player(AcceptedPlayer &accepted_player) {
        players[accepted_player.id] = accepted_player.player;
    }

    void add_player(AcceptedPlayer &&accepted_player) {
        players[accepted_player.id] = accepted_player.player;
    }

private:
    Hello game_settings;
    std::map<Player::id_t, Player> players;

    friend OutBuffer &operator<<(OutBuffer &buff, const Lobby &lobby) {
        buff << lobby.game_settings << lobby.players;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Lobby &lobby) {
        stream << " Lobby { game_settings " << lobby.game_settings << ", players: " << lobby.players;
        return stream; 
    }
};

Lobby Game::end_game() {
    return Lobby(std::move(this->game_settings));
}

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