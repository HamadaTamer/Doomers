/**
 * DOOMERS - Particle System
 * 
 * Professional particle effects:
 * - Blood splatter
 * - Muzzle flash particles
 * - Bullet impacts
 * - Smoke & dust
 * - Explosion particles
 * - Environmental effects
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include <vector>
#include <GL/gl.h>

namespace Doomers {

// ============================================================================
// Particle Types
// ============================================================================
enum class ParticleType {
    Generic,
    Blood,
    Spark,
    Smoke,
    Dust,
    Fire,
    Debris,
    MuzzleFlash,
    BulletTrail
};

// ============================================================================
// Single Particle
// ============================================================================
struct Particle {
    Math::Vector3 position;
    Math::Vector3 velocity;
    Math::Color color;
    float size = 0.1f;
    float life = 1.0f;
    float maxLife = 1.0f;
    float gravity = 1.0f;
    float drag = 0.0f;
    float rotation = 0.0f;
    float rotationSpeed = 0.0f;
    ParticleType type = ParticleType::Generic;
    bool active = false;
    
    void update(float dt) {
        if (!active) return;
        
        life -= dt;
        if (life <= 0) {
            active = false;
            return;
        }
        
        // Apply physics
        velocity.y -= gravity * 9.81f * dt;
        velocity = velocity * (1.0f - drag * dt);
        position = position + velocity * dt;
        rotation += rotationSpeed * dt;
        
        // Fade color
        float lifeRatio = life / maxLife;
        color.a = lifeRatio;
    }
    
    float getLifeRatio() const {
        return maxLife > 0 ? life / maxLife : 0;
    }
};

// ============================================================================
// Particle Emitter Configuration
// ============================================================================
struct ParticleEmitterConfig {
    // Spawn settings
    int maxParticles = 100;
    float spawnRate = 10.0f;  // Particles per second
    int burstCount = 0;        // For one-shot bursts
    
    // Lifetime
    float minLife = 0.5f;
    float maxLife = 2.0f;
    
    // Position variance
    Math::Vector3 positionVariance{0.1f, 0.1f, 0.1f};
    
    // Velocity
    Math::Vector3 minVelocity{-1, 0, -1};
    Math::Vector3 maxVelocity{1, 3, 1};
    
    // Appearance
    float minSize = 0.05f;
    float maxSize = 0.2f;
    Math::Color startColor{1, 1, 1, 1};
    Math::Color endColor{1, 1, 1, 0};
    
    // Physics
    float gravity = 1.0f;
    float drag = 0.5f;
    
    // Rotation
    float minRotationSpeed = 0;
    float maxRotationSpeed = 0;
    
    ParticleType type = ParticleType::Generic;
};

// ============================================================================
// Particle Emitter
// ============================================================================
class ParticleEmitter {
public:
    std::vector<Particle> particles;
    ParticleEmitterConfig config;
    Math::Vector3 position;
    Math::Vector3 direction{0, 1, 0};
    bool active = true;
    float spawnTimer = 0;
    
    ParticleEmitter() = default;
    
    explicit ParticleEmitter(const ParticleEmitterConfig& cfg)
        : config(cfg) {
        particles.resize(config.maxParticles);
    }
    
    void init(const ParticleEmitterConfig& cfg) {
        config = cfg;
        particles.resize(config.maxParticles);
    }
    
    void setPosition(const Math::Vector3& pos) {
        position = pos;
    }
    
    void setDirection(const Math::Vector3& dir) {
        direction = dir.normalized();
    }
    
    void update(float dt) {
        // Update existing particles
        for (auto& p : particles) {
            p.update(dt);
        }
        
        // Spawn new particles
        if (active && config.spawnRate > 0) {
            spawnTimer += dt;
            float spawnInterval = 1.0f / config.spawnRate;
            
            while (spawnTimer >= spawnInterval) {
                spawnTimer -= spawnInterval;
                spawnParticle();
            }
        }
    }
    
    void burst(int count) {
        for (int i = 0; i < count; ++i) {
            spawnParticle();
        }
    }
    
    void burst() {
        burst(config.burstCount > 0 ? config.burstCount : 10);
    }
    
    void spawnParticle() {
        // Find inactive particle
        for (auto& p : particles) {
            if (!p.active) {
                initParticle(p);
                return;
            }
        }
    }
    
    void clear() {
        for (auto& p : particles) {
            p.active = false;
        }
    }
    
    void draw() {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);
        
        for (const auto& p : particles) {
            if (!p.active) continue;
            
            glPushMatrix();
            glTranslatef(p.position.x, p.position.y, p.position.z);
            
            // Billboard - face camera
            float modelview[16];
            glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
            
            // Zero out rotation
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (i == j) {
                        modelview[i * 4 + j] = 1.0f;
                    } else {
                        modelview[i * 4 + j] = 0.0f;
                    }
                }
            }
            glLoadMatrixf(modelview);
            
            // Apply rotation
            glRotatef(p.rotation, 0, 0, 1);
            
            // Draw quad
            float s = p.size;
            glColor4f(p.color.r, p.color.g, p.color.b, p.color.a);
            
            glBegin(GL_QUADS);
            glVertex3f(-s, -s, 0);
            glVertex3f(s, -s, 0);
            glVertex3f(s, s, 0);
            glVertex3f(-s, s, 0);
            glEnd();
            
            glPopMatrix();
        }
        
        glPopAttrib();
    }
    
    int getActiveCount() const {
        int count = 0;
        for (const auto& p : particles) {
            if (p.active) count++;
        }
        return count;
    }
    
private:
    void initParticle(Particle& p) {
        p.active = true;
        p.type = config.type;
        
        // Random position variance
        p.position = position + Math::Vector3(
            randomRange(-config.positionVariance.x, config.positionVariance.x),
            randomRange(-config.positionVariance.y, config.positionVariance.y),
            randomRange(-config.positionVariance.z, config.positionVariance.z)
        );
        
        // Random velocity
        p.velocity = Math::Vector3(
            randomRange(config.minVelocity.x, config.maxVelocity.x),
            randomRange(config.minVelocity.y, config.maxVelocity.y),
            randomRange(config.minVelocity.z, config.maxVelocity.z)
        );
        
        // Add directional bias
        p.velocity = p.velocity + direction * 2.0f;
        
        // Random properties
        p.life = randomRange(config.minLife, config.maxLife);
        p.maxLife = p.life;
        p.size = randomRange(config.minSize, config.maxSize);
        p.gravity = config.gravity;
        p.drag = config.drag;
        p.color = config.startColor;
        p.rotation = randomRange(0.0f, 360.0f);
        p.rotationSpeed = randomRange(config.minRotationSpeed, config.maxRotationSpeed);
    }
    
    float randomRange(float min, float max) {
        return min + (float)(rand() % 1000) / 1000.0f * (max - min);
    }
};

// ============================================================================
// Preset Configurations
// ============================================================================
namespace ParticlePresets {
    
inline ParticleEmitterConfig blood() {
    ParticleEmitterConfig cfg;
    cfg.maxParticles = 50;
    cfg.burstCount = 20;
    cfg.spawnRate = 0;  // Burst only
    cfg.minLife = 0.5f;
    cfg.maxLife = 1.5f;
    cfg.minVelocity = {-3, 1, -3};
    cfg.maxVelocity = {3, 5, 3};
    cfg.minSize = 0.02f;
    cfg.maxSize = 0.08f;
    cfg.startColor = {0.8f, 0.0f, 0.0f, 1.0f};
    cfg.gravity = 2.0f;
    cfg.drag = 0.5f;
    cfg.type = ParticleType::Blood;
    return cfg;
}

inline ParticleEmitterConfig spark() {
    ParticleEmitterConfig cfg;
    cfg.maxParticles = 30;
    cfg.burstCount = 15;
    cfg.spawnRate = 0;
    cfg.minLife = 0.2f;
    cfg.maxLife = 0.6f;
    cfg.minVelocity = {-5, 0, -5};
    cfg.maxVelocity = {5, 3, 5};
    cfg.minSize = 0.01f;
    cfg.maxSize = 0.03f;
    cfg.startColor = {1.0f, 0.8f, 0.3f, 1.0f};
    cfg.gravity = 0.5f;
    cfg.drag = 2.0f;
    cfg.type = ParticleType::Spark;
    return cfg;
}

inline ParticleEmitterConfig smoke() {
    ParticleEmitterConfig cfg;
    cfg.maxParticles = 100;
    cfg.spawnRate = 10.0f;
    cfg.minLife = 1.0f;
    cfg.maxLife = 3.0f;
    cfg.minVelocity = {-0.5f, 0.5f, -0.5f};
    cfg.maxVelocity = {0.5f, 2.0f, 0.5f};
    cfg.minSize = 0.1f;
    cfg.maxSize = 0.4f;
    cfg.startColor = {0.5f, 0.5f, 0.5f, 0.5f};
    cfg.gravity = -0.2f;  // Rise
    cfg.drag = 1.0f;
    cfg.type = ParticleType::Smoke;
    return cfg;
}

inline ParticleEmitterConfig dust() {
    ParticleEmitterConfig cfg;
    cfg.maxParticles = 50;
    cfg.burstCount = 20;
    cfg.spawnRate = 0;
    cfg.minLife = 0.5f;
    cfg.maxLife = 2.0f;
    cfg.positionVariance = {0.5f, 0.1f, 0.5f};
    cfg.minVelocity = {-1, 0, -1};
    cfg.maxVelocity = {1, 2, 1};
    cfg.minSize = 0.05f;
    cfg.maxSize = 0.15f;
    cfg.startColor = {0.6f, 0.5f, 0.4f, 0.6f};
    cfg.gravity = 0.3f;
    cfg.drag = 2.0f;
    cfg.type = ParticleType::Dust;
    return cfg;
}

inline ParticleEmitterConfig muzzleFlash() {
    ParticleEmitterConfig cfg;
    cfg.maxParticles = 20;
    cfg.burstCount = 5;
    cfg.spawnRate = 0;
    cfg.minLife = 0.05f;
    cfg.maxLife = 0.15f;
    cfg.minVelocity = {-1, -1, -3};
    cfg.maxVelocity = {1, 1, -8};
    cfg.minSize = 0.01f;
    cfg.maxSize = 0.03f;
    cfg.startColor = {1.0f, 0.9f, 0.5f, 1.0f};
    cfg.gravity = 0;
    cfg.drag = 5.0f;
    cfg.type = ParticleType::MuzzleFlash;
    return cfg;
}

inline ParticleEmitterConfig explosion() {
    ParticleEmitterConfig cfg;
    cfg.maxParticles = 200;
    cfg.burstCount = 100;
    cfg.spawnRate = 0;
    cfg.minLife = 0.3f;
    cfg.maxLife = 1.5f;
    cfg.minVelocity = {-8, 0, -8};
    cfg.maxVelocity = {8, 10, 8};
    cfg.minSize = 0.05f;
    cfg.maxSize = 0.3f;
    cfg.startColor = {1.0f, 0.6f, 0.2f, 1.0f};
    cfg.gravity = 1.0f;
    cfg.drag = 1.0f;
    cfg.type = ParticleType::Fire;
    return cfg;
}

} // namespace ParticlePresets

// ============================================================================
// Particle Manager
// ============================================================================
class ParticleManager {
public:
    std::vector<ParticleEmitter> emitters;
    
    ParticleManager() {
        emitters.reserve(50);
    }
    
    void update(float dt) {
        for (auto& e : emitters) {
            e.update(dt);
        }
        
        // Remove inactive burst emitters
        emitters.erase(
            std::remove_if(emitters.begin(), emitters.end(),
                [](const ParticleEmitter& e) {
                    return !e.active && e.getActiveCount() == 0;
                }),
            emitters.end()
        );
    }
    
    void draw() {
        for (auto& e : emitters) {
            e.draw();
        }
    }
    
    void spawnBurst(const Math::Vector3& position, const ParticleEmitterConfig& config) {
        ParticleEmitter emitter(config);
        emitter.position = position;
        emitter.active = false;  // One-shot burst
        emitter.burst();
        emitters.push_back(emitter);
    }
    
    void spawnBlood(const Math::Vector3& position, const Math::Vector3& direction) {
        auto config = ParticlePresets::blood();
        ParticleEmitter emitter(config);
        emitter.position = position;
        emitter.direction = direction;
        emitter.active = false;
        emitter.burst();
        emitters.push_back(emitter);
    }
    
    void spawnSparks(const Math::Vector3& position) {
        spawnBurst(position, ParticlePresets::spark());
    }
    
    void spawnDust(const Math::Vector3& position) {
        spawnBurst(position, ParticlePresets::dust());
    }
    
    void spawnExplosion(const Math::Vector3& position) {
        spawnBurst(position, ParticlePresets::explosion());
        spawnBurst(position, ParticlePresets::smoke());
    }
    
    ParticleEmitter& addEmitter(const ParticleEmitterConfig& config) {
        emitters.emplace_back(config);
        return emitters.back();
    }
    
    void clear() {
        emitters.clear();
    }
    
    int getTotalParticles() const {
        int total = 0;
        for (const auto& e : emitters) {
            total += e.getActiveCount();
        }
        return total;
    }
};

// Type alias for backwards compatibility
using ParticleSystem = ParticleManager;

} // namespace Doomers
