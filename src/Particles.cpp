#include "Particles.hpp"
#include <cstdlib>
#include <cmath>
#include <algorithm>

ParticleSystem::ParticleSystem()
    : m_shakeIntensity(0.0f), m_shakeDuration(0.0f), m_shakeTimer(0.0f), m_shakeOffset(0.0f, 0.0f)
{
    m_particles.reserve(500);
}

void ParticleSystem::clear() {
    m_particles.clear();
    m_shakeTimer = 0.0f;
    m_shakeOffset = {0.0f, 0.0f};
}

void ParticleSystem::triggerShake(float intensity, float duration) {
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_shakeTimer = duration;
}

void ParticleSystem::update(float dt) {
    // Update screen shake
    if (m_shakeTimer > 0.0f) {
        m_shakeTimer -= dt;
        float progress = m_shakeTimer / m_shakeDuration;
        float currentIntensity = m_shakeIntensity * progress;
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
        m_shakeOffset.x = std::cos(angle) * currentIntensity;
        m_shakeOffset.y = std::sin(angle) * currentIntensity;
    } else {
        m_shakeOffset = {0.0f, 0.0f};
    }

    // Update particles
    for (size_t i = 0; i < m_particles.size();) {
        auto& p = m_particles[i];
        p.life -= dt;
        if (p.life <= 0.0f) {
            // Fast swap-and-pop removal
            p = m_particles.back();
            m_particles.pop_back();
            continue;
        }

        // Apply physics
        p.position += p.velocity * dt;
        p.velocity.y += 260.0f * dt; // Gravity
        p.velocity.x *= (1.0f - 0.4f * dt); // Air resistance
        ++i;
    }
}

void ParticleSystem::draw(sf::RenderTarget& target) const {
    if (m_particles.empty()) return;

    for (const auto& p : m_particles) {
        float alphaRatio = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);
        float currentSize = p.size * (0.3f + 0.7f * alphaRatio);
        sf::Uint8 alpha = static_cast<sf::Uint8>(255 * alphaRatio);

        sf::Color drawColor = p.color;
        drawColor.a = alpha;

        // Draw glowing particle as a rotated square or diamond
        sf::RectangleShape shape(sf::Vector2f(currentSize, currentSize));
        shape.setOrigin(currentSize * 0.5f, currentSize * 0.5f);
        shape.setPosition(p.position);
        shape.setRotation(alphaRatio * 180.0f);
        shape.setFillColor(drawColor);

        target.draw(shape);
    }
}

void ParticleSystem::emitLineClear(const std::vector<int>& rows, float boardX, float boardY, 
                                   float boardWidth, float blockSize, int lineCount) 
{
    // Neon explosion colors
    static const sf::Color PALETTE[] = {
        sf::Color(0, 240, 255),   // Cyan
        sf::Color(255, 215, 0),   // Gold
        sf::Color(175, 60, 255),  // Violet
        sf::Color(50, 240, 80),   // Green
        sf::Color(255, 45, 75),   // Red
        sf::Color(255, 255, 255)  // Electric White
    };
    constexpr int PALETTE_SIZE = sizeof(PALETTE) / sizeof(PALETTE[0]);

    int particlesPerRow = (lineCount >= 4) ? 45 : 22;

    for (int row : rows) {
        float rowCenterY = boardY + (row + 0.5f) * blockSize;

        for (int i = 0; i < particlesPerRow; ++i) {
            Particle p;
            float spawnX = boardX + (static_cast<float>(rand()) / RAND_MAX) * boardWidth;
            float spawnY = rowCenterY + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * blockSize;
            p.position = {spawnX, spawnY};

            float speed = 120.0f + (static_cast<float>(rand()) / RAND_MAX) * ((lineCount >= 4) ? 360.0f : 220.0f);
            float angle = (static_cast<float>(rand()) / RAND_MAX) * 2.0f * 3.14159f;
            p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed - 60.0f};

            p.color = PALETTE[rand() % PALETTE_SIZE];
            p.size = 4.0f + (static_cast<float>(rand()) / RAND_MAX) * ((lineCount >= 4) ? 5.0f : 3.5f);
            p.maxLife = 0.45f + (static_cast<float>(rand()) / RAND_MAX) * 0.4f;
            p.life = p.maxLife;

            m_particles.push_back(p);
        }
    }

    // Trigger screen shake for clears
    if (lineCount >= 4) {
        triggerShake(7.5f, 0.28f); // Heavy punch for Tetris!
    } else {
        triggerShake(2.5f * lineCount, 0.16f);
    }
}

void ParticleSystem::emitHardDrop(float startX, float y, float width, sf::Color color) {
    int count = 16;
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = {startX + (static_cast<float>(rand()) / RAND_MAX) * width, y - 2.0f};
        
        float vx = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 160.0f;
        float vy = -50.0f - (static_cast<float>(rand()) / RAND_MAX) * 110.0f;
        p.velocity = {vx, vy};

        p.color = color;
        p.size = 3.0f + (static_cast<float>(rand()) / RAND_MAX) * 2.5f;
        p.maxLife = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.15f;
        p.life = p.maxLife;

        m_particles.push_back(p);
    }
}
