#pragma once
#include "Config.hpp"
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

struct Point {
    int x;
    int y;
};

class Tetromino {
public:
    Tetromino();
    explicit Tetromino(PieceType type);

    void reset(PieceType type);
    
    PieceType getType() const { return m_type; }
    int getRotation() const { return m_rotation; }
    Point getPosition() const { return m_position; }
    void setPosition(const Point& pos) { m_position = pos; }

    sf::Color getColor() const;
    static sf::Color getColorForType(PieceType type);

    // Returns the 4 block coordinates in board space (position + local offset)
    std::array<Point, 4> getBlockPositions() const;
    std::array<Point, 4> getBlockPositions(int rotation, const Point& pos) const;

    // Local block positions for given rotation
    static std::array<Point, 4> getShape(PieceType type, int rotation);

    // Wall kick offsets for SRS
    static const std::vector<Point>& getWallKicks(PieceType type, int fromRot, int toRot);

    void rotateCW();
    void rotateCCW();
    void setRotation(int rot) { m_rotation = (rot % 4 + 4) % 4; }

    void move(int dx, int dy);

    // Rendering helpers
    static void drawBlock(sf::RenderTarget& target, float x, float y, float size, 
                          sf::Color color, float alphaFactor = 1.0f, bool isGhost = false);
    
    void draw(sf::RenderTarget& target, float offsetX, float offsetY, float blockSize, 
              float alpha = 1.0f, bool isGhost = false) const;

    // Draw preview in mini-box (e.g. Hold/Next)
    static void drawPreview(sf::RenderTarget& target, PieceType type, float centerX, float centerY, 
                            float blockSize, float alpha = 1.0f);

private:
    PieceType m_type;
    int m_rotation; // 0, 1, 2, 3
    Point m_position; // Board coordinates of bounding box origin
};
