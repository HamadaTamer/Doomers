/**
 * DOOMERS - FPS Game Core
 * Main game class that ties everything together
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/InputManager.hpp"
#include "../Engine/AssimpLoader.hpp"
#include "../Engine/ParticleSystem.hpp"
#include <vector>
#include <memory>

namespace Doomers {

// ============================================================================
// Game Configuration
// ============================================================================
struct GameConfig {
    // Paths to FBX assets
    static constexpr const char* LEVEL1_FBX = "assets/sci-fi-interior-pack-lowpoly/source/_CombinedAssets_DisplayPack.fbx";
    static constexpr const char* LEVEL2_FBX = "assets/free-lava-zone-environment/source/TerrainGEN_3Model.fbx";
    static constexpr const char* WEAPON_FBX = "assets/sci-fi-laser-rifle/source/toSketchfab.fbx";
    static constexpr const char* ZOMBIE_FBX = "assets/scary-zombie-pack/zombie idle.fbx";
    static constexpr const char* DEVIL_FBX = "assets/devil/devil.fbx";
    
    // Player settings
    static constexpr float PLAYER_WALK_SPEED = 5.0f;
    static constexpr float PLAYER_SPRINT_SPEED = 9.0f;
    static constexpr float PLAYER_JUMP_FORCE = 8.0f;
    static constexpr float MOUSE_SENSITIVITY = 0.15f;
    
    // Weapon settings
    static constexpr float FIRE_RATE = 0.12f;
    static constexpr int WEAPON_DAMAGE = 25;
    static constexpr int START_AMMO = 50;
    static constexpr int MAX_AMMO = 150;
};

// ============================================================================
// FPS Camera
// ============================================================================
struct FPSCamera {
    Math::Vector3 position{0, 1.7f, 5};
    float yaw = -90.0f;
    float pitch = 0.0f;
    Math::Vector3 forward{0, 0, -1};
    Math::Vector3 right{1, 0, 0};
    Math::Vector3 up{0, 1, 0};
    
    void updateVectors() {
        float yawRad = yaw * 0.01745329f;
        float pitchRad = pitch * 0.01745329f;
        forward.x = cosf(pitchRad) * cosf(yawRad);
        forward.y = sinf(pitchRad);
        forward.z = cosf(pitchRad) * sinf(yawRad);
        forward = forward.normalized();
        right = forward.cross(Math::Vector3(0, 1, 0)).normalized();
        up = right.cross(forward).normalized();
    }
    
    void rotate(float dx, float dy, float sens = 0.15f) {
        yaw += dx * sens;
        pitch += dy * sens;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        updateVectors();
    }
    
    Math::Vector3 getForwardXZ() const {
        return Math::Vector3(forward.x, 0, forward.z).normalized();
    }
    
    Math::Vector3 getRightXZ() const {
        return Math::Vector3(right.x, 0, right.z).normalized();
    }
    
    void applyView() {
        Math::Vector3 target = position + forward;
        gluLookAt(position.x, position.y, position.z,
                  target.x, target.y, target.z,
                  up.x, up.y, up.z);
    }
};

// ============================================================================
// Game Enemy
// ============================================================================
struct GameEnemy {
    enum class Type { Zombie, Devil };
    enum class State { Idle, Walking, Attacking, Dying, Dead };
    
    Type type = Type::Zombie;
    State state = State::Idle;
    Math::Vector3 position;
    float yaw = 0;
    float health = 100.0f;
    float maxHealth = 100.0f;
    float speed = 2.5f;
    float attackRange = 2.0f;
    float attackCooldown = 0;
    float damage = 15.0f;
    float deathTimer = 0;
    AnimatedModel* model = nullptr;
    
    GameEnemy(Type t, const Math::Vector3& pos) : type(t), position(pos) {
        if (t == Type::Devil) {
            health = maxHealth = 200.0f;
            speed = 3.5f;
            damage = 25.0f;
        }
    }
    
    bool isAlive() const { return state != State::Dead; }
    bool canAttack() const { return attackCooldown <= 0 && state == State::Attacking; }
    
    void update(float dt, const Math::Vector3& playerPos) {
        if (state == State::Dead) return;
        
        if (state == State::Dying) {
            deathTimer += dt;
            if (deathTimer > 2.0f) state = State::Dead;
            return;
        }
        
        attackCooldown -= dt;
        
        Math::Vector3 toPlayer = playerPos - position;
        toPlayer.y = 0;
        float dist = toPlayer.length();
        
        if (dist > 0.1f) {
            yaw = atan2f(toPlayer.x, toPlayer.z) * 57.2958f;
            
            if (dist <= attackRange) {
                state = State::Attacking;
            } else {
                state = State::Walking;
                Math::Vector3 dir = toPlayer.normalized();
                position = position + dir * speed * dt;
            }
        }
        
        if (model) {
            model->update(dt);
        }
    }
    
    void takeDamage(float dmg) {
        health -= dmg;
        if (health <= 0) {
            health = 0;
            state = State::Dying;
            deathTimer = 0;
        }
    }
    
    void draw() {
        if (state == State::Dead) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(yaw, 0, 1, 0);
        
        if (model) {
            glScalef(0.01f, 0.01f, 0.01f); // FBX scale
            model->draw();
        } else {
            // Fallback: colored cube
            if (type == Type::Zombie) {
                glColor3f(0.3f, 0.6f, 0.3f);
            } else {
                glColor3f(0.8f, 0.2f, 0.2f);
            }
            glTranslatef(0, 1, 0);
            glutSolidCube(1.8f);
            
            // Health bar
            glDisable(GL_LIGHTING);
            glTranslatef(0, 1.5f, 0);
            float hp = health / maxHealth;
            glColor3f(1 - hp, hp, 0);
            glScalef(hp * 1.5f, 0.1f, 0.1f);
            glutSolidCube(1.0f);
            glEnable(GL_LIGHTING);
        }
        
        glPopMatrix();
    }
};

// ============================================================================
// Game Player
// ============================================================================
struct GamePlayer {
    Math::Vector3 position{0, 0, 5};
    Math::Vector3 velocity{0, 0, 0};
    FPSCamera camera;
    
    float health = 100.0f;
    float maxHealth = 100.0f;
    float armor = 0.0f;
    int ammo = GameConfig::START_AMMO;
    int maxAmmo = GameConfig::MAX_AMMO;
    int score = 0;
    int kills = 0;
    
    bool grounded = true;
    bool sprinting = false;
    float fireTimer = 0;
    
    GamePlayer() { camera.updateVectors(); }
    
    void update(float dt) {
        auto& inp = InputManager::instance();
        
        Math::Vector3 moveDir{0, 0, 0};
        if (inp.isKeyDown('w')) moveDir = moveDir + camera.getForwardXZ();
        if (inp.isKeyDown('s')) moveDir = moveDir - camera.getForwardXZ();
        if (inp.isKeyDown('d')) moveDir = moveDir + camera.getRightXZ();
        if (inp.isKeyDown('a')) moveDir = moveDir - camera.getRightXZ();
        
        if (moveDir.lengthSquared() > 0.01f) moveDir = moveDir.normalized();
        
        sprinting = inp.isShiftDown();
        float speed = sprinting ? GameConfig::PLAYER_SPRINT_SPEED : GameConfig::PLAYER_WALK_SPEED;
        
        velocity.x = moveDir.x * speed;
        velocity.z = moveDir.z * speed;
        
        if (inp.isKeyDown(' ') && grounded) {
            velocity.y = GameConfig::PLAYER_JUMP_FORCE;
            grounded = false;
        }
        
        if (!grounded) velocity.y -= 20.0f * dt;
        
        position = position + velocity * dt;
        
        if (position.y <= 0) {
            position.y = 0;
            velocity.y = 0;
            grounded = true;
        }
        
        camera.position = position + Math::Vector3(0, 1.7f, 0);
        if (fireTimer > 0) fireTimer -= dt;
    }
    
    bool canFire() const { return fireTimer <= 0 && ammo > 0; }
    
    void fire() {
        if (canFire()) {
            fireTimer = GameConfig::FIRE_RATE;
            ammo--;
        }
    }
    
    void takeDamage(float dmg) {
        float absorbed = std::min(armor, dmg * 0.5f);
        armor -= absorbed;
        health -= (dmg - absorbed);
        if (health < 0) health = 0;
    }
    
    bool isAlive() const { return health > 0; }
    
    void addHealth(float h) { health = std::min(health + h, maxHealth); }
    void addArmor(float a) { armor = std::min(armor + a, 100.0f); }
    void addAmmo(int a) { ammo = std::min(ammo + a, maxAmmo); }
};

// ============================================================================
// Pickup Item
// ============================================================================
struct GamePickup {
    enum class Type { Health, Ammo, Armor };
    Type type;
    Math::Vector3 position;
    int value;
    bool active = true;
    float bobTimer = 0;
    float spinAngle = 0;
    
    GamePickup(Type t, const Math::Vector3& pos, int val) 
        : type(t), position(pos), value(val) {}
    
    void update(float dt) {
        bobTimer += dt * 2.0f;
        spinAngle += dt * 90.0f;
    }
    
    void draw() {
        if (!active) return;
        float bob = sinf(bobTimer) * 0.15f;
        
        glPushMatrix();
        glTranslatef(position.x, position.y + 0.5f + bob, position.z);
        glRotatef(spinAngle, 0, 1, 0);
        
        switch (type) {
            case Type::Health: glColor3f(0.2f, 0.9f, 0.2f); break;
            case Type::Ammo:   glColor3f(0.9f, 0.7f, 0.1f); break;
            case Type::Armor:  glColor3f(0.2f, 0.5f, 0.9f); break;
        }
        
        glutSolidCube(0.5f);
        glPopMatrix();
    }
    
    bool checkCollision(const Math::Vector3& playerPos) {
        if (!active) return false;
        float dist = (position - playerPos).length();
        return dist < 1.5f;
    }
};

} // namespace Doomers
