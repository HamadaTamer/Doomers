// ============================================================================
// DOOMERS - PlayerModel.h
// ULTIMATE DETAILED SOLDIER MODEL - Professional Military Operator
// Using advanced OpenGL primitives for maximum visual quality
// ============================================================================
#ifndef PLAYER_MODEL_H
#define PLAYER_MODEL_H

#include "ModelUtils.h"

namespace PlayerModel {

    using namespace ModelUtils;

    // Scale factor for player - Matches PLAYER_HEIGHT (1.8f)
    // Model head is at Y~1.2, so 1.2 * 1.5 = 1.8
    const float PLAYER_SCALE = 1.5f;

    // ==================== HELPER: Draw Tactical Armor Plate ====================
    inline void drawArmorPlate(float width, float height, float depth, float bevelSize = 0.02f) {
        // Main plate
        drawBox(width, height, depth);
        
        // Edge bevels for realism
        setColorMetallic(0.25f, 0.27f, 0.24f);
        glPushMatrix();
        glTranslatef(0, height/2 - bevelSize, 0);
        drawBox(width - bevelSize*2, bevelSize*2, depth + 0.01f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0, -height/2 + bevelSize, 0);
        drawBox(width - bevelSize*2, bevelSize*2, depth + 0.01f);
        glPopMatrix();
    }

    // ==================== HELPER: Draw Military Boot ====================
    inline void drawMilitaryBoot(float walkAnim) {
        glPushMatrix();
        glRotatef(walkAnim, 1, 0, 0);
        
        // Boot base - rugged tactical boot
        setColor(0.08f, 0.06f, 0.05f);
        drawBox(0.12f, 0.18f, 0.22f);
        
        // Boot sole - thick rubber
        setColor(0.03f, 0.03f, 0.03f);
        glPushMatrix();
        glTranslatef(0, -0.08f, 0.02f);
        drawBox(0.14f, 0.04f, 0.26f);
        // Tread pattern
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glTranslatef(0, -0.02f, -0.08f + i * 0.06f);
            drawBox(0.12f, 0.015f, 0.04f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Boot tongue
        setColor(0.12f, 0.1f, 0.08f);
        glPushMatrix();
        glTranslatef(0, 0.02f, 0.1f);
        drawBox(0.08f, 0.14f, 0.03f);
        glPopMatrix();
        
        // Laces
        setColor(0.02f, 0.02f, 0.02f);
        for (int i = 0; i < 5; i++) {
            glPushMatrix();
            glTranslatef(0, -0.04f + i * 0.04f, 0.115f);
            drawBox(0.06f, 0.012f, 0.01f);
            glPopMatrix();
        }
        
        // Ankle support
        setColor(0.1f, 0.08f, 0.06f);
        glPushMatrix();
        glTranslatef(0, 0.12f, 0);
        drawBox(0.13f, 0.08f, 0.18f);
        glPopMatrix();
        
        glPopMatrix();
    }

    // ==================== HELPER: Draw Tactical Pants Leg ====================
    inline void drawTacticalLeg(float walkAnim, bool isLeft) {
        glPushMatrix();
        glRotatef(walkAnim * 0.8f, 1, 0, 0);
        
        // Thigh
        setColor(0.18f, 0.2f, 0.16f); // OD Green tactical
        drawBox(0.13f, 0.28f, 0.14f);
        
        // Thigh cargo pocket
        setColor(0.16f, 0.18f, 0.14f);
        glPushMatrix();
        glTranslatef(isLeft ? -0.07f : 0.07f, -0.02f, 0);
        drawBox(0.04f, 0.12f, 0.12f);
        // Pocket flap
        setColor(0.15f, 0.17f, 0.13f);
        glTranslatef(0, 0.05f, 0.01f);
        drawBox(0.038f, 0.03f, 0.01f);
        glPopMatrix();
        
        // Knee pad mount
        setColorMetallic(0.12f, 0.12f, 0.1f);
        glPushMatrix();
        glTranslatef(0, -0.18f, 0.08f);
        drawBox(0.1f, 0.12f, 0.04f);
        glPopMatrix();
        
        // Knee pad (rubber)
        setColor(0.06f, 0.06f, 0.06f);
        glPushMatrix();
        glTranslatef(0, -0.18f, 0.105f);
        drawBox(0.09f, 0.1f, 0.03f);
        // Pad ridges
        for (int i = 0; i < 3; i++) {
            glPushMatrix();
            glTranslatef(0, -0.03f + i * 0.03f, 0.018f);
            drawBox(0.07f, 0.015f, 0.01f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Lower leg / shin
        glPushMatrix();
        glTranslatef(0, -0.32f, 0);
        setColor(0.18f, 0.2f, 0.16f);
        drawBox(0.11f, 0.2f, 0.12f);
        glPopMatrix();
        
        glPopMatrix();
    }

    // ==================== HELPER: Draw Tactical Vest / Plate Carrier ====================
    inline void drawPlateCarrier() {
        // Front plate
        setColorMetallic(0.15f, 0.17f, 0.13f);
        glPushMatrix();
        glTranslatef(0, 0.05f, 0.12f);
        drawArmorPlate(0.28f, 0.32f, 0.05f);
        glPopMatrix();
        
        // Back plate
        glPushMatrix();
        glTranslatef(0, 0.05f, -0.12f);
        drawArmorPlate(0.28f, 0.32f, 0.05f);
        glPopMatrix();
        
        // Shoulder straps
        setColor(0.14f, 0.16f, 0.12f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.12f, 0.18f, 0);
            drawBox(0.06f, 0.06f, 0.22f);
            glPopMatrix();
        }
        
        // MOLLE webbing on front (horizontal straps)
        setColor(0.13f, 0.15f, 0.11f);
        for (int row = 0; row < 4; row++) {
            glPushMatrix();
            glTranslatef(0, 0.12f - row * 0.06f, 0.148f);
            drawBox(0.26f, 0.02f, 0.008f);
            glPopMatrix();
        }
        
        // Magazine pouches (triple)
        setColor(0.12f, 0.14f, 0.1f);
        for (int i = -1; i <= 1; i++) {
            glPushMatrix();
            glTranslatef(i * 0.07f, -0.08f, 0.16f);
            drawBox(0.055f, 0.12f, 0.04f);
            // Mag visible inside
            setColorMetallic(0.08f, 0.08f, 0.08f);
            glTranslatef(0, 0.02f, 0.005f);
            drawBox(0.04f, 0.06f, 0.025f);
            glPopMatrix();
        }
        
        // Admin pouch (top center)
        setColor(0.13f, 0.15f, 0.11f);
        glPushMatrix();
        glTranslatef(0, 0.16f, 0.155f);
        drawBox(0.12f, 0.06f, 0.03f);
        glPopMatrix();
        
        // Side cummerbund
        setColor(0.14f, 0.16f, 0.12f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.16f, 0, 0);
            drawBox(0.04f, 0.28f, 0.18f);
            glPopMatrix();
        }
        
        // Radio pouch (left side)
        setColor(0.1f, 0.1f, 0.1f);
        glPushMatrix();
        glTranslatef(-0.19f, 0.08f, 0.02f);
        drawBox(0.04f, 0.1f, 0.06f);
        // Radio antenna
        setColorMetallic(0.2f, 0.2f, 0.2f);
        glTranslatef(0, 0.08f, 0);
        drawCylinder(0.006f, 0.12f, 6);
        glPopMatrix();
    }

    // ==================== HELPER: Draw Combat Helmet ====================
    inline void drawCombatHelmet(float breathe) {
        glPushMatrix();
        glTranslatef(0, breathe, 0);
        
        // Helmet shell (FAST/Ops-Core style)
        setColorMetallic(0.16f, 0.18f, 0.14f);
        
        // Main dome - use multiple boxes for rounded shape
        drawBox(0.18f, 0.14f, 0.2f);
        
        // Front brim cut
        glPushMatrix();
        glTranslatef(0, -0.02f, 0.08f);
        glRotatef(-10, 1, 0, 0);
        drawBox(0.17f, 0.06f, 0.06f);
        glPopMatrix();
        
        // Side rails (for accessories)
        setColorMetallic(0.1f, 0.1f, 0.1f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.095f, -0.02f, 0);
            drawBox(0.015f, 0.04f, 0.16f);
            glPopMatrix();
        }
        
        // NVG mount (front shroud)
        setColorMetallic(0.08f, 0.08f, 0.08f);
        glPushMatrix();
        glTranslatef(0, 0.06f, 0.1f);
        drawBox(0.08f, 0.03f, 0.03f);
        // Mount screws
        setColorMetallic(0.15f, 0.15f, 0.15f);
        for (int i = -1; i <= 1; i += 2) {
            glPushMatrix();
            glTranslatef(i * 0.025f, 0, 0.018f);
            drawCylinder(0.006f, 0.01f, 6);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Helmet padding visible from bottom
        setColor(0.2f, 0.18f, 0.15f);
        glPushMatrix();
        glTranslatef(0, -0.06f, 0);
        drawBox(0.14f, 0.02f, 0.16f);
        glPopMatrix();
        
        // Velcro patch area (top)
        setColor(0.14f, 0.16f, 0.12f);
        glPushMatrix();
        glTranslatef(0, 0.075f, 0);
        drawBox(0.1f, 0.01f, 0.1f);
        glPopMatrix();
        
        // Ear protection / headset
        setColor(0.08f, 0.08f, 0.08f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.11f, -0.03f, 0);
            // Ear cup
            drawBox(0.03f, 0.08f, 0.08f);
            // Headset boom mic (left side only)
            if (side == -1) {
                setColor(0.05f, 0.05f, 0.05f);
                glPushMatrix();
                glTranslatef(-0.02f, -0.02f, 0.06f);
                glRotatef(-30, 0, 1, 0);
                drawCylinder(0.008f, 0.08f, 6);
                // Mic head
                glTranslatef(0, 0, 0.08f);
                drawSphere(0.015f, 8);
                glPopMatrix();
            }
            glPopMatrix();
        }
        
        // Goggles (pushed up on helmet)
        setColor(0.06f, 0.06f, 0.06f);
        glPushMatrix();
        glTranslatef(0, 0.04f, 0.105f);
        // Frame
        drawBox(0.14f, 0.03f, 0.025f);
        // Lenses
        setColor(0.15f, 0.12f, 0.08f); // Tan/amber tint
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.04f, 0, 0.015f);
            drawBox(0.05f, 0.025f, 0.008f);
            glPopMatrix();
        }
        glPopMatrix();
        
        glPopMatrix();
    }

    // ==================== HELPER: Draw Tactical Arm ====================
    inline void drawTacticalArm(float armAngle, float recoil, bool isLeft, bool holdingWeapon) {
        float side = isLeft ? -1.0f : 1.0f;
        
        glPushMatrix();
        
        // Shoulder joint
        setColorMetallic(0.16f, 0.18f, 0.14f);
        drawSphere(0.055f, 10);
        
        // Upper arm (combat shirt)
        glPushMatrix();
        if (holdingWeapon) {
            // Arms reach forward to weapon position
            // Left arm (support hand) reaches further forward to foregrip
            // Right arm (trigger hand) is closer to body on pistol grip
            if (isLeft) {
                // Support arm - reaches forward and across to foregrip
                glRotatef(75 + armAngle * 8 - recoil * 2, 1, 0, 0);  // Forward pitch
                glRotatef(35, 0, 0, 1);   // Inward toward center
                glRotatef(-10, 0, 1, 0);  // Slight rotation
            } else {
                // Trigger arm - closer to body, gripping pistol grip
                glRotatef(65 + armAngle * 8 - recoil * 3, 1, 0, 0);  // Forward pitch
                glRotatef(-25, 0, 0, 1);  // Outward slightly
                glRotatef(5, 0, 1, 0);    // Slight rotation
            }
        } else {
            glRotatef(15, 1, 0, 0);
            glRotatef(side * 10, 0, 0, 1);
        }
        
        glTranslatef(0, -0.10f, 0);
        setColor(0.16f, 0.18f, 0.14f);
        drawBox(0.055f, 0.16f, 0.06f);
        
        // Elbow pad
        glPushMatrix();
        glTranslatef(0, -0.06f, 0);
        setColor(0.06f, 0.06f, 0.06f);
        drawBox(0.05f, 0.05f, 0.055f);
        glPopMatrix();
        
        // Forearm
        glTranslatef(0, -0.14f, 0);
        if (holdingWeapon) {
            // Bend elbow to reach weapon
            if (isLeft) {
                glRotatef(55, 1, 0, 0);  // Bend at elbow
                glRotatef(-15, 0, 0, 1); // Wrist alignment
            } else {
                glRotatef(60, 1, 0, 0);  // Bend at elbow
                glRotatef(10, 0, 0, 1);  // Wrist alignment
            }
        } else {
            glRotatef(20, 1, 0, 0);
        }
        
        setColor(0.16f, 0.18f, 0.14f);
        drawBox(0.05f, 0.14f, 0.055f);
        
        // Watch/GPS (left arm only)
        if (isLeft) {
            glPushMatrix();
            glTranslatef(0.03f, -0.03f, 0);
            setColor(0.05f, 0.05f, 0.05f);
            drawBox(0.02f, 0.035f, 0.04f);
            setColor(0.1f, 0.3f, 0.1f);
            setEmissive(0.05f, 0.15f, 0.05f);
            glTranslatef(0.012f, 0, 0);
            drawBox(0.004f, 0.02f, 0.025f);
            clearEmissive();
            glPopMatrix();
        }
        
        // Tactical glove / hand
        glTranslatef(0, -0.09f, 0);
        setColor(0.08f, 0.08f, 0.06f);
        drawBox(0.045f, 0.06f, 0.05f);
        
        // Fingers gripping
        setColor(0.07f, 0.07f, 0.05f);
        glPushMatrix();
        glTranslatef(0, -0.04f, 0);
        drawBox(0.035f, 0.035f, 0.04f);
        glPopMatrix();
        
        glPopMatrix();
        glPopMatrix();
    }

    // ==================== MAIN: Draw Ultimate Soldier ====================
    inline void drawPlayerWithWeapon(float rotY, float walkPhase, float armAimAngle, bool isRunning,
                                      float weaponRecoil, bool isFiring, bool weaponLightOn, float lightIntensity) {
        glPushMatrix();
        // rotY = camera yaw. Negate for left/right, add 180 for forward/backward.
        glRotatef(180.0f - rotY, 0, 1, 0);
        glScalef(PLAYER_SCALE, PLAYER_SCALE, PLAYER_SCALE);
        
        float walkAnim = sin(walkPhase) * (isRunning ? 30.0f : 18.0f);
        float bodyBob = fabs(sin(walkPhase * 2)) * 0.018f;
        float breathe = sin(getTime() * 1.5f) * 0.004f;
        float shootRecoil = weaponRecoil * 10.0f;
        
        // ===================== FEET / BOOTS =====================
        // Left boot
        glPushMatrix();
        glTranslatef(-0.1f, 0.09f, 0);
        drawMilitaryBoot(walkAnim);
        glPopMatrix();
        
        // Right boot
        glPushMatrix();
        glTranslatef(0.1f, 0.09f, 0);
        drawMilitaryBoot(-walkAnim);
        glPopMatrix();
        
        // ===================== LEGS =====================
        // Left leg
        glPushMatrix();
        glTranslatef(-0.08f, 0.45f, 0);
        drawTacticalLeg(walkAnim, true);
        glPopMatrix();
        
        // Right leg
        glPushMatrix();
        glTranslatef(0.08f, 0.45f, 0);
        drawTacticalLeg(-walkAnim, false);
        glPopMatrix();
        
        // ===================== BELT / WAIST =====================
        glPushMatrix();
        glTranslatef(0, 0.58f, 0);
        
        // Battle belt
        setColor(0.12f, 0.14f, 0.1f);
        drawBox(0.24f, 0.06f, 0.16f);
        
        // Belt buckle
        setColorMetallic(0.2f, 0.2f, 0.18f);
        glPushMatrix();
        glTranslatef(0, 0, 0.085f);
        drawBox(0.05f, 0.04f, 0.015f);
        glPopMatrix();
        
        // Holster (right side)
        setColor(0.08f, 0.08f, 0.06f);
        glPushMatrix();
        glTranslatef(0.14f, -0.06f, 0.04f);
        drawBox(0.04f, 0.14f, 0.06f);
        // Pistol grip visible
        setColor(0.05f, 0.05f, 0.05f);
        glTranslatef(0, 0.04f, 0.02f);
        drawBox(0.025f, 0.04f, 0.03f);
        glPopMatrix();
        
        // Drop leg platform
        setColor(0.1f, 0.12f, 0.08f);
        glPushMatrix();
        glTranslatef(0.14f, -0.02f, 0);
        drawBox(0.035f, 0.08f, 0.1f);
        glPopMatrix();
        
        // Dump pouch (left rear)
        setColor(0.13f, 0.15f, 0.11f);
        glPushMatrix();
        glTranslatef(-0.12f, 0, -0.08f);
        drawBox(0.08f, 0.08f, 0.06f);
        glPopMatrix();
        
        glPopMatrix();
        
        // ===================== TORSO =====================
        glPushMatrix();
        glTranslatef(0, 0.82f + bodyBob + breathe, 0);
        
        // Base torso (combat shirt) - doesn't tilt, stays upright
        setColor(0.18f, 0.2f, 0.16f);
        drawBox(0.26f, 0.32f, 0.16f);
        
        // Draw plate carrier on top
        drawPlateCarrier();
        
        // ===================== ARMS + WEAPON AS ONE UNIT =====================
        // This entire unit pivots based on where player is aiming
        glPushMatrix();
        // Pivot point at shoulder height
        glTranslatef(0, 0.10f, 0.06f);
        
        // armAimAngle is camera pitch. Positive = looking up, negative = looking down.
        // Positive X rotation tilts weapon forward (down in world).
        // So use positive aimPitch to aim down, negative to aim up.
        float aimPitch = armAimAngle + shootRecoil * 5.0f;
        glRotatef(aimPitch, 1, 0, 0);
        
        // === WEAPON FIRST (arms attach to it) ===
        glPushMatrix();
        glTranslatef(0.02f, 0, 0.20f); // Weapon centered, forward from chest
        glScalef(0.55f, 0.55f, 0.55f);
        
        // Draw shoulder connection pieces (visible from behind)
        setColor(0.16f, 0.18f, 0.14f);
        
        // LEFT ARM - from shoulder to foregrip
        glPushMatrix();
        glTranslatef(-0.28f, 0.08f, 0.25f); // Shoulder position relative to weapon
        
        // Upper arm pointing toward foregrip
        glRotatef(85, 1, 0, 0);   // Forward
        glRotatef(40, 0, 0, 1);   // Inward
        drawBox(0.08f, 0.22f, 0.08f);
        
        // Elbow
        glTranslatef(0, -0.20f, 0);
        glRotatef(60, 1, 0, 0);
        setColor(0.06f, 0.06f, 0.06f);
        drawBox(0.07f, 0.06f, 0.07f);
        
        // Forearm
        setColor(0.16f, 0.18f, 0.14f);
        glTranslatef(0, -0.04f, 0);
        drawBox(0.07f, 0.18f, 0.07f);
        
        // Gloved hand gripping foregrip
        glTranslatef(0, -0.12f, 0);
        setColor(0.08f, 0.08f, 0.06f);
        drawBox(0.06f, 0.08f, 0.07f);
        glPopMatrix();
        
        // RIGHT ARM - from shoulder to pistol grip  
        glPushMatrix();
        glTranslatef(0.22f, 0.08f, -0.08f); // Shoulder position relative to weapon
        
        // Upper arm pointing toward grip
        glRotatef(75, 1, 0, 0);   // Forward
        glRotatef(-30, 0, 0, 1);  // Outward slightly
        setColor(0.16f, 0.18f, 0.14f);
        drawBox(0.08f, 0.22f, 0.08f);
        
        // Elbow
        glTranslatef(0, -0.20f, 0);
        glRotatef(70, 1, 0, 0);
        setColor(0.06f, 0.06f, 0.06f);
        drawBox(0.07f, 0.06f, 0.07f);
        
        // Forearm
        setColor(0.16f, 0.18f, 0.14f);
        glTranslatef(0, -0.04f, 0);
        drawBox(0.07f, 0.18f, 0.07f);
        
        // Gloved hand on pistol grip
        glTranslatef(0, -0.12f, 0);
        setColor(0.08f, 0.08f, 0.06f);
        drawBox(0.06f, 0.08f, 0.06f);
        // Trigger finger
        glPushMatrix();
        glTranslatef(0, -0.02f, 0.04f);
        drawBox(0.02f, 0.04f, 0.03f);
        glPopMatrix();
        glPopMatrix();
        
        // ===== REDESIGNED SCI-FI ASSAULT RIFLE =====
        // Clean, stylized design with glowing accents
        
        // WEAPON LIGHT
        if (weaponLightOn) {
            float lightPos[] = {0.06f, 0.02f, 0.6f, 1.0f};
            float lightColor[] = {lightIntensity * 1.5f, lightIntensity * 1.4f, lightIntensity * 1.2f, 1.0f};
            float lightDir[] = {0, 0, 1};
            glLightfv(GL_LIGHT2, GL_POSITION, lightPos);
            glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor);
            glLightfv(GL_LIGHT2, GL_SPECULAR, lightColor);
            glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, lightDir);
            glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 30.0f);
            glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 8.0f);
            glEnable(GL_LIGHT2);
        }
        
        // === MAIN BODY - Sleek angular design ===
        // Core receiver - dark gunmetal
        setColorMetallic(0.12f, 0.12f, 0.14f);
        drawBox(0.055f, 0.10f, 0.35f);
        
        // Top rail housing - slightly lighter
        setColorMetallic(0.15f, 0.15f, 0.17f);
        glPushMatrix();
        glTranslatef(0, 0.07f, 0.02f);
        drawBox(0.048f, 0.04f, 0.32f);
        glPopMatrix();
        
        // Accent stripe (glowing cyan)
        setColor(0.2f, 0.8f, 1.0f);
        setEmissive(0.1f, 0.4f, 0.5f);
        glPushMatrix();
        glTranslatef(0, 0.052f, 0.02f);
        drawBox(0.056f, 0.008f, 0.30f);
        glPopMatrix();
        clearEmissive();
        
        // === BARREL SHROUD - Futuristic hexagonal ===
        setColorMetallic(0.10f, 0.10f, 0.12f);
        glPushMatrix();
        glTranslatef(0, 0.03f, 0.38f);
        drawBox(0.045f, 0.06f, 0.30f);
        
        // Cooling vents (angular cuts)
        setColorMetallic(0.06f, 0.06f, 0.08f);
        for (int i = 0; i < 3; i++) {
            glPushMatrix();
            glTranslatef(0.025f, 0.015f, -0.08f + i * 0.10f);
            drawBox(0.008f, 0.025f, 0.06f);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(-0.025f, 0.015f, -0.08f + i * 0.10f);
            drawBox(0.008f, 0.025f, 0.06f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // === BARREL ===
        setColorMetallic(0.08f, 0.08f, 0.10f);
        glPushMatrix();
        glTranslatef(0, 0.03f, 0.55f);
        drawCylinder(0.022f, 0.20f, 12);
        glPopMatrix();
        
        // Muzzle brake - angular sci-fi design
        setColorMetallic(0.14f, 0.14f, 0.16f);
        glPushMatrix();
        glTranslatef(0, 0.03f, 0.74f);
        drawBox(0.038f, 0.038f, 0.08f);
        // Muzzle opening
        setColorMetallic(0.04f, 0.04f, 0.04f);
        glTranslatef(0, 0, 0.042f);
        drawCylinder(0.018f, 0.02f, 8);
        glPopMatrix();
        
        // === OPTIC - Holographic sight ===
        setColorMetallic(0.08f, 0.08f, 0.10f);
        glPushMatrix();
        glTranslatef(0, 0.12f, 0.08f);
        // Sight housing
        drawBox(0.04f, 0.045f, 0.10f);
        // Window frame
        setColorMetallic(0.05f, 0.05f, 0.06f);
        glTranslatef(0, 0.005f, 0.052f);
        drawBox(0.035f, 0.035f, 0.008f);
        // Glass (tinted)
        setColor(0.15f, 0.18f, 0.22f);
        glTranslatef(0, 0, 0.005f);
        drawBox(0.028f, 0.028f, 0.003f);
        // Reticle (glowing red dot)
        setColor(1.0f, 0.15f, 0.1f);
        setEmissive(0.9f, 0.1f, 0.05f);
        drawSphere(0.006f, 6);
        clearEmissive();
        glPopMatrix();
        
        // === MAGAZINE - Angular polymer ===
        setColor(0.18f, 0.16f, 0.12f);
        glPushMatrix();
        glTranslatef(0, -0.12f, 0.05f);
        glRotatef(5, 1, 0, 0);
        drawBox(0.038f, 0.14f, 0.055f);
        // Mag texture lines
        setColor(0.14f, 0.12f, 0.09f);
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glTranslatef(0, -0.04f + i * 0.025f, 0.029f);
            drawBox(0.034f, 0.008f, 0.002f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // === PISTOL GRIP - Ergonomic ===
        setColor(0.10f, 0.10f, 0.08f);
        glPushMatrix();
        glTranslatef(0, -0.08f, -0.06f);
        glRotatef(18, 1, 0, 0);
        drawBox(0.035f, 0.10f, 0.04f);
        // Grip texture
        setColor(0.07f, 0.07f, 0.06f);
        glTranslatef(0.019f, 0, 0);
        drawBox(0.003f, 0.08f, 0.035f);
        glPopMatrix();
        
        // === STOCK - Minimalist skeleton ===
        setColorMetallic(0.11f, 0.11f, 0.13f);
        glPushMatrix();
        glTranslatef(0, 0.02f, -0.22f);
        // Stock tube
        drawCylinder(0.018f, 0.16f, 10);
        // Stock pad
        glTranslatef(0, 0, -0.16f);
        setColor(0.06f, 0.06f, 0.06f);
        drawBox(0.04f, 0.07f, 0.03f);
        glPopMatrix();
        
        // === FOREGRIP - Angled ===
        setColor(0.09f, 0.09f, 0.08f);
        glPushMatrix();
        glTranslatef(0, -0.04f, 0.30f);
        glRotatef(-20, 1, 0, 0);
        drawBox(0.028f, 0.07f, 0.035f);
        glPopMatrix();
        
        // === TACTICAL LIGHT (when on) ===
        if (weaponLightOn) {
            setColorMetallic(0.10f, 0.10f, 0.10f);
            glPushMatrix();
            glTranslatef(0.05f, 0.02f, 0.42f);
            drawCylinder(0.016f, 0.06f, 10);
            // Light lens (glowing)
            glTranslatef(0, 0, 0.062f);
            setColor(1.0f, 0.98f, 0.92f);
            setEmissive(0.95f * lightIntensity, 0.90f * lightIntensity, 0.75f * lightIntensity);
            drawSphere(0.018f, 10);
            clearEmissive();
            glPopMatrix();
        }
        
        // === MUZZLE FLASH ===
        if (isFiring) {
            glPushMatrix();
            glTranslatef(0, 0.03f, 0.85f);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDisable(GL_LIGHTING);
            
            // Bright core
            glColor4f(1.0f, 1.0f, 0.9f, 1.0f);
            drawSphere(0.05f, 8);
            
            // Inner flash
            glColor4f(1.0f, 0.85f, 0.4f, 0.85f);
            drawSphere(0.09f, 8);
            
            // Outer glow
            glColor4f(1.0f, 0.5f, 0.15f, 0.5f);
            drawSphere(0.14f, 8);
            
            // Dynamic spikes
            glColor4f(1.0f, 0.9f, 0.6f, 0.7f);
            for (int i = 0; i < 6; i++) {
                glPushMatrix();
                glRotatef(i * 60 + getTime() * 400, 0, 0, 1);
                glTranslatef(0.06f, 0, 0);
                drawBox(0.10f + sin(getTime() * 15 + i) * 0.03f, 0.018f, 0.018f);
                glPopMatrix();
            }
            
            glEnable(GL_LIGHTING);
            glDisable(GL_BLEND);
            glPopMatrix();
        }
        
        if (weaponLightOn) {
            glDisable(GL_LIGHT2);
        }
        
        glPopMatrix(); // End weapon
        
        glPopMatrix(); // End arm+weapon unit
        
        glPopMatrix(); // End torso
        
        // ===================== NECK =====================
        glPushMatrix();
        glTranslatef(0, 1.08f + bodyBob + breathe, 0);
        setColor(0.55f, 0.45f, 0.38f); // Skin tone
        drawCylinder(0.045f, 0.08f, 12);
        
        // Neck gaiter / shemagh
        setColor(0.15f, 0.17f, 0.13f);
        glTranslatef(0, 0, 0);
        drawCylinder(0.055f, 0.05f, 12);
        glPopMatrix();
        
        // ===================== HEAD =====================
        glPushMatrix();
        glTranslatef(0, 1.22f + bodyBob + breathe, 0);
        
        // Face (visible under helmet)
        setColor(0.6f, 0.5f, 0.42f);
        glPushMatrix();
        glTranslatef(0, -0.03f, 0.06f);
        drawBox(0.1f, 0.1f, 0.06f);
        glPopMatrix();
        
        // Balaclava / face covering
        setColor(0.08f, 0.08f, 0.08f);
        glPushMatrix();
        glTranslatef(0, -0.06f, 0.04f);
        drawBox(0.11f, 0.06f, 0.08f);
        glPopMatrix();
        
        // Eye pro / glasses
        setColor(0.02f, 0.02f, 0.02f);
        glPushMatrix();
        glTranslatef(0, 0.0f, 0.1f);
        drawBox(0.11f, 0.025f, 0.015f);
        // Lenses (dark)
        setColor(0.05f, 0.05f, 0.08f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.035f, 0, 0.01f);
            drawBox(0.035f, 0.022f, 0.005f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Helmet
        drawCombatHelmet(breathe);
        
        glPopMatrix(); // End head
        
        glPopMatrix(); // End main transform
    }

    // Legacy function for compatibility
    inline void drawPlayerDetailed(float rotY, float walkPhase, float armAimAngle, bool isRunning,
                                    float weaponRecoil = 0.0f, bool isFiring = false, bool weaponLightOn = true) {
        drawPlayerWithWeapon(rotY, walkPhase, armAimAngle, isRunning, weaponRecoil, isFiring, weaponLightOn, 1.0f);
    }

    // First person arms - STABLE, only move with recoil/bob, not mouse
    inline void drawPlayerArms(float recoil, float bob) {
        glPushMatrix();
        
        // Very subtle movement - arms should feel attached to camera
        float bobY = sin(bob * 2) * 0.008f;
        float bobX = cos(bob) * 0.004f;
        glTranslatef(bobX, bobY, 0);
        
        // Left arm (support hand on foregrip)
        glPushMatrix();
        glTranslatef(-0.14f, -0.08f - recoil * 0.008f, 0.28f);
        glRotatef(-6, 0, 0, 1);
        glRotatef(45 + recoil * 3, 1, 0, 0);
        
        // Upper arm
        setColor(0.16f, 0.18f, 0.14f);
        drawBox(0.04f, 0.12f, 0.045f);
        
        // Forearm
        glTranslatef(0, -0.09f, 0.02f);
        glRotatef(50, 1, 0, 0);
        drawBox(0.038f, 0.10f, 0.042f);
        
        // Gloved hand
        setColor(0.08f, 0.08f, 0.06f);
        glTranslatef(0, -0.07f, 0);
        drawBox(0.035f, 0.05f, 0.038f);
        glPopMatrix();
        
        // Right arm (trigger hand on grip)
        glPushMatrix();
        glTranslatef(0.08f, -0.10f - recoil * 0.01f, 0.10f);
        glRotatef(6, 0, 0, 1);
        glRotatef(50 + recoil * 5, 1, 0, 0);
        
        // Upper arm
        setColor(0.16f, 0.18f, 0.14f);
        drawBox(0.04f, 0.12f, 0.045f);
        
        // Forearm
        glTranslatef(0, -0.09f, 0.015f);
        glRotatef(45, 1, 0, 0);
        drawBox(0.038f, 0.10f, 0.042f);
        
        // Gloved hand
        setColor(0.08f, 0.08f, 0.06f);
        glTranslatef(0, -0.07f, 0);
        drawBox(0.035f, 0.05f, 0.038f);
        glPopMatrix();
        
        glPopMatrix();
    }
    
    // ==================== FIRST PERSON PARKOUR VAULT ARMS ====================
    // Vector-style vault animation - HUGE VISIBLE ARMS reaching forward
    inline void drawParkourArmsFirstPerson(float parkourProgress) {
        // Make arms EXTREMELY visible - large scale, centered in view
        glPushMatrix();
        
        // SCALE UP everything so it's very visible
        glScalef(1.5f, 1.5f, 1.5f);
        
        // Animation phases:
        // 0.0-0.25: Reach forward (arms extending into view)
        // 0.25-0.5: Plant hands (hands slam on obstacle)
        // 0.5-0.75: Push/slide over (arms pushing, body moving)
        // 0.75-1.0: Release and land (arms pull back)
        
        float reachPhase = 0.0f;
        float plantPhase = 0.0f;
        float pushPhase = 0.0f;
        float releasePhase = 0.0f;
        
        if (parkourProgress < 0.25f) {
            reachPhase = parkourProgress / 0.25f;
        } else if (parkourProgress < 0.5f) {
            reachPhase = 1.0f;
            plantPhase = (parkourProgress - 0.25f) / 0.25f;
        } else if (parkourProgress < 0.75f) {
            plantPhase = 1.0f - (parkourProgress - 0.5f) / 0.25f;
            pushPhase = (parkourProgress - 0.5f) / 0.25f;
        } else {
            releasePhase = (parkourProgress - 0.75f) / 0.25f;
        }
        
        // === OBSTACLE BAR (the thing we're vaulting over) ===
        // Draw a HUGE prominent bar in the lower center of view
        if (parkourProgress > 0.08f && parkourProgress < 0.92f) {
            glPushMatrix();
            // Bar position - lower center, comes up during plant, goes down during push
            float barY = -0.35f + plantPhase * 0.15f - pushPhase * 0.25f;
            float barZ = -0.6f + reachPhase * 0.15f - pushPhase * 0.15f;
            glTranslatef(0, barY, barZ);
            
            // MAIN BAR - Dark metallic, VERY visible
            setColorMetallic(0.3f, 0.32f, 0.35f);
            drawBox(1.0f, 0.12f, 0.25f);
            
            // Rail highlight on top - BRIGHT
            setColorMetallic(0.5f, 0.52f, 0.55f);
            glTranslatef(0, 0.07f, 0);
            drawBox(0.95f, 0.04f, 0.2f);
            
            // Shiny edge highlight
            setEmissive(0.15f, 0.15f, 0.2f);
            glTranslatef(0, 0.025f, 0);
            drawBox(0.9f, 0.01f, 0.15f);
            clearEmissive();
            glPopMatrix();
        }
        
        // === LEFT ARM - HUGE and visible ===
        glPushMatrix();
        {
            // Start from left side of screen, reach toward center-forward
            float armX = -0.3f + reachPhase * 0.12f; // Move toward center
            float armY = -0.15f - plantPhase * 0.12f + pushPhase * 0.08f - releasePhase * 0.08f;
            float armZ = -0.25f - reachPhase * 0.4f + pushPhase * 0.15f + releasePhase * 0.25f;
            
            glTranslatef(armX, armY, armZ);
            
            // Arm angle - reaches forward then pushes down
            float armPitch = 30.0f + reachPhase * 60.0f - pushPhase * 30.0f - releasePhase * 40.0f;
            float armRoll = 10.0f + reachPhase * 5.0f;
            
            glRotatef(armPitch, 1, 0, 0);
            glRotatef(armRoll, 0, 0, 1);
            
            // UPPER ARM - Tactical sleeve (olive drab)
            setColor(0.25f, 0.28f, 0.22f); // Brighter green for visibility
            drawBox(0.08f, 0.22f, 0.08f);
            
            // Elbow pad
            setColor(0.15f, 0.15f, 0.12f);
            glPushMatrix();
            glTranslatef(0, -0.1f, 0.045f);
            drawBox(0.06f, 0.08f, 0.03f);
            glPopMatrix();
            
            // FOREARM
            glPushMatrix();
            glTranslatef(0, -0.18f, 0.04f);
            float elbowBend = 40.0f + plantPhase * 30.0f - pushPhase * 50.0f;
            glRotatef(elbowBend, 1, 0, 0);
            
            setColor(0.25f, 0.28f, 0.22f);
            drawBox(0.07f, 0.18f, 0.07f);
            
            // GLOVED HAND - Black tactical glove
            glPushMatrix();
            glTranslatef(0, -0.14f, 0.03f);
            float handAngle = 20.0f + plantPhase * 40.0f - pushPhase * 20.0f;
            glRotatef(handAngle, 1, 0, 0);
            
            // Hand back
            setColor(0.12f, 0.12f, 0.1f);
            drawBox(0.065f, 0.08f, 0.04f);
            
            // Fingers - spread during reach, grip during plant
            float fingerSpread = reachPhase * 0.5f - plantPhase * 0.4f;
            setColor(0.1f, 0.1f, 0.08f);
            for (int i = 0; i < 4; i++) {
                glPushMatrix();
                float fingerX = -0.02f + i * 0.015f;
                glTranslatef(fingerX + fingerSpread * (i - 1.5f) * 0.01f, -0.07f, 0);
                glRotatef(-5 + plantPhase * 15, 1, 0, 0);
                drawBox(0.012f, 0.05f, 0.015f);
                // Fingertip
                glTranslatef(0, -0.03f, 0);
                drawBox(0.01f, 0.02f, 0.012f);
                glPopMatrix();
            }
            // Thumb
            glPushMatrix();
            glTranslatef(0.04f, -0.03f, 0.02f);
            glRotatef(-45 + plantPhase * 20, 0, 0, 1);
            drawBox(0.015f, 0.04f, 0.015f);
            glPopMatrix();
            
            glPopMatrix(); // Hand
            glPopMatrix(); // Forearm
        }
        glPopMatrix(); // Left arm
        
        // === RIGHT ARM - Mirror of left ===
        glPushMatrix();
        {
            float armX = 0.35f - reachPhase * 0.15f;
            float armY = -0.2f - plantPhase * 0.15f + pushPhase * 0.1f - releasePhase * 0.1f;
            float armZ = -0.3f - reachPhase * 0.5f + pushPhase * 0.2f + releasePhase * 0.3f;
            
            glTranslatef(armX, armY, armZ);
            
            float armPitch = 30.0f + reachPhase * 60.0f - pushPhase * 30.0f - releasePhase * 40.0f;
            float armRoll = -10.0f - reachPhase * 5.0f;
            
            glRotatef(armPitch, 1, 0, 0);
            glRotatef(armRoll, 0, 0, 1);
            
            // Upper arm
            setColor(0.25f, 0.28f, 0.22f);
            drawBox(0.08f, 0.22f, 0.08f);
            
            // Elbow pad
            setColor(0.15f, 0.15f, 0.12f);
            glPushMatrix();
            glTranslatef(0, -0.1f, 0.045f);
            drawBox(0.06f, 0.08f, 0.03f);
            glPopMatrix();
            
            // Forearm
            glPushMatrix();
            glTranslatef(0, -0.18f, 0.04f);
            float elbowBend = 40.0f + plantPhase * 30.0f - pushPhase * 50.0f;
            glRotatef(elbowBend, 1, 0, 0);
            
            setColor(0.25f, 0.28f, 0.22f);
            drawBox(0.07f, 0.18f, 0.07f);
            
            // Gloved hand
            glPushMatrix();
            glTranslatef(0, -0.14f, 0.03f);
            float handAngle = 20.0f + plantPhase * 40.0f - pushPhase * 20.0f;
            glRotatef(handAngle, 1, 0, 0);
            
            setColor(0.12f, 0.12f, 0.1f);
            drawBox(0.065f, 0.08f, 0.04f);
            
            // Fingers
            float fingerSpread = reachPhase * 0.5f - plantPhase * 0.4f;
            setColor(0.1f, 0.1f, 0.08f);
            for (int i = 0; i < 4; i++) {
                glPushMatrix();
                float fingerX = -0.02f + i * 0.015f;
                glTranslatef(fingerX - fingerSpread * (i - 1.5f) * 0.01f, -0.07f, 0);
                glRotatef(-5 + plantPhase * 15, 1, 0, 0);
                drawBox(0.012f, 0.05f, 0.015f);
                glTranslatef(0, -0.03f, 0);
                drawBox(0.01f, 0.02f, 0.012f);
                glPopMatrix();
            }
            // Thumb
            glPushMatrix();
            glTranslatef(-0.04f, -0.03f, 0.02f);
            glRotatef(45 - plantPhase * 20, 0, 0, 1);
            drawBox(0.015f, 0.04f, 0.015f);
            glPopMatrix();
            
            glPopMatrix(); // Hand
            glPopMatrix(); // Forearm
        }
        glPopMatrix(); // Right arm
        
        glPopMatrix();
    }
    
    // ==================== THIRD PERSON PARKOUR POSE ====================
    // Draw player in vault pose for third-person view
    inline void drawPlayerParkourPose(float rotY, float parkourProgress) {
        glPushMatrix();
        glRotatef(rotY, 0, 1, 0);
        glScalef(PLAYER_SCALE, PLAYER_SCALE, PLAYER_SCALE);
        
        // Animation phases
        float reachPhase = 0.0f, plantPhase = 0.0f, pushPhase = 0.0f, releasePhase = 0.0f;
        
        if (parkourProgress < 0.25f) {
            reachPhase = parkourProgress / 0.25f;
        } else if (parkourProgress < 0.5f) {
            reachPhase = 1.0f;
            plantPhase = (parkourProgress - 0.25f) / 0.25f;
        } else if (parkourProgress < 0.75f) {
            plantPhase = 1.0f - (parkourProgress - 0.5f) / 0.25f;
            pushPhase = (parkourProgress - 0.5f) / 0.25f;
        } else {
            releasePhase = (parkourProgress - 0.75f) / 0.25f;
        }
        
        // Body tilts forward during vault
        float bodyTilt = reachPhase * 30.0f + plantPhase * 20.0f - pushPhase * 10.0f - releasePhase * 40.0f;
        float bodyRise = plantPhase * 0.3f + pushPhase * 0.1f - releasePhase * 0.2f;
        
        glTranslatef(0, bodyRise, 0);
        glRotatef(bodyTilt, 1, 0, 0);
        
        // === TORSO (tilted forward) ===
        glPushMatrix();
        glTranslatef(0, 0.45f, 0);
        
        // Main torso
        setColor(0.22f, 0.24f, 0.2f);
        drawBox(0.28f, 0.35f, 0.18f);
        
        // Tactical vest
        setColorMetallic(0.18f, 0.2f, 0.16f);
        glPushMatrix();
        glTranslatef(0, 0.02f, 0.08f);
        drawBox(0.26f, 0.28f, 0.06f);
        glPopMatrix();
        
        glPopMatrix();
        
        // === HEAD ===
        glPushMatrix();
        glTranslatef(0, 0.72f, 0);
        // Head looks down during vault
        glRotatef(20.0f + plantPhase * 20.0f - releasePhase * 30.0f, 1, 0, 0);
        
        setColor(0.75f, 0.6f, 0.5f);
        drawBox(0.14f, 0.16f, 0.14f);
        
        // Helmet
        setColor(0.2f, 0.22f, 0.18f);
        glTranslatef(0, 0.06f, 0);
        drawBox(0.16f, 0.1f, 0.16f);
        glPopMatrix();
        
        // === ARMS - Extended forward during vault ===
        // Left arm
        glPushMatrix();
        {
            glTranslatef(-0.2f, 0.55f, 0);
            
            // Arm reaches forward
            float shoulderPitch = 90.0f + reachPhase * 40.0f - pushPhase * 60.0f - releasePhase * 30.0f;
            glRotatef(shoulderPitch, 1, 0, 0);
            glRotatef(-15, 0, 0, 1);
            
            // Upper arm
            setColor(0.22f, 0.24f, 0.2f);
            drawBox(0.08f, 0.22f, 0.08f);
            
            // Forearm
            glTranslatef(0, -0.2f, 0);
            float elbowAngle = 30.0f + plantPhase * 40.0f - pushPhase * 50.0f;
            glRotatef(elbowAngle, 1, 0, 0);
            drawBox(0.07f, 0.2f, 0.07f);
            
            // Hand
            setColor(0.12f, 0.12f, 0.1f);
            glTranslatef(0, -0.15f, 0);
            drawBox(0.06f, 0.08f, 0.05f);
        }
        glPopMatrix();
        
        // Right arm (mirror)
        glPushMatrix();
        {
            glTranslatef(0.2f, 0.55f, 0);
            
            float shoulderPitch = 90.0f + reachPhase * 40.0f - pushPhase * 60.0f - releasePhase * 30.0f;
            glRotatef(shoulderPitch, 1, 0, 0);
            glRotatef(15, 0, 0, 1);
            
            setColor(0.22f, 0.24f, 0.2f);
            drawBox(0.08f, 0.22f, 0.08f);
            
            glTranslatef(0, -0.2f, 0);
            float elbowAngle = 30.0f + plantPhase * 40.0f - pushPhase * 50.0f;
            glRotatef(elbowAngle, 1, 0, 0);
            drawBox(0.07f, 0.2f, 0.07f);
            
            setColor(0.12f, 0.12f, 0.1f);
            glTranslatef(0, -0.15f, 0);
            drawBox(0.06f, 0.08f, 0.05f);
        }
        glPopMatrix();
        
        // === LEGS - Tucked/streaming behind during vault ===
        // Left leg
        glPushMatrix();
        {
            glTranslatef(-0.1f, 0.15f, 0);
            
            // Leg streams behind during vault
            float hipAngle = -30.0f + reachPhase * 50.0f + plantPhase * 30.0f - releasePhase * 60.0f;
            glRotatef(hipAngle, 1, 0, 0);
            
            // Thigh
            setColor(0.2f, 0.22f, 0.18f);
            drawBox(0.1f, 0.25f, 0.1f);
            
            // Lower leg
            glTranslatef(0, -0.22f, 0);
            float kneeAngle = 20.0f + plantPhase * 40.0f + pushPhase * 20.0f - releasePhase * 50.0f;
            glRotatef(kneeAngle, 1, 0, 0);
            drawBox(0.09f, 0.22f, 0.09f);
            
            // Boot
            setColor(0.08f, 0.06f, 0.05f);
            glTranslatef(0, -0.15f, 0.03f);
            drawBox(0.1f, 0.08f, 0.15f);
        }
        glPopMatrix();
        
        // Right leg
        glPushMatrix();
        {
            glTranslatef(0.1f, 0.15f, 0);
            
            float hipAngle = -30.0f + reachPhase * 40.0f + plantPhase * 40.0f - releasePhase * 50.0f;
            glRotatef(hipAngle, 1, 0, 0);
            
            setColor(0.2f, 0.22f, 0.18f);
            drawBox(0.1f, 0.25f, 0.1f);
            
            glTranslatef(0, -0.22f, 0);
            float kneeAngle = 30.0f + plantPhase * 30.0f + pushPhase * 30.0f - releasePhase * 40.0f;
            glRotatef(kneeAngle, 1, 0, 0);
            drawBox(0.09f, 0.22f, 0.09f);
            
            setColor(0.08f, 0.06f, 0.05f);
            glTranslatef(0, -0.15f, 0.03f);
            drawBox(0.1f, 0.08f, 0.15f);
        }
        glPopMatrix();
        
        glPopMatrix();
    }
}

#endif // PLAYER_MODEL_H
