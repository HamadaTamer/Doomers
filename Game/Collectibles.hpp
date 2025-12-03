/**
 * DOOMERS - Collectibles and Interactive Objects
 * 
 * Based on project requirements:
 * - Health packs
 * - Ammo boxes (magazines)
 * - Keycards (to unlock security doors)
 * - Score system integration
 * 
 * Animations:
 * - Collectible rotates and scales up slightly, then disappears
 * - Pickup sound effect
 * - Score immediately increases
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Animation.hpp"
#include <vector>
#include <functional>
#include <string>

namespace Doomers {

// ============================================================================
// Collectible Types
// ============================================================================
enum class CollectibleType {
    HealthPack,
    AmmoPistol,
    AmmoShotgun,
    AmmoRifle,
    KeycardRed,
    KeycardBlue,
    KeycardYellow,
    Armor
};

// ============================================================================
// Base Collectible
// ============================================================================
class Collectible {
public:
    CollectibleType type;
    Math::Vector3 position;
    float rotation = 0;
    float rotationSpeed = 90.0f; // Degrees per second
    float bobOffset = 0;
    float bobSpeed = 2.0f;
    float bobAmount = 0.2f;
    float scale = 1.0f;
    float baseScale = 1.0f;
    bool active = true;
    bool collected = false;
    
    // Collection animation
    float collectTimer = 0;
    float collectDuration = 0.3f;
    float collectScaleMultiplier = 1.5f;
    
    // Value
    int value = 0;
    int scoreValue = 10;
    
    // Collision
    float pickupRadius = 1.0f;
    
    // Respawn (optional)
    bool canRespawn = false;
    float respawnTime = 30.0f;
    float respawnTimer = 0;
    
    Collectible(CollectibleType t, const Math::Vector3& pos) 
        : type(t), position(pos) {
        setupType();
    }
    
    void setupType() {
        switch (type) {
            case CollectibleType::HealthPack:
                value = 25;
                scoreValue = 10;
                pickupRadius = 1.0f;
                break;
            case CollectibleType::AmmoPistol:
                value = 12;
                scoreValue = 5;
                pickupRadius = 0.8f;
                break;
            case CollectibleType::AmmoShotgun:
                value = 8;
                scoreValue = 5;
                pickupRadius = 0.8f;
                break;
            case CollectibleType::AmmoRifle:
                value = 30;
                scoreValue = 5;
                pickupRadius = 0.8f;
                break;
            case CollectibleType::KeycardRed:
            case CollectibleType::KeycardBlue:
            case CollectibleType::KeycardYellow:
                value = 1;
                scoreValue = 50;
                pickupRadius = 1.0f;
                bobAmount = 0.3f;
                break;
            case CollectibleType::Armor:
                value = 50;
                scoreValue = 15;
                pickupRadius = 1.0f;
                break;
        }
    }
    
    void update(float deltaTime, float time) {
        if (collected) {
            // Collection animation
            collectTimer += deltaTime;
            float t = collectTimer / collectDuration;
            
            // Scale up then disappear
            if (t < 0.5f) {
                scale = baseScale * (1.0f + (collectScaleMultiplier - 1.0f) * (t * 2.0f));
            } else {
                scale = baseScale * collectScaleMultiplier * (1.0f - (t - 0.5f) * 2.0f);
            }
            
            // Faster rotation during collection
            rotation += rotationSpeed * 5.0f * deltaTime;
            
            if (t >= 1.0f) {
                active = false;
                if (canRespawn) {
                    respawnTimer = respawnTime;
                }
            }
            return;
        }
        
        if (!active) {
            // Handle respawn
            if (canRespawn && respawnTimer > 0) {
                respawnTimer -= deltaTime;
                if (respawnTimer <= 0) {
                    active = true;
                    collected = false;
                    scale = baseScale;
                }
            }
            return;
        }
        
        // Idle animation
        rotation += rotationSpeed * deltaTime;
        if (rotation >= 360.0f) rotation -= 360.0f;
        
        bobOffset = sinf(time * bobSpeed) * bobAmount;
    }
    
    bool checkPickup(const Math::Vector3& playerPos) {
        if (!active || collected) return false;
        
        float dist = (position - playerPos).length();
        return dist < pickupRadius;
    }
    
    void collect() {
        if (active && !collected) {
            collected = true;
            collectTimer = 0;
        }
    }
    
    void render() const {
        if (!active) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y + bobOffset, position.z);
        glRotatef(rotation, 0, 1, 0);
        glScalef(scale, scale, scale);
        
        // Draw based on type
        switch (type) {
            case CollectibleType::HealthPack:
                renderHealthPack();
                break;
            case CollectibleType::AmmoPistol:
            case CollectibleType::AmmoShotgun:
            case CollectibleType::AmmoRifle:
                renderAmmoBox();
                break;
            case CollectibleType::KeycardRed:
                renderKeycard(Math::Color::red());
                break;
            case CollectibleType::KeycardBlue:
                renderKeycard(Math::Color::blue());
                break;
            case CollectibleType::KeycardYellow:
                renderKeycard(Math::Color::yellow());
                break;
            case CollectibleType::Armor:
                renderArmor();
                break;
        }
        
        glPopMatrix();
    }
    
private:
    void renderHealthPack() const {
        glDisable(GL_LIGHTING);
        
        // White box with red cross
        float size = 0.3f;
        
        // Box
        glColor3f(0.9f, 0.9f, 0.9f);
        glBegin(GL_QUADS);
        // Front
        glVertex3f(-size, -size*0.5f, size);
        glVertex3f(size, -size*0.5f, size);
        glVertex3f(size, size*0.5f, size);
        glVertex3f(-size, size*0.5f, size);
        // Back
        glVertex3f(-size, -size*0.5f, -size);
        glVertex3f(size, -size*0.5f, -size);
        glVertex3f(size, size*0.5f, -size);
        glVertex3f(-size, size*0.5f, -size);
        // Top
        glVertex3f(-size, size*0.5f, -size);
        glVertex3f(size, size*0.5f, -size);
        glVertex3f(size, size*0.5f, size);
        glVertex3f(-size, size*0.5f, size);
        glEnd();
        
        // Red cross on top
        glColor3f(1.0f, 0.0f, 0.0f);
        float cs = size * 0.8f;
        float cw = size * 0.2f;
        glBegin(GL_QUADS);
        // Horizontal bar
        glVertex3f(-cs, size*0.51f, -cw);
        glVertex3f(cs, size*0.51f, -cw);
        glVertex3f(cs, size*0.51f, cw);
        glVertex3f(-cs, size*0.51f, cw);
        // Vertical bar
        glVertex3f(-cw, size*0.51f, -cs);
        glVertex3f(cw, size*0.51f, -cs);
        glVertex3f(cw, size*0.51f, cs);
        glVertex3f(-cw, size*0.51f, cs);
        glEnd();
        
        glEnable(GL_LIGHTING);
    }
    
    void renderAmmoBox() const {
        glDisable(GL_LIGHTING);
        
        float size = 0.25f;
        
        // Military green box
        glColor3f(0.3f, 0.35f, 0.25f);
        glBegin(GL_QUADS);
        // All faces
        // Front
        glVertex3f(-size, -size*0.6f, size*0.5f);
        glVertex3f(size, -size*0.6f, size*0.5f);
        glVertex3f(size, size*0.6f, size*0.5f);
        glVertex3f(-size, size*0.6f, size*0.5f);
        // Back
        glVertex3f(-size, -size*0.6f, -size*0.5f);
        glVertex3f(size, -size*0.6f, -size*0.5f);
        glVertex3f(size, size*0.6f, -size*0.5f);
        glVertex3f(-size, size*0.6f, -size*0.5f);
        // Top
        glVertex3f(-size, size*0.6f, -size*0.5f);
        glVertex3f(size, size*0.6f, -size*0.5f);
        glVertex3f(size, size*0.6f, size*0.5f);
        glVertex3f(-size, size*0.6f, size*0.5f);
        glEnd();
        
        // Yellow stripe
        glColor3f(0.8f, 0.7f, 0.2f);
        glBegin(GL_QUADS);
        glVertex3f(-size*0.8f, size*0.61f, -size*0.1f);
        glVertex3f(size*0.8f, size*0.61f, -size*0.1f);
        glVertex3f(size*0.8f, size*0.61f, size*0.1f);
        glVertex3f(-size*0.8f, size*0.61f, size*0.1f);
        glEnd();
        
        glEnable(GL_LIGHTING);
    }
    
    void renderKeycard(const Math::Color& color) const {
        glDisable(GL_LIGHTING);
        
        // Flat card shape
        float w = 0.3f;
        float h = 0.2f;
        float d = 0.02f;
        
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_QUADS);
        // Front
        glVertex3f(-w, -h, d);
        glVertex3f(w, -h, d);
        glVertex3f(w, h, d);
        glVertex3f(-w, h, d);
        // Back
        glVertex3f(-w, -h, -d);
        glVertex3f(w, -h, -d);
        glVertex3f(w, h, -d);
        glVertex3f(-w, h, -d);
        glEnd();
        
        // White stripe
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex3f(-w*0.8f, -h*0.3f, d*1.1f);
        glVertex3f(w*0.8f, -h*0.3f, d*1.1f);
        glVertex3f(w*0.8f, h*0.1f, d*1.1f);
        glVertex3f(-w*0.8f, h*0.1f, d*1.1f);
        glEnd();
        
        // Glow effect
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(color.r, color.g, color.b, 0.3f);
        float glow = 0.5f;
        glBegin(GL_QUADS);
        glVertex3f(-w-glow, -h-glow, 0);
        glVertex3f(w+glow, -h-glow, 0);
        glVertex3f(w+glow, h+glow, 0);
        glVertex3f(-w-glow, h+glow, 0);
        glEnd();
        glDisable(GL_BLEND);
        
        glEnable(GL_LIGHTING);
    }
    
    void renderArmor() const {
        glDisable(GL_LIGHTING);
        
        // Blue armor vest shape
        float size = 0.35f;
        
        glColor3f(0.2f, 0.3f, 0.6f);
        glBegin(GL_QUADS);
        // Front
        glVertex3f(-size, -size, size*0.3f);
        glVertex3f(size, -size, size*0.3f);
        glVertex3f(size*0.7f, size, size*0.3f);
        glVertex3f(-size*0.7f, size, size*0.3f);
        glEnd();
        
        // Trim
        glColor3f(0.4f, 0.5f, 0.8f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-size, -size, size*0.31f);
        glVertex3f(size, -size, size*0.31f);
        glVertex3f(size*0.7f, size, size*0.31f);
        glVertex3f(-size*0.7f, size, size*0.31f);
        glEnd();
        
        glEnable(GL_LIGHTING);
    }
};

// ============================================================================
// Security Door - Requires keycard to open
// ============================================================================
enum class DoorState {
    Closed,
    Opening,
    Open,
    Closing
};

enum class KeycardColor {
    None,
    Red,
    Blue,
    Yellow
};

class SecurityDoor {
public:
    Math::Vector3 position;
    float rotation = 0; // Y rotation in degrees
    KeycardColor requiredKeycard = KeycardColor::None;
    DoorState state = DoorState::Closed;
    
    // Door dimensions
    float width = 2.0f;
    float height = 3.0f;
    float thickness = 0.2f;
    
    // Animation
    float openProgress = 0;
    float openSpeed = 2.0f;
    float openDistance = 2.5f; // How far the door slides
    
    // Two door panels (sliding doors)
    bool isDoubleDoor = true;
    
    // Auto close
    bool autoClose = true;
    float autoCloseDelay = 3.0f;
    float autoCloseTimer = 0;
    
    // Trigger distance
    float triggerDistance = 3.0f;
    
    // Locked message
    std::string lockedMessage = "Requires keycard";
    
    SecurityDoor(const Math::Vector3& pos, KeycardColor keycard = KeycardColor::None)
        : position(pos), requiredKeycard(keycard) {
        setupMessage();
    }
    
    void setupMessage() {
        switch (requiredKeycard) {
            case KeycardColor::Red:
                lockedMessage = "Requires RED keycard";
                break;
            case KeycardColor::Blue:
                lockedMessage = "Requires BLUE keycard";
                break;
            case KeycardColor::Yellow:
                lockedMessage = "Requires YELLOW keycard";
                break;
            default:
                lockedMessage = "";
                break;
        }
    }
    
    bool tryOpen(bool hasRed, bool hasBlue, bool hasYellow) {
        if (state == DoorState::Open || state == DoorState::Opening) {
            return true;
        }
        
        // Check keycard requirement
        bool canOpen = false;
        switch (requiredKeycard) {
            case KeycardColor::None:
                canOpen = true;
                break;
            case KeycardColor::Red:
                canOpen = hasRed;
                break;
            case KeycardColor::Blue:
                canOpen = hasBlue;
                break;
            case KeycardColor::Yellow:
                canOpen = hasYellow;
                break;
        }
        
        if (canOpen) {
            state = DoorState::Opening;
            return true;
        }
        
        return false;
    }
    
    void update(float deltaTime) {
        switch (state) {
            case DoorState::Opening:
                openProgress += openSpeed * deltaTime;
                if (openProgress >= 1.0f) {
                    openProgress = 1.0f;
                    state = DoorState::Open;
                    autoCloseTimer = autoCloseDelay;
                }
                break;
                
            case DoorState::Open:
                if (autoClose) {
                    autoCloseTimer -= deltaTime;
                    if (autoCloseTimer <= 0) {
                        state = DoorState::Closing;
                    }
                }
                break;
                
            case DoorState::Closing:
                openProgress -= openSpeed * deltaTime;
                if (openProgress <= 0) {
                    openProgress = 0;
                    state = DoorState::Closed;
                }
                break;
                
            default:
                break;
        }
    }
    
    bool isBlocking() const {
        return state == DoorState::Closed || state == DoorState::Closing;
    }
    
    void render() const {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation, 0, 1, 0);
        
        // Door frame
        renderFrame();
        
        // Door panels
        float slide = openProgress * openDistance;
        
        if (isDoubleDoor) {
            // Left panel
            glPushMatrix();
            glTranslatef(-slide, 0, 0);
            renderPanel(-width * 0.25f);
            glPopMatrix();
            
            // Right panel
            glPushMatrix();
            glTranslatef(slide, 0, 0);
            renderPanel(width * 0.25f);
            glPopMatrix();
        } else {
            // Single sliding door
            glPushMatrix();
            glTranslatef(slide, 0, 0);
            renderPanel(0);
            glPopMatrix();
        }
        
        // Keycard indicator light
        renderIndicator();
        
        glPopMatrix();
    }
    
private:
    void renderFrame() const {
        glDisable(GL_LIGHTING);
        
        // Dark metal frame
        glColor3f(0.2f, 0.2f, 0.25f);
        float frameThick = 0.15f;
        
        // Top frame
        glBegin(GL_QUADS);
        glVertex3f(-width/2 - frameThick, height, -thickness);
        glVertex3f(width/2 + frameThick, height, -thickness);
        glVertex3f(width/2 + frameThick, height + frameThick, -thickness);
        glVertex3f(-width/2 - frameThick, height + frameThick, -thickness);
        glEnd();
        
        // Left frame
        glBegin(GL_QUADS);
        glVertex3f(-width/2 - frameThick, 0, -thickness);
        glVertex3f(-width/2, 0, -thickness);
        glVertex3f(-width/2, height + frameThick, -thickness);
        glVertex3f(-width/2 - frameThick, height + frameThick, -thickness);
        glEnd();
        
        // Right frame
        glBegin(GL_QUADS);
        glVertex3f(width/2, 0, -thickness);
        glVertex3f(width/2 + frameThick, 0, -thickness);
        glVertex3f(width/2 + frameThick, height + frameThick, -thickness);
        glVertex3f(width/2, height + frameThick, -thickness);
        glEnd();
        
        glEnable(GL_LIGHTING);
    }
    
    void renderPanel(float xOffset) const {
        glDisable(GL_LIGHTING);
        
        float panelWidth = isDoubleDoor ? width * 0.5f : width;
        
        // Main panel
        glColor3f(0.4f, 0.4f, 0.45f);
        glBegin(GL_QUADS);
        // Front
        glVertex3f(xOffset - panelWidth/2, 0, 0);
        glVertex3f(xOffset + panelWidth/2, 0, 0);
        glVertex3f(xOffset + panelWidth/2, height, 0);
        glVertex3f(xOffset - panelWidth/2, height, 0);
        // Back
        glVertex3f(xOffset - panelWidth/2, 0, -thickness);
        glVertex3f(xOffset + panelWidth/2, 0, -thickness);
        glVertex3f(xOffset + panelWidth/2, height, -thickness);
        glVertex3f(xOffset - panelWidth/2, height, -thickness);
        glEnd();
        
        // Warning stripes
        glColor3f(0.8f, 0.6f, 0.0f);
        float stripeY = height * 0.7f;
        glBegin(GL_QUADS);
        glVertex3f(xOffset - panelWidth/2 + 0.1f, stripeY, 0.01f);
        glVertex3f(xOffset + panelWidth/2 - 0.1f, stripeY, 0.01f);
        glVertex3f(xOffset + panelWidth/2 - 0.1f, stripeY + 0.1f, 0.01f);
        glVertex3f(xOffset - panelWidth/2 + 0.1f, stripeY + 0.1f, 0.01f);
        glEnd();
        
        glEnable(GL_LIGHTING);
    }
    
    void renderIndicator() const {
        glDisable(GL_LIGHTING);
        
        // Position indicator near the door frame
        float indicatorX = width/2 + 0.3f;
        float indicatorY = height * 0.6f;
        float indicatorSize = 0.1f;
        
        // Determine color
        Math::Color indicatorColor;
        if (state == DoorState::Open || state == DoorState::Opening) {
            indicatorColor = Math::Color::green();
        } else {
            switch (requiredKeycard) {
                case KeycardColor::Red:
                    indicatorColor = Math::Color::red();
                    break;
                case KeycardColor::Blue:
                    indicatorColor = Math::Color::blue();
                    break;
                case KeycardColor::Yellow:
                    indicatorColor = Math::Color::yellow();
                    break;
                default:
                    indicatorColor = Math::Color::green();
                    break;
            }
        }
        
        // Draw indicator light
        glColor3f(indicatorColor.r, indicatorColor.g, indicatorColor.b);
        glBegin(GL_QUADS);
        glVertex3f(indicatorX, indicatorY - indicatorSize, 0.01f);
        glVertex3f(indicatorX + indicatorSize*2, indicatorY - indicatorSize, 0.01f);
        glVertex3f(indicatorX + indicatorSize*2, indicatorY + indicatorSize, 0.01f);
        glVertex3f(indicatorX, indicatorY + indicatorSize, 0.01f);
        glEnd();
        
        // Glow
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(indicatorColor.r, indicatorColor.g, indicatorColor.b, 0.3f);
        float glow = indicatorSize * 2;
        glBegin(GL_QUADS);
        glVertex3f(indicatorX - glow, indicatorY - indicatorSize - glow, 0.02f);
        glVertex3f(indicatorX + indicatorSize*2 + glow, indicatorY - indicatorSize - glow, 0.02f);
        glVertex3f(indicatorX + indicatorSize*2 + glow, indicatorY + indicatorSize + glow, 0.02f);
        glVertex3f(indicatorX - glow, indicatorY + indicatorSize + glow, 0.02f);
        glEnd();
        glDisable(GL_BLEND);
        
        glEnable(GL_LIGHTING);
    }
};

// ============================================================================
// Score System
// ============================================================================
class ScoreSystem {
public:
    int score = 0;
    int enemiesKilled = 0;
    int collectiblesObtained = 0;
    float timeTaken = 0;
    
    // Score multipliers
    float comboMultiplier = 1.0f;
    float comboTimer = 0;
    float comboDuration = 3.0f;
    int comboKills = 0;
    
    // Bonuses
    int killBonus = 100;
    int headshotBonus = 50;
    int collectibleBonus = 10;
    int keycardBonus = 50;
    int levelCompleteBonus = 500;
    int timeBonus = 1000; // Full bonus if under target time
    float targetTime = 300.0f; // 5 minutes
    
    void addKill(bool headshot = false) {
        enemiesKilled++;
        
        int points = killBonus;
        if (headshot) points += headshotBonus;
        
        // Combo system
        comboKills++;
        comboTimer = comboDuration;
        comboMultiplier = 1.0f + (comboKills - 1) * 0.1f;
        if (comboMultiplier > 3.0f) comboMultiplier = 3.0f;
        
        addScore((int)(points * comboMultiplier));
    }
    
    void addCollectible(CollectibleType type) {
        collectiblesObtained++;
        
        int points = collectibleBonus;
        if (type == CollectibleType::KeycardRed || 
            type == CollectibleType::KeycardBlue || 
            type == CollectibleType::KeycardYellow) {
            points = keycardBonus;
        }
        
        addScore(points);
    }
    
    void addScore(int points) {
        score += points;
    }
    
    void update(float deltaTime) {
        timeTaken += deltaTime;
        
        // Update combo
        if (comboTimer > 0) {
            comboTimer -= deltaTime;
            if (comboTimer <= 0) {
                comboMultiplier = 1.0f;
                comboKills = 0;
            }
        }
    }
    
    void completeLevel() {
        // Level completion bonus
        addScore(levelCompleteBonus);
        
        // Time bonus (decreases as time increases)
        if (timeTaken < targetTime) {
            float timeRatio = 1.0f - (timeTaken / targetTime);
            addScore((int)(timeBonus * timeRatio));
        }
    }
    
    int getFinalScore() const {
        return score;
    }
    
    std::string getGrade() const {
        if (score >= 5000) return "S";
        if (score >= 4000) return "A";
        if (score >= 3000) return "B";
        if (score >= 2000) return "C";
        if (score >= 1000) return "D";
        return "F";
    }
};

// ============================================================================
// Collectibles Manager
// ============================================================================
class CollectiblesManager {
public:
    std::vector<Collectible> collectibles;
    std::vector<SecurityDoor> doors;
    ScoreSystem scoreSystem;
    
    // Player inventory
    bool hasRedKeycard = false;
    bool hasBlueKeycard = false;
    bool hasYellowKeycard = false;
    
    // Callbacks
    std::function<void(int)> onHealthPickup;
    std::function<void(CollectibleType, int)> onAmmoPickup;
    std::function<void(CollectibleType)> onKeycardPickup;
    std::function<void(int)> onArmorPickup;
    std::function<void(const std::string&)> onMessage;
    
    void addCollectible(CollectibleType type, const Math::Vector3& pos) {
        collectibles.emplace_back(type, pos);
    }
    
    void addDoor(const Math::Vector3& pos, float rotation, KeycardColor keycard) {
        SecurityDoor door(pos, keycard);
        door.rotation = rotation;
        doors.push_back(door);
    }
    
    void update(float deltaTime, const Math::Vector3& playerPos, float time) {
        scoreSystem.update(deltaTime);
        
        // Update collectibles
        for (auto& item : collectibles) {
            item.update(deltaTime, time);
            
            if (item.active && !item.collected && item.checkPickup(playerPos)) {
                collectItem(item);
            }
        }
        
        // Update doors
        for (auto& door : doors) {
            door.update(deltaTime);
            
            // Check if player is near and try to open
            float dist = (door.position - playerPos).length();
            if (dist < door.triggerDistance) {
                if (!door.tryOpen(hasRedKeycard, hasBlueKeycard, hasYellowKeycard)) {
                    // Door is locked - show message
                    if (onMessage && !door.lockedMessage.empty()) {
                        // Only show message occasionally
                        // (would need a timer to prevent spam)
                    }
                }
            }
        }
    }
    
    void collectItem(Collectible& item) {
        item.collect();
        scoreSystem.addCollectible(item.type);
        
        switch (item.type) {
            case CollectibleType::HealthPack:
                if (onHealthPickup) onHealthPickup(item.value);
                if (onMessage) onMessage("Health +" + std::to_string(item.value));
                break;
                
            case CollectibleType::AmmoPistol:
            case CollectibleType::AmmoShotgun:
            case CollectibleType::AmmoRifle:
                if (onAmmoPickup) onAmmoPickup(item.type, item.value);
                if (onMessage) onMessage("Ammo +" + std::to_string(item.value));
                break;
                
            case CollectibleType::KeycardRed:
                hasRedKeycard = true;
                if (onKeycardPickup) onKeycardPickup(item.type);
                if (onMessage) onMessage("RED KEYCARD acquired!");
                break;
                
            case CollectibleType::KeycardBlue:
                hasBlueKeycard = true;
                if (onKeycardPickup) onKeycardPickup(item.type);
                if (onMessage) onMessage("BLUE KEYCARD acquired!");
                break;
                
            case CollectibleType::KeycardYellow:
                hasYellowKeycard = true;
                if (onKeycardPickup) onKeycardPickup(item.type);
                if (onMessage) onMessage("YELLOW KEYCARD acquired!");
                break;
                
            case CollectibleType::Armor:
                if (onArmorPickup) onArmorPickup(item.value);
                if (onMessage) onMessage("Armor +" + std::to_string(item.value));
                break;
        }
    }
    
    void tryOpenDoor(int doorIndex) {
        if (doorIndex >= 0 && doorIndex < (int)doors.size()) {
            SecurityDoor& door = doors[doorIndex];
            if (!door.tryOpen(hasRedKeycard, hasBlueKeycard, hasYellowKeycard)) {
                if (onMessage) onMessage(door.lockedMessage);
            }
        }
    }
    
    void render() const {
        for (const auto& item : collectibles) {
            item.render();
        }
        
        for (const auto& door : doors) {
            door.render();
        }
    }
    
    void reset() {
        collectibles.clear();
        doors.clear();
        hasRedKeycard = false;
        hasBlueKeycard = false;
        hasYellowKeycard = false;
        scoreSystem = ScoreSystem();
    }
};

} // namespace Doomers
