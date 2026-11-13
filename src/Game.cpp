#include "Game.hpp"
#include <fstream>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <sstream>

Game::Game()
    : m_window(sf::VideoMode(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT), "TETRIS // NEON ARCADE", sf::Style::Titlebar | sf::Style::Close),
      m_holdPiece(PieceType::NONE),
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
    m_window.setFramerateLimit(60);

    // Load system font (Windows has segoeui.ttf, arial.ttf, consola.ttf)
    const std::vector<std::string> fontCandidates = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/tahoma.ttf"
    };

    for (const auto& path : fontCandidates) {
        if (m_font.loadFromFile(path)) {
            m_fontLoaded = true;
            break;
        }
    }

    loadHighScore();
}

void Game::loadHighScore() {
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> m_highScore;
        file.close();
    } else {
        m_highScore = 0;
    }
}

void Game::saveHighScore() {
    std::ofstream file("highscore.txt");
    if (file.is_open()) {
        file << m_highScore;
        file.close();
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
    float startX = Config::BOARD_OFFSET_X + ghostPos.x * blockPx;
    float impactY = Config::BOARD_OFFSET_Y + (ghostPos.y + 2) * blockPx;
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
        m_particles.emitLineClear(fullRows, Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, 
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

void Game::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
            return;
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

    // Roll-up animated score
    if (m_displayScore < m_score) {
        int diff = m_score - m_displayScore;
        int step = std::max(1, static_cast<int>(diff * 12.0f * dt));
        m_displayScore = std::min(m_score, m_displayScore + step);
    }

    if (m_tetrisBannerTimer > 0.0f) {
        m_tetrisBannerTimer -= dt;
    }

    if (m_state != GameState::PLAYING) return;

    // Handle board line clear animation
    if (m_board.isAnimatingLineClear()) {
        m_board.update(dt);
        if (!m_board.isAnimatingLineClear()) {
            spawnNextPiece();
        }
        return;
    }

    // Horizontal Auto-Shift (DAS / ARR)
    if (m_moveDir != 0) {
        m_dasTimer += dt;
        if (m_dasTimer >= Config::DAS_DELAY) {
            m_arrTimer += dt;
            while (m_arrTimer >= Config::ARR_INTERVAL) {
                m_arrTimer -= Config::ARR_INTERVAL;
                tryMove(m_moveDir, 0);
            }
        }
    }

    // Soft Drop Continuous
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

    // Check if piece is touching stack/floor for Lock Delay
    Point currentPos = m_currentPiece.getPosition();
    bool touchesFloor = !m_board.isValidPosition(m_currentPiece.getType(), 
                                                 m_currentPiece.getRotation(), 
                                                 Point{currentPos.x, currentPos.y + 1});

    if (touchesFloor) {
        m_isLocking = true;
        m_lockTimer += dt;
        if (m_lockTimer >= Config::LOCK_DELAY) {
            onPieceLocked();
            return;
        }
    } else {
        m_isLocking = false;
        m_lockTimer = 0.0f;
    }

    // Normal Gravity Fall
    m_fallTimer += dt;
    if (m_fallTimer >= getFallInterval()) {
        m_fallTimer = 0.0f;
        if (!tryMove(0, 1)) {
            // Reached bottom, lock delay handles the rest
            m_isLocking = true;
        }
    }
}

void Game::drawCard(float x, float y, float width, float height, const std::string& title) {
    // Outer card rectangle with subtle border
    sf::RectangleShape card(sf::Vector2f(width, height));
    card.setPosition(x, y);
    card.setFillColor(Config::PANEL_BG);
    card.setOutlineThickness(1.5f);
    card.setOutlineColor(Config::PANEL_BORDER);
    m_window.draw(card);

    // Header bar if title provided
    if (!title.empty()) {
        sf::RectangleShape titleBar(sf::Vector2f(width, 24.0f));
        titleBar.setPosition(x, y);
        titleBar.setFillColor(sf::Color(25, 34, 55, 200));
        m_window.draw(titleBar);

        renderText(title, x + 10.0f, y + 4.0f, 13, Config::TEXT_MUTED, false, true);
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

void Game::drawHUD() {
    float boardX = Config::BOARD_OFFSET_X;
    float boardY = Config::BOARD_OFFSET_Y;
    float blockSize = Config::BLOCK_SIZE;

    // LEFT PANEL: HOLD BOX
    float leftX = 60.0f;
    drawCard(leftX, boardY, 210.0f, 150.0f, "HOLD [C / LSHIFT]");
    if (m_holdPiece != PieceType::NONE) {
        float holdAlpha = m_canHold ? 1.0f : 0.45f;
        Tetromino::drawPreview(m_window, m_holdPiece, leftX + 105.0f, boardY + 88.0f, 26.0f, holdAlpha);
    }

    // LEFT PANEL: LEVEL & LINES CARD
    drawCard(leftX, boardY + 170.0f, 210.0f, 150.0f, "STATISTICS");
    renderText("LEVEL", leftX + 16.0f, boardY + 204.0f, 13, Config::TEXT_MUTED, false, true);
    renderText(std::to_string(m_level), leftX + 16.0f, boardY + 220.0f, 28, Config::ACCENT_CYAN, false, true);

    renderText("LINES CLEARED", leftX + 16.0f, boardY + 258.0f, 13, Config::TEXT_MUTED, false, true);
    renderText(std::to_string(m_linesCleared), leftX + 16.0f, boardY + 274.0f, 28, Config::TEXT_PRIMARY, false, true);

    // LEFT PANEL: CONTROLS LEGEND
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

    // RIGHT PANEL: NEXT PIECES
    float rightX = boardX + Config::BOARD_WIDTH * blockSize + 50.0f;
    drawCard(rightX, boardY, 210.0f, 290.0f, "NEXT PIECES");
    // Show top 3 upcoming pieces
    float nextY = boardY + 68.0f;
    for (int i = 0; i < 3; ++i) {
        PieceType p = m_bag.peek(i);
        Tetromino::drawPreview(m_window, p, rightX + 105.0f, nextY, (i == 0) ? 26.0f : 20.0f, (i == 0) ? 1.0f : 0.8f);
        nextY += (i == 0) ? 80.0f : 70.0f;
    }

    // RIGHT PANEL: SCORE & HIGH SCORE
    drawCard(rightX, boardY + 310.0f, 210.0f, 170.0f, "SCORE");
    renderText("CURRENT SCORE", rightX + 16.0f, boardY + 344.0f, 13, Config::TEXT_MUTED, false, true);
    renderText(std::to_string(m_displayScore), rightX + 16.0f, boardY + 360.0f, 28, Config::COLOR_O, false, true);

    renderText("HIGH SCORE", rightX + 16.0f, boardY + 404.0f, 13, Config::TEXT_MUTED, false, true);
    renderText(std::to_string(std::max(m_highScore, m_score)), rightX + 16.0f, boardY + 420.0f, 26, Config::ACCENT_VIOLET, false, true);

    // Audio Mute Indicator at bottom right
    if (m_sounds.isMuted()) {
        renderText("[AUDIO MUTED]", rightX + 105.0f, boardY + 500.0f, 13, Config::COLOR_Z, true, true);
    }

    // Celebratory TETRIS 4-Line Clear Banner
    if (m_tetrisBannerTimer > 0.0f) {
        float alpha = std::clamp(m_tetrisBannerTimer / 1.35f, 0.0f, 1.0f);
        sf::Uint8 a = static_cast<sf::Uint8>(255 * alpha);

        float bannerY = boardY + Config::BOARD_HEIGHT * blockSize * 0.42f;
        sf::RectangleShape banner(sf::Vector2f(Config::BOARD_WIDTH * blockSize, 60.0f));
        banner.setPosition(boardX, bannerY - 30.0f);
        banner.setFillColor(sf::Color(10, 14, 25, static_cast<sf::Uint8>(210 * alpha)));
        banner.setOutlineThickness(2.0f);
        banner.setOutlineColor(sf::Color(0, 240, 255, a));
        m_window.draw(banner);

        renderText("TETRIS! +800", boardX + (Config::BOARD_WIDTH * blockSize) / 2.0f, bannerY, 32, 
                   sf::Color(0, 240, 255, a), true, true);
    }
}

void Game::drawStartScreen() {
    // Translucent dark gradient overlay
    sf::RectangleShape overlay(sf::Vector2f(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(11, 14, 23, 235));
    m_window.draw(overlay);

    float cx = Config::WINDOW_WIDTH / 2.0f;
    float cy = Config::WINDOW_HEIGHT / 2.0f;

    // Glowing Logo Title
    renderText("CYBERPUNK", cx, cy - 130.0f, 22, Config::ACCENT_CYAN, true, true);
    renderText("T E T R I S", cx, cy - 75.0f, 62, Config::COLOR_O, true, true);
    renderText("NEON ARCADE EDITION", cx, cy - 15.0f, 16, Config::TEXT_MUTED, true, true);

    // High score banner
    std::string hsStr = "HIGH SCORE: " + std::to_string(m_highScore);
    renderText(hsStr, cx, cy + 45.0f, 20, Config::ACCENT_VIOLET, true, true);

    // Pulsing Start Prompt
    renderText("PRESS [SPACE] OR [ENTER] TO START", cx, cy + 120.0f, 22, Config::TEXT_PRIMARY, true, true);
    renderText("P: Pause  |  M: Mute Audio  |  C: Hold Piece", cx, cy + 175.0f, 14, Config::TEXT_MUTED, true, false);
}

void Game::drawPauseScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(11, 14, 23, 200));
    m_window.draw(overlay);

    float cx = Config::WINDOW_WIDTH / 2.0f;
    float cy = Config::WINDOW_HEIGHT / 2.0f;

    drawCard(cx - 180.0f, cy - 110.0f, 360.0f, 220.0f);
    renderText("PAUSED", cx, cy - 45.0f, 44, Config::ACCENT_CYAN, true, true);
    renderText("PRESS [P] TO RESUME", cx, cy + 25.0f, 20, Config::TEXT_PRIMARY, true, true);
    renderText("PRESS [R] TO RESTART", cx, cy + 65.0f, 15, Config::TEXT_MUTED, true, false);
}

void Game::drawGameOverScreen() {
    sf::RectangleShape overlay(sf::Vector2f(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(20, 8, 14, 215));
    m_window.draw(overlay);

    float cx = Config::WINDOW_WIDTH / 2.0f;
    float cy = Config::WINDOW_HEIGHT / 2.0f;

    drawCard(cx - 210.0f, cy - 160.0f, 420.0f, 320.0f);

    renderText("GAME OVER", cx, cy - 105.0f, 46, Config::COLOR_Z, true, true);

    renderText("FINAL SCORE", cx, cy - 40.0f, 15, Config::TEXT_MUTED, true, true);
    renderText(std::to_string(m_score), cx, cy - 12.0f, 34, Config::COLOR_O, true, true);

    if (m_score >= m_highScore && m_score > 0) {
        renderText("* NEW HIGH SCORE! *", cx, cy + 35.0f, 18, Config::ACCENT_CYAN, true, true);
    } else {
        renderText("HIGH SCORE: " + std::to_string(m_highScore), cx, cy + 35.0f, 16, Config::ACCENT_VIOLET, true, true);
    }

    renderText("PRESS [R] OR [SPACE] TO RESTART", cx, cy + 95.0f, 18, Config::TEXT_PRIMARY, true, true);
}

void Game::render() {
    m_window.clear(Config::BG_COLOR);

    // Apply screen shake offset from explosions/drops
    sf::Vector2f shake = m_particles.getShakeOffset();
    sf::View view = m_window.getDefaultView();
    view.setCenter(view.getCenter() + shake);
    m_window.setView(view);

    // 1. Draw Board
    m_board.draw(m_window, Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, Config::BLOCK_SIZE);

    // 2. Draw Ghost Piece and Active Piece during gameplay
    if (m_state == GameState::PLAYING || m_state == GameState::PAUSED) {
        if (m_currentPiece.getType() != PieceType::NONE && !m_board.isAnimatingLineClear()) {
            // Draw Ghost Piece
            Point ghostPos = m_board.calculateGhostPosition(m_currentPiece);
            Tetromino ghostPiece = m_currentPiece;
            ghostPiece.setPosition(ghostPos);
            ghostPiece.draw(m_window, Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, Config::BLOCK_SIZE, 0.7f, true);

            // Draw Active Piece
            m_currentPiece.draw(m_window, Config::BOARD_OFFSET_X, Config::BOARD_OFFSET_Y, Config::BLOCK_SIZE, 1.0f, false);
        }
    }

    // 3. Draw Particles
    m_particles.draw(m_window);

    // Reset view for static HUD and overlays
    m_window.setView(m_window.getDefaultView());

    // 4. Draw HUD
    drawHUD();

    // 5. Draw State Overlays
    if (m_state == GameState::START_SCREEN) {
        drawStartScreen();
    } else if (m_state == GameState::PAUSED) {
        drawPauseScreen();
    } else if (m_state == GameState::GAME_OVER) {
        drawGameOverScreen();
    }

    m_window.display();
}

void Game::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        // Prevent huge delta spikes on lag
        dt = std::min(dt, 0.1f);

        processEvents();
        update(dt);
        render();
    }
}
