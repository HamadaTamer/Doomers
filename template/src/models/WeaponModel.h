// ============================================================================
// DOOMERS - WeaponModel.h
// Detailed weapon models with effects
// ============================================================================
#ifndef WEAPON_MODEL_H
#define WEAPON_MODEL_H

#include "ModelUtils.h"
#include "PlayerModel.h"
#include "../TextureManager.h"

namespace WeaponModel {

    using namespace ModelUtils;

    // ==================== ASSAULT RIFLE ====================
    inline void drawAssaultRifleDetailed(float recoil, bool firing, float weaponLight, float reloadPhase = 0.0f) {
        glPushMatrix();
        
        // Apply recoil
        glTranslatef(0, 0, recoil * 0.12f);
        glRotatef(recoil * 8, 1, 0, 0);
        
        float scale = 1.3f; // Bigger weapon
        glScalef(scale, scale, scale);
        
        // === MAIN RECEIVER === (TEXTURED)
        if (TextureManager::isLoaded(TEX_WEAPON_METAL)) {
            GLuint metalTex = TextureManager::get(TEX_WEAPON_METAL);
            drawTexturedBox(0.065f, 0.11f, 0.55f, metalTex);
        } else {
            setColorMetallic(0.1f, 0.1f, 0.12f);
            drawBox(0.065f, 0.11f, 0.55f);
        }
        
        // Upper receiver with rails (TEXTURED)
        glPushMatrix();
        glTranslatef(0, 0.065f, -0.08f);
        if (TextureManager::isLoaded(TEX_WEAPON_METAL)) {
            GLuint metalTex = TextureManager::get(TEX_WEAPON_METAL);
            drawTexturedBox(0.055f, 0.045f, 0.4f, metalTex);
        } else {
            setColorMetallic(0.08f, 0.08f, 0.1f);
            drawBox(0.055f, 0.045f, 0.4f);
        }
        
        // Picatinny rail detail
        setColorMetallic(0.12f, 0.12f, 0.14f);
        for (int i = 0; i < 8; i++) {
            glPushMatrix();
            glTranslatef(0, 0.025f, -0.15f + i * 0.04f);
            drawBox(0.05f, 0.008f, 0.015f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // === BARREL ASSEMBLY ===
        // Outer barrel shroud with ventilation
        setColorMetallic(0.13f, 0.13f, 0.15f);
        glPushMatrix();
        glTranslatef(0, 0.02f, -0.32f);
        drawBox(0.06f, 0.075f, 0.32f);
        
        // Ventilation holes
        glDisable(GL_LIGHTING);
        glColor3f(0.03f, 0.03f, 0.03f);
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glTranslatef(0.032f, 0.02f, -0.05f + i * 0.06f);
            drawBox(0.005f, 0.03f, 0.02f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(-0.032f, 0.02f, -0.05f + i * 0.06f);
            drawBox(0.005f, 0.03f, 0.02f);
            glPopMatrix();
        }
        glEnable(GL_LIGHTING);
        glPopMatrix();
        
        // Inner barrel
        setColorMetallic(0.06f, 0.06f, 0.08f);
        glPushMatrix();
        glTranslatef(0, 0.02f, -0.52f);
        drawCylinder(0.018f, 0.45f, 12);
        glPopMatrix();
        
        // Muzzle device (flash hider)
        setColorMetallic(0.08f, 0.08f, 0.1f);
        glPushMatrix();
        glTranslatef(0, 0.02f, -0.72f);
        drawBox(0.04f, 0.04f, 0.1f);
        // Prongs
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glRotatef(i * 90, 0, 0, 1);
            glTranslatef(0.025f, 0, -0.03f);
            drawBox(0.012f, 0.008f, 0.06f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // === GAS BLOCK & FRONT SIGHT ===
        setColorMetallic(0.1f, 0.1f, 0.1f);
        glPushMatrix();
        glTranslatef(0, 0.06f, -0.42f);
        drawBox(0.04f, 0.05f, 0.05f);
        // Front sight post
        glTranslatef(0, 0.05f, 0);
        drawBox(0.008f, 0.06f, 0.008f);
        // Protective wings
        setColorMetallic(0.08f, 0.08f, 0.08f);
        for (int i = -1; i <= 1; i += 2) {
            glPushMatrix();
            glTranslatef(i * 0.018f, 0.02f, 0);
            drawBox(0.005f, 0.04f, 0.015f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // === STOCK (Adjustable) ===
        setColorMetallic(0.1f, 0.1f, 0.12f);
        glPushMatrix();
        glTranslatef(0, 0.01f, 0.32f);
        // Buffer tube
        drawCylinder(0.03f, 0.18f, 10);
        glTranslatef(0, 0.18f, 0);
        glRotatef(90, 1, 0, 0);
        // Stock body
        setColor(0.08f, 0.06f, 0.04f);
        glPushMatrix();
        glTranslatef(0, 0.08f, -0.02f);
        drawBox(0.05f, 0.16f, 0.08f);
        glPopMatrix();
        // Cheek rest
        glPushMatrix();
        glTranslatef(0, 0.12f, 0.03f);
        drawBox(0.045f, 0.08f, 0.04f);
        glPopMatrix();
        // Buttpad
        setColor(0.06f, 0.04f, 0.03f);
        glPushMatrix();
        glTranslatef(0, 0.17f, -0.02f);
        drawBox(0.055f, 0.02f, 0.1f);
        glPopMatrix();
        glPopMatrix();
        
        // === MAGAZINE (with reload animation) ===
        glPushMatrix();
        float magOffset = sin(reloadPhase * 3.14159f) * 0.15f; // Magazine drop during reload
        glTranslatef(0, -0.15f - magOffset, 0.06f);
        glRotatef(6 + reloadPhase * 20, 1, 0, 0);
        
        setColorMetallic(0.1f, 0.1f, 0.1f);
        drawBox(0.038f, 0.2f, 0.065f);
        
        // Magazine ridges
        setColorMetallic(0.08f, 0.08f, 0.08f);
        for (int i = 0; i < 5; i++) {
            glPushMatrix();
            glTranslatef(0, -0.08f + i * 0.035f, 0.034f);
            drawBox(0.035f, 0.01f, 0.003f);
            glPopMatrix();
        }
        
        // Floor plate
        setColorMetallic(0.12f, 0.12f, 0.12f);
        glPushMatrix();
        glTranslatef(0, -0.095f, 0);
        drawBox(0.04f, 0.015f, 0.07f);
        glPopMatrix();
        glPopMatrix();
        
        // === PISTOL GRIP === (TEXTURED)
        glPushMatrix();
        glTranslatef(0, -0.12f, 0.2f);
        glRotatef(-18, 1, 0, 0);
        if (TextureManager::isLoaded(TEX_WEAPON_GRIP)) {
            GLuint gripTex = TextureManager::get(TEX_WEAPON_GRIP);
            drawTexturedBox(0.04f, 0.14f, 0.05f, gripTex);
        } else {
            setColor(0.07f, 0.05f, 0.03f);
            drawBox(0.04f, 0.14f, 0.05f);
            // Grip texture lines
            setColor(0.05f, 0.03f, 0.02f);
            for (int i = 0; i < 4; i++) {
                glPushMatrix();
                glTranslatef(0.021f, -0.03f + i * 0.025f, 0);
                drawBox(0.003f, 0.012f, 0.045f);
                glPopMatrix();
            }
        }
        glPopMatrix();
        
        // === FOREGRIP (Angled) === (TEXTURED)
        glPushMatrix();
        glTranslatef(0, -0.08f, -0.18f);
        glRotatef(-25, 1, 0, 0);
        if (TextureManager::isLoaded(TEX_WEAPON_GRIP)) {
            GLuint gripTex = TextureManager::get(TEX_WEAPON_GRIP);
            drawTexturedBox(0.035f, 0.12f, 0.045f, gripTex);
        } else {
            setColor(0.07f, 0.05f, 0.03f);
            drawBox(0.035f, 0.12f, 0.045f);
        }
        glPopMatrix();
        
        // === OPTIC (Red Dot Sight) ===
        setColorMetallic(0.06f, 0.06f, 0.08f);
        glPushMatrix();
        glTranslatef(0, 0.13f, 0.02f);
        
        // Sight body
        drawBox(0.045f, 0.055f, 0.1f);
        
        // Front window
        setColor(0.1f, 0.15f, 0.2f);
        glPushMatrix();
        glTranslatef(0, 0, -0.048f);
        drawBox(0.035f, 0.04f, 0.005f);
        glPopMatrix();
        
        // Rear window
        glPushMatrix();
        glTranslatef(0, 0, 0.048f);
        drawBox(0.03f, 0.035f, 0.005f);
        glPopMatrix();
        
        // Brightness dial
        setColorMetallic(0.15f, 0.15f, 0.15f);
        glPushMatrix();
        glTranslatef(0.025f, 0, 0);
        drawCylinder(0.012f, 0.015f, 8);
        glPopMatrix();
        
        // Red dot (glowing)
        setColor(1.0f, 0.1f, 0.1f);
        setEmissive(0.8f, 0.0f, 0.0f);
        glPushMatrix();
        glTranslatef(0, 0.01f, -0.02f);
        drawSphere(0.008f, 6);
        glPopMatrix();
        clearEmissive();
        glPopMatrix();
        
        // === TACTICAL FLASHLIGHT ===
        setColorMetallic(0.08f, 0.08f, 0.08f);
        glPushMatrix();
        glTranslatef(0.055f, 0.02f, -0.22f);
        
        // Light body
        drawCylinder(0.022f, 0.1f, 10);
        
        // Lens bezel
        setColorMetallic(0.1f, 0.1f, 0.1f);
        glPushMatrix();
        glTranslatef(0, 0.1f, 0);
        drawCylinder(0.025f, 0.015f, 10);
        glPopMatrix();
        
        // Light lens (glows when on)
        glPushMatrix();
        glTranslatef(0, 0.115f, 0);
        if (weaponLight > 0.5f) {
            setColor(1.0f, 0.98f, 0.85f);
            setEmissive(weaponLight * 0.9f, weaponLight * 0.85f, weaponLight * 0.6f);
        } else {
            setColor(0.4f, 0.38f, 0.35f);
        }
        drawSphere(0.02f, 10);
        clearEmissive();
        glPopMatrix();
        
        // Activation switch
        setColor(0.05f, 0.05f, 0.05f);
        glPushMatrix();
        glTranslatef(-0.02f, 0.05f, 0);
        drawBox(0.015f, 0.02f, 0.01f);
        glPopMatrix();
        glPopMatrix();
        
        // === LASER SIGHT (Side-mounted) ===
        setColorMetallic(0.07f, 0.07f, 0.07f);
        glPushMatrix();
        glTranslatef(-0.05f, 0.035f, -0.2f);
        drawBox(0.025f, 0.025f, 0.06f);
        
        // Laser aperture
        setColor(0.6f, 0.08f, 0.08f);
        setEmissive(0.4f, 0.0f, 0.0f);
        glPushMatrix();
        glTranslatef(0, 0, -0.032f);
        drawSphere(0.01f, 6);
        glPopMatrix();
        clearEmissive();
        glPopMatrix();
        
        // === CHARGING HANDLE ===
        setColorMetallic(0.12f, 0.12f, 0.12f);
        glPushMatrix();
        glTranslatef(0, 0.08f, 0.22f);
        drawBox(0.06f, 0.025f, 0.04f);
        // Latch
        glTranslatef(0, 0.015f, -0.015f);
        drawBox(0.025f, 0.015f, 0.02f);
        glPopMatrix();
        
        // === EJECTION PORT COVER ===
        setColorMetallic(0.1f, 0.1f, 0.1f);
        glPushMatrix();
        glTranslatef(0.035f, 0.04f, 0.05f);
        drawBox(0.005f, 0.05f, 0.08f);
        glPopMatrix();
        
        // === SLING MOUNT POINTS ===
        setColorMetallic(0.15f, 0.15f, 0.15f);
        glPushMatrix();
        glTranslatef(0, -0.05f, 0.28f);
        drawBox(0.02f, 0.04f, 0.015f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0, -0.02f, -0.38f);
        drawBox(0.015f, 0.03f, 0.015f);
        glPopMatrix();
        
        // === MUZZLE FLASH ===
        if (firing) {
            enableGlow();
            
            glPushMatrix();
            glTranslatef(0, 0.02f, -0.82f);
            
            // Intense core
            glColor4f(1.0f, 1.0f, 0.9f, 1.0f);
            drawSphere(0.06f, 8);
            
            // Inner flash
            glColor4f(1.0f, 0.95f, 0.5f, 0.9f);
            drawBox(0.12f, 0.12f, 0.18f);
            
            // Mid flash
            glColor4f(1.0f, 0.7f, 0.2f, 0.7f);
            drawBox(0.2f, 0.2f, 0.12f);
            
            // Outer flash
            glColor4f(1.0f, 0.4f, 0.1f, 0.4f);
            drawBox(0.3f, 0.3f, 0.08f);
            
            // Spark jets
            glColor4f(1.0f, 0.8f, 0.3f, 0.5f);
            for (int i = 0; i < 6; i++) {
                glPushMatrix();
                float angle = i * 60.0f + getTime() * 800.0f;
                float sparkLen = 0.15f + sin(getTime() * 20 + i) * 0.08f;
                glRotatef(angle, 0, 0, 1);
                glTranslatef(0.08f, 0, 0);
                drawBox(sparkLen, 0.025f, 0.025f);
                glPopMatrix();
            }
            
            glPopMatrix();
            
            disableGlow();
        }
        
        // Weapon light illumination on weapon itself
        if (weaponLight > 0.5f) {
            // Subtle glow on front of weapon
            enableGlow();
            glColor4f(1.0f, 0.95f, 0.8f, 0.15f * weaponLight);
            glPushMatrix();
            glTranslatef(0.03f, 0.02f, -0.35f);
            drawBox(0.15f, 0.12f, 0.2f);
            glPopMatrix();
            disableGlow();
        }
        
        glPopMatrix();
    }

    // First person weapon view - REDESIGNED for smooth gameplay
    inline void drawWeaponFirstPerson(float recoil, float bob, bool firing, float weaponLight, float reloadPhase = 0.0f) {
        glPushMatrix();
        
        // Base weapon position - centered and stable
        // Reduced bob effect for smoother feel
        float smoothBob = bob * 0.15f;  // Much less bob
        float smoothRecoil = recoil * 0.08f;  // Subtle recoil
        
        glTranslatef(0.22f, -0.18f + smoothBob, -0.45f + smoothRecoil);
        glRotatef(-2 + recoil * 12, 1, 0, 0);  // Less recoil rotation
        glRotatef(3, 0, 1, 0);
        glRotatef(-2, 0, 0, 1);
        
        // Reload animation
        if (reloadPhase > 0) {
            glRotatef(sin(reloadPhase * 3.14159f) * -25, 1, 0, 0);
            glTranslatef(0, sin(reloadPhase * 3.14159f) * -0.08f, 0);
        }
        
        glScalef(0.85f, 0.85f, 0.85f);
        drawAssaultRifleDetailed(recoil, firing, weaponLight, reloadPhase);
        
        // Draw arms - with reduced bob
        PlayerModel::drawPlayerArms(recoil * 0.6f, bob * 0.3f);
        
        glPopMatrix();
    }

    // Compatibility wrapper
    inline void drawAssaultRifle() {
        drawAssaultRifleDetailed(0, false, 0);
    }
}

#endif // WEAPON_MODEL_H
