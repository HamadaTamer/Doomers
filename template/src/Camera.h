// ============================================================================
// DOOMERS - Camera.h
// Third-person shooter camera with top-back view
// ============================================================================
#ifndef CAMERA_H
#define CAMERA_H

#include "Vector3.h"
#include "GameConfig.h"
#include <glut.h>
#include <math.h>

class Camera {
public:
    Vector3 eye;        // Camera position
    Vector3 center;     // Look at point
    Vector3 up;         // Up vector
    
    float pitch;        // Vertical rotation (up/down)
    float yaw;          // Horizontal rotation (left/right)
    
    // Third person settings - TOP-BACK VIEW
    float distance;     // Distance behind player
    float height;       // Height above player
    float lookAheadDist; // How far ahead of player to look
    
    // Smooth camera movement
    Vector3 smoothEye;
    Vector3 smoothCenter;
    float smoothSpeed;
    
    // Camera shake for effects
    float shakeIntensity;
    float shakeDuration;
    float shakeTimer;
    
    // Parkour camera tilt
    float parkourTilt;
    
    CameraMode mode;
    
    // Mouse sensitivity
    float sensitivity;
    
    Camera() {
        eye = Vector3(0, 10, 8);
        center = Vector3(0, PLAYER_HEIGHT, 0);
        up = Vector3(0, 1, 0);
        smoothEye = eye;
        smoothCenter = center;
        pitch = -15.0f;  // Slight downward angle - better for aiming
        yaw = 0.0f;
        
        // Third person settings - over-the-shoulder style
        distance = 6.0f;    // Further back so player is always visible
        height = 3.5f;      // Higher for better view
        lookAheadDist = 8.0f; // Look ahead
        
        smoothSpeed = 10.0f;
        mode = CAMERA_THIRD_PERSON; // Default to third person
        sensitivity = 0.12f;
        shakeIntensity = 0.0f;
        shakeDuration = 0.0f;
        shakeTimer = 0.0f;
        parkourTilt = 0.0f;
    }
    
    void setMode(CameraMode newMode) {
        mode = newMode;
    }
    
    // Set parkour camera tilt (for vault animation visual feedback)
    void setParkourTilt(float tilt) {
        parkourTilt = tilt;
    }
    
    void clearParkourTilt() {
        parkourTilt = 0.0f;
    }
    
    void toggleMode() {
        if (mode == CAMERA_FIRST_PERSON) {
            mode = CAMERA_THIRD_PERSON;
        } else {
            mode = CAMERA_FIRST_PERSON;
        }
    }
    
    // Add camera shake effect
    void addShake(float intensity, float duration) {
        shakeIntensity = intensity;
        shakeDuration = duration;
        shakeTimer = duration;
    }
    
    // Rotate camera based on mouse movement
    void rotate(float deltaX, float deltaY) {
        // Different sensitivity for each mode
        float sens = (mode == CAMERA_FIRST_PERSON) ? sensitivity * 1.2f : sensitivity;
        
        yaw += deltaX * sens;
        pitch -= deltaY * sens;
        
        // Clamp pitch based on mode
        if (mode == CAMERA_THIRD_PERSON) {
            if (pitch > 25.0f) pitch = 25.0f;
            if (pitch < -35.0f) pitch = -35.0f;
        } else {
            // First person allows more vertical look range
            if (pitch > 85.0f) pitch = 85.0f;
            if (pitch < -85.0f) pitch = -85.0f;
        }
        
        // Keep yaw in bounds
        if (yaw > 360.0f) yaw -= 360.0f;
        if (yaw < 0.0f) yaw += 360.0f;
    }
    
    // Calculate shake offset
    Vector3 getShakeOffset() {
        if (shakeTimer <= 0) return Vector3(0, 0, 0);
        
        float progress = shakeTimer / shakeDuration;
        float currentIntensity = shakeIntensity * progress;
        
        return Vector3(
            (float)(rand() % 1000 - 500) / 500.0f * currentIntensity,
            (float)(rand() % 1000 - 500) / 500.0f * currentIntensity,
            (float)(rand() % 1000 - 500) / 500.0f * currentIntensity
        );
    }
    
    // Clamp camera position to stay within level boundaries - STRICT ENFORCEMENT
    void clampToBounds(float& x, float& y, float& z) {
        float margin = 3.0f;  // Keep camera well inside walls
        if (x < -BOUNDARY + margin) x = -BOUNDARY + margin;
        if (x > BOUNDARY - margin) x = BOUNDARY - margin;
        if (z < -BOUNDARY + margin) z = -BOUNDARY + margin;
        if (z > BOUNDARY - margin) z = BOUNDARY - margin;
        if (y < 2.0f) y = 2.0f;  // Don't go too low
        if (y > WALL_HEIGHT - 3.0f) y = WALL_HEIGHT - 3.0f;  // Stay well below ceiling
    }
    
    // Update camera based on player position
    void update(const Vector3& playerPos, float playerRotY, float deltaTime = 0.016f) {
        // Update shake timer
        if (shakeTimer > 0) {
            shakeTimer -= deltaTime;
            if (shakeTimer < 0) shakeTimer = 0;
        }
        
        Vector3 shake = getShakeOffset();
        
        if (mode == CAMERA_FIRST_PERSON) {
            // First person - camera at player's eye level
            // Smooth eye position for less jarring movement
            float targetEyeX = playerPos.x;
            float targetEyeY = playerPos.y + PLAYER_HEIGHT * 0.35f;
            float targetEyeZ = playerPos.z;
            
            // Smooth interpolation for less jarring camera
            float fpLerp = 0.5f;
            eye.x += (targetEyeX - eye.x) * fpLerp;
            eye.y += (targetEyeY - eye.y) * fpLerp;
            eye.z += (targetEyeZ - eye.z) * fpLerp;
            
            // Calculate look direction from pitch and yaw
            // Apply parkour tilt to pitch for vault animation visual feedback
            float radYaw = DEG2RAD(yaw);
            float effectivePitch = pitch - parkourTilt; // Tilt down during parkour
            float radPitch = DEG2RAD(effectivePitch);
            
            Vector3 lookDir;
            lookDir.x = cos(radPitch) * sin(radYaw);
            lookDir.y = sin(radPitch);
            lookDir.z = -cos(radPitch) * cos(radYaw);
            
            center = eye + lookDir;
            
            // Apply shake after position is set
            eye = eye + shake * 0.5f; // Reduced shake in first person
        }
        else {
            // OVER-THE-SHOULDER THIRD PERSON VIEW
            float radYaw = DEG2RAD(yaw);
            // Apply parkour tilt to pitch in third person too
            float effectivePitch = pitch - parkourTilt * 0.5f; // Less dramatic in third person
            float radPitch = DEG2RAD(effectivePitch);
            
            // Dynamic height - follows player Y position (for jumping)
            float dynamicHeight = height;
            float dynamicDistance = distance;
            
            // Slight offset to right for over-the-shoulder feel
            float shoulderOffset = 1.0f;
            
            // Ideal camera position: behind and above player, slightly right
            float idealCamX = playerPos.x - sin(radYaw) * dynamicDistance + cos(radYaw) * shoulderOffset;
            float idealCamY = playerPos.y + dynamicHeight;
            float idealCamZ = playerPos.z + cos(radYaw) * dynamicDistance + sin(radYaw) * shoulderOffset;
            
            // Wall/boundary collision - push camera closer if it would go through walls
            float margin = 2.0f;
            float camX = idealCamX;
            float camY = idealCamY;
            float camZ = idealCamZ;
            
            // Check X bounds and adjust distance if needed
            if (camX < -BOUNDARY + margin) {
                float overflow = (-BOUNDARY + margin) - camX;
                camX = -BOUNDARY + margin;
                // Push camera closer to player on this axis
            }
            if (camX > BOUNDARY - margin) {
                camX = BOUNDARY - margin;
            }
            
            // Check Z bounds
            if (camZ < -BOUNDARY + margin) {
                camZ = -BOUNDARY + margin;
            }
            if (camZ > BOUNDARY - margin) {
                camZ = BOUNDARY - margin;
            }
            
            // Height bounds - follow player during jumps
            float maxCamHeight = WALL_HEIGHT - 4.0f;
            float minCamHeight = playerPos.y + 1.5f;  // Always stay above player
            if (camY < minCamHeight) camY = minCamHeight;
            if (camY > maxCamHeight) camY = maxCamHeight;
            
            // Much faster lerp for responsive camera
            float lerpFactor = 0.35f;
            smoothEye.x += (camX - smoothEye.x) * lerpFactor;
            smoothEye.y += (camY - smoothEye.y) * lerpFactor;
            smoothEye.z += (camZ - smoothEye.z) * lerpFactor;
            
            // Clamp smoothed position again
            if (smoothEye.x < -BOUNDARY + margin) smoothEye.x = -BOUNDARY + margin;
            if (smoothEye.x > BOUNDARY - margin) smoothEye.x = BOUNDARY - margin;
            if (smoothEye.z < -BOUNDARY + margin) smoothEye.z = -BOUNDARY + margin;
            if (smoothEye.z > BOUNDARY - margin) smoothEye.z = BOUNDARY - margin;
            if (smoothEye.y < 2.0f) smoothEye.y = 2.0f;
            if (smoothEye.y > maxCamHeight) smoothEye.y = maxCamHeight;
            
            eye = smoothEye + shake;
            
            // Look at player's chest height + pitch-based vertical aim offset
            float lookAheadDist = 6.0f;
            float lookX = playerPos.x + sin(radYaw) * lookAheadDist;
            float lookY = playerPos.y + sin(radPitch) * 3.0f;  // Pitch affects aim point vertically
            float lookZ = playerPos.z - cos(radYaw) * lookAheadDist;
            
            // Smooth center movement
            smoothCenter.x += (lookX - smoothCenter.x) * lerpFactor;
            smoothCenter.y += (lookY - smoothCenter.y) * lerpFactor;
            smoothCenter.z += (lookZ - smoothCenter.z) * lerpFactor;
            
            center = smoothCenter;
        }
    }
    
    // Get forward direction (for movement) - horizontal only
    Vector3 getForward() const {
        float radYaw = DEG2RAD(yaw);
        return Vector3(sin(radYaw), 0, -cos(radYaw)).normalize();
    }
    
    // Get right direction (for strafing)
    Vector3 getRight() const {
        float radYaw = DEG2RAD(yaw);
        return Vector3(cos(radYaw), 0, sin(radYaw)).normalize();
    }
    
    // Get look direction (for shooting)
    Vector3 getLookDirection() const {
        if (mode == CAMERA_THIRD_PERSON) {
            // In third person, shooting direction should go from player toward crosshair
            // The crosshair is at the center of screen, which points to 'center'
            // But we shoot FROM the player, so we need direction from player to far target
            float radYaw = DEG2RAD(yaw);
            float radPitch = DEG2RAD(pitch);
            
            // Direction player is facing (with pitch for vertical aim)
            return Vector3(
                cos(radPitch) * sin(radYaw),
                sin(radPitch),
                -cos(radPitch) * cos(radYaw)
            ).normalize();
        }
        else {
            // First person - direction camera is looking
            float radYaw = DEG2RAD(yaw);
            float radPitch = DEG2RAD(pitch);
            
            return Vector3(
                cos(radPitch) * sin(radYaw),
                sin(radPitch),
                -cos(radPitch) * cos(radYaw)
            ).normalize();
        }
    }
    
    // Apply camera transformation
    void apply() {
        gluLookAt(
            eye.x, eye.y, eye.z,
            center.x, center.y, center.z,
            up.x, up.y, up.z
        );
    }
    
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    void setYaw(float newYaw) { yaw = newYaw; }
    
    // Zoom in/out for third person
    void adjustDistance(float delta) {
        distance += delta;
        if (distance < 3.0f) distance = 3.0f;
        if (distance > 10.0f) distance = 10.0f;
        
        // Height scales with distance
        height = 2.5f + distance * 0.2f;
    }
};

#endif // CAMERA_H
