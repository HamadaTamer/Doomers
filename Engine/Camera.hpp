/**
 * DOOMERS - Camera System
 * 
 * Professional camera with:
 * - Smooth FPS/TPS transitions
 * - Camera shake with multiple layers
 * - Head bob and sway
 * - Recoil effects
 * - Cinematic effects
 * - Spring-based smoothing
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include "Animation.hpp"

namespace Doomers {

// ============================================================================
// Camera Shake Layer - Different shake intensities
// ============================================================================
struct ShakeLayer {
    float trauma;       // 0-1 shake intensity
    float frequency;    // Shake speed
    float decay;        // How fast it decays
    Math::Vector3 offset;
    float rotationOffset;
    
    ShakeLayer(float freq = 20.0f, float dec = 3.0f)
        : trauma(0), frequency(freq), decay(dec), rotationOffset(0) {}
    
    void addTrauma(float amount) {
        trauma = fminf(1.0f, trauma + amount);
    }
    
    void update(float deltaTime, float time) {
        if (trauma > 0) {
            // Perlin-like noise using multiple sine waves
            float shake = trauma * trauma; // Quadratic falloff feels better
            
            float angle = time * frequency;
            offset.x = shake * 0.5f * (sinf(angle * 1.0f) + sinf(angle * 2.3f) * 0.5f);
            offset.y = shake * 0.3f * (sinf(angle * 1.7f) + sinf(angle * 3.1f) * 0.5f);
            offset.z = shake * 0.2f * (sinf(angle * 2.1f) + sinf(angle * 1.3f) * 0.5f);
            rotationOffset = shake * 2.0f * sinf(angle * 1.5f);
            
            trauma -= decay * deltaTime;
            if (trauma < 0) trauma = 0;
        } else {
            offset = Math::Vector3();
            rotationOffset = 0;
        }
    }
};

// ============================================================================
// Enhanced Camera
// ============================================================================
class EnhancedCamera {
public:
    EnhancedCamera()
        : position()
        , targetPosition()
        , smoothPosition()
        , yaw(0), pitch(0)
        , targetYaw(0), targetPitch(0)
        , fov(70.0f), targetFov(70.0f)
        , aspectRatio(16.0f / 9.0f)
        , nearPlane(0.1f), farPlane(500.0f)
        , isFirstPersonMode(true)
        , tpsDistance(4.0f), targetTpsDistance(4.0f)
        , tpsHeight(1.5f)
        , tpsOffset(0.5f)  // Shoulder offset
        , transitionProgress(1.0f)
        , transitionDuration(0.4f)
        , bobTimer(0), bobAmount(0), targetBobAmount(0)
        , swayAmount(0)
        , recoilPitch(0), recoilYaw(0)
        , recoilRecoverySpeed(8.0f)
        , time(0)
        , positionSpring(80.0f, 12.0f)
        , rotationSpring(120.0f, 15.0f)
        , fovSpring(100.0f, 12.0f)
        // Shake layers
        , fireShake(25.0f, 8.0f)
        , damageShakeLayer(15.0f, 4.0f)
        , explosionShakeLayer(8.0f, 2.0f)
    {
        positionVelocity = Math::Vector3();
        rotationVelocity = Math::Vector2();
    }
    
    // ========================================================================
    // Update
    // ========================================================================
    void update(float deltaTime, const Math::Vector3& playerPos, float playerYaw) {
        time += deltaTime;
        
        // Update target position based on camera mode
        if (isFirstPersonMode) {
            targetPosition = playerPos + Math::Vector3(0, GameConstants::PLAYER_EYE_HEIGHT, 0);
        } else {
            // Third person: behind and above player
            float yawRad = playerYaw * Math::DEG_TO_RAD;
            Math::Vector3 offset(
                -sinf(yawRad) * tpsDistance + cosf(yawRad) * tpsOffset,
                tpsHeight,
                cosf(yawRad) * tpsDistance + sinf(yawRad) * tpsOffset
            );
            targetPosition = playerPos + offset;
        }
        
        // Smooth position using spring physics or smooth damp
        smoothPosition = Anim::smoothDamp(smoothPosition, targetPosition, positionVelocity, 
                                          0.05f, 100.0f, deltaTime);
        
        // Apply head bob
        updateHeadBob(deltaTime);
        
        // Update recoil recovery
        if (recoilPitch > 0.01f || recoilPitch < -0.01f) {
            recoilPitch *= (1.0f - recoilRecoverySpeed * deltaTime);
        }
        if (fabsf(recoilYaw) > 0.01f) {
            recoilYaw *= (1.0f - recoilRecoverySpeed * deltaTime);
        }
        
        // Smooth FOV
        fov = Anim::lerp(fov, targetFov, deltaTime * 8.0f);
        
        // Smooth TPS distance
        tpsDistance = Anim::lerp(tpsDistance, targetTpsDistance, deltaTime * 5.0f);
        
        // Update shake layers
        fireShake.update(deltaTime, time);
        damageShakeLayer.update(deltaTime, time);
        explosionShakeLayer.update(deltaTime, time);
        
        // Calculate final position with all offsets
        Math::Vector3 bobOffset(0, bobOffsetY, 0);
        Math::Vector3 shakeOffset = fireShake.offset + damageShakeLayer.offset + explosionShakeLayer.offset;
        
        position = smoothPosition + bobOffset + shakeOffset;
        
        // Apply recoil to view
        float totalPitch = pitch + recoilPitch;
        float totalYaw = yaw + recoilYaw;
        
        // Clamp pitch
        totalPitch = fmaxf(-85.0f, fminf(85.0f, totalPitch));
        
        // Update view vectors
        updateViewVectors(totalYaw, totalPitch);
    }
    
    // ========================================================================
    // Input
    // ========================================================================
    void handleMouseInput(float deltaX, float deltaY, float sensitivity = 0.15f) {
        yaw -= deltaX * sensitivity;
        pitch -= deltaY * sensitivity;
        
        // Clamp pitch
        pitch = fmaxf(-85.0f, fminf(85.0f, pitch));
        
        // Normalize yaw
        while (yaw > 360.0f) yaw -= 360.0f;
        while (yaw < 0.0f) yaw += 360.0f;
    }
    
    // ========================================================================
    // Camera Effects
    // ========================================================================
    void addRecoil(float pitchAmount, float yawAmount = 0) {
        recoilPitch += pitchAmount;
        recoilYaw += yawAmount * ((rand() % 2) * 2 - 1); // Random left/right
        fireShake.addTrauma(0.15f);
    }
    
    void shake(float intensity) {
        fireShake.addTrauma(intensity);
    }
    
    void addDamageShake(float intensity) {
        damageShakeLayer.addTrauma(intensity);
    }
    
    void addExplosionShake(float intensity) {
        explosionShakeLayer.addTrauma(intensity);
    }
    
    void addShake(int type, float intensity) {
        switch(type) {
            case 0: fireShake.addTrauma(intensity); break;  // Fire
            case 1: damageShakeLayer.addTrauma(intensity); break;  // Damage
            case 2: explosionShakeLayer.addTrauma(intensity); break;  // Explosion
        }
    }
    
    void setFPS(bool fps) { isFirstPersonMode = fps; }
    void setFOV(float newFov) { targetFov = newFov; }
    void setPosition(const Math::Vector3& pos) { targetPosition = pos; position = pos; smoothPosition = pos; }
    void setHeadBob(bool enabled, float amount) { targetBobAmount = enabled ? amount : 0.0f; }
    float getPitch() const { return pitch; }
    void setPitch(float p) { pitch = p; }
    
    void setBob(float amount) {
        targetBobAmount = amount;
    }
    
    void setAimDownSights(bool ads) {
        if (ads) {
            targetFov = 50.0f;
        } else {
            targetFov = 70.0f;
        }
    }
    
    // ========================================================================
    // Camera Mode
    // ========================================================================
    void toggleMode() {
        isFirstPersonMode = !isFirstPersonMode;
        transitionProgress = 0;
    }
    
    void setFirstPerson(bool fps) {
        if (isFirstPersonMode != fps) {
            isFirstPersonMode = fps;
            transitionProgress = 0;
        }
    }
    
    bool isFirstPerson() const { return isFirstPersonMode; }
    
    void setTpsDistance(float dist) {
        targetTpsDistance = fmaxf(2.0f, fminf(10.0f, dist));
    }
    
    // ========================================================================
    // Apply to OpenGL
    // ========================================================================
    void applyViewMatrix() const {
        // Apply perspective
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(fov, aspectRatio, nearPlane, farPlane);
        
        // Apply view
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        Math::Vector3 target = position + forward;
        gluLookAt(
            position.x, position.y, position.z,
            target.x, target.y, target.z,
            up.x, up.y, up.z
        );
    }
    
    // ========================================================================
    // Getters
    // ========================================================================
    const Math::Vector3& getPosition() const { return position; }
    const Math::Vector3& getForward() const { return forward; }
    const Math::Vector3& getRight() const { return right; }
    const Math::Vector3& getUp() const { return up; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getFOV() const { return fov; }
    float getAspectRatio() const { return aspectRatio; }
    
    void setAspectRatio(float ratio) { aspectRatio = ratio; }
    void setYaw(float y) { yaw = y; }
    void setPitch(float p) { pitch = fmaxf(-85.0f, fminf(85.0f, p)); }
    
    // Get muzzle position for weapon effects
    Math::Vector3 getMuzzlePosition() const {
        return position + forward * 0.5f + right * 0.2f - up * 0.1f;
    }
    
private:
    void updateViewVectors(float totalYaw, float totalPitch) {
        float yawRad = totalYaw * Math::DEG_TO_RAD;
        float pitchRad = totalPitch * Math::DEG_TO_RAD;
        
        forward.x = sinf(yawRad) * cosf(pitchRad);
        forward.y = sinf(pitchRad);
        forward.z = -cosf(yawRad) * cosf(pitchRad);
        forward.normalize();
        
        Math::Vector3 worldUp(0, 1, 0);
        right = forward.cross(worldUp);
        right.normalize();
        
        up = right.cross(forward);
        up.normalize();
    }
    
    void updateHeadBob(float deltaTime) {
        // Smooth bob amount
        bobAmount = Anim::lerp(bobAmount, targetBobAmount, deltaTime * 5.0f);
        
        if (bobAmount > 0.01f) {
            bobTimer += deltaTime * (bobAmount > 0.3f ? 12.0f : 8.0f);
            
            // Vertical bob (up and down)
            bobOffsetY = sinf(bobTimer * 2.0f) * bobAmount * 0.05f;
            
            // Horizontal sway
            bobOffsetX = sinf(bobTimer) * bobAmount * 0.02f;
        } else {
            // Smooth return to center
            bobOffsetY = Anim::lerp(bobOffsetY, 0.0f, deltaTime * 8.0f);
            bobOffsetX = Anim::lerp(bobOffsetX, 0.0f, deltaTime * 8.0f);
        }
    }
    
    // Position
    Math::Vector3 position;
    Math::Vector3 targetPosition;
    Math::Vector3 smoothPosition;
    Math::Vector3 positionVelocity;
    
    // Rotation
    float yaw, pitch;
    float targetYaw, targetPitch;
    Math::Vector2 rotationVelocity;
    
    // View vectors
    Math::Vector3 forward, right, up;
    
    // Projection
    float fov, targetFov;
    float aspectRatio;
    float nearPlane, farPlane;
    
    // Camera mode
    bool isFirstPersonMode;
    float tpsDistance, targetTpsDistance;
    float tpsHeight;
    float tpsOffset;
    float transitionProgress;
    float transitionDuration;
    
    // Head bob
    float bobTimer;
    float bobAmount, targetBobAmount;
    float bobOffsetX, bobOffsetY;
    float swayAmount;
    
    // Recoil
    float recoilPitch, recoilYaw;
    float recoilRecoverySpeed;
    
    // Time
    float time;
    
    // Springs for smooth movement
    Anim::Spring3D positionSpring;
    Anim::Spring rotationSpring;
    Anim::Spring fovSpring;
    
    // Shake layers
    ShakeLayer fireShake;
    ShakeLayer damageShakeLayer;
    ShakeLayer explosionShakeLayer;
};

} // namespace Doomers
