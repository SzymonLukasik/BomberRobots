#ifndef __GAME_H__
#define __GAME_H__

#include <string>
#include <list>
#include <variant>


#include "buffers/outbuffer.hpp"
#include "buffers/inbuffer.hpp"
#include "../tests/utils.hpp"

class Player {
public:
    using id_t = uint8_t;
    Player() = default;
    Player(std::string name, std::string address)
    : name(name), address(address) {}

    std::string get_name() {
        return name;
    }

private:
    std::string name;
    std::string address;

    friend OutBuffer &operator<<(OutBuffer &buff, const Player &player) {
        buff << player.name << player.address;
        return buff;
    }
    
    friend InBuffer &operator>>(InBuffer &buff, Player &player) {
        buff >> player.name >> player.address;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Player &player) {
        stream << " Player { name: " << player.name << ", address: " << player.address << " } ";
        return stream;
    }
    
};

class Position {
public:
    Position() = default;
    using coord_t = uint16_t;
    Position(coord_t x, coord_t y) : x(x), y(y) {}

    coord_t get_x() const { return x; }
    coord_t get_y() const { return y; }

    bool operator<(const Position &rhs) const {
        if (x < rhs.x || (x == rhs.x && y < rhs.y)) {
            return true;
        }
        return false;
    }

    Position shift(const Direction &direction) {
        switch (direction) {
            case Direction::Up:
                return Position(x, y + 1);
            case Direction::Right:
                return Position(x + 1, y);
            case Direction::Down:
                return Position(x, y - 1);
            default: // Direction::Left
                return Position(x - 1, y);
        }
    }

private:
    coord_t x;
    coord_t y;

    friend OutBuffer &operator<<(OutBuffer &buff, const Position &position) {
        buff << position.x << position.y;
        return buff;
    }

    friend InBuffer &operator>>(InBuffer &buff, Position &position) {
        buff >> position.x >> position.y;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Position &position) {
        stream << " Position { x: " << position.x << ", y: " << position.y << " } ";
        return stream;
    }
};

class Bomb {
public:
    using id_t = uint32_t;
    using timer_t = uint16_t;
    using explosion_rad_t = uint16_t;
    Bomb() = default;
    Bomb(Position position, timer_t timer) : position(position), timer(timer) {}

    void decrease_timer() {
        if (timer == 0) {
            throw std::runtime_error("Decreasing zero timer.");
        }
        timer--;
    }

    Position get_position() const {
        return position;
    }

private:
    Position position;
    timer_t timer;

    friend OutBuffer &operator<<(OutBuffer &buff, const Bomb &bomb) {
        buff << bomb.position << bomb.timer;
        return buff;
    }

    friend InBuffer &operator>>(InBuffer &buff, Bomb &bomb) {
        buff >> bomb.position >> bomb.timer;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Bomb &bomb) {
        stream << " Bomb { position: " << bomb.position << ", timer: " << bomb.timer << " } "; 
        return stream;
    }
};

using score_t = uint32_t;

using game_length_t = uint16_t;

using players_count_t = uint8_t;

class BombPlaced {
public:
    BombPlaced() = default;
    BombPlaced(Bomb::id_t id, Position position) : id(id), position(position) {}

    Bomb::id_t get_id() const {
        return id;
    }

    Position get_position() const {
        return position;
    }

private:
    Bomb::id_t id;
    Position position;

    friend OutBuffer &operator<<(OutBuffer &buff, const BombPlaced &bomb_placed) {
        buff << bomb_placed.id << bomb_placed.position;
        return buff;
    }

    friend InBuffer &operator>>(InBuffer &buff, BombPlaced &bomb_placed) {
        buff >> bomb_placed.id >> bomb_placed.position;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const BombPlaced &bomb_placed) {
        stream << " BombPlaced { id: " << bomb_placed.id << ", position: " << bomb_placed.position << " } ";
        return stream;
    }
};

class BombExploded {
public:
    BombExploded() = default;
    BombExploded(Bomb::id_t id, std::list<Player::id_t> robots_destroyed,
                 std::list<Position> blocks_destroyed)
    : id(id), robots_destroyed(robots_destroyed), blocks_destroyed(blocks_destroyed) {}

    Bomb::id_t get_id() const {
        return id;
    }

    const std::list<Player::id_t> &get_robots_destroyed() const {
        return robots_destroyed;
    }

    const std::list<Position> &get_blocks_destroyed() const {
        return blocks_destroyed;
    }

private:
    Bomb::id_t id;
    std::list<Player::id_t> robots_destroyed;
    std::list<Position> blocks_destroyed;

    friend OutBuffer &operator<<(OutBuffer &buff, const BombExploded &bomb_exploded) {
        buff << bomb_exploded.id << bomb_exploded.robots_destroyed << bomb_exploded.blocks_destroyed;
        return buff;
    }

    friend InBuffer &operator>>(InBuffer &buff, BombExploded &bomb_exploded) {
        buff >> bomb_exploded.id >> bomb_exploded.robots_destroyed >> bomb_exploded.blocks_destroyed;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const BombExploded &bomb_exploded) {
        stream << " BombExploded { id: " << bomb_exploded.id << ", robots_destroyed: " << bomb_exploded.robots_destroyed << ", blocks_destroyed: " << bomb_exploded.blocks_destroyed << " } ";
        return stream;
    }
};

class PlayerMoved {
public:
    PlayerMoved() = default;
    PlayerMoved(Player::id_t id, Position position) : id(id), position(position) {}

    Player::id_t get_id() const {
        return id;
    }

    Position get_position() const {
        return position;
    }

private:
    Player::id_t id;
    Position position;

    friend OutBuffer &operator<<(OutBuffer &buff, const PlayerMoved &player_moved) {
        buff << player_moved.id << player_moved.position;
        return buff;
    }

    friend InBuffer &operator>>(InBuffer &buff, PlayerMoved &player_moved) {
        buff >> player_moved.id >> player_moved.position;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const PlayerMoved &player_moved) {
        stream << " PlayerMoved { id: " << player_moved.id << ", position: " << player_moved.position << " } "; 
        return stream;
    }
};

class BlockPlaced {
public:
    BlockPlaced() = default;
    BlockPlaced(Position position) : position(position) {}

    Position get_position() const {
        return position;
    }

private:
    Position position;

    friend OutBuffer &operator<<(OutBuffer &buff, const BlockPlaced &player_placed) {
        buff << player_placed.position;
        return buff;
    }

    friend InBuffer &operator>>(InBuffer &buff, BlockPlaced &player_placed) {
        buff >> player_placed.position;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const BlockPlaced &player_placed) {
        stream << " BlockPlaced { position: " << player_placed.position << " } ";
        return stream;
    }
};

using Event = std::variant<
    BombPlaced, 
    BombExploded, 
    PlayerMoved, 
    BlockPlaced
>;

#endif // __GAME_H__