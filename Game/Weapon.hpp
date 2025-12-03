/**
 * DOOMERS - Weapon System
 * 
 * Single gun weapon with flashlight (per requirements)
 * - Sci-fi assault rifle
 * - Attached flashlight
 * - Organic animations using springs
 * - Reload mechanics
 * - Muzzle flash effects
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Animation.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <algorithm>

namespace Doomers {

// ============================================================================
// Weapon State
// ============================================================================
enum class WeaponState {
    Idle,
    Firing,
    Reloading,
    Empty,
    Switching
};

// ============================================================================
// Weapon Stats
// ============================================================================
struct WeaponStats {
    // Damage
    float damage = 25.0f;
    float fireRate = 10.0f;  // Shots per second
    float range = 100.0f;
    float spread = 0.02f;    // Accuracy spread in radians
    
    // Ammo
    int magazineSize = 30;
    int maxReserve = 300;
    float reloadTime = 2.0f;
    
    // Recoil
    float recoilUp = 0.02f;
    float recoilSide = 0.01f;
    float recoilRecovery = 8.0f;
};

// ============================================================================
// Weapon Class (Single Gun with Flashlight)
// ============================================================================
class Weapon {
public:
    std::string name = "Sci-Fi Rifle";
    WeaponStats stats;
    WeaponState state = WeaponState::Idle;
    
    // Ammo
    int currentMag = 30;
    int reserveAmmo = 120;
    
    // Timing
    float fireTimer = 0;
    float reloadTimer = 0;
    float stateTimer = 0;
    
    // Flashlight
    bool flashlightOn = false;
    float flashlightRange = 30.0f;
    float flashlightAngle = 25.0f;  // Degrees
    Math::Color flashlightColor{1.0f, 0.95f, 0.85f, 1.0f};
    float flashlightIntensity = 1.0f;
    
    // Animation springs
    Anim::Spring3D positionOffset;
    Anim::Spring3D rotationOffset;
    Anim::Spring bobSpring;
    Anim::Spring recoilSpring;
    Anim::Spring swaySpring;
    
    // Muzzle flash
    bool showMuzzleFlash = false;
    float muzzleFlashTimer = 0;
    float muzzleFlashDuration = 0.05f;
    
    // Sway parameters
    float swayAmount = 0.02f;
    float swaySpeed = 2.0f;
    float bobAmount = 0.03f;
    float bobSpeed = 10.0f;
    float bobPhase = 0;
    
    Weapon() {
        initSprings();
    }
    
    void initSprings() {
        // Position spring for weapon sway
        positionOffset.stiffness = 200.0f;
        positionOffset.damping = 20.0f;
        
        // Rotation spring for recoil
        rotationOffset.stiffness = 300.0f;
        rotationOffset.damping = 25.0f;
        
        // Bob spring for walking
        bobSpring.stiffness = 150.0f;
        bobSpring.damping = 12.0f;
        
        // Recoil spring
        recoilSpring.stiffness = 400.0f;
        recoilSpring.damping = 30.0f;
        
        // Sway spring
        swaySpring.stiffness = 100.0f;
        swaySpring.damping = 10.0f;
    }
    
    void init() {
        currentMag = stats.magazineSize;
        reserveAmmo = 120;
        state = WeaponState::Idle;
        fireTimer = 0;
        reloadTimer = 0;
    }
    
    void update(float dt, bool moving = false, float moveSpeed = 0) {
        // Update fire cooldown
        if (fireTimer > 0) {
            fireTimer -= dt;
        }
        
        // Update muzzle flash
        if (muzzleFlashTimer > 0) {
            muzzleFlashTimer -= dt;
            if (muzzleFlashTimer <= 0) {
                showMuzzleFlash = false;
            }
        }
        
        // Update reload
        if (state == WeaponState::Reloading) {
            reloadTimer -= dt;
            if (reloadTimer <= 0) {
                finishReload();
            }
        }
        
        // Update animation springs
        positionOffset.update(dt);
        rotationOffset.update(dt);
        recoilSpring.update(dt);
        swaySpring.update(dt);
        bobSpring.update(dt);
        
        // Bob animation when moving
        if (moving) {
            bobPhase += bobSpeed * dt * (moveSpeed / 5.0f);
            float bob = sinf(bobPhase) * bobAmount * moveSpeed;
            bobSpring.target = bob;
        } else {
            bobSpring.target = 0;
        }
        
        // Idle sway
        stateTimer += dt;
        float sway = sinf(stateTimer * swaySpeed) * swayAmount;
        swaySpring.target = sway;
    }
    
    void applyMouseMovement(float dx, float dy) {
        // Add weapon sway based on mouse movement
        positionOffset.target.x = clamp(dx * -0.01f, -0.1f, 0.1f);
        positionOffset.target.y = clamp(dy * 0.01f, -0.1f, 0.1f);
    }
    
    // Returns true if shot was fired
    bool fire() {
        if (state != WeaponState::Idle && state != WeaponState::Firing) {
            return false;
        }
        
        if (fireTimer > 0) {
            return false;
        }
        
        if (currentMag <= 0) {
            state = WeaponState::Empty;
            // Play empty click sound here
            return false;
        }
        
        // Fire!
        currentMag--;
        fireTimer = 1.0f / stats.fireRate;
        state = WeaponState::Firing;
        
        // Apply recoil
        applyRecoil();
        
        // Muzzle flash
        showMuzzleFlash = true;
        muzzleFlashTimer = muzzleFlashDuration;
        
        // Auto-return to idle
        if (currentMag > 0) {
            state = WeaponState::Idle;
        }
        
        return true;
    }
    
    void applyRecoil() {
        // Random recoil within bounds
        float upRecoil = stats.recoilUp * (0.8f + (rand() % 100) / 250.0f);
        float sideRecoil = stats.recoilSide * ((rand() % 100 - 50) / 50.0f);
        
        rotationOffset.velocity.x += upRecoil * 5.0f;
        rotationOffset.velocity.y += sideRecoil * 5.0f;
        
        // Visual kickback
        recoilSpring.velocity += 5.0f;
    }
    
    void startReload() {
        if (state == WeaponState::Reloading) return;
        if (currentMag == stats.magazineSize) return;
        if (reserveAmmo <= 0) return;
        
        state = WeaponState::Reloading;
        reloadTimer = stats.reloadTime;
    }
    
    void finishReload() {
        int needed = stats.magazineSize - currentMag;
        int toLoad = (std::min)(needed, reserveAmmo);
        
        currentMag += toLoad;
        reserveAmmo -= toLoad;
        
        state = (currentMag > 0) ? WeaponState::Idle : WeaponState::Empty;
    }
    
    void toggleFlashlight() {
        flashlightOn = !flashlightOn;
    }
    
    void addAmmo(int amount) {
        reserveAmmo = (std::min)(reserveAmmo + amount, stats.maxReserve);
    }
    
    // Getters
    bool canFire() const {
        return state == WeaponState::Idle && currentMag > 0 && fireTimer <= 0;
    }
    
    bool isReloading() const { return state == WeaponState::Reloading; }
    bool isEmpty() const { return currentMag <= 0 && reserveAmmo <= 0; }
    
    float getReloadProgress() const {
        if (state != WeaponState::Reloading) return 0;
        return 1.0f - (reloadTimer / stats.reloadTime);
    }
    
    int getTotalAmmo() const { return currentMag + reserveAmmo; }
    
    // Get spread direction for bullet
    Math::Vector3 getSpreadOffset() const {
        float angle = (rand() % 1000) / 1000.0f * Math::PI * 2.0f;
        float radius = (rand() % 1000) / 1000.0f * stats.spread;
        return Math::Vector3(
            cosf(angle) * radius,
            sinf(angle) * radius,
            0
        );
    }
    
    // Apply flashlight lighting
    void applyFlashlight(const Math::Vector3& position, const Math::Vector3& direction) {
        if (!flashlightOn) return;
        
        // Set up spotlight
        GLfloat lightPos[] = { position.x, position.y, position.z, 1.0f };
        GLfloat lightDir[] = { direction.x, direction.y, direction.z };
        GLfloat lightColor[] = { 
            flashlightColor.r * flashlightIntensity,
            flashlightColor.g * flashlightIntensity,
            flashlightColor.b * flashlightIntensity,
            1.0f 
        };
        GLfloat ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDir);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
        glLightfv(GL_LIGHT1, GL_SPECULAR, lightColor);
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, flashlightAngle);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 20.0f);
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.01f);
    }
    
    void disableFlashlight() {
        if (!flashlightOn) return;
        glDisable(GL_LIGHT1);
    }
    
    // Draw weapon in first person view
    void draw() {
        glPushMatrix();
        
        // Base weapon position (right side, lower)
        glTranslatef(0.3f, -0.2f, -0.5f);
        
        // Apply spring animations
        glTranslatef(
            positionOffset.current.x + swaySpring.current,
            positionOffset.current.y + bobSpring.current,
            -recoilSpring.current * 0.1f
        );
        
        // Apply rotation animations
        glRotatef(rotationOffset.current.x * 57.2958f, 1, 0, 0);
        glRotatef(rotationOffset.current.y * 57.2958f, 0, 1, 0);
        
        // Draw weapon model (placeholder - replace with OBJ)
        drawWeaponModel();
        
        // Draw muzzle flash
        if (showMuzzleFlash) {
            drawMuzzleFlash();
        }
        
        glPopMatrix();
    }
    
private:
    void drawWeaponModel() {
        // Placeholder weapon geometry
        // Replace with actual OBJ model loading
        
        glColor4f(0.3f, 0.3f, 0.35f, 1.0f);
        
        // Main body
        glPushMatrix();
        glScalef(0.08f, 0.08f, 0.4f);
        drawBox();
        glPopMatrix();
        
        // Barrel
        glPushMatrix();
        glTranslatef(0, 0, -0.25f);
        glScalef(0.03f, 0.03f, 0.2f);
        drawBox();
        glPopMatrix();
        
        // Stock
        glPushMatrix();
        glTranslatef(0, 0.02f, 0.2f);
        glRotatef(-15, 1, 0, 0);
        glScalef(0.06f, 0.1f, 0.15f);
        drawBox();
        glPopMatrix();
        
        // Magazine
        glColor4f(0.2f, 0.2f, 0.25f, 1.0f);
        glPushMatrix();
        glTranslatef(0, -0.1f, 0.05f);
        glScalef(0.04f, 0.12f, 0.08f);
        drawBox();
        glPopMatrix();
        
        // Flashlight attachment
        if (flashlightOn) {
            glColor4f(1.0f, 1.0f, 0.8f, 1.0f);
        } else {
            glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
        }
        glPushMatrix();
        glTranslatef(0.06f, 0, -0.1f);
        glScalef(0.03f, 0.03f, 0.08f);
        drawBox();
        glPopMatrix();
        
        // Scope/sight
        glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
        glPushMatrix();
        glTranslatef(0, 0.06f, 0);
        glScalef(0.02f, 0.03f, 0.1f);
        drawBox();
        glPopMatrix();
    }
    
    void drawMuzzleFlash() {
        // Additive blending for flash
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        
        float intensity = muzzleFlashTimer / muzzleFlashDuration;
        
        glPushMatrix();
        glTranslatef(0, 0, -0.5f);
        
        // Flash sprite
        float size = 0.15f * (0.5f + intensity * 0.5f);
        
        glColor4f(1.0f, 0.9f, 0.5f, intensity);
        
        glBegin(GL_QUADS);
        glVertex3f(-size, -size, 0);
        glVertex3f(size, -size, 0);
        glVertex3f(size, size, 0);
        glVertex3f(-size, size, 0);
        glEnd();
        
        // Secondary flash
        glColor4f(1.0f, 0.6f, 0.2f, intensity * 0.5f);
        size *= 1.5f;
        
        glBegin(GL_QUADS);
        glVertex3f(-size, -size, 0);
        glVertex3f(size, -size, 0);
        glVertex3f(size, size, 0);
        glVertex3f(-size, size, 0);
        glEnd();
        
        glPopMatrix();
        glPopAttrib();
    }
    
    void drawBox() {
        glBegin(GL_QUADS);
        // Front
        glNormal3f(0, 0, -1);
        glVertex3f(-1, -1, -1);
        glVertex3f(1, -1, -1);
        glVertex3f(1, 1, -1);
        glVertex3f(-1, 1, -1);
        
        // Back
        glNormal3f(0, 0, 1);
        glVertex3f(-1, -1, 1);
        glVertex3f(-1, 1, 1);
        glVertex3f(1, 1, 1);
        glVertex3f(1, -1, 1);
        
        // Top
        glNormal3f(0, 1, 0);
        glVertex3f(-1, 1, -1);
        glVertex3f(1, 1, -1);
        glVertex3f(1, 1, 1);
        glVertex3f(-1, 1, 1);
        
        // Bottom
        glNormal3f(0, -1, 0);
        glVertex3f(-1, -1, -1);
        glVertex3f(-1, -1, 1);
        glVertex3f(1, -1, 1);
        glVertex3f(1, -1, -1);
        
        // Right
        glNormal3f(1, 0, 0);
        glVertex3f(1, -1, -1);
        glVertex3f(1, -1, 1);
        glVertex3f(1, 1, 1);
        glVertex3f(1, 1, -1);
        
        // Left
        glNormal3f(-1, 0, 0);
        glVertex3f(-1, -1, -1);
        glVertex3f(-1, 1, -1);
        glVertex3f(-1, 1, 1);
        glVertex3f(-1, -1, 1);
        glEnd();
    }
};

// ============================================================================
// Weapon Inventory (manages single weapon + flashlight)
// ============================================================================
class WeaponInventory {
public:
    Weapon primaryWeapon;
    bool weaponDrawn = true;
    
    // Animation for weapon draw/holster
    Anim::Tween<float> drawTween;
    float weaponVisibility = 1.0f;
    
    WeaponInventory() {
        primaryWeapon.init();
    }
    
    void init() {
        primaryWeapon.init();
        weaponDrawn = true;
        weaponVisibility = 1.0f;
    }
    
    void update(float dt, bool moving = false, float moveSpeed = 0) {
        primaryWeapon.update(dt, moving, moveSpeed);
        
        // Update weapon visibility tween
        drawTween.update(dt);
        if (drawTween.isPlaying()) {
            weaponVisibility = drawTween.getValue();
        }
    }
    
    void applyMouseMovement(float dx, float dy) {
        primaryWeapon.applyMouseMovement(dx, dy);
    }
    
    bool fire() {
        if (!weaponDrawn) return false;
        return primaryWeapon.fire();
    }
    
    void reload() {
        if (weaponDrawn) {
            primaryWeapon.startReload();
        }
    }
    
    void toggleFlashlight() {
        primaryWeapon.toggleFlashlight();
    }
    
    void holsterWeapon() {
        if (weaponDrawn) {
            weaponDrawn = false;
            drawTween.start(weaponVisibility, 0.0f, 0.3f);
        }
    }
    
    void drawWeapon() {
        if (!weaponDrawn) {
            weaponDrawn = true;
            drawTween.start(weaponVisibility, 1.0f, 0.3f);
        }
    }
    
    void toggleWeaponDrawn() {
        if (weaponDrawn) {
            holsterWeapon();
        } else {
            drawWeapon();
        }
    }
    
    void addAmmo(int amount) {
        primaryWeapon.addAmmo(amount);
    }
    
    // Apply flashlight
    void applyFlashlight(const Math::Vector3& position, const Math::Vector3& direction) {
        primaryWeapon.applyFlashlight(position, direction);
    }
    
    void disableFlashlight() {
        primaryWeapon.disableFlashlight();
    }
    
    // Getters
    int getCurrentAmmo() const { return primaryWeapon.currentMag; }
    int getReserveAmmo() const { return primaryWeapon.reserveAmmo; }
    int getTotalAmmo() const { return primaryWeapon.getTotalAmmo(); }
    bool isReloading() const { return primaryWeapon.isReloading(); }
    float getReloadProgress() const { return primaryWeapon.getReloadProgress(); }
    bool isFlashlightOn() const { return primaryWeapon.flashlightOn; }
    bool isWeaponDrawn() const { return weaponDrawn; }
    
    const Weapon& getWeapon() const { return primaryWeapon; }
    Weapon& getWeapon() { return primaryWeapon; }
    
    void draw() {
        if (weaponVisibility < 0.01f) return;
        
        glPushMatrix();
        
        // Weapon bob down when holstered
        float yOffset = (1.0f - weaponVisibility) * -0.5f;
        glTranslatef(0, yOffset, 0);
        
        primaryWeapon.draw();
        
        glPopMatrix();
    }
};

} // namespace Doomers
