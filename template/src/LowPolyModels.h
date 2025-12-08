// ============================================================================
// DOOMERS - LowPolyModels.h
// Main wrapper - includes all modular model files
// ============================================================================
#ifndef LOW_POLY_MODELS_H
#define LOW_POLY_MODELS_H

// Include all model modules
#include "models/ModelUtils.h"
#include "models/PlayerModel.h"
#include "models/WeaponModel.h"
#include "models/EnemyModels.h"
#include "models/EnvironmentModels.h"
#include "models/EffectsModels.h"
#include "ModelLoader.h"  // For AR gun 3D model

// ============================================================================
// UNIFIED INTERFACE - For backwards compatibility and ease of use
// ============================================================================
namespace LowPolyModels {
    
    using namespace ModelUtils;
    
    // -------------------- TIME UPDATE --------------------
    inline void updateAnimationTime(float deltaTime) {
        ModelUtils::updateTime(deltaTime);
    }
    
    // -------------------- PLAYER --------------------
    inline void drawPlayer(float rotY, float walkPhase, float armAimAngle, bool isRunning,
                          float weaponRecoil = 0.0f, bool isFiring = false, bool weaponLightOn = true) {
        PlayerModel::drawPlayerDetailed(rotY, walkPhase, armAimAngle, isRunning, weaponRecoil, isFiring, weaponLightOn);
    }
    
    // Set player animation state (no-op for procedural)
    inline void setPlayerAnimation(bool isMoving, bool isRunning, bool isShooting, bool isReloading) {
        // Procedural model doesn't use this - animations are handled via walkPhase
    }
    
    // -------------------- WEAPON --------------------
    inline void drawWeapon(float recoil, bool firing, bool weaponLightOn, float flashlightIntensity) {
        // Try to use the AR 3D model first
        if (ModelLoader::isLoaded(MODEL_AR_GUN)) {
            glPushMatrix();
            // Apply recoil
            glTranslatef(0, 0, -recoil * 0.1f);  // Recoil moves gun backward (-Z)
            glRotatef(recoil * 5, 1, 0, 0);
            // Scale and orient the AR model properly
            // Model imports with different axes - rotate to point barrel FORWARD (-Z in world)
            glRotatef(180, 0, 1, 0);  // Turn around to face forward
            glScalef(0.8f, 0.8f, 0.8f);  // Scale down slightly
            ModelLoader::draw(MODEL_AR_GUN, 1.0f);
            glPopMatrix();
        } else {
            // Fall back to procedural weapon
            WeaponModel::drawAssaultRifleDetailed(recoil, firing, weaponLightOn ? flashlightIntensity : 0.0f);
        }
    }
    
    inline void drawWeaponFirstPerson(float recoil, float bob, bool firing, bool weaponLightOn, bool aimDownSights) {
        // Try to use the AR 3D model for first person too
        if (ModelLoader::isLoaded(MODEL_AR_GUN)) {
            glPushMatrix();
            // Position for first person view - right side, down, forward
            glTranslatef(0.25f, -0.20f, -0.5f);
            // Apply bob and recoil
            float bobY = sin(bob * 2) * 0.01f;
            float bobX = cos(bob) * 0.005f;
            glTranslatef(bobX, bobY - recoil * 0.02f, recoil * 0.08f);  // Recoil pushes back (+Z toward camera)
            glRotatef(recoil * 3, 1, 0, 0);  // Tilt up from recoil
            // Orient model - barrel points FORWARD (-Z)
            glRotatef(180, 0, 1, 0);  // Turn around to face forward
            glScalef(1.0f, 1.0f, 1.0f);  // Normal scale
            ModelLoader::draw(MODEL_AR_GUN, 1.0f);
            glPopMatrix();
        } else {
            WeaponModel::drawWeaponFirstPerson(recoil, bob, firing, weaponLightOn ? 1.0f : 0.0f);
        }
    }
    
    inline void drawArmsFirstPerson(float recoil, float bob) {
        // Arms are drawn as part of first person weapon
    }
    
    // First person parkour vault arms animation
    inline void drawParkourArmsFirstPerson(float parkourProgress) {
        PlayerModel::drawParkourArmsFirstPerson(parkourProgress);
    }
    
    // Third person parkour vault pose
    inline void drawPlayerParkourPose(float rotY, float parkourProgress) {
        PlayerModel::drawPlayerParkourPose(rotY, parkourProgress);
    }
    
    // -------------------- ENEMIES --------------------
    inline void drawZombie(float rotY, float animPhase, float health, float maxHealth, float attackPhase = 0.0f) {
        EnemyModels::drawZombieDetailed(rotY, animPhase, health, maxHealth, attackPhase);
    }
    
    inline void drawDemon(float rotY, float animPhase, float attackPhase) {
        EnemyModels::drawDemonDetailed(rotY, animPhase, attackPhase);
    }
    
    inline void drawBoss(float rotY, float animPhase, float health, float maxHealth, bool isEnraged) {
        EnemyModels::drawBossDetailed(rotY, animPhase, health, maxHealth);
    }
    
    // -------------------- ENVIRONMENT --------------------
    inline void drawCrate(float size = 1.0f) {
        EnvironmentModels::drawCrate(size);
    }
    
    inline void drawSciFiCrate(float size = 1.0f) {
        EnvironmentModels::drawSciFiCrate(size);
    }
    
    inline void drawHealthPack() {
        EnvironmentModels::drawHealthPack();
    }
    
    inline void drawAmmoBox() {
        EnvironmentModels::drawAmmoBox();
    }
    
    inline void drawKeycard(float r, float g, float b) {
        EnvironmentModels::drawKeycard(r, g, b);
    }
    
    inline void drawDoor(bool isOpen, float openAmount) {
        EnvironmentModels::drawDoor(isOpen, openAmount);
    }
    
    inline void drawPlatform(float sizeX, float sizeY, float sizeZ) {
        EnvironmentModels::drawPlatform(sizeX, sizeY, sizeZ);
    }
    
    inline void drawLavaRock(float size = 1.0f) {
        EnvironmentModels::drawLavaRock(size);
    }
    
    inline void drawObelisk(float glowIntensity = 1.0f) {
        EnvironmentModels::drawObelisk(glowIntensity);
    }
    
    inline void drawWallPanel(float width, float height) {
        EnvironmentModels::drawWallPanel(width, height);
    }
    
    inline void drawPillar(float height) {
        EnvironmentModels::drawPillar(height);
    }
    
    // -------------------- EFFECTS --------------------
    inline void drawBulletTracer(const Vector3& start, const Vector3& end, float r, float g, float b, float alpha) {
        EffectsModels::drawBulletTracer(start, end, r, g, b, alpha);
    }
    
    inline void drawLaserBullet(float length = 2.5f) {
        EffectsModels::drawLaserBullet(length);
    }
    
    inline void drawMuzzleFlash(float intensity = 1.0f, float size = 1.0f) {
        EffectsModels::drawMuzzleFlash(intensity, size);
    }
    
    inline void drawExplosion(float progress, float size = 1.0f) {
        EffectsModels::drawExplosion(progress, size);
    }
    
    inline void drawBloodSplatter(float progress, float size = 1.0f) {
        EffectsModels::drawBloodSplatter(progress, size);
    }
    
    inline void drawCrosshair(float spread = 0.0f, bool targetInSight = false) {
        EffectsModels::drawCrosshair(spread, targetInSight);
    }
    
    inline void drawPickupGlow(float r, float g, float b, float intensity = 1.0f) {
        EffectsModels::drawPickupGlow(r, g, b, intensity);
    }
    
    inline void drawWeaponLightCone(float range, float angle, float intensity) {
        EffectsModels::drawWeaponLightCone(range, angle, intensity);
    }
    
    inline void drawFootstepDust(float progress, float size = 0.3f) {
        EffectsModels::drawFootstepDust(progress, size);
    }
    
    inline void drawEnergyShield(float health, float maxHealth) {
        EffectsModels::drawEnergyShield(health, maxHealth);
    }
    
    inline void drawTeleportEffect(float progress, bool appearing) {
        EffectsModels::drawTeleportEffect(progress, appearing);
    }
    
    inline void drawDamageIndicator(float angle, float intensity) {
        EffectsModels::drawDamageIndicator(angle, intensity);
    }
    
    // -------------------- LEVEL GEOMETRY --------------------
    
    // OPTIMIZED: Simple floor tile for better FPS (fewer draw calls)
    inline void drawFloorTileSimple(float x, float z, float size) {
        glPushMatrix();
        glTranslatef(x, 0, z);
        
        // Single base plate - no extra details
        int tileVariant = ((int)(x * 7 + z * 13)) % 3;
        float colorVar = 0.95f + (tileVariant * 0.015f);
        setColorMetallic(0.22f * colorVar, 0.24f * colorVar, 0.28f * colorVar);
        drawBox(size, 0.08f, size);
        
        // Just simple grid lines
        setColorMetallic(0.18f, 0.2f, 0.24f);
        glPushMatrix();
        glTranslatef(0, 0.05f, 0);
        drawBox(size * 0.9f, 0.01f, 0.03f);
        drawBox(0.03f, 0.01f, size * 0.9f);
        glPopMatrix();
        
        glPopMatrix();
    }
    
    // Draw an ENHANCED sci-fi floor tile with detailed texturing
    inline void drawFloorTile(float x, float z, float size) {
        glPushMatrix();
        glTranslatef(x, 0, z);
        
        // Pseudo-random variation based on position
        int tileVariant = ((int)(x * 7 + z * 13)) % 5;
        float colorVar = 0.95f + (tileVariant * 0.02f);
        
        // === BASE FLOOR PLATE ===
        setColorMetallic(0.22f * colorVar, 0.24f * colorVar, 0.28f * colorVar);
        drawBox(size, 0.12f, size);
        
        // === MAIN GRID PATTERN ===
        setColorMetallic(0.18f, 0.2f, 0.24f);
        glPushMatrix();
        glTranslatef(0, 0.065f, 0);
        // Cross pattern
        drawBox(size * 0.85f, 0.015f, 0.05f);
        drawBox(0.05f, 0.015f, size * 0.85f);
        // Diagonal accents
        if (tileVariant % 2 == 0) {
            glPushMatrix();
            glRotatef(45, 0, 1, 0);
            drawBox(size * 0.5f, 0.012f, 0.03f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // === CORNER DETAILS - Tech panels ===
        setColorMetallic(0.3f, 0.32f, 0.36f);
        float corners[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            float cx = corners[i][0] * (size/2 - 0.25f);
            float cz = corners[i][1] * (size/2 - 0.25f);
            glTranslatef(cx, 0.07f, cz);
            
            // Corner plate
            drawBox(0.4f, 0.025f, 0.4f);
            
            // Corner bolt/rivet details
            setColorMetallic(0.4f, 0.42f, 0.45f);
            drawBox(0.08f, 0.04f, 0.08f);
            
            // Small LED indicator (some tiles)
            if ((tileVariant + i) % 3 == 0) {
                float pulse = sin(getTime() * 3.0f + x + z + i) * 0.3f + 0.7f;
                setColor(0.1f, 0.8f * pulse, 0.3f * pulse);
                setEmissive(0.05f, 0.4f * pulse, 0.15f * pulse);
                glTranslatef(0.12f, 0.02f, 0.12f);
                drawBox(0.05f, 0.02f, 0.05f);
                clearEmissive();
            }
            glPopMatrix();
        }
        
        // === CENTER DETAIL - varies by tile ===
        glPushMatrix();
        glTranslatef(0, 0.07f, 0);
        
        if (tileVariant == 0) {
            // Circular grate pattern
            setColorMetallic(0.15f, 0.16f, 0.2f);
            for (int r = 0; r < 3; r++) {
                float radius = 0.3f + r * 0.25f;
                for (int a = 0; a < 8; a++) {
                    float angle = a * 45.0f * 3.14159f / 180.0f;
                    glPushMatrix();
                    glTranslatef(cos(angle) * radius, 0, sin(angle) * radius);
                    drawBox(0.08f, 0.02f, 0.08f);
                    glPopMatrix();
                }
            }
        } else if (tileVariant == 1) {
            // Hazard stripes
            setColor(0.8f, 0.6f, 0.1f);
            for (int s = -2; s <= 2; s++) {
                glPushMatrix();
                glTranslatef(s * 0.3f, 0, s * 0.3f);
                glRotatef(45, 0, 1, 0);
                drawBox(0.08f, 0.015f, size * 0.4f);
                glPopMatrix();
            }
        } else if (tileVariant == 2) {
            // Tech panel with glow
            setColorMetallic(0.12f, 0.14f, 0.18f);
            drawBox(0.8f, 0.02f, 0.8f);
            float pulse = sin(getTime() * 2.0f + x * z) * 0.2f + 0.8f;
            setColor(0.2f * pulse, 0.4f * pulse, 0.8f * pulse);
            setEmissive(0.1f * pulse, 0.2f * pulse, 0.4f * pulse);
            drawBox(0.6f, 0.025f, 0.1f);
            drawBox(0.1f, 0.025f, 0.6f);
            clearEmissive();
        } else if (tileVariant == 3) {
            // Vent grate
            setColorMetallic(0.1f, 0.1f, 0.12f);
            drawBox(0.9f, 0.01f, 0.9f);
            for (int v = -3; v <= 3; v++) {
                glPushMatrix();
                glTranslatef(v * 0.12f, 0.015f, 0);
                drawBox(0.04f, 0.02f, 0.85f);
                glPopMatrix();
            }
        } else {
            // Simple raised center
            setColorMetallic(0.28f, 0.3f, 0.34f);
            drawBox(0.5f, 0.03f, 0.5f);
        }
        glPopMatrix();
        
        // === EDGE TRIM ===
        setColorMetallic(0.16f, 0.17f, 0.2f);
        // North/South edges
        glPushMatrix();
        glTranslatef(0, 0.06f, size/2 - 0.05f);
        drawBox(size - 0.1f, 0.02f, 0.08f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0, 0.06f, -size/2 + 0.05f);
        drawBox(size - 0.1f, 0.02f, 0.08f);
        glPopMatrix();
        // East/West edges
        glPushMatrix();
        glTranslatef(size/2 - 0.05f, 0.06f, 0);
        drawBox(0.08f, 0.02f, size - 0.1f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-size/2 + 0.05f, 0.06f, 0);
        drawBox(0.08f, 0.02f, size - 0.1f);
        glPopMatrix();
        
        glPopMatrix();
    }
    
    // Draw an ENHANCED sci-fi wall segment with detailed panels and tech details
    // DETAILED wall segment with optimizations (fewer panels per wall)
    inline void drawWallSegment(float x, float z, float rotation, float width, float height) {
        glPushMatrix();
        glTranslatef(x, height/2, z);
        glRotatef(rotation, 0, 1, 0);
        
        // Pseudo-random based on position
        int wallVariant = ((int)(fabs(x) * 11 + fabs(z) * 17)) % 4;
        
        // === MAIN WALL BODY ===
        setColorMetallic(0.2f, 0.22f, 0.26f);
        drawBox(width, height, 0.5f);
        
        // Draw BOTH SIDES of the wall with panels (optimized: fewer panels)
        for (int side = -1; side <= 1; side += 2) {
            float zOffset = side * 0.26f;
            
            // Fewer panels for optimization (larger panel width)
            int numPanels = (int)(width / 6.0f);  // Was 3.5f - now larger panels
            if (numPanels < 1) numPanels = 1;
            float panelWidth = (width - 0.4f) / numPanels;
            
            for (int i = 0; i < numPanels; i++) {
                float panelX = -width/2 + 0.2f + panelWidth/2 + i * panelWidth;
                int panelType = (i + wallVariant + (side == -1 ? 2 : 0)) % 4;
                
                glPushMatrix();
                glTranslatef(panelX, 0, zOffset);
                if (side == -1) glRotatef(180, 0, 1, 0);
                
                // Panel base (recessed)
                setColorMetallic(0.15f, 0.17f, 0.2f);
                drawBox(panelWidth - 0.15f, height - 0.6f, 0.03f);
                
                // Panel details based on type
                if (panelType == 0) {
                    // Tech panel with screen
                    setColorMetallic(0.1f, 0.12f, 0.15f);
                    glPushMatrix();
                    glTranslatef(0, height * 0.15f, 0.025f);
                    drawBox(panelWidth * 0.6f, height * 0.3f, 0.02f);
                    
                    // Screen glow
                    float pulse = sin(getTime() * 1.5f + i + x + side) * 0.15f + 0.85f;
                    setColor(0.1f, 0.3f * pulse, 0.5f * pulse);
                    setEmissive(0.05f, 0.15f * pulse, 0.25f * pulse);
                    drawBox(panelWidth * 0.55f, height * 0.25f, 0.025f);
                    clearEmissive();
                    glPopMatrix();
                    
                } else if (panelType == 1) {
                    // Ventilation panel (simplified - fewer slats)
                    setColorMetallic(0.12f, 0.12f, 0.14f);
                    int numVents = (int)(height / 1.5f);  // Fewer vents
                    for (int v = 0; v < numVents; v++) {
                        glPushMatrix();
                        glTranslatef(0, -height/2 + 0.5f + v * 1.2f, 0.025f);
                        drawBox(panelWidth * 0.7f, 0.15f, 0.015f);
                        glPopMatrix();
                    }
                    
                } else if (panelType == 2) {
                    // Pipe/conduit panel
                    setColorMetallic(0.25f, 0.27f, 0.3f);
                    // Vertical pipes
                    for (int p = -1; p <= 1; p += 2) {
                        glPushMatrix();
                        glTranslatef(p * panelWidth * 0.25f, 0, 0.04f);
                        drawBox(0.12f, height - 0.8f, 0.12f);
                        // Fewer pipe bands
                        setColorMetallic(0.35f, 0.35f, 0.38f);
                        for (int b = 0; b < 2; b++) {
                            glPushMatrix();
                            glTranslatef(0, -height/2 + 1.0f + b * (height - 1.5f), 0);
                            drawBox(0.15f, 0.08f, 0.15f);
                            glPopMatrix();
                        }
                        glPopMatrix();
                    }
                    
                } else {
                    // Industrial panel with rivets (simplified)
                    setColorMetallic(0.18f, 0.2f, 0.24f);
                    drawBox(panelWidth - 0.3f, height - 0.8f, 0.02f);
                    
                    // Just corner rivets
                    setColorMetallic(0.4f, 0.42f, 0.45f);
                    float rivetOff = 0.2f;
                    float pW = (panelWidth - 0.3f) / 2 - rivetOff;
                    float pH = (height - 0.8f) / 2 - rivetOff;
                    for (int rx = -1; rx <= 1; rx += 2) {
                        for (int ry = -1; ry <= 1; ry += 2) {
                            glPushMatrix();
                            glTranslatef(rx * pW, ry * pH, 0.025f);
                            drawBox(0.06f, 0.06f, 0.03f);
                            glPopMatrix();
                        }
                    }
                }
                glPopMatrix();
            }
            
            // Running light strip (one per side)
            float runningPos = fmod(getTime() * 2.0f + side, width) - width/2;
            setColor(0.0f, 0.8f, 1.0f);
            setEmissive(0.0f, 0.4f, 0.5f);
            glPushMatrix();
            glTranslatef(runningPos, -height/2 + 0.15f, zOffset + 0.02f * side);
            drawBox(0.5f, 0.05f, 0.02f);
            glPopMatrix();
            clearEmissive();
        }
        
        glPopMatrix();
    }
    
    // Simple wall segment (kept for reference)
    inline void drawWallSegmentSimple(float x, float z, float rotation, float width, float height) {
        glPushMatrix();
        glTranslatef(x, height/2, z);
        glRotatef(rotation, 0, 1, 0);
        
        // Pseudo-random based on position
        int wallVariant = ((int)(fabs(x) * 11 + fabs(z) * 17)) % 4;
        
        // === MAIN WALL BODY (THICKER for double-sided appearance) ===
        setColorMetallic(0.2f, 0.22f, 0.26f);
        drawBox(width, height, 0.5f);
        
        // Draw BOTH SIDES of the wall with panels
        for (int side = -1; side <= 1; side += 2) {
            float zOffset = side * 0.26f;  // Front and back face offset
            float normalFlip = (side == 1) ? 1.0f : -1.0f;
            
            // === WALL PANELS ===
            int numPanels = (int)(width / 3.5f);
            if (numPanels < 1) numPanels = 1;
            float panelWidth = (width - 0.4f) / numPanels;
            
            for (int i = 0; i < numPanels; i++) {
                float panelX = -width/2 + 0.2f + panelWidth/2 + i * panelWidth;
                int panelType = (i + wallVariant + (side == -1 ? 2 : 0)) % 4;
                
                glPushMatrix();
                glTranslatef(panelX, 0, zOffset);
                if (side == -1) glRotatef(180, 0, 1, 0); // Flip for back side
                
                // Panel base (recessed)
                setColorMetallic(0.15f, 0.17f, 0.2f);
                drawBox(panelWidth - 0.15f, height - 0.6f, 0.03f);
                
                // Panel details based on type
                if (panelType == 0) {
                    // Tech panel with screen
                    setColorMetallic(0.1f, 0.12f, 0.15f);
                    glPushMatrix();
                    glTranslatef(0, height * 0.15f, 0.025f);
                    drawBox(panelWidth * 0.6f, height * 0.3f, 0.02f);
                    
                    // Screen glow
                    float pulse = sin(getTime() * 1.5f + i + x + side) * 0.15f + 0.85f;
                    setColor(0.1f, 0.3f * pulse, 0.5f * pulse);
                    setEmissive(0.05f, 0.15f * pulse, 0.25f * pulse);
                    drawBox(panelWidth * 0.55f, height * 0.25f, 0.025f);
                    
                    // Scan line effect
                    float scanY = fmod(getTime() * 0.5f + i + side, 1.0f) * height * 0.25f - height * 0.125f;
                    setColor(0.2f, 0.6f * pulse, 0.9f * pulse);
                    setEmissive(0.1f, 0.3f * pulse, 0.45f * pulse);
                    glTranslatef(0, scanY, 0.003f);
                    drawBox(panelWidth * 0.5f, 0.02f, 0.01f);
                    clearEmissive();
                    glPopMatrix();
                    
                } else if (panelType == 1) {
                    // Ventilation panel
                    setColorMetallic(0.12f, 0.12f, 0.14f);
                    int numVents = (int)(height / 0.8f);
                    for (int v = 0; v < numVents; v++) {
                        glPushMatrix();
                        glTranslatef(0, -height/2 + 0.5f + v * 0.7f, 0.025f);
                        // Vent slats
                        for (int s = 0; s < 5; s++) {
                            glPushMatrix();
                            glTranslatef(0, -0.15f + s * 0.08f, 0);
                            drawBox(panelWidth * 0.7f, 0.03f, 0.015f);
                            glPopMatrix();
                        }
                        glPopMatrix();
                    }
                    
                } else if (panelType == 2) {
                    // Pipe/conduit panel
                    setColorMetallic(0.25f, 0.27f, 0.3f);
                    // Vertical pipes
                    for (int p = -1; p <= 1; p += 2) {
                        glPushMatrix();
                        glTranslatef(p * panelWidth * 0.25f, 0, 0.04f);
                        drawBox(0.12f, height - 0.8f, 0.12f);
                        // Pipe bands
                        setColorMetallic(0.35f, 0.35f, 0.38f);
                        for (int b = 0; b < 4; b++) {
                            glPushMatrix();
                            glTranslatef(0, -height/2 + 0.8f + b * (height - 1.2f) / 3, 0);
                            drawBox(0.15f, 0.08f, 0.15f);
                            glPopMatrix();
                        }
                        glPopMatrix();
                    }
                    // Horizontal connector
                    setColorMetallic(0.22f, 0.24f, 0.28f);
                    glPushMatrix();
                    glTranslatef(0, 0, 0.06f);
                    drawBox(panelWidth * 0.5f, 0.1f, 0.1f);
                    glPopMatrix();
                    
                } else {
                    // Industrial panel with rivets
                    setColorMetallic(0.18f, 0.2f, 0.24f);
                    // Panel sections
                    for (int sy = 0; sy < 3; sy++) {
                        glPushMatrix();
                        glTranslatef(0, -height/2 + 0.6f + sy * (height - 0.8f) / 2.5f, 0.025f);
                        drawBox(panelWidth * 0.85f, (height - 0.8f) / 3.2f, 0.015f);
                        glPopMatrix();
                    }
                    // Rivets
                    setColorMetallic(0.35f, 0.35f, 0.4f);
                    for (int ry = 0; ry < 4; ry++) {
                        for (int rx = -1; rx <= 1; rx += 2) {
                            glPushMatrix();
                            glTranslatef(rx * panelWidth * 0.35f, -height/2 + 0.4f + ry * (height - 0.6f) / 3.5f, 0.04f);
                            drawBox(0.06f, 0.06f, 0.04f);
                            glPopMatrix();
                        }
                    }
                }
                glPopMatrix();
            }
            
            // === TOP TRIM with running lights (both sides) ===
            setColorMetallic(0.28f, 0.3f, 0.34f);
            glPushMatrix();
            glTranslatef(0, height/2 - 0.12f, zOffset);
            if (side == -1) glRotatef(180, 0, 1, 0);
            drawBox(width - 0.1f, 0.18f, 0.06f);
            glPopMatrix();
            
            // Running light strip (both sides)
            float lightTime = getTime() * 3.0f;
            int numLights = (int)(width / 1.2f);
            for (int l = 0; l < numLights; l++) {
                float lightPhase = fmod(lightTime + l * 0.3f + (side == -1 ? numLights/2 : 0), (float)numLights) / (float)numLights;
                float intensity = (lightPhase < 0.3f) ? lightPhase / 0.3f : (lightPhase < 0.5f) ? 1.0f : 1.0f - (lightPhase - 0.5f) / 0.5f;
                intensity = intensity * intensity; // Smoother falloff
                
                glPushMatrix();
                glTranslatef(-width/2 + 0.6f + l * 1.2f, height/2 - 0.12f, zOffset + (side == 1 ? 0.04f : -0.04f));
                setColor(0.2f + 0.6f * intensity, 0.5f + 0.4f * intensity, 0.9f * intensity);
                setEmissive(0.1f * intensity, 0.25f * intensity, 0.45f * intensity);
                drawBox(0.25f, 0.06f, 0.02f);
                clearEmissive();
                glPopMatrix();
            }
            
            // === BOTTOM VENT/TRIM (both sides) ===
            setColorMetallic(0.12f, 0.13f, 0.16f);
            glPushMatrix();
            glTranslatef(0, -height/2 + 0.2f, zOffset);
            if (side == -1) glRotatef(180, 0, 1, 0);
            drawBox(width - 0.1f, 0.35f, 0.06f);
            // Vent slots
            int numSlots = (int)(width / 0.4f);
            for (int v = 0; v < numSlots; v++) {
                glPushMatrix();
                glTranslatef(-width/2 + 0.2f + v * 0.4f, 0.02f, 0.035f);
                setColorMetallic(0.05f, 0.05f, 0.08f);
                drawBox(0.25f, 0.2f, 0.02f);
                glPopMatrix();
            }
            glPopMatrix();
        }
        
        // === WALL COLUMNS at edges (both sides) ===
        setColorMetallic(0.25f, 0.27f, 0.3f);
        for (int sideX = -1; sideX <= 1; sideX += 2) {
            glPushMatrix();
            glTranslatef(sideX * (width/2 - 0.12f), 0, 0);
            drawBox(0.2f, height, 0.5f); // Full-depth column
            // Column detail bands on both faces
            setColorMetallic(0.32f, 0.34f, 0.38f);
            for (int b = 0; b < 5; b++) {
                for (int sideZ = -1; sideZ <= 1; sideZ += 2) {
                    glPushMatrix();
                    glTranslatef(0, -height/2 + 0.5f + b * height / 5.0f, sideZ * 0.26f);
                    drawBox(0.22f, 0.1f, 0.02f);
                    glPopMatrix();
                }
            }
            glPopMatrix();
        }
        
        glPopMatrix();
    }
    
    // OPTIMIZED: Draw complete level floor using detailed tiles but larger size
    inline void drawLevelFloor(float width, float depth) {
        float tileSize = 8.0f;  // Larger tiles = fewer draw calls (was 4.0f)
        int tilesX = (int)(width / tileSize);
        int tilesZ = (int)(depth / tileSize);
        
        for (int x = 0; x < tilesX; x++) {
            for (int z = 0; z < tilesZ; z++) {
                float posX = -width/2 + tileSize/2 + x * tileSize;
                float posZ = -depth/2 + tileSize/2 + z * tileSize;
                drawFloorTile(posX, posZ, tileSize - 0.05f);  // Use detailed tiles
            }
        }
    }
    
    // OPTIMIZED: Draw ceiling with detailed panels but larger spacing
    inline void drawCeiling(float width, float depth, float height) {
        float tileSize = 10.0f;  // Larger tiles (was 6.0f)
        int tilesX = (int)(width / tileSize);
        int tilesZ = (int)(depth / tileSize);
        
        for (int x = 0; x < tilesX; x++) {
            for (int z = 0; z < tilesZ; z++) {
                float posX = -width/2 + tileSize/2 + x * tileSize;
                float posZ = -depth/2 + tileSize/2 + z * tileSize;
                int tileType = (x + z) % 3;
                
                glPushMatrix();
                glTranslatef(posX, height, posZ);
                
                // Main ceiling panel
                setColorMetallic(0.18f, 0.19f, 0.22f);
                drawBox(tileSize - 0.08f, 0.18f, tileSize - 0.08f);
                
                // Panel frame/border
                setColorMetallic(0.22f, 0.24f, 0.28f);
                // North/South borders
                glPushMatrix();
                glTranslatef(0, -0.05f, tileSize/2 - 0.12f);
                drawBox(tileSize - 0.1f, 0.08f, 0.15f);
                glPopMatrix();
                glPushMatrix();
                glTranslatef(0, -0.05f, -tileSize/2 + 0.12f);
                drawBox(tileSize - 0.1f, 0.08f, 0.15f);
                glPopMatrix();
                // East/West borders
                glPushMatrix();
                glTranslatef(tileSize/2 - 0.12f, -0.05f, 0);
                drawBox(0.15f, 0.08f, tileSize - 0.1f);
                glPopMatrix();
                glPushMatrix();
                glTranslatef(-tileSize/2 + 0.12f, -0.05f, 0);
                drawBox(0.15f, 0.08f, tileSize - 0.1f);
                glPopMatrix();
                
                // Center detail based on tile type
                if (tileType == 0) {
                    // Main light fixture (large)
                    float pulse = sin(getTime() * 1.5f + x + z) * 0.08f + 0.92f;
                    float flicker = (sin(getTime() * 30.0f + x * z) > 0.95f) ? 0.7f : 1.0f;
                    pulse *= flicker;
                    
                    // Light housing
                    setColorMetallic(0.25f, 0.27f, 0.3f);
                    glPushMatrix();
                    glTranslatef(0, -0.12f, 0);
                    drawBox(2.2f, 0.1f, 2.2f);
                    
                    // Light diffuser
                    setColor(0.85f * pulse, 0.9f * pulse, 1.0f * pulse);
                    setEmissive(0.5f * pulse, 0.55f * pulse, 0.6f * pulse);
                    glTranslatef(0, -0.06f, 0);
                    drawBox(1.9f, 0.04f, 1.9f);
                    
                    // Inner bright core
                    setColor(0.95f * pulse, 0.98f * pulse, 1.0f * pulse);
                    setEmissive(0.7f * pulse, 0.75f * pulse, 0.8f * pulse);
                    glTranslatef(0, -0.02f, 0);
                    drawBox(1.4f, 0.02f, 1.4f);
                    clearEmissive();
                    glPopMatrix();
                    
                } else if (tileType == 1) {
                    // Ventilation grate
                    setColorMetallic(0.12f, 0.13f, 0.16f);
                    glPushMatrix();
                    glTranslatef(0, -0.1f, 0);
                    drawBox(2.5f, 0.05f, 2.5f);
                    
                    // Grate bars
                    setColorMetallic(0.15f, 0.16f, 0.2f);
                    for (int v = -4; v <= 4; v++) {
                        glPushMatrix();
                        glTranslatef(v * 0.28f, -0.03f, 0);
                        drawBox(0.06f, 0.04f, 2.3f);
                        glPopMatrix();
                    }
                    glPopMatrix();
                    
                    // Small status light
                    float pulse = sin(getTime() * 2.0f + x * z) * 0.3f + 0.7f;
                    setColor(0.2f, 0.8f * pulse, 0.3f * pulse);
                    setEmissive(0.1f, 0.4f * pulse, 0.15f * pulse);
                    glPushMatrix();
                    glTranslatef(1.8f, -0.08f, 1.8f);
                    drawBox(0.15f, 0.04f, 0.15f);
                    glPopMatrix();
                    clearEmissive();
                    
                } else {
                    // Smaller accent lights
                    float pulse = sin(getTime() * 2.0f + x - z) * 0.1f + 0.9f;
                    
                    // Four corner lights
                    for (int cx = -1; cx <= 1; cx += 2) {
                        for (int cz = -1; cz <= 1; cz += 2) {
                            glPushMatrix();
                            glTranslatef(cx * 1.2f, -0.1f, cz * 1.2f);
                            
                            // Light housing
                            setColorMetallic(0.22f, 0.24f, 0.28f);
                            drawBox(0.6f, 0.06f, 0.6f);
                            
                            // Light
                            setColor(0.7f * pulse, 0.85f * pulse, 1.0f * pulse);
                            setEmissive(0.3f * pulse, 0.4f * pulse, 0.5f * pulse);
                            glTranslatef(0, -0.04f, 0);
                            drawBox(0.45f, 0.03f, 0.45f);
                            clearEmissive();
                            glPopMatrix();
                        }
                    }
                    
                    // Center tech panel
                    setColorMetallic(0.14f, 0.15f, 0.18f);
                    glPushMatrix();
                    glTranslatef(0, -0.1f, 0);
                    drawBox(1.0f, 0.04f, 1.0f);
                    glPopMatrix();
                }
                
                // Structural beam supports (every tile)
                setColorMetallic(0.2f, 0.22f, 0.26f);
                // Cross beams
                glPushMatrix();
                glTranslatef(0, 0.12f, 0);
                drawBox(tileSize + 0.1f, 0.15f, 0.2f);
                drawBox(0.2f, 0.15f, tileSize + 0.1f);
                glPopMatrix();
                
                glPopMatrix();
            }
        }
    }
    
    // Draw arena boundary walls
    inline void drawArenaWalls(float width, float depth, float height) {
        // North wall
        drawWallSegment(0, -depth/2, 0, width, height);
        // South wall
        drawWallSegment(0, depth/2, 180, width, height);
        // East wall
        drawWallSegment(width/2, 0, 90, depth, height);
        // West wall
        drawWallSegment(-width/2, 0, -90, depth, height);
    }
}

#endif // LOW_POLY_MODELS_H
