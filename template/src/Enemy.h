// ============================================================================
// DOOMERS - Enemy.h
// Enemy AI and behavior system
// ============================================================================
#ifndef ENEMY_H
#define ENEMY_H

#include "Vector3.h"
#include "GameConfig.h"
#include "LowPolyModels.h"
#include "TextureManager.h"
#include "ModelLoader.h"
#include <glut.h>
#include <stdlib.h>

enum EnemyType {
    ENEMY_ZOMBIE,
    ENEMY_DEMON,
    ENEMY_BOSS
};

enum EnemyState {
    ENEMY_IDLE,
    ENEMY_PATROL,
    ENEMY_CHASE,
    ENEMY_ATTACK,
    ENEMY_HURT,
    ENEMY_DEAD
};

// Boss projectile structure
struct BossProjectile {
    Vector3 position;
    Vector3 velocity;
    float lifetime;
    bool active;
    int type; // 0 = fireball, 1 = meteor, 2 = shockwave
    float size;
    
    BossProjectile() : lifetime(0), active(false), type(0), size(1.0f) {}
};

class Enemy {
public:
    Vector3 position;
    Vector3 velocity;
    Vector3 patrolStart;
    Vector3 patrolEnd;
    
    float rotationY;
    float speed;
    float animPhase;
    float hurtTimer;
    float attackCooldown;
    float deathTimer;
    float deathScale;  // For death shrink animation
    bool patrolToEnd;  // Patrol direction per enemy
    
    // NEW: For one-at-a-time attack system
    bool isActiveAttacker;
    float damageFlashTimer;  // Red flash when hit
    
    // BOSS-SPECIFIC ABILITIES
    static const int MAX_BOSS_PROJECTILES = 20;
    BossProjectile projectiles[MAX_BOSS_PROJECTILES];
    float specialAbilityCooldown;  // Cooldown for special attacks
    float teleportCooldown;        // Cooldown for teleport
    float groundSlamCooldown;      // Ground slam attack
    float meteorShowerCooldown;    // Meteor shower attack
    int currentPhase;              // Boss fight phase (1, 2, 3)
    float phaseTransitionTimer;    // Animation when changing phases
    bool isCharging;               // Charging an attack
    float chargeTimer;             // How long been charging
    Vector3 chargeTarget;          // Where charging towards
    float hoverHeight;             // Boss floats
    float wingFlapPhase;           // Wing animation
    bool hasGravity;               // Whether boss falls from platforms
    float verticalVelocity;        // For gravity
    
    // Kick attack tracking
    bool isKicking;                // Currently performing kick animation
    float kickTimer;               // Time into kick animation
    bool kickDamageDealt;          // Whether this kick already dealt damage
    
    int health;
    int maxHealth;
    int damage;
    int scoreValue;
    
    EnemyType type;
    EnemyState state;
    bool active;
    
    Enemy() {
        reset();
    }
    
    void reset() {
        position = Vector3(0, 0, 0);
        velocity = Vector3(0, 0, 0);
        patrolStart = Vector3(-5, 0, 0);
        patrolEnd = Vector3(5, 0, 0);
        
        rotationY = 0.0f;
        speed = ENEMY_SPEED;
        animPhase = 0.0f;
        hurtTimer = 0.0f;
        attackCooldown = 0.0f;
        deathTimer = 0.0f;
        deathScale = 1.0f;
        patrolToEnd = true;
        
        // NEW: Attack system
        isActiveAttacker = false;
        damageFlashTimer = 0.0f;
        showHealthBar = true;
        
        // Boss ability reset
        specialAbilityCooldown = 0.0f;
        teleportCooldown = 0.0f;
        groundSlamCooldown = 0.0f;
        meteorShowerCooldown = 0.0f;
        currentPhase = 1;
        phaseTransitionTimer = 0.0f;
        isCharging = false;
        chargeTimer = 0.0f;
        chargeTarget = Vector3(0, 0, 0);
        hoverHeight = 0.0f;
        wingFlapPhase = 0.0f;
        hasGravity = true;
        verticalVelocity = 0.0f;
        
        // Kick attack reset
        isKicking = false;
        kickTimer = 0.0f;
        kickDamageDealt = false;
        
        for (int i = 0; i < MAX_BOSS_PROJECTILES; i++) {
            projectiles[i].active = false;
        }
        
        health = 50;
        maxHealth = 50;
        damage = ENEMY_DAMAGE;
        scoreValue = 100;
        
        type = ENEMY_ZOMBIE;
        state = ENEMY_PATROL;
        active = true;
    }
    
    void init(EnemyType enemyType, const Vector3& pos, const Vector3& patrolA, const Vector3& patrolB) {
        reset(); // Reset all values first
        type = enemyType;
        position = pos;
        patrolStart = patrolA;
        patrolEnd = patrolB;
        
        switch (type) {
            case ENEMY_ZOMBIE:
                health = maxHealth = ZOMBIE_HEALTH;
                speed = ENEMY_SPEED * ZOMBIE_SPEED_MULT;
                damage = ZOMBIE_DAMAGE;
                scoreValue = ZOMBIE_SCORE;
                hasGravity = false;
                break;
            case ENEMY_DEMON:
                health = maxHealth = DEMON_HEALTH;
                speed = ENEMY_SPEED * DEMON_SPEED_MULT;
                damage = DEMON_DAMAGE;
                scoreValue = DEMON_SCORE;
                hasGravity = false;
                break;
            case ENEMY_BOSS:
                health = maxHealth = BOSS_HEALTH;
                speed = ENEMY_SPEED * BOSS_SPEED_MULT;
                damage = BOSS_DAMAGE;
                scoreValue = BOSS_SCORE;
                hasGravity = true;  // Boss has gravity
                hoverHeight = 0.5f;
                specialAbilityCooldown = BOSS_ROCKET_COOLDOWN;
                teleportCooldown = BOSS_CHARGE_COOLDOWN;
                groundSlamCooldown = BOSS_GROUNDSLAM_COOLDOWN;
                meteorShowerCooldown = BOSS_METEOR_COOLDOWN;
                break;
        }
        
        state = ENEMY_PATROL;
        active = true;
    }
    
    // Fire a projectile at target (uses config speeds)
    void fireProjectile(const Vector3& target, int projectileType = 0) {
        for (int i = 0; i < MAX_BOSS_PROJECTILES; i++) {
            if (!projectiles[i].active) {
                projectiles[i].active = true;
                projectiles[i].position = position + Vector3(0, 2.0f, 0);
                projectiles[i].type = projectileType;
                projectiles[i].lifetime = 5.0f;
                
                Vector3 dir = (target - projectiles[i].position).normalize();
                // Use config speeds
                float projSpeed = (projectileType == 0) ? BOSS_PROJECTILE_SPEED : BOSS_METEOR_SPEED;
                projectiles[i].velocity = dir * projSpeed;
                projectiles[i].size = (projectileType == 0) ? 0.8f : 1.5f;
                break;
            }
        }
    }
    
    // Update boss projectiles
    void updateProjectiles(float deltaTime) {
        for (int i = 0; i < MAX_BOSS_PROJECTILES; i++) {
            if (projectiles[i].active) {
                projectiles[i].position = projectiles[i].position + projectiles[i].velocity * deltaTime;
                projectiles[i].lifetime -= deltaTime;
                
                // Deactivate if lifetime expired or hit ground
                if (projectiles[i].lifetime <= 0 || projectiles[i].position.y < -1.0f) {
                    projectiles[i].active = false;
                }
            }
        }
    }
    
    // Check if any projectile hits the player
    bool checkProjectileHit(const Vector3& playerPos, float hitRadius = 1.5f) {
        for (int i = 0; i < MAX_BOSS_PROJECTILES; i++) {
            if (projectiles[i].active) {
                float dist = projectiles[i].position.distanceTo(playerPos);
                if (dist < hitRadius + projectiles[i].size) {
                    projectiles[i].active = false;
                    return true;
                }
            }
        }
        return false;
    }
    
    // Get projectile damage based on type (uses config values)
    int getProjectileDamage() const {
        // Return different damage based on last hit projectile type
        // Default to fireball damage
        return BOSS_FIREBALL_DAMAGE;
    }
    
    // Check hit and return damage amount
    int checkProjectileHitDamage(const Vector3& playerPos, float hitRadius = 1.5f) {
        for (int i = 0; i < MAX_BOSS_PROJECTILES; i++) {
            if (projectiles[i].active) {
                float dist = projectiles[i].position.distanceTo(playerPos);
                if (dist < hitRadius + projectiles[i].size) {
                    int projType = projectiles[i].type;
                    projectiles[i].active = false;
                    // Return damage based on projectile type
                    switch (projType) {
                        case 0: return BOSS_FIREBALL_DAMAGE;     // Fireball/rocket
                        case 1: return BOSS_METEOR_DAMAGE;       // Meteor
                        case 2: return BOSS_GROUNDSLAM_DAMAGE;   // Ground slam
                        default: return BOSS_FIREBALL_DAMAGE;
                    }
                }
            }
        }
        return 0; // No hit
    }
    
    void update(float deltaTime, const Vector3& playerPos) {
        if (!active) return;
        
        // Update damage flash timer
        if (damageFlashTimer > 0) {
            damageFlashTimer -= deltaTime;
        }
        
        // Update animation
        animPhase += deltaTime * (state == ENEMY_CHASE ? 8.0f : 4.0f);
        
        // BOSS-SPECIFIC UPDATES
        if (type == ENEMY_BOSS) {
            updateBossAbilities(deltaTime, playerPos);
            wingFlapPhase += deltaTime * 6.0f;
            
            // Boss phase transitions based on health (uses config thresholds)
            float healthPercent = (float)health / maxHealth;
            if (healthPercent < BOSS_PHASE3_THRESHOLD && currentPhase < 3) {
                currentPhase = 3;
                phaseTransitionTimer = 1.5f;
                speed = ENEMY_SPEED * BOSS_PHASE3_SPEED_MULT; // Enraged speed!
            } else if (healthPercent < BOSS_PHASE2_THRESHOLD && currentPhase < 2) {
                currentPhase = 2;
                phaseTransitionTimer = 1.0f;
                speed = ENEMY_SPEED * BOSS_PHASE2_SPEED_MULT;
            }
            
            if (phaseTransitionTimer > 0) {
                phaseTransitionTimer -= deltaTime;
            }
        }
        
        // Update hurt timer - ENEMY STOPS MOVING WHEN HURT
        if (hurtTimer > 0) {
            hurtTimer -= deltaTime;
            velocity.x = 0;
            velocity.z = 0;
            if (hurtTimer <= 0 && state != ENEMY_DEAD) {
                // Only change to chase if NOT dead
                state = ENEMY_CHASE;
            }
            return;  // Don't move while hurt
        }
        
        // Update attack cooldown
        if (attackCooldown > 0) {
            attackCooldown -= deltaTime;
        }
        
        // Death handling - remove enemy after short death animation
        if (state == ENEMY_DEAD) {
            deathTimer += deltaTime;
            // Shrink and fade out
            deathScale = 1.0f - (deathTimer / 1.0f);
            if (deathScale < 0) deathScale = 0;
            if (deathTimer > 1.0f) {
                active = false;
            }
            return;
        }
        
        // AI behavior
        float distToPlayer = position.distanceTo(playerPos);
        
        // BOSS has different state logic - always aggressive!
        if (type == ENEMY_BOSS) {
            // Boss attacks from further away so player can see the kick animation
            if (distToPlayer < ENEMY_ATTACK_RANGE * 3.0f) {
                state = ENEMY_ATTACK;
            } else {
                state = ENEMY_CHASE;
            }
        }
        // State transitions - ONLY CHASE IF ACTIVE ATTACKER (for regular enemies)
        else if (distToPlayer < ENEMY_ATTACK_RANGE && isActiveAttacker) {
            state = ENEMY_ATTACK;
        } else if (distToPlayer < ENEMY_DETECT_RANGE && isActiveAttacker) {
            state = ENEMY_CHASE;
        } else {
            // If not the active attacker, patrol normally
            state = ENEMY_PATROL;
        }
        
        // Behavior execution
        switch (state) {
            case ENEMY_PATROL:
                patrol(deltaTime);
                break;
            case ENEMY_CHASE:
                if (type == ENEMY_BOSS) {
                    chaseBoss(deltaTime, playerPos);
                } else {
                    chase(deltaTime, playerPos);
                }
                break;
            case ENEMY_ATTACK:
                // Stay in place when attacking
                velocity.x = 0;
                velocity.z = 0;
                lookAt(playerPos);
                
                // BOSS kick attack logic
                if (type == ENEMY_BOSS) {
                    if (!isKicking && attackCooldown <= 0) {
                        // Start a new kick
                        isKicking = true;
                        kickTimer = 0.0f;
                        kickDamageDealt = false;
                    }
                    
                    if (isKicking) {
                        kickTimer += deltaTime;
                        // Kick animation is 20 frames at 24fps = ~0.83 seconds
                        float kickDuration = 20.0f / 24.0f;
                        if (kickTimer >= kickDuration) {
                            isKicking = false;
                            attackCooldown = 1.0f;  // Cooldown before next kick
                        }
                    }
                }
                break;
            default:
                break;
        }
        
        // Apply velocity with deltaTime for consistent movement
        position = position + velocity * deltaTime * 60.0f;
        
        // BOSS GRAVITY - falls from platforms!
        if (type == ENEMY_BOSS && hasGravity) {
            verticalVelocity -= 30.0f * deltaTime; // Gravity
            position.y += verticalVelocity * deltaTime;
            
            // Platform collision check - boss should land on platforms
            // Check against known Level 2 platform heights
            float platformY = -10.0f; // Default to below lava
            
            // Main arena floor (Y=1.5, size 35x35 at center)
            if (position.x >= -35 && position.x <= 35 && position.z >= -35 && position.z <= 35) {
                platformY = 1.5f + 0.5f; // Platform top + half height
            }
            
            // Corner platforms (Y=3.0, size 8x8)
            if ((position.x >= -28 && position.x <= -12 && position.z >= -28 && position.z <= -12) ||
                (position.x >= 12 && position.x <= 28 && position.z >= -28 && position.z <= -12) ||
                (position.x >= -28 && position.x <= -12 && position.z >= 12 && position.z <= 28) ||
                (position.x >= 12 && position.x <= 28 && position.z >= 12 && position.z <= 28)) {
                platformY = 3.0f + 0.5f;
            }
            
            // Boss platform (Y=4.5, size 15x10 at Z=30)
            if (position.x >= -15 && position.x <= 15 && position.z >= 20 && position.z <= 40) {
                platformY = 4.5f + 0.75f;
            }
            
            // Steps to boss platform
            if (position.x >= -10 && position.x <= 10 && position.z >= 17 && position.z <= 27) {
                platformY = 2.5f + 0.4f;
            }
            if (position.x >= -12 && position.x <= 12 && position.z >= 21 && position.z <= 31) {
                platformY = 3.5f + 0.4f;
            }
            
            // Side walkways (Y=2.0)
            if ((position.x >= -35 && position.x <= -25 && position.z >= -20 && position.z <= 20) ||
                (position.x >= 25 && position.x <= 35 && position.z >= -20 && position.z <= 20)) {
                platformY = 2.0f + 0.3f;
            }
            
            // Ground check against platform or lava floor
            float groundY = platformY + hoverHeight;
            if (position.y < groundY) {
                position.y = groundY;
                verticalVelocity = 0;
            }
        }
        
        // WALL COLLISION - prevent passing through walls
        float margin = 1.0f;
        if (position.x < -BOUNDARY + margin) position.x = -BOUNDARY + margin;
        if (position.x > BOUNDARY - margin) position.x = BOUNDARY - margin;
        if (position.z < -BOUNDARY + margin) position.z = -BOUNDARY + margin;
        if (position.z > BOUNDARY - margin) position.z = BOUNDARY - margin;
        
        // Ground clamping (for non-boss)
        if (type != ENEMY_BOSS) {
            float groundHeight = getGroundHeight();
            if (position.y < groundHeight) {
                position.y = groundHeight;
            }
        }
    }
    
    // BOSS-SPECIFIC CHASE - more aggressive!
    void chaseBoss(float deltaTime, const Vector3& playerPos) {
        Vector3 toPlayer = playerPos - position;
        toPlayer.y = 0; // Horizontal only
        
        float dist = toPlayer.length();
        
        // CHARGE ATTACK - when close enough but not too close (uses config speed)
        if (dist > 8.0f && dist < 20.0f && isCharging) {
            // Fast charge toward player
            Vector3 dir = toPlayer.normalize();
            velocity.x = dir.x * speed * BOSS_CHARGE_SPEED_MULT;
            velocity.z = dir.z * speed * BOSS_CHARGE_SPEED_MULT;
            lookAt(playerPos);
        } else if (dist > 2.0f) {
            Vector3 dir = toPlayer.normalize();
            velocity.x = dir.x * speed;
            velocity.z = dir.z * speed;
            lookAt(playerPos);
        } else {
            velocity.x = 0;
            velocity.z = 0;
            lookAt(playerPos);
        }
    }
    
    // BOSS SPECIAL ABILITIES - Devastating attacks!
    void updateBossAbilities(float deltaTime, const Vector3& playerPos) {
        // Update projectiles
        updateProjectiles(deltaTime);
        
        // Update cooldowns
        if (specialAbilityCooldown > 0) specialAbilityCooldown -= deltaTime;
        if (teleportCooldown > 0) teleportCooldown -= deltaTime;
        if (groundSlamCooldown > 0) groundSlamCooldown -= deltaTime;
        if (meteorShowerCooldown > 0) meteorShowerCooldown -= deltaTime;
        if (chargeTimer > 0) chargeTimer -= deltaTime;
        else isCharging = false;
        
        float distToPlayer = position.distanceTo(playerPos);
        
        // ROCKET BARRAGE - fires from cybernetic arm (uses config values)
        if (specialAbilityCooldown <= 0 && distToPlayer < 40.0f && distToPlayer > 6.0f) {
            // Fire rockets - count from config
            for (int i = 0; i < BOSS_ROCKET_COUNT; i++) {
                Vector3 target = playerPos;
                fireProjectile(target, 0);
            }
            specialAbilityCooldown = BOSS_ROCKET_COOLDOWN;
        }
        
        // CHARGE ATTACK - Rush at player with devastating force
        if (teleportCooldown <= 0 && distToPlayer > 10.0f && distToPlayer < 30.0f && !isCharging) {
            isCharging = true;
            chargeTimer = BOSS_CHARGE_DURATION;
            teleportCooldown = BOSS_CHARGE_COOLDOWN;
        }
        
        // GROUND SLAM - AOE attack when player is close (uses config)
        if (groundSlamCooldown <= 0 && distToPlayer < 6.0f) {
            // Create shockwave of projectiles - count from config
            float angleStep = 360.0f / BOSS_GROUNDSLAM_PROJECTILES;
            for (int i = 0; i < BOSS_GROUNDSLAM_PROJECTILES; i++) {
                float angle = i * angleStep * 3.14159f / 180.0f;
                for (int j = 0; j < MAX_BOSS_PROJECTILES; j++) {
                    if (!projectiles[j].active) {
                        projectiles[j].active = true;
                        projectiles[j].position = position + Vector3(0, 0.5f, 0);
                        projectiles[j].velocity = Vector3(cos(angle) * BOSS_GROUNDSLAM_SPEED, 2.0f, sin(angle) * BOSS_GROUNDSLAM_SPEED);
                        projectiles[j].type = 2; // Ground slam type
                        projectiles[j].lifetime = 1.5f;
                        projectiles[j].size = 0.8f;
                        break;
                    }
                }
            }
            groundSlamCooldown = BOSS_GROUNDSLAM_COOLDOWN;
        }
        
        // METEOR SHOWER - Phase 2+ only, rain of fire (uses config)
        if (currentPhase >= 2 && meteorShowerCooldown <= 0) {
            // Fire meteors from above - count from config
            for (int i = 0; i < BOSS_METEOR_COUNT; i++) {
                for (int j = 0; j < MAX_BOSS_PROJECTILES; j++) {
                    if (!projectiles[j].active) {
                        projectiles[j].active = true;
                        // Random position above player area
                        float rx = playerPos.x + (rand() % 30 - 15);
                        float rz = playerPos.z + (rand() % 30 - 15);
                        projectiles[j].position = Vector3(rx, 50.0f, rz);
                        projectiles[j].velocity = Vector3(0, -BOSS_METEOR_SPEED, 0);
                        projectiles[j].type = 1; // Meteor type
                        projectiles[j].lifetime = 4.0f;
                        projectiles[j].size = 1.5f;
                        break;
                    }
                }
            }
            meteorShowerCooldown = BOSS_METEOR_COOLDOWN;
        }
        
        // PHASE 3 ENRAGE - Rapid fire (uses config)
        if (currentPhase >= 3 && (int)(levelTime * 3) % 2 == 0) {
            if (specialAbilityCooldown <= 0) {
                fireProjectile(playerPos, 0);
                specialAbilityCooldown = BOSS_ENRAGE_FIRE_RATE;
            }
        }
    }
    
    float levelTime = 0.0f; // Track time for abilities
    
    float getGroundHeight() const {
        // Base ground height - can be extended for platforms
        return type == ENEMY_BOSS ? 0.5f : 0.0f;
    }
    
    void patrol(float deltaTime) {
        // Move between patrol points - use instance variable
        Vector3 target = patrolToEnd ? patrolEnd : patrolStart;
        
        Vector3 toTarget = target - position;
        toTarget.y = 0;
        float dist = toTarget.length();
        
        if (dist < 0.5f) {
            patrolToEnd = !patrolToEnd;
        } else {
            Vector3 dir = toTarget.normalize();
            velocity.x = dir.x * speed * 0.5f;
            velocity.z = dir.z * speed * 0.5f;
            lookAt(target);
        }
    }
    
    void chase(float deltaTime, const Vector3& playerPos) {
        // Calculate actual player feet position (playerPos.y is at eye level)
        float playerFeetY = playerPos.y - 1.8f; // PLAYER_HEIGHT
        
        Vector3 toPlayer = playerPos - position;
        
        // If player is on a platform significantly above us (more than 2.5 units above ground)
        // The enemy Y is at ground level (0), player feet should also be near 0 if on ground
        if (playerFeetY > position.y + 2.0f) {
            // Player is on an elevated platform - wait below
            toPlayer.y = 0;
            float dist = toPlayer.length();
            if (dist < 3.0f) {
                // Back away slightly
                Vector3 dir = toPlayer.normalize();
                velocity.x = -dir.x * speed * 0.3f;
                velocity.z = -dir.z * speed * 0.3f;
            } else {
                // Just stand and wait
                velocity.x = 0;
                velocity.z = 0;
            }
            lookAt(playerPos);
            return;
        }
        
        // Normal chase - ignore Y component for horizontal chase
        toPlayer.y = 0;
        float dist = toPlayer.length();
        
        // Chase until VERY close - at attack range * 0.3 (about 0.75 units from player)
        // This ensures enemies actually get close enough to attack
        if (dist > ENEMY_ATTACK_RANGE * 0.3f) {
            // Chase player - get right up to them!
            Vector3 dir = toPlayer.normalize();
            velocity.x = dir.x * speed;
            velocity.z = dir.z * speed;
            lookAt(playerPos);
        } else {
            // Very close - stop and attack!
            velocity.x = 0;
            velocity.z = 0;
            lookAt(playerPos);
        }
    }
    
    void lookAt(const Vector3& target) {
        Vector3 dir = target - position;
        dir.y = 0;
        if (dir.lengthSquared() > 0.01f) {
            rotationY = RAD2DEG(atan2(dir.x, dir.z));
        }
    }
    
    bool canAttack() const {
        return state == ENEMY_ATTACK && attackCooldown <= 0;
    }
    
    void performAttack() {
        attackCooldown = 1.5f;
    }
    
    // Check if boss kick hits the player - returns true if damage should be dealt
    // The kick hitbox is in front of the boss during the middle of the animation
    bool checkKickHit(const Vector3& playerPos, float playerRadius = 1.0f) {
        if (type != ENEMY_BOSS || !isKicking || kickDamageDealt) {
            return false;
        }
        
        // Kick deals damage around frames 8-14 (out of 20) - the actual kick swing
        float kickDuration = 20.0f / 24.0f;  // ~0.83 seconds total
        float kickProgress = kickTimer / kickDuration;  // 0 to 1
        
        // Damage window is 40% to 70% of the animation (the kick swing)
        if (kickProgress < 0.4f || kickProgress > 0.7f) {
            return false;
        }
        
        // Calculate kick hitbox position - in front of boss based on rotation
        float kickReach = 6.5f;  // How far the kick reaches (matches attack range)
        float radians = DEG2RAD(rotationY);
        Vector3 kickPos = position;
        kickPos.x += sinf(radians) * kickReach;
        kickPos.z += cosf(radians) * kickReach;
        kickPos.y += 1.5f;  // Kick height (leg level)
        
        // Check collision with player
        float dist = kickPos.distanceTo(playerPos);
        float kickRadius = 2.0f;  // Kick hitbox radius (slightly smaller for precision)
        
        if (dist < kickRadius + playerRadius) {
            kickDamageDealt = true;  // Only deal damage once per kick
            return true;
        }
        
        return false;
    }
    
    // Get the kick damage (boss melee damage)
    int getKickDamage() const {
        return damage;  // Uses boss damage value from config
    }
    
    void takeDamage(int dmg) {
        if (state == ENEMY_DEAD) return;
        
        // BOSS has 50% damage resistance - he's tanky!
        if (type == ENEMY_BOSS) {
            dmg = dmg / 2;
            if (dmg < 1) dmg = 1;
        }
        
        health -= dmg;
        hurtTimer = 0.5f;  // Increased hurt duration - enemy stops for longer
        damageFlashTimer = 0.3f;  // Red flash effect
        state = ENEMY_HURT;
        
        // Stop movement when hit (but boss doesn't stop as long)
        if (type != ENEMY_BOSS) {
            velocity.x = 0;
            velocity.z = 0;
        }
        
        // Knockback (boss has less knockback)
        if (type != ENEMY_BOSS) {
            velocity = velocity * -0.5f;
        }
        
        if (health <= 0) {
            health = 0;
            state = ENEMY_DEAD;
            deathTimer = 0.0f;
        }
    }
    
    bool isDead() const {
        return state == ENEMY_DEAD;
    }
    
    void draw() {
        if (!active) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        
        // Flash red when hurt - apply red color tint
        bool isFlashing = damageFlashTimer > 0 || hurtTimer > 0;
        if (isFlashing) {
            // Enable additive blend for red overlay
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // Draw a red overlay effect
            float flash = sin((damageFlashTimer + hurtTimer) * 30.0f) * 0.3f + 0.7f;
            glColor4f(1.0f, flash * 0.3f, flash * 0.3f, 1.0f);
        }
        
        // Death animation - sink into ground
        if (state == ENEMY_DEAD) {
            glTranslatef(0, -deathTimer * 0.5f, 0);
            glRotatef(deathTimer * 45.0f, 0, 0, 1);
        }
        
        // Calculate attack phase for attack animation
        float attackPhase = (state == ENEMY_ATTACK) ? attackCooldown : 0.0f;
        
        // Check if boss is enraged (low health)
        bool isEnraged = (type == ENEMY_BOSS && health < maxHealth * 0.3f);
        
        // Apply skin texture if available for more detailed rendering
        TextureID enemyTex = TEX_ENEMY_ZOMBIE;
        switch (type) {
            case ENEMY_ZOMBIE:
                enemyTex = TEX_ENEMY_ZOMBIE;
                break;
            case ENEMY_DEMON:
                enemyTex = TEX_ENEMY_DEMON;
                break;
            case ENEMY_BOSS:
                enemyTex = TEX_ENEMY_BOSS;
                break;
        }
        
        // NOTE: Procedural enemy models don't have texture coordinates
        // They use vertex colors via setColor() in EnemyModels.h
        // Textures would require UV coordinates on each vertex
        
        switch (type) {
            case ENEMY_ZOMBIE:
                // Use new detailed zombie model with attack animation
                LowPolyModels::drawZombie(rotationY, animPhase, (float)health, (float)maxHealth, attackPhase);
                break;
            case ENEMY_DEMON:
                // Use new detailed demon model with attack animation
                LowPolyModels::drawDemon(rotationY, animPhase, attackPhase);
                break;
            case ENEMY_BOSS:
                // Use devil 3D model with baked frame animation
                glPushMatrix();
                glRotatef(rotationY, 0, 1, 0);
                
                float bossScale = 4.0f;
                
                // Material setup
                GLfloat bossAmbient[] = {0.6f, 0.6f, 0.6f, 1.0f};
                GLfloat bossDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
                GLfloat bossSpecular[] = {0.2f, 0.2f, 0.2f, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, bossAmbient);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, bossDiffuse);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, bossSpecular);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);
                
                glEnable(GL_COLOR_MATERIAL);
                glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
                
                // Color tint based on state
                if (isEnraged) {
                    glColor3f(1.0f, 0.4f, 0.4f);  // Strong red when enraged
                } else if (state == ENEMY_ATTACK || isKicking) {
                    glColor3f(1.0f, 0.7f, 0.7f);  // Light red for attack
                } else {
                    glColor3f(1.0f, 1.0f, 1.0f);  // Normal color
                }
                
                // Choose animation based on state
                if (isKicking && AnimationLoader::isLoaded(ANIM_KICK)) {
                    // Play kick animation
                    AnimationLoader::drawAnimated(ANIM_KICK, kickTimer, bossScale);
                } else if (AnimationLoader::isLoaded(ANIM_WALK)) {
                    // Use animPhase for walk animation timing
                    float animTime = animPhase * 0.05f;
                    AnimationLoader::drawAnimated(ANIM_WALK, animTime, bossScale);
                } else {
                    // Fallback to procedural boss model only
                    LowPolyModels::drawBoss(rotationY, animPhase, (float)health, (float)maxHealth, isEnraged);
                }
                
                glDisable(GL_COLOR_MATERIAL);
                glPopMatrix();
                break;
        }
        
        // Procedural models rendered with vertex colors
        
        // Draw red damage overlay when hurt - FULL BODY RED FLASH
        if (isFlashing) {
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);  // Draw over everything
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            
            float alpha = (damageFlashTimer + hurtTimer) * 2.0f;
            if (alpha > 0.8f) alpha = 0.8f;
            glColor4f(1.0f, 0.0f, 0.0f, alpha);
            
            // Draw overlapping spheres to cover entire enemy body
            float enemyHeight = (type == ENEMY_BOSS) ? 3.0f : ((type == ENEMY_DEMON) ? 2.0f : 1.8f);
            float bodyWidth = (type == ENEMY_BOSS) ? 1.8f : ((type == ENEMY_DEMON) ? 1.4f : 1.0f);
            
            // Lower body (legs/torso)
            glPushMatrix();
            glTranslatef(0, enemyHeight * 0.25f, 0);
            glScalef(bodyWidth, enemyHeight * 0.4f, bodyWidth * 0.8f);
            glutSolidSphere(1.0f, 8, 8);
            glPopMatrix();
            
            // Upper body (chest)
            glPushMatrix();
            glTranslatef(0, enemyHeight * 0.55f, 0);
            glScalef(bodyWidth * 1.1f, enemyHeight * 0.35f, bodyWidth * 0.9f);
            glutSolidSphere(1.0f, 8, 8);
            glPopMatrix();
            
            // Head
            glPushMatrix();
            glTranslatef(0, enemyHeight * 0.85f, 0);
            glScalef(bodyWidth * 0.6f, enemyHeight * 0.2f, bodyWidth * 0.6f);
            glutSolidSphere(1.0f, 8, 8);
            glPopMatrix();
            
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);  // Reset color
        }
        
        glPopMatrix();
        
        // Draw health bar for non-dead enemies (above the model)
        // Health bar visibility is now controlled externally via showHealthBar flag
        if (state != ENEMY_DEAD && showHealthBar) {
            drawHealthBar();
        }
    }
    
    // Flag to control health bar visibility (set by Game based on line of sight)
    bool showHealthBar;
    
    void drawHealthBar() {
        // Larger health bar for demons and bosses
        float barHeight = (type == ENEMY_BOSS) ? 4.5f : ((type == ENEMY_DEMON) ? 3.5f : 2.5f);
        float barWidth = (type == ENEMY_BOSS) ? 1.5f : ((type == ENEMY_DEMON) ? 1.0f : 0.5f);
        float barThickness = (type == ENEMY_BOSS) ? 0.15f : ((type == ENEMY_DEMON) ? 0.1f : 0.05f);
        
        glPushMatrix();
        glTranslatef(position.x, position.y + barHeight, position.z);
        
        // Billboard - face camera
        float modelview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        
        // Remove rotation from modelview matrix
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (i == j) modelview[i * 4 + j] = 1.0f;
                else modelview[i * 4 + j] = 0.0f;
            }
        }
        glLoadMatrixf(modelview);
        
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);  // Always visible
        
        // Black outline/border
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(-barWidth - 0.02f, -barThickness - 0.02f, 0);
        glVertex3f(barWidth + 0.02f, -barThickness - 0.02f, 0);
        glVertex3f(barWidth + 0.02f, barThickness + 0.02f, 0);
        glVertex3f(-barWidth - 0.02f, barThickness + 0.02f, 0);
        glEnd();
        
        // Dark red background
        glColor3f(0.3f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(-barWidth, -barThickness, 0.001f);
        glVertex3f(barWidth, -barThickness, 0.001f);
        glVertex3f(barWidth, barThickness, 0.001f);
        glVertex3f(-barWidth, barThickness, 0.001f);
        glEnd();
        
        // Health bar - color changes from green to red as health decreases
        float healthPercent = (float)health / maxHealth;
        float r = 1.0f - healthPercent;
        float g = healthPercent;
        
        // Calculate bar fill width
        float fillWidth = barWidth * 2.0f * healthPercent;
        float innerThickness = barThickness * 0.6f;
        
        glColor3f(r, g, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(-barWidth + 0.02f, -innerThickness, 0.002f);
        glVertex3f(-barWidth + 0.02f + fillWidth - 0.04f, -innerThickness, 0.002f);
        glVertex3f(-barWidth + 0.02f + fillWidth - 0.04f, innerThickness, 0.002f);
        glVertex3f(-barWidth + 0.02f, innerThickness, 0.002f);
        glEnd();
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
    
    // Draw boss projectiles (fireballs, meteors)
    void drawProjectiles() {
        if (type != ENEMY_BOSS) return;
        
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        
        for (int i = 0; i < MAX_BOSS_PROJECTILES; i++) {
            if (projectiles[i].active) {
                glPushMatrix();
                glTranslatef(projectiles[i].position.x, 
                           projectiles[i].position.y, 
                           projectiles[i].position.z);
                
                if (projectiles[i].type == 0) {
                    // ROCKET - Gray missile with fire trail
                    float pulse = sin(projectiles[i].lifetime * 20.0f) * 0.2f + 0.8f;
                    
                    // Calculate rocket orientation from velocity
                    Vector3 vel = projectiles[i].velocity;
                    float angle = atan2(vel.x, vel.z) * 180.0f / 3.14159f;
                    glRotatef(-angle, 0, 1, 0);
                    glRotatef(90, 1, 0, 0);
                    
                    // ROCKET BODY - metallic gray
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glColor4f(0.4f, 0.4f, 0.45f, 1.0f);
                    GLUquadric* quad = gluNewQuadric();
                    gluCylinder(quad, projectiles[i].size * 0.3f, projectiles[i].size * 0.3f, projectiles[i].size * 2.0f, 8, 1);
                    
                    // Rocket nose cone
                    glColor4f(0.6f, 0.1f, 0.05f, 1.0f);
                    glTranslatef(0, 0, projectiles[i].size * 2.0f);
                    gluCylinder(quad, projectiles[i].size * 0.3f, 0, projectiles[i].size * 0.5f, 8, 1);
                    
                    // FIRE EXHAUST behind rocket
                    glTranslatef(0, 0, -projectiles[i].size * 2.2f);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    glColor4f(1.0f, 0.5f * pulse, 0.0f, 0.8f);
                    glutSolidSphere(projectiles[i].size * 0.5f, 8, 8);
                    
                    // Outer exhaust glow
                    glColor4f(1.0f, 0.3f, 0.0f, 0.4f);
                    glutSolidSphere(projectiles[i].size * 0.9f, 8, 8);
                    
                    gluDeleteQuadric(quad);
                    
                } else if (projectiles[i].type == 1) {
                    // METEOR - Flaming rock from sky
                    float pulse = sin(projectiles[i].lifetime * 15.0f) * 0.15f + 0.85f;
                    
                    // Fire trail above (meteor falling down)
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    glColor4f(1.0f, 0.4f, 0.0f, 0.6f);
                    glPushMatrix();
                    glTranslatef(0, projectiles[i].size * 2.5f, 0);
                    glutSolidSphere(projectiles[i].size * 1.3f, 8, 8);
                    glTranslatef(0, projectiles[i].size, 0);
                    glColor4f(1.0f, 0.2f, 0.0f, 0.3f);
                    glutSolidSphere(projectiles[i].size * 0.8f, 6, 6);
                    glPopMatrix();
                    
                    // Outer glow
                    glColor4f(0.9f, 0.3f, 0.0f, 0.5f);
                    glutSolidSphere(projectiles[i].size * 1.6f, 10, 10);
                    
                    // Rock core - dark
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glColor4f(0.25f, 0.12f * pulse, 0.08f, 1.0f);
                    glutSolidSphere(projectiles[i].size, 10, 10);
                    
                    // Hot cracks in rock
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    glColor4f(1.0f, 0.5f * pulse, 0.0f, 0.7f);
                    glutSolidSphere(projectiles[i].size * 0.6f, 6, 6);
                    
                } else if (projectiles[i].type == 2) {
                    // GROUND SLAM - Shockwave ring
                    float pulse = sin(projectiles[i].lifetime * 25.0f) * 0.2f + 0.8f;
                    
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    
                    // Fiery shockwave
                    glColor4f(1.0f, 0.4f * pulse, 0.0f, 0.7f);
                    glutSolidSphere(projectiles[i].size * 1.2f, 10, 10);
                    
                    // Inner hot core
                    glColor4f(1.0f, 0.7f * pulse, 0.2f, 0.9f);
                    glutSolidSphere(projectiles[i].size * 0.7f, 8, 8);
                    
                    // Outer glow
                    glColor4f(1.0f, 0.2f, 0.0f, 0.3f);
                    glutSolidSphere(projectiles[i].size * 1.8f, 8, 8);
                }
                
                glPopMatrix();
            }
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }
};

#endif // ENEMY_H
