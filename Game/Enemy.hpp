/**
 * DOOMERS - Enemy
 * 
 * Professional enemy AI with:
 * - State machine AI
 * - Damage system integration
 * - Death animations
 * - Pathfinding basics
 * - Attack patterns
 * - Hit reactions
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Animation.hpp"
#include "DamageSystem.hpp"
#include <functional>
#include <vector>

namespace Doomers {

// ============================================================================
// Enemy Type
// ============================================================================
enum class EnemyType {
    Zombie,
    Demon,
    Soldier,
    Heavy,
    Boss
};

// ============================================================================
// Enemy AI State
// ============================================================================
enum class EnemyState {
    Idle,
    Patrol,
    Alert,
    Chase,
    Attack,
    Stunned,
    Dying,
    Dead
};

// ============================================================================
// Enemy Stats
// ============================================================================
struct EnemyStats {
    float maxHealth = 100.0f;
    float moveSpeed = 3.0f;
    float runSpeed = 5.0f;
    float attackDamage = 20.0f;
    float attackRange = 2.0f;
    float attackCooldown = 1.5f;
    float sightRange = 20.0f;
    float sightAngle = 60.0f; // Degrees
    float hearingRange = 10.0f;
    int scoreValue = 100;
    float knockbackResistance = 0.0f; // 0-1
};

// ============================================================================
// Enhanced Enemy Class
// ============================================================================
class EnhancedEnemy {
public:
    // Identity
    int enemyId = 0;
    EnemyType type = EnemyType::Zombie;
    std::string name = "Enemy";
    
    // Transform
    Math::Vector3 position;
    Math::Vector3 velocity;
    float yaw = 0;
    float height = 1.8f;
    float radius = 0.4f;
    
    // Stats
    EnemyStats stats;
    
    // State
    EnemyState state = EnemyState::Idle;
    EnemyState previousState = EnemyState::Idle;
    float stateTimer = 0;
    
    // Damage system
    Damageable damageable;
    
    // AI
    Math::Vector3 targetPosition;
    Math::Vector3 lastKnownPlayerPos;
    bool canSeePlayer = false;
    bool heardSound = false;
    float alertLevel = 0; // 0-1
    float searchTimer = 0;
    
    // Patrol
    std::vector<Math::Vector3> patrolPoints;
    int currentPatrolIndex = 0;
    float patrolWaitTimer = 0;
    float patrolWaitTime = 2.0f;
    
    // Attack
    float attackTimer = 0;
    bool isAttacking = false;
    Anim::Tween<float> attackAnimation;
    
    // Movement interpolation
    Anim::Spring3D positionSpring;
    Anim::Spring rotationSpring;
    
    // Animation state
    float animTimer = 0;
    float hitReactionTimer = 0;
    Math::Vector3 hitDirection;
    
    // Death
    float deathTimer = 0;
    Math::Vector3 deathDirection;
    bool ragdollActive = false;
    
    // Callbacks
    std::function<void()> onDeath;
    std::function<void()> onAlert;
    std::function<void(float, const Math::Vector3&)> onAttack;
    std::function<void(const Math::Vector3&)> onFootstep;
    
    EnhancedEnemy() {
        setupDamageSystem();
        setupSprings();
        setupDefaultStats();
    }
    
    EnhancedEnemy(EnemyType enemyType) : type(enemyType) {
        setupDamageSystem();
        setupSprings();
        setupDefaultStats();
    }
    
    void setupDamageSystem() {
        damageable.setMaxHealth(stats.maxHealth);
        damageable.knockbackMultiplier = 1.0f - stats.knockbackResistance;
        damageable.iframeDuration = 0.2f;
        damageable.stunDurationPerDamage = 0.02f;
        
        damageable.onDamageTaken = [this](const DamageInfo& info) {
            onHit(info);
        };
        
        damageable.onDeath = [this](const DamageInfo& info) {
            die(info);
        };
    }
    
    void setupSprings() {
        positionSpring.stiffness = 50.0f;
        positionSpring.damping = 10.0f;
        
        rotationSpring.stiffness = 100.0f;
        rotationSpring.damping = 15.0f;
    }
    
    void setupDefaultStats() {
        switch (type) {
            case EnemyType::Zombie:
                stats.maxHealth = 80.0f;
                stats.moveSpeed = 2.0f;
                stats.runSpeed = 4.0f;
                stats.attackDamage = 15.0f;
                stats.attackRange = 1.5f;
                stats.attackCooldown = 1.0f;
                stats.sightRange = 15.0f;
                stats.scoreValue = 50;
                name = "Zombie";
                break;
                
            case EnemyType::Soldier:
                stats.maxHealth = 100.0f;
                stats.moveSpeed = 3.5f;
                stats.runSpeed = 6.0f;
                stats.attackDamage = 25.0f;
                stats.attackRange = 20.0f; // Ranged
                stats.attackCooldown = 0.5f;
                stats.sightRange = 30.0f;
                stats.scoreValue = 100;
                name = "Soldier";
                break;
                
            case EnemyType::Heavy:
                stats.maxHealth = 300.0f;
                stats.moveSpeed = 2.0f;
                stats.runSpeed = 3.0f;
                stats.attackDamage = 40.0f;
                stats.attackRange = 2.5f;
                stats.attackCooldown = 2.0f;
                stats.knockbackResistance = 0.7f;
                stats.scoreValue = 250;
                name = "Heavy";
                break;
                
            case EnemyType::Boss:
                stats.maxHealth = 1000.0f;
                stats.moveSpeed = 2.5f;
                stats.runSpeed = 4.0f;
                stats.attackDamage = 50.0f;
                stats.attackRange = 3.0f;
                stats.attackCooldown = 1.5f;
                stats.knockbackResistance = 0.9f;
                stats.scoreValue = 1000;
                name = "Boss";
                break;
        }
        
        damageable.setMaxHealth(stats.maxHealth);
        damageable.knockbackMultiplier = 1.0f - stats.knockbackResistance;
    }
    
    void update(float deltaTime, const Math::Vector3& playerPos) {
        // Get time scale from hit stop
        float timeScale = damageable.hitStop.getTimeScale();
        float scaledDelta = deltaTime * timeScale;
        
        // Update damage system
        damageable.update(scaledDelta);
        
        // State machine
        stateTimer += scaledDelta;
        
        switch (state) {
            case EnemyState::Idle:
                updateIdle(scaledDelta, playerPos);
                break;
            case EnemyState::Patrol:
                updatePatrol(scaledDelta, playerPos);
                break;
            case EnemyState::Alert:
                updateAlert(scaledDelta, playerPos);
                break;
            case EnemyState::Chase:
                updateChase(scaledDelta, playerPos);
                break;
            case EnemyState::Attack:
                updateAttack(scaledDelta, playerPos);
                break;
            case EnemyState::Stunned:
                updateStunned(scaledDelta);
                break;
            case EnemyState::Dying:
                updateDying(scaledDelta);
                break;
            case EnemyState::Dead:
                // Do nothing
                break;
        }
        
        // Update movement
        updateMovement(scaledDelta);
        
        // Update springs
        positionSpring.update(scaledDelta);
        rotationSpring.update(scaledDelta);
        
        // Update hit reaction
        if (hitReactionTimer > 0) {
            hitReactionTimer -= scaledDelta;
        }
    }
    
    void updateIdle(float deltaTime, const Math::Vector3& playerPos) {
        // Check for player
        if (checkPlayerVisibility(playerPos)) {
            transitionTo(EnemyState::Alert);
            return;
        }
        
        // Random idle behavior
        if (stateTimer > 3.0f && !patrolPoints.empty()) {
            transitionTo(EnemyState::Patrol);
        }
    }
    
    void updatePatrol(float deltaTime, const Math::Vector3& playerPos) {
        // Check for player
        if (checkPlayerVisibility(playerPos)) {
            transitionTo(EnemyState::Alert);
            return;
        }
        
        if (patrolPoints.empty()) {
            transitionTo(EnemyState::Idle);
            return;
        }
        
        // Move to patrol point
        Math::Vector3 target = patrolPoints[currentPatrolIndex];
        float dist = (target - position).length();
        
        if (dist < 0.5f) {
            // Reached patrol point
            patrolWaitTimer += deltaTime;
            if (patrolWaitTimer >= patrolWaitTime) {
                patrolWaitTimer = 0;
                currentPatrolIndex = (currentPatrolIndex + 1) % patrolPoints.size();
            }
        } else {
            moveToward(target, stats.moveSpeed, deltaTime);
        }
    }
    
    void updateAlert(float deltaTime, const Math::Vector3& playerPos) {
        alertLevel += deltaTime * 2.0f; // Build alert
        
        if (alertLevel >= 1.0f) {
            if (onAlert) onAlert();
            transitionTo(EnemyState::Chase);
            return;
        }
        
        // Look toward last known position
        lookAt(lastKnownPlayerPos);
        
        // Can still lose player
        if (!checkPlayerVisibility(playerPos)) {
            alertLevel -= deltaTime * 0.5f;
            if (alertLevel <= 0) {
                transitionTo(EnemyState::Idle);
            }
        }
    }
    
    void updateChase(float deltaTime, const Math::Vector3& playerPos) {
        // Update visibility
        canSeePlayer = checkPlayerVisibility(playerPos);
        
        if (canSeePlayer) {
            lastKnownPlayerPos = playerPos;
            searchTimer = 5.0f; // Reset search timer
        }
        
        // Check attack range
        float dist = (playerPos - position).length();
        if (dist < stats.attackRange && canSeePlayer) {
            transitionTo(EnemyState::Attack);
            return;
        }
        
        // Move toward player (or last known position)
        Math::Vector3 target = canSeePlayer ? playerPos : lastKnownPlayerPos;
        moveToward(target, canSeePlayer ? stats.runSpeed : stats.moveSpeed, deltaTime);
        
        // Search behavior
        if (!canSeePlayer) {
            searchTimer -= deltaTime;
            if (searchTimer <= 0) {
                alertLevel = 0;
                transitionTo(EnemyState::Idle);
            }
        }
    }
    
    void updateAttack(float deltaTime, const Math::Vector3& playerPos) {
        // Look at player
        lookAt(playerPos);
        
        // Attack cooldown
        attackTimer -= deltaTime;
        
        if (attackTimer <= 0 && !isAttacking) {
            // Start attack
            isAttacking = true;
            attackAnimation.start(0.0f, 1.0f, 0.3f);
        }
        
        // Attack animation
        if (isAttacking) {
            attackAnimation.update(deltaTime);
            
            // Deal damage at peak of attack
            if (attackAnimation.progress > 0.5f && attackAnimation.progress < 0.6f) {
                float dist = (playerPos - position).length();
                if (dist < stats.attackRange) {
                    // Deal damage
                    Math::Vector3 attackDir = playerPos - position;
                    attackDir.normalize();
                    
                    if (onAttack) {
                        onAttack(stats.attackDamage, attackDir);
                    }
                }
            }
            
            if (attackAnimation.isFinished()) {
                isAttacking = false;
                attackTimer = stats.attackCooldown;
            }
        }
        
        // Check if player moved out of range
        float dist = (playerPos - position).length();
        if (dist > stats.attackRange * 1.5f && !isAttacking) {
            transitionTo(EnemyState::Chase);
        }
        
        // Check visibility
        if (!checkPlayerVisibility(playerPos) && !isAttacking) {
            lastKnownPlayerPos = playerPos;
            transitionTo(EnemyState::Chase);
        }
    }
    
    void updateStunned(float deltaTime) {
        if (!damageable.knockback.isStunned()) {
            // Recovery from stun
            if (canSeePlayer) {
                transitionTo(EnemyState::Chase);
            } else {
                transitionTo(EnemyState::Alert);
                alertLevel = 0.5f;
            }
        }
    }
    
    void updateDying(float deltaTime) {
        deathTimer += deltaTime;
        
        // Simple death fall
        velocity = velocity + deathDirection * 2.0f;
        velocity.y -= 15.0f * deltaTime;
        
        if (deathTimer > 2.0f) {
            transitionTo(EnemyState::Dead);
        }
    }
    
    void updateMovement(float deltaTime) {
        // Apply knockback
        Math::Vector3 knockbackOffset = damageable.getPositionOffset();
        
        // Update position
        position = position + velocity * deltaTime + knockbackOffset * deltaTime;
        
        // Ground clamp
        if (position.y < 0) {
            position.y = 0;
            velocity.y = 0;
        }
        
        // Dampen velocity
        velocity.x *= 0.9f;
        velocity.z *= 0.9f;
    }
    
    void transitionTo(EnemyState newState) {
        previousState = state;
        state = newState;
        stateTimer = 0;
    }
    
    void moveToward(const Math::Vector3& target, float speed, float deltaTime) {
        Math::Vector3 dir = target - position;
        dir.y = 0;
        float dist = dir.length();
        
        if (dist > 0.1f) {
            dir.normalize();
            
            velocity.x = dir.x * speed;
            velocity.z = dir.z * speed;
            
            // Face movement direction
            lookAt(target);
        }
    }
    
    void lookAt(const Math::Vector3& target) {
        Math::Vector3 dir = target - position;
        float targetYaw = atan2f(dir.x, dir.z);
        
        // Smooth rotation
        rotationSpring.target = targetYaw;
        yaw = rotationSpring.current;
    }
    
    bool checkPlayerVisibility(const Math::Vector3& playerPos) {
        Math::Vector3 toPlayer = playerPos - position;
        float dist = toPlayer.length();
        
        // Range check
        if (dist > stats.sightRange) return false;
        
        // Angle check
        toPlayer.normalize();
        Math::Vector3 forward(sinf(yaw), 0, cosf(yaw));
        float dot = forward.dot(toPlayer);
        float angle = acosf(Math::clamp(dot, -1.0f, 1.0f));
        
        if (angle > stats.sightAngle * Math::PI / 180.0f) return false;
        
        // Should raycast for obstacles here
        
        canSeePlayer = true;
        lastKnownPlayerPos = playerPos;
        return true;
    }
    
    void onHit(const DamageInfo& info) {
        hitDirection = info.direction;
        hitReactionTimer = 0.3f;
        
        // Interrupt attack
        if (isAttacking) {
            isAttacking = false;
            attackTimer = stats.attackCooldown * 0.5f;
        }
        
        // Stun if enough damage
        if (damageable.knockback.isStunned() && state != EnemyState::Stunned) {
            transitionTo(EnemyState::Stunned);
        }
        
        // Alert if not already
        if (state == EnemyState::Idle || state == EnemyState::Patrol) {
            lastKnownPlayerPos = info.sourcePosition;
            transitionTo(EnemyState::Alert);
            alertLevel = 0.7f;
        }
    }
    
    void die(const DamageInfo& info) {
        transitionTo(EnemyState::Dying);
        deathDirection = info.direction;
        
        if (onDeath) onDeath();
    }
    
    bool takeDamage(DamageInfo& info) {
        return damageable.takeDamage(info);
    }
    
    // Getters
    bool isAlive() const { return state != EnemyState::Dead && state != EnemyState::Dying; }
    bool isDead() const { return state == EnemyState::Dead; }
    float getHealthPercent() const { return damageable.getHealthPercent(); }
    
    Math::Vector3 getCenter() const {
        return Math::Vector3(position.x, position.y + height * 0.5f, position.z);
    }
    
    // Patrol setup
    void addPatrolPoint(const Math::Vector3& point) {
        patrolPoints.push_back(point);
    }
    
    // Rendering
    void draw() {
        if (state == EnemyState::Dead) return;
        
        glPushMatrix();
        
        // Apply position with spring smoothing
        Math::Vector3 renderPos = positionSpring.current;
        glTranslatef(renderPos.x, renderPos.y + height * 0.5f, renderPos.z);
        glRotatef(rotationSpring.current * 57.2958f, 0, 1, 0);
        
        // Death rotation
        if (state == EnemyState::Dying) {
            float deathProgress = Math::clamp(deathTimer / 1.0f, 0.0f, 1.0f);
            glRotatef(-deathProgress * 90.0f, 1, 0, 0);
        }
        
        // Hit reaction lean
        if (hitReactionTimer > 0) {
            float hitProgress = hitReactionTimer / 0.3f;
            float leanAngle = sinf(hitProgress * Math::PI) * 15.0f;
            glRotatef(leanAngle, hitDirection.z, 0, -hitDirection.x);
        }
        
        // Flash effect
        Math::Color flashCol = damageable.getFlashColor();
        float alpha = damageable.getRenderAlpha();
        
        if (flashCol.a > 0.01f) {
            glColor4f(1.0f, flashCol.g, flashCol.b, alpha);
        } else {
            setColorByType();
        }
        
        // Draw enemy model
        drawEnemyModel();
        
        glPopMatrix();
        
        // Draw health bar if damaged
        if (damageable.currentHealth < damageable.maxHealth && isAlive()) {
            drawHealthBar();
        }
    }
    
private:
    void setColorByType() {
        switch (type) {
            case EnemyType::Zombie:
                glColor3f(0.4f, 0.5f, 0.3f); // Greenish
                break;
            case EnemyType::Soldier:
                glColor3f(0.3f, 0.35f, 0.4f); // Military gray
                break;
            case EnemyType::Heavy:
                glColor3f(0.5f, 0.3f, 0.3f); // Reddish
                break;
            case EnemyType::Boss:
                glColor3f(0.6f, 0.2f, 0.2f); // Dark red
                break;
        }
    }
    
    void drawEnemyModel() {
        // Simple capsule shape
        float r = radius;
        float h = height;
        
        // Body (cylinder)
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 16; ++i) {
            float angle = (float)i / 16.0f * Math::PI * 2.0f;
            float x = cosf(angle) * r;
            float z = sinf(angle) * r;
            glNormal3f(cosf(angle), 0, sinf(angle));
            glVertex3f(x, -h * 0.4f, z);
            glVertex3f(x, h * 0.4f, z);
        }
        glEnd();
        
        // Head (sphere-ish)
        glPushMatrix();
        glTranslatef(0, h * 0.35f, 0);
        
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 1, 0);
        glVertex3f(0, r * 0.8f, 0);
        for (int i = 0; i <= 16; ++i) {
            float angle = (float)i / 16.0f * Math::PI * 2.0f;
            glVertex3f(cosf(angle) * r * 0.6f, 0, sinf(angle) * r * 0.6f);
        }
        glEnd();
        
        glPopMatrix();
    }
    
    void drawHealthBar() {
        glPushMatrix();
        
        glTranslatef(position.x, position.y + height + 0.3f, position.z);
        
        // Billboard
        float modelview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        
        // Remove rotation
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                modelview[i * 4 + j] = (i == j) ? 1.0f : 0.0f;
            }
        }
        glLoadMatrixf(modelview);
        
        float barWidth = 0.8f;
        float barHeight = 0.08f;
        float healthPercent = getHealthPercent();
        
        // Background
        glColor4f(0.2f, 0.0f, 0.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex3f(-barWidth * 0.5f, 0, 0);
        glVertex3f(barWidth * 0.5f, 0, 0);
        glVertex3f(barWidth * 0.5f, barHeight, 0);
        glVertex3f(-barWidth * 0.5f, barHeight, 0);
        glEnd();
        
        // Health
        float healthWidth = barWidth * healthPercent;
        glColor4f(0.8f, 0.2f, 0.2f, 0.9f);
        glBegin(GL_QUADS);
        glVertex3f(-barWidth * 0.5f, 0, 0.01f);
        glVertex3f(-barWidth * 0.5f + healthWidth, 0, 0.01f);
        glVertex3f(-barWidth * 0.5f + healthWidth, barHeight, 0.01f);
        glVertex3f(-barWidth * 0.5f, barHeight, 0.01f);
        glEnd();
        
        glPopMatrix();
    }
};

// Type alias for backward compatibility
using Enemy = EnhancedEnemy;

} // namespace Doomers
