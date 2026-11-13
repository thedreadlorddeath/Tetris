#include "Tetromino.hpp"
#include <algorithm>
#include <cmath>

Tetromino::Tetromino()
    : m_type(PieceType::NONE), m_rotation(0), m_position{3, 0}
{
}

Tetromino::Tetromino(PieceType type)
    : m_type(type), m_rotation(0), m_position{3, 0}
{
    reset(type);
}

void Tetromino::reset(PieceType type) {
    m_type = type;
    m_rotation = 0;
    if (m_type == PieceType::O) {
        m_position = {3, 0};
    } else if (m_type == PieceType::I) {
        m_position = {3, -1};
    } else {
        m_position = {3, 0};
    }
}

sf::Color Tetromino::getColor() const {
    return getColorForType(m_type);
}

sf::Color Tetromino::getColorForType(PieceType type) {
    switch (type) {
        case PieceType::I: return Config::COLOR_I;
        case PieceType::O: return Config::COLOR_O;
        case PieceType::T: return Config::COLOR_T;
        case PieceType::S: return Config::COLOR_S;
        case PieceType::Z: return Config::COLOR_Z;
        case PieceType::J: return Config::COLOR_J;
        case PieceType::L: return Config::COLOR_L;
        default: return sf::Color(100, 100, 100);
    }
}

std::array<Point, 4> Tetromino::getShape(PieceType type, int rotation) {
    rotation = (rotation % 4 + 4) % 4;
    switch (type) {
        case PieceType::I:
            switch (rotation) {
                case 0: return {Point{0, 1}, Point{1, 1}, Point{2, 1}, Point{3, 1}};
                case 1: return {Point{2, 0}, Point{2, 1}, Point{2, 2}, Point{2, 3}};
                case 2: return {Point{0, 2}, Point{1, 2}, Point{2, 2}, Point{3, 2}};
                case 3: return {Point{1, 0}, Point{1, 1}, Point{1, 2}, Point{1, 3}};
            }
            break;
        case PieceType::O:
            return {Point{1, 0}, Point{2, 0}, Point{1, 1}, Point{2, 1}};
        case PieceType::T:
            switch (rotation) {
                case 0: return {Point{1, 0}, Point{0, 1}, Point{1, 1}, Point{2, 1}};
                case 1: return {Point{1, 0}, Point{1, 1}, Point{2, 1}, Point{1, 2}};
                case 2: return {Point{0, 1}, Point{1, 1}, Point{2, 1}, Point{1, 2}};
                case 3: return {Point{1, 0}, Point{0, 1}, Point{1, 1}, Point{1, 2}};
            }
            break;
        case PieceType::S:
            switch (rotation) {
                case 0: return {Point{1, 0}, Point{2, 0}, Point{0, 1}, Point{1, 1}};
                case 1: return {Point{1, 0}, Point{1, 1}, Point{2, 1}, Point{2, 2}};
                case 2: return {Point{1, 1}, Point{2, 1}, Point{0, 2}, Point{1, 2}};
                case 3: return {Point{0, 0}, Point{0, 1}, Point{1, 1}, Point{1, 2}};
            }
            break;
        case PieceType::Z:
            switch (rotation) {
                case 0: return {Point{0, 0}, Point{1, 0}, Point{1, 1}, Point{2, 1}};
                case 1: return {Point{2, 0}, Point{1, 1}, Point{2, 1}, Point{1, 2}};
                case 2: return {Point{0, 1}, Point{1, 1}, Point{1, 2}, Point{2, 2}};
                case 3: return {Point{1, 0}, Point{0, 1}, Point{1, 1}, Point{0, 2}};
            }
            break;
        case PieceType::J:
            switch (rotation) {
                case 0: return {Point{0, 0}, Point{0, 1}, Point{1, 1}, Point{2, 1}};
                case 1: return {Point{1, 0}, Point{2, 0}, Point{1, 1}, Point{1, 2}};
                case 2: return {Point{0, 1}, Point{1, 1}, Point{2, 1}, Point{2, 2}};
                case 3: return {Point{1, 0}, Point{1, 1}, Point{0, 2}, Point{1, 2}};
            }
            break;
        case PieceType::L:
            switch (rotation) {
                case 0: return {Point{2, 0}, Point{0, 1}, Point{1, 1}, Point{2, 1}};
                case 1: return {Point{1, 0}, Point{1, 1}, Point{1, 2}, Point{2, 2}};
                case 2: return {Point{0, 1}, Point{1, 1}, Point{2, 1}, Point{0, 2}};
                case 3: return {Point{0, 0}, Point{1, 0}, Point{1, 1}, Point{1, 2}};
            }
            break;
        default:
            break;
    }
    return {Point{0, 0}, Point{0, 0}, Point{0, 0}, Point{0, 0}};
}

std::array<Point, 4> Tetromino::getBlockPositions() const {
    return getBlockPositions(m_rotation, m_position);
}

std::array<Point, 4> Tetromino::getBlockPositions(int rotation, const Point& pos) const {
    std::array<Point, 4> shape = getShape(m_type, rotation);
    for (auto& pt : shape) {
        pt.x += pos.x;
        pt.y += pos.y;
    }
    return shape;
}

const std::vector<Point>& Tetromino::getWallKicks(PieceType type, int fromRot, int toRot) {
    fromRot = (fromRot % 4 + 4) % 4;
    toRot = (toRot % 4 + 4) % 4;
    static const std::vector<Point> NO_KICKS = {{0, 0}};

    if (type == PieceType::O) {
        return NO_KICKS;
    }

    // Standard SRS Wall Kicks for J, L, S, T, Z pieces (Y inverted for screen coordinates)
    static const std::vector<Point> JLSTZ_0_1 = {{0, 0}, {-1, 0}, {-1, -1}, {0,  2}, {-1,  2}};
    static const std::vector<Point> JLSTZ_1_0 = {{0, 0}, { 1, 0}, { 1,  1}, {0, -2}, { 1, -2}};
    static const std::vector<Point> JLSTZ_1_2 = {{0, 0}, { 1, 0}, { 1,  1}, {0, -2}, { 1, -2}};
    static const std::vector<Point> JLSTZ_2_1 = {{0, 0}, {-1, 0}, {-1, -1}, {0,  2}, {-1,  2}};
    static const std::vector<Point> JLSTZ_2_3 = {{0, 0}, { 1, 0}, { 1, -1}, {0,  2}, { 1,  2}};
    static const std::vector<Point> JLSTZ_3_2 = {{0, 0}, {-1, 0}, {-1,  1}, {0, -2}, {-1, -2}};
    static const std::vector<Point> JLSTZ_3_0 = {{0, 0}, {-1, 0}, {-1,  1}, {0, -2}, {-1, -2}};
    static const std::vector<Point> JLSTZ_0_3 = {{0, 0}, { 1, 0}, { 1, -1}, {0,  2}, { 1,  2}};

    // Standard SRS Wall Kicks for I piece
    static const std::vector<Point> I_0_1 = {{0, 0}, {-2, 0}, { 1, 0}, {-2,  1}, { 1, -2}};
    static const std::vector<Point> I_1_0 = {{0, 0}, { 2, 0}, {-1, 0}, { 2, -1}, {-1,  2}};
    static const std::vector<Point> I_1_2 = {{0, 0}, {-1, 0}, { 2, 0}, {-1, -2}, { 2,  1}};
    static const std::vector<Point> I_2_1 = {{0, 0}, { 1, 0}, {-2, 0}, { 1,  2}, {-2, -1}};
    static const std::vector<Point> I_2_3 = {{0, 0}, { 2, 0}, {-1, 0}, { 2, -1}, {-1,  2}};
    static const std::vector<Point> I_3_2 = {{0, 0}, {-2, 0}, { 1, 0}, {-2,  1}, { 1, -2}};
    static const std::vector<Point> I_3_0 = {{0, 0}, { 1, 0}, {-2, 0}, { 1,  2}, {-2, -1}};
    static const std::vector<Point> I_0_3 = {{0, 0}, {-1, 0}, { 2, 0}, {-1, -2}, { 2,  1}};

    if (type == PieceType::I) {
        if (fromRot == 0 && toRot == 1) return I_0_1;
        if (fromRot == 1 && toRot == 0) return I_1_0;
        if (fromRot == 1 && toRot == 2) return I_1_2;
        if (fromRot == 2 && toRot == 1) return I_2_1;
        if (fromRot == 2 && toRot == 3) return I_2_3;
        if (fromRot == 3 && toRot == 2) return I_3_2;
        if (fromRot == 3 && toRot == 0) return I_3_0;
        if (fromRot == 0 && toRot == 3) return I_0_3;
    } else {
        if (fromRot == 0 && toRot == 1) return JLSTZ_0_1;
        if (fromRot == 1 && toRot == 0) return JLSTZ_1_0;
        if (fromRot == 1 && toRot == 2) return JLSTZ_1_2;
        if (fromRot == 2 && toRot == 1) return JLSTZ_2_1;
        if (fromRot == 2 && toRot == 3) return JLSTZ_2_3;
        if (fromRot == 3 && toRot == 2) return JLSTZ_3_2;
        if (fromRot == 3 && toRot == 0) return JLSTZ_3_0;
        if (fromRot == 0 && toRot == 3) return JLSTZ_0_3;
    }
    return NO_KICKS;
}

void Tetromino::rotateCW() {
    m_rotation = (m_rotation + 1) % 4;
}

void Tetromino::rotateCCW() {
    m_rotation = (m_rotation + 3) % 4;
}

void Tetromino::move(int dx, int dy) {
    m_position.x += dx;
    m_position.y += dy;
}

void Tetromino::drawBlock(sf::RenderTarget& target, float x, float y, float size, 
                          sf::Color color, float alphaFactor, bool isGhost) 
{
    float pad = 1.0f;
    float blockSize = size - pad * 2.0f;

    if (isGhost) {
        // Translucent neon ghost piece with glowing outline
        sf::RectangleShape ghostBox(sf::Vector2f(blockSize, blockSize));
        ghostBox.setPosition(x + pad, y + pad);
        ghostBox.setFillColor(sf::Color(color.r, color.g, color.b, static_cast<sf::Uint8>(45 * alphaFactor)));
        ghostBox.setOutlineThickness(-1.5f);
        ghostBox.setOutlineColor(sf::Color(color.r, color.g, color.b, static_cast<sf::Uint8>(180 * alphaFactor)));
        target.draw(ghostBox);

        // Subtle inner dot
        sf::RectangleShape innerDot(sf::Vector2f(blockSize * 0.4f, blockSize * 0.4f));
        innerDot.setPosition(x + pad + blockSize * 0.3f, y + pad + blockSize * 0.3f);
        innerDot.setFillColor(sf::Color(color.r, color.g, color.b, static_cast<sf::Uint8>(35 * alphaFactor)));
        target.draw(innerDot);
        return;
    }

    sf::Uint8 alpha = static_cast<sf::Uint8>(255 * std::clamp(alphaFactor, 0.0f, 1.0f));

    // Base block body
    sf::RectangleShape baseBlock(sf::Vector2f(blockSize, blockSize));
    baseBlock.setPosition(x + pad, y + pad);
    baseBlock.setFillColor(sf::Color(color.r, color.g, color.b, alpha));
    target.draw(baseBlock);

    // Bevel effect calculations
    float bevel = std::max(2.0f, std::round(size * 0.11f));

    // Highlight color (Top and Left inner bevel)
    sf::Uint8 hlR = static_cast<sf::Uint8>(std::min(255, static_cast<int>(color.r * 0.65f + 255 * 0.35f)));
    sf::Uint8 hlG = static_cast<sf::Uint8>(std::min(255, static_cast<int>(color.g * 0.65f + 255 * 0.35f)));
    sf::Uint8 hlB = static_cast<sf::Uint8>(std::min(255, static_cast<int>(color.b * 0.65f + 255 * 0.35f)));
    sf::Color highlightColor(hlR, hlG, hlB, alpha);

    // Shadow color (Bottom and Right inner bevel)
    sf::Uint8 shR = static_cast<sf::Uint8>(color.r * 0.45f);
    sf::Uint8 shG = static_cast<sf::Uint8>(color.g * 0.45f);
    sf::Uint8 shB = static_cast<sf::Uint8>(color.b * 0.45f);
    sf::Color shadowColor(shR, shG, shB, alpha);

    // Top bevel
    sf::ConvexShape topBevel;
    topBevel.setPointCount(4);
    topBevel.setPoint(0, sf::Vector2f(x + pad, y + pad));
    topBevel.setPoint(1, sf::Vector2f(x + pad + blockSize, y + pad));
    topBevel.setPoint(2, sf::Vector2f(x + pad + blockSize - bevel, y + pad + bevel));
    topBevel.setPoint(3, sf::Vector2f(x + pad + bevel, y + pad + bevel));
    topBevel.setFillColor(highlightColor);
    target.draw(topBevel);

    // Left bevel
    sf::ConvexShape leftBevel;
    leftBevel.setPointCount(4);
    leftBevel.setPoint(0, sf::Vector2f(x + pad, y + pad));
    leftBevel.setPoint(1, sf::Vector2f(x + pad + bevel, y + pad + bevel));
    leftBevel.setPoint(2, sf::Vector2f(x + pad + bevel, y + pad + blockSize - bevel));
    leftBevel.setPoint(3, sf::Vector2f(x + pad, y + pad + blockSize));
    leftBevel.setFillColor(highlightColor);
    target.draw(leftBevel);

    // Right bevel
    sf::ConvexShape rightBevel;
    rightBevel.setPointCount(4);
    rightBevel.setPoint(0, sf::Vector2f(x + pad + blockSize, y + pad));
    rightBevel.setPoint(1, sf::Vector2f(x + pad + blockSize, y + pad + blockSize));
    rightBevel.setPoint(2, sf::Vector2f(x + pad + blockSize - bevel, y + pad + blockSize - bevel));
    rightBevel.setPoint(3, sf::Vector2f(x + pad + blockSize - bevel, y + pad + bevel));
    rightBevel.setFillColor(shadowColor);
    target.draw(rightBevel);

    // Bottom bevel
    sf::ConvexShape bottomBevel;
    bottomBevel.setPointCount(4);
    bottomBevel.setPoint(0, sf::Vector2f(x + pad, y + pad + blockSize));
    bottomBevel.setPoint(1, sf::Vector2f(x + pad + bevel, y + pad + blockSize - bevel));
    bottomBevel.setPoint(2, sf::Vector2f(x + pad + blockSize - bevel, y + pad + blockSize - bevel));
    bottomBevel.setPoint(3, sf::Vector2f(x + pad + blockSize, y + pad + blockSize));
    bottomBevel.setFillColor(shadowColor);
    target.draw(bottomBevel);

    // Glossy Specular Center Highlight (Arcade reflection)
    float innerSize = blockSize - bevel * 2.0f;
    if (innerSize > 4.0f) {
        sf::RectangleShape gloss(sf::Vector2f(innerSize * 0.45f, innerSize * 0.45f));
        gloss.setPosition(x + pad + bevel + 1.0f, y + pad + bevel + 1.0f);
        gloss.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(100 * alphaFactor)));
        target.draw(gloss);
    }
}

void Tetromino::draw(sf::RenderTarget& target, float offsetX, float offsetY, float blockSize, 
                     float alpha, bool isGhost) const 
{
    if (m_type == PieceType::NONE) return;

    sf::Color color = getColor();
    auto blocks = getBlockPositions();
    for (const auto& pt : blocks) {
        if (pt.y >= 0) { // Don't draw above the ceiling
            drawBlock(target, offsetX + pt.x * blockSize, offsetY + pt.y * blockSize, 
                      blockSize, color, alpha, isGhost);
        }
    }
}

void Tetromino::drawPreview(sf::RenderTarget& target, PieceType type, float centerX, float centerY, 
                            float blockSize, float alpha) 
{
    if (type == PieceType::NONE) return;

    auto shape = getShape(type, 0);
    sf::Color color = getColorForType(type);

    // Compute bounding box
    int minX = 999, maxX = -999, minY = 999, maxY = -999;
    for (const auto& pt : shape) {
        minX = std::min(minX, pt.x);
        maxX = std::max(maxX, pt.x);
        minY = std::min(minY, pt.y);
        maxY = std::max(maxY, pt.y);
    }

    float pieceWidth = (maxX - minX + 1) * blockSize;
    float pieceHeight = (maxY - minY + 1) * blockSize;

    float startX = centerX - pieceWidth / 2.0f - minX * blockSize;
    float startY = centerY - pieceHeight / 2.0f - minY * blockSize;

    for (const auto& pt : shape) {
        drawBlock(target, startX + pt.x * blockSize, startY + pt.y * blockSize, 
                  blockSize, color, alpha, false);
    }
}
