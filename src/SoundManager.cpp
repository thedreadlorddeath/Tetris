#include "SoundManager.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace {
    constexpr unsigned int SAMPLE_RATE = 44100;
    constexpr double PI = 3.14159265358979323846;

    // Helper: Clamps a normalized float sample [-1.0, 1.0] to signed 16-bit PCM
    inline sf::Int16 toSample(float v) {
        v = std::clamp(v, -1.0f, 1.0f);
        return static_cast<sf::Int16>(v * 32767.0f);
    }
}

SoundManager::SoundManager()
    : m_poolIndex(0), m_muted(false)
{
    try {
        m_soundPool.resize(POOL_SIZE);
        initBuffers();
    } catch (...) {
        m_muted = true;
    }
}

void SoundManager::initBuffers() {
    try {
        auto samplesMove = generateMoveSound();
        m_bufMove.loadFromSamples(samplesMove.data(), samplesMove.size(), 1, SAMPLE_RATE);

        auto samplesRotate = generateRotateSound();
        m_bufRotate.loadFromSamples(samplesRotate.data(), samplesRotate.size(), 1, SAMPLE_RATE);

        auto samplesHardDrop = generateHardDropSound();
        m_bufHardDrop.loadFromSamples(samplesHardDrop.data(), samplesHardDrop.size(), 1, SAMPLE_RATE);

        auto samplesHold = generateHoldSound();
        m_bufHold.loadFromSamples(samplesHold.data(), samplesHold.size(), 1, SAMPLE_RATE);

        auto samplesLineClear = generateLineClearSound();
        m_bufLineClear.loadFromSamples(samplesLineClear.data(), samplesLineClear.size(), 1, SAMPLE_RATE);

        auto samplesTetris = generateTetrisSound();
        m_bufTetris.loadFromSamples(samplesTetris.data(), samplesTetris.size(), 1, SAMPLE_RATE);

        auto samplesGameOver = generateGameOverSound();
        m_bufGameOver.loadFromSamples(samplesGameOver.data(), samplesGameOver.size(), 1, SAMPLE_RATE);

        auto samplesLevelUp = generateLevelUpSound();
        m_bufLevelUp.loadFromSamples(samplesLevelUp.data(), samplesLevelUp.size(), 1, SAMPLE_RATE);
    } catch (...) {
        m_muted = true;
    }
}

void SoundManager::playSound(const sf::SoundBuffer& buffer, float pitch, float volume) {
    if (m_muted) return;

    try {
        sf::Sound& sound = m_soundPool[m_poolIndex];
        m_poolIndex = (m_poolIndex + 1) % POOL_SIZE;

        sound.setBuffer(buffer);
        sound.setPitch(pitch);
        sound.setVolume(volume);
        sound.play();
    } catch (...) {
    }
}

void SoundManager::playMove() {
    playSound(m_bufMove, 1.0f, 60.0f);
}

void SoundManager::playRotate() {
    playSound(m_bufRotate, 1.0f, 75.0f);
}

void SoundManager::playHardDrop() {
    playSound(m_bufHardDrop, 1.0f, 90.0f);
}

void SoundManager::playHold() {
    playSound(m_bufHold, 1.0f, 70.0f);
}

void SoundManager::playLineClear() {
    playSound(m_bufLineClear, 1.0f, 85.0f);
}

void SoundManager::playTetris() {
    playSound(m_bufTetris, 1.0f, 95.0f);
}

void SoundManager::playGameOver() {
    playSound(m_bufGameOver, 1.0f, 90.0f);
}

void SoundManager::playLevelUp() {
    playSound(m_bufLevelUp, 1.0f, 85.0f);
}

// Move: Short high blip
std::vector<sf::Int16> SoundManager::generateMoveSound() {
    float duration = 0.035f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(count);

    float freq = 380.0f;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float env = std::exp(-t * 85.0f); // Quick exponential decay
        float wave = std::sin(2.0f * PI * freq * t);
        samples[i] = toSample(wave * env * 0.4f);
    }
    return samples;
}

// Rotate: Quick frequency sweep upward
std::vector<sf::Int16> SoundManager::generateRotateSound() {
    float duration = 0.045f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(count);

    float startFreq = 480.0f;
    float endFreq = 720.0f;
    double phase = 0.0;

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;
        float freq = startFreq + (endFreq - startFreq) * progress;
        phase += 2.0 * PI * freq / SAMPLE_RATE;

        float env = (progress < 0.15f) ? (progress / 0.15f) : (1.0f - (progress - 0.15f) / 0.85f);
        // Blend sine and soft square for retro warmth
        float wave = std::sin(phase) * 0.7f + (std::sin(phase) > 0 ? 0.2f : -0.2f);
        samples[i] = toSample(wave * env * 0.45f);
    }
    return samples;
}

// Hard Drop: Heavy bass impact thud
std::vector<sf::Int16> SoundManager::generateHardDropSound() {
    float duration = 0.09f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(count);

    double phase = 0.0;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;
        // Frequency sweep from 150 Hz down to 38 Hz
        float freq = 150.0f * std::exp(-progress * 3.2f);
        phase += 2.0 * PI * freq / SAMPLE_RATE;

        float env = std::exp(-progress * 6.5f);
        float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.25f * std::exp(-progress * 20.0f);
        float wave = std::sin(phase) + noise;
        samples[i] = toSample(wave * env * 0.75f);
    }
    return samples;
}

// Hold: Subtle double-tick
std::vector<sf::Int16> SoundManager::generateHoldSound() {
    float duration = 0.05f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(count);

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float wave = 0.0f;
        // First tick at 0ms, second at 22ms
        if (t < 0.015f) {
            float env = std::exp(-t * 220.0f);
            wave = std::sin(2.0f * PI * 650.0f * t) * env;
        } else if (t >= 0.022f && t < 0.038f) {
            float t2 = t - 0.022f;
            float env = std::exp(-t2 * 220.0f);
            wave = std::sin(2.0f * PI * 880.0f * t2) * env;
        }
        samples[i] = toSample(wave * 0.5f);
    }
    return samples;
}

// Line Clear: Bright pleasant 3-tone arpeggio (C5 -> E5 -> G5)
std::vector<sf::Int16> SoundManager::generateLineClearSound() {
    float duration = 0.22f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(count);

    float notes[3] = {523.25f, 659.25f, 783.99f};
    float noteDur = duration / 3.0f;

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        int noteIndex = std::min(2, static_cast<int>(t / noteDur));
        float noteTime = t - noteIndex * noteDur;

        float freq = notes[noteIndex];
        float env = (noteTime < 0.01f) ? (noteTime / 0.01f) : std::exp(-noteTime * 12.0f);

        // Sine with slight 2nd harmonic
        float wave = std::sin(2.0f * PI * freq * t) * 0.75f + 
                     std::sin(4.0f * PI * freq * t) * 0.25f;

        samples[i] = toSample(wave * env * 0.6f);
    }
    return samples;
}

// Tetris (4-line clear): Energetic fanfare with warm harmonic chorus
std::vector<sf::Int16> SoundManager::generateTetrisSound() {
    float duration = 0.42f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(count);

    float notes[4] = {523.25f, 659.25f, 783.99f, 1046.50f}; // C5, E5, G5, C6
    float noteDur = 0.08f;

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float wave = 0.0f;

        if (t < noteDur * 3) {
            int noteIndex = static_cast<int>(t / noteDur);
            float noteTime = t - noteIndex * noteDur;
            float freq = notes[noteIndex];
            float env = std::exp(-noteTime * 8.0f);
            wave = (std::sin(2.0f * PI * freq * t) + 0.4f * std::sin(4.0f * PI * freq * t)) * env;
        } else {
            // Sustained final high chord C6 + G5
            float noteTime = t - noteDur * 3;
            float env = std::exp(-noteTime * 4.5f);
            float c6 = std::sin(2.0f * PI * notes[3] * t);
            float g5 = std::sin(2.0f * PI * notes[2] * t) * 0.6f;
            float chorus = std::sin(2.0f * PI * (notes[3] + 2.0f) * t) * 0.4f;
            wave = (c6 + g5 + chorus) * env * 0.7f;
        }

        samples[i] = toSample(wave * 0.65f);
    }
    return samples;
}

// Game Over: Descending retro game over wah-wah
std::vector<sf::Int16> SoundManager::generateGameOverSound() {
    float duration = 0.65f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(count);

    double phase = 0.0;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        // Descend from 440 Hz down to 105 Hz with vibrato
        float baseFreq = 440.0f * std::exp(-progress * 1.5f);
        float vibrato = std::sin(2.0f * PI * 8.0f * t) * 12.0f;
        float freq = std::max(60.0f, baseFreq + vibrato);

        phase += 2.0 * PI * freq / SAMPLE_RATE;

        // Vintage square wave with decay
        float square = (std::sin(phase) > 0.0) ? 0.6f : -0.6f;
        float sine = std::sin(phase);
        float env = 1.0f - progress * 0.9f;

        samples[i] = toSample((square * 0.5f + sine * 0.5f) * env * 0.5f);
    }
    return samples;
}

// Level Up: Sparkling rising chime
std::vector<sf::Int16> SoundManager::generateLevelUpSound() {
    float duration = 0.28f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<sf::Int16> samples(count);

    float notes[4] = {440.0f, 554.37f, 659.25f, 880.0f}; // A4, C#5, E5, A5
    float noteDur = duration / 4.0f;

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        int noteIndex = std::min(3, static_cast<int>(t / noteDur));
        float noteTime = t - noteIndex * noteDur;

        float freq = notes[noteIndex];
        float env = (noteTime < 0.008f) ? (noteTime / 0.008f) : std::exp(-noteTime * 14.0f);
        float wave = std::sin(2.0f * PI * freq * t) * 0.7f + std::sin(4.0f * PI * freq * t) * 0.3f;

        samples[i] = toSample(wave * env * 0.55f);
    }
    return samples;
}
