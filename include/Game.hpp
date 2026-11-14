#pragma once
#include "Config.hpp"
#include "Board.hpp"
#include "Tetromino.hpp"
#include "PieceBag.hpp"
#include "Particles.hpp"
#include "SoundManager.hpp"
#include <SFML/Graphics.hpp>
#include <string>

class Game {
public:
    Game();

    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

    // Game lifecycle
    void startNewGame();
    void spawnNextPiece();
    bool tryMove(int dx, int dy);
    bool tryRotate(bool clockwise);
    void hardDrop();
    void holdCurrentPiece();
    void onPieceLocked();
    void checkLineClears();
    void gameOver();

    // High Score Persistence
    void loadHighScore();
    void saveHighScore();

    // Fall timing
    float getFallInterval() const;

    // View & Layout Helpers
    void updateView();
    float getBoardOffsetX() const;
    float getBoardOffsetY() const;
    void handleTouch(int pixelX, int pixelY, bool isDown, unsigned int finger = 0);
    void drawMobileControls();

    // Drawing Helpers
    void drawHUD();
    void drawStartScreen();
    void drawPauseScreen();
    void drawGameOverScreen();
    void drawCard(float x, float y, float width, float height, const std::string& title = "");
    void renderText(const std::string& str, float x, float y, unsigned int size, 
                    sf::Color color = Config::TEXT_PRIMARY, bool center = false, bool bold = false);

    sf::RenderWindow m_window;
    Board m_board;
    PieceBag m_bag;
    Tetromino m_currentPiece;
    PieceType m_holdPiece;
    bool m_canHold;

    ParticleSystem m_particles;
    SoundManager m_sounds;
    GameState m_state;

    // Gameplay Statistics
    int m_score;
    int m_displayScore; // Animated roll-up score
    int m_highScore;
    int m_level;
    int m_linesCleared;

    // Movement & Handling (DAS / ARR)
    int m_moveDir; // -1 = Left, +1 = Right, 0 = None
    float m_dasTimer;
    float m_arrTimer;
    bool m_softDropActive;
    float m_softDropTimer;

    // Gravity & Lock Delay
    float m_fallTimer;
    float m_lockTimer;
    int m_lockResets;
    bool m_isLocking;

    // Tetris banner popup
    float m_tetrisBannerTimer;

    // Fonts
    sf::Font m_font;
    bool m_fontLoaded;

    // Touch button geometries (for mobile layout)
    sf::FloatRect m_btnHold;
    sf::FloatRect m_btnPause;
    sf::FloatRect m_btnRotCCW;
    sf::FloatRect m_btnRotCW;
    sf::FloatRect m_btnLeft;
    sf::FloatRect m_btnRight;
    sf::FloatRect m_btnHardDrop;
    sf::FloatRect m_btnSoftDrop;
    sf::FloatRect m_btnMute;
    sf::FloatRect m_btnRestart;
};
