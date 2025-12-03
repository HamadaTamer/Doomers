/**
 * DOOMERS - Gun System
 * 
 * Single gun with flashlight as per description:
 * - Gun recoil animation (rotation + translation)
 * - Muzzle flash light animation
 * - Flashlight that follows camera direction
 * 
 * Uses ORGANIC procedural animations (not robotic):
 * - Spring physics for natural overshoot/settle
 * - Perlin-like noise for subtle variations
 * - Secondary motion with delay
 * - Layered animations combined
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Animation.hpp"
#include <cmath>

namespace Doomers {

// ============================================================================
// Organic Motion Helpers - Make animations feel alive, not robotic
// ============================================================================
namespace OrganicMotion {
    
    // Simple noise function (pseudo-random but smooth)
    inline float noise1D(float x) {
        int xi = (int)floorf(x);
        float xf = x - xi;
        
        // Hash function
        auto hash = [](int n) -> float {
            n = (n << 13) ^ n;
            return 1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
        };
        
        // Smooth interpolation
        float t = xf * xf * (3.0f - 2.0f * xf); // Smoothstep
        return hash(xi) * (1.0f - t) + hash(xi + 1) * t;
    }
    
    // 2D noise for more complex motion
    inline float noise2D(float x, float y) {
        return (noise1D(x + y * 31.7f) + noise1D(y + x * 17.3f)) * 0.5f;
    }
    
    // Breathing motion - subtle periodic variation
    inline float breathing(float time, float rate = 1.0f) {
        return sinf(time * rate * Math::PI * 2.0f) * 0.5f + 0.5f;
    }
    
    // Micro tremor - tiny hand shake
    inline Math::Vector3 microTremor(float time, float intensity = 0.001f) {
        return Math::Vector3(
            noise1D(time * 15.0f) * intensity,
            noise1D(time * 15.0f + 100.0f) * intensity,
            noise1D(time * 15.0f + 200.0f) * intensity * 0.5f
        );
    }
    
    // Natural sway - like holding something with your arms
    inline Math::Vector3 naturalSway(float time, float walkSpeed = 0.0f) {
        float breathe = breathing(time, 0.3f);
        float idleSway = sinf(time * 0.7f) * 0.002f;
        
        return Math::Vector3(
            idleSway + noise1D(time * 0.5f) * 0.001f,
            breathe * 0.003f + noise1D(time * 0.4f + 50.0f) * 0.001f,
            0
        );
    }
}

// ============================================================================
// Spring3D with Damping - For organic recoil recovery
// ============================================================================
class OrganicSpring3D {
public:
    Math::Vector3 current;
    Math::Vector3 target;
    Math::Vector3 velocity;
    
    float stiffness = 150.0f;  // How fast it returns
    float damping = 12.0f;     // How much it overshoots (lower = more bouncy)
    
    void update(float dt) {
        // Spring force
        Math::Vector3 force = (target - current) * stiffness;
        // Damping force
        Math::Vector3 dampForce = velocity * damping;
        // Acceleration
        Math::Vector3 accel = force - dampForce;
        
        velocity = velocity + accel * dt;
        current = current + velocity * dt;
    }
    
    void impulse(const Math::Vector3& force) {
        velocity = velocity + force;
    }
    
    void setImmediate(const Math::Vector3& pos) {
        current = pos;
        target = pos;
        velocity = Math::Vector3();
    }
};

// ============================================================================
// Muzzle Flash - Brief intense light at gun barrel
// ============================================================================
struct MuzzleFlash {
    bool active = false;
    float timer = 0;
    float duration = 0.05f;  // Very brief flash
    float intensity = 1.0f;
    Math::Color color = Math::Color(1.0f, 0.8f, 0.3f);  // Orange-yellow
    
    void trigger() {
        active = true;
        timer = 0;
        intensity = 1.0f;
    }
    
    void update(float dt) {
        if (!active) return;
        
        timer += dt;
        // Quick flash then fade
        intensity = 1.0f - (timer / duration);
        intensity = fmaxf(0.0f, intensity);
        
        if (timer >= duration) {
            active = false;
            intensity = 0;
        }
    }
    
    void render(const Math::Vector3& position) {
        if (!active || intensity <= 0) return;
        
        // Set up point light for muzzle flash
        GLfloat lightPos[] = { position.x, position.y, position.z, 1.0f };
        GLfloat lightColor[] = { 
            color.r * intensity * 2.0f, 
            color.g * intensity * 2.0f, 
            color.b * intensity * 2.0f, 
            1.0f 
        };
        GLfloat lightAtten[] = { 1.0f, 0.1f, 0.05f };
        
        glEnable(GL_LIGHT2);
        glLightfv(GL_LIGHT2, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor);
        glLightfv(GL_LIGHT2, GL_SPECULAR, lightColor);
        glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, lightAtten[0]);
        glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, lightAtten[1]);
        glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, lightAtten[2]);
        
        // Also render a visual flash sprite
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending
        
        glColor4f(color.r, color.g, color.b, intensity);
        
        // Simple billboard quad
        float size = 0.15f * intensity;
        glBegin(GL_TRIANGLES);
        // Draw a simple star shape
        for (int i = 0; i < 8; ++i) {
            float angle1 = (float)i / 8.0f * Math::PI * 2.0f;
            float angle2 = (float)(i + 1) / 8.0f * Math::PI * 2.0f;
            glVertex3f(0, 0, 0);
            glVertex3f(cosf(angle1) * size, sinf(angle1) * size, 0);
            glVertex3f(cosf(angle2) * size, sinf(angle2) * size, 0);
        }
        glEnd();
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
    
    void disable() {
        if (!active) {
            glDisable(GL_LIGHT2);
        }
    }
};

// ============================================================================
// Flashlight - Spot light attached to gun, follows camera
// ============================================================================
class Flashlight {
public:
    bool enabled = true;
    float intensity = 1.0f;
    float flickerTimer = 0;
    bool flickering = false;  // Horror effect in dark areas
    
    Math::Color color = Math::Color(0.95f, 0.95f, 0.85f);  // Slightly warm white
    float spotCutoff = 25.0f;      // Cone angle
    float spotExponent = 30.0f;    // Focus
    float range = 50.0f;
    
    void update(float dt, bool inDarkArea = false) {
        // Flicker effect in dark areas for horror vibe
        if (inDarkArea && enabled) {
            flickerTimer += dt;
            if (flickerTimer > 0.1f) {
                flickerTimer = 0;
                // Random flicker
                flickering = (rand() % 100) < 5;  // 5% chance
            }
            
            if (flickering) {
                intensity = 0.6f + OrganicMotion::noise1D(dt * 100.0f) * 0.4f;
            } else {
                intensity = 1.0f;
            }
        } else {
            intensity = 1.0f;
            flickering = false;
        }
    }
    
    void render(const Math::Vector3& position, const Math::Vector3& direction) {
        if (!enabled) {
            glDisable(GL_LIGHT1);
            return;
        }
        
        glEnable(GL_LIGHT1);
        
        float actualIntensity = intensity * (flickering ? 0.7f : 1.0f);
        
        GLfloat lightPos[] = { position.x, position.y, position.z, 1.0f };
        GLfloat lightDir[] = { direction.x, direction.y, direction.z };
        GLfloat lightDiffuse[] = { 
            color.r * actualIntensity, 
            color.g * actualIntensity, 
            color.b * actualIntensity, 
            1.0f 
        };
        GLfloat lightAmbient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
        
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDir);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT1, GL_SPECULAR, lightDiffuse);
        glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
        
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spotCutoff);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spotExponent);
        
        // Attenuation for range
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.02f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.001f);
    }
};

// ============================================================================
// Gun - Single weapon with organic animations
// ============================================================================
class Gun {
public:
    // Ammo
    int currentAmmo = 30;
    int maxAmmo = 30;
    int reserveAmmo = 90;
    
    // Firing
    float fireRate = 8.0f;  // Rounds per second
    float fireTimer = 0;
    bool canFire = true;
    
    // Damage
    float damage = 25.0f;
    float range = 100.0f;
    
    // Position relative to camera
    Math::Vector3 basePosition = Math::Vector3(0.25f, -0.2f, 0.5f);
    Math::Vector3 currentPosition;
    Math::Vector3 currentRotation;  // Euler angles for display
    
    // Organic animation components
    OrganicSpring3D positionSpring;
    OrganicSpring3D rotationSpring;
    
    // Animation state
    float animTime = 0;
    float walkCycleTime = 0;
    bool isWalking = false;
    float walkSpeed = 0;
    
    // Recoil settings (per description: rotation + translation)
    Math::Vector3 recoilTranslation = Math::Vector3(0, 0.02f, -0.08f);
    Math::Vector3 recoilRotation = Math::Vector3(-0.15f, 0.02f, 0.05f);  // Pitch up, slight yaw/roll
    
    // Effects
    MuzzleFlash muzzleFlash;
    Flashlight flashlight;
    
    // Reload
    bool isReloading = false;
    float reloadTimer = 0;
    float reloadDuration = 1.5f;
    
    Gun() {
        currentPosition = basePosition;
        positionSpring.current = basePosition;
        positionSpring.target = basePosition;
        positionSpring.stiffness = 200.0f;
        positionSpring.damping = 15.0f;
        
        rotationSpring.stiffness = 180.0f;
        rotationSpring.damping = 14.0f;
    }
    
    void update(float dt, bool walking, float playerWalkSpeed, const Math::Vector3& cameraDir) {
        animTime += dt;
        fireTimer -= dt;
        if (fireTimer < 0) fireTimer = 0;
        
        isWalking = walking;
        walkSpeed = playerWalkSpeed;
        
        // Update reload
        if (isReloading) {
            reloadTimer += dt;
            if (reloadTimer >= reloadDuration) {
                finishReload();
            }
        }
        
        // Update springs (organic motion)
        positionSpring.update(dt);
        rotationSpring.update(dt);
        
        // Calculate final position with organic overlays
        Math::Vector3 organicOffset(0, 0, 0);
        Math::Vector3 organicRotation(0, 0, 0);
        
        // 1. Micro tremor (tiny hand shake - always present)
        organicOffset = organicOffset + OrganicMotion::microTremor(animTime, 0.0008f);
        
        // 2. Breathing sway (subtle up/down)
        organicOffset.y += OrganicMotion::breathing(animTime, 0.25f) * 0.004f - 0.002f;
        
        // 3. Idle sway (side to side)
        if (!isWalking) {
            Math::Vector3 sway = OrganicMotion::naturalSway(animTime);
            organicOffset = organicOffset + sway;
            organicRotation.z = sinf(animTime * 0.5f) * 0.01f;
        }
        
        // 4. Walk bob (if walking)
        if (isWalking && !isReloading) {
            walkCycleTime += dt * walkSpeed * 0.5f;
            
            // Figure-8 pattern for realistic walk
            float bobX = sinf(walkCycleTime * 2.0f) * 0.015f;
            float bobY = fabsf(sinf(walkCycleTime)) * 0.02f - 0.01f;
            
            organicOffset.x += bobX;
            organicOffset.y += bobY;
            
            // Slight rotation with walk
            organicRotation.z = sinf(walkCycleTime * 2.0f) * 0.02f;
        }
        
        // 5. Reload animation
        if (isReloading) {
            float t = reloadTimer / reloadDuration;
            // Down, rotate, up motion
            float reloadCurve;
            if (t < 0.3f) {
                // Move down
                reloadCurve = Anim::Ease::OutQuad(t / 0.3f);
                organicOffset.y -= reloadCurve * 0.15f;
                organicRotation.x = reloadCurve * 0.3f;
            } else if (t < 0.7f) {
                // Hold down
                organicOffset.y -= 0.15f;
                organicRotation.x = 0.3f;
            } else {
                // Move up
                reloadCurve = Anim::Ease::OutBack((t - 0.7f) / 0.3f);
                organicOffset.y -= (1.0f - reloadCurve) * 0.15f;
                organicRotation.x = (1.0f - reloadCurve) * 0.3f;
            }
        }
        
        // Combine spring physics with organic motion
        currentPosition = positionSpring.current + organicOffset;
        currentRotation = rotationSpring.current + organicRotation;
        
        // Reset spring target to base
        positionSpring.target = basePosition;
        rotationSpring.target = Math::Vector3();
        
        // Update effects
        muzzleFlash.update(dt);
        flashlight.update(dt);
    }
    
    bool fire() {
        if (!canFire || fireTimer > 0 || currentAmmo <= 0 || isReloading) {
            return false;
        }
        
        currentAmmo--;
        fireTimer = 1.0f / fireRate;
        
        // Apply recoil through spring impulse (organic, not instant)
        positionSpring.impulse(recoilTranslation * 15.0f);
        rotationSpring.impulse(recoilRotation * 8.0f);
        
        // Trigger muzzle flash
        muzzleFlash.trigger();
        
        // Auto reload if empty
        if (currentAmmo <= 0 && reserveAmmo > 0) {
            startReload();
        }
        
        return true;
    }
    
    void startReload() {
        if (isReloading || currentAmmo >= maxAmmo || reserveAmmo <= 0) return;
        
        isReloading = true;
        reloadTimer = 0;
    }
    
    void finishReload() {
        isReloading = false;
        int needed = maxAmmo - currentAmmo;
        int available = (reserveAmmo >= needed) ? needed : reserveAmmo;
        currentAmmo += available;
        reserveAmmo -= available;
    }
    
    void addAmmo(int amount) {
        reserveAmmo += amount;
    }
    
    Math::Vector3 getMuzzlePosition(const Math::Vector3& cameraPos, const Math::Vector3& cameraForward,
                                     const Math::Vector3& cameraRight, const Math::Vector3& cameraUp) {
        // Muzzle is at front of gun
        Math::Vector3 localMuzzle = currentPosition + Math::Vector3(0, 0.05f, 0.3f);
        return cameraPos 
             + cameraRight * localMuzzle.x 
             + cameraUp * localMuzzle.y 
             + cameraForward * localMuzzle.z;
    }
    
    void render(const Math::Vector3& cameraPos, const Math::Vector3& cameraForward,
                const Math::Vector3& cameraRight, const Math::Vector3& cameraUp) {
        glPushMatrix();
        
        // Position relative to camera
        Math::Vector3 worldPos = cameraPos 
                               + cameraRight * currentPosition.x 
                               + cameraUp * currentPosition.y 
                               + cameraForward * currentPosition.z;
        
        glTranslatef(worldPos.x, worldPos.y, worldPos.z);
        
        // Apply rotation (yaw, pitch, roll)
        glRotatef(currentRotation.y * 180.0f / Math::PI, 0, 1, 0);
        glRotatef(currentRotation.x * 180.0f / Math::PI, 1, 0, 0);
        glRotatef(currentRotation.z * 180.0f / Math::PI, 0, 0, 1);
        
        // Draw gun model (placeholder - box shape)
        glColor3f(0.2f, 0.2f, 0.25f);
        
        // Gun body
        glPushMatrix();
        glScalef(0.04f, 0.06f, 0.25f);
        drawCube();
        glPopMatrix();
        
        // Gun handle
        glPushMatrix();
        glTranslatef(0, -0.05f, -0.05f);
        glRotatef(-20, 1, 0, 0);
        glScalef(0.03f, 0.08f, 0.04f);
        glColor3f(0.15f, 0.1f, 0.05f);  // Brown grip
        drawCube();
        glPopMatrix();
        
        // Gun barrel
        glPushMatrix();
        glTranslatef(0, 0.01f, 0.15f);
        glScalef(0.02f, 0.02f, 0.12f);
        glColor3f(0.1f, 0.1f, 0.12f);
        drawCube();
        glPopMatrix();
        
        // Flashlight attachment
        glPushMatrix();
        glTranslatef(0, -0.02f, 0.1f);
        glScalef(0.025f, 0.025f, 0.06f);
        glColor3f(0.3f, 0.3f, 0.35f);
        drawCube();
        glPopMatrix();
        
        glPopMatrix();
        
        // Render muzzle flash
        Math::Vector3 muzzlePos = getMuzzlePosition(cameraPos, cameraForward, cameraRight, cameraUp);
        muzzleFlash.render(muzzlePos);
        muzzleFlash.disable();
        
        // Render flashlight
        flashlight.render(muzzlePos, cameraForward);
    }
    
private:
    void drawCube() {
        glBegin(GL_QUADS);
        // Front
        glNormal3f(0, 0, 1);
        glVertex3f(-1, -1, 1); glVertex3f(1, -1, 1); glVertex3f(1, 1, 1); glVertex3f(-1, 1, 1);
        // Back
        glNormal3f(0, 0, -1);
        glVertex3f(1, -1, -1); glVertex3f(-1, -1, -1); glVertex3f(-1, 1, -1); glVertex3f(1, 1, -1);
        // Top
        glNormal3f(0, 1, 0);
        glVertex3f(-1, 1, -1); glVertex3f(-1, 1, 1); glVertex3f(1, 1, 1); glVertex3f(1, 1, -1);
        // Bottom
        glNormal3f(0, -1, 0);
        glVertex3f(-1, -1, 1); glVertex3f(-1, -1, -1); glVertex3f(1, -1, -1); glVertex3f(1, -1, 1);
        // Right
        glNormal3f(1, 0, 0);
        glVertex3f(1, -1, 1); glVertex3f(1, -1, -1); glVertex3f(1, 1, -1); glVertex3f(1, 1, 1);
        // Left
        glNormal3f(-1, 0, 0);
        glVertex3f(-1, -1, -1); glVertex3f(-1, -1, 1); glVertex3f(-1, 1, 1); glVertex3f(-1, 1, -1);
        glEnd();
    }
};

} // namespace Doomers
