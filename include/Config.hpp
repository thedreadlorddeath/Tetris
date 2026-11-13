#pragma once
#include <SFML/Graphics/Color.hpp>

namespace Config {
    // Board Dimensions
    constexpr int BOARD_WIDTH = 10;
    constexpr int BOARD_HEIGHT = 20;
    constexpr int BLOCK_SIZE = 32;

    // Window Dimensions
    constexpr int WINDOW_WIDTH = 960;
    constexpr int WINDOW_HEIGHT = 760;

    // Board Rendering Position (Centered on screen)
    constexpr int BOARD_OFFSET_X = 320;
    constexpr int BOARD_OFFSET_Y = 60;

    // Theme Colors (Cyberpunk / Modern Neon Arcade)
    inline const sf::Color BG_COLOR(11, 14, 23);           // #0B0E17 Deep midnight navy
    inline const sf::Color PLAYFIELD_BG(16, 21, 35);       // Darker playfield
    inline const sf::Color GRID_LINE_COLOR(32, 44, 70, 90);// Subtle grid
    inline const sf::Color PANEL_BG(18, 24, 40, 230);      // HUD Card Background
    inline const sf::Color PANEL_BORDER(45, 62, 95);       // HUD Card Border
    inline const sf::Color TEXT_PRIMARY(235, 245, 255);    // Bright white
    inline const sf::Color TEXT_MUTED(120, 145, 180);      // Dim label text
    inline const sf::Color ACCENT_CYAN(0, 240, 255);       // Neon cyan
    inline const sf::Color ACCENT_VIOLET(175, 60, 255);    // Neon violet

    // Tetromino Palette
    inline const sf::Color COLOR_I(0, 240, 255);    // Electric Cyan
    inline const sf::Color COLOR_O(255, 215, 0);    // Bright Gold/Yellow
    inline const sf::Color COLOR_T(175, 60, 255);   // Neon Violet
    inline const sf::Color COLOR_S(50, 240, 80);    // Acid Green
    inline const sf::Color COLOR_Z(255, 45, 75);    // Crimson Red
    inline const sf::Color COLOR_J(30, 100, 255);   // Cobalt Blue
    inline const sf::Color COLOR_L(255, 130, 20);   // Vibrant Orange

    // Gameplay Timing Constants
    constexpr float LOCK_DELAY = 0.5f;          // 0.5s standard lock delay
    constexpr int MAX_LOCK_RESETS = 15;         // Max resets before forced lock
    constexpr float DAS_DELAY = 0.160f;         // Delayed Auto Shift (160ms)
    constexpr float ARR_INTERVAL = 0.033f;      // Auto Repeat Rate (~30 Hz)
    constexpr float SOFT_DROP_INTERVAL = 0.035f;// Fast soft drop
    constexpr float LINE_CLEAR_DURATION = 0.28f;// Flash and dissolve animation duration
}

enum class PieceType {
    NONE = -1,
    I = 0,
    O,
    T,
    S,
    Z,
    J,
    L,
    COUNT
};

enum class GameState {
    START_SCREEN,
    PLAYING,
    PAUSED,
    GAME_OVER
};
