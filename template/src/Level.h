// ============================================================================
// DOOMERS - Level.h
// Level management: Level 1 (Facility) and Level 2 (Hell Arena)
// ============================================================================
#ifndef LEVEL_H
#define LEVEL_H

#include "GameConfig.h"
#include "Vector3.h"
#include "Enemy.h"
#include "Collectible.h"
#include "Collision.h"
#include "LowPolyModels.h"
#include "TextureManager.h"
#include "ModelLoader.h"
#include <glut.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <stdio.h>

// Use global debug function from Doomers.cpp
extern void debugLog(const char* msg);
#define DEBUG_LOG(msg) debugLog(msg)

// Import types from Collision namespace for convenience
using Collision::AABB;
using Collision::Sphere;
using Collision::Ray;
using Collision::Platform;
using Collision::CollisionResult;
using Collision::resolveSphereAABB;

// Door structure
struct Door {
    Vector3 position;
    float rotation;
    bool isLocked;
    int requiredKeycard;
    bool isOpen;
    float openAmount;
    AABB bounds;
    
    Door() {
        position = Vector3(0, 0, 0);
        rotation = 0.0f;
        isLocked = false;
        requiredKeycard = 0;
        isOpen = false;
        openAmount = 0.0f;
        updateBounds();
    }
    
    void updateBounds() {
        bounds = AABB::fromCenterSize(position + Vector3(0, 1.5f, 0), Vector3(1.2f, 1.5f, 0.3f));
    }
    
    void update(float deltaTime) {
        if (isOpen && openAmount < 1.0f) {
            openAmount += deltaTime * 2.0f;
            if (openAmount > 1.0f) openAmount = 1.0f;
        }
    }
    
    void draw() {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation, 0, 1, 0);
        
        // Apply texture to door if available
        if (TextureManager::isLoaded(TEX_WALL_PANEL)) {
            glEnable(GL_TEXTURE_2D);
            TextureManager::bind(TEX_WALL_PANEL);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
        
        LowPolyModels::drawDoor(isOpen, openAmount);
        
        if (TextureManager::isLoaded(TEX_WALL_PANEL)) {
            TextureManager::unbind();
            glDisable(GL_TEXTURE_2D);
        }
        
        glPopMatrix();
    }
};

// Mystery Box contents
enum MysteryBoxContent {
    MYSTERY_NOTHING = 0,
    MYSTERY_HEALTH,
    MYSTERY_AMMO
};

// Crate/Obstacle - can be a mystery box
struct Crate {
    Vector3 position;
    float size;
    bool isSciFi;
    AABB bounds;
    
    // Mystery box properties
    bool isMysteryBox;
    bool isOpened;
    float openAnimProgress;
    MysteryBoxContent content;
    bool contentCollected;
    float glowPhase;
    
    Crate() : position(0,0,0), size(1.0f), isSciFi(false), isMysteryBox(false), 
              isOpened(false), openAnimProgress(0.0f), content(MYSTERY_NOTHING),
              contentCollected(false), glowPhase(0.0f) {
        updateBounds();
    }
    
    void updateBounds() {
        float halfSize = size * 0.5f;
        bounds = AABB::fromCenterSize(position + Vector3(0, halfSize, 0), Vector3(halfSize, halfSize, halfSize));
    }
    
    void setAsMysteryBox() {
        isMysteryBox = true;
        isOpened = false;
        openAnimProgress = 0.0f;
        contentCollected = false;
        // Random content: 40% nothing, 35% health, 25% ammo
        int roll = rand() % 100;
        if (roll < 40) content = MYSTERY_NOTHING;
        else if (roll < 75) content = MYSTERY_HEALTH;
        else content = MYSTERY_AMMO;
    }
    
    void update(float deltaTime) {
        glowPhase += deltaTime * 3.0f;
        if (isOpened && openAnimProgress < 1.0f) {
            openAnimProgress += deltaTime * 2.5f;
            if (openAnimProgress > 1.0f) openAnimProgress = 1.0f;
        }
    }
    
    bool tryOpen() {
        if (!isMysteryBox || isOpened) return false;
        isOpened = true;
        return true;
    }
    
    // Returns content type and marks as collected
    MysteryBoxContent collectContent() {
        if (!isMysteryBox || !isOpened || contentCollected) return MYSTERY_NOTHING;
        contentCollected = true;
        return content;
    }
    
    void draw() {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        
        if (isMysteryBox) {
            drawMysteryBox();
        } else {
            // Try to use 3D model first for regular crates
            if (ModelLoader::isLoaded(MODEL_CRATE)) {
                glTranslatef(0, size * 0.5f, 0);
                ModelLoader::draw(MODEL_CRATE, size * 1.2f);
            } else {
                // Fallback to procedural with texture
                glTranslatef(0, size * 0.5f, 0);
                if (isSciFi) {
                    TextureManager::drawTexturedBox(TEX_CRATE_SCIFI, 0, 0, 0, size, size, size, 1.0f);
                } else {
                    TextureManager::drawTexturedBox(TEX_CRATE, 0, 0, 0, size, size, size, 1.0f);
                }
            }
        }
        glPopMatrix();
    }
    
    void drawMysteryBox() {
        float pulse = sin(glowPhase * 2.5f) * 0.25f + 0.75f;
        float fastPulse = sin(glowPhase * 4.0f) * 0.2f + 0.8f;
        
        // Gentle hover animation when not opened
        float hoverOffset = isOpened ? 0.0f : sin(glowPhase * 2.0f) * 0.05f;
        
        glPushMatrix();
        glTranslatef(0, size * 0.5f + hoverOffset, 0);
        
        if (!isOpened) {
            // Very slow rotation
            glRotatef(glowPhase * 8.0f, 0, 1, 0);
            
            // Use 3D model if available, with pulsing color tint
            if (ModelLoader::isLoaded(MODEL_CRATE)) {
                // Apply pulsing blue-ish glow color
                GLfloat mysteryColor[] = {0.5f + 0.3f * pulse, 0.6f + 0.3f * pulse, 0.9f + 0.1f * pulse, 1.0f};
                GLfloat mysteryEmissive[] = {0.1f * pulse, 0.15f * pulse, 0.3f * pulse, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mysteryColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mysteryEmissive);
                ModelLoader::draw(MODEL_CRATE, size * 1.2f);
                // Clear emissive
                GLfloat noEmissive[] = {0, 0, 0, 1};
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmissive);
            } else if (TextureManager::isLoaded(TEX_CRATE_SCIFI)) {
                glColor3f(0.5f + 0.3f * pulse, 0.6f + 0.3f * pulse, 0.9f + 0.1f * pulse);
                TextureManager::drawTexturedBox(TEX_CRATE_SCIFI, 0, 0, 0, size, size, size, 0.5f);
            } else {
                LowPolyModels::setColor(0.15f + 0.15f * pulse, 0.25f + 0.25f * pulse, 0.5f + 0.3f * pulse);
                LowPolyModels::drawSciFiCrate(size);
            }
            
            // Glowing edges effect
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            float edgeGlow = 0.6f + 0.4f * fastPulse;
            glColor4f(0.3f * edgeGlow, 0.6f * edgeGlow, 1.0f * edgeGlow, 0.7f);
            float hs = size * 0.52f;
            // Vertical edges
            for (int i = 0; i < 4; i++) {
                float ex = (i < 2) ? -hs : hs;
                float ez = (i % 2 == 0) ? -hs : hs;
                glVertex3f(ex, -hs, ez);
                glVertex3f(ex, hs, ez);
            }
            glEnd();
            glLineWidth(1.0f);
            
            // Question mark symbol (simple, no sphere)
            glColor4f(1.0f * pulse, 0.85f * pulse, 0.3f * pulse, 0.85f);
            glPushMatrix();
            glTranslatef(0, 0, size * 0.53f);
            float qSize = size * 0.2f;
            glBegin(GL_QUADS);
            // Top arc of ?
            glVertex3f(-qSize * 0.4f, qSize * 0.6f, 0);
            glVertex3f(qSize * 0.4f, qSize * 0.6f, 0);
            glVertex3f(qSize * 0.4f, qSize * 0.9f, 0);
            glVertex3f(-qSize * 0.4f, qSize * 0.9f, 0);
            // Stem
            glVertex3f(-qSize * 0.12f, -qSize * 0.1f, 0);
            glVertex3f(qSize * 0.12f, -qSize * 0.1f, 0);
            glVertex3f(qSize * 0.12f, qSize * 0.35f, 0);
            glVertex3f(-qSize * 0.12f, qSize * 0.35f, 0);
            // Dot (square, not sphere)
            glVertex3f(-qSize * 0.12f, -qSize * 0.5f, 0);
            glVertex3f(qSize * 0.12f, -qSize * 0.5f, 0);
            glVertex3f(qSize * 0.12f, -qSize * 0.28f, 0);
            glVertex3f(-qSize * 0.12f, -qSize * 0.28f, 0);
            glEnd();
            glPopMatrix();
            
            // Small floating particles (tiny, subtle)
            for (int i = 0; i < 4; i++) {
                float angle = glowPhase * 0.5f + i * 1.57f;
                float radius = size * 0.7f;
                float px = cos(angle) * radius;
                float pz = sin(angle) * radius;
                float py = sin(glowPhase * 1.2f + i) * 0.15f;
                
                glColor4f(0.5f, 0.8f, 1.0f, 0.3f * fastPulse);
                glPointSize(4.0f);
                glBegin(GL_POINTS);
                glVertex3f(px, py, pz);
                glEnd();
            }
            glPointSize(1.0f);
            
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
        } else {
            // OPENED BOX animation
            float lidAngle = openAnimProgress * 115.0f;
            
            // Box base - use 3D model if available
            if (ModelLoader::isLoaded(MODEL_CRATE)) {
                glPushMatrix();
                glScalef(1.0f, 0.6f, 1.0f);
                GLfloat openedColor[] = {0.4f, 0.45f, 0.5f, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, openedColor);
                ModelLoader::draw(MODEL_CRATE, size * 1.2f);
                glPopMatrix();
            } else {
                LowPolyModels::setColorMetallic(0.18f, 0.2f, 0.24f);
                glPushMatrix();
                glScalef(1.0f, 0.6f, 1.0f);
                LowPolyModels::drawSciFiCrate(size);
                glPopMatrix();
            }
            
            // Lid hinging open - use 3D model scaled as lid if available
            glPushMatrix();
            glTranslatef(0, size * 0.3f, -size * 0.5f);
            glRotatef(-lidAngle, 1, 0, 0);
            glTranslatef(0, 0, size * 0.5f);
            if (ModelLoader::isLoaded(MODEL_CRATE)) {
                glScalef(1.0f, 0.15f, 1.0f);
                GLfloat lidColor[] = {0.45f, 0.5f, 0.55f, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, lidColor);
                ModelLoader::draw(MODEL_CRATE, size * 1.2f);
            } else {
                LowPolyModels::setColorMetallic(0.22f, 0.25f, 0.3f);
                glScalef(1.0f, 0.15f, 1.0f);
                LowPolyModels::drawBox(size, size * 0.3f, size);
            }
            glPopMatrix();
            
            // Light beam from inside when opening
            if (openAnimProgress > 0.2f && openAnimProgress < 0.9f) {
                glDisable(GL_LIGHTING);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                float beamAlpha = sin((openAnimProgress - 0.2f) / 0.7f * 3.14159f) * 0.5f;
                glColor4f(0.6f, 0.85f, 1.0f, beamAlpha);
                // Light cone coming up
                glBegin(GL_TRIANGLE_FAN);
                glVertex3f(0, size * 0.8f, 0);
                for (int i = 0; i <= 8; i++) {
                    float a = i * 0.785f;
                    glVertex3f(cos(a) * size * 0.25f, 0, sin(a) * size * 0.25f);
                }
                glEnd();
                glDisable(GL_BLEND);
                glEnable(GL_LIGHTING);
            }
            
            // Draw content rising
            if (!contentCollected && openAnimProgress > 0.4f) {
                float contentT = (openAnimProgress - 0.4f) / 0.6f;
                if (contentT > 1.0f) contentT = 1.0f;
                float riseHeight = contentT * 0.6f;
                float spinAngle = contentT * 180.0f;
                float bobble = sin(glowPhase * 4.0f) * 0.03f * contentT;
                
                glPushMatrix();
                glTranslatef(0, riseHeight + bobble + 0.15f, 0);
                glRotatef(spinAngle, 0, 1, 0);
                
                if (content == MYSTERY_HEALTH) {
                    // Subtle green glow
                    glDisable(GL_LIGHTING);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    glColor4f(0.2f, 0.8f, 0.3f, 0.25f);
                    glBegin(GL_TRIANGLE_FAN);
                    glVertex3f(0, 0.2f, 0);
                    for (int i = 0; i <= 8; i++) {
                        float a = i * 0.785f;
                        glVertex3f(cos(a) * 0.35f, 0, sin(a) * 0.35f);
                    }
                    glEnd();
                    glDisable(GL_BLEND);
                    glEnable(GL_LIGHTING);
                    // Use 3D model if available, otherwise fallback to procedural
                    if (ModelLoader::isLoaded(MODEL_HEALTHPACK)) {
                        ModelLoader::draw(MODEL_HEALTHPACK, 0.4f);
                    } else {
                        LowPolyModels::drawHealthPack();
                    }
                } else if (content == MYSTERY_AMMO) {
                    // Subtle yellow glow
                    glDisable(GL_LIGHTING);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    glColor4f(0.9f, 0.7f, 0.2f, 0.25f);
                    glBegin(GL_TRIANGLE_FAN);
                    glVertex3f(0, 0.2f, 0);
                    for (int i = 0; i <= 8; i++) {
                        float a = i * 0.785f;
                        glVertex3f(cos(a) * 0.35f, 0, sin(a) * 0.35f);
                    }
                    glEnd();
                    glDisable(GL_BLEND);
                    glEnable(GL_LIGHTING);
                    // Use 3D model if available, otherwise fallback to procedural
                    if (ModelLoader::isLoaded(MODEL_AMMO_MAGAZINE)) {
                        ModelLoader::draw(MODEL_AMMO_MAGAZINE, 0.5f);
                    } else {
                        LowPolyModels::drawAmmoBox();
                    }
                } else {
                    // Dust puff for empty (no spheres)
                    glDisable(GL_LIGHTING);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glColor4f(0.5f, 0.5f, 0.5f, 0.4f * (1.0f - contentT));
                    // Draw as flat expanding ring
                    float dustSize = 0.15f + contentT * 0.3f;
                    glBegin(GL_TRIANGLE_FAN);
                    glVertex3f(0, 0.1f, 0);
                    for (int i = 0; i <= 12; i++) {
                        float a = i * 0.524f;
                        glVertex3f(cos(a) * dustSize, 0, sin(a) * dustSize);
                    }
                    glEnd();
                    glDisable(GL_BLEND);
                    glEnable(GL_LIGHTING);
                }
                
                glPopMatrix();
            }
        }
        
        glPopMatrix();
    }
};

// Parkour Obstacle - for vaulting
struct ParkourObstacle {
    Vector3 position;
    float width;
    float height;
    float depth;
    float rotation;
    AABB bounds;
    
    ParkourObstacle() : position(0,0,0), width(3.0f), height(1.2f), depth(0.4f), rotation(0.0f) {
        updateBounds();
    }
    
    void updateBounds() {
        // Calculate rotated bounds - swap width/depth based on rotation
        // Use FULL height for collision to prevent ANY pass-through
        float halfH = height * 0.5f + 0.5f; // Extra height padding
        float halfW, halfD;
        
        // If rotation is near 90 or 270 degrees, swap width and depth for AABB
        float absRot = fabs(fmod(rotation, 180.0f));
        if (absRot > 45.0f && absRot < 135.0f) {
            // Rotated 90 degrees - width becomes depth, depth becomes width
            halfW = depth * 0.5f + 0.8f;  // Large padding for solid collision
            halfD = width * 0.5f + 0.8f;
        } else {
            halfW = width * 0.5f + 0.8f;
            halfD = depth * 0.5f + 0.8f;
        }
        // Center the bounds on the obstacle, slightly raised
        bounds = AABB::fromCenterSize(position + Vector3(0, halfH, 0), Vector3(halfW, halfH, halfD));
    }
    
    bool isPlayerNearForVault(const Vector3& playerPos, float playerRadius) {
        float dx = playerPos.x - position.x;
        float dz = playerPos.z - position.z;
        float dist = sqrt(dx * dx + dz * dz);
        return dist < (width * 0.5f + playerRadius + 0.5f);
    }
    
    void draw() {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation, 0, 1, 0);
        
        // Try textured rendering first
        if (TextureManager::isLoaded(TEX_PARKOUR) || TextureManager::isLoaded(TEX_PLATFORM)) {
            TextureID texID = TextureManager::isLoaded(TEX_PARKOUR) ? TEX_PARKOUR : TEX_PLATFORM;
            
            // Base platform textured
            glPushMatrix();
            glTranslatef(0, 0.05f, 0);
            TextureManager::drawTexturedBox(texID, 0, 0, 0, width + 0.4f, 0.1f, depth + 0.8f, 0.5f);
            glPopMatrix();
            
            // Main barrier body textured
            glPushMatrix();
            glTranslatef(0, height * 0.5f, 0);
            TextureManager::drawTexturedBox(texID, 0, 0, 0, width, height, depth + 0.3f, 0.3f);
            glPopMatrix();
            
            // Top rail textured
            glPushMatrix();
            glTranslatef(0, height + 0.08f, 0);
            TextureManager::drawTexturedBox(TEX_WALL_ORANGE_WARNING, 0, 0, 0, width + 0.2f, 0.16f, depth + 0.4f, 0.5f);
            glPopMatrix();
            
            // Support posts textured
            float postX[] = {-width * 0.4f, width * 0.4f};
            for (int i = 0; i < 2; i++) {
                glPushMatrix();
                glTranslatef(postX[i], height * 0.5f, depth * 0.4f);
                TextureManager::drawTexturedBox(texID, 0, 0, 0, 0.2f, height, 0.2f, 0.5f);
                glPopMatrix();
                glPushMatrix();
                glTranslatef(postX[i], height * 0.5f, -depth * 0.4f);
                TextureManager::drawTexturedBox(texID, 0, 0, 0, 0.2f, height, 0.2f, 0.5f);
                glPopMatrix();
            }
        } else {
            // Fallback to procedural rendering
            // Base platform (so it's more visible)
            LowPolyModels::setColorMetallic(0.25f, 0.25f, 0.28f);
            glPushMatrix();
            glTranslatef(0, 0.05f, 0);
            LowPolyModels::drawBox(width + 0.4f, 0.1f, depth + 0.8f);
            glPopMatrix();
            
            // Main barrier body - THICKER for visibility
            LowPolyModels::setColorMetallic(0.4f, 0.42f, 0.45f);
            glPushMatrix();
            glTranslatef(0, height * 0.5f, 0);
            LowPolyModels::drawBox(width, height, depth + 0.3f);
            glPopMatrix();
            
            // Top rail - BIGGER and BRIGHTER
            LowPolyModels::setColorMetallic(0.6f, 0.62f, 0.65f);
            glPushMatrix();
            glTranslatef(0, height + 0.08f, 0);
            LowPolyModels::drawBox(width + 0.2f, 0.16f, depth + 0.4f);
            glPopMatrix();
            
            // Highlight strip on top rail
            LowPolyModels::setEmissive(0.1f, 0.1f, 0.15f);
            glPushMatrix();
            glTranslatef(0, height + 0.17f, 0);
            LowPolyModels::drawBox(width, 0.02f, depth + 0.2f);
            glPopMatrix();
            LowPolyModels::clearEmissive();
            
            // Support posts - LARGER
            LowPolyModels::setColorMetallic(0.35f, 0.35f, 0.38f);
            float postX[] = {-width * 0.4f, width * 0.4f};
            for (int i = 0; i < 2; i++) {
                glPushMatrix();
                glTranslatef(postX[i], height * 0.5f, depth * 0.4f);
                LowPolyModels::drawBox(0.2f, height, 0.2f);
                glPopMatrix();
                glPushMatrix();
                glTranslatef(postX[i], height * 0.5f, -depth * 0.4f);
                LowPolyModels::drawBox(0.2f, height, 0.2f);
                glPopMatrix();
            }
        }
        
        // Caution stripes - MORE VISIBLE
        glDisable(GL_LIGHTING);
        glColor3f(0.9f, 0.7f, 0.1f);
        glPushMatrix();
        glTranslatef(0, height * 0.7f, depth * 0.51f);
        glBegin(GL_QUADS);
        for (float x = -width * 0.45f; x < width * 0.4f; x += 0.4f) {
            glVertex3f(x, -0.1f, 0);
            glVertex3f(x + 0.2f, -0.1f, 0);
            glVertex3f(x + 0.3f, 0.1f, 0);
            glVertex3f(x + 0.1f, 0.1f, 0);
        }
        glEnd();
        glPopMatrix();
        glEnable(GL_LIGHTING);
        
        glPopMatrix();
    }
};

// Exit Door for level completion
struct ExitDoor {
    Vector3 position;
    float rotation;
    bool isActive;
    bool isOpen;
    float openAmount;
    float lightIntensity;
    float lightPhase;
    AABB bounds;
    
    ExitDoor() : position(0,0,0), rotation(0.0f), isActive(false), isOpen(false),
                 openAmount(0.0f), lightIntensity(0.0f), lightPhase(0.0f) {
        updateBounds();
    }
    
    void updateBounds() {
        bounds = AABB::fromCenterSize(position + Vector3(0, 1.5f, 0), Vector3(1.5f, 1.5f, 0.5f));
    }
    
    void activate() {
        isActive = true;
    }
    
    void update(float deltaTime) {
        lightPhase += deltaTime * 4.0f;
        
        if (isActive) {
            lightIntensity += deltaTime * 2.0f;
            if (lightIntensity > 1.0f) lightIntensity = 1.0f;
        }
        
        if (isOpen && openAmount < 1.0f) {
            openAmount += deltaTime * 1.5f;
            if (openAmount > 1.0f) openAmount = 1.0f;
        }
    }
    
    bool tryOpen() {
        if (!isActive) return false;
        isOpen = true;
        return true;
    }
    
    void draw() {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation, 0, 1, 0);
        
        // Door frame with texture
        if (TextureManager::isLoaded(TEX_WALL_PANEL)) {
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, -1.3f, 1.5f, 0, 0.2f, 3.0f, 0.3f, 0.5f);
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, 1.3f, 1.5f, 0, 0.2f, 3.0f, 0.3f, 0.5f);
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, 0, 3.1f, 0, 2.8f, 0.2f, 0.3f, 0.5f);
        } else {
            LowPolyModels::setColorMetallic(0.3f, 0.32f, 0.35f);
            glPushMatrix();
            glTranslatef(-1.3f, 1.5f, 0);
            LowPolyModels::drawBox(0.2f, 3.0f, 0.3f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(1.3f, 1.5f, 0);
            LowPolyModels::drawBox(0.2f, 3.0f, 0.3f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(0, 3.1f, 0);
            LowPolyModels::drawBox(2.8f, 0.2f, 0.3f);
            glPopMatrix();
        }
        
        // Door panels (slide open)
        float slideOffset = openAmount * 1.1f;
        
        // Left door panel with texture
        if (TextureManager::isLoaded(TEX_CRATE_SCIFI)) {
            TextureManager::drawTexturedBox(TEX_CRATE_SCIFI, -0.55f - slideOffset, 1.5f, 0, 1.0f, 2.9f, 0.15f, 0.3f);
            TextureManager::drawTexturedBox(TEX_CRATE_SCIFI, 0.55f + slideOffset, 1.5f, 0, 1.0f, 2.9f, 0.15f, 0.3f);
        } else {
            LowPolyModels::setColorMetallic(0.25f, 0.28f, 0.32f);
            glPushMatrix();
            glTranslatef(-0.55f - slideOffset, 1.5f, 0);
            LowPolyModels::drawBox(1.0f, 2.9f, 0.15f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(0.55f + slideOffset, 1.5f, 0);
            LowPolyModels::drawBox(1.0f, 2.9f, 0.15f);
            glPopMatrix();
        }
        
        // Light above door
        if (isActive) {
            float pulse = sin(lightPhase) * 0.2f + 0.8f;
            float intensity = lightIntensity * pulse;
            
            // Light housing
            LowPolyModels::setColorMetallic(0.2f, 0.22f, 0.25f);
            glPushMatrix();
            glTranslatef(0, 3.5f, 0.3f);
            LowPolyModels::drawBox(1.5f, 0.3f, 0.3f);
            glPopMatrix();
            
            // Light glow
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glColor4f(0.2f * intensity, 0.9f * intensity, 0.3f * intensity, 0.8f);
            glPushMatrix();
            glTranslatef(0, 3.5f, 0.5f);
            glutSolidSphere(0.25f * intensity, 8, 8);
            glPopMatrix();
            
            // Arrow indicator on floor
            glColor4f(0.1f * intensity, 0.8f * intensity, 0.2f * intensity, 0.6f);
            glPushMatrix();
            glTranslatef(0, 0.02f, 1.5f);
            glBegin(GL_TRIANGLES);
            glVertex3f(-0.4f, 0, 0.5f);
            glVertex3f(0.4f, 0, 0.5f);
            glVertex3f(0, 0, -0.5f);
            glEnd();
            glPopMatrix();
            
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
        }
        
        glPopMatrix();
    }
    
    // Draw as a mystical victory portal (for Level 2)
    void drawAsPortal() {
        if (!isActive) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation, 0, 1, 0);
        
        float pulse = sin(lightPhase) * 0.2f + 0.8f;
        float fastPulse = sin(lightPhase * 2.0f) * 0.15f + 0.85f;
        float intensity = lightIntensity * pulse;
        
        // ===== Portal Frame (textured stone arch) =====
        glEnable(GL_LIGHTING);
        
        // Left pillar
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, -2.0f, 2.0f, 0, 0.8f, 4.0f, 0.8f, 0.5f);
        } else {
            LowPolyModels::setColor(0.3f, 0.25f, 0.2f);
            glPushMatrix();
            glTranslatef(-2.0f, 2.0f, 0);
            LowPolyModels::drawBox(0.8f, 4.0f, 0.8f);
            glPopMatrix();
        }
        
        // Right pillar
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, 2.0f, 2.0f, 0, 0.8f, 4.0f, 0.8f, 0.5f);
        } else {
            LowPolyModels::setColor(0.3f, 0.25f, 0.2f);
            glPushMatrix();
            glTranslatef(2.0f, 2.0f, 0);
            LowPolyModels::drawBox(0.8f, 4.0f, 0.8f);
            glPopMatrix();
        }
        
        // Top arch
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, 0, 4.3f, 0, 4.8f, 0.6f, 0.8f, 0.5f);
        } else {
            LowPolyModels::setColor(0.35f, 0.28f, 0.22f);
            glPushMatrix();
            glTranslatef(0, 4.3f, 0);
            LowPolyModels::drawBox(4.8f, 0.6f, 0.8f);
            glPopMatrix();
        }
        
        // ===== Portal Energy Effect (swirling glow) =====
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        // Outer ring glow
        glColor4f(0.2f * intensity, 0.8f * intensity, 1.0f * intensity, 0.6f);
        glPushMatrix();
        glTranslatef(0, 2.0f, 0.1f);
        glRotatef(lightPhase * 30.0f, 0, 0, 1);
        glutSolidTorus(0.15f, 1.8f, 12, 24);
        glPopMatrix();
        
        // Inner ring (counter-rotate)
        glColor4f(0.4f * intensity, 0.9f * intensity, 1.0f * intensity, 0.7f);
        glPushMatrix();
        glTranslatef(0, 2.0f, 0.15f);
        glRotatef(-lightPhase * 45.0f, 0, 0, 1);
        glutSolidTorus(0.1f, 1.2f, 10, 20);
        glPopMatrix();
        
        // Center energy core
        glColor4f(0.6f * fastPulse, 0.95f * fastPulse, 1.0f * fastPulse, 0.8f);
        glPushMatrix();
        glTranslatef(0, 2.0f, 0.2f);
        glutSolidSphere(0.5f * fastPulse, 16, 16);
        glPopMatrix();
        
        // Portal surface with texture
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (TextureManager::isLoaded(TEX_LAVA_GLOW)) {
            TextureManager::bind(TEX_LAVA_GLOW);
            glColor4f(0.3f, 0.7f, 1.0f, 0.5f * intensity);
        } else {
            glColor4f(0.1f * intensity, 0.5f * intensity, 0.8f * intensity, 0.4f);
        }
        glPushMatrix();
        glTranslatef(0, 2.0f, 0.05f);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-1.5f, -1.8f, 0);
        glTexCoord2f(1, 0); glVertex3f(1.5f, -1.8f, 0);
        glTexCoord2f(1, 1); glVertex3f(1.5f, 1.8f, 0);
        glTexCoord2f(0, 1); glVertex3f(-1.5f, 1.8f, 0);
        glEnd();
        TextureManager::unbind();
        glPopMatrix();
        
        // Floating runes around portal
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        for (int i = 0; i < 6; i++) {
            float angle = (float)i / 6.0f * 6.28318f + lightPhase * 0.5f;
            float rx = sin(angle) * 2.5f;
            float ry = 2.0f + cos(angle * 2.0f + lightPhase) * 0.5f;
            
            glColor4f(0.5f * fastPulse, 0.9f * fastPulse, 1.0f * fastPulse, 0.7f);
            glPushMatrix();
            glTranslatef(rx, ry, 0.3f);
            glutSolidSphere(0.1f, 6, 6);
            glPopMatrix();
        }
        
        // Ground indicator
        glColor4f(0.2f * intensity, 0.7f * intensity, 1.0f * intensity, 0.5f);
        glPushMatrix();
        glTranslatef(0, 0.05f, 1.5f);
        glRotatef(90, 1, 0, 0);
        glutSolidTorus(0.08f, 0.8f, 8, 16);
        glPopMatrix();
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        glPopMatrix();
    }
};

class Level {
public:
    LevelID levelID;
    
    // Arrays of game objects
    Enemy enemies[MAX_ENEMIES];
    int numEnemies;
    
    Collectible collectibles[MAX_HEALTH_PACKS + MAX_AMMO_BOXES + MAX_KEYCARDS];
    int numCollectibles;
    
    Platform platforms[MAX_PLATFORMS];
    int numPlatforms;
    
    Crate crates[MAX_CRATES];
    int numCrates;
    
    Door doors[MAX_DOORS];
    int numDoors;
    
    // Parkour obstacles
    static const int MAX_PARKOUR_OBSTACLES = 10;
    ParkourObstacle parkourObstacles[MAX_PARKOUR_OBSTACLES];
    int numParkourObstacles;
    
    // Exit door (shown when all enemies killed)
    ExitDoor exitDoor;
    bool allEnemiesKilled;
    bool exitDoorJustActivated; // Flag for Game to play sound
    
    // Boss phase system for Level 2
    bool bossPhaseStarted;      // Has the boss fight begun?
    bool regularEnemiesCleared; // Are all non-boss enemies dead?
    int bossEnemyIndex;         // Index of the boss enemy in array
    bool bossKilledPortalReady; // Boss killed, portal should appear after shake
    
    // Level properties
    Vector3 playerStart;
    Vector3 objective; // Goal position
    float objectiveRadius;
    bool objectiveReached;
    
    // Environment
    float floorSize;
    float wallHeight;
    bool hasLava;
    float lavaHeight;
    
    // For culling optimization
    Vector3 lastPlayerPos;
    float drawDistance; // Objects beyond this distance not drawn
    
    // Timing
    float levelTime;
    float maxTime;
    
    Level() {
        reset();
    }
    
    void reset() {
        levelID = LEVEL_1_FACILITY;
        numEnemies = 0;
        numCollectibles = 0;
        numPlatforms = 0;
        numCrates = 0;
        numDoors = 0;
        numParkourObstacles = 0;
        allEnemiesKilled = false;
        exitDoorJustActivated = false;
        bossPhaseStarted = false;
        regularEnemiesCleared = false;
        bossEnemyIndex = -1;
        bossKilledPortalReady = false;
        
        // Reset exit door
        exitDoor = ExitDoor();
        
        playerStart = Vector3(0, PLAYER_HEIGHT, 0);
        objective = Vector3(0, 0, -25);
        objectiveRadius = 3.0f;
        objectiveReached = false;
        
        floorSize = FLOOR_SIZE;
        wallHeight = WALL_HEIGHT;
        hasLava = false;
        lavaHeight = -2.0f;
        drawDistance = 80.0f; // Culling distance
        
        levelTime = 0.0f;
        maxTime = 300.0f; // 5 minutes
    }
    
    void loadLevel1() {
        DEBUG_LOG("Level::loadLevel1 START\n");
        reset();
        DEBUG_LOG("Level::loadLevel1 reset done\n");
        levelID = LEVEL_1_FACILITY;
        floorSize = 80.0f;   // Larger facility
        wallHeight = 10.0f;
        hasLava = false;
        maxTime = 360.0f;    // 6 minutes
        
        // Player starts in the entrance/security checkpoint area
        playerStart = Vector3(-32, PLAYER_HEIGHT, -32);
        // Objective is in the central reactor core
        objective = Vector3(25, 0, 25);
        
        DEBUG_LOG("Level::loadLevel1 adding enemies\n");
        // ===================================================================
        // ENEMIES - Strategically placed throughout the lab
        // ===================================================================
        numEnemies = 0;
        
        // SECTOR A: Entrance/Security (SW quadrant) - Light resistance
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(-25, 0, -25), Vector3(-30, 0, -25), Vector3(-20, 0, -25));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(-20, 0, -15), Vector3(-25, 0, -15), Vector3(-15, 0, -15));
        numEnemies++;
        
        // SECTOR B: Research Labs (NW quadrant) - Medium resistance
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(-25, 0, 10), Vector3(-30, 0, 10), Vector3(-20, 0, 10));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(-15, 0, 20), Vector3(-20, 0, 20), Vector3(-10, 0, 20));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_DEMON, Vector3(-10, 0, 28), Vector3(-15, 0, 28), Vector3(-5, 0, 28));
        numEnemies++;
        
        // SECTOR C: Containment (SE quadrant) - Heavy resistance
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(20, 0, -20), Vector3(15, 0, -20), Vector3(25, 0, -20));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_DEMON, Vector3(25, 0, -10), Vector3(20, 0, -10), Vector3(30, 0, -10));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(15, 0, -5), Vector3(10, 0, -5), Vector3(20, 0, -5));
        numEnemies++;
        
        // SECTOR D: Reactor Core (NE quadrant) - Boss area
        enemies[numEnemies].init(ENEMY_DEMON, Vector3(15, 0, 15), Vector3(10, 0, 15), Vector3(20, 0, 15));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_DEMON, Vector3(30, 0, 20), Vector3(25, 0, 20), Vector3(35, 0, 20));
        numEnemies++;
        
        // Central corridor patrols
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(0, 0, 0), Vector3(-10, 0, 0), Vector3(10, 0, 0));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(5, 0, 10), Vector3(0, 0, 10), Vector3(10, 0, 10));
        numEnemies++;
        DEBUG_LOG("Level::loadLevel1 enemies done\n");
        
        // ===================================================================
        // COLLECTIBLES - Health, Ammo, Keycards
        // ===================================================================
        DEBUG_LOG("Level::loadLevel1 adding collectibles\n");
        numCollectibles = 0;
        
        // Health packs - in open areas of each zone
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(-25, 0.5f, -25), 25);  // Start area (SW corner)
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(-18, 0.5f, 15), 25);   // Research area (NW)
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(15, 0.5f, -18), 30);  // Containment (SE)
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(0, 0.5f, 0), 25);    // Central hub
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(18, 0.5f, 18), 50);   // Reactor area (NE)
        numCollectibles++;
        
        // Ammo boxes - scattered in open spaces
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(-22, 0.5f, -15), 20);  // Security corridor
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(-12, 0.5f, 10), 20);   // Near research
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(12, 0.5f, -22), 25);   // Containment entrance
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(22, 0.5f, 8), 25);    // Reactor corridor
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(8, 0.5f, 22), 30);    // North corridor
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(-8, 0.5f, 25), 20);   // Research exit
        numCollectibles++;
        
        // Keycards - in clearly reachable locations
        collectibles[numCollectibles].init(COLLECT_KEYCARD, Vector3(-22, 0.5f, 18), 1);  // Blue keycard - research lab
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_KEYCARD, Vector3(22, 0.5f, -18), 2);  // Red keycard - containment
        numCollectibles++;
        DEBUG_LOG("Level::loadLevel1 collectibles done\n");
        
        // ===================================================================
        // PLATFORMS - Catwalks, observation decks, server room platforms
        // ===================================================================
        DEBUG_LOG("Level::loadLevel1 adding platforms\n");
        numPlatforms = 0;
        
        // Central observation platform (overlooks main corridor)
        platforms[numPlatforms] = Platform(Vector3(0, 2.5f, 0), Vector3(6, 0.4f, 6));
        numPlatforms++;
        
        // Access ramp to central platform
        platforms[numPlatforms] = Platform(Vector3(-5, 1.2f, 0), Vector3(4, 0.4f, 3));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(-8, 0.6f, 0), Vector3(2, 0.4f, 3));
        numPlatforms++;
        
        // Research lab elevated walkway
        platforms[numPlatforms] = Platform(Vector3(-25, 2.0f, 15), Vector3(8, 0.4f, 3));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(-25, 2.0f, 25), Vector3(8, 0.4f, 3));
        numPlatforms++;
        
        // Containment observation deck
        platforms[numPlatforms] = Platform(Vector3(25, 3.0f, -15), Vector3(6, 0.4f, 4));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(20, 1.5f, -15), Vector3(4, 0.4f, 4));
        numPlatforms++;
        
        // Reactor control platforms
        platforms[numPlatforms] = Platform(Vector3(20, 2.0f, 20), Vector3(5, 0.4f, 5));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(30, 1.0f, 25), Vector3(4, 0.4f, 4));
        numPlatforms++;
        
        // Server room platforms
        platforms[numPlatforms] = Platform(Vector3(10, 1.5f, -25), Vector3(4, 0.4f, 4));
        numPlatforms++;
        
        // ===================================================================
        // CRATES - Lab equipment, server racks, supply containers
        // Some are MYSTERY BOXES that can contain health, ammo, or nothing
        // ===================================================================
        numCrates = 0;
        
        // Entrance security checkpoint crates
        float cratePositions[][4] = {
            // Security checkpoint (SW)
            {-30, 0, -30, 1.0f}, {-28, 0, -32, 1.2f}, {-35, 0, -25, 1.0f},
            // Research lab storage (NW)
            {-30, 0, 15, 1.2f}, {-32, 0, 20, 1.0f}, {-28, 0, 28, 1.5f}, {-20, 0, 30, 1.0f},
            // Containment area (SE)
            {25, 0, -30, 1.2f}, {30, 0, -28, 1.0f}, {20, 0, -25, 1.3f}, {32, 0, -18, 1.0f},
            // Reactor area (NE)
            {30, 0, 15, 1.0f}, {35, 0, 22, 1.2f}, {28, 0, 30, 1.0f},
            // Central corridor
            {-10, 0, 5, 1.0f}, {10, 0, -5, 1.2f}, {-5, 0, -10, 1.0f}, {5, 0, 8, 1.0f},
            // Server room
            {15, 0, -30, 1.3f}, {8, 0, -28, 1.0f},
        };
        
        // Mystery box indices (these crates become mystery boxes)
        int mysteryBoxIndices[] = {0, 3, 7, 10, 14, 17};
        int numMysteryBoxes = 6;
        
        for (int i = 0; i < 20 && numCrates < MAX_CRATES; i++) {
            crates[numCrates].position = Vector3(cratePositions[i][0], cratePositions[i][1], cratePositions[i][2]);
            crates[numCrates].size = cratePositions[i][3];
            crates[numCrates].isSciFi = true;  // All sci-fi crates in the lab
            crates[numCrates].updateBounds();
            
            // Check if this should be a mystery box
            for (int j = 0; j < numMysteryBoxes; j++) {
                if (mysteryBoxIndices[j] == i) {
                    crates[numCrates].setAsMysteryBox();
                    break;
                }
            }
            numCrates++;
        }
        DEBUG_LOG("Level::loadLevel1 crates done\n");
        
        // ===================================================================
        // PARKOUR OBSTACLES - Barriers for vaulting (press E)
        // ===================================================================
        DEBUG_LOG("Level::loadLevel1 adding parkour obstacles\n");
        numParkourObstacles = 0;
        
        // Barrier between Security and Research
        parkourObstacles[numParkourObstacles].position = Vector3(-25, 0, 2);
        parkourObstacles[numParkourObstacles].width = 4.0f;
        parkourObstacles[numParkourObstacles].height = 1.2f;
        parkourObstacles[numParkourObstacles].depth = 0.5f;
        parkourObstacles[numParkourObstacles].rotation = 90.0f;
        parkourObstacles[numParkourObstacles].updateBounds();
        numParkourObstacles++;
        
        // Barrier in central corridor
        parkourObstacles[numParkourObstacles].position = Vector3(0, 0, -8);
        parkourObstacles[numParkourObstacles].width = 5.0f;
        parkourObstacles[numParkourObstacles].height = 1.0f;
        parkourObstacles[numParkourObstacles].depth = 0.4f;
        parkourObstacles[numParkourObstacles].rotation = 0.0f;
        parkourObstacles[numParkourObstacles].updateBounds();
        numParkourObstacles++;
        
        // Barrier between Containment and Reactor
        parkourObstacles[numParkourObstacles].position = Vector3(25, 0, 2);
        parkourObstacles[numParkourObstacles].width = 4.0f;
        parkourObstacles[numParkourObstacles].height = 1.2f;
        parkourObstacles[numParkourObstacles].depth = 0.5f;
        parkourObstacles[numParkourObstacles].rotation = 90.0f;
        parkourObstacles[numParkourObstacles].updateBounds();
        numParkourObstacles++;
        
        // Barrier near entrance
        parkourObstacles[numParkourObstacles].position = Vector3(-15, 0, -20);
        parkourObstacles[numParkourObstacles].width = 3.5f;
        parkourObstacles[numParkourObstacles].height = 1.1f;
        parkourObstacles[numParkourObstacles].depth = 0.4f;
        parkourObstacles[numParkourObstacles].rotation = 45.0f;
        parkourObstacles[numParkourObstacles].updateBounds();
        numParkourObstacles++;
        
        DEBUG_LOG("Level::loadLevel1 parkour obstacles done\n");
        
        // ===================================================================
        // EXIT DOOR - Appears when all enemies are killed
        // Position at a clear spot near the edge of the map, NOT on platforms
        // ===================================================================
        exitDoor.position = Vector3(0, 0, -38);  // Near back wall, center
        exitDoor.rotation = 0.0f;  // Facing player
        exitDoor.isActive = false;
        exitDoor.updateBounds();
        DEBUG_LOG("Level::loadLevel1 exit door done\n");
        
        // ===================================================================
        // DOORS - Security doors throughout facility
        // ===================================================================
        DEBUG_LOG("Level::loadLevel1 adding doors\n");
        numDoors = 0;
        
        // Research lab access (requires blue keycard)
        doors[numDoors].position = Vector3(-15, 0, 10);
        doors[numDoors].rotation = 0.0f;
        doors[numDoors].isLocked = true;
        doors[numDoors].requiredKeycard = 1;
        doors[numDoors].updateBounds();
        numDoors++;
        
        // Reactor core access (requires red keycard)
        doors[numDoors].position = Vector3(15, 0, 15);
        doors[numDoors].rotation = 90.0f;
        doors[numDoors].isLocked = true;
        doors[numDoors].requiredKeycard = 2;
        doors[numDoors].updateBounds();
        numDoors++;
        
        // Containment to central corridor
        doors[numDoors].position = Vector3(10, 0, -10);
        doors[numDoors].rotation = 0.0f;
        doors[numDoors].isLocked = false;
        doors[numDoors].updateBounds();
        numDoors++;
        DEBUG_LOG("Level::loadLevel1 COMPLETE\n");
    }
    
    void loadLevel2() {
        reset();
        levelID = LEVEL_2_HELL_ARENA;
        
        // ===================================================================
        // LEVEL 2: HELL ARENA - Compact, well-designed arena
        // ===================================================================
        floorSize = 80.0f;       // Smaller, tighter arena
        wallHeight = 0.0f;        // Outdoor - no walls
        hasLava = true;
        lavaHeight = 0.0f;        // Lava at floor level - platforms float above it
        maxTime = 480.0f;         // 8 minutes
        drawDistance = 100.0f;
        
        // Player starts on the main arena platform (elevated starting platform)
        playerStart = Vector3(0, PLAYER_HEIGHT + 2.0f, -10);
        objective = Vector3(0, 5, 25);
        
        // ===================================================================
        // PLATFORMS - Simple, connected arena design
        // ===================================================================
        numPlatforms = 0;
        
        // MAIN ARENA FLOOR - Large central platform floating above lava
        platforms[numPlatforms] = Platform(Vector3(0, 1.5f, 0), Vector3(35, 1.0f, 35));
        numPlatforms++;
        
        // ELEVATED CORNERS - 4 raised platforms around the arena
        platforms[numPlatforms] = Platform(Vector3(-20, 3.0f, -20), Vector3(8, 1.0f, 8));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(20, 3.0f, -20), Vector3(8, 1.0f, 8));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(-20, 3.0f, 20), Vector3(8, 1.0f, 8));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(20, 3.0f, 20), Vector3(8, 1.0f, 8));
        numPlatforms++;
        
        // RAMPS connecting corners to main floor (easy traversal)
        platforms[numPlatforms] = Platform(Vector3(-15, 2.2f, -15), Vector3(5, 0.5f, 5));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(15, 2.2f, -15), Vector3(5, 0.5f, 5));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(-15, 2.2f, 15), Vector3(5, 0.5f, 5));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(15, 2.2f, 15), Vector3(5, 0.5f, 5));
        numPlatforms++;
        
        // BOSS PLATFORM - Elevated throne at the back
        platforms[numPlatforms] = Platform(Vector3(0, 4.5f, 30), Vector3(15, 1.5f, 10));
        numPlatforms++;
        // Steps leading to boss platform
        platforms[numPlatforms] = Platform(Vector3(0, 2.5f, 22), Vector3(10, 0.8f, 5));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(0, 3.5f, 26), Vector3(12, 0.8f, 5));
        numPlatforms++;
        
        // SIDE WALKWAYS - For strategic movement
        platforms[numPlatforms] = Platform(Vector3(-30, 2.0f, 0), Vector3(5, 0.6f, 20));
        numPlatforms++;
        platforms[numPlatforms] = Platform(Vector3(30, 2.0f, 0), Vector3(5, 0.6f, 20));
        numPlatforms++;
        
        // ===================================================================
        // ENEMIES - Phase 1: Regular enemies, Phase 2: Boss spawns
        // ===================================================================
        numEnemies = 0;
        
        // REGULAR ENEMIES (10 total - clear these first)
        // Main arena zombies (on main platform at Y=1.5)
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(-10, 2.0f, -5), Vector3(-15, 2.0f, -5), Vector3(-5, 2.0f, -5));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(10, 2.0f, -5), Vector3(5, 2.0f, -5), Vector3(15, 2.0f, -5));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(0, 2.0f, 10), Vector3(-5, 2.0f, 10), Vector3(5, 2.0f, 10));
        numEnemies++;
        
        // Corner platform demons (on corner platforms at Y=3.0)
        enemies[numEnemies].init(ENEMY_DEMON, Vector3(-20, 3.5f, -20), Vector3(-23, 3.5f, -20), Vector3(-17, 3.5f, -20));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_DEMON, Vector3(20, 3.5f, -20), Vector3(17, 3.5f, -20), Vector3(23, 3.5f, -20));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_DEMON, Vector3(-20, 3.5f, 20), Vector3(-23, 3.5f, 20), Vector3(-17, 3.5f, 20));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_DEMON, Vector3(20, 3.5f, 20), Vector3(17, 3.5f, 20), Vector3(23, 3.5f, 20));
        numEnemies++;
        
        // Side walkway enemies (on side walkways at Y=2.0)
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(-30, 2.5f, 5), Vector3(-30, 2.5f, 0), Vector3(-30, 2.5f, 10));
        numEnemies++;
        enemies[numEnemies].init(ENEMY_ZOMBIE, Vector3(30, 2.5f, -5), Vector3(30, 2.5f, -10), Vector3(30, 2.5f, 0));
        numEnemies++;
        // Enemy near player - on main platform
        enemies[numEnemies].init(ENEMY_DEMON, Vector3(0, 2.0f, LEVEL2_ENEMY_SPAWN_DISTANCE), 
                                 Vector3(-5, 2.0f, LEVEL2_ENEMY_SPAWN_DISTANCE), 
                                 Vector3(5, 2.0f, LEVEL2_ENEMY_SPAWN_DISTANCE));
        numEnemies++;
        
        // THE DEMON KING BOSS - Starts inactive, spawns after clearing regular enemies
        // Platform center Y=4.5, half-height=0.75, top surface = 5.25
        // Boss needs to stand ON the platform
        bossEnemyIndex = numEnemies;
        enemies[numEnemies].init(ENEMY_BOSS, Vector3(0, 5.25f, 30), Vector3(-5, 5.25f, 30), Vector3(5, 5.25f, 30));
        enemies[numEnemies].active = false; // Boss starts INACTIVE
        numEnemies++;
        
        // ===================================================================
        // COLLECTIBLES - Health and Ammo strategically placed ON PLATFORMS
        // ===================================================================
        numCollectibles = 0;
        
        // Starting supplies (on main platform center, Y=1.5 + 0.5 offset = 2.0)
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(-8, 2.5f, -15), 30);
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(8, 2.5f, -15), 40);
        numCollectibles++;
        
        // Corner platform pickups (corner platforms at Y=3.0, so Y=3.5 for items)
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(-20, 4.0f, -20), 35);  // SW corner
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(20, 4.0f, -20), 35);    // SE corner
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(-20, 4.0f, 20), 35);   // NW corner
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(20, 4.0f, 20), 35);     // NE corner
        numCollectibles++;
        
        // Main arena center pickups (main platform Y=1.5, items at Y=2.5)
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(0, 2.5f, 5), 50);
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(-10, 2.5f, 0), 50);
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_AMMO, Vector3(10, 2.5f, 0), 50);
        numCollectibles++;
        
        // Boss area supplies (boss platform Y=4.5, items at Y=5.5)
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(-6, 6.0f, 30), 75);
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(6, 6.0f, 30), 75);
        numCollectibles++;
        
        // Side walkway powerups (walkways at Y=2.0, items at Y=2.5)
        collectibles[numCollectibles].init(COLLECT_DAMAGE_BOOST, Vector3(-30, 3.0f, -5), 12);
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_SPEED_BOOST, Vector3(30, 3.0f, 5), 12);
        numCollectibles++;
        
        // Step platforms before boss (steps at Y=2.5 and Y=3.5)
        collectibles[numCollectibles].init(COLLECT_INVINCIBILITY, Vector3(0, 4.0f, 24), 10);
        numCollectibles++;
        
        // SHIELD - on corner platforms for boss fight prep
        collectibles[numCollectibles].init(COLLECT_SHIELD, Vector3(-18, 4.0f, 18), 100);
        numCollectibles++;
        collectibles[numCollectibles].init(COLLECT_SHIELD, Vector3(18, 4.0f, -18), 100);
        numCollectibles++;
        
        // ===================================================================
        // ROCKS/CRATES - Cover (keep the drawn boxes, they look cool)
        // ===================================================================
        numCrates = 0;
        
        // Cover rocks on main arena (on main platform at Y=1.5)
        crates[numCrates].position = Vector3(-12, 1.5f, 5);
        crates[numCrates].size = 2.0f;
        crates[numCrates].isSciFi = false;
        crates[numCrates].updateBounds();
        numCrates++;
        
        crates[numCrates].position = Vector3(12, 1.5f, -5);
        crates[numCrates].size = 2.0f;
        crates[numCrates].isSciFi = false;
        crates[numCrates].updateBounds();
        numCrates++;
        
        crates[numCrates].position = Vector3(0, 1.5f, 12);
        crates[numCrates].size = 1.8f;
        crates[numCrates].isSciFi = false;
        crates[numCrates].updateBounds();
        numCrates++;
        
        // Additional cover rocks
        crates[numCrates].position = Vector3(-8, 1.5f, -8);
        crates[numCrates].size = 1.5f;
        crates[numCrates].isSciFi = false;
        crates[numCrates].updateBounds();
        numCrates++;
        
        crates[numCrates].position = Vector3(8, 1.5f, 8);
        crates[numCrates].size = 1.6f;
        crates[numCrates].isSciFi = false;
        crates[numCrates].updateBounds();
        numCrates++;
        
        // ===================================================================
        // MYSTERY BOXES - Same style as Level 1 (sci-fi crates)
        // ===================================================================
        crates[numCrates].position = Vector3(-18, 3.5f, -18);
        crates[numCrates].size = 1.2f;
        crates[numCrates].isSciFi = true;
        crates[numCrates].setAsMysteryBox();
        crates[numCrates].updateBounds();
        numCrates++;
        
        crates[numCrates].position = Vector3(18, 3.5f, 18);
        crates[numCrates].size = 1.2f;
        crates[numCrates].isSciFi = true;
        crates[numCrates].setAsMysteryBox();
        crates[numCrates].updateBounds();
        numCrates++;
        
        crates[numCrates].position = Vector3(-15, 2.5f, 0);
        crates[numCrates].size = 1.2f;
        crates[numCrates].isSciFi = true;
        crates[numCrates].setAsMysteryBox();
        crates[numCrates].updateBounds();
        numCrates++;
        
        crates[numCrates].position = Vector3(15, 2.5f, 0);
        crates[numCrates].size = 1.2f;
        crates[numCrates].isSciFi = true;
        crates[numCrates].setAsMysteryBox();
        crates[numCrates].updateBounds();
        numCrates++;
        
        numDoors = 0;
        
        // ===================================================================
        // PARKOUR OBSTACLES - Barriers on main platform for vaulting (press E)
        // Main platform is at Y=1.5, extends from -17.5 to +17.5 in X and Z
        // Avoiding rocks at: (-12,5), (12,-5), (0,12), (-8,-8), (8,8)
        // ===================================================================
        numParkourObstacles = 0;
        
        // Barrier at front-left of platform (away from rocks)
        parkourObstacles[numParkourObstacles].position = Vector3(-14, 2.0f, -12);
        parkourObstacles[numParkourObstacles].width = 5.0f;
        parkourObstacles[numParkourObstacles].height = 1.2f;
        parkourObstacles[numParkourObstacles].depth = 0.5f;
        parkourObstacles[numParkourObstacles].rotation = 30.0f;
        parkourObstacles[numParkourObstacles].updateBounds();
        numParkourObstacles++;
        
        // Barrier at front-right of platform
        parkourObstacles[numParkourObstacles].position = Vector3(14, 2.0f, -12);
        parkourObstacles[numParkourObstacles].width = 5.0f;
        parkourObstacles[numParkourObstacles].height = 1.0f;
        parkourObstacles[numParkourObstacles].depth = 0.4f;
        parkourObstacles[numParkourObstacles].rotation = -30.0f;
        parkourObstacles[numParkourObstacles].updateBounds();
        numParkourObstacles++;
        
        // Barrier in center-back area (between rocks)
        parkourObstacles[numParkourObstacles].position = Vector3(5, 2.0f, -2);
        parkourObstacles[numParkourObstacles].width = 4.0f;
        parkourObstacles[numParkourObstacles].height = 1.0f;
        parkourObstacles[numParkourObstacles].depth = 0.4f;
        parkourObstacles[numParkourObstacles].rotation = 0.0f;
        parkourObstacles[numParkourObstacles].updateBounds();
        numParkourObstacles++;
        
        // Barrier on left side near edge
        parkourObstacles[numParkourObstacles].position = Vector3(-5, 2.0f, -2);
        parkourObstacles[numParkourObstacles].width = 4.0f;
        parkourObstacles[numParkourObstacles].height = 1.1f;
        parkourObstacles[numParkourObstacles].depth = 0.5f;
        parkourObstacles[numParkourObstacles].rotation = 0.0f;
        parkourObstacles[numParkourObstacles].updateBounds();
        numParkourObstacles++;
        
        // ===================================================================
        // EXIT PORTAL - Appears after killing boss (on boss platform)
        // Boss platform is at Y=4.5, so portal at Y=5.5 (on top of platform)
        // ===================================================================
        exitDoor.position = Vector3(0, 5.5f, 28);  // On boss platform, near front
        exitDoor.rotation = 180.0f;  // Face the player coming from main arena
        exitDoor.isActive = false;
        exitDoor.updateBounds();
    }
    
    void update(float deltaTime, const Vector3& playerPos) {
        levelTime += deltaTime;
        lastPlayerPos = playerPos; // Store for culling
        
        // Update enemies
        for (int i = 0; i < numEnemies; i++) {
            enemies[i].update(deltaTime, playerPos);
            
            // Enemy collision with crates
            if (enemies[i].active && !enemies[i].isDead()) {
                Sphere enemySphere(enemies[i].position, 0.8f);
                for (int j = 0; j < numCrates; j++) {
                    CollisionResult result = Collision::resolveSphereAABB(enemySphere, crates[j].bounds);
                    if (result.hit) {
                        enemies[i].position = enemies[i].position + result.normal * result.penetration;
                    }
                }
                
                // Enemy collision with parkour obstacles
                for (int j = 0; j < numParkourObstacles; j++) {
                    CollisionResult result = Collision::resolveSphereAABB(enemySphere, parkourObstacles[j].bounds);
                    if (result.hit) {
                        enemies[i].position = enemies[i].position + result.normal * result.penetration;
                    }
                }
                
                // Enemy collision with interior walls
                checkInteriorWallCollision(enemies[i].position, 0.8f);
            }
        }
        
        // Update collectibles
        for (int i = 0; i < numCollectibles; i++) {
            collectibles[i].update(deltaTime);
        }
        
        // Update crates (including mystery boxes)
        for (int i = 0; i < numCrates; i++) {
            crates[i].update(deltaTime);
        }
        
        // Update doors
        for (int i = 0; i < numDoors; i++) {
            doors[i].update(deltaTime);
        }
        
        // Update exit door
        exitDoor.update(deltaTime);
        
        // ===================================================================
        // LEVEL 2 TWO-PHASE BOSS SYSTEM
        // Phase 1: Kill all regular enemies -> Boss spawns
        // Phase 2: Kill boss -> Win
        // ===================================================================
        if (levelID == LEVEL_2_HELL_ARENA) {
            // Phase 1: Check if all regular enemies (non-boss) are dead
            if (!regularEnemiesCleared && !bossPhaseStarted) {
                bool anyRegularAlive = false;
                for (int i = 0; i < numEnemies; i++) {
                    // Skip the boss enemy
                    if (i == bossEnemyIndex) continue;
                    if (enemies[i].active && !enemies[i].isDead()) {
                        anyRegularAlive = true;
                        break;
                    }
                }
                
                if (!anyRegularAlive) {
                    regularEnemiesCleared = true;
                    bossPhaseStarted = true;
                    exitDoorJustActivated = true; // Flag for Game to show "BOSS INCOMING" and play sound
                    
                    // SPAWN THE BOSS!
                    if (bossEnemyIndex >= 0 && bossEnemyIndex < numEnemies) {
                        enemies[bossEnemyIndex].active = true;
                    }
                }
            }
            
            // Phase 2: Check if boss is dead
            if (bossPhaseStarted && !allEnemiesKilled) {
                if (bossEnemyIndex >= 0 && bossEnemyIndex < numEnemies) {
                    if (enemies[bossEnemyIndex].isDead()) {
                        allEnemiesKilled = true;
                        bossKilledPortalReady = true; // Triggers victory shake, then portal appears
                    }
                }
            }
        }
        // ===================================================================
        // LEVEL 1 STANDARD LOGIC
        // ===================================================================
        else if (!allEnemiesKilled) {
            bool anyAlive = false;
            for (int i = 0; i < numEnemies; i++) {
                if (enemies[i].active && !enemies[i].isDead()) {
                    anyAlive = true;
                    break;
                }
            }
            if (!anyAlive && numEnemies > 0) {
                allEnemiesKilled = true;
                exitDoorJustActivated = true;
                exitDoor.activate();
            }
        }
        
        // Check objective - only for Level 1, complete when at exit door after all enemies killed
        if (levelID == LEVEL_1_FACILITY && allEnemiesKilled && exitDoor.isOpen) {
            float distToExit = playerPos.distanceTo(exitDoor.position);
            if (distToExit < 2.5f) {
                objectiveReached = true;
            }
        }
    }
    
    // Check if boss phase has started (for HUD messages)
    bool isBossPhaseActive() const {
        return bossPhaseStarted && !allEnemiesKilled;
    }
    
    // Check if regular enemies are cleared (boss about to spawn)
    bool areRegularEnemiesCleared() const {
        return regularEnemiesCleared;
    }
    
    // Get nearest interactable object for E key
    // Returns: 0 = nothing, 1 = mystery box, 2 = parkour obstacle, 3 = exit door
    int getNearestInteractable(const Vector3& playerPos, int& outIndex, float maxDist = 2.5f) {
        float closestDist = maxDist;
        int closestType = 0;
        int closestIndex = -1;
        
        // Check mystery boxes
        for (int i = 0; i < numCrates; i++) {
            if (crates[i].isMysteryBox && !crates[i].isOpened) {
                float dist = playerPos.distanceTo(crates[i].position + Vector3(0, 0.5f, 0));
                if (dist < closestDist) {
                    closestDist = dist;
                    closestType = 1;
                    closestIndex = i;
                }
            }
            // Also check opened boxes with uncollected content
            if (crates[i].isMysteryBox && crates[i].isOpened && !crates[i].contentCollected && crates[i].openAnimProgress > 0.8f) {
                float dist = playerPos.distanceTo(crates[i].position + Vector3(0, 0.5f, 0));
                if (dist < closestDist) {
                    closestDist = dist;
                    closestType = 1;
                    closestIndex = i;
                }
            }
        }
        
        // Check parkour obstacles
        for (int i = 0; i < numParkourObstacles; i++) {
            if (parkourObstacles[i].isPlayerNearForVault(playerPos, PLAYER_COLLISION_RADIUS)) {
                float dist = playerPos.distanceTo(parkourObstacles[i].position);
                if (dist < closestDist) {
                    closestDist = dist;
                    closestType = 2;
                    closestIndex = i;
                }
            }
        }
        
        // Check exit door
        if (exitDoor.isActive && !exitDoor.isOpen) {
            float dist = playerPos.distanceTo(exitDoor.position);
            if (dist < closestDist) {
                closestDist = dist;
                closestType = 3;
                closestIndex = 0;
            }
        }
        
        outIndex = closestIndex;
        return closestType;
    }
    
    // Check parkour obstacle collision
    bool checkParkourObstacleCollision(Vector3& playerPos, float playerRadius) {
        bool collided = false;
        Sphere playerSphere(playerPos, playerRadius);
        
        for (int i = 0; i < numParkourObstacles; i++) {
            CollisionResult result = Collision::resolveSphereAABB(playerSphere, parkourObstacles[i].bounds);
            if (result.hit) {
                playerPos = playerPos + result.normal * result.penetration;
                collided = true;
            }
        }
        
        return collided;
    }
    
    // Check if player is on a platform, returns ground height
    float checkPlatformCollision(const Vector3& playerPos, float playerRadius) {
        float groundHeight = 0.0f;
        
        for (int i = 0; i < numPlatforms; i++) {
            float platformGround;
            if (platforms[i].isPlayerOnTop(playerPos, playerRadius, platformGround)) {
                if (platformGround > groundHeight) {
                    groundHeight = platformGround;
                }
            }
        }
        
        return groundHeight;
    }
    
    // Check crate collisions and return push direction
    bool checkCrateCollision(Vector3& playerPos, float playerRadius) {
        bool collided = false;
        Sphere playerSphere(playerPos, playerRadius);
        
        for (int i = 0; i < numCrates; i++) {
            CollisionResult result = Collision::resolveSphereAABB(playerSphere, crates[i].bounds);
            if (result.hit) {
                playerPos = playerPos + result.normal * result.penetration;
                collided = true;
            }
        }
        
        return collided;
    }
    
    // Check door collision
    bool checkDoorCollision(Vector3& playerPos, float playerRadius, int* keycards, int numKeycards) {
        Sphere playerSphere(playerPos, playerRadius);
        
        for (int i = 0; i < numDoors; i++) {
            if (!doors[i].isOpen) {
                // Check if player has keycard
                if (doors[i].isLocked) {
                    bool hasKey = false;
                    for (int k = 0; k < numKeycards; k++) {
                        if (keycards[k] == doors[i].requiredKeycard) {
                            hasKey = true;
                            break;
                        }
                    }
                    if (hasKey) {
                        doors[i].isLocked = false;
                        doors[i].isOpen = true;
                    }
                } else {
                    doors[i].isOpen = true;
                }
                
                // Block if still locked
                if (!doors[i].isOpen) {
                    CollisionResult result = Collision::resolveSphereAABB(playerSphere, doors[i].bounds);
                    if (result.hit) {
                        playerPos = playerPos + result.normal * result.penetration;
                        return true;
                    }
                }
            }
        }
        
        return false;
    }
    
    // Check interior wall collisions for player/enemies
    bool checkInteriorWallCollision(Vector3& pos, float radius) {
        // Only check interior walls in facility level
        if (levelID != LEVEL_1_FACILITY) return false;
        
        bool collided = false;
        Sphere entity(pos, radius);
        float halfFloor = floorSize / 2.0f;
        
        // Interior wall AABBs - simplified collision volumes
        // These match the visual walls drawn in drawFacilityWalls()
        AABB interiorWalls[] = {
            // === OUTER ARENA WALLS (prevent walking through boundary) ===
            // North wall (back wall at z = -halfFloor)
            AABB::fromCenter(Vector3(0, wallHeight/2, -halfFloor), Vector3(halfFloor, wallHeight/2, 1.0f)),
            // South wall (front wall at z = +halfFloor)
            AABB::fromCenter(Vector3(0, wallHeight/2, halfFloor), Vector3(halfFloor, wallHeight/2, 1.0f)),
            // East wall (right wall at x = +halfFloor)
            AABB::fromCenter(Vector3(halfFloor, wallHeight/2, 0), Vector3(1.0f, wallHeight/2, halfFloor)),
            // West wall (left wall at x = -halfFloor)
            AABB::fromCenter(Vector3(-halfFloor, wallHeight/2, 0), Vector3(1.0f, wallHeight/2, halfFloor)),
            
            // === INTERIOR WALLS ===
            // Security (SW) - East wall
            AABB::fromCenter(Vector3(-5, wallHeight/2, -20), Vector3(1.0f, wallHeight/2, 10)),
            // Security (SW) - North wall
            AABB::fromCenter(Vector3(-20, wallHeight/2, -5), Vector3(7.5f, wallHeight/2, 1.0f)),
            // Research (NW) - South wall  
            AABB::fromCenter(Vector3(-25, wallHeight/2, 5), Vector3(9, wallHeight/2, 1.0f)),
            // Research (NW) - East wall
            AABB::fromCenter(Vector3(-5, wallHeight/2, 20), Vector3(1.0f, wallHeight/2, 10)),
            // Containment (SE) - West wall
            AABB::fromCenter(Vector3(5, wallHeight/2, -20), Vector3(1.0f, wallHeight/2, 10)),
            // Containment (SE) - North wall
            AABB::fromCenter(Vector3(20, wallHeight/2, -5), Vector3(7.5f, wallHeight/2, 1.0f)),
            // Reactor (NE) - South wall
            AABB::fromCenter(Vector3(25, wallHeight/2, 5), Vector3(6, wallHeight/2, 1.0f)),
            // Reactor (NE) - West wall
            AABB::fromCenter(Vector3(5, wallHeight/2, 20), Vector3(1.0f, wallHeight/2, 8))
        };
        
        int numWalls = sizeof(interiorWalls) / sizeof(AABB);
        for (int i = 0; i < numWalls; i++) {
            CollisionResult result = Collision::resolveSphereAABB(entity, interiorWalls[i]);
            if (result.hit) {
                pos = pos + result.normal * result.penetration;
                collided = true;
            }
        }
        
        return collided;
    }
    
    void drawFloor() {
        if (levelID == LEVEL_1_FACILITY) {
            drawFacilityFloor();
        } else {
            drawHellFloor();
        }
    }
    
    void drawFacilityFloor() {
        glPushMatrix();
        
        float halfSize = floorSize / 2.0f;
        
        // =====================================================
        // CHECKERBOARD FLOOR - Alternating tiles for visual interest
        // =====================================================
        
        // Temporarily disable face culling for floor (draw both sides)
        glDisable(GL_CULL_FACE);
        
        // Set material properties for lighting
        GLfloat floorDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
        GLfloat floorAmbient[] = {0.8f, 0.8f, 0.8f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, floorDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, floorAmbient);
        
        // Draw checkerboard pattern with two different tile textures
        float tileSize = 4.0f;  // Each tile is 4x4 units
        int numTiles = (int)(floorSize / tileSize);
        
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        for (int i = 0; i < numTiles; i++) {
            for (int j = 0; j < numTiles; j++) {
                // Checkerboard pattern - alternate textures
                bool isDark = ((i + j) % 2 == 0);
                
                if (isDark) {
                    TextureManager::bind(TEX_FLOOR_TILE);  // Tile texture
                    glColor3f(0.9f, 0.9f, 0.95f);  // Slightly tinted
                } else {
                    TextureManager::bind(TEX_FLOOR_LAB);   // Lab floor texture
                    glColor3f(1.0f, 1.0f, 1.0f);  // Full brightness
                }
                
                float x0 = -halfSize + i * tileSize;
                float z0 = -halfSize + j * tileSize;
                float x1 = x0 + tileSize;
                float z1 = z0 + tileSize;
                
                glBegin(GL_QUADS);
                glNormal3f(0, 1, 0);
                glTexCoord2f(0, 0); glVertex3f(x0, 0.01f, z0);
                glTexCoord2f(1, 0); glVertex3f(x1, 0.01f, z0);
                glTexCoord2f(1, 1); glVertex3f(x1, 0.01f, z1);
                glTexCoord2f(0, 1); glVertex3f(x0, 0.01f, z1);
                glEnd();
            }
        }
        
        TextureManager::unbind();
        glColor3f(1.0f, 1.0f, 1.0f);  // Reset color
        
        // Re-enable face culling
        glEnable(GL_CULL_FACE);
        
        // Draw sector markings on floor
        glDisable(GL_LIGHTING);
        
        // Sector A - Security (SW) - Yellow marking
        glColor3f(0.8f, 0.7f, 0.2f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-halfSize + 1, 0.02f, -halfSize + 1);
        glVertex3f(-5, 0.02f, -halfSize + 1);
        glVertex3f(-5, 0.02f, 5);
        glVertex3f(-halfSize + 1, 0.02f, 5);
        glEnd();
        
        // Sector B - Research (NW) - Blue marking
        glColor3f(0.2f, 0.5f, 0.9f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-halfSize + 1, 0.02f, 5);
        glVertex3f(-5, 0.02f, 5);
        glVertex3f(-5, 0.02f, halfSize - 1);
        glVertex3f(-halfSize + 1, 0.02f, halfSize - 1);
        glEnd();
        
        // Sector C - Containment (SE) - Red marking
        glColor3f(0.9f, 0.2f, 0.2f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(5, 0.02f, -halfSize + 1);
        glVertex3f(halfSize - 1, 0.02f, -halfSize + 1);
        glVertex3f(halfSize - 1, 0.02f, 5);
        glVertex3f(5, 0.02f, 5);
        glEnd();
        
        // Sector D - Reactor (NE) - Green marking
        glColor3f(0.2f, 0.9f, 0.3f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(5, 0.02f, 5);
        glVertex3f(halfSize - 1, 0.02f, 5);
        glVertex3f(halfSize - 1, 0.02f, halfSize - 1);
        glVertex3f(5, 0.02f, halfSize - 1);
        glEnd();
        
        // Central corridor markings
        glColor3f(0.4f, 0.4f, 0.5f);
        glLineWidth(2.0f);
        // Horizontal corridor
        glBegin(GL_LINES);
        glVertex3f(-halfSize + 1, 0.02f, -2);
        glVertex3f(halfSize - 1, 0.02f, -2);
        glVertex3f(-halfSize + 1, 0.02f, 2);
        glVertex3f(halfSize - 1, 0.02f, 2);
        glEnd();
        // Vertical corridor
        glBegin(GL_LINES);
        glVertex3f(-2, 0.02f, -halfSize + 1);
        glVertex3f(-2, 0.02f, halfSize - 1);
        glVertex3f(2, 0.02f, -halfSize + 1);
        glVertex3f(2, 0.02f, halfSize - 1);
        glEnd();
        glLineWidth(1.0f);
        
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
    
    // Helper function to draw a textured wall segment with proper 3D thickness
    void drawTexturedWallSegment(float x, float z, float rotation, float width, float height, TextureID texID = TEX_WALL_PANEL) {
        glPushMatrix();
        glTranslatef(x, height/2, z);
        glRotatef(rotation, 0, 1, 0);
        
        // Enable texturing with proper material for lighting
        glEnable(GL_TEXTURE_2D);
        TextureManager::bind(texID);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor3f(1.0f, 1.0f, 1.0f);
        // Bright material so texture shows well
        GLfloat wallDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
        GLfloat wallAmbient[] = {0.7f, 0.7f, 0.7f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, wallDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, wallAmbient);
        
        float texU = width / 4.0f;  // Tile every 4 units
        float texV = height / 4.0f;
        float thickness = 0.5f;  // Wall thickness
        float halfThick = thickness / 2.0f;
        float texT = thickness / 4.0f;  // Texture scale for thickness
        
        glBegin(GL_QUADS);
        
        // Front face (+Z)
        glNormal3f(0, 0, 1);
        glTexCoord2f(0, 0); glVertex3f(-width/2, -height/2, halfThick);
        glTexCoord2f(texU, 0); glVertex3f(width/2, -height/2, halfThick);
        glTexCoord2f(texU, texV); glVertex3f(width/2, height/2, halfThick);
        glTexCoord2f(0, texV); glVertex3f(-width/2, height/2, halfThick);
        
        // Back face (-Z)
        glNormal3f(0, 0, -1);
        glTexCoord2f(0, 0); glVertex3f(width/2, -height/2, -halfThick);
        glTexCoord2f(texU, 0); glVertex3f(-width/2, -height/2, -halfThick);
        glTexCoord2f(texU, texV); glVertex3f(-width/2, height/2, -halfThick);
        glTexCoord2f(0, texV); glVertex3f(width/2, height/2, -halfThick);
        
        // Left face (-X)
        glNormal3f(-1, 0, 0);
        glTexCoord2f(0, 0); glVertex3f(-width/2, -height/2, -halfThick);
        glTexCoord2f(texT, 0); glVertex3f(-width/2, -height/2, halfThick);
        glTexCoord2f(texT, texV); glVertex3f(-width/2, height/2, halfThick);
        glTexCoord2f(0, texV); glVertex3f(-width/2, height/2, -halfThick);
        
        // Right face (+X)
        glNormal3f(1, 0, 0);
        glTexCoord2f(0, 0); glVertex3f(width/2, -height/2, halfThick);
        glTexCoord2f(texT, 0); glVertex3f(width/2, -height/2, -halfThick);
        glTexCoord2f(texT, texV); glVertex3f(width/2, height/2, -halfThick);
        glTexCoord2f(0, texV); glVertex3f(width/2, height/2, halfThick);
        
        // Top face (+Y)
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, 0); glVertex3f(-width/2, height/2, halfThick);
        glTexCoord2f(texU, 0); glVertex3f(width/2, height/2, halfThick);
        glTexCoord2f(texU, texT); glVertex3f(width/2, height/2, -halfThick);
        glTexCoord2f(0, texT); glVertex3f(-width/2, height/2, -halfThick);
        
        // Bottom face (-Y) - optional but good for completeness
        glNormal3f(0, -1, 0);
        glTexCoord2f(0, 0); glVertex3f(-width/2, -height/2, -halfThick);
        glTexCoord2f(texU, 0); glVertex3f(width/2, -height/2, -halfThick);
        glTexCoord2f(texU, texT); glVertex3f(width/2, -height/2, halfThick);
        glTexCoord2f(0, texT); glVertex3f(-width/2, -height/2, halfThick);
        
        glEnd();
        
        TextureManager::unbind();
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }
    
    void drawFacilityWalls() {
        float halfSize = floorSize / 2.0f;
        
        // =====================================================
        // TEXTURED OUTER WALLS
        // =====================================================
        // North wall
        drawTexturedWallSegment(0, -halfSize, 0, floorSize, wallHeight, TEX_WALL_GREY);
        // South wall
        drawTexturedWallSegment(0, halfSize, 180, floorSize, wallHeight, TEX_WALL_GREY);
        // East wall
        drawTexturedWallSegment(halfSize, 0, 90, floorSize, wallHeight, TEX_WALL_GREY);
        // West wall
        drawTexturedWallSegment(-halfSize, 0, -90, floorSize, wallHeight, TEX_WALL_GREY);
        
        // =====================================================
        // TEXTURED CEILING
        // =====================================================
        glEnable(GL_TEXTURE_2D);
        TextureManager::bind(TEX_FLOOR_TILE);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor3f(1.0f, 1.0f, 1.0f);
        // Bright material for ceiling
        GLfloat ceilDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
        GLfloat ceilAmbient[] = {0.8f, 0.8f, 0.8f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ceilDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ceilAmbient);
        
        float texRepeat = floorSize / 6.0f;
        glBegin(GL_QUADS);
        glNormal3f(0, -1, 0);
        glTexCoord2f(0, 0); glVertex3f(-halfSize, wallHeight, -halfSize);
        glTexCoord2f(texRepeat, 0); glVertex3f(halfSize, wallHeight, -halfSize);
        glTexCoord2f(texRepeat, texRepeat); glVertex3f(halfSize, wallHeight, halfSize);
        glTexCoord2f(0, texRepeat); glVertex3f(-halfSize, wallHeight, halfSize);
        glEnd();
        TextureManager::unbind();
        glDisable(GL_TEXTURE_2D);

        // =====================================================
        // INTERIOR WALLS - Create actual lab rooms
        // =====================================================
        
        // --- SECTOR A: Security Checkpoint (SW) ---
        drawTexturedWallSegment(-5, -20, 90, 20, wallHeight, TEX_WALL_BLUE);
        drawTexturedWallSegment(-20, -5, 0, 15, wallHeight, TEX_WALL_BLUE);
        
        // --- SECTOR B: Research Labs (NW) ---
        drawTexturedWallSegment(-25, 5, 0, 18, wallHeight, TEX_WALL_PANEL);
        drawTexturedWallSegment(-5, 20, 90, 20, wallHeight, TEX_WALL_PANEL);
        drawTexturedWallSegment(-20, 18, 0, 12, wallHeight * 0.6f, TEX_WALL_PANEL);
        
        // --- SECTOR C: Containment (SE) ---
        drawTexturedWallSegment(5, -20, 90, 20, wallHeight, TEX_WALL_ORANGE_WARNING);
        drawTexturedWallSegment(20, -5, 0, 15, wallHeight, TEX_WALL_ORANGE_WARNING);
        drawTexturedWallSegment(15, -20, 0, 8, wallHeight * 0.7f, TEX_WALL_ORANGE_WARNING);
        drawTexturedWallSegment(25, -20, 0, 8, wallHeight * 0.7f, TEX_WALL_ORANGE_WARNING);
        
        // --- SECTOR D: Reactor Core (NE) ---
        drawTexturedWallSegment(25, 5, 0, 12, wallHeight, TEX_WALL_GREY);
        drawTexturedWallSegment(5, 20, 90, 16, wallHeight, TEX_WALL_GREY);
        
        // =====================================================
        // LAB EQUIPMENT AND DECORATIONS
        // =====================================================
        DEBUG_LOG("Level: Starting drawLabEquipment\n");
        drawLabEquipment();
        DEBUG_LOG("Level: Finished drawLabEquipment\n");
    }
    
    void drawLabEquipment() {
        DEBUG_LOG("drawLabEquipment: START\n");
        float halfSize = floorSize / 2.0f;
        
        // --- SECURITY SECTOR (SW) ---
        DEBUG_LOG("drawLabEquipment: Security desk\n");
        glPushMatrix();
        glTranslatef(-30, 0, -25);
        drawSecurityDesk();
        glPopMatrix();
        
        DEBUG_LOG("drawLabEquipment: Monitor bank\n");
        glPushMatrix();
        glTranslatef(-35, 0, -30);
        drawMonitorBank(3);
        glPopMatrix();
        
        // --- RESEARCH SECTOR (NW) ---
        DEBUG_LOG("drawLabEquipment: Lab bench 1\n");
        glPushMatrix();
        glTranslatef(-30, 0, 20);
        drawLabBench();
        glPopMatrix();
        
        DEBUG_LOG("drawLabEquipment: Lab bench 2\n");
        glPushMatrix();
        glTranslatef(-20, 0, 25);
        drawLabBench();
        glPopMatrix();
        
        DEBUG_LOG("drawLabEquipment: Specimen tube 1\n");
        glPushMatrix();
        glTranslatef(-35, 0, 30);
        drawSpecimenTube(true);
        glPopMatrix();
        
        DEBUG_LOG("drawLabEquipment: Specimen tube 2\n");
        glPushMatrix();
        glTranslatef(-32, 0, 30);
        drawSpecimenTube(false);
        glPopMatrix();
        
        DEBUG_LOG("drawLabEquipment: Specimen tube 3\n");
        glPushMatrix();
        glTranslatef(-29, 0, 30);
        drawSpecimenTube(true);
        glPopMatrix();
        
        DEBUG_LOG("drawLabEquipment: Computer terminal\n");
        glPushMatrix();
        glTranslatef(-25, 0, 12);
        drawComputerTerminal();
        glPopMatrix();
        
        // --- CONTAINMENT SECTOR (SE) ---
        DEBUG_LOG("drawLabEquipment: Containment cells\n");
        for (int i = 0; i < 3; i++) {
            DEBUG_LOG("drawLabEquipment: Containment cell");
            glPushMatrix();
            glTranslatef(20 + i * 5, 0, -25);
            drawContainmentCell(i == 1);
            glPopMatrix();
        }
        
        DEBUG_LOG("drawLabEquipment: Warning light\n");
        glPushMatrix();
        glTranslatef(25, wallHeight - 1.5f, -15);
        drawWarningLight();
        glPopMatrix();
        
        // --- REACTOR SECTOR (NE) ---
        DEBUG_LOG("drawLabEquipment: Server racks\n");
        for (int i = 0; i < 3; i++) {
            DEBUG_LOG("drawLabEquipment: Server rack");
            glPushMatrix();
            glTranslatef(30 + i * 2.5f, 0, 12);
            drawServerRack();
            glPopMatrix();
        }
        
        DEBUG_LOG("drawLabEquipment: Reactor pipes\n");
        drawReactorPipes();
        
        DEBUG_LOG("drawLabEquipment: Power conduits\n");
        drawPowerConduits();
        
        // --- CENTRAL CORRIDOR ---
        DEBUG_LOG("drawLabEquipment: Pillars\n");
        for (int x = -1; x <= 1; x += 2) {
            for (int z = -1; z <= 1; z += 2) {
                glPushMatrix();
                glTranslatef(x * 3.0f, 0, z * 3.0f);
                LowPolyModels::drawPillar(wallHeight);
                glPopMatrix();
            }
        }
        
        // Emergency lighting strips along corridors
        drawEmergencyLighting();
    }
    
    // Simplified lab equipment - uses only basic primitives to avoid crashes
    void drawSimplifiedLabEquipment() {
        // --- SECURITY SECTOR (SW) ---
        // Simple desk
        glPushMatrix();
        glTranslatef(-30, 0.5f, -25);
        LowPolyModels::setColorMetallic(0.25f, 0.28f, 0.32f);
        LowPolyModels::drawBox(3.0f, 1.0f, 1.5f);
        glPopMatrix();
        
        // --- RESEARCH SECTOR (NW) ---
        // Lab tables
        glPushMatrix();
        glTranslatef(-30, 0.5f, 20);
        LowPolyModels::setColorMetallic(0.5f, 0.52f, 0.55f);
        LowPolyModels::drawBox(2.5f, 1.0f, 1.2f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-20, 0.5f, 25);
        LowPolyModels::drawBox(2.5f, 1.0f, 1.2f);
        glPopMatrix();
        
        // Simple specimen containers (boxes instead of tubes)
        for (int i = 0; i < 3; i++) {
            glPushMatrix();
            glTranslatef(-35 + i * 3, 1.25f, 30);
            LowPolyModels::setColor(0.3f, 0.5f, 0.6f);
            LowPolyModels::drawBox(0.8f, 2.5f, 0.8f);
            glPopMatrix();
        }
        
        // --- CONTAINMENT SECTOR (SE) ---
        // Simple containment cells (just walls)
        for (int i = 0; i < 3; i++) {
            glPushMatrix();
            glTranslatef(20 + i * 5, 1.5f, -25);
            LowPolyModels::setColorMetallic(0.3f, 0.32f, 0.35f);
            LowPolyModels::drawBox(3.0f, 3.0f, 0.2f);
            glPopMatrix();
        }
        
        // --- REACTOR SECTOR (NE) ---
        // Server racks (simple boxes)
        for (int i = 0; i < 3; i++) {
            glPushMatrix();
            glTranslatef(30 + i * 2.5f, 2.0f, 12);
            LowPolyModels::setColorMetallic(0.18f, 0.18f, 0.2f);
            LowPolyModels::drawBox(0.8f, 4.0f, 1.0f);
            glPopMatrix();
        }
        
        // --- CENTRAL CORRIDOR ---
        // Pillars at intersections
        for (int x = -1; x <= 1; x += 2) {
            for (int z = -1; z <= 1; z += 2) {
                glPushMatrix();
                glTranslatef(x * 3.0f, 0, z * 3.0f);
                LowPolyModels::drawPillar(wallHeight);
                glPopMatrix();
            }
        }
    }

    // Lab equipment drawing functions
    void drawSecurityDesk() {
        // Desk surface - use floor metal texture for desks
        if (TextureManager::isLoaded(TEX_FLOOR_METAL)) {
            glColor3f(0.7f, 0.7f, 0.75f);
            glPushMatrix();
            glTranslatef(0, 1.0f, 0);
            TextureManager::drawTexturedBox(TEX_FLOOR_METAL, 0, 0, 0, 3.0f, 0.15f, 1.5f, 1.0f);
            glPopMatrix();
        } else {
            LowPolyModels::setColorMetallic(0.25f, 0.28f, 0.32f);
            glPushMatrix();
            glTranslatef(0, 1.0f, 0);
            LowPolyModels::drawBox(3.0f, 0.15f, 1.5f);
            glPopMatrix();
        }
        
        // Desk supports - wall panel texture
        if (TextureManager::isLoaded(TEX_WALL_PANEL)) {
            glColor3f(0.6f, 0.6f, 0.65f);
            glPushMatrix();
            glTranslatef(-1.2f, 0.5f, 0);
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, 0, 0, 0, 0.15f, 1.0f, 1.3f, 0.5f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(1.2f, 0.5f, 0);
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, 0, 0, 0, 0.15f, 1.0f, 1.3f, 0.5f);
            glPopMatrix();
        } else {
            LowPolyModels::setColorMetallic(0.2f, 0.22f, 0.26f);
            glPushMatrix();
            glTranslatef(-1.2f, 0.5f, 0);
            LowPolyModels::drawBox(0.15f, 1.0f, 1.3f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(1.2f, 0.5f, 0);
            LowPolyModels::drawBox(0.15f, 1.0f, 1.3f);
            glPopMatrix();
        }
        
        // Monitor on desk
        glPushMatrix();
        glTranslatef(0, 1.3f, -0.3f);
        drawMonitor();
        glPopMatrix();
    }
    
    void drawMonitor() {
        // Screen housing
        LowPolyModels::setColorMetallic(0.15f, 0.15f, 0.18f);
        LowPolyModels::drawBox(0.8f, 0.5f, 0.08f);
        
        // Screen
        float pulse = sin(levelTime * 2) * 0.1f + 0.9f;
        LowPolyModels::setColor(0.1f * pulse, 0.3f * pulse, 0.4f * pulse);
        GLfloat emission[] = {0.05f * pulse, 0.15f * pulse, 0.2f * pulse, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, emission);
        glPushMatrix();
        glTranslatef(0, 0, 0.045f);
        LowPolyModels::drawBox(0.7f, 0.4f, 0.01f);
        glPopMatrix();
        GLfloat noEmission[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
        
        // Stand
        LowPolyModels::setColorMetallic(0.2f, 0.2f, 0.22f);
        glPushMatrix();
        glTranslatef(0, -0.35f, 0);
        LowPolyModels::drawBox(0.15f, 0.2f, 0.1f);
        glPopMatrix();
    }
    
    void drawMonitorBank(int count) {
        for (int i = 0; i < count; i++) {
            glPushMatrix();
            glTranslatef(i * 1.0f, 1.5f, 0);
            drawMonitor();
            glPopMatrix();
        }
    }
    
    void drawLabBench() {
        // Bench top - use metal floor texture
        if (TextureManager::isLoaded(TEX_FLOOR_METAL)) {
            glColor3f(0.85f, 0.87f, 0.9f);
            glPushMatrix();
            glTranslatef(0, 1.0f, 0);
            TextureManager::drawTexturedBox(TEX_FLOOR_METAL, 0, 0, 0, 2.5f, 0.1f, 1.2f, 1.0f);
            glPopMatrix();
        } else {
            LowPolyModels::setColorMetallic(0.5f, 0.52f, 0.55f);
            glPushMatrix();
            glTranslatef(0, 1.0f, 0);
            LowPolyModels::drawBox(2.5f, 0.1f, 1.2f);
            glPopMatrix();
        }
        
        // Legs - use wall panel texture
        if (TextureManager::isLoaded(TEX_WALL_PANEL)) {
            glColor3f(0.6f, 0.62f, 0.65f);
            float legX[] = {-1.0f, 1.0f};
            float legZ[] = {-0.4f, 0.4f};
            for (int x = 0; x < 2; x++) {
                for (int z = 0; z < 2; z++) {
                    glPushMatrix();
                    glTranslatef(legX[x], 0.5f, legZ[z]);
                    TextureManager::drawTexturedBox(TEX_WALL_PANEL, 0, 0, 0, 0.1f, 1.0f, 0.1f, 0.5f);
                    glPopMatrix();
                }
            }
        } else {
            LowPolyModels::setColorMetallic(0.3f, 0.32f, 0.35f);
            float legX[] = {-1.0f, 1.0f};
            float legZ[] = {-0.4f, 0.4f};
            for (int x = 0; x < 2; x++) {
                for (int z = 0; z < 2; z++) {
                    glPushMatrix();
                    glTranslatef(legX[x], 0.5f, legZ[z]);
                    LowPolyModels::drawBox(0.1f, 1.0f, 0.1f);
                    glPopMatrix();
                }
            }
        }
        
        // Equipment on bench (beakers, etc)
        LowPolyModels::setColor(0.7f, 0.8f, 0.9f);
        glPushMatrix();
        glTranslatef(-0.5f, 1.2f, 0);
        glRotatef(-90, 1, 0, 0); // Make beaker vertical
        GLUquadric* quad = gluNewQuadric();
        if (quad) {
            gluCylinder(quad, 0.08f, 0.06f, 0.25f, 8, 1);
            gluDeleteQuadric(quad);
        }
        glPopMatrix();
        
        // Glowing liquid
        LowPolyModels::setColor(0.2f, 0.9f, 0.3f);
        GLfloat liquidEmit[] = {0.1f, 0.4f, 0.15f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, liquidEmit);
        glPushMatrix();
        glTranslatef(0.5f, 1.2f, 0);
        glutSolidSphere(0.1f, 8, 8);
        glPopMatrix();
        GLfloat noEmit[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    }
    
    void drawSpecimenTube(bool hasSpecimen) {
        // Glass tube - draw vertical
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.5f, 0.7f, 0.8f, 0.3f);
        
        glPushMatrix();
        glRotatef(-90, 1, 0, 0); // Rotate to make vertical
        GLUquadric* quad = gluNewQuadric();
        if (quad) {
            gluCylinder(quad, 0.5f, 0.5f, 2.5f, 16, 1);
            gluDeleteQuadric(quad);
        }
        glPopMatrix();
        
        glDisable(GL_BLEND);
        
        // Base
        LowPolyModels::setColorMetallic(0.25f, 0.28f, 0.32f);
        glPushMatrix();
        glTranslatef(0, 0.15f, 0);
        LowPolyModels::drawBox(1.2f, 0.3f, 1.2f);
        glPopMatrix();
        
        // Top cap
        glPushMatrix();
        glTranslatef(0, 2.65f, 0);
        LowPolyModels::drawBox(1.0f, 0.2f, 1.0f);
        glPopMatrix();
        
        // Specimen inside (if present)
        if (hasSpecimen) {
            float pulse = sin(levelTime * 1.5f) * 0.1f + 0.9f;
            LowPolyModels::setColor(0.6f * pulse, 0.2f * pulse, 0.2f * pulse);
            GLfloat specEmit[] = {0.2f * pulse, 0.05f, 0.05f, 1.0f};
            glMaterialfv(GL_FRONT, GL_EMISSION, specEmit);
            glPushMatrix();
            glTranslatef(0, 1.3f, 0);
            glutSolidSphere(0.35f, 12, 12);
            glPopMatrix();
            GLfloat noEmit[] = {0, 0, 0, 1};
            glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
        }
        
        // Liquid glow at base
        float glowPulse = sin(levelTime * 2 + 1) * 0.15f + 0.85f;
        LowPolyModels::setColor(0.2f * glowPulse, 0.8f * glowPulse, 0.3f * glowPulse);
        GLfloat baseEmit[] = {0.1f * glowPulse, 0.3f * glowPulse, 0.15f * glowPulse, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, baseEmit);
        glPushMatrix();
        glTranslatef(0, 0.5f, 0);
        LowPolyModels::drawBox(0.9f, 0.4f, 0.9f);
        glPopMatrix();
        GLfloat noEmit2[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit2);
    }
    
    void drawComputerTerminal() {
        // Terminal housing - use wall panel texture
        if (TextureManager::isLoaded(TEX_WALL_PANEL)) {
            glColor3f(0.5f, 0.52f, 0.55f);
            glPushMatrix();
            glTranslatef(0, 0.8f, 0);
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, 0, 0, 0, 0.8f, 1.6f, 0.6f, 1.0f);
            glPopMatrix();
        } else {
            LowPolyModels::setColorMetallic(0.22f, 0.24f, 0.28f);
            glPushMatrix();
            glTranslatef(0, 0.8f, 0);
            LowPolyModels::drawBox(0.8f, 1.6f, 0.6f);
            glPopMatrix();
        }
        
        // Screen
        float pulse = sin(levelTime * 3 + 2) * 0.1f + 0.9f;
        LowPolyModels::setColor(0.0f, 0.5f * pulse, 0.7f * pulse);
        GLfloat screenEmit[] = {0.0f, 0.2f * pulse, 0.3f * pulse, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, screenEmit);
        glPushMatrix();
        glTranslatef(0, 1.2f, 0.31f);
        LowPolyModels::drawBox(0.6f, 0.8f, 0.02f);
        glPopMatrix();
        GLfloat noEmit[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
        
        // Keyboard shelf
        LowPolyModels::setColorMetallic(0.2f, 0.2f, 0.22f);
        glPushMatrix();
        glTranslatef(0, 0.9f, 0.45f);
        LowPolyModels::drawBox(0.7f, 0.08f, 0.35f);
        glPopMatrix();
        
        // Status lights
        LowPolyModels::setColor(0.1f, 0.9f, 0.2f);
        GLfloat greenEmit[] = {0.05f, 0.4f, 0.1f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, greenEmit);
        glPushMatrix();
        glTranslatef(-0.25f, 0.3f, 0.31f);
        glutSolidSphere(0.03f, 6, 6);
        glPopMatrix();
        
        LowPolyModels::setColor(0.9f, 0.7f, 0.1f);
        GLfloat yellowEmit[] = {0.4f, 0.3f, 0.05f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, yellowEmit);
        glPushMatrix();
        glTranslatef(0, 0.3f, 0.31f);
        glutSolidSphere(0.03f, 6, 6);
        glPopMatrix();
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    }
    
    void drawContainmentCell(bool breached) {
        // Cell walls - use wall panel texture
        if (TextureManager::isLoaded(TEX_WALL_PANEL)) {
            glColor3f(0.6f, 0.62f, 0.65f);
            
            // Back wall
            glPushMatrix();
            glTranslatef(0, 1.5f, -1.5f);
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, 0, 0, 0, 3.0f, 3.0f, 0.2f, 1.5f);
            glPopMatrix();
            
            // Side walls
            glPushMatrix();
            glTranslatef(-1.4f, 1.5f, 0);
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, 0, 0, 0, 0.2f, 3.0f, 3.0f, 1.5f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(1.4f, 1.5f, 0);
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, 0, 0, 0, 0.2f, 3.0f, 3.0f, 1.5f);
            glPopMatrix();
        } else {
            LowPolyModels::setColorMetallic(0.3f, 0.32f, 0.35f);
            
            // Back wall
            glPushMatrix();
            glTranslatef(0, 1.5f, -1.5f);
            LowPolyModels::drawBox(3.0f, 3.0f, 0.2f);
            glPopMatrix();
            
            // Side walls
            glPushMatrix();
            glTranslatef(-1.4f, 1.5f, 0);
            LowPolyModels::drawBox(0.2f, 3.0f, 3.0f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(1.4f, 1.5f, 0);
            LowPolyModels::drawBox(0.2f, 3.0f, 3.0f);
            glPopMatrix();
        }
        
        // Energy barrier (front)
        if (!breached) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            float pulse = sin(levelTime * 4) * 0.2f + 0.8f;
            glColor4f(0.2f * pulse, 0.5f * pulse, 0.9f * pulse, 0.4f);
            glPushMatrix();
            glTranslatef(0, 1.5f, 1.3f);
            LowPolyModels::drawBox(2.6f, 2.8f, 0.05f);
            glPopMatrix();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_BLEND);
        } else {
            // Broken barrier effect - sparks
            float sparkPhase = fmod(levelTime * 5, 1.0f);
            if (sparkPhase < 0.3f) {
                glDisable(GL_LIGHTING);
                glPointSize(3.0f);
                glBegin(GL_POINTS);
                for (int i = 0; i < 5; i++) {
                    float px = -1.0f + (float)(rand() % 20) / 10.0f;
                    float py = 0.5f + (float)(rand() % 20) / 10.0f;
                    glColor3f(0.3f + sparkPhase, 0.5f + sparkPhase * 0.5f, 1.0f);
                    glVertex3f(px, py, 1.3f);
                }
                glEnd();
                glEnable(GL_LIGHTING);
            }
        }
    }
    
    void drawWarningLight() {
        float flash = sin(levelTime * 8) > 0 ? 1.0f : 0.3f;
        
        // Housing
        LowPolyModels::setColorMetallic(0.2f, 0.2f, 0.22f);
        LowPolyModels::drawBox(0.3f, 0.2f, 0.3f);
        
        // Light
        LowPolyModels::setColor(0.9f * flash, 0.2f * flash, 0.1f * flash);
        GLfloat redEmit[] = {0.5f * flash, 0.1f * flash, 0.05f * flash, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, redEmit);
        glPushMatrix();
        glTranslatef(0, -0.15f, 0);
        glutSolidSphere(0.12f, 8, 8);
        glPopMatrix();
        GLfloat noEmit[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    }
    
    void drawServerRack() {
        // Main rack - use wall panel texture
        if (TextureManager::isLoaded(TEX_WALL_PANEL)) {
            glColor3f(0.4f, 0.4f, 0.45f);
            glPushMatrix();
            glTranslatef(0, 2.0f, 0);
            TextureManager::drawTexturedBox(TEX_WALL_PANEL, 0, 0, 0, 0.8f, 4.0f, 1.0f, 2.0f);
            glPopMatrix();
        } else {
            LowPolyModels::setColorMetallic(0.18f, 0.18f, 0.2f);
            glPushMatrix();
            glTranslatef(0, 2.0f, 0);
            LowPolyModels::drawBox(0.8f, 4.0f, 1.0f);
            glPopMatrix();
        }
        
        // Server units with blinking lights
        for (int i = 0; i < 8; i++) {
            float y = 0.4f + i * 0.45f;
            
            // Unit - use wall grey texture for server units
            if (TextureManager::isLoaded(TEX_WALL_GREY)) {
                glColor3f(0.35f, 0.35f, 0.4f);
                glPushMatrix();
                glTranslatef(0, y, 0);
                TextureManager::drawTexturedBox(TEX_WALL_GREY, 0, 0, 0, 0.75f, 0.35f, 0.95f, 0.5f);
                glPopMatrix();
            } else {
                LowPolyModels::setColorMetallic(0.15f, 0.15f, 0.17f);
                glPushMatrix();
                glTranslatef(0, y, 0);
                LowPolyModels::drawBox(0.75f, 0.35f, 0.95f);
                glPopMatrix();
            }
            
            // Status lights
            float phase = sin(levelTime * 3 + i * 0.7f);
            if (phase > 0) {
                LowPolyModels::setColor(0.1f, 0.9f, 0.2f);
            } else {
                LowPolyModels::setColor(0.9f, 0.6f, 0.1f);
            }
            GLfloat lightEmit[] = {phase > 0 ? 0.05f : 0.4f, phase > 0 ? 0.4f : 0.25f, phase > 0 ? 0.1f : 0.05f, 1.0f};
            glMaterialfv(GL_FRONT, GL_EMISSION, lightEmit);
            glPushMatrix();
            glTranslatef(-0.3f, y, 0.48f);
            glutSolidSphere(0.02f, 6, 6);
            glPopMatrix();
            GLfloat noEmit[] = {0, 0, 0, 1};
            glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
        }
    }
    
    void drawReactorPipes() {
        float halfSize = floorSize / 2.0f;
        
        // Large pipes along reactor walls
        LowPolyModels::setColorMetallic(0.35f, 0.38f, 0.42f);
        
        GLUquadric* quad = gluNewQuadric();
        if (!quad) return; // Safety check
        
        // Horizontal pipes near ceiling
        glPushMatrix();
        glTranslatef(10, wallHeight - 1.5f, 20);
        glRotatef(90, 0, 1, 0);
        gluCylinder(quad, 0.4f, 0.4f, 25, 12, 1);
        glPopMatrix();
        
        // Vertical pipes
        glPushMatrix();
        glTranslatef(25, 0, 30);
        glRotatef(-90, 1, 0, 0);
        gluCylinder(quad, 0.35f, 0.35f, wallHeight, 12, 1);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(35, 0, 30);
        glRotatef(-90, 1, 0, 0);
        gluCylinder(quad, 0.35f, 0.35f, wallHeight, 12, 1);
        glPopMatrix();
        
        gluDeleteQuadric(quad);
        
        // Pipe joints (glowing)
        float pulse = sin(levelTime * 2) * 0.15f + 0.85f;
        LowPolyModels::setColor(0.2f * pulse, 0.6f * pulse, 0.9f * pulse);
        GLfloat pipeEmit[] = {0.1f * pulse, 0.3f * pulse, 0.45f * pulse, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, pipeEmit);
        
        glPushMatrix();
        glTranslatef(25, wallHeight - 1.5f, 30);
        glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(35, wallHeight - 1.5f, 30);
        glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        
        GLfloat noEmit[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    }
    
    void drawPowerConduits() {
        float halfSize = floorSize / 2.0f;
        
        // Conduits run along floor edges with glowing cores
        float pulse = sin(levelTime * 3) * 0.1f + 0.9f;
        
        // Draw conduit strips
        glDisable(GL_LIGHTING);
        glLineWidth(3.0f);
        
        // North wall conduit
        glColor3f(0.0f, 0.4f * pulse, 0.6f * pulse);
        glBegin(GL_LINE_STRIP);
        for (float x = -halfSize + 2; x < halfSize - 2; x += 0.5f) {
            float glow = sin(levelTime * 4 + x * 0.5f) * 0.3f + 0.7f;
            glColor3f(0.0f, 0.4f * glow, 0.6f * glow);
            glVertex3f(x, 0.3f, -halfSize + 1);
        }
        glEnd();
        
        // South wall conduit
        glBegin(GL_LINE_STRIP);
        for (float x = -halfSize + 2; x < halfSize - 2; x += 0.5f) {
            float glow = sin(levelTime * 4 + x * 0.5f + 1) * 0.3f + 0.7f;
            glColor3f(0.0f, 0.4f * glow, 0.6f * glow);
            glVertex3f(x, 0.3f, halfSize - 1);
        }
        glEnd();
        
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }
    
    void drawEmergencyLighting() {
        // Emergency floor lights in corridors
        float halfSize = floorSize / 2.0f;
        
        glDisable(GL_LIGHTING);
        
        // Main corridor lights
        float pulse = sin(levelTime) * 0.2f + 0.8f;
        
        glPointSize(4.0f);
        glBegin(GL_POINTS);
        
        // Horizontal corridor
        for (float x = -halfSize + 5; x < halfSize - 5; x += 4) {
            float glow = sin(levelTime * 2 + x * 0.3f) * 0.3f + 0.7f;
            glColor3f(0.9f * glow, 0.5f * glow, 0.1f * glow);
            glVertex3f(x, 0.05f, -1.5f);
            glVertex3f(x, 0.05f, 1.5f);
        }
        
        // Vertical corridor
        for (float z = -halfSize + 5; z < halfSize - 5; z += 4) {
            float glow = sin(levelTime * 2 + z * 0.3f) * 0.3f + 0.7f;
            glColor3f(0.9f * glow, 0.5f * glow, 0.1f * glow);
            glVertex3f(-1.5f, 0.05f, z);
            glVertex3f(1.5f, 0.05f, z);
        }
        
        glEnd();
        glPointSize(1.0f);
        glEnable(GL_LIGHTING);
    }
    
    void drawHellFloor() {
        // FIRST: Draw sky (behind everything)
        drawHellSky();
        
        glPushMatrix();
        float halfSize = floorSize / 2.0f;
        
        // =====================================================
        // MOLTEN LAVA FLOOR - The base layer (at Y=0)
        // Platforms float above this lava
        // =====================================================
        
        // Reset all OpenGL state to ensure clean rendering
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_LIGHTING);
        glDisable(GL_FOG);  // Disable fog for lava - we want full brightness
        glDisable(GL_CULL_FACE);  // Disable culling so floor is visible from above
        
        float time = levelTime;
        float mainPulse = sin(time * 1.5f) * 0.1f + 0.9f;
        
        // Try to use textured lava floor
        if (TextureManager::isLoaded(TEX_LAVA)) {
            glEnable(GL_TEXTURE_2D);
            TextureManager::bind(TEX_LAVA);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);  // Modulate for proper color
            
            // Animated UV offset for flowing lava effect
            float uvOffsetX = fmod(time * 0.05f, 1.0f);
            float uvOffsetY = fmod(time * 0.03f, 1.0f);
            
            // Draw large textured lava floor with animated UVs - FULL BRIGHTNESS
            glColor3f(1.0f, 1.0f, 1.0f);  // White for full texture brightness
            
            float texRepeat = 15.0f;  // How many times texture repeats
            
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glTexCoord2f(uvOffsetX, uvOffsetY); 
            glVertex3f(-halfSize * 3.0f, lavaHeight, -halfSize * 3.0f);
            glTexCoord2f(texRepeat + uvOffsetX, uvOffsetY); 
            glVertex3f(halfSize * 3.0f, lavaHeight, -halfSize * 3.0f);
            glTexCoord2f(texRepeat + uvOffsetX, texRepeat + uvOffsetY); 
            glVertex3f(halfSize * 3.0f, lavaHeight, halfSize * 3.0f);
            glTexCoord2f(uvOffsetX, texRepeat + uvOffsetY); 
            glVertex3f(-halfSize * 3.0f, lavaHeight, halfSize * 3.0f);
            glEnd();
            
            // Add glow overlay layer with lava effect texture
            if (TextureManager::isLoaded(TEX_LAVA_GLOW)) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                TextureManager::bind(TEX_LAVA_GLOW);
                
                float uvOffset2X = fmod(time * -0.07f, 1.0f);  // Opposite direction
                float uvOffset2Y = fmod(time * 0.04f, 1.0f);
                
                glColor4f(1.0f * mainPulse, 0.6f * mainPulse, 0.2f * mainPulse, 0.4f);
                
                glBegin(GL_QUADS);
                glNormal3f(0, 1, 0);
                glTexCoord2f(uvOffset2X, uvOffset2Y); 
                glVertex3f(-halfSize * 3.0f, lavaHeight + 0.02f, -halfSize * 3.0f);
                glTexCoord2f(texRepeat * 0.5f + uvOffset2X, uvOffset2Y); 
                glVertex3f(halfSize * 3.0f, lavaHeight + 0.02f, -halfSize * 3.0f);
                glTexCoord2f(texRepeat * 0.5f + uvOffset2X, texRepeat * 0.5f + uvOffset2Y); 
                glVertex3f(halfSize * 3.0f, lavaHeight + 0.02f, halfSize * 3.0f);
                glTexCoord2f(uvOffset2X, texRepeat * 0.5f + uvOffset2Y); 
                glVertex3f(-halfSize * 3.0f, lavaHeight + 0.02f, halfSize * 3.0f);
                glEnd();
                
                glDisable(GL_BLEND);
            }
            
            TextureManager::unbind();
            glDisable(GL_TEXTURE_2D);
        } else {
            // =====================================================
            // FALLBACK - Procedural orange lava
            // =====================================================
            glColor3f(0.8f * mainPulse, 0.25f * mainPulse, 0.03f);
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f(-halfSize * 3.0f, lavaHeight, -halfSize * 3.0f);
            glVertex3f(halfSize * 3.0f, lavaHeight, -halfSize * 3.0f);
            glVertex3f(halfSize * 3.0f, lavaHeight, halfSize * 3.0f);
            glVertex3f(-halfSize * 3.0f, lavaHeight, halfSize * 3.0f);
            glEnd();
        }
        
        // Re-enable culling for other elements
        glEnable(GL_CULL_FACE);
        
        // =====================================================
        // LAVA GRID OVERLAY - Wave animation
        // =====================================================
        int gridSize = 20;
        float cellSize = (halfSize * 3.0f) / gridSize;
        
        for (int gx = 0; gx < gridSize; gx++) {
            for (int gz = 0; gz < gridSize; gz++) {
                float cx = -halfSize * 1.5f + gx * cellSize + cellSize * 0.5f;
                float cz = -halfSize * 1.5f + gz * cellSize + cellSize * 0.5f;
                
                // Wave effect
                float wave = sin(time * 2.0f + gx * 0.4f + gz * 0.3f) * 0.15f;
                float cellY = lavaHeight + 0.05f + wave;
                
                // Color varies across grid - hot orange to deep red
                float heatVar = sin(time * 1.0f + gx * 0.5f + gz * 0.7f) * 0.15f + 0.85f;
                float r = 0.9f * mainPulse * heatVar;
                float g = 0.3f * mainPulse * heatVar;
                float b = 0.05f;
                
                glColor3f(r, g, b);
                
                float hs = cellSize * 0.48f;
                glBegin(GL_QUADS);
                glNormal3f(0, 1, 0);
                glVertex3f(cx - hs, cellY, cz - hs);
                glVertex3f(cx + hs, cellY, cz - hs);
                glVertex3f(cx + hs, cellY, cz + hs);
                glVertex3f(cx - hs, cellY, cz + hs);
                glEnd();
            }
        }
        
        // =====================================================
        // HOT SPOTS - Bright glowing patches
        // =====================================================
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        for (int i = 0; i < 25; i++) {
            float hx = sin(i * 2.1f + time * 0.08f) * halfSize * 1.2f;
            float hz = cos(i * 2.9f + time * 0.06f) * halfSize * 1.2f;
            float hy = lavaHeight + 0.2f;
            float spotSize = 3.0f + sin(i * 1.5f + time * 1.5f) * 1.5f;
            float intensity = sin(time * 2.5f + i * 0.6f) * 0.2f + 0.8f;
            
            glColor4f(1.0f * intensity, 0.45f * intensity, 0.05f, 0.5f);
            glBegin(GL_QUADS);
            glVertex3f(hx - spotSize, hy, hz - spotSize);
            glVertex3f(hx + spotSize, hy, hz - spotSize);
            glVertex3f(hx + spotSize, hy, hz + spotSize);
            glVertex3f(hx - spotSize, hy, hz + spotSize);
            glEnd();
        }
        
        // =====================================================
        // RISING EMBERS - Orange particles
        // =====================================================
        glPointSize(4.0f);
        glBegin(GL_POINTS);
        
        for (int e = 0; e < 100; e++) {
            float emberX = sin(e * 3.3f + time * 0.08f) * halfSize * 1.4f;
            float emberZ = cos(e * 4.7f + time * 0.06f) * halfSize * 1.4f;
            float emberY = fmod(time * 1.2f + e * 0.8f, 25.0f) + lavaHeight;
            float emberAlpha = 1.0f - (emberY - lavaHeight) / 25.0f;
            
            glColor4f(1.0f, 0.4f + sin(e * 0.3f) * 0.1f, 0.05f, emberAlpha * 0.85f);
            glVertex3f(emberX + sin(emberY * 0.3f) * 1.5f, emberY, emberZ + cos(emberY * 0.3f) * 1.5f);
        }
        
        glEnd();
        glPointSize(1.0f);
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        glPopMatrix();
    }
    
    // Draw demonic pillars, portals, and runes around the arena
    void drawDemonicStructures() {
        glPushMatrix();
        
        // =====================================================
        // DEMONIC PILLARS - Towering obsidian spires
        // =====================================================
        float pillarPositions[][3] = {
            {-60, 0, -60}, {60, 0, -60},
            {-80, 0, 0}, {80, 0, 0},
            {-60, 0, 60}, {60, 0, 60},
            {0, 0, -85}, {0, 0, 95},
        };
        
        for (int p = 0; p < 8; p++) {
            glPushMatrix();
            glTranslatef(pillarPositions[p][0], lavaHeight, pillarPositions[p][2]);
            
            // Dark obsidian pillar with texture
            float pillarHeight = 15.0f + sin(p * 2.5f) * 5.0f;
            
            if (TextureManager::isLoaded(TEX_PILLAR) || TextureManager::isLoaded(TEX_ROCK)) {
                TextureID pillarTex = TextureManager::isLoaded(TEX_PILLAR) ? TEX_PILLAR : TEX_ROCK;
                TextureManager::drawTexturedBox(pillarTex, 0, pillarHeight * 0.5f, 0, 3.0f, pillarHeight, 3.0f, 0.5f);
            } else {
                LowPolyModels::setColor(0.1f, 0.08f, 0.12f);
                LowPolyModels::drawBox(3.0f, pillarHeight, 3.0f);
            }
            
            // Glowing rune at top
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            
            float runeGlow = sin(levelTime * 2.0f + p * 0.8f) * 0.3f + 0.7f;
            glColor4f(0.8f * runeGlow, 0.1f * runeGlow, 0.0f, 0.9f);
            
            glPushMatrix();
            glTranslatef(0, pillarHeight + 0.5f, 0);
            glutSolidSphere(1.0f, 10, 10);
            glPopMatrix();
            
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            
            glPopMatrix();
        }
        
        // =====================================================
        // CORNER FIRE BRAZIERS - Instead of purple portals
        // =====================================================
        float brazierPositions[][3] = {
            {-40, 2.0f, -40}, {40, 2.0f, -40},
            {-40, 2.0f, 40}, {40, 2.0f, 40},
        };
        
        for (int br = 0; br < 4; br++) {
            glPushMatrix();
            glTranslatef(brazierPositions[br][0], brazierPositions[br][1], brazierPositions[br][2]);
            
            float flamePulse = sin(levelTime * 5.0f + br * 1.2f) * 0.3f + 0.7f;
            
            // Stone brazier base
            glEnable(GL_LIGHTING);
            glDisable(GL_BLEND);
            if (TextureManager::isLoaded(TEX_ROCK)) {
                TextureManager::drawTexturedBox(TEX_ROCK, 0, 0, 0, 2.0f, 1.5f, 2.0f, 0.4f);
                TextureManager::drawTexturedBox(TEX_ROCK, 0, 1.5f, 0, 2.5f, 0.3f, 2.5f, 0.3f);
            } else {
                LowPolyModels::setColor(0.3f, 0.25f, 0.2f);
                LowPolyModels::drawBox(2.0f, 1.5f, 2.0f);
                glTranslatef(0, 1.5f, 0);
                LowPolyModels::drawBox(2.5f, 0.3f, 2.5f);
            }
            
            // Fire effect on top
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            
            // Orange-red flames
            for (int f = 0; f < 3; f++) {
                float fOff = f * 0.4f;
                float fSize = (1.2f - f * 0.3f) * flamePulse;
                glColor4f(1.0f, 0.4f + f * 0.2f, 0.1f, 0.7f - f * 0.15f);
                glPushMatrix();
                glTranslatef(sin(levelTime * 3 + f) * 0.3f, 2.0f + fOff, cos(levelTime * 2 + f) * 0.3f);
                glutSolidSphere(fSize, 8, 8);
                glPopMatrix();
            }
            
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            
            glPopMatrix();
        }
        
        // =====================================================
        // BOSS THRONE - Demonic throne at boss spawn point (0, 4.5, 30)
        // =====================================================
        glPushMatrix();
        glTranslatef(0, 6.0f, 30);  // Boss platform is at Y=4.5, throne sits above it
        
        float thronePulse = sin(levelTime * 2.0f) * 0.15f + 0.85f;
        
        // Main throne back (tall slab)
        glEnable(GL_LIGHTING);
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, 0, 2.0f, 2.0f, 4.0f, 6.0f, 1.0f, 0.3f);
        } else {
            LowPolyModels::setColor(0.15f, 0.1f, 0.12f);
            LowPolyModels::drawBox(4.0f, 6.0f, 1.0f);
        }
        
        // Throne seat
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, 0, 0.3f, 0, 5.0f, 0.6f, 3.0f, 0.4f);
        } else {
            LowPolyModels::setColor(0.12f, 0.08f, 0.1f);
            glPushMatrix();
            glTranslatef(0, 0.3f, 0);
            LowPolyModels::drawBox(5.0f, 0.6f, 3.0f);
            glPopMatrix();
        }
        
        // Armrests
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, -2.2f, 0.8f, 0.5f, 0.6f, 1.6f, 2.0f, 0.3f);
            TextureManager::drawTexturedBox(TEX_ROCK, 2.2f, 0.8f, 0.5f, 0.6f, 1.6f, 2.0f, 0.3f);
        } else {
            LowPolyModels::setColor(0.15f, 0.1f, 0.12f);
            glPushMatrix();
            glTranslatef(-2.2f, 0.8f, 0.5f);
            LowPolyModels::drawBox(0.6f, 1.6f, 2.0f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(2.2f, 0.8f, 0.5f);
            LowPolyModels::drawBox(0.6f, 1.6f, 2.0f);
            glPopMatrix();
        }
        
        // Demonic horns on throne back
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Left horn
        glPushMatrix();
        glTranslatef(-1.5f, 5.5f, 2.0f);
        glRotatef(-30, 0, 0, 1);
        glRotatef(15, 1, 0, 0);
        glColor4f(0.2f, 0.1f, 0.15f, 1.0f);
        GLUquadric* hornQuad = gluNewQuadric();
        gluCylinder(hornQuad, 0.4f, 0.05f, 3.0f, 8, 4);
        glPopMatrix();
        
        // Right horn
        glPushMatrix();
        glTranslatef(1.5f, 5.5f, 2.0f);
        glRotatef(30, 0, 0, 1);
        glRotatef(15, 1, 0, 0);
        glColor4f(0.2f, 0.1f, 0.15f, 1.0f);
        gluCylinder(hornQuad, 0.4f, 0.05f, 3.0f, 8, 4);
        gluDeleteQuadric(hornQuad);
        glPopMatrix();
        
        // Glowing runes on throne
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(0.8f * thronePulse, 0.1f * thronePulse, 0.2f * thronePulse, 0.9f);
        
        // Rune circles on throne back
        for (int i = 0; i < 3; i++) {
            glPushMatrix();
            glTranslatef(0, 1.5f + i * 1.5f, 2.6f);
            glutSolidSphere(0.2f * thronePulse, 8, 8);
            glPopMatrix();
        }
        
        // Runes on armrests
        glPushMatrix();
        glTranslatef(-2.2f, 1.6f, 0.5f);
        glutSolidSphere(0.15f * thronePulse, 6, 6);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(2.2f, 1.6f, 0.5f);
        glutSolidSphere(0.15f * thronePulse, 6, 6);
        glPopMatrix();
        
        // (Summoning circle removed - was too purple)
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        glPopMatrix();
        
        // =====================================================
        // BOSS PLATFORM CEILING AND COLUMNS - Textured structure
        // =====================================================
        // Boss platform is at (0, 4.5, 30), platform is about 15x15
        float bpX = 0;
        float bpY = 4.5f;
        float bpZ = 30;
        float ceilingHeight = 10.0f;  // Height of ceiling - lower for visibility
        float columnHeight = ceilingHeight - bpY;  // Column height = 5.5 units
        
        glEnable(GL_LIGHTING);
        
        // ===== Supporting Columns (4 corners) =====
        float columnOffset = 6.0f;  // Distance from center to columns (smaller for tighter structure)
        float colRadius = 1.5f;  // Column width - BIGGER
        
        // Column positions (4 corners)
        float columnPositions[4][2] = {
            {bpX - columnOffset, bpZ - columnOffset},  // Front-left
            {bpX + columnOffset, bpZ - columnOffset},  // Front-right
            {bpX - columnOffset, bpZ + columnOffset},  // Back-left
            {bpX + columnOffset, bpZ + columnOffset}   // Back-right
        };
        
        for (int i = 0; i < 4; i++) {
            float cx = columnPositions[i][0];
            float cz = columnPositions[i][1];
            
            // Main column shaft - textured
            if (TextureManager::isLoaded(TEX_ROCK)) {
                TextureManager::drawTexturedBox(TEX_ROCK, cx, bpY + columnHeight/2, cz, 
                                                colRadius, columnHeight, colRadius, 0.3f);
            } else {
                LowPolyModels::setColor(0.25f, 0.2f, 0.18f);
                glPushMatrix();
                glTranslatef(cx, bpY + columnHeight/2, cz);
                LowPolyModels::drawBox(colRadius, columnHeight, colRadius);
                glPopMatrix();
            }
            
            // Column base (wider)
            if (TextureManager::isLoaded(TEX_ROCK)) {
                TextureManager::drawTexturedBox(TEX_ROCK, cx, bpY + 0.4f, cz, 
                                                colRadius * 1.5f, 0.8f, colRadius * 1.5f, 0.4f);
            } else {
                LowPolyModels::setColor(0.2f, 0.15f, 0.13f);
                glPushMatrix();
                glTranslatef(cx, bpY + 0.4f, cz);
                LowPolyModels::drawBox(colRadius * 1.5f, 0.8f, colRadius * 1.5f);
                glPopMatrix();
            }
            
            // Column capital (decorative top)
            if (TextureManager::isLoaded(TEX_ROCK)) {
                TextureManager::drawTexturedBox(TEX_ROCK, cx, ceilingHeight - 0.4f, cz, 
                                                colRadius * 1.8f, 0.8f, colRadius * 1.8f, 0.4f);
            } else {
                LowPolyModels::setColor(0.28f, 0.22f, 0.2f);
                glPushMatrix();
                glTranslatef(cx, ceilingHeight - 0.4f, cz);
                LowPolyModels::drawBox(colRadius * 1.8f, 0.8f, colRadius * 1.8f);
                glPopMatrix();
            }
        }
        
        // ===== Ceiling Structure =====
        float ceilingWidth = columnOffset * 2 + 6.0f;  // Wider ceiling
        float ceilingThickness = 2.0f;  // Thicker ceiling
        
        // Main ceiling slab - textured with darker rock
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, bpX, ceilingHeight + ceilingThickness/2, bpZ, 
                                            ceilingWidth, ceilingThickness, ceilingWidth, 0.2f);
        } else {
            LowPolyModels::setColor(0.25f, 0.18f, 0.15f);
            glPushMatrix();
            glTranslatef(bpX, ceilingHeight + ceilingThickness/2, bpZ);
            LowPolyModels::drawBox(ceilingWidth, ceilingThickness, ceilingWidth);
            glPopMatrix();
        }
        
        // Ceiling edge beams (cross beams connecting columns)
        float beamThickness = 1.2f;  // Thicker beams
        
        // Front beam
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, bpX, ceilingHeight - beamThickness/2, bpZ - columnOffset, 
                                            ceilingWidth, beamThickness, beamThickness, 0.35f);
        } else {
            LowPolyModels::setColor(0.22f, 0.16f, 0.13f);
            glPushMatrix();
            glTranslatef(bpX, ceilingHeight - beamThickness/2, bpZ - columnOffset);
            LowPolyModels::drawBox(ceilingWidth, beamThickness, beamThickness);
            glPopMatrix();
        }
        // Back beam
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, bpX, ceilingHeight - beamThickness/2, bpZ + columnOffset, 
                                            ceilingWidth, beamThickness, beamThickness, 0.35f);
        } else {
            LowPolyModels::setColor(0.22f, 0.16f, 0.13f);
            glPushMatrix();
            glTranslatef(bpX, ceilingHeight - beamThickness/2, bpZ + columnOffset);
            LowPolyModels::drawBox(ceilingWidth, beamThickness, beamThickness);
            glPopMatrix();
        }
        // Left beam
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, bpX - columnOffset, ceilingHeight - beamThickness/2, bpZ, 
                                            beamThickness, beamThickness, ceilingWidth, 0.35f);
        } else {
            LowPolyModels::setColor(0.22f, 0.16f, 0.13f);
            glPushMatrix();
            glTranslatef(bpX - columnOffset, ceilingHeight - beamThickness/2, bpZ);
            LowPolyModels::drawBox(beamThickness, beamThickness, ceilingWidth);
            glPopMatrix();
        }
        // Right beam
        if (TextureManager::isLoaded(TEX_ROCK)) {
            TextureManager::drawTexturedBox(TEX_ROCK, bpX + columnOffset, ceilingHeight - beamThickness/2, bpZ, 
                                            beamThickness, beamThickness, ceilingWidth, 0.35f);
        } else {
            LowPolyModels::setColor(0.22f, 0.16f, 0.13f);
            glPushMatrix();
            glTranslatef(bpX + columnOffset, ceilingHeight - beamThickness/2, bpZ);
            LowPolyModels::drawBox(beamThickness, beamThickness, ceilingWidth);
            glPopMatrix();
        }
        
        // Central hanging fixture (ominous)
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        float fixturePulse = sin(levelTime * 2.5f) * 0.2f + 0.8f;
        glColor4f(0.9f * fixturePulse, 0.3f * fixturePulse, 0.1f * fixturePulse, 0.7f);
        
        // Hanging chains (simple lines)
        glPushMatrix();
        glTranslatef(bpX, ceilingHeight - 2.0f, bpZ);
        glutSolidSphere(0.8f * fixturePulse, 12, 12);  // Central orb
        glPopMatrix();
        
        // Smaller orbs around the main one
        for (int i = 0; i < 4; i++) {
            float angle = i * 1.5708f + levelTime * 0.5f;
            float orbX = bpX + sin(angle) * 2.0f;
            float orbZ = bpZ + cos(angle) * 2.0f;
            
            glPushMatrix();
            glTranslatef(orbX, ceilingHeight - 2.5f, orbZ);
            glColor4f(0.8f * fixturePulse, 0.2f * fixturePulse, 0.05f * fixturePulse, 0.6f);
            glutSolidSphere(0.4f * fixturePulse, 8, 8);
            glPopMatrix();
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);

        // =====================================================
        // FLOATING FIRE ORBS - Mystical floating flames around the arena
        // =====================================================
        for (int r = 0; r < 12; r++) {
            float orbX = sin(r * 0.524f) * 55.0f;
            float orbZ = cos(r * 0.524f) * 55.0f;
            float orbY = 10.0f + sin(levelTime * 0.8f + r * 0.5f) * 3.0f;
            
            float orbPulse = sin(levelTime * 3.0f + r * 0.7f) * 0.25f + 0.75f;
            
            glPushMatrix();
            glTranslatef(orbX, orbY, orbZ);
            
            // Core flame orb - solid glowing sphere
            glEnable(GL_LIGHTING);
            GLfloat orbEmissive[] = {0.9f * orbPulse, 0.3f * orbPulse, 0.05f, 1.0f};
            GLfloat orbDiffuse[] = {1.0f, 0.5f, 0.1f, 1.0f};
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, orbEmissive);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, orbDiffuse);
            glutSolidSphere(1.2f * orbPulse, 12, 12);
            
            // Clear emissive
            GLfloat noEmissive[] = {0, 0, 0, 1};
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmissive);
            
            // Outer glow effect
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            
            // Multiple glow layers
            for (int g = 0; g < 3; g++) {
                float glowSize = 1.8f + g * 0.8f;
                float glowAlpha = (0.4f - g * 0.12f) * orbPulse;
                glColor4f(1.0f, 0.4f - g * 0.1f, 0.05f, glowAlpha);
                glutSolidSphere(glowSize, 8, 8);
            }
            
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            
            glPopMatrix();
        }
        glEnable(GL_FOG);  // Re-enable fog for other level elements
        
        glPopMatrix();
    }
    
    void drawHellSky() {
        // =====================================================
        // TITAN MOON SKYBOX - Beautiful space environment for Level 2
        // With graceful day-night transition based on enemy kills
        // =====================================================
        
        // Calculate how many regular enemies are killed (for day-night transition)
        int totalRegularEnemies = 0;
        int killedRegularEnemies = 0;
        for (int i = 0; i < numEnemies; i++) {
            if (enemies[i].type != ENEMY_BOSS) {
                totalRegularEnemies++;
                if (!enemies[i].active || enemies[i].health <= 0) {
                    killedRegularEnemies++;
                }
            }
        }
        
        // Transition progress: 0.0 = start (sunset), 1.0 = all enemies killed (night)
        float transitionProgress = (totalRegularEnemies > 0) ? 
            (float)killedRegularEnemies / (float)totalRegularEnemies : 0.0f;
        
        // Check if boss is active for final darkness
        bool bossActive = bossEnemyIndex >= 0 && bossEnemyIndex < numEnemies && enemies[bossEnemyIndex].active;
        if (bossActive) {
            transitionProgress = 1.0f;  // Force full night when boss spawns
        }
        
        // Try to use TitanMoon textured skybox first
        if (TextureManager::isLoaded(TEX_SKYBOX_FRONT)) {
            // Draw the textured skybox centered on player
            // Size must be smaller than far plane (200.0f) - use 180.0f for safety
            TextureManager::drawSkybox(lastPlayerPos.x, lastPlayerPos.y, lastPlayerPos.z, 180.0f);
            
            // =====================================================
            // GRACEFUL DAY-NIGHT FILTER TRANSITION
            // =====================================================
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // Calculate filter colors based on transition progress
            float filterR, filterG, filterB, filterAlpha;
            float filterPulse = sin(levelTime * 0.8f) * 0.03f;
            
            if (transitionProgress < 0.3f) {
                // SUNSET - warm orange/red tint (start of level)
                float t = transitionProgress / 0.3f;
                filterR = 0.9f - t * 0.2f;    // 0.9 -> 0.7
                filterG = 0.4f - t * 0.15f;   // 0.4 -> 0.25
                filterB = 0.1f + t * 0.1f;    // 0.1 -> 0.2
                filterAlpha = 0.1f + filterPulse;
            } else if (transitionProgress < 0.7f) {
                // DUSK - transition from orange to purple/blue
                float t = (transitionProgress - 0.3f) / 0.4f;
                filterR = 0.7f - t * 0.4f;    // 0.7 -> 0.3
                filterG = 0.25f - t * 0.1f;   // 0.25 -> 0.15
                filterB = 0.2f + t * 0.3f;    // 0.2 -> 0.5
                filterAlpha = 0.12f + t * 0.08f + filterPulse;  // Gradually darker
            } else {
                // NIGHT - deep blue/purple darkness (boss phase)
                float t = (transitionProgress - 0.7f) / 0.3f;
                filterR = 0.3f - t * 0.2f;    // 0.3 -> 0.1
                filterG = 0.15f - t * 0.05f;  // 0.15 -> 0.1
                filterB = 0.5f - t * 0.1f;    // 0.5 -> 0.4
                filterAlpha = 0.2f + t * 0.15f + filterPulse;  // Dark overlay
                
                // Extra darkness pulse when boss is active
                if (bossActive) {
                    float bossPulse = sin(levelTime * 2.0f) * 0.05f;
                    filterAlpha += 0.1f + bossPulse;
                    filterR = 0.05f;
                    filterG = 0.05f;
                    filterB = 0.15f;
                }
            }
            
            glColor4f(filterR, filterG, filterB, filterAlpha);
            
            // Draw fullscreen overlay quad
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(-1, 1, -1, 1, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            glBegin(GL_QUADS);
            glVertex2f(-1, -1);
            glVertex2f(1, -1);
            glVertex2f(1, 1);
            glVertex2f(-1, 1);
            glEnd();
            
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_LIGHTING);
            
            return;  // Done with textured skybox + filter
        }
        
        // =====================================================
        // FALLBACK - Procedural gradient sky with transition
        // =====================================================
        glPushMatrix();
        
        // Move sky to player position so it surrounds them
        glTranslatef(lastPlayerPos.x, 0, lastPlayerPos.z);
        
        float halfSize = 90.0f;  // Size that fits within far plane
        
        // Disable lighting, depth and fog for sky - sky is always behind everything
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FOG);
        glDepthMask(GL_FALSE);
        
        float skyPulse = sin(levelTime * 0.5f) * 0.1f + 0.9f;
        
        // Sky colors based on transition progress (graceful change)
        float skyTopR, skyTopG, skyTopB;
        float skyBotR, skyBotG, skyBotB;
        
        if (transitionProgress < 0.3f) {
            // SUNSET - Orange/red sky
            float t = transitionProgress / 0.3f;
            skyTopR = 0.3f - t * 0.15f;
            skyTopG = 0.1f - t * 0.05f;
            skyTopB = 0.08f;
            skyBotR = 0.9f * skyPulse - t * 0.2f;
            skyBotG = 0.4f * skyPulse - t * 0.15f;
            skyBotB = 0.15f * skyPulse;
        } else if (transitionProgress < 0.7f) {
            // DUSK - Purple transition
            float t = (transitionProgress - 0.3f) / 0.4f;
            skyTopR = 0.15f - t * 0.1f;
            skyTopG = 0.05f;
            skyTopB = 0.08f + t * 0.1f;
            skyBotR = 0.7f * skyPulse - t * 0.4f;
            skyBotG = 0.25f * skyPulse - t * 0.1f;
            skyBotB = 0.15f * skyPulse + t * 0.25f;
        } else {
            // NIGHT - Dark blue (boss phase)
            float t = (transitionProgress - 0.7f) / 0.3f;
            skyTopR = 0.05f - t * 0.03f;
            skyTopG = 0.05f - t * 0.02f;
            skyTopB = 0.18f - t * 0.06f;
            skyBotR = 0.3f * skyPulse - t * 0.2f;
            skyBotG = 0.15f * skyPulse;
            skyBotB = 0.4f * skyPulse - t * 0.1f;
        }
        
        // =====================================================
        // HELL SKY BOX - Gradient in world space
        // =====================================================
        
        // Upper sky ceiling
        glBegin(GL_QUADS);
        glColor3f(skyTopR, skyTopG, skyTopB);
        glVertex3f(-halfSize, 150.0f, -halfSize);
        glVertex3f(halfSize, 150.0f, -halfSize);
        glVertex3f(halfSize, 150.0f, halfSize);
        glVertex3f(-halfSize, 150.0f, halfSize);
        glEnd();
        
        // Draw sky walls - 4 sides with gradient from dark top to bright bottom
        // Front wall (negative Z)
        glBegin(GL_QUADS);
        glColor3f(skyTopR, skyTopG, skyTopB);
        glVertex3f(-halfSize, 150.0f, -halfSize);
        glVertex3f(halfSize, 150.0f, -halfSize);
        glColor3f(skyBotR, skyBotG, skyBotB);
        glVertex3f(halfSize, -50.0f, -halfSize);
        glVertex3f(-halfSize, -50.0f, -halfSize);
        glEnd();
        
        // Back wall (positive Z)
        glBegin(GL_QUADS);
        glColor3f(skyTopR, skyTopG, skyTopB);
        glVertex3f(halfSize, 150.0f, halfSize);
        glVertex3f(-halfSize, 150.0f, halfSize);
        glColor3f(skyBotR, skyBotG, skyBotB);
        glVertex3f(-halfSize, -50.0f, halfSize);
        glVertex3f(halfSize, -50.0f, halfSize);
        glEnd();
        
        // Left wall (negative X)
        glBegin(GL_QUADS);
        glColor3f(skyTopR, skyTopG, skyTopB);
        glVertex3f(-halfSize, 150.0f, halfSize);
        glVertex3f(-halfSize, 150.0f, -halfSize);
        glColor3f(skyBotR, skyBotG, skyBotB);
        glVertex3f(-halfSize, -50.0f, -halfSize);
        glVertex3f(-halfSize, -50.0f, halfSize);
        glEnd();
        
        // Right wall (positive X)
        glBegin(GL_QUADS);
        glColor3f(skyTopR, skyTopG, skyTopB);
        glVertex3f(halfSize, 150.0f, -halfSize);
        glVertex3f(halfSize, 150.0f, halfSize);
        glColor3f(skyBotR, skyBotG, skyBotB);
        glVertex3f(halfSize, -50.0f, halfSize);
        glVertex3f(halfSize, -50.0f, -halfSize);
        glEnd();
        
        // MOON in the sky - Color changes based on transition progress
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float moonGlow = sin(levelTime * 0.3f) * 0.15f + 0.85f;
        
        // Moon color transitions with the sky
        float moonR, moonG, moonB;
        if (transitionProgress < 0.3f) {
            // Orange/yellow moon at sunset
            moonR = 1.0f * moonGlow;
            moonG = 0.6f * moonGlow;
            moonB = 0.2f * moonGlow;
        } else if (transitionProgress < 0.7f) {
            // Transition to blood red
            float t = (transitionProgress - 0.3f) / 0.4f;
            moonR = (1.0f - t * 0.1f) * moonGlow;
            moonG = (0.6f - t * 0.45f) * moonGlow;
            moonB = (0.2f - t * 0.1f) * moonGlow;
        } else {
            // Blue electric moon during boss phase
            float t = (transitionProgress - 0.7f) / 0.3f;
            moonR = (0.9f - t * 0.6f) * moonGlow;
            moonG = (0.15f + t * 0.35f) * moonGlow;
            moonB = (0.1f + t * 0.85f) * moonGlow;
        }
        
        glColor4f(moonR, moonG, moonB, 0.9f);
        glPushMatrix();
        glTranslatef(100.0f, 100.0f, -200.0f);
        glutSolidSphere(25.0f, 24, 24);
        glPopMatrix();
        glDisable(GL_BLEND);
        
        // Re-enable depth test, lighting and fog
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_FOG);
        
        glPopMatrix();
    }
    
    void drawWalls() {
        DEBUG_LOG("Level::drawWalls START\n");
        if (levelID == LEVEL_1_FACILITY) {
            DEBUG_LOG("Level::drawWalls calling drawFacilityWalls\n");
            drawFacilityWalls();
            DEBUG_LOG("Level::drawWalls drawFacilityWalls done\n");
        }
        // Level 2 has no walls (outdoor)
        DEBUG_LOG("Level::drawWalls COMPLETE\n");
    }
    
    void drawObjective() {
        glPushMatrix();
        glTranslatef(objective.x, objective.y, objective.z);
        
        if (levelID == LEVEL_1_FACILITY) {
            // Portal/reactor device
            float pulse = sin(levelTime * 3.0f) * 0.3f + 0.7f;
            
            // Base
            LowPolyModels::setColor(0.2f, 0.2f, 0.25f);
            glPushMatrix();
            glTranslatef(0, 0.25f, 0);
            LowPolyModels::drawBox(3.0f, 0.5f, 3.0f);
            glPopMatrix();
            
            // Core
            LowPolyModels::setColor(0.0f * pulse, 0.6f * pulse, 0.8f * pulse);
            glPushMatrix();
            glTranslatef(0, 1.5f, 0);
            glRotatef(levelTime * 50.0f, 0, 1, 0);
            glutSolidSphere(0.8f, 16, 16);
            glPopMatrix();
            
            // Ring
            LowPolyModels::setColor(0.0f, 0.8f * pulse, 1.0f * pulse);
            glPushMatrix();
            glTranslatef(0, 1.5f, 0);
            glRotatef(90, 1, 0, 0);
            glRotatef(levelTime * 30.0f, 0, 0, 1);
            glutSolidTorus(0.1f, 1.5f, 8, 24);
            glPopMatrix();
            
        } else {
            // Hell obelisk
            float glowIntensity = sin(levelTime * 2.0f) * 0.3f + 0.7f;
            LowPolyModels::drawObelisk(glowIntensity);
        }
        
        glPopMatrix();
    }
    
    // Draw a glowing path guide to the exit door - HIGHLY VISIBLE
    void drawGlowingPathToExit() {
        // Path from PLAYER'S CURRENT POSITION towards exit door (not spawn!)
        Vector3 start = lastPlayerPos;  // Use current player position!
        start.y = 0.0f;  // Floor level
        Vector3 end = exitDoor.position;
        end.y = 0.0f;
        
        // Calculate path direction and distance
        Vector3 pathDir = end - start;
        float pathLength = pathDir.length();
        if (pathLength < 1.0f) return; // Already at exit
        pathDir = pathDir.normalize();
        
        float time = levelTime;
        
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST); // Draw on top of everything
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        // MUCH MORE segments for continuous path
        int numSegments = (int)(pathLength / 0.8f);
        if (numSegments < 20) numSegments = 20;
        if (numSegments > 100) numSegments = 100;
        
        float segmentLength = pathLength / numSegments;
        
        // Animated wave moving towards exit - FASTER
        float waveSpeed = 4.0f;
        float wavePhase = fmod(time * waveSpeed, (float)numSegments);
        
        // Calculate rotation angle once
        float angle = atan2(pathDir.x, pathDir.z) * 180.0f / 3.14159f;
        
        for (int i = 0; i < numSegments; i++) {
            float t = (float)i / (float)(numSegments - 1);
            Vector3 pos = start + pathDir * (t * pathLength);
            
            // Wave animation - lights pulse in sequence towards exit
            float distFromWave = fmod(i - wavePhase + numSegments, (float)numSegments);
            float waveIntensity = 0.0f;
            
            if (distFromWave < 8.0f) {
                waveIntensity = 1.0f - distFromWave / 8.0f;
            }
            
            // BRIGHT base glow with wave overlay
            float baseGlow = 0.6f + sin(time * 4.0f + i * 0.2f) * 0.2f;
            float totalIntensity = baseGlow + waveIntensity * 0.4f;
            if (totalIntensity > 1.0f) totalIntensity = 1.0f;
            
            // ====== FLOOR LIGHT STRIP ======
            glPushMatrix();
            glTranslatef(pos.x, 0.05f, pos.z);
            glRotatef(angle, 0, 1, 0);
            
            // Main light strip segment - MUCH WIDER
            glColor4f(0.2f * totalIntensity, 1.0f * totalIntensity, 0.4f * totalIntensity, 0.9f);
            glBegin(GL_QUADS);
            glVertex3f(-0.4f, 0, -segmentLength * 0.5f);
            glVertex3f(0.4f, 0, -segmentLength * 0.5f);
            glVertex3f(0.4f, 0, segmentLength * 0.5f);
            glVertex3f(-0.4f, 0, segmentLength * 0.5f);
            glEnd();
            
            // Outer glow - WIDER
            glColor4f(0.1f * totalIntensity, 0.7f * totalIntensity, 0.25f * totalIntensity, 0.4f * totalIntensity);
            glBegin(GL_QUADS);
            glVertex3f(-1.2f, 0.02f, -segmentLength * 0.6f);
            glVertex3f(1.2f, 0.02f, -segmentLength * 0.6f);
            glVertex3f(1.2f, 0.02f, segmentLength * 0.6f);
            glVertex3f(-1.2f, 0.02f, segmentLength * 0.6f);
            glEnd();
            
            glPopMatrix();
            
            // ====== VERTICAL LIGHT BEACONS every few segments ======
            if (i % 8 == 0 && i > 0) {
                float beaconPulse = sin(time * 5.0f + i * 0.5f) * 0.3f + 0.7f;
                float beaconHeight = 3.0f + sin(time * 2.0f + i) * 0.5f;
                
                // Vertical light beam
                glPushMatrix();
                glTranslatef(pos.x, 0, pos.z);
                
                // Draw vertical beam
                glColor4f(0.1f * beaconPulse, 0.9f * beaconPulse, 0.3f * beaconPulse, 0.6f * beaconPulse);
                glBegin(GL_QUADS);
                // Front face
                glVertex3f(-0.15f, 0, 0);
                glVertex3f(0.15f, 0, 0);
                glVertex3f(0.15f, beaconHeight, 0);
                glVertex3f(-0.15f, beaconHeight, 0);
                // Side face
                glVertex3f(0, 0, -0.15f);
                glVertex3f(0, 0, 0.15f);
                glVertex3f(0, beaconHeight, 0.15f);
                glVertex3f(0, beaconHeight, -0.15f);
                glEnd();
                
                // Top glow sphere
                glColor4f(0.3f * beaconPulse, 1.0f * beaconPulse, 0.5f * beaconPulse, 0.8f * beaconPulse);
                glTranslatef(0, beaconHeight, 0);
                glutSolidSphere(0.25f * beaconPulse, 8, 8);
                
                glPopMatrix();
            }
            
            // ====== ARROW MARKERS every few segments ======
            if (i % 6 == 3 && i < numSegments - 3) {
                glPushMatrix();
                glTranslatef(pos.x, 0.08f, pos.z);
                glRotatef(angle, 0, 1, 0);
                
                // Arrow shape pointing towards exit - BIGGER
                glColor4f(0.4f * totalIntensity, 1.0f * totalIntensity, 0.6f * totalIntensity, 0.9f);
                glBegin(GL_TRIANGLES);
                glVertex3f(-0.6f, 0, 0.4f);
                glVertex3f(0.6f, 0, 0.4f);
                glVertex3f(0.0f, 0, -0.8f);
                glEnd();
                
                glPopMatrix();
            }
        }
        
        // ====== LARGE PULSING BEACON AT EXIT ======
        float beaconPulse = sin(time * 4.0f) * 0.3f + 0.7f;
        glPushMatrix();
        glTranslatef(end.x, 0, end.z);
        
        // Large ground ring
        glColor4f(0.2f * beaconPulse, 1.0f * beaconPulse, 0.4f * beaconPulse, 0.7f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0.05f, 0);
        for (int a = 0; a <= 24; a++) {
            float ang = a * 3.14159f * 2.0f / 24.0f;
            float r = 3.0f * beaconPulse;
            glVertex3f(cos(ang) * r, 0.05f, sin(ang) * r);
        }
        glEnd();
        
        // Inner bright ring
        glColor4f(0.5f * beaconPulse, 1.0f * beaconPulse, 0.7f * beaconPulse, 0.9f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0.08f, 0);
        for (int a = 0; a <= 24; a++) {
            float ang = a * 3.14159f * 2.0f / 24.0f;
            glVertex3f(cos(ang) * 1.5f, 0.08f, sin(ang) * 1.5f);
        }
        glEnd();
        
        // Tall vertical beacon at exit
        float exitBeamHeight = 8.0f;
        glColor4f(0.3f * beaconPulse, 1.0f * beaconPulse, 0.5f * beaconPulse, 0.5f);
        glBegin(GL_QUADS);
        glVertex3f(-0.3f, 0, -0.3f);
        glVertex3f(0.3f, 0, -0.3f);
        glVertex3f(0.3f, exitBeamHeight, -0.3f);
        glVertex3f(-0.3f, exitBeamHeight, -0.3f);
        
        glVertex3f(-0.3f, 0, 0.3f);
        glVertex3f(0.3f, 0, 0.3f);
        glVertex3f(0.3f, exitBeamHeight, 0.3f);
        glVertex3f(-0.3f, exitBeamHeight, 0.3f);
        
        glVertex3f(-0.3f, 0, -0.3f);
        glVertex3f(-0.3f, 0, 0.3f);
        glVertex3f(-0.3f, exitBeamHeight, 0.3f);
        glVertex3f(-0.3f, exitBeamHeight, -0.3f);
        
        glVertex3f(0.3f, 0, -0.3f);
        glVertex3f(0.3f, 0, 0.3f);
        glVertex3f(0.3f, exitBeamHeight, 0.3f);
        glVertex3f(0.3f, exitBeamHeight, -0.3f);
        glEnd();
        
        glPopMatrix();
        
        // ====== "EXIT" TEXT INDICATOR floating above exit ======
        glPushMatrix();
        glTranslatef(end.x, 4.0f + sin(time * 2.0f) * 0.3f, end.z);
        
        // Billboard facing player
        Vector3 toPlayer = lastPlayerPos - end;
        toPlayer.y = 0;
        float textAngle = atan2(toPlayer.x, toPlayer.z) * 180.0f / 3.14159f;
        glRotatef(textAngle, 0, 1, 0);
        
        // Draw "EXIT" as a glowing panel
        glColor4f(0.3f * beaconPulse, 1.0f * beaconPulse, 0.5f * beaconPulse, 0.9f);
        glBegin(GL_QUADS);
        glVertex3f(-1.5f, -0.4f, 0);
        glVertex3f(1.5f, -0.4f, 0);
        glVertex3f(1.5f, 0.4f, 0);
        glVertex3f(-1.5f, 0.4f, 0);
        glEnd();
        
        glPopMatrix();
        
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }
    
    void draw() {
        DEBUG_LOG("Level::draw START\n");
        drawFloor();
        DEBUG_LOG("Level::draw floor done\n");
        drawWalls();
        DEBUG_LOG("Level::draw walls done\n");
        
        // Draw platforms (with distance culling)
        DEBUG_LOG("Level::draw platforms START\n");
        for (int i = 0; i < numPlatforms; i++) {
            // Distance culling
            float dist = lastPlayerPos.distanceTo(platforms[i].center);
            if (dist > drawDistance) continue;
            
            glPushMatrix();
            glTranslatef(platforms[i].center.x, platforms[i].center.y, platforms[i].center.z);
            
            if (levelID == LEVEL_2_HELL_ARENA) {
                // Different textures for different platform types in hell arena
                TextureID platTex;
                float texScale = 0.5f;
                
                // Main arena platform (index 0) - large stone texture
                if (i == 0) {
                    platTex = TextureManager::isLoaded(TEX_FLOOR_METAL) ? TEX_FLOOR_METAL : TEX_ROCK;
                    texScale = 0.15f;  // More texture repeats for large platform
                }
                // Corner platforms (indices 1-4) - different rocky texture
                else if (i >= 1 && i <= 4) {
                    platTex = TextureManager::isLoaded(TEX_WALL_GREY) ? TEX_WALL_GREY : TEX_ROCK;
                    texScale = 0.4f;
                }
                // Boss platform (index 9) - special dark texture
                else if (i == 9 || i == 10 || i == 11) {
                    platTex = TextureManager::isLoaded(TEX_PLATFORM_LAVA) ? TEX_PLATFORM_LAVA : TEX_ROCK;
                    texScale = 0.3f;
                }
                // Other platforms - standard rock texture
                else {
                    platTex = TextureManager::isLoaded(TEX_ROCK) ? TEX_ROCK : TEX_PLATFORM_LAVA;
                    texScale = 0.5f;
                }
                
                if (TextureManager::isLoaded(platTex)) {
                    TextureManager::drawTexturedBox(platTex, 0, 0, 0,
                        platforms[i].size.x, platforms[i].size.y, platforms[i].size.z, texScale);
                } else {
                    // Fallback with different colors based on platform index
                    if (i == 0) {
                        LowPolyModels::setColor(0.35f, 0.3f, 0.25f);  // Lighter for main
                    } else if (i >= 1 && i <= 4) {
                        LowPolyModels::setColor(0.45f, 0.35f, 0.25f);  // Warmer for corners
                    } else if (i >= 9 && i <= 11) {
                        LowPolyModels::setColor(0.25f, 0.15f, 0.1f);  // Darker for boss area
                    } else {
                        LowPolyModels::setColor(0.4f, 0.25f, 0.15f);  // Standard rock color
                    }
                    LowPolyModels::drawPlatform(platforms[i].size.x, platforms[i].size.y, platforms[i].size.z);
                }
            } else {
                // Draw textured metal platform for facility
                if (TextureManager::isLoaded(TEX_PLATFORM)) {
                    TextureManager::drawTexturedBox(TEX_PLATFORM, 0, 0, 0,
                        platforms[i].size.x, platforms[i].size.y, platforms[i].size.z, 0.3f);
                } else {
                    LowPolyModels::setColor(0.35f, 0.35f, 0.38f);
                    LowPolyModels::drawPlatform(platforms[i].size.x, platforms[i].size.y, platforms[i].size.z);
                }
            }
            glPopMatrix();
        }
        DEBUG_LOG("Level::draw platforms done\n");
        
        // Draw demonic structures for Level 2 (ceiling, columns, pillars)
        if (levelID == LEVEL_2_HELL_ARENA) {
            drawDemonicStructures();
        }
        
        // Draw crates/rocks (with distance culling)
        DEBUG_LOG("Level::draw crates START\n");
        for (int i = 0; i < numCrates; i++) {
            // Distance culling
            float dist = lastPlayerPos.distanceTo(crates[i].position);
            if (dist > drawDistance) continue;
            
            if (levelID == LEVEL_2_HELL_ARENA) {
                // Mystery boxes use the animated 3D model, rocks use textured rocks
                if (crates[i].isMysteryBox) {
                    crates[i].draw();  // Use animated mystery box with 3D model
                } else {
                    // Non-mystery boxes are cover rocks
                    glPushMatrix();
                    glTranslatef(crates[i].position.x, crates[i].position.y, crates[i].position.z);
                    
                    // Draw textured lava rock
                    if (TextureManager::isLoaded(TEX_ROCK)) {
                        TextureManager::drawTexturedBox(TEX_ROCK, 0, crates[i].size * 0.5f, 0,
                            crates[i].size, crates[i].size, crates[i].size, 0.5f);
                    } else {
                        LowPolyModels::drawLavaRock(crates[i].size);
                    }
                    
                    glPopMatrix();
                }
            } else {
                crates[i].draw();
            }
        }
        DEBUG_LOG("Level::draw crates done\n");
        
        // Draw parkour obstacles (with distance culling)
        DEBUG_LOG("Level::draw parkour obstacles START\n");
        for (int i = 0; i < numParkourObstacles; i++) {
            float dist = lastPlayerPos.distanceTo(parkourObstacles[i].position);
            if (dist > drawDistance) continue;
            parkourObstacles[i].draw();
        }
        DEBUG_LOG("Level::draw parkour obstacles done\n");
        
        // Draw doors (with distance culling)
        DEBUG_LOG("Level::draw doors START\n");
        for (int i = 0; i < numDoors; i++) {
            float dist = lastPlayerPos.distanceTo(doors[i].position);
            if (dist > drawDistance) continue;
            doors[i].draw();
        }
        DEBUG_LOG("Level::draw doors done\n");
        
        // Draw exit door (if all enemies killed) - use portal for Level 2
        if (allEnemiesKilled || exitDoor.isActive || bossKilledPortalReady) {
            if (levelID == LEVEL_2_HELL_ARENA && bossKilledPortalReady) {
                exitDoor.drawAsPortal();  // Victory portal for Level 2
            } else {
                exitDoor.draw();
            }
            
            // Draw glowing path guide to exit door when all enemies killed
            if (allEnemiesKilled && exitDoor.isActive) {
                drawGlowingPathToExit();
            }
        }
        
        // Draw enemies (with distance culling)
        DEBUG_LOG("Level::draw enemies START\n");
        for (int i = 0; i < numEnemies; i++) {
            // Enemies have looser culling - they need to be visible from further
            float dist = lastPlayerPos.distanceTo(enemies[i].position);
            if (dist > drawDistance + 20.0f) continue;
            enemies[i].draw();
            
            // Draw boss projectiles
            if (enemies[i].type == ENEMY_BOSS) {
                enemies[i].drawProjectiles();
            }
        }
        DEBUG_LOG("Level::draw enemies done\n");
        
        // Draw collectibles (with distance culling)
        DEBUG_LOG("Level::draw collectibles START\n");
        for (int i = 0; i < numCollectibles; i++) {
            float dist = lastPlayerPos.distanceTo(collectibles[i].position);
            if (dist > drawDistance) continue;
            collectibles[i].draw();
        }
        DEBUG_LOG("Level::draw collectibles done\n");
        
        // Draw objective
        DEBUG_LOG("Level::draw objective START\n");
        drawObjective();
        DEBUG_LOG("Level::draw COMPLETE\n");
    }
    
    bool isComplete() const {
        return objectiveReached;
    }
    
    bool isTimeUp() const {
        return levelTime >= maxTime;
    }
    
    int getRemainingTime() const {
        return (int)(maxTime - levelTime);
    }
    
    bool areAllEnemiesKilled() const {
        // For Level 2 with boss phase system
        if (levelID == LEVEL_2_HELL_ARENA) {
            // All enemies killed when boss phase started AND boss is dead
            if (!bossPhaseStarted) return false; // Still clearing regular enemies
            // Boss must be dead
            if (bossEnemyIndex >= 0 && bossEnemyIndex < numEnemies) {
                return !enemies[bossEnemyIndex].active;
            }
            return regularEnemiesCleared; // Fallback
        }
        // For other levels, check all enemies
        for (int i = 0; i < numEnemies; i++) {
            if (enemies[i].active) return false;
        }
        return true;
    }
};

#endif // LEVEL_H
