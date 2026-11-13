#include "Board.hpp"
#include <algorithm>
#include <cmath>

Board::Board()
    : m_animatingClear(false), m_clearTimer(0.0f)
{
    reset();
}

void Board::reset() {
    for (int y = 0; y < Config::BOARD_HEIGHT; ++y) {
        for (int x = 0; x < Config::BOARD_WIDTH; ++x) {
            m_grid[y][x].occupied = false;
            m_grid[y][x].color = sf::Color::Transparent;
        }
    }
    m_animatingClear = false;
    m_clearTimer = 0.0f;
    m_clearingRows.clear();
}

bool Board::isCellOccupied(int x, int y) const {
    if (x < 0 || x >= Config::BOARD_WIDTH || y < 0 || y >= Config::BOARD_HEIGHT) {
        return true;
    }
    return m_grid[y][x].occupied;
}

bool Board::isValidPosition(const Tetromino& piece) const {
    return isValidPosition(piece.getType(), piece.getRotation(), piece.getPosition());
}

bool Board::isValidPosition(PieceType type, int rotation, const Point& pos) const {
    if (type == PieceType::NONE) return false;

    auto shape = Tetromino::getShape(type, rotation);
    for (const auto& pt : shape) {
        int gx = pos.x + pt.x;
        int gy = pos.y + pt.y;

        // Check horizontal bounds
        if (gx < 0 || gx >= Config::BOARD_WIDTH) {
            return false;
        }

        // Check bottom floor
        if (gy >= Config::BOARD_HEIGHT) {
            return false;
        }

        // Check placed blocks (ignore if above the ceiling)
        if (gy >= 0 && m_grid[gy][gx].occupied) {
            return false;
        }
    }
    return true;
}

Point Board::calculateGhostPosition(const Tetromino& piece) const {
    Point ghostPos = piece.getPosition();
    while (isValidPosition(piece.getType(), piece.getRotation(), Point{ghostPos.x, ghostPos.y + 1})) {
        ghostPos.y += 1;
    }
    return ghostPos;
}

void Board::lockPiece(const Tetromino& piece) {
    if (piece.getType() == PieceType::NONE) return;

    auto blocks = piece.getBlockPositions();
    sf::Color color = piece.getColor();

    for (const auto& pt : blocks) {
        if (pt.x >= 0 && pt.x < Config::BOARD_WIDTH && pt.y >= 0 && pt.y < Config::BOARD_HEIGHT) {
            m_grid[pt.y][pt.x].occupied = true;
            m_grid[pt.y][pt.x].color = color;
        }
    }
}

std::vector<int> Board::findFullRows() const {
    std::vector<int> fullRows;
    for (int y = 0; y < Config::BOARD_HEIGHT; ++y) {
        bool full = true;
        for (int x = 0; x < Config::BOARD_WIDTH; ++x) {
            if (!m_grid[y][x].occupied) {
                full = false;
                break;
            }
        }
        if (full) {
            fullRows.push_back(y);
        }
    }
    return fullRows;
}

void Board::clearRows(const std::vector<int>& rows) {
    if (rows.empty()) return;

    for (int clearY : rows) {
        // Shift every row above clearY down by 1
        for (int y = clearY; y > 0; --y) {
            for (int x = 0; x < Config::BOARD_WIDTH; ++x) {
                m_grid[y][x] = m_grid[y - 1][x];
            }
        }
        // Top row becomes empty
        for (int x = 0; x < Config::BOARD_WIDTH; ++x) {
            m_grid[0][x].occupied = false;
            m_grid[0][x].color = sf::Color::Transparent;
        }
    }
}

void Board::startLineClearAnimation(const std::vector<int>& rows) {
    m_clearingRows = rows;
    m_animatingClear = true;
    m_clearTimer = Config::LINE_CLEAR_DURATION;
}

void Board::update(float dt) {
    if (m_animatingClear) {
        m_clearTimer -= dt;
        if (m_clearTimer <= 0.0f) {
            clearRows(m_clearingRows);
            m_animatingClear = false;
            m_clearingRows.clear();
        }
    }
}

void Board::draw(sf::RenderTarget& target, float offsetX, float offsetY, float blockSize) const {
    float playfieldWidth = Config::BOARD_WIDTH * blockSize;
    float playfieldHeight = Config::BOARD_HEIGHT * blockSize;

    // Outer glow / border backdrop
    sf::RectangleShape outerFrame(sf::Vector2f(playfieldWidth + 8.0f, playfieldHeight + 8.0f));
    outerFrame.setPosition(offsetX - 4.0f, offsetY - 4.0f);
    outerFrame.setFillColor(sf::Color(10, 14, 25));
    outerFrame.setOutlineThickness(2.0f);
    outerFrame.setOutlineColor(Config::PANEL_BORDER);
    target.draw(outerFrame);

    // Playfield background
    sf::RectangleShape background(sf::Vector2f(playfieldWidth, playfieldHeight));
    background.setPosition(offsetX, offsetY);
    background.setFillColor(Config::PLAYFIELD_BG);
    target.draw(background);

    // Subtle grid lines
    sf::VertexArray gridLines(sf::Lines);

    // Vertical grid lines
    for (int x = 1; x < Config::BOARD_WIDTH; ++x) {
        float px = offsetX + x * blockSize;
        gridLines.append(sf::Vertex(sf::Vector2f(px, offsetY), Config::GRID_LINE_COLOR));
        gridLines.append(sf::Vertex(sf::Vector2f(px, offsetY + playfieldHeight), Config::GRID_LINE_COLOR));
    }

    // Horizontal grid lines
    for (int y = 1; y < Config::BOARD_HEIGHT; ++y) {
        float py = offsetY + y * blockSize;
        gridLines.append(sf::Vertex(sf::Vector2f(offsetX, py), Config::GRID_LINE_COLOR));
        gridLines.append(sf::Vertex(sf::Vector2f(offsetX + playfieldWidth, py), Config::GRID_LINE_COLOR));
    }
    target.draw(gridLines);

    // Draw placed blocks
    for (int y = 0; y < Config::BOARD_HEIGHT; ++y) {
        // If row is currently flashing in line clear animation, handle specially
        bool isClearing = m_animatingClear && 
            (std::find(m_clearingRows.begin(), m_clearingRows.end(), y) != m_clearingRows.end());

        for (int x = 0; x < Config::BOARD_WIDTH; ++x) {
            if (m_grid[y][x].occupied) {
                float bx = offsetX + x * blockSize;
                float by = offsetY + y * blockSize;

                if (isClearing) {
                    // Flash with electric white/cyan and shrinking/fading
                    float progress = m_clearTimer / Config::LINE_CLEAR_DURATION;
                    sf::Color flashColor = (static_cast<int>(progress * 24.0f) % 2 == 0) ? 
                        sf::Color(255, 255, 255) : Config::ACCENT_CYAN;
                    Tetromino::drawBlock(target, bx, by, blockSize, flashColor, progress, false);
                } else {
                    Tetromino::drawBlock(target, bx, by, blockSize, m_grid[y][x].color, 1.0f, false);
                }
            }
        }
    }

    // Glowing corner brackets / accents on playfield frame
    float accentLen = 14.0f;
    float accentThick = 2.5f;
    sf::Color neonCyan = Config::ACCENT_CYAN;

    auto drawCorner = [&](float cx, float cy, int dirX, int dirY) {
        sf::RectangleShape hBar(sf::Vector2f(dirX * accentLen, accentThick));
        hBar.setPosition(cx, cy);
        hBar.setFillColor(neonCyan);
        target.draw(hBar);

        sf::RectangleShape vBar(sf::Vector2f(accentThick, dirY * accentLen));
        vBar.setPosition(cx, cy);
        vBar.setFillColor(neonCyan);
        target.draw(vBar);
    };

    drawCorner(offsetX - 4.0f, offsetY - 4.0f, 1, 1);
    drawCorner(offsetX + playfieldWidth + 4.0f, offsetY - 4.0f, -1, 1);
    drawCorner(offsetX - 4.0f, offsetY + playfieldHeight + 4.0f, 1, -1);
    drawCorner(offsetX + playfieldWidth + 4.0f, offsetY + playfieldHeight + 4.0f, -1, -1);
}
