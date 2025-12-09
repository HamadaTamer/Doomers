// ============================================================================
// DOOMERS - Player.h
// Player controller with movement, shooting, health management
// ============================================================================
#ifndef PLAYER_H
#define PLAYER_H

#include "Vector3.h"
#include "GameConfig.h"
#include "Camera.h"
#include "LowPolyModels.h"
#include "TextureManager.h"
#include <glut.h>

class Player {
public:
    // Position and movement
    Vector3 position;
    Vector3 velocity;
    float rotationY;
    float speed;
    bool isSprinting;
    bool isOnGround;
    
    // Stats
    int health;
    int maxHealth;
    int ammo;
    int maxAmmo;
    int score;
    int enemiesKilled;
    
    // Weapon state
    float weaponRecoil;
    float weaponBob;
    float lastFireTime;
    bool isFiring;
    
    // Animation
    float damageFlash;
    float invincibilityTime;
    float walkPhase;
    float muzzleFlashTimer;
    
    // Lava damage
    float lavaDamageTimer;      // Cooldown between lava damage ticks
    float lavaInvincibilityTime; // Invincibility frames after lava damage
    bool isInLava;
    
    // Powerup states
    float speedBoostTime;
    float damageBoostTime;
    float invincibilityPowerupTime;
    bool hasSpeedBoost;
    bool hasDamageBoost;
    bool hasInvincibility;
    
    // Shield system
    float shieldHealth;
    float maxShieldHealth;
    bool hasShield;
    float shieldFlashTime;
    
    // Parkour animation state
    bool isDoingParkour;
    float parkourProgress;
    Vector3 parkourStartPos;
    Vector3 parkourEndPos;
    float parkourHeight;
    
    // Weapon light
    bool weaponLightOn;
    float weaponLightIntensity;
    
    // Knockback
    Vector3 knockbackVelocity;
    float knockbackTimer;
    
    // Camera shake (for big hits like boss kick)
    float cameraShakeIntensity;
    float cameraShakeTime;
    float cameraShakeOffsetX;
    float cameraShakeOffsetY;
    
    // Input state
    bool moveForward, moveBackward, moveLeft, moveRight;
    bool wantJump, wantSprint;
    
    // Level boundary (set from Game based on current level)
    float currentBoundary;
    
    // Reference to camera
    Camera* camera;
    
    Player() {
        reset();
        camera = nullptr;
    }
    
    void reset() {
        position = Vector3(0, PLAYER_HEIGHT, 0);
        velocity = Vector3(0, 0, 0);
        rotationY = 0.0f;
        speed = PLAYER_SPEED;
        isSprinting = false;
        isOnGround = true;
        
        health = PLAYER_MAX_HEALTH;
        maxHealth = PLAYER_MAX_HEALTH;
        ammo = 50;
        maxAmmo = MAX_AMMO;
        score = 0;
        enemiesKilled = 0;
        
        weaponRecoil = 0.0f;
        weaponBob = 0.0f;
        lastFireTime = 0.0f;
        isFiring = false;
        
        damageFlash = 0.0f;
        invincibilityTime = 0.0f;
        walkPhase = 0.0f;
        muzzleFlashTimer = 0.0f;
        
        // Lava damage
        lavaDamageTimer = 0.0f;
        lavaInvincibilityTime = 0.0f;
        isInLava = false;
        
        // Powerup states
        speedBoostTime = 0.0f;
        damageBoostTime = 0.0f;
        invincibilityPowerupTime = 0.0f;
        hasSpeedBoost = false;
        hasDamageBoost = false;
        hasInvincibility = false;
        
        // Shield system
        shieldHealth = 0.0f;
        maxShieldHealth = PLAYER_SHIELD_MAX; // Use config
        hasShield = false;
        shieldFlashTime = 0.0f;
        
        // Parkour state
        isDoingParkour = false;
        parkourProgress = 0.0f;
        parkourStartPos = Vector3(0, 0, 0);
        parkourEndPos = Vector3(0, 0, 0);
        parkourHeight = 0.0f;
        
        weaponLightOn = true;
        weaponLightIntensity = 1.0f;
        
        knockbackVelocity = Vector3(0, 0, 0);
        knockbackTimer = 0.0f;
        
        // Camera shake reset
        cameraShakeIntensity = 0.0f;
        cameraShakeTime = 0.0f;
        cameraShakeOffsetX = 0.0f;
        cameraShakeOffsetY = 0.0f;
        
        moveForward = moveBackward = moveLeft = moveRight = false;
        wantJump = wantSprint = false;
        
        currentBoundary = BOUNDARY; // Default boundary
    }
    
    void toggleWeaponLight() {
        weaponLightOn = !weaponLightOn;
    }
    
    void setCamera(Camera* cam) {
        camera = cam;
    }
    
    void setPosition(const Vector3& pos) {
        position = pos;
    }
    
    // Parkour direction vector (stored during startParkour)
    Vector3 parkourDirection;
    Vector3 obstacleCenter;
    
    // Start parkour vault animation (Vector-style - hands on obstacle, swing over)
    void startParkour(const Vector3& obstaclePos, float obstacleHeight, float obstacleDepth, float obstacleRotation) {
        if (isDoingParkour) return;
        
        isDoingParkour = true;
        parkourProgress = 0.0f;
        parkourStartPos = position;
        parkourHeight = obstacleHeight; // Height of obstacle top
        obstacleCenter = obstaclePos;
        
        // Direction player is facing (this is the vault direction)
        Vector3 forward = camera ? camera->getForward() : Vector3(0, 0, -1);
        forward.y = 0;
        if (forward.length() > 0.01f) {
            forward = forward.normalize();
        } else {
            forward = Vector3(0, 0, -1);
        }
        parkourDirection = forward;
        
        // End position: on the OTHER side of the obstacle
        // Total vault distance = approach to obstacle + obstacle depth + landing distance
        float totalVaultDist = obstacleDepth + 3.5f; // Enough to clear obstacle and land safely
        
        parkourEndPos = Vector3(
            parkourStartPos.x + forward.x * totalVaultDist,
            PLAYER_HEIGHT,
            parkourStartPos.z + forward.z * totalVaultDist
        );
    }
    
    // Update parkour animation - VECTOR-STYLE VAULT:
    // This is NOT a jump! Player places hands on obstacle and swings body over.
    // Phase 1 (0.0-0.15): Quick approach run to obstacle
    // Phase 2 (0.15-0.35): Plant hands, lift body horizontally onto obstacle
    // Phase 3 (0.35-0.65): Slide/swing body across obstacle (body stays low, near obstacle top)
    // Phase 4 (0.65-1.0): Drop down and land on other side
    void updateParkour(float deltaTime) {
        if (!isDoingParkour) return;
        
        // Animation speed - SLOWER so animation is more visible
        parkourProgress += deltaTime * 1.8f;
        
        if (parkourProgress >= 1.0f) {
            parkourProgress = 1.0f;
            isDoingParkour = false;
            position = parkourEndPos;
            velocity = Vector3(0, 0, 0);
            isOnGround = true;
            // Clear camera tilt when parkour ends
            if (camera) {
                camera->clearParkourTilt();
            }
            return;
        }
        
        float t = parkourProgress;
        
        // === HORIZONTAL MOVEMENT ===
        // Continuous forward motion with speed variation
        float horizT;
        if (t < 0.15f) {
            // Phase 1: Fast approach (0-30% of distance)
            horizT = (t / 0.15f) * 0.30f;
        } else if (t < 0.35f) {
            // Phase 2: Slow down as hands plant (30-45% of distance)
            horizT = 0.30f + ((t - 0.15f) / 0.20f) * 0.15f;
        } else if (t < 0.65f) {
            // Phase 3: Body slides over obstacle (45-75% of distance)
            horizT = 0.45f + ((t - 0.35f) / 0.30f) * 0.30f;
        } else {
            // Phase 4: Fast landing/run out (75-100% of distance)
            horizT = 0.75f + ((t - 0.65f) / 0.35f) * 0.25f;
        }
        
        // Smooth easing for fluid motion
        float smoothHoriz = horizT * horizT * (3.0f - 2.0f * horizT);
        position.x = parkourStartPos.x + (parkourEndPos.x - parkourStartPos.x) * smoothHoriz;
        position.z = parkourStartPos.z + (parkourEndPos.z - parkourStartPos.z) * smoothHoriz;
        
        // === VERTICAL MOVEMENT - THE KEY TO VECTOR-STYLE ===
        // Player body stays LOW - never goes high above obstacle
        // Like sliding across a table, not jumping over it
        float targetY;
        float obstacleTopY = parkourHeight + 0.5f; // Just above obstacle surface
        
        if (t < 0.15f) {
            // Phase 1: Running approach - normal height, slight forward lean
            targetY = PLAYER_HEIGHT;
        } else if (t < 0.35f) {
            // Phase 2: Hands plant, body lifts UP to obstacle level
            // Smooth rise from ground to obstacle top
            float liftT = (t - 0.15f) / 0.20f;
            float smoothLift = liftT * liftT * (3.0f - 2.0f * liftT);
            targetY = PLAYER_HEIGHT + smoothLift * (obstacleTopY - PLAYER_HEIGHT);
        } else if (t < 0.65f) {
            // Phase 3: BODY HORIZONTAL OVER OBSTACLE - this is the Vector signature move
            // Player's body is essentially lying flat on/over the obstacle
            targetY = obstacleTopY;
        } else {
            // Phase 4: Drop down smoothly to landing
            float dropT = (t - 0.65f) / 0.35f;
            // Smooth drop curve
            float smoothDrop = dropT * dropT;
            targetY = obstacleTopY - smoothDrop * (obstacleTopY - PLAYER_HEIGHT);
        }
        position.y = targetY;
        
        // === CAMERA TILT FOR VISUAL FEEDBACK ===
        // This makes the parkour feel more dynamic
        if (camera) {
            float tiltAmount = 0.0f;
            if (t < 0.15f) {
                // Phase 1: Slight forward lean while running
                tiltAmount = (t / 0.15f) * 5.0f;
            } else if (t < 0.35f) {
                // Phase 2: Camera tilts down as hands plant (looking at obstacle)
                float plantT = (t - 0.15f) / 0.20f;
                tiltAmount = 5.0f + plantT * 15.0f; // Tilt down to 20 degrees
            } else if (t < 0.65f) {
                // Phase 3: Camera stays tilted while body is over obstacle
                tiltAmount = 20.0f - ((t - 0.35f) / 0.30f) * 10.0f; // Gradually reduce
            } else {
                // Phase 4: Camera levels out during landing
                float landT = (t - 0.65f) / 0.35f;
                tiltAmount = 10.0f * (1.0f - landT);
            }
            // Apply temporary pitch offset for vault feeling
            camera->setParkourTilt(tiltAmount);
        }
        
        // Zero out velocity during parkour - movement is controlled by animation
        velocity = Vector3(0, 0, 0);
        isOnGround = false;
    }
    
    void update(float deltaTime) {
        // If doing parkour, only update parkour animation
        if (isDoingParkour) {
            updateParkour(deltaTime);
            // Update weapon animation even during parkour
            updateWeaponAnimation(deltaTime);
            if (muzzleFlashTimer > 0) {
                muzzleFlashTimer -= deltaTime;
                if (muzzleFlashTimer < 0) muzzleFlashTimer = 0;
            }
            return;
        }
        
        // Get movement direction from camera
        Vector3 forward = camera ? camera->getForward() : Vector3(0, 0, -1);
        Vector3 right = camera ? camera->getRight() : Vector3(1, 0, 0);
        
        // Calculate movement
        Vector3 moveDir(0, 0, 0);
        if (moveForward) moveDir = moveDir + forward;
        if (moveBackward) moveDir = moveDir - forward;
        if (moveRight) moveDir = moveDir + right;
        if (moveLeft) moveDir = moveDir - right;
        
        // Normalize diagonal movement
        if (moveDir.lengthSquared() > 0.01f) {
            moveDir = moveDir.normalize();
        }
        
        // Apply sprint
        isSprinting = wantSprint && isOnGround;
        float currentSpeed = speed * (isSprinting ? PLAYER_SPRINT_MULTIPLIER : 1.0f);
        
        // Apply speed boost powerup
        if (hasSpeedBoost) {
            currentSpeed *= SPEED_BOOST_MULTIPLIER;
        }
        
        // Apply movement
        velocity.x = moveDir.x * currentSpeed;
        velocity.z = moveDir.z * currentSpeed;
        
        // Apply knockback
        if (knockbackTimer > 0) {
            knockbackTimer -= deltaTime;
            float knockbackStrength = knockbackTimer / 0.3f; // Decay over 0.3 seconds
            velocity.x += knockbackVelocity.x * knockbackStrength;
            velocity.z += knockbackVelocity.z * knockbackStrength;
        }
        
        // Update walk animation phase
        if (moveDir.lengthSquared() > 0.01f) {
            float walkSpeed = isSprinting ? 15.0f : 10.0f;
            walkPhase += deltaTime * walkSpeed;
            if (walkPhase > 6.28318f) walkPhase -= 6.28318f;
        } else {
            // Smoothly return to idle
            walkPhase *= 0.9f;
        }
        
        // Jump - only when player presses jump key AND is on ground
        // REMOVED auto-jumping. Player must explicitly press jump.
        if (wantJump && isOnGround) {
            velocity.y = PLAYER_JUMP_FORCE;
            isOnGround = false;
            wantJump = false;  // Consume the jump input
        } else {
            wantJump = false;  // Reset wantJump to prevent auto-jumping next frame
        }
        
        // Apply gravity ALWAYS when not on ground
        // This ensures player falls off platforms when walking off edge
        if (!isOnGround) {
            velocity.y -= GRAVITY;
            // Clamp fall velocity to prevent falling through floor
            if (velocity.y < -2.0f) velocity.y = -2.0f;
        }
        
        // Update position
        position = position + velocity;
        
        // Floor ground check - this is just the minimum floor
        // Platforms are checked separately in Game.h
        if (position.y <= PLAYER_HEIGHT) {
            position.y = PLAYER_HEIGHT;
            velocity.y = 0;
            isOnGround = true;
        }
        
        // World boundaries - walls and ceiling collision
        // Use larger boundary if currentBoundary is set (for Level 2)
        float activeBoundary = currentBoundary > 0 ? currentBoundary : BOUNDARY;
        float wallMargin = PLAYER_COLLISION_RADIUS + 0.3f;
        if (position.x < -activeBoundary + wallMargin) 
            position.x = -activeBoundary + wallMargin;
        if (position.x > activeBoundary - wallMargin) 
            position.x = activeBoundary - wallMargin;
        if (position.z < -activeBoundary + wallMargin) 
            position.z = -activeBoundary + wallMargin;
        if (position.z > activeBoundary - wallMargin) 
            position.z = activeBoundary - wallMargin;
        
        // Ceiling collision (only for indoor levels)
        if (currentBoundary <= BOUNDARY && position.y > WALL_HEIGHT - 1.0f) {
            position.y = WALL_HEIGHT - 1.0f;
            velocity.y = 0;
        }
        
        // Update rotation based on camera
        if (camera) {
            rotationY = camera->getYaw();
        }
        
        // Update weapon animation
        updateWeaponAnimation(deltaTime);
        
        // Update damage flash
        if (damageFlash > 0) {
            damageFlash -= deltaTime * 3.0f;
            if (damageFlash < 0) damageFlash = 0;
        }
        
        // Update camera shake
        if (cameraShakeTime > 0) {
            cameraShakeTime -= deltaTime;
            float shakeAmount = cameraShakeIntensity * (cameraShakeTime / 0.5f);  // Fade out
            cameraShakeOffsetX = ((rand() % 200) / 100.0f - 1.0f) * shakeAmount;
            cameraShakeOffsetY = ((rand() % 200) / 100.0f - 1.0f) * shakeAmount;
            if (cameraShakeTime <= 0) {
                cameraShakeTime = 0;
                cameraShakeOffsetX = 0;
                cameraShakeOffsetY = 0;
            }
        }
        
        // Update invincibility
        if (invincibilityTime > 0) {
            invincibilityTime -= deltaTime;
            if (invincibilityTime < 0) invincibilityTime = 0;
        }
        
        // Update powerup timers
        if (speedBoostTime > 0) {
            speedBoostTime -= deltaTime;
            if (speedBoostTime <= 0) {
                speedBoostTime = 0;
                hasSpeedBoost = false;
            }
        }
        if (damageBoostTime > 0) {
            damageBoostTime -= deltaTime;
            if (damageBoostTime <= 0) {
                damageBoostTime = 0;
                hasDamageBoost = false;
            }
        }
        if (invincibilityPowerupTime > 0) {
            invincibilityPowerupTime -= deltaTime;
            if (invincibilityPowerupTime <= 0) {
                invincibilityPowerupTime = 0;
                hasInvincibility = false;
            }
        }
        
        // Update shield flash
        if (shieldFlashTime > 0) {
            shieldFlashTime -= deltaTime;
            if (shieldFlashTime < 0) shieldFlashTime = 0;
        }
        
        // Update muzzle flash
        if (muzzleFlashTimer > 0) {
            muzzleFlashTimer -= deltaTime;
            if (muzzleFlashTimer < 0) muzzleFlashTimer = 0;
        }
        
        // Weapon light intensity flicker effect
        if (weaponLightOn) {
            // BRIGHT weapon flashlight - high intensity for visibility
            weaponLightIntensity = 1.8f + 0.2f * sin(walkPhase * 5.0f);
        }
    }
    
    void updateWeaponAnimation(float deltaTime) {
        // Weapon bob when moving - SMOOTH and subtle
        static float bobPhase = 0.0f;
        if (moveForward || moveBackward || moveLeft || moveRight) {
            float bobSpeed = isSprinting ? 10.0f : 6.0f;
            bobPhase += deltaTime * bobSpeed;
            // Smoother sine wave with lower amplitude
            float targetBob = sin(bobPhase) * 0.012f;
            weaponBob += (targetBob - weaponBob) * 0.3f; // Smooth interpolation
        } else {
            weaponBob *= 0.85f; // Faster decay when stopped
            bobPhase *= 0.95f; // Reset phase gradually
        }
        
        // Recoil recovery - faster for smoother feel
        if (weaponRecoil > 0) {
            weaponRecoil -= deltaTime * 8.0f;
            if (weaponRecoil < 0) weaponRecoil = 0;
        }
    }
    
    bool canFire(float currentTime) {
        return (currentTime - lastFireTime >= WEAPON_FIRE_RATE) && ammo > 0;
    }
    
    void fire(float currentTime) {
        if (canFire(currentTime)) {
            ammo--;
            lastFireTime = currentTime;
            weaponRecoil = 0.79f;  // Reduced recoil for smoother feel
            muzzleFlashTimer = 0.04f; // Slightly shorter flash
            isFiring = true;
        }
    }
    
    void takeDamage(int damage, Vector3 attackDir = Vector3(0,0,0)) {
        // Check invincibility powerup first
        if (hasInvincibility) return;
        
        if (invincibilityTime <= 0) {
            int remainingDamage = damage;
            
            // Shield absorbs damage first
            if (hasShield && shieldHealth > 0) {
                shieldFlashTime = 0.3f; // Flash effect when hit
                if (shieldHealth >= remainingDamage) {
                    shieldHealth -= remainingDamage;
                    remainingDamage = 0;
                } else {
                    remainingDamage -= (int)shieldHealth;
                    shieldHealth = 0;
                    hasShield = false; // Shield broken
                }
            }
            
            // Apply remaining damage to health
            if (remainingDamage > 0) {
                health -= remainingDamage;
                damageFlash = 1.0f;
            }
            
            invincibilityTime = PLAYER_INVINCIBILITY_TIME; // Use config value
            
            // Calculate knockback direction (uses config force)
            if (attackDir.lengthSquared() > 0.01f) {
                knockbackVelocity = attackDir.normalize() * PLAYER_KNOCKBACK_FORCE;
            } else if (camera) {
                knockbackVelocity = camera->getForward() * -PLAYER_KNOCKBACK_FORCE;
            }
            knockbackTimer = PLAYER_KNOCKBACK_DURATION; // Use config value
            
            if (health < 0) health = 0;
        }
    }
    
    // Apply camera shake effect (for big impacts like boss kick)
    void applyCameraShake(float intensity, float duration = 0.5f) {
        cameraShakeIntensity = intensity;
        cameraShakeTime = duration;
    }
    
    // Get camera shake offsets for rendering
    float getCameraShakeX() const { return cameraShakeOffsetX; }
    float getCameraShakeY() const { return cameraShakeOffsetY; }
    
    // Lava damage with invincibility frames - player can jump to recover
    void takeLavaDamage(int damage) {
        // Check invincibility powerup
        if (hasInvincibility) return;
        
        // Check lava invincibility frames (uses config value)
        if (lavaInvincibilityTime <= 0) {
            health -= damage;
            damageFlash = 1.0f;
            lavaInvincibilityTime = LAVA_INVINCIBILITY_TIME; // Use config
            
            // Slight upward boost to help player jump out (uses config)
            if (velocity.y < LAVA_KNOCKBACK) {
                velocity.y = LAVA_KNOCKBACK;
            }
            
            if (health < 0) health = 0;
        }
    }
    
    // Update lava state
    void updateLavaState(bool inLava, float deltaTime) {
        isInLava = inLava;
        
        // Update lava invincibility timer
        if (lavaInvincibilityTime > 0) {
            lavaInvincibilityTime -= deltaTime;
            if (lavaInvincibilityTime < 0) lavaInvincibilityTime = 0;
        }
    }
    
    bool isInLavaInvincible() const {
        return lavaInvincibilityTime > 0;
    }
    
    void heal(int amount) {
        health += amount;
        if (health > maxHealth) health = maxHealth;
    }
    
    void addAmmo(int amount) {
        ammo += amount;
        if (ammo > maxAmmo) ammo = maxAmmo;
    }
    
    void setMaxAmmo() {
        ammo = maxAmmo;
    }
    
    void addScore(int points) {
        score += points;
    }
    
    // Powerup activation methods
    void activateSpeedBoost(float duration) {
        hasSpeedBoost = true;
        speedBoostTime = duration;
    }
    
    void activateDamageBoost(float duration) {
        hasDamageBoost = true;
        damageBoostTime = duration;
    }
    
    void activateInvincibility(float duration) {
        hasInvincibility = true;
        invincibilityPowerupTime = duration;
    }
    
    // Activate shield
    void activateShield(float amount) {
        hasShield = true;
        shieldHealth = amount;
        if (shieldHealth > maxShieldHealth) shieldHealth = maxShieldHealth;
    }
    
    // Get damage multiplier for shooting
    float getDamageMultiplier() const {
        return hasDamageBoost ? DAMAGE_BOOST_MULTIPLIER : 1.0f;
    }
    
    // Check if player can take damage (invincibility powerup check)
    bool canTakeDamage() const {
        return !hasInvincibility && invincibilityTime <= 0;
    }
    
    bool isDead() const {
        return health <= 0;
    }
    
    void draw() {
        // Update animation state for animated model
        bool isMoving = moveForward || moveBackward || moveLeft || moveRight;
        bool firing = muzzleFlashTimer > 0;
        LowPolyModels::setPlayerAnimation(isMoving, isSprinting, firing, false);
        
        if (camera && camera->mode == CAMERA_FIRST_PERSON) {
            // Draw weapon in first person - FIXED to screen space
            // Save current matrices
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluPerspective(65.0f, 16.0f/9.0f, 0.01f, 100.0f); // Wider FOV, closer near plane for weapon
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            // Disable depth test temporarily so weapon always renders on top
            glClear(GL_DEPTH_BUFFER_BIT);
            
            // During parkour, draw vault arms instead of weapon
            if (isDoingParkour) {
                LowPolyModels::drawParkourArmsFirstPerson(parkourProgress);
            } else {
                LowPolyModels::drawWeaponFirstPerson(weaponRecoil, weaponBob, firing, weaponLightOn, false);
            }
            
            // Restore matrices
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
        } else {
            // Draw full detailed player model in third person WITH WEAPON IN HANDS
            glPushMatrix();
            
            // Position player at ground level (feet on floor)
            // Player position is at eye height, so subtract player height to get feet position
            float groundY = position.y - PLAYER_HEIGHT;
            glTranslatef(position.x, groundY, position.z);
            
            // NOTE: Procedural player model doesn't have texture coordinates
            // The model uses vertex colors via setColor() in LowPolyModels
            
            // During parkour, draw vault pose instead of normal pose
            if (isDoingParkour) {
                LowPolyModels::drawPlayerParkourPose(rotationY, parkourProgress);
            } else {
                // Draw player with weapon - pass all necessary state for proper sync
                // rotationY syncs player body with camera direction
                // armAimAngle (from camera pitch) syncs weapon aim with where player is looking
                // weaponRecoil and firing sync weapon animation with shooting
                float aimAngle = camera ? camera->getPitch() : 0.0f;
                LowPolyModels::drawPlayer(rotationY, walkPhase, aimAngle, isSprinting,
                                          weaponRecoil, firing, weaponLightOn);
            }
            
            glPopMatrix();
        }
        
        // Draw invincibility effect (flashing)
        if (invincibilityTime > 0) {
            // Player flashes when invincible - handled by alpha
        }
    }
    
    // Get shooting ray direction
    Vector3 getShootDirection() const {
        return camera ? camera->getLookDirection() : Vector3(0, 0, -1);
    }
    
    Vector3 getShootOrigin() const {
        // In third person, shoot from player position (chest height)
        // In first person, shoot from camera/eye position
        if (camera && camera->mode == CAMERA_THIRD_PERSON) {
            // Shoot from player's chest area, slightly forward in aiming direction
            // Use same direction as getLookDirection for consistency
            Vector3 lookDir = camera->getLookDirection();
            return Vector3(
                position.x + lookDir.x * 0.8f,  // Forward from player
                position.y - PLAYER_HEIGHT * 0.3f,  // Chest height
                position.z + lookDir.z * 0.8f
            );
        }
        return camera ? camera->eye : position;
    }
};

#endif // PLAYER_H
