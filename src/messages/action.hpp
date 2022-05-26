#ifndef __ACTION_H__
#define __ACTION_H__


#include "../buffers/outbuffer.hpp"
#include "../buffers/inbuffer.hpp"
#include <iostream>

class PlaceBomb {
public:
    PlaceBomb() = default;

    friend OutBuffer &operator<<(OutBuffer &buff, [[maybe_unused]] const PlaceBomb &place_bomb) {
        return buff;
    }

    friend InBuffer &operator>>(InBuffer &buff, [[maybe_unused]] PlaceBomb &place_bomb) {
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, [[maybe_unused]] const PlaceBomb &place_bomb) {
        stream << "PlaceBomb";
        return stream;
    }
};

class PlaceBlock {
public:
    PlaceBlock() = default;

    friend OutBuffer &operator<<(OutBuffer &buff, [[maybe_unused]] const PlaceBlock &place_block) {
        return buff;
    }

    friend InBuffer &operator>>(InBuffer &buff, [[maybe_unused]] PlaceBlock &place_block) {
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, [[maybe_unused]] const PlaceBlock &place_block) {
        stream << "PlaceBlock";
        return stream;
    }
};

enum class Direction : uint8_t {
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3
};


OutBuffer &operator<<(OutBuffer &buff, const Direction &direction) {
    buff << static_cast<std::underlying_type<Direction>::type>(direction);
    return buff;
}

InBuffer &operator>>(InBuffer &buff, Direction &direction) {
    std::underlying_type<Direction>::type val;
    buff >> val;
    direction = static_cast<Direction>(val);
    return buff;
}

std::ostream &operator<<(std::ostream &stream, const Direction &direction) {
        stream << "Direction { " << (
              direction == Direction::Up ? "Up"
            : direction == Direction::Right ? "Right"
            : direction == Direction::Down ? "Down"
            : "Left"
        ) << " }";
        return stream;
}


class Move {
public:
    Move() = default;

    Move(Direction direction) : direction(direction) {}

private:
    Direction direction;

    friend OutBuffer &operator<<(OutBuffer &buff, const Move &move) {
        buff << move.direction;
        return buff;
    }

    friend InBuffer &operator>>(InBuffer &buff, Move &move) {
        buff >> move.direction;
        return buff;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Move &move) {
        stream << "Move { " << move.direction << " }";
        return stream;
    }
};

#endif // __ACTION_H__