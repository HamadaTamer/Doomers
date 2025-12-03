/**
 * DOOMERS - Player
 * 
 * Professional player implementation with:
 * - Full damage system integration
 * - Movement interpolation
 * - Enhanced camera controls
 * - Weapon system
 * - Footsteps & sounds
 * - Death & respawn
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Animation.hpp"
#include "../Engine/Camera.hpp"
#include "DamageSystem.hpp"
#include "Weapon.hpp"
#include <functional>

namespace Doomers {

// ============================================================================
// Player State
// ============================================================================
enum class PlayerState {
    Alive,
    Dying,
    Dead,
    Respawning
};

// ============================================================================
// Movement State
// ============================================================================
enum class MovementState {
    Idle,
    Walking,
    Running,
    Crouching,
    Jumping,
    Falling
};

// ============================================================================
// Enhanced Player Class
// ============================================================================
class EnhancedPlayer {
public:
    // Identity
    std::string name = "Player";
    int playerId = 0;
    
    // Transform
    Math::Vector3 position;
    Math::Vector3 velocity;
    float yaw = 0;
    float pitch = 0;
    
    // Movement interpolation
    Anim::Spring3D positionSpring;
    Anim::Spring3D velocitySmooth;
    
    // Physics
    float height = 1.8f;
    float crouchHeight = 0.9f;
    float currentHeight = 1.8f;
    float radius = 0.4f;
    bool onGround = false;
    float groundCheckDistance = 0.1f;
    
    // Movement stats
    float walkSpeed = 5.0f;
    float runSpeed = 8.0f;
    float crouchSpeed = 2.5f;
    float jumpForce = 8.0f;
    float acceleration = 50.0f;
    float deceleration = 10.0f;
    float airControl = 0.3f;
    float gravity = 20.0f;
    
    // States
    PlayerState state = PlayerState::Alive;
    MovementState movementState = MovementState::Idle;
    
    // Damage system
    Damageable damageable;
    
    // Camera
    EnhancedCamera camera;
    float mouseSensitivity = 0.002f;
    bool invertY = false;
    
    // Weapons
    WeaponInventory weapons;
    
    // Input state
    struct InputState {
        bool forward = false;
        bool backward = false;
        bool left = false;
        bool right = false;
        bool jump = false;
        bool crouch = false;
        bool run = false;
        bool fire = false;
        bool aim = false;
        bool reload = false;
        bool interact = false;
        float mouseX = 0;
        float mouseY = 0;
    } input;
    
    // Footsteps
    float footstepTimer = 0;
    float walkStepInterval = 0.5f;
    float runStepInterval = 0.3f;
    
    // Stats
    int kills = 0;
    int deaths = 0;
    int score = 0;
    
    // Callbacks
    std::function<void()> onDeath;
    std::function<void()> onRespawn;
    std::function<void(const Math::Vector3&, const Math::Vector3&)> onWeaponFire;
    std::function<void(float)> onDamageTaken;
    std::function<void()> onFootstep;
    std::function<void()> onJump;
    std::function<void()> onLand;
    
    EnhancedPlayer() {
        setupDamageSystem();
        setupWeapons();
        setupSprings();
        setupCamera();
    }
    
    void setupDamageSystem() {
        damageable.setMaxHealth(100.0f);
        damageable.iframeDuration = 0.5f;
        damageable.knockbackMultiplier = 0.3f; // Players resist knockback more
        
        damageable.onDamageTaken = [this](const DamageInfo& info) {
            // Camera shake based on damage
            float trauma = info.amount / damageable.maxHealth;
            camera.addDamageShake(trauma);
            
            // Callback
            if (onDamageTaken) onDamageTaken(info.amount);
        };
        
        damageable.onDeath = [this](const DamageInfo& info) {
            die();
        };
    }
    
    void setupWeapons() {
        // Start with pistol
        EnhancedWeapon pistol("Pistol", WeaponType::Pistol);
        float pistolRecoilPitch = pistol.stats.recoilPitch;
        float pistolRecoilYaw = pistol.stats.recoilYaw;
        pistol.onFire = [this, pistolRecoilPitch, pistolRecoilYaw](const Math::Vector3& pos, const Math::Vector3& dir) {
            if (onWeaponFire) {
                Math::Vector3 worldDir = getForwardVector();
                onWeaponFire(getEyePosition(), worldDir);
            }
            camera.addRecoil(pistolRecoilPitch, pistolRecoilYaw);
        };
        weapons.addWeapon(pistol);
        
        // Add assault rifle
        EnhancedWeapon rifle("Assault Rifle", WeaponType::AssaultRifle);
        float rifleRecoilPitch = rifle.stats.recoilPitch;
        float rifleRecoilYaw = rifle.stats.recoilYaw;
        rifle.onFire = [this, rifleRecoilPitch, rifleRecoilYaw](const Math::Vector3& pos, const Math::Vector3& dir) {
            if (onWeaponFire) {
                Math::Vector3 worldDir = getForwardVector();
                onWeaponFire(getEyePosition(), worldDir);
            }
            camera.addRecoil(rifleRecoilPitch, rifleRecoilYaw);
        };
        weapons.addWeapon(rifle);
        
        // Add shotgun
        EnhancedWeapon shotgun("Shotgun", WeaponType::Shotgun);
        int shotgunPellets = shotgun.stats.projectilesPerShot;
        float shotgunRecoilPitch = shotgun.stats.recoilPitch;
        float shotgunRecoilYaw = shotgun.stats.recoilYaw;
        shotgun.onFire = [this, shotgunPellets, shotgunRecoilPitch, shotgunRecoilYaw](const Math::Vector3& pos, const Math::Vector3& dir) {
            if (onWeaponFire) {
                // Shotgun fires multiple pellets
                for (int i = 0; i < shotgunPellets; ++i) {
                    // Use some random spread
                    float spreadX = (float)(rand() % 100 - 50) / 1000.0f;
                    float spreadY = (float)(rand() % 100 - 50) / 1000.0f;
                    Math::Vector3 spreadDir = getForwardVector();
                    spreadDir.x += spreadX;
                    spreadDir.y += spreadY;
                    spreadDir.normalize();
                    onWeaponFire(getEyePosition(), spreadDir);
                }
            }
            camera.shake(0.5f);
            camera.addRecoil(shotgunRecoilPitch, shotgunRecoilYaw);
        };
        weapons.addWeapon(shotgun);
    }
    
    void setupSprings() {
        positionSpring.stiffness = 100.0f;
        positionSpring.damping = 15.0f;
        
        velocitySmooth.stiffness = 200.0f;
        velocitySmooth.damping = 20.0f;
    }
    
    void setupCamera() {
        camera.setFPS(true);
        camera.setFOV(90.0f);
    }
    
    void handleInput(float deltaTime) {
        if (state != PlayerState::Alive) return;
        
        // Mouse look
        float dx = input.mouseX * mouseSensitivity;
        float dy = input.mouseY * mouseSensitivity * (invertY ? 1.0f : -1.0f);
        
        yaw += dx;
        pitch += dy;
        
        // Clamp pitch
        pitch = clamp(pitch, -Math::PI * 0.49f, Math::PI * 0.49f);
        
        // Clear mouse input
        input.mouseX = 0;
        input.mouseY = 0;
        
        // Update camera
        camera.setYaw(yaw);
        camera.setPitch(pitch);
        
        // Movement direction
        Math::Vector3 moveDir(0, 0, 0);
        
        if (input.forward) moveDir.z += 1;
        if (input.backward) moveDir.z -= 1;
        if (input.left) moveDir.x -= 1;
        if (input.right) moveDir.x += 1;
        
        // Normalize
        if (moveDir.length() > 0) {
            moveDir.normalize();
        }
        
        // Rotate by yaw
        float c = cosf(yaw);
        float s = sinf(yaw);
        Math::Vector3 worldDir(
            moveDir.x * c - moveDir.z * s,
            0,
            moveDir.x * s + moveDir.z * c
        );
        
        // Determine speed
        float targetSpeed = walkSpeed;
        if (input.crouch) {
            targetSpeed = crouchSpeed;
            movementState = MovementState::Crouching;
        } else if (input.run && moveDir.length() > 0) {
            targetSpeed = runSpeed;
            movementState = MovementState::Running;
        } else if (moveDir.length() > 0) {
            movementState = MovementState::Walking;
        } else {
            movementState = MovementState::Idle;
        }
        
        // Apply movement
        Math::Vector3 targetVelocity = worldDir * targetSpeed;
        
        float accel = onGround ? acceleration : acceleration * airControl;
        float decel = onGround ? deceleration : deceleration * airControl;
        
        // Interpolate velocity
        if (targetVelocity.length() > 0.01f) {
            velocity.x = lerp(velocity.x, targetVelocity.x, accel * deltaTime);
            velocity.z = lerp(velocity.z, targetVelocity.z, accel * deltaTime);
        } else {
            velocity.x = lerp(velocity.x, 0.0f, decel * deltaTime);
            velocity.z = lerp(velocity.z, 0.0f, decel * deltaTime);
        }
        
        // Jumping
        if (input.jump && onGround) {
            velocity.y = jumpForce;
            onGround = false;
            movementState = MovementState::Jumping;
            if (onJump) onJump();
        }
        
        // Gravity
        if (!onGround) {
            velocity.y -= gravity * deltaTime;
            if (velocity.y < 0) {
                movementState = MovementState::Falling;
            }
        }
        
        // Crouch height
        float targetHeight = input.crouch ? crouchHeight : height;
        currentHeight = lerp(currentHeight, targetHeight, deltaTime * 10.0f);
        
        // Weapon input
        EnhancedWeapon* weapon = weapons.getCurrentWeapon();
        if (weapon) {
            weapon->setADS(input.aim);
            
            if (input.fire) {
                weapon->startFiring();
                weapon->tryFire();
            } else {
                weapon->stopFiring();
            }
            
            if (input.reload) {
                weapon->reload();
            }
        }
    }
    
    void update(float deltaTime) {
        // Get time scale from hit stop
        float timeScale = damageable.hitStop.getTimeScale();
        float scaledDelta = deltaTime * timeScale;
        
        // Update damage system
        damageable.update(scaledDelta);
        
        // State machine
        switch (state) {
            case PlayerState::Alive:
                handleInput(scaledDelta);
                updateMovement(scaledDelta);
                updateWeapons(scaledDelta);
                updateFootsteps(scaledDelta);
                break;
                
            case PlayerState::Dying:
                // Play death animation
                updateDying(scaledDelta);
                break;
                
            case PlayerState::Dead:
                // Wait for respawn
                break;
                
            case PlayerState::Respawning:
                // Play respawn animation
                break;
        }
        
        // Update camera
        updateCamera(scaledDelta);
        
        // Update springs
        positionSpring.update(scaledDelta);
        velocitySmooth.target = velocity;
        velocitySmooth.update(scaledDelta);
    }
    
    void updateMovement(float deltaTime) {
        // Apply knockback offset
        Math::Vector3 knockbackOffset = damageable.getPositionOffset();
        
        // Update position
        Math::Vector3 movement = velocity * deltaTime + knockbackOffset * deltaTime;
        position = position + movement;
        
        // Simple ground check (should use collision)
        if (position.y < 0) {
            if (!onGround && velocity.y < -1.0f) {
                // Landed
                if (onLand) onLand();
                camera.addDamageShake(clamp(-velocity.y / 20.0f, 0.0f, 0.5f));
            }
            position.y = 0;
            velocity.y = 0;
            onGround = true;
        } else if (position.y > 0.01f) {
            onGround = false;
        }
    }
    
    void updateCamera(float deltaTime) {
        // Update camera position
        Math::Vector3 eyePos = getEyePosition();
        camera.setPosition(eyePos);
        
        // Add head bob based on movement
        bool moving = velocity.length() > 0.5f && onGround;
        float bobIntensity = (movementState == MovementState::Running) ? 1.5f : 1.0f;
        camera.setHeadBob(moving, bobIntensity);
        
        // Update camera effects
        camera.update(deltaTime, position, yaw * Math::RAD_TO_DEG);
        
        // ADS FOV
        EnhancedWeapon* weapon = weapons.getCurrentWeapon();
        if (weapon) {
            float targetFOV = weapon->getADS() ? 60.0f : 90.0f;
            camera.setFOV(targetFOV);
        }
    }
    
    void updateWeapons(float deltaTime) {
        bool moving = velocity.length() > 0.5f;
        Math::Vector2 lookDelta(input.mouseX, input.mouseY);
        weapons.update(deltaTime, moving, lookDelta);
    }
    
    void updateFootsteps(float deltaTime) {
        if (!onGround) return;
        
        float speed = Math::Vector3(velocity.x, 0, velocity.z).length();
        if (speed < 0.5f) return;
        
        float interval = (movementState == MovementState::Running) ? runStepInterval : walkStepInterval;
        
        footstepTimer += deltaTime;
        if (footstepTimer >= interval) {
            footstepTimer = 0;
            if (onFootstep) onFootstep();
        }
    }
    
    void updateDying(float deltaTime) {
        // Simple death: camera drops
        static float deathTimer = 0;
        deathTimer += deltaTime;
        
        // Camera falls
        camera.setPitch(lerp(camera.getPitch(), -Math::PI * 0.3f, deltaTime * 3.0f));
        
        if (deathTimer > 2.0f) {
            state = PlayerState::Dead;
            deathTimer = 0;
        }
    }
    
    void die() {
        if (state != PlayerState::Alive) return;
        
        state = PlayerState::Dying;
        deaths++;
        
        if (onDeath) onDeath();
    }
    
    void respawn(const Math::Vector3& spawnPoint) {
        position = spawnPoint;
        velocity = Math::Vector3(0, 0, 0);
        
        damageable.revive();
        
        state = PlayerState::Respawning;
        
        // Quick transition to alive
        state = PlayerState::Alive;
        
        if (onRespawn) onRespawn();
    }
    
    bool takeDamage(DamageInfo& info) {
        return damageable.takeDamage(info);
    }
    
    void heal(float amount) {
        damageable.heal(amount);
    }
    
    // Getters
    Math::Vector3 getEyePosition() const {
        return Math::Vector3(position.x, position.y + currentHeight - 0.1f, position.z);
    }
    
    Math::Vector3 getForwardVector() const {
        return Math::Vector3(
            sinf(yaw) * cosf(pitch),
            sinf(pitch),
            cosf(yaw) * cosf(pitch)
        );
    }
    
    Math::Vector3 getRightVector() const {
        return Math::Vector3(cosf(yaw), 0, -sinf(yaw));
    }
    
    bool isAlive() const { return state == PlayerState::Alive; }
    bool isInvincible() const { return damageable.isInvincible(); }
    float getHealthPercent() const { return damageable.getHealthPercent(); }
    float getHealth() const { return damageable.currentHealth; }
    float getMaxHealth() const { return damageable.maxHealth; }
    
    // Position getter
    Math::Vector3 getPosition() const { return position; }
    
    // Collectible methods - aliases for compatibility
    void collectHealth(float amount) { heal(amount); }
    void collectAmmo(int amount) { weapons.addAmmo(amount); }
    
    void addMouseInput(float dx, float dy) {
        input.mouseX += dx;
        input.mouseY += dy;
    }
    
    // Rendering helpers
    float getRenderAlpha() const {
        return damageable.getRenderAlpha();
    }
    
    Math::Color getFlashColor() const {
        return damageable.getFlashColor();
    }
    
    void drawFirstPerson() {
        // Draw weapon
        glPushMatrix();
        
        // Set up view-space rendering
        glLoadIdentity();
        
        // Apply weapon transform
        weapons.draw();
        
        glPopMatrix();
    }
    
    void drawThirdPerson() {
        // Draw player model (placeholder)
        glPushMatrix();
        
        glTranslatef(position.x, position.y + currentHeight * 0.5f, position.z);
        glRotatef(yaw * 57.2958f, 0, 1, 0);
        
        // Flash effect when damaged
        Math::Color flashCol = getFlashColor();
        float alpha = getRenderAlpha();
        
        if (flashCol.a > 0.01f) {
            glColor4f(flashCol.r, flashCol.g, flashCol.b, alpha);
        } else {
            glColor4f(0.2f, 0.3f, 0.8f, alpha);
        }
        
        // Draw capsule placeholder
        drawPlayerCapsule();
        
        glPopMatrix();
    }
    
private:
    void drawPlayerCapsule() {
        // Simple capsule shape
        float r = radius;
        float h = currentHeight;
        
        // Cylinder body
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 16; ++i) {
            float angle = (float)i / 16.0f * Math::PI * 2.0f;
            float x = cosf(angle) * r;
            float z = sinf(angle) * r;
            glNormal3f(cosf(angle), 0, sinf(angle));
            glVertex3f(x, -h * 0.3f, z);
            glVertex3f(x, h * 0.3f, z);
        }
        glEnd();
    }
};

} // namespace Doomers
