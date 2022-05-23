#ifndef __GAME_H__
#define __GAME_H__

#include <string>
#include <list>
#include <variant>

#include "outbuffer.hpp"

class Player {
public:
    using id_t = uint8_t;
    Player(std::string name, std::string address)
    : name(name), address(address) {}
private:
    std::string name;
    std::string address;

    friend OutBuffer &operator<<(OutBuffer &buff, const Player &player) {
        buff << player.name << player.address;
        return buff;
    }
};

class Position {
public:
    using coord_t = uint16_t;
    Position(coord_t x, coord_t y) : x(x), y(y) {}
private:
    coord_t x;
    coord_t y;

    friend OutBuffer &operator<<(OutBuffer &buff, const Position &position) {
        buff << position.x << position.y;
        return buff;
    }
};

class Bomb {
public:
    using id_t = uint32_t;
    using timer_t = uint16_t;
    using explosion_rad_t = uint16_t;
    Bomb(Position position, timer_t timer) : position(position), timer(timer) {}
private:
    Position position;
    timer_t timer;

    friend OutBuffer &operator<<(OutBuffer &buff, const Bomb &bomb) {
        buff << bomb.position << bomb.timer;
        return buff;
    }
};

using score_t = uint32_t;

using game_length_t = uint16_t;

using players_count_t = uint8_t;

class BombPlaced {
public:
    BombPlaced(Bomb::id_t id, Position position) : id(id), position(position) {}
private:
    Bomb::id_t id;
    Position position;
};

class BombExploded {
public:
    BombExploded(Bomb::id_t id, std::list<Player::id_t> robots_destroyed,
                 std::list<Position> blocks_destroyed)
    : id(id), robots_destroyed(robots_destroyed), blocks_destroyed(blocks_destroyed) {}
private:
    Bomb::id_t id;
    std::list<Player::id_t> robots_destroyed;
    std::list<Position> blocks_destroyed;
};

class PlayerMoved {
public:
    PlayerMoved(Player::id_t id, Position position) : id(id), position(position) {}
private:
    Player::id_t id;
    Position position;
};

class PlayerPlaced {
public:
    PlayerPlaced(Position position) : position(position) {}
private:
    Position position;
};

using Event = std::variant<
    BombPlaced, 
    BombExploded, 
    PlayerMoved, 
    PlayerPlaced
>;

#endif // __GAME_H__