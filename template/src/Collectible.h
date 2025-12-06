// ============================================================================
// DOOMERS - Collectible.h
// Pickups: Health packs, ammo boxes, keycards
// ============================================================================
#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "Vector3.h"
#include "GameConfig.h"
#include "LowPolyModels.h"
#include <glut.h>

enum CollectibleType {
    COLLECT_HEALTH,
    COLLECT_AMMO,
    COLLECT_KEYCARD
};

class Collectible {
public:
    Vector3 position;
    CollectibleType type;
    bool active;
    float rotation;
    float bobPhase;
    float pickupScale;
    bool beingCollected;
    int value;
    int keycardID; // For keycards: which door it opens
    
    Collectible() {
        position = Vector3(0, 0, 0);
        type = COLLECT_HEALTH;
        active = false;
        rotation = 0.0f;
        bobPhase = 0.0f;
        pickupScale = 1.0f;
        beingCollected = false;
        value = 0;
        keycardID = 0;
    }
    
    void init(CollectibleType t, const Vector3& pos, int val = 0) {
        type = t;
        position = pos;
        active = true;
        rotation = 0.0f;
        bobPhase = (float)(rand() % 100) / 100.0f * 6.28f;
        pickupScale = 1.0f;
        beingCollected = false;
        
        switch (type) {
            case COLLECT_HEALTH:
                value = (val > 0) ? val : HEALTH_PACK_HEAL;
                break;
            case COLLECT_AMMO:
                value = (val > 0) ? val : AMMO_PICKUP_AMOUNT;
                break;
            case COLLECT_KEYCARD:
                value = 1;
                keycardID = val;
                break;
        }
    }
    
    void update(float deltaTime) {
        if (!active) return;
        
        // Rotate and bob
        rotation += deltaTime * 60.0f;
        if (rotation > 360.0f) rotation -= 360.0f;
        
        bobPhase += deltaTime * 3.0f;
        
        // Collection animation - fast dramatic pickup
        if (beingCollected) {
            pickupScale += deltaTime * 8.0f; // Faster scaling
            rotation += deltaTime * 360.0f; // Spin fast when collected
            if (pickupScale > 2.0f) {
                active = false;
            }
        }
    }
    
    bool checkCollection(const Vector3& playerPos, float collectRadius = 2.5f) {
        if (!active || beingCollected) return false;
        
        // Use horizontal distance only (ignore Y) so player can pick up items
        float dx = position.x - playerPos.x;
        float dz = position.z - playerPos.z;
        float horizontalDist = sqrt(dx * dx + dz * dz);
        
        // Player position.y is at eye level (includes PLAYER_HEIGHT)
        // Collectible is at ground level (y=0 to y=0.5 with bob)
        // So player foot is at playerPos.y - PLAYER_HEIGHT
        float playerFeetY = playerPos.y - 1.8f; // PLAYER_HEIGHT
        float collectibleY = position.y + 0.5f; // Collectible visual center
        float dy = fabs(playerFeetY - collectibleY);
        
        // Allow collection if within 2.5 units vertically (for platforms)
        if (dy > 2.5f) return false;
        
        if (horizontalDist < collectRadius) {
            beingCollected = true;
            return true;
        }
        return false;
    }
    
    void draw() {
        if (!active) return;
        
        glPushMatrix();
        
        // Position with bob
        float bob = sin(bobPhase) * 0.15f;
        glTranslatef(position.x, position.y + 0.5f + bob, position.z);
        glRotatef(rotation, 0, 1, 0);
        
        // Collection animation
        if (beingCollected) {
            glScalef(pickupScale, pickupScale, pickupScale);
            
            // Fade out
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            float alpha = 1.0f - (pickupScale - 1.0f) * 2.0f;
            glColor4f(1, 1, 1, alpha);
        }
        
        // Draw based on type
        switch (type) {
            case COLLECT_HEALTH:
                LowPolyModels::drawHealthPack();
                break;
            case COLLECT_AMMO:
                LowPolyModels::drawAmmoBox();
                break;
            case COLLECT_KEYCARD:
                drawKeycard();
                break;
        }
        
        if (beingCollected) {
            glDisable(GL_BLEND);
        }
        
        glPopMatrix();
        
        // Draw glow effect
        drawGlow();
    }
    
    void drawKeycard() {
        glPushMatrix();
        
        // Card body
        float r = 0.2f, g = 0.2f, b = 0.8f;
        if (keycardID == 1) { r = 0.8f; g = 0.2f; b = 0.2f; }      // Red
        else if (keycardID == 2) { r = 0.2f; g = 0.8f; b = 0.2f; } // Green
        else if (keycardID == 3) { r = 0.8f; g = 0.8f; b = 0.2f; } // Yellow
        
        LowPolyModels::setColor(r, g, b);
        LowPolyModels::drawBox(0.3f, 0.02f, 0.2f);
        
        // Chip
        LowPolyModels::setColor(0.8f, 0.75f, 0.2f);
        glPushMatrix();
        glTranslatef(-0.05f, 0.015f, 0);
        LowPolyModels::drawBox(0.1f, 0.01f, 0.08f);
        glPopMatrix();
        
        // Stripe
        LowPolyModels::setColor(0.9f, 0.9f, 0.9f);
        glPushMatrix();
        glTranslatef(0.05f, 0.015f, 0);
        LowPolyModels::drawBox(0.15f, 0.01f, 0.04f);
        glPopMatrix();
        
        glPopMatrix();
    }
    
    void drawGlow() {
        if (beingCollected) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y + 0.1f, position.z);
        
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        // Pulsing glow
        float pulse = sin(bobPhase * 2.0f) * 0.2f + 0.8f;
        
        float r = 0.5f, g = 0.5f, b = 0.5f;
        switch (type) {
            case COLLECT_HEALTH:
                r = 0.2f; g = 0.8f; b = 0.2f;
                break;
            case COLLECT_AMMO:
                r = 0.9f; g = 0.7f; b = 0.1f;
                break;
            case COLLECT_KEYCARD:
                if (keycardID == 1) { r = 0.8f; g = 0.2f; b = 0.2f; }
                else if (keycardID == 2) { r = 0.2f; g = 0.8f; b = 0.2f; }
                else { r = 0.2f; g = 0.2f; b = 0.8f; }
                break;
        }
        
        glColor4f(r, g, b, 0.3f * pulse);
        glutSolidSphere(0.6f, 12, 12);
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        glPopMatrix();
    }
};

#endif // COLLECTIBLE_H
