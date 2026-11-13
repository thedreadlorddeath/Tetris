#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float size;
    float life;
    float maxLife;
};

class ParticleSystem {
public:
    ParticleSystem();

    void update(float dt);
    void draw(sf::RenderTarget& target) const;

    void clear();

    // Spawns energetic neon bursts for line clears
    void emitLineClear(const std::vector<int>& rows, float boardX, float boardY, 
                       float boardWidth, float blockSize, int lineCount);

    // Spawns impact dust sparks on hard drop
    void emitHardDrop(float startX, float y, float width, sf::Color color);

    // Screen Shake management
    void triggerShake(float intensity, float duration);
    sf::Vector2f getShakeOffset() const { return m_shakeOffset; }

private:
    std::vector<Particle> m_particles;
    
    // Screen shake state
    float m_shakeIntensity;
    float m_shakeDuration;
    float m_shakeTimer;
    sf::Vector2f m_shakeOffset;
};
