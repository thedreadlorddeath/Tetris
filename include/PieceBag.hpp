#pragma once
#include "Config.hpp"
#include <deque>
#include <random>

class PieceBag {
public:
    PieceBag();

    void reset();
    PieceType getNextPiece();
    PieceType peek(size_t index = 0) const;

private:
    void refillBag();

    std::deque<PieceType> m_queue;
    std::mt19937 m_rng;
};
