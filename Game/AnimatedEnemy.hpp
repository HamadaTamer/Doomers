/**
 * DOOMERS - Animated Enemy with FBX Support
 * 
 * Enemies with skeletal animations and AI:
 * - Zombie: Standard enemy with walk, run, attack, death
 * - Devil: Boss enemy with special attacks
 * - State-machine AI with detection and pathing
 * - Damage reactions and death animations
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Animation.hpp"
#include "../Engine/AssimpLoader.hpp"
#include "GameAssets.hpp"
#include "DamageSystem.hpp"
#include <functional>

namespace Doomers {

// ============================================================================
// Enemy Type
// ============================================================================
enum class AnimEnemyType {
    Zombie,
    Devil,
    CrawlingZombie
};

// ============================================================================
// Enemy AI State
// ============================================================================
enum class EnemyAIState {
    Idle,           // Standing around
    Patrol,         // Walking between points
    Alert,          // Heard/saw something
    Chase,          // Running toward player
    Attack,         // Attacking player
    Stunned,        // Hit stun
    Dying,          // Playing death animation
    Dead            // Dead, can be removed
};

// ============================================================================
// Animated Enemy Class
// ============================================================================
class AnimatedEnemy {
public:
    // Identity
    int id = 0;
    AnimEnemyType type = AnimEnemyType::Zombie;
    std::string name = "Enemy";
    
    // Transform
    Math::Vector3 position;
    Math::Vector3 velocity;
    float yaw = 0.0f;
    float height = 1.8f;
    float radius = 0.5f;
    
    // Stats
    float maxHealth = 100.0f;
    float moveSpeed = 2.0f;
    float runSpeed = 4.0f;
    float attackDamage = 20.0f;
    float attackRange = 2.0f;
    float attackCooldown = 1.5f;
    float sightRange = 15.0f;
    float hearingRange = 10.0f;
    int scoreValue = 100;
    
    // AI State
    EnemyAIState aiState = EnemyAIState::Idle;
    float stateTimer = 0.0f;
    float attackTimer = 0.0f;
    bool canSeePlayer = false;
    Math::Vector3 lastKnownPlayerPos;
    float alertLevel = 0.0f;
    
    // Patrol
    std::vector<Math::Vector3> patrolPoints;
    int currentPatrolIndex = 0;
    float patrolWaitTime = 2.0f;
    
    // Target (usually the player)
    Math::Vector3* targetPos = nullptr;
    
    // Damage system
    Damageable health;
    
    // Animation
    CharacterModel* model = nullptr;
    float deathTimer = 0.0f;
    float deathAnimDuration = 2.0f;
    
    // Smoothing
    Anim::Spring rotationSmooth;
    
    // Callbacks
    std::function<void()> onDeath;
    std::function<void(float, const Math::Vector3&)> onAttack;
    std::function<void()> onAlert;
    
    // ========================================================================
    // Constructor
    // ========================================================================
    AnimatedEnemy(AnimEnemyType enemyType = AnimEnemyType::Zombie) : type(enemyType) {
        setupStats();
        setupDamageSystem();
        setupSmoothers();
    }
    
    void setupStats() {
        switch (type) {
            case AnimEnemyType::Zombie:
                name = "Zombie";
                maxHealth = 80.0f;
                moveSpeed = 2.0f;
                runSpeed = 4.0f;
                attackDamage = 15.0f;
                attackRange = 1.5f;
                attackCooldown = 1.0f;
                sightRange = 15.0f;
                scoreValue = 50;
                height = 1.8f;
                break;
                
            case AnimEnemyType::CrawlingZombie:
                name = "Crawler";
                maxHealth = 40.0f;
                moveSpeed = 3.0f;
                runSpeed = 5.0f;
                attackDamage = 10.0f;
                attackRange = 1.0f;
                attackCooldown = 0.8f;
                sightRange = 12.0f;
                scoreValue = 30;
                height = 0.5f;
                break;
                
            case AnimEnemyType::Devil:
                name = "Devil";
                maxHealth = 500.0f;
                moveSpeed = 3.0f;
                runSpeed = 6.0f;
                attackDamage = 40.0f;
                attackRange = 3.0f;
                attackCooldown = 2.0f;
                sightRange = 30.0f;
                scoreValue = 500;
                height = 2.5f;
                radius = 0.8f;
                break;
        }
        
        health.setMaxHealth(maxHealth);
    }
    
    void setupDamageSystem() {
        health.iframeDuration = 0.2f;
        health.knockbackMultiplier = 0.8f;
        health.stunDurationPerDamage = 0.02f;
        
        health.onDamageTaken = [this](const DamageInfo& info) {
            onHit(info);
        };
        
        health.onDeath = [this](const DamageInfo& info) {
            startDeath(info);
        };
    }
    
    void setupSmoothers() {
        rotationSmooth.stiffness = 100.0f;
        rotationSmooth.damping = 15.0f;
    }
    
    // ========================================================================
    // Initialize - Load model
    // ========================================================================
    bool initialize() {
        if (type == AnimEnemyType::Devil) {
            model = GameAssets::instance().loadDevilModel();
        } else {
            model = GameAssets::instance().loadZombieModel();
        }
        
        if (!model) {
            LOG_ERROR("Failed to load enemy model for: " << name);
            return false;
        }
        
        // Set initial animation
        setIdleAnimation();
        
        return true;
    }
    
    // ========================================================================
    // Update
    // ========================================================================
    void update(float dt) {
        // Update damage system
        health.update(dt);
        
        // Update timers
        stateTimer += dt;
        if (attackTimer > 0) attackTimer -= dt;
        
        // Update rotation smoothing
        rotationSmooth.update(dt);
        
        // State machine
        switch (aiState) {
            case EnemyAIState::Idle:
                updateIdle(dt);
                break;
            case EnemyAIState::Patrol:
                updatePatrol(dt);
                break;
            case EnemyAIState::Alert:
                updateAlert(dt);
                break;
            case EnemyAIState::Chase:
                updateChase(dt);
                break;
            case EnemyAIState::Attack:
                updateAttack(dt);
                break;
            case EnemyAIState::Stunned:
                updateStunned(dt);
                break;
            case EnemyAIState::Dying:
                updateDying(dt);
                break;
            case EnemyAIState::Dead:
                // Do nothing
                break;
        }
        
        // Apply knockback velocity
        position = position + health.knockbackVelocity * dt;
        health.knockbackVelocity = health.knockbackVelocity * 0.9f; // Decay
        
        // Update animation
        if (model) {
            model->update(dt);
        }
    }
    
    // ========================================================================
    // AI States
    // ========================================================================
    void updateIdle(float dt) {
        // Check for player detection
        if (checkPlayerVisible()) {
            alertLevel = 1.0f;
            changeState(EnemyAIState::Chase);
            return;
        }
        
        if (checkPlayerAudible()) {
            alertLevel = 0.5f;
            changeState(EnemyAIState::Alert);
            return;
        }
        
        // Maybe switch to patrol if we have patrol points
        if (!patrolPoints.empty() && stateTimer > 3.0f) {
            changeState(EnemyAIState::Patrol);
        }
    }
    
    void updatePatrol(float dt) {
        // Check for player
        if (checkPlayerVisible()) {
            alertLevel = 1.0f;
            changeState(EnemyAIState::Chase);
            return;
        }
        
        if (checkPlayerAudible()) {
            alertLevel = 0.5f;
            changeState(EnemyAIState::Alert);
            return;
        }
        
        // Move toward current patrol point
        if (patrolPoints.empty()) {
            changeState(EnemyAIState::Idle);
            return;
        }
        
        Math::Vector3 target = patrolPoints[currentPatrolIndex];
        Math::Vector3 toTarget = target - position;
        toTarget.y = 0; // Stay on ground
        float dist = toTarget.length();
        
        if (dist < 1.0f) {
            // Reached patrol point, wait then go to next
            if (stateTimer > patrolWaitTime) {
                currentPatrolIndex = (currentPatrolIndex + 1) % patrolPoints.size();
                stateTimer = 0;
            }
        } else {
            // Move toward patrol point
            moveToward(target, moveSpeed, dt);
        }
    }
    
    void updateAlert(float dt) {
        // Look around, investigate
        if (checkPlayerVisible()) {
            alertLevel = 1.0f;
            changeState(EnemyAIState::Chase);
            return;
        }
        
        // Move toward last known position
        Math::Vector3 toTarget = lastKnownPlayerPos - position;
        toTarget.y = 0;
        float dist = toTarget.length();
        
        if (dist < 2.0f || stateTimer > 5.0f) {
            // Lost interest
            alertLevel = 0;
            changeState(EnemyAIState::Idle);
        } else {
            moveToward(lastKnownPlayerPos, moveSpeed, dt);
        }
    }
    
    void updateChase(float dt) {
        if (!targetPos) {
            changeState(EnemyAIState::Idle);
            return;
        }
        
        lastKnownPlayerPos = *targetPos;
        
        // Check if player still visible
        if (!checkPlayerVisible()) {
            // Lost sight, switch to alert
            changeState(EnemyAIState::Alert);
            return;
        }
        
        Math::Vector3 toPlayer = *targetPos - position;
        toPlayer.y = 0;
        float dist = toPlayer.length();
        
        // If in attack range, attack!
        if (dist < attackRange) {
            changeState(EnemyAIState::Attack);
            return;
        }
        
        // Chase player
        moveToward(*targetPos, runSpeed, dt);
        setRunAnimation();
    }
    
    void updateAttack(float dt) {
        if (!targetPos) {
            changeState(EnemyAIState::Idle);
            return;
        }
        
        // Face player
        faceToward(*targetPos);
        
        // Check distance
        Math::Vector3 toPlayer = *targetPos - position;
        toPlayer.y = 0;
        float dist = toPlayer.length();
        
        if (dist > attackRange * 1.5f) {
            // Player moved away, chase
            changeState(EnemyAIState::Chase);
            return;
        }
        
        // Attack if cooldown ready
        if (attackTimer <= 0) {
            performAttack();
            attackTimer = attackCooldown;
        }
    }
    
    void updateStunned(float dt) {
        // Wait for stun to end
        if (stateTimer > 0.5f) {
            changeState(EnemyAIState::Chase);
        }
    }
    
    void updateDying(float dt) {
        deathTimer += dt;
        if (deathTimer > deathAnimDuration) {
            aiState = EnemyAIState::Dead;
        }
    }
    
    // ========================================================================
    // State Changes
    // ========================================================================
    void changeState(EnemyAIState newState) {
        if (aiState == newState) return;
        if (aiState == EnemyAIState::Dying || aiState == EnemyAIState::Dead) return;
        
        EnemyAIState oldState = aiState;
        aiState = newState;
        stateTimer = 0;
        
        // Apply appropriate animation
        switch (newState) {
            case EnemyAIState::Idle:
                setIdleAnimation();
                break;
            case EnemyAIState::Patrol:
                setWalkAnimation();
                break;
            case EnemyAIState::Alert:
                setIdleAnimation();
                if (onAlert) onAlert();
                break;
            case EnemyAIState::Chase:
                setRunAnimation();
                break;
            case EnemyAIState::Attack:
                setAttackAnimation();
                break;
            case EnemyAIState::Stunned:
                // Keep current animation or play hit reaction
                break;
            case EnemyAIState::Dying:
                setDeathAnimation();
                break;
            default:
                break;
        }
    }
    
    // ========================================================================
    // Movement Helpers
    // ========================================================================
    void moveToward(const Math::Vector3& target, float speed, float dt) {
        Math::Vector3 toTarget = target - position;
        toTarget.y = 0;
        float dist = toTarget.length();
        
        if (dist > 0.1f) {
            Math::Vector3 dir = toTarget / dist;
            position = position + dir * speed * dt;
            
            // Update facing direction
            faceToward(target);
        }
    }
    
    void faceToward(const Math::Vector3& target) {
        Math::Vector3 toTarget = target - position;
        toTarget.y = 0;
        
        if (toTarget.lengthSquared() > 0.01f) {
            float targetYaw = atan2f(toTarget.x, toTarget.z);
            rotationSmooth.target = targetYaw;
            yaw = rotationSmooth.value;
        }
    }
    
    // ========================================================================
    // Detection
    // ========================================================================
    bool checkPlayerVisible() {
        if (!targetPos) return false;
        
        Math::Vector3 toPlayer = *targetPos - position;
        float dist = toPlayer.length();
        
        if (dist > sightRange) return false;
        
        // Simple cone check
        toPlayer = toPlayer / dist;
        Math::Vector3 forward(sinf(yaw), 0, cosf(yaw));
        float dot = toPlayer.x * forward.x + toPlayer.z * forward.z;
        
        // Within ~120 degree cone
        if (dot > -0.5f) {
            canSeePlayer = true;
            lastKnownPlayerPos = *targetPos;
            return true;
        }
        
        canSeePlayer = false;
        return false;
    }
    
    bool checkPlayerAudible() {
        if (!targetPos) return false;
        
        float dist = (*targetPos - position).length();
        return dist < hearingRange;
    }
    
    // ========================================================================
    // Combat
    // ========================================================================
    void performAttack() {
        setAttackAnimation();
        
        if (onAttack && targetPos) {
            Math::Vector3 attackDir = (*targetPos - position).normalized();
            onAttack(attackDamage, attackDir);
        }
    }
    
    void onHit(const DamageInfo& info) {
        // Brief stun
        if (aiState != EnemyAIState::Dying && aiState != EnemyAIState::Dead) {
            if (info.amount > 20.0f) {
                changeState(EnemyAIState::Stunned);
            }
            
            // Alert to player if not already
            if (alertLevel < 1.0f) {
                alertLevel = 1.0f;
                if (targetPos) {
                    lastKnownPlayerPos = *targetPos;
                }
            }
        }
    }
    
    void startDeath(const DamageInfo& info) {
        changeState(EnemyAIState::Dying);
        velocity = Math::Vector3(0, 0, 0);
        
        if (onDeath) onDeath();
    }
    
    // ========================================================================
    // Animation Helpers
    // ========================================================================
    void setIdleAnimation() {
        if (!model) return;
        
        if (type == AnimEnemyType::Devil) {
            // Devil doesn't have idle, use walk
            model->setAnimation(DevilAnimations::WALK, 0.3f);
        } else {
            model->setAnimation(ZombieAnimations::IDLE, 0.3f);
        }
    }
    
    void setWalkAnimation() {
        if (!model) return;
        
        if (type == AnimEnemyType::Devil) {
            model->setAnimation(DevilAnimations::WALK, 0.2f);
        } else if (type == AnimEnemyType::CrawlingZombie) {
            model->setAnimation(ZombieAnimations::CRAWL, 0.2f);
        } else {
            model->setAnimation(ZombieAnimations::WALK, 0.2f);
        }
    }
    
    void setRunAnimation() {
        if (!model) return;
        
        if (type == AnimEnemyType::Devil) {
            model->setAnimation(DevilAnimations::WALK, 0.2f);
        } else if (type == AnimEnemyType::CrawlingZombie) {
            model->setAnimation(ZombieAnimations::RUNNING_CRAWL, 0.2f);
        } else {
            model->setAnimation(ZombieAnimations::RUN, 0.2f);
        }
    }
    
    void setAttackAnimation() {
        if (!model) return;
        
        if (type == AnimEnemyType::Devil) {
            // Randomly choose devil attack
            if (rand() % 2 == 0) {
                model->setAnimation(DevilAnimations::MELEE_KICK, 0.1f);
            } else {
                model->setAnimation(DevilAnimations::DROP_KICK, 0.1f);
            }
        } else {
            model->setAnimation(ZombieAnimations::ATTACK, 0.1f);
        }
    }
    
    void setDeathAnimation() {
        if (!model) return;
        
        if (type == AnimEnemyType::Devil) {
            // Devil uses zombie death for now
            model->setAnimation(ZombieAnimations::DEATH, 0.1f);
        } else if (rand() % 2 == 0) {
            model->setAnimation(ZombieAnimations::DEATH, 0.1f);
        } else {
            model->setAnimation(ZombieAnimations::DYING, 0.1f);
        }
    }
    
    void setScreamAnimation() {
        if (!model) return;
        if (type != AnimEnemyType::Devil) {
            model->setAnimation(ZombieAnimations::SCREAM, 0.2f);
        }
    }
    
    // ========================================================================
    // Render
    // ========================================================================
    void render() {
        if (!model || aiState == EnemyAIState::Dead) return;
        
        glPushMatrix();
        
        glTranslatef(position.x, position.y, position.z);
        glRotatef(-yaw * 57.2958f + 180.0f, 0, 1, 0);
        
        // Scale based on enemy type
        float scale = 0.01f;
        if (type == AnimEnemyType::Devil) {
            scale = 0.015f; // Devil is larger
        } else if (type == AnimEnemyType::CrawlingZombie) {
            scale = 0.008f;
        }
        glScalef(scale, scale, scale);
        
        model->draw();
        
        glPopMatrix();
    }
    
    // ========================================================================
    // Raycast for shooting
    // ========================================================================
    bool raycast(const Math::Vector3& origin, const Math::Vector3& dir, 
                 Math::Vector3& hitPoint, float maxDist) {
        // Simple sphere raycast
        Math::Vector3 center = position + Math::Vector3(0, height * 0.5f, 0);
        Math::Vector3 oc = origin - center;
        
        float a = dir.dot(dir);
        float b = 2.0f * oc.dot(dir);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) return false;
        
        float t = (-b - sqrtf(discriminant)) / (2.0f * a);
        if (t < 0 || t > maxDist) return false;
        
        hitPoint = origin + dir * t;
        return true;
    }
    
    // ========================================================================
    // Utility
    // ========================================================================
    bool isAlive() const { return aiState != EnemyAIState::Dead; }
    bool isActive() const { return aiState != EnemyAIState::Dead; }
    bool isDying() const { return aiState == EnemyAIState::Dying; }
    
    void setTarget(Math::Vector3* target) { targetPos = target; }
    
    void takeDamage(float amount, const Math::Vector3& fromDir = Math::Vector3()) {
        DamageInfo info;
        info.amount = amount;
        info.direction = fromDir;
        health.takeDamage(info);
    }
    
    float getHealth() const { return health.currentHealth; }
    Math::Vector3 getPosition() const { return position; }
};

// ============================================================================
// Factory Function
// ============================================================================
inline AnimatedEnemy* CreateZombie(const Math::Vector3& pos) {
    AnimatedEnemy* enemy = new AnimatedEnemy(AnimEnemyType::Zombie);
    enemy->position = pos;
    enemy->initialize();
    return enemy;
}

inline AnimatedEnemy* CreateCrawler(const Math::Vector3& pos) {
    AnimatedEnemy* enemy = new AnimatedEnemy(AnimEnemyType::CrawlingZombie);
    enemy->position = pos;
    enemy->initialize();
    return enemy;
}

inline AnimatedEnemy* CreateDevil(const Math::Vector3& pos) {
    AnimatedEnemy* enemy = new AnimatedEnemy(AnimEnemyType::Devil);
    enemy->position = pos;
    enemy->initialize();
    return enemy;
}

} // namespace Doomers
