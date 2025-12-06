// ============================================================================
// DOOMERS - Enemy.h
// Enemy AI and behavior system
// ============================================================================
#ifndef ENEMY_H
#define ENEMY_H

#include "Vector3.h"
#include "GameConfig.h"
#include "LowPolyModels.h"
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
        
        health = 50;
        maxHealth = 50;
        damage = ENEMY_DAMAGE;
        scoreValue = 100;
        
        type = ENEMY_ZOMBIE;
        state = ENEMY_PATROL;
        active = true;
    }
    
    void init(EnemyType enemyType, const Vector3& pos, const Vector3& patrolA, const Vector3& patrolB) {
        type = enemyType;
        position = pos;
        patrolStart = patrolA;
        patrolEnd = patrolB;
        
        switch (type) {
            case ENEMY_ZOMBIE:
                health = maxHealth = 50;
                speed = ENEMY_SPEED;
                damage = 10;
                scoreValue = 100;
                break;
            case ENEMY_DEMON:
                health = maxHealth = 80;
                speed = ENEMY_SPEED * 1.3f;
                damage = 20;
                scoreValue = 200;
                break;
            case ENEMY_BOSS:
                health = maxHealth = 200;
                speed = ENEMY_SPEED * 0.8f;
                damage = 30;
                scoreValue = 500;
                break;
        }
        
        state = ENEMY_PATROL;
        active = true;
    }
    
    void update(float deltaTime, const Vector3& playerPos) {
        if (!active) return;
        
        // Update damage flash timer
        if (damageFlashTimer > 0) {
            damageFlashTimer -= deltaTime;
        }
        
        // Update animation
        animPhase += deltaTime * (state == ENEMY_CHASE ? 8.0f : 4.0f);
        
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
        
        // State transitions - ONLY CHASE IF ACTIVE ATTACKER
        if (distToPlayer < ENEMY_ATTACK_RANGE && isActiveAttacker) {
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
                chase(deltaTime, playerPos);
                break;
            case ENEMY_ATTACK:
                // Stay in place when attacking
                velocity.x = 0;
                velocity.z = 0;
                lookAt(playerPos);
                break;
            default:
                break;
        }
        
        // Apply velocity with deltaTime for consistent movement
        position = position + velocity * deltaTime * 60.0f;
        
        // WALL COLLISION - prevent passing through walls
        float margin = 1.0f;
        if (position.x < -BOUNDARY + margin) position.x = -BOUNDARY + margin;
        if (position.x > BOUNDARY - margin) position.x = BOUNDARY - margin;
        if (position.z < -BOUNDARY + margin) position.z = -BOUNDARY + margin;
        if (position.z > BOUNDARY - margin) position.z = BOUNDARY - margin;
        
        // Ground clamping
        float groundHeight = getGroundHeight();
        if (position.y < groundHeight) {
            position.y = groundHeight;
        }
    }
    
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
    
    void takeDamage(int dmg) {
        if (state == ENEMY_DEAD) return;
        
        health -= dmg;
        hurtTimer = 0.5f;  // Increased hurt duration - enemy stops for longer
        damageFlashTimer = 0.3f;  // Red flash effect
        state = ENEMY_HURT;
        
        // Stop movement when hit
        velocity.x = 0;
        velocity.z = 0;
        
        // Knockback
        velocity = velocity * -0.5f;
        
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
                // Use new detailed boss model with rage effect
                LowPolyModels::drawBoss(rotationY, animPhase, (float)health, (float)maxHealth, isEnraged);
                break;
        }
        
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
};

#endif // ENEMY_H
