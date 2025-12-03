/**
 * DOOMERS - Animated Player with FBX Support
 * 
 * Player character that uses FBX skeletal animations:
 * - Walking, running, sprinting with proper animation blending
 * - Jumping with up/loop/down phases
 * - Crouching with crouch-walk animations
 * - Death animations from multiple directions
 * - First-person weapon rendering
 * - Third-person full body rendering
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Animation.hpp"
#include "../Engine/Camera.hpp"
#include "../Engine/AssimpLoader.hpp"
#include "GameAssets.hpp"
#include "DamageSystem.hpp"
#include <functional>

namespace Doomers {

// ============================================================================
// Player Animation State
// ============================================================================
enum class AnimState {
    Idle,
    IdleAiming,
    Walking,
    WalkingCrouched,
    Running,
    Sprinting,
    Jumping,
    JumpingLoop,
    JumpingDown,
    Landing,
    Dying,
    Dead
};

// ============================================================================
// Movement Direction (for animation selection)
// ============================================================================
enum class MoveDirection {
    None,
    Forward,
    Backward,
    Left,
    Right,
    ForwardLeft,
    ForwardRight,
    BackwardLeft,
    BackwardRight
};

// ============================================================================
// Animated Player Class
// ============================================================================
class AnimatedPlayer {
public:
    // Transform
    Math::Vector3 position;
    Math::Vector3 velocity;
    float yaw = 0.0f;   // Horizontal rotation (radians)
    float pitch = 0.0f; // Vertical look (radians)
    
    // Physical properties
    float height = 1.8f;
    float crouchHeight = 0.9f;
    float currentHeight = 1.8f;
    float radius = 0.4f;
    float eyeHeightRatio = 0.9f; // Eye at 90% of height
    
    // Movement settings
    float walkSpeed = 5.0f;
    float runSpeed = 8.0f;
    float sprintSpeed = 12.0f;
    float crouchSpeed = 2.5f;
    float jumpForce = 8.0f;
    float gravity = 20.0f;
    
    // Ground/Air state
    bool onGround = true;
    float groundY = 0.0f; // Current ground level
    
    // Input state
    struct {
        bool forward = false;
        bool backward = false;
        bool left = false;
        bool right = false;
        bool jump = false;
        bool crouch = false;
        bool sprint = false;
        bool fire = false;
        bool aim = false;
        bool reload = false;
        float mouseDX = 0.0f;
        float mouseDY = 0.0f;
    } input;
    
    // Animation state
    AnimState animState = AnimState::Idle;
    MoveDirection moveDir = MoveDirection::None;
    CharacterModel* characterModel = nullptr;
    AnimatedModel* weaponModel = nullptr;
    
    // Camera
    bool firstPerson = true;
    float cameraPitch = 0.0f;
    float cameraYaw = 0.0f;
    float mouseSensitivity = 0.002f;
    float tpsCameraDistance = 4.0f;
    float tpsCameraHeight = 2.0f;
    
    // Combat
    Damageable health;
    int ammo = 30;
    int maxAmmo = 30;
    int reserveAmmo = 90;
    bool isReloading = false;
    float reloadTimer = 0.0f;
    float reloadTime = 1.5f;
    float fireTimer = 0.0f;
    float fireRate = 0.15f;
    
    // Stats
    int score = 0;
    int kills = 0;
    
    // Smoothing
    Anim::Spring3D positionSmooth;
    Anim::Spring heightSmooth;
    
    // Flashlight
    bool flashlightOn = true;
    Math::Vector3 flashlightColor = Math::Vector3(1.0f, 0.95f, 0.8f);
    float flashlightIntensity = 1.0f;
    float flashlightAngle = 30.0f;
    float flashlightRange = 30.0f;
    
    // Callbacks
    std::function<void(const Math::Vector3&, const Math::Vector3&)> onShoot;
    std::function<void()> onReloadStart;
    std::function<void()> onReloadEnd;
    std::function<void()> onDeath;
    std::function<void()> onJump;
    std::function<void()> onLand;
    std::function<void(float)> onDamage;
    
    // ========================================================================
    // Constructor
    // ========================================================================
    AnimatedPlayer() {
        // Setup damage system
        health.setMaxHealth(100.0f);
        health.iframeDuration = 0.5f;
        health.knockbackMultiplier = 0.3f;
        
        health.onDamageTaken = [this](const DamageInfo& info) {
            if (onDamage) onDamage(info.amount);
        };
        
        health.onDeath = [this](const DamageInfo& info) {
            die(info.direction);
        };
        
        // Setup springs
        positionSmooth.stiffness = 100.0f;
        positionSmooth.damping = 15.0f;
        
        heightSmooth.stiffness = 50.0f;
        heightSmooth.damping = 10.0f;
    }
    
    // ========================================================================
    // Initialize - Load models
    // ========================================================================
    bool initialize() {
        // Load player character model
        characterModel = GameAssets::instance().loadPlayerModel();
        if (!characterModel) {
            LOG_ERROR("Failed to load player character model");
            return false;
        }
        
        // Load weapon model
        weaponModel = GameAssets::instance().loadWeaponModel();
        if (!weaponModel) {
            LOG_WARN("Failed to load weapon model - will use placeholder");
        }
        
        LOG_INFO("AnimatedPlayer initialized");
        return true;
    }
    
    // ========================================================================
    // Update
    // ========================================================================
    void update(float dt) {
        // Update damage system (i-frames, etc)
        health.update(dt);
        
        // Don't update movement if dead
        if (animState == AnimState::Dead) return;
        
        // Handle camera rotation from mouse
        updateCamera(dt);
        
        // Handle movement
        updateMovement(dt);
        
        // Handle jumping & gravity
        updateJumping(dt);
        
        // Handle crouching
        updateCrouching(dt);
        
        // Handle shooting & reloading
        updateCombat(dt);
        
        // Update animation state
        updateAnimationState(dt);
        
        // Update the character model animation
        if (characterModel) {
            characterModel->update(dt);
        }
        
        // Reset mouse delta
        input.mouseDX = 0;
        input.mouseDY = 0;
    }
    
    // ========================================================================
    // Camera Update
    // ========================================================================
    void updateCamera(float dt) {
        // Apply mouse input to camera
        cameraYaw -= input.mouseDX * mouseSensitivity;
        cameraPitch -= input.mouseDY * mouseSensitivity;
        
        // Clamp pitch
        const float maxPitch = 1.4f; // ~80 degrees
        if (cameraPitch > maxPitch) cameraPitch = maxPitch;
        if (cameraPitch < -maxPitch) cameraPitch = -maxPitch;
        
        // Keep yaw in 0-2PI
        while (cameraYaw > 6.28318f) cameraYaw -= 6.28318f;
        while (cameraYaw < 0) cameraYaw += 6.28318f;
        
        // Player body yaw follows camera
        yaw = cameraYaw;
    }
    
    // ========================================================================
    // Movement Update
    // ========================================================================
    void updateMovement(float dt) {
        // Determine move direction
        int moveX = 0, moveZ = 0;
        if (input.forward) moveZ++;
        if (input.backward) moveZ--;
        if (input.right) moveX++;
        if (input.left) moveX--;
        
        // Calculate move direction enum
        moveDir = MoveDirection::None;
        if (moveZ > 0 && moveX == 0) moveDir = MoveDirection::Forward;
        else if (moveZ < 0 && moveX == 0) moveDir = MoveDirection::Backward;
        else if (moveZ == 0 && moveX > 0) moveDir = MoveDirection::Right;
        else if (moveZ == 0 && moveX < 0) moveDir = MoveDirection::Left;
        else if (moveZ > 0 && moveX > 0) moveDir = MoveDirection::ForwardRight;
        else if (moveZ > 0 && moveX < 0) moveDir = MoveDirection::ForwardLeft;
        else if (moveZ < 0 && moveX > 0) moveDir = MoveDirection::BackwardRight;
        else if (moveZ < 0 && moveX < 0) moveDir = MoveDirection::BackwardLeft;
        
        // Calculate world-space movement direction
        float sinYaw = sinf(yaw);
        float cosYaw = cosf(yaw);
        
        Math::Vector3 forward(sinYaw, 0, cosYaw);
        Math::Vector3 right(cosYaw, 0, -sinYaw);
        
        Math::Vector3 moveVec(0, 0, 0);
        if (input.forward) moveVec = moveVec + forward;
        if (input.backward) moveVec = moveVec - forward;
        if (input.right) moveVec = moveVec + right;
        if (input.left) moveVec = moveVec - right;
        
        // Normalize if moving diagonally
        float len = moveVec.length();
        if (len > 0.01f) {
            moveVec = moveVec / len;
        }
        
        // Determine speed based on state
        float speed = walkSpeed;
        if (input.crouch) {
            speed = crouchSpeed;
        } else if (input.sprint && moveDir != MoveDirection::None) {
            speed = sprintSpeed;
        } else if (moveDir != MoveDirection::None) {
            speed = runSpeed;
        }
        
        // Apply air control
        if (!onGround) {
            speed *= 0.3f;
        }
        
        // Set horizontal velocity
        velocity.x = moveVec.x * speed;
        velocity.z = moveVec.z * speed;
        
        // Apply movement
        position = position + velocity * dt;
    }
    
    // ========================================================================
    // Jumping Update
    // ========================================================================
    void updateJumping(float dt) {
        // Apply gravity
        if (!onGround) {
            velocity.y -= gravity * dt;
        }
        
        // Apply vertical velocity
        position.y += velocity.y * dt;
        
        // Ground check (simple for now - assumes flat ground at groundY)
        if (position.y <= groundY) {
            position.y = groundY;
            if (!onGround && velocity.y < -0.5f) {
                // Landing
                if (onLand) onLand();
            }
            onGround = true;
            velocity.y = 0;
        }
        
        // Jump input
        if (input.jump && onGround && !input.crouch) {
            velocity.y = jumpForce;
            onGround = false;
            if (onJump) onJump();
        }
    }
    
    // ========================================================================
    // Crouching Update
    // ========================================================================
    void updateCrouching(float dt) {
        float targetHeight = input.crouch ? crouchHeight : height;
        
        // Smooth height transition
        heightSmooth.target = targetHeight;
        heightSmooth.update(dt);
        currentHeight = heightSmooth.value;
    }
    
    // ========================================================================
    // Combat Update
    // ========================================================================
    void updateCombat(float dt) {
        // Update timers
        if (fireTimer > 0) fireTimer -= dt;
        
        // Reloading
        if (isReloading) {
            reloadTimer -= dt;
            if (reloadTimer <= 0) {
                // Finish reload
                int ammoNeeded = maxAmmo - ammo;
                int ammoToAdd = std::min(ammoNeeded, reserveAmmo);
                ammo += ammoToAdd;
                reserveAmmo -= ammoToAdd;
                isReloading = false;
                if (onReloadEnd) onReloadEnd();
            }
        }
        
        // Start reload
        if (input.reload && !isReloading && ammo < maxAmmo && reserveAmmo > 0) {
            isReloading = true;
            reloadTimer = reloadTime;
            if (onReloadStart) onReloadStart();
        }
        
        // Shooting
        if (input.fire && fireTimer <= 0 && ammo > 0 && !isReloading) {
            shoot();
        }
    }
    
    // ========================================================================
    // Shooting
    // ========================================================================
    void shoot() {
        ammo--;
        fireTimer = fireRate;
        
        // Calculate shot direction
        Math::Vector3 shotOrigin = getEyePosition();
        Math::Vector3 shotDir = getLookDirection();
        
        if (onShoot) {
            onShoot(shotOrigin, shotDir);
        }
    }
    
    // ========================================================================
    // Animation State Update
    // ========================================================================
    void updateAnimationState(float dt) {
        if (!characterModel) return;
        
        AnimState newState = animState;
        
        // Determine animation state based on movement
        if (animState == AnimState::Dying || animState == AnimState::Dead) {
            return; // Don't change state when dead
        }
        
        if (!onGround) {
            // Jumping states
            if (velocity.y > 0.5f) {
                newState = AnimState::Jumping;
            } else if (velocity.y < -0.5f) {
                newState = AnimState::JumpingDown;
            } else {
                newState = AnimState::JumpingLoop;
            }
        } else if (moveDir == MoveDirection::None) {
            // Standing still
            newState = input.aim ? AnimState::IdleAiming : AnimState::Idle;
        } else if (input.crouch) {
            newState = AnimState::WalkingCrouched;
        } else if (input.sprint) {
            newState = AnimState::Sprinting;
        } else {
            newState = AnimState::Running;
        }
        
        // Apply animation if state changed
        if (newState != animState) {
            animState = newState;
            applyAnimation();
        }
    }
    
    // ========================================================================
    // Apply Animation Based on State
    // ========================================================================
    void applyAnimation() {
        if (!characterModel) return;
        
        const char* animName = PlayerAnimations::IDLE;
        float blendTime = 0.2f;
        
        switch (animState) {
            case AnimState::Idle:
                animName = PlayerAnimations::IDLE;
                break;
            case AnimState::IdleAiming:
                animName = PlayerAnimations::IDLE_AIMING;
                break;
            case AnimState::Walking:
            case AnimState::Running:
                // Select based on direction
                animName = getRunAnimationForDirection();
                break;
            case AnimState::WalkingCrouched:
                animName = getCrouchAnimationForDirection();
                break;
            case AnimState::Sprinting:
                animName = getSprintAnimationForDirection();
                break;
            case AnimState::Jumping:
                animName = PlayerAnimations::JUMP_UP;
                blendTime = 0.1f;
                break;
            case AnimState::JumpingLoop:
                animName = PlayerAnimations::JUMP_LOOP;
                blendTime = 0.1f;
                break;
            case AnimState::JumpingDown:
                animName = PlayerAnimations::JUMP_DOWN;
                blendTime = 0.1f;
                break;
            case AnimState::Dying:
            case AnimState::Dead:
                animName = PlayerAnimations::DEATH_FRONT;
                blendTime = 0.1f;
                break;
            default:
                animName = PlayerAnimations::IDLE;
                break;
        }
        
        characterModel->setAnimation(animName, blendTime);
    }
    
    const char* getRunAnimationForDirection() {
        switch (moveDir) {
            case MoveDirection::Forward: return PlayerAnimations::RUN_FORWARD;
            case MoveDirection::Backward: return PlayerAnimations::RUN_BACKWARD;
            case MoveDirection::Left: return PlayerAnimations::RUN_LEFT;
            case MoveDirection::Right: return PlayerAnimations::RUN_RIGHT;
            case MoveDirection::ForwardLeft: return PlayerAnimations::RUN_FORWARD_LEFT;
            case MoveDirection::ForwardRight: return PlayerAnimations::RUN_FORWARD_RIGHT;
            default: return PlayerAnimations::RUN_FORWARD;
        }
    }
    
    const char* getCrouchAnimationForDirection() {
        switch (moveDir) {
            case MoveDirection::Forward: return PlayerAnimations::WALK_CROUCH_FORWARD;
            case MoveDirection::Backward: return PlayerAnimations::WALK_CROUCH_BACKWARD;
            case MoveDirection::Left: return PlayerAnimations::WALK_CROUCH_LEFT;
            case MoveDirection::Right: return PlayerAnimations::WALK_CROUCH_RIGHT;
            default: return PlayerAnimations::WALK_CROUCH_FORWARD;
        }
    }
    
    const char* getSprintAnimationForDirection() {
        switch (moveDir) {
            case MoveDirection::Forward: return PlayerAnimations::SPRINT_FORWARD;
            case MoveDirection::Backward: return PlayerAnimations::SPRINT_BACKWARD;
            case MoveDirection::Left: return PlayerAnimations::SPRINT_LEFT;
            case MoveDirection::Right: return PlayerAnimations::SPRINT_RIGHT;
            default: return PlayerAnimations::SPRINT_FORWARD;
        }
    }
    
    // ========================================================================
    // Death
    // ========================================================================
    void die(const Math::Vector3& damageDir) {
        animState = AnimState::Dying;
        
        // Choose death animation based on damage direction
        float dotForward = damageDir.dot(getLookDirection());
        if (dotForward > 0) {
            characterModel->setAnimation(PlayerAnimations::DEATH_BACK, 0.1f);
        } else {
            characterModel->setAnimation(PlayerAnimations::DEATH_FRONT, 0.1f);
        }
        
        if (onDeath) onDeath();
    }
    
    // ========================================================================
    // Position / View Helpers
    // ========================================================================
    Math::Vector3 getEyePosition() const {
        return Math::Vector3(position.x, position.y + currentHeight * eyeHeightRatio, position.z);
    }
    
    Math::Vector3 getLookDirection() const {
        float cosPitch = cosf(cameraPitch);
        return Math::Vector3(
            sinf(cameraYaw) * cosPitch,
            sinf(cameraPitch),
            cosf(cameraYaw) * cosPitch
        );
    }
    
    Math::Vector3 getForward() const {
        return Math::Vector3(sinf(yaw), 0, cosf(yaw));
    }
    
    Math::Vector3 getRight() const {
        return Math::Vector3(cosf(yaw), 0, -sinf(yaw));
    }
    
    // ========================================================================
    // Render
    // ========================================================================
    void render() {
        if (firstPerson) {
            renderFirstPerson();
        } else {
            renderThirdPerson();
        }
    }
    
    void renderFirstPerson() {
        // In first person, only render the weapon
        if (weaponModel) {
            glPushMatrix();
            
            // Position weapon in front of camera
            Math::Vector3 eye = getEyePosition();
            Math::Vector3 fwd = getLookDirection();
            Math::Vector3 right = getRight();
            Math::Vector3 up(0, 1, 0);
            
            // Weapon position: offset down and to the right
            Math::Vector3 weaponPos = eye + fwd * 0.3f + right * 0.2f - up * 0.15f;
            
            glTranslatef(weaponPos.x, weaponPos.y, weaponPos.z);
            glRotatef(-cameraYaw * 57.2958f, 0, 1, 0);
            glRotatef(cameraPitch * 57.2958f, 1, 0, 0);
            glScalef(0.01f, 0.01f, 0.01f); // Scale down weapon model
            
            weaponModel->draw();
            
            glPopMatrix();
        }
    }
    
    void renderThirdPerson() {
        // Render full character model
        if (characterModel) {
            glPushMatrix();
            
            glTranslatef(position.x, position.y, position.z);
            glRotatef(-yaw * 57.2958f + 180.0f, 0, 1, 0); // Convert radians to degrees
            glScalef(0.01f, 0.01f, 0.01f); // Scale down character
            
            characterModel->draw();
            
            glPopMatrix();
        }
    }
    
    // ========================================================================
    // Flashlight Rendering
    // ========================================================================
    void setupFlashlight() {
        if (!flashlightOn) return;
        
        // Enable light 1 as spotlight
        glEnable(GL_LIGHT1);
        
        Math::Vector3 pos = getEyePosition();
        Math::Vector3 dir = getLookDirection();
        
        GLfloat lightPos[] = { pos.x, pos.y, pos.z, 1.0f };
        GLfloat lightDir[] = { dir.x, dir.y, dir.z };
        GLfloat diffuse[] = { 
            flashlightColor.x * flashlightIntensity, 
            flashlightColor.y * flashlightIntensity, 
            flashlightColor.z * flashlightIntensity, 
            1.0f 
        };
        GLfloat ambient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
        
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDir);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, flashlightAngle);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 20.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05f);
    }
    
    // ========================================================================
    // Camera Setup (for OpenGL)
    // ========================================================================
    void setupCamera() {
        if (firstPerson) {
            // First person camera
            Math::Vector3 eye = getEyePosition();
            Math::Vector3 target = eye + getLookDirection();
            gluLookAt(
                eye.x, eye.y, eye.z,
                target.x, target.y, target.z,
                0, 1, 0
            );
        } else {
            // Third person camera
            Math::Vector3 target = position + Math::Vector3(0, currentHeight * 0.7f, 0);
            Math::Vector3 camOffset(
                -sinf(cameraYaw) * tpsCameraDistance,
                tpsCameraHeight,
                -cosf(cameraYaw) * tpsCameraDistance
            );
            Math::Vector3 eye = target + camOffset;
            
            gluLookAt(
                eye.x, eye.y, eye.z,
                target.x, target.y, target.z,
                0, 1, 0
            );
        }
    }
    
    // ========================================================================
    // Utility
    // ========================================================================
    bool isAlive() const { return health.isAlive(); }
    float getHealth() const { return health.currentHealth; }
    float getMaxHealth() const { return health.maxHealth; }
    void heal(float amount) { health.heal(amount); }
    void takeDamage(float amount, const Math::Vector3& dir = Math::Vector3()) {
        DamageInfo info;
        info.amount = amount;
        info.direction = dir;
        health.takeDamage(info);
    }
    
    void toggleCameraMode() { firstPerson = !firstPerson; }
    void toggleFlashlight() { flashlightOn = !flashlightOn; }
    
    void respawn(const Math::Vector3& spawnPos) {
        position = spawnPos;
        velocity = Math::Vector3();
        health.fullHeal();
        ammo = maxAmmo;
        isReloading = false;
        animState = AnimState::Idle;
        onGround = true;
    }
};

} // namespace Doomers
