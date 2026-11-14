#include "Game.hpp"
#include <fstream>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <sstream>
#ifdef __ANDROID__
#include <sys/stat.h>
#endif

Game::Game()
    : m_holdPiece(PieceType::NONE),
      m_canHold(true),
      m_state(GameState::START_SCREEN),
      m_score(0),
      m_displayScore(0),
      m_highScore(0),
      m_level(1),
      m_linesCleared(0),
      m_moveDir(0),
      m_dasTimer(0.0f),
      m_arrTimer(0.0f),
      m_softDropActive(false),
      m_softDropTimer(0.0f),
      m_fallTimer(0.0f),
      m_lockTimer(0.0f),
      m_lockResets(0),
      m_isLocking(false),
      m_tetrisBannerTimer(0.0f),
      m_fontLoaded(false)
{
    // Setup touch button geometry
    m_btnHold      = sf::FloatRect(60.0f,  930.0f, 180.0f, 80.0f);
    m_btnRotCCW    = sf::FloatRect(270.0f, 930.0f, 180.0f, 80.0f);
    m_btnRotCW     = sf::FloatRect(510.0f, 930.0f, 180.0f, 80.0f);
    m_btnPause     = sf::FloatRect(720.0f, 930.0f, 180.0f, 80.0f);

    m_btnLeft      = sf::FloatRect(60.0f,  1040.0f, 220.0f, 140.0f);
    m_btnHardDrop  = sf::FloatRect(310.0f, 1040.0f, 340.0f, 140.0f);
    m_btnRight     = sf::FloatRect(680.0f, 1040.0f, 220.0f, 140.0f);

    m_btnMute      = sf::FloatRect(60.0f,  1210.0f, 220.0f, 130.0f);
    m_btnSoftDrop  = sf::FloatRect(310.0f, 1210.0f, 340.0f, 130.0f);
    m_btnRestart   = sf::FloatRect(680.0f, 1210.0f, 220.0f, 130.0f);

#ifdef __ANDROID__
    m_window.create(sf::VideoMode::getDesktopMode(), "TETRIS // NEON ARCADE");
    m_window.setFramerateLimit(60);
    updateView();
#else
    m_window.create(sf::VideoMode(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT), "TETRIS // NEON ARCADE", sf::Style::Titlebar | sf::Style::Close);
    m_window.setFramerateLimit(60);
#endif

    // Font Candidate loading (Android uses AAssetManager "font.ttf" inside APK)
#ifdef __ANDROID__
    const std::vector<std::string> fontCandidates = {
        "font.ttf",
        "assets/font.ttf",
        "/system/fonts/Roboto-Regular.ttf"
    };
#else
    const std::vector<std::string> fontCandidates = {
        "assets/font.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/tahoma.ttf"
    };
#endif

    for (const auto& path : fontCandidates) {
        if (m_font.loadFromFile(path)) {
            m_fontLoaded = true;
            break;
        }
    }

    loadHighScore();
}

float Game::getBoardOffsetX() const {
#ifdef __ANDROID__
    return 320.0f;
#else
    return Config::BOARD_OFFSET_X;
#endif
}

float Game::getBoardOffsetY() const {
#ifdef __ANDROID__
    return 240.0f;
#else
    return Config::BOARD_OFFSET_Y;
#endif
}

void Game::updateView() {
    float windowWidth = static_cast<float>(m_window.getSize().x);
    float windowHeight = static_cast<float>(m_window.getSize().y);
    if (windowWidth <= 0.0f || windowHeight <= 0.0f) return;

#ifdef __ANDROID__
    float targetWidth = 960.0f;
    float targetHeight = 1600.0f;
#else
    float targetWidth = Config::WINDOW_WIDTH;
    float targetHeight = Config::WINDOW_HEIGHT;
#endif

    float windowRatio = windowWidth / windowHeight;
    float targetRatio = targetWidth / targetHeight;
    float sizeX = 1.0f;
    float sizeY = 1.0f;
    float posX = 0.0f;
    float posY = 0.0f;

    if (windowRatio >= targetRatio) {
        sizeX = targetRatio / windowRatio;
        posX = (1.0f - sizeX) / 2.0f;
    } else {
        sizeY = windowRatio / targetRatio;
        posY = (1.0f - sizeY) / 2.0f;
    }

    sf::View view(sf::FloatRect(0.0f, 0.0f, targetWidth, targetHeight));
    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
    m_window.setView(view);
}

void Game::loadHighScore() {
    try {
#ifdef __ANDROID__
        mkdir("/data/data/com.thedreadlorddeath.tetris/files", 0755);
        const char* path = "/data/data/com.thedreadlorddeath.tetris/files/highscore.txt";
#else
        const char* path = "highscore.txt";
#endif
        std::ifstream file(path);
        if (file.is_open()) {
            file >> m_highScore;
            file.close();
        } else {
            m_highScore = 0;
        }
    } catch (...) {
        m_highScore = 0;
    }
}

void Game::saveHighScore() {
    try {
#ifdef __ANDROID__
        mkdir("/data/data/com.thedreadlorddeath.tetris/files", 0755);
        const char* path = "/data/data/com.thedreadlorddeath.tetris/files/highscore.txt";
#else
        const char* path = "highscore.txt";
#endif
        std::ofstream file(path);
        if (file.is_open()) {
            file << m_highScore;
            file.close();
        }
    } catch (...) {
    }
}

void Game::startNewGame() {
    m_board.reset();
    m_bag.reset();
    m_particles.clear();
    m_score = 0;
    m_displayScore = 0;
    m_level = 1;
    m_linesCleared = 0;
    m_holdPiece = PieceType::NONE;
    m_canHold = true;
    m_moveDir = 0;
    m_dasTimer = 0.0f;
    m_arrTimer = 0.0f;
    m_softDropActive = false;
    m_softDropTimer = 0.0f;
    m_fallTimer = 0.0f;
    m_lockTimer = 0.0f;
    m_lockResets = 0;
    m_isLocking = false;
    m_tetrisBannerTimer = 0.0f;

    spawnNextPiece();
    m_state = GameState::PLAYING;
}

void Game::spawnNextPiece() {
    PieceType nextType = m_bag.getNextPiece();
    m_currentPiece.reset(nextType);
    m_canHold = true;
    m_lockTimer = 0.0f;
    m_lockResets = 0;
    m_isLocking = false;
    m_fallTimer = 0.0f;

    // Check if new piece immediately collides (Game Over condition)
    if (!m_board.isValidPosition(m_currentPiece)) {
        gameOver();
    }
}

void Game::gameOver() {
    m_state = GameState::GAME_OVER;
    if (m_score > m_highScore) {
        m_highScore = m_score;
        saveHighScore();
    }
    m_sounds.playGameOver();
    m_particles.triggerShake(9.0f, 0.45f);
}

float Game::getFallInterval() const {
    // Standard guideline gravity progression (faster with each level)
    float baseInterval = std::max(0.06f, static_cast<float>(std::pow(0.85f - (m_level - 1) * 0.005f, m_level - 1)) * 0.85f);
    return baseInterval;
}

bool Game::tryMove(int dx, int dy) {
    Point currentPos = m_currentPiece.getPosition();
    Point targetPos = {currentPos.x + dx, currentPos.y + dy};

    if (m_board.isValidPosition(m_currentPiece.getType(), m_currentPiece.getRotation(), targetPos)) {
        m_currentPiece.setPosition(targetPos);

        // Moving horizontally while resting on stack resets lock delay
        if (dx != 0 && m_isLocking) {
            if (m_lockResets < Config::MAX_LOCK_RESETS) {
                m_lockTimer = 0.0f;
                m_lockResets++;
            }
        }
        if (dx != 0) {
            m_sounds.playMove();
        }
        return true;
    }
    return false;
}

bool Game::tryRotate(bool clockwise) {
    int fromRot = m_currentPiece.getRotation();
    int toRot = clockwise ? (fromRot + 1) % 4 : (fromRot + 3) % 4;

    const auto& kicks = Tetromino::getWallKicks(m_currentPiece.getType(), fromRot, toRot);
    Point basePos = m_currentPiece.getPosition();

    for (const auto& kick : kicks) {
        Point testPos = {basePos.x + kick.x, basePos.y + kick.y};
        if (m_board.isValidPosition(m_currentPiece.getType(), toRot, testPos)) {
            m_currentPiece.setRotation(toRot);
            m_currentPiece.setPosition(testPos);

            // Rotating resets lock delay
            if (m_isLocking) {
                if (m_lockResets < Config::MAX_LOCK_RESETS) {
                    m_lockTimer = 0.0f;
                    m_lockResets++;
                }
            }
            m_sounds.playRotate();
            return true;
        }
    }
    return false;
}

void Game::hardDrop() {
    Point ghostPos = m_board.calculateGhostPosition(m_currentPiece);
    int dropDistance = ghostPos.y - m_currentPiece.getPosition().y;

    if (dropDistance > 0) {
        m_score += dropDistance * 2; // +2 points per dropped cell
    }

    m_currentPiece.setPosition(ghostPos);

    // Hard drop particle impact burst
    float blockPx = Config::BLOCK_SIZE;
    float startX = getBoardOffsetX() + ghostPos.x * blockPx;
    float impactY = getBoardOffsetY() + (ghostPos.y + 2) * blockPx;
    m_particles.emitHardDrop(startX, impactY, 3.0f * blockPx, m_currentPiece.getColor());

    m_sounds.playHardDrop();
    onPieceLocked();
}

void Game::holdCurrentPiece() {
    if (!m_canHold || m_board.isAnimatingLineClear()) return;

    m_sounds.playHold();
    m_canHold = false;

    PieceType currentType = m_currentPiece.getType();
    if (m_holdPiece == PieceType::NONE) {
        m_holdPiece = currentType;
        spawnNextPiece();
    } else {
        PieceType swapType = m_holdPiece;
        m_holdPiece = currentType;
        m_currentPiece.reset(swapType);
        m_lockTimer = 0.0f;
        m_lockResets = 0;
        m_isLocking = false;
        m_fallTimer = 0.0f;

        if (!m_board.isValidPosition(m_currentPiece)) {
            gameOver();
        }
    }
}

void Game::onPieceLocked() {
    m_board.lockPiece(m_currentPiece);
    m_currentPiece.reset(PieceType::NONE);
    checkLineClears();
}

void Game::checkLineClears() {
    auto fullRows = m_board.findFullRows();
    if (!fullRows.empty()) {
        int count = static_cast<int>(fullRows.size());
        m_linesCleared += count;

        // Standard Scoring (1: 100, 2: 300, 3: 500, 4: 800) multiplied by level
        int points = 0;
        switch (count) {
            case 1: points = 100 * m_level; break;
            case 2: points = 300 * m_level; break;
            case 3: points = 500 * m_level; break;
            case 4: points = 800 * m_level; break;
        }
        m_score += points;

        // Level up every 10 lines
        int newLevel = (m_linesCleared / 10) + 1;
        if (newLevel > m_level) {
            m_level = newLevel;
            m_sounds.playLevelUp();
        }

        // Particle explosion
        float boardWidth = Config::BOARD_WIDTH * Config::BLOCK_SIZE;
        m_particles.emitLineClear(fullRows, getBoardOffsetX(), getBoardOffsetY(), 
                                  boardWidth, Config::BLOCK_SIZE, count);

        if (count >= 4) {
            m_sounds.playTetris();
            m_tetrisBannerTimer = 1.35f;
        } else {
            m_sounds.playLineClear();
        }

        // Start flash and dissolve animation on board
        m_board.startLineClearAnimation(fullRows);
    } else {
        spawnNextPiece();
    }
}

void Game::handleTouch(int pixelX, int pixelY, bool isDown, unsigned int finger) {
    (void)finger;
    sf::Vector2f pos = m_window.mapPixelToCoords(sf::Vector2i(pixelX, pixelY));

    if (m_state == GameState::START_SCREEN || m_state == GameState::GAME_OVER) {
        if (isDown) {
            startNewGame();
        }
        return;
    }

    if (m_state == GameState::PAUSED) {
        if (isDown) {
            m_state = GameState::PLAYING;
        }
        return;
    }

    if (m_state == GameState::PLAYING) {
        if (isDown) {
            if (m_btnHold.contains(pos)) {
                holdCurrentPiece();
            } else if (m_btnPause.contains(pos)) {
                m_state = GameState::PAUSED;
            } else if (m_btnRotCCW.contains(pos)) {
                tryRotate(false);
            } else if (m_btnRotCW.contains(pos)) {
                tryRotate(true);
            } else if (m_btnLeft.contains(pos)) {
                m_moveDir = -1;
                m_dasTimer = 0.0f;
                m_arrTimer = 0.0f;
                tryMove(-1, 0);
            } else if (m_btnRight.contains(pos)) {
                m_moveDir = 1;
                m_dasTimer = 0.0f;
                m_arrTimer = 0.0f;
                tryMove(1, 0);
            } else if (m_btnHardDrop.contains(pos)) {
                hardDrop();
            } else if (m_btnSoftDrop.contains(pos)) {
                m_softDropActive = true;
                m_softDropTimer = 0.0f;
                if (tryMove(0, 1)) {
                    m_score += 1;
                    m_fallTimer = 0.0f;
                }
            } else if (m_btnMute.contains(pos)) {
                m_sounds.toggleMute();
            } else if (m_btnRestart.contains(pos)) {
                startNewGame();
            }
        } else {
            if (m_btnLeft.contains(pos) || m_btnRight.contains(pos)) {
                m_moveDir = 0;
            }
            if (m_btnSoftDrop.contains(pos)) {
                m_softDropActive = false;
            }
        }
    }
}

void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
            return;
        }

        if (event.type == sf::Event::LostFocus) {
            if (m_state == GameState::PLAYING) {
                m_state = GameState::PAUSED;
            }
        }

        if (event.type == sf::Event::Resized) {
            updateView();
        }

#ifdef __ANDROID__
        if (event.type == sf::Event::TouchBegan) {
            handleTouch(event.touch.x, event.touch.y, true, event.touch.finger);
        } else if (event.type == sf::Event::TouchEnded) {
            handleTouch(event.touch.x, event.touch.y, false, event.touch.finger);
        }
#endif
        if (event.type == sf::Event::MouseButtonPressed) {
            handleTouch(event.mouseButton.x, event.mouseButton.y, true, 0);
        } else if (event.type == sf::Event::MouseButtonReleased) {
            handleTouch(event.mouseButton.x, event.mouseButton.y, false, 0);
        }

        if (event.type == sf::Event::KeyPressed) {
            // Global Hotkeys
            if (event.key.code == sf::Keyboard::P) {
                if (m_state == GameState::PLAYING) {
                    m_state = GameState::PAUSED;
                } else if (m_state == GameState::PAUSED) {
                    m_state = GameState::PLAYING;
                }
                return;
            }

            if (event.key.code == sf::Keyboard::M) {
                m_sounds.toggleMute();
                return;
            }

            // Start screen controls
            if (m_state == GameState::START_SCREEN) {
                if (event.key.code == sf::Keyboard::Space || 
                    event.key.code == sf::Keyboard::Enter) {
                    startNewGame();
                }
                return;
            }

            // Game over restart
            if (m_state == GameState::GAME_OVER) {
                if (event.key.code == sf::Keyboard::R || 
                    event.key.code == sf::Keyboard::Space || 
                    event.key.code == sf::Keyboard::Enter) {
                    startNewGame();
                }
                return;
            }

            // Active Gameplay Controls
            if (m_state == GameState::PLAYING) {
                if (m_board.isAnimatingLineClear()) continue;

                // Move Left
                if (event.key.code == sf::Keyboard::Left) {
                    m_moveDir = -1;
                    m_dasTimer = 0.0f;
                    m_arrTimer = 0.0f;
                    tryMove(-1, 0);
                }
                // Move Right
                else if (event.key.code == sf::Keyboard::Right) {
                    m_moveDir = 1;
                    m_dasTimer = 0.0f;
                    m_arrTimer = 0.0f;
                    tryMove(1, 0);
                }
                // Soft Drop Start
                else if (event.key.code == sf::Keyboard::Down) {
                    m_softDropActive = true;
                    m_softDropTimer = 0.0f;
                    if (tryMove(0, 1)) {
                        m_score += 1;
                        m_fallTimer = 0.0f;
                    }
                }
                // Hard Drop
                else if (event.key.code == sf::Keyboard::Space) {
                    hardDrop();
                }
                // Rotate CW (Up Arrow or X)
                else if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::X) {
                    tryRotate(true);
                }
                // Rotate CCW (Z or Left Ctrl)
                else if (event.key.code == sf::Keyboard::Z || event.key.code == sf::Keyboard::LControl) {
                    tryRotate(false);
                }
                // Hold (C or Left Shift)
                else if (event.key.code == sf::Keyboard::C || event.key.code == sf::Keyboard::LShift) {
                    holdCurrentPiece();
                }
                // Instant Restart during gameplay
                else if (event.key.code == sf::Keyboard::R) {
                    startNewGame();
                }
            }
        }

        if (event.type == sf::Event::KeyReleased) {
            if (m_state == GameState::PLAYING) {
                if (event.key.code == sf::Keyboard::Left) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                        m_moveDir = 1;
                        m_dasTimer = 0.0f;
                        m_arrTimer = 0.0f;
                        tryMove(1, 0);
                    } else {
                        m_moveDir = 0;
                    }
                } else if (event.key.code == sf::Keyboard::Right) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                        m_moveDir = -1;
                        m_dasTimer = 0.0f;
                        m_arrTimer = 0.0f;
                        tryMove(-1, 0);
                    } else {
                        m_moveDir = 0;
                    }
                } else if (event.key.code == sf::Keyboard::Down) {
                    m_softDropActive = false;
                }
            }
        }
    }
}

void Game::update(float dt) {
    m_particles.update(dt);

    if (m_state != GameState::PLAYING) {
        return;
    }

    // Animated score roll-up
    if (m_displayScore < m_score) {
        int diff = m_score - m_displayScore;
        int step = std::max(1, static_cast<int>(diff * 14.0f * dt));
        m_displayScore = std::min(m_score, m_displayScore + step);
    }

    // Tetris banner decay
    if (m_tetrisBannerTimer > 0.0f) {
        m_tetrisBannerTimer -= dt;
    }

    // Check line clear animation status
    if (m_board.isAnimatingLineClear()) {
        m_board.update(dt);
        if (!m_board.isAnimatingLineClear()) {
            spawnNextPiece();
        }
        return; // Pause piece falling while line clears animate
    }

    // Horizontal Movement (DAS & ARR Handling)
    if (m_moveDir != 0) {
        m_dasTimer += dt;
        if (m_dasTimer >= Config::DAS_DELAY) {
            m_arrTimer += dt;
            while (m_arrTimer >= Config::ARR_INTERVAL) {
                m_arrTimer -= Config::ARR_INTERVAL;
                if (!tryMove(m_moveDir, 0)) {
                    break;
                }
            }
        }
    }

    // Soft Drop Tick
    if (m_softDropActive) {
        m_softDropTimer += dt;
        while (m_softDropTimer >= Config::SOFT_DROP_INTERVAL) {
            m_softDropTimer -= Config::SOFT_DROP_INTERVAL;
            if (tryMove(0, 1)) {
                m_score += 1;
                m_fallTimer = 0.0f;
            }
        }
    }

    // Natural Gravity Fall
    float fallInterval = getFallInterval();
    m_fallTimer += dt;
    if (m_fallTimer >= fallInterval) {
        m_fallTimer = 0.0f;
        if (!tryMove(0, 1)) {
            m_isLocking = true;
        }
    }

    // Lock Delay Logic
    Point curPos = m_currentPiece.getPosition();
    bool onGround = !m_board.isValidPosition(m_currentPiece.getType(), m_currentPiece.getRotation(), 
                                             {curPos.x, curPos.y + 1});

    if (onGround) {
        m_isLocking = true;
        m_lockTimer += dt;
        if (m_lockTimer >= Config::LOCK_DELAY) {
            onPieceLocked();
        }
    } else {
        m_isLocking = false;
        m_lockTimer = 0.0f;
    }
}

void Game::renderText(const std::string& str, float x, float y, unsigned int size, 
                      sf::Color color, bool center, bool bold) 
{
    if (!m_fontLoaded) return;

    sf::Text text;
    text.setFont(m_font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);
    if (bold) {
        text.setStyle(sf::Text::Bold);
    }

    if (center) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(std::round(bounds.left + bounds.width / 2.0f), 
                       std::round(bounds.top + bounds.height / 2.0f));
    }
    text.setPosition(x, y);
    m_window.draw(text);
}

void Game::drawCard(float x, float y, float width, float height, const std::string& title) {
    sf::RectangleShape bg(sf::Vector2f(width, height));
    bg.setPosition(x, y);
    bg.setFillColor(Config::PANEL_BG);
    bg.setOutlineThickness(1.5f);
    bg.setOutlineColor(Config::PANEL_BORDER);
    m_window.draw(bg);

    if (!title.empty()) {
        renderText(title, x + 16.0f, y + 14.0f, 13, Config::TEXT_MUTED, false, true);
    }
}

void Game::drawMobileControls() {
    auto drawBtn = [this](const sf::FloatRect& r, const std::string& label, sf::Color borderCol) {
        sf::RectangleShape box(sf::Vector2f(r.width, r.height));
        box.setPosition(r.left, r.top);
        box.setFillColor(sf::Color(18, 24, 40, 220));
        box.setOutlineThickness(2.0f);
        box.setOutlineColor(borderCol);
        m_window.draw(box);

        renderText(label, r.left + r.width / 2.0f, r.top + r.height / 2.0f, 20, sf::Color(235, 245, 255), true, true);
    };

    drawBtn(m_btnHold, "HOLD", Config::ACCENT_CYAN);
    drawBtn(m_btnRotCCW, "ROT L", Config::ACCENT_VIOLET);
    drawBtn(m_btnRotCW, "ROT R", Config::ACCENT_VIOLET);
    drawBtn(m_btnPause, "PAUSE", Config::PANEL_BORDER);

    drawBtn(m_btnLeft, "< LEFT", Config::ACCENT_CYAN);
    drawBtn(m_btnHardDrop, "HARD DROP", sf::Color(0, 240, 255));
    drawBtn(m_btnRight, "RIGHT >", Config::ACCENT_CYAN);

    drawBtn(m_btnMute, "MUTE", Config::TEXT_MUTED);
    drawBtn(m_btnSoftDrop, "SOFT DROP", sf::Color(50, 240, 80));
    drawBtn(m_btnRestart, "RESTART", sf::Color(255, 45, 75));
}

void Game::drawHUD() {
    float boardX = getBoardOffsetX();
    float boardY = getBoardOffsetY();
    float blockSize = Config::BLOCK_SIZE;

#ifdef __ANDROID__
    // Mobile Top Bar
    drawCard(60.0f, 30.0f, 260.0f, 170.0f, "SCORE");
    renderText(std::to_string(m_displayScore), 80.0f, 85.0f, 36, Config::ACCENT_CYAN, false, true);
    renderText("BEST: " + std::to_string(m_highScore), 80.0f, 140.0f, 18, Config::TEXT_MUTED, false, false);

    drawCard(350.0f, 30.0f, 260.0f, 170.0f, "LEVEL & SPEED");
    renderText("LEVEL " + std::to_string(m_level), 370.0f, 85.0f, 32, sf::Color(255, 215, 0), false, true);
    renderText("LINES: " + std::to_string(m_linesCleared), 370.0f, 140.0f, 18, Config::TEXT_MUTED, false, false);

    drawCard(640.0f, 30.0f, 260.0f, 170.0f, "STATUS");
    renderText("CYBERPUNK", 660.0f, 85.0f, 22, Config::ACCENT_VIOLET, false, true);
    renderText("ARCADE 60FPS", 660.0f, 140.0f, 18, Config::ACCENT_CYAN, false, false);

    // Left Panel: HOLD BOX
    float leftX = 60.0f;
    drawCard(leftX, boardY, 220.0f, 180.0f, "HOLD PIECE");
    if (m_holdPiece != PieceType::NONE) {
        float holdAlpha = m_canHold ? 1.0f : 0.45f;
        Tetromino::drawPreview(m_window, m_holdPiece, leftX + 110.0f, boardY + 105.0f, 28.0f, holdAlpha);
    }

    // Right Panel: NEXT QUEUE
    float rightX = boardX + Config::BOARD_WIDTH * blockSize + 40.0f;
    drawCard(rightX, boardY, 220.0f, 420.0f, "NEXT QUEUE");
    float nextY = boardY + 80.0f;
    for (int i = 0; i < 3; ++i) {
        PieceType p = m_bag.peek(i);
        Tetromino::drawPreview(m_window, p, rightX + 110.0f, nextY, 26.0f, 1.0f);
        nextY += 105.0f;
    }

    // Mobile Touch Controls
    drawMobileControls();

#else
    // Desktop HUD Layout
    float leftX = 60.0f;
    drawCard(leftX, boardY, 210.0f, 150.0f, "HOLD [C / LSHIFT]");
    if (m_holdPiece != PieceType::NONE) {
        float holdAlpha = m_canHold ? 1.0f : 0.45f;
        Tetromino::drawPreview(m_window, m_holdPiece, leftX + 105.0f, boardY + 88.0f, 26.0f, holdAlpha);
    }

    drawCard(leftX, boardY + 170.0f, 210.0f, 150.0f, "STATISTICS");
    renderText("LEVEL", leftX + 16.0f, boardY + 204.0f, 13, Config::TEXT_MUTED, false, true);
    renderText(std::to_string(m_level), leftX + 16.0f, boardY + 220.0f, 28, Config::ACCENT_CYAN, false, true);

    renderText("LINES CLEARED", leftX + 16.0f, boardY + 258.0f, 13, Config::TEXT_MUTED, false, true);
    renderText(std::to_string(m_linesCleared), leftX + 16.0f, boardY + 274.0f, 28, Config::TEXT_PRIMARY, false, true);

    drawCard(leftX, boardY + 340.0f, 210.0f, 300.0f, "CONTROLS");
    float cy = boardY + 372.0f;
    float rowH = 26.0f;
    auto drawControlRow = [&](const std::string& key, const std::string& act) {
        renderText(key, leftX + 14.0f, cy, 12, Config::ACCENT_CYAN, false, true);
        renderText(act, leftX + 110.0f, cy, 12, Config::TEXT_PRIMARY, false, false);
        cy += rowH;
    };
    drawControlRow("LEFT/RIGHT", "Move (DAS)");
    drawControlRow("DOWN", "Soft Drop (+1)");
    drawControlRow("SPACE", "Hard Drop (+2)");
    drawControlRow("UP / X", "Rotate CW");
    drawControlRow("Z / LCTRL", "Rotate CCW");
    drawControlRow("C / LSHIFT", "Hold Piece");
    drawControlRow("P", "Pause Game");
    drawControlRow("R", "Restart Game");
    drawControlRow("M", "Toggle Sound");

    float rightX = boardX + Config::BOARD_WIDTH * blockSize + 50.0f;
    drawCard(rightX, boardY, 210.0f, 290.0f, "NEXT PIECES");
    float nextY = boardY + 68.0f;
    for (int i = 0; i < 3; ++i) {
        PieceType p = m_bag.peek(i);
        Tetromino::drawPreview(m_window, p, rightX + 105.0f, nextY, 24.0f, 1.0f);
        nextY += 76.0f;
    }

    drawCard(rightX, boardY + 310.0f, 210.0f, 150.0f, "SCORE");
    renderText("CURRENT", rightX + 16.0f, boardY + 344.0f, 13, Config::TEXT_MUTED, false, true);
    renderText(std::to_string(m_displayScore), rightX + 16.0f, boardY + 360.0f, 30, Config::ACCENT_CYAN, false, true);

    renderText("HIGH SCORE", rightX + 16.0f, boardY + 404.0f, 13, Config::TEXT_MUTED, false, true);
    renderText(std::to_string(m_highScore), rightX + 16.0f, boardY + 420.0f, 24, sf::Color(255, 215, 0), false, true);
#endif
}

void Game::drawStartScreen() {
    float cx = m_window.getView().getSize().x / 2.0f;
    float cy = m_window.getView().getSize().y / 2.0f;

    renderText("CYBERPUNK", cx, cy - 140.0f, 54, Config::ACCENT_CYAN, true, true);
    renderText("T E T R I S", cx, cy - 70.0f, 44, Config::TEXT_PRIMARY, true, true);
    renderText("NEON ARCADE // SFML 2.6.2", cx, cy - 15.0f, 16, Config::ACCENT_VIOLET, true, false);

#ifdef __ANDROID__
    renderText("TAP ANYWHERE TO START", cx, cy + 90.0f, 24, Config::COLOR_S, true, true);
#else
    renderText("PRESS SPACE OR ENTER TO START", cx, cy + 90.0f, 20, Config::COLOR_S, true, true);
#endif

    if (m_highScore > 0) {
        renderText("RECORD HIGH SCORE: " + std::to_string(m_highScore), cx, cy + 160.0f, 16, sf::Color(255, 215, 0), true, true);
    }
}

void Game::drawPauseScreen() {
    float cx = m_window.getView().getSize().x / 2.0f;
    float cy = m_window.getView().getSize().y / 2.0f;

    sf::RectangleShape dim(m_window.getView().getSize());
    dim.setFillColor(sf::Color(11, 14, 23, 210));
    m_window.draw(dim);

    renderText("GAME PAUSED", cx, cy - 40.0f, 42, Config::TEXT_PRIMARY, true, true);
#ifdef __ANDROID__
    renderText("TAP ANYWHERE TO RESUME", cx, cy + 30.0f, 20, Config::ACCENT_CYAN, true, false);
#else
    renderText("PRESS P TO RESUME", cx, cy + 30.0f, 18, Config::ACCENT_CYAN, true, false);
#endif
}

void Game::drawGameOverScreen() {
    float cx = m_window.getView().getSize().x / 2.0f;
    float cy = m_window.getView().getSize().y / 2.0f;

    sf::RectangleShape dim(m_window.getView().getSize());
    dim.setFillColor(sf::Color(11, 14, 23, 230));
    m_window.draw(dim);

    renderText("GAME OVER", cx, cy - 110.0f, 50, Config::COLOR_Z, true, true);

    renderText("FINAL SCORE", cx, cy - 35.0f, 14, Config::TEXT_MUTED, true, true);
    renderText(std::to_string(m_score), cx, cy - 5.0f, 40, Config::ACCENT_CYAN, true, true);

    if (m_score >= m_highScore && m_score > 0) {
        renderText("★ NEW HIGH SCORE! ★", cx, cy + 45.0f, 18, sf::Color(255, 215, 0), true, true);
    }

#ifdef __ANDROID__
    renderText("TAP ANYWHERE TO REPLAY", cx, cy + 110.0f, 22, Config::COLOR_S, true, true);
#else
    renderText("PRESS R OR SPACE TO RESTART", cx, cy + 110.0f, 18, Config::COLOR_S, true, true);
#endif
}

void Game::render() {
    m_window.clear(Config::BG_COLOR);

    // Camera screen shake offset
    sf::Vector2f shake = m_particles.getShakeOffset();
    sf::View view = m_window.getView();
    view.move(shake);
    m_window.setView(view);

    if (m_state == GameState::START_SCREEN) {
        drawStartScreen();
    } else {
        // Draw Matrix & HUD
        drawHUD();
        m_board.draw(m_window, getBoardOffsetX(), getBoardOffsetY(), Config::BLOCK_SIZE);

        // Draw Ghost & Active piece
        if (m_state == GameState::PLAYING && !m_board.isAnimatingLineClear() && 
            m_currentPiece.getType() != PieceType::NONE) 
        {
            // Ghost Piece
            Tetromino ghostPiece = m_currentPiece;
            ghostPiece.setPosition(m_board.calculateGhostPosition(m_currentPiece));
            ghostPiece.draw(m_window, getBoardOffsetX(), getBoardOffsetY(), Config::BLOCK_SIZE, 0.7f, true);

            // Active Falling Piece
            m_currentPiece.draw(m_window, getBoardOffsetX(), getBoardOffsetY(), Config::BLOCK_SIZE, 1.0f, false);
        }

        // Particle sparks
        m_particles.draw(m_window);

        // Tetris 4-Line Clear Celebration Banner
        if (m_tetrisBannerTimer > 0.0f) {
            float bannerX = getBoardOffsetX() + 160.0f;
            float bannerY = getBoardOffsetY() + 300.0f;

            sf::RectangleShape bannerBox(sf::Vector2f(280.0f, 54.0f));
            bannerBox.setOrigin(140.0f, 27.0f);
            bannerBox.setPosition(bannerX, bannerY);
            bannerBox.setFillColor(sf::Color(18, 24, 40, 240));
            bannerBox.setOutlineThickness(2.0f);
            bannerBox.setOutlineColor(Config::ACCENT_CYAN);
            m_window.draw(bannerBox);

            renderText("T E T R I S !", bannerX, bannerY, 26, Config::ACCENT_CYAN, true, true);
        }

        if (m_state == GameState::PAUSED) {
            drawPauseScreen();
        } else if (m_state == GameState::GAME_OVER) {
            drawGameOverScreen();
        }
    }

    // Reset view offset after shake
    view.move(-shake);
    m_window.setView(view);

    m_window.display();
}

void Game::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.1f);

        processEvents();
        update(dt);
        render();
    }
}
