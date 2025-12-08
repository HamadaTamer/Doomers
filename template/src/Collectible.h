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
    COLLECT_KEYCARD,
    // New powerups for Level 2
    COLLECT_SPEED_BOOST,
    COLLECT_DAMAGE_BOOST,
    COLLECT_INVINCIBILITY,
    COLLECT_MAX_AMMO,
    COLLECT_SHIELD  // Protection from boss attacks
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
            case COLLECT_SPEED_BOOST:
                value = (int)(POWERUP_DURATION);
                break;
            case COLLECT_DAMAGE_BOOST:
                value = (int)(POWERUP_DURATION);
                break;
            case COLLECT_INVINCIBILITY:
                value = (int)(INVINCIBILITY_DURATION);
                break;
            case COLLECT_MAX_AMMO:
                value = MAX_AMMO;
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
            case COLLECT_SPEED_BOOST:
                drawSpeedBoost();
                break;
            case COLLECT_DAMAGE_BOOST:
                drawDamageBoost();
                break;
            case COLLECT_INVINCIBILITY:
                drawInvincibility();
                break;
            case COLLECT_MAX_AMMO:
                drawMaxAmmo();
                break;
            case COLLECT_SHIELD:
                drawShield();
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
    
    // Speed boost powerup - lightning bolt shape
    void drawSpeedBoost() {
        float pulse = sin(bobPhase * 3.0f) * 0.2f + 0.8f;
        
        // Glowing base sphere
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);
        glColor4f(0.0f, 0.8f * pulse, 1.0f * pulse, 0.4f);
        glutSolidSphere(0.35f, 12, 12);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        
        // Lightning bolt (simplified triangular shape)
        LowPolyModels::setColor(0.2f, 0.9f * pulse, 1.0f * pulse);
        GLfloat emit[] = {0.1f, 0.5f * pulse, 0.6f * pulse, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, emit);
        
        glBegin(GL_TRIANGLES);
        // Top part of bolt
        glVertex3f(-0.1f, 0.3f, 0);
        glVertex3f(0.15f, 0.15f, 0);
        glVertex3f(-0.05f, 0.1f, 0);
        // Bottom part of bolt
        glVertex3f(0.1f, -0.3f, 0);
        glVertex3f(-0.15f, -0.15f, 0);
        glVertex3f(0.05f, -0.1f, 0);
        // Middle connector
        glVertex3f(-0.05f, 0.1f, 0);
        glVertex3f(0.15f, 0.15f, 0);
        glVertex3f(0.05f, -0.1f, 0);
        glEnd();
        
        GLfloat noEmit[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    }
    
    // Damage boost powerup - fiery skull/orb
    void drawDamageBoost() {
        float pulse = sin(bobPhase * 2.5f) * 0.25f + 0.75f;
        
        // Fiery glow
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);
        glColor4f(1.0f * pulse, 0.4f * pulse, 0.0f, 0.5f);
        glutSolidSphere(0.4f, 12, 12);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        
        // Core orb with fire colors
        LowPolyModels::setColor(1.0f * pulse, 0.3f * pulse, 0.1f);
        GLfloat emit[] = {0.6f * pulse, 0.2f * pulse, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, emit);
        glutSolidSphere(0.2f, 10, 10);
        
        // Fire spikes
        for (int i = 0; i < 6; i++) {
            float angle = i * 60.0f + rotation;
            glPushMatrix();
            glRotatef(angle, 0, 1, 0);
            glTranslatef(0.25f, 0, 0);
            glScalef(0.08f, 0.15f, 0.08f);
            glutSolidCone(1.0f, 2.0f, 4, 1);
            glPopMatrix();
        }
        
        GLfloat noEmit[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    }
    
    // Invincibility powerup - golden star/shield
    void drawInvincibility() {
        float pulse = sin(bobPhase * 4.0f) * 0.3f + 0.7f;
        
        // Golden aura
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);
        glColor4f(1.0f * pulse, 0.85f * pulse, 0.2f, 0.4f);
        glutSolidSphere(0.45f, 12, 12);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        
        // Inner shield shape
        LowPolyModels::setColor(1.0f, 0.9f * pulse, 0.3f * pulse);
        GLfloat emit[] = {0.5f * pulse, 0.45f * pulse, 0.1f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, emit);
        
        // Draw as a flattened hexagonal shield shape
        glPushMatrix();
        glScalef(1.0f, 1.2f, 0.3f);
        glutSolidSphere(0.2f, 6, 6);
        glPopMatrix();
        
        // Star points around shield
        for (int i = 0; i < 5; i++) {
            float angle = i * 72.0f + rotation * 0.5f;
            glPushMatrix();
            glRotatef(angle, 0, 0, 1);
            glTranslatef(0.3f, 0, 0);
            glutSolidSphere(0.06f, 6, 6);
            glPopMatrix();
        }
        
        GLfloat noEmit[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    }
    
    // Max ammo powerup - ammo crate with glow
    void drawMaxAmmo() {
        float pulse = sin(bobPhase * 2.0f) * 0.2f + 0.8f;
        
        // Yellow glow
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);
        glColor4f(1.0f * pulse, 0.8f * pulse, 0.1f, 0.3f);
        glutSolidSphere(0.5f, 10, 10);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        
        // Enhanced ammo box
        LowPolyModels::setColor(0.9f * pulse, 0.75f * pulse, 0.2f);
        GLfloat emit[] = {0.3f * pulse, 0.25f * pulse, 0.05f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, emit);
        
        // Main crate
        LowPolyModels::drawBox(0.3f, 0.2f, 0.25f);
        
        // Bullet symbol on top
        LowPolyModels::setColor(0.8f, 0.4f, 0.1f);
        glPushMatrix();
        glTranslatef(0, 0.12f, 0);
        LowPolyModels::drawBox(0.05f, 0.1f, 0.05f);
        glPopMatrix();
        
        GLfloat noEmit[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    }
    
    // SHIELD POWERUP - Protects from boss attacks
    void drawShield() {
        float pulse = sin(bobPhase * 3.0f) * 0.25f + 0.75f;
        float time = bobPhase;
        
        // Outer energy field glow
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);
        
        // Rotating energy rings
        for (int ring = 0; ring < 3; ring++) {
            float ringAngle = time * 100.0f + ring * 120.0f;
            float ringAlpha = 0.3f + sin(time * 2.0f + ring) * 0.15f;
            glColor4f(0.2f, 0.6f * pulse, 1.0f * pulse, ringAlpha);
            
            glPushMatrix();
            glRotatef(ringAngle, ring % 2, 1, (ring + 1) % 2);
            glutSolidTorus(0.02f, 0.35f + ring * 0.05f, 8, 20);
            glPopMatrix();
        }
        
        // Central shield bubble
        glColor4f(0.3f * pulse, 0.7f * pulse, 1.0f, 0.4f);
        glutSolidSphere(0.28f, 16, 16);
        
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        
        // Inner core - crystalline
        LowPolyModels::setColor(0.4f * pulse, 0.8f * pulse, 1.0f);
        GLfloat emit[] = {0.2f * pulse, 0.5f * pulse, 0.8f * pulse, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, emit);
        
        // Hexagonal shield shape
        glPushMatrix();
        glRotatef(time * 30.0f, 0, 1, 0);
        
        // Front face
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        glVertex3f(0, 0, 0.1f);
        for (int i = 0; i <= 6; i++) {
            float angle = i * 60.0f * 3.14159f / 180.0f;
            float r = 0.18f;
            glVertex3f(cos(angle) * r, sin(angle) * r, 0.1f);
        }
        glEnd();
        
        // Back face
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, -1);
        glVertex3f(0, 0, -0.1f);
        for (int i = 6; i >= 0; i--) {
            float angle = i * 60.0f * 3.14159f / 180.0f;
            float r = 0.18f;
            glVertex3f(cos(angle) * r, sin(angle) * r, -0.1f);
        }
        glEnd();
        
        glPopMatrix();
        
        // Floating particles around shield
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);
        glPointSize(4.0f);
        glBegin(GL_POINTS);
        for (int p = 0; p < 8; p++) {
            float pAngle = p * 45.0f + time * 80.0f;
            float pRad = pAngle * 3.14159f / 180.0f;
            float pDist = 0.4f + sin(time * 3.0f + p) * 0.08f;
            float px = cos(pRad) * pDist;
            float py = sin(time * 2.0f + p * 0.5f) * 0.15f;
            float pz = sin(pRad) * pDist;
            
            glColor4f(0.4f, 0.8f, 1.0f, 0.7f);
            glVertex3f(px, py, pz);
        }
        glEnd();
        glPointSize(1.0f);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        
        GLfloat noEmit2[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmit2);
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
            case COLLECT_SPEED_BOOST:
                r = 0.0f; g = 0.8f; b = 1.0f;
                break;
            case COLLECT_DAMAGE_BOOST:
                r = 1.0f; g = 0.4f; b = 0.0f;
                break;
            case COLLECT_INVINCIBILITY:
                r = 1.0f; g = 0.85f; b = 0.2f;
                break;
            case COLLECT_MAX_AMMO:
                r = 1.0f; g = 0.8f; b = 0.1f;
                break;
            case COLLECT_SHIELD:
                r = 0.3f; g = 0.7f; b = 1.0f;
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
