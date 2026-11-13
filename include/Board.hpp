#pragma once
#include "Config.hpp"
#include "Tetromino.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <array>

struct Cell {
    bool occupied = false;
    sf::Color color = sf::Color::Transparent;
};

class Board {
public:
    Board();

    void reset();

    // Collision detection
    bool isValidPosition(const Tetromino& piece) const;
    bool isValidPosition(PieceType type, int rotation, const Point& pos) const;
    bool isCellOccupied(int x, int y) const;

    // Ghost piece calculation
    Point calculateGhostPosition(const Tetromino& piece) const;

    // Piece locking
    void lockPiece(const Tetromino& piece);

    // Line clearing
    std::vector<int> findFullRows() const;
    void clearRows(const std::vector<int>& rows);

    // Flash animation state
    void startLineClearAnimation(const std::vector<int>& rows);
    bool isAnimatingLineClear() const { return m_animatingClear; }
    void update(float dt);

    // Rendering
    void draw(sf::RenderTarget& target, float offsetX, float offsetY, float blockSize) const;

private:
    std::array<std::array<Cell, Config::BOARD_WIDTH>, Config::BOARD_HEIGHT> m_grid;

    // Line clear animation state
    bool m_animatingClear;
    float m_clearTimer;
    std::vector<int> m_clearingRows;
};
