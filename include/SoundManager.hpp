#pragma once
#include <SFML/Audio.hpp>
#include <vector>
#include <memory>

class SoundManager {
public:
    SoundManager();

    void playMove();
    void playRotate();
    void playHardDrop();
    void playHold();
    void playLineClear();
    void playTetris();
    void playGameOver();
    void playLevelUp();

    void setMuted(bool muted) { m_muted = muted; }
    bool isMuted() const { return m_muted; }
    void toggleMute() { m_muted = !m_muted; }

private:
    void initBuffers();
    void playSound(const sf::SoundBuffer& buffer, float pitch = 1.0f, float volume = 80.0f);

    // Procedural sound generators
    std::vector<sf::Int16> generateMoveSound();
    std::vector<sf::Int16> generateRotateSound();
    std::vector<sf::Int16> generateHardDropSound();
    std::vector<sf::Int16> generateHoldSound();
    std::vector<sf::Int16> generateLineClearSound();
    std::vector<sf::Int16> generateTetrisSound();
    std::vector<sf::Int16> generateGameOverSound();
    std::vector<sf::Int16> generateLevelUpSound();

    sf::SoundBuffer m_bufMove;
    sf::SoundBuffer m_bufRotate;
    sf::SoundBuffer m_bufHardDrop;
    sf::SoundBuffer m_bufHold;
    sf::SoundBuffer m_bufLineClear;
    sf::SoundBuffer m_bufTetris;
    sf::SoundBuffer m_bufGameOver;
    sf::SoundBuffer m_bufLevelUp;

    // Polyphonic sound pool to handle overlapping audio
    static constexpr size_t POOL_SIZE = 16;
    std::vector<sf::Sound> m_soundPool;
    size_t m_poolIndex;

    bool m_muted;
};
