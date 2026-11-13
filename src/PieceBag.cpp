#include "PieceBag.hpp"
#include <algorithm>
#include <chrono>
#include <vector>

PieceBag::PieceBag()
    : m_rng(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()))
{
    reset();
}

void PieceBag::reset() {
    m_queue.clear();
    refillBag();
    refillBag(); // Keep at least 14 pieces in buffer so peek(0..5) is always valid
}

void PieceBag::refillBag() {
    std::vector<PieceType> bag = {
        PieceType::I,
        PieceType::O,
        PieceType::T,
        PieceType::S,
        PieceType::Z,
        PieceType::J,
        PieceType::L
    };
    std::shuffle(bag.begin(), bag.end(), m_rng);
    for (auto piece : bag) {
        m_queue.push_back(piece);
    }
}

PieceType PieceBag::getNextPiece() {
    if (m_queue.size() <= 7) {
        refillBag();
    }
    PieceType next = m_queue.front();
    m_queue.pop_front();
    return next;
}

PieceType PieceBag::peek(size_t index) const {
    if (index < m_queue.size()) {
        return m_queue[index];
    }
    return PieceType::NONE;
}
