// ============================================================================
// DOOMERS - EnemyModels.h
// Detailed enemy models - Zombies, Demons, Boss
// ============================================================================
#ifndef ENEMY_MODELS_H
#define ENEMY_MODELS_H

#include "ModelUtils.h"

namespace EnemyModels {

    using namespace ModelUtils;

    // Enemy scale factors (BIGGER!)
    const float ZOMBIE_SCALE = 1.8f;
    const float DEMON_SCALE = 2.5f;
    const float BOSS_SCALE = 4.0f;

    // ==================== ZOMBIE ====================
    inline void drawZombieDetailed(float rotY, float animPhase, float health, float maxHealth, float attackPhase = 0.0f) {
        glPushMatrix();
        glRotatef(rotY, 0, 1, 0);
        glScalef(ZOMBIE_SCALE, ZOMBIE_SCALE, ZOMBIE_SCALE);
        
        float damage = 1.0f - (health / maxHealth);
        float shamble = sin(animPhase) * 25.0f;
        float armSwing = sin(animPhase * 0.7f) * 35.0f;
        float headTilt = sin(animPhase * 0.3f) * 10.0f;
        float stumble = sin(animPhase * 1.3f) * 3.0f;
        
        // Attack animation - lunge forward
        float attackLunge = 0.0f;
        float attackArmRaise = 0.0f;
        if (attackPhase > 0.0f) {
            // attackPhase goes from ENEMY_ATTACK_COOLDOWN down to 0
            // We want the lunge to happen at the start
            float attackT = attackPhase / 1.5f; // Normalize assuming ~1.5 second cooldown
            attackT = 1.0f - attackT; // Invert so 0 = start of attack
            
            // Quick lunge forward then back
            if (attackT < 0.3f) {
                attackLunge = sin(attackT * 3.14159f / 0.3f) * 0.3f;
                attackArmRaise = sin(attackT * 3.14159f / 0.3f) * 60.0f;
            }
        }
        
        // Apply shamble tilt and attack lunge
        glRotatef(stumble, 0, 0, 1);
        glTranslatef(0, 0, attackLunge);
        
        // === FEET ===
        setColor(0.18f, 0.15f, 0.12f);
        // Left foot
        glPushMatrix();
        glTranslatef(-0.1f, 0.05f, 0.02f);
        glRotatef(shamble * 0.3f, 1, 0, 0);
        drawBox(0.1f, 0.1f, 0.18f);
        // Missing toes on one foot
        setColor(0.4f, 0.2f, 0.15f);
        glTranslatef(0.03f, 0, 0.08f);
        drawBox(0.04f, 0.05f, 0.04f);
        glPopMatrix();
        
        // Right foot (more intact)
        glPushMatrix();
        glTranslatef(0.1f, 0.05f, 0.02f);
        glRotatef(-shamble * 0.3f, 1, 0, 0);
        setColor(0.18f, 0.15f, 0.12f);
        drawBox(0.1f, 0.1f, 0.18f);
        glPopMatrix();

        // === LEGS ===
        // Left leg (torn pants)
        glPushMatrix();
        glTranslatef(-0.1f, 0.45f, 0);
        glRotatef(shamble, 1, 0, 0);
        
        setColor(0.25f + damage * 0.15f, 0.3f, 0.2f);
        drawBox(0.14f, 0.55f, 0.14f);
        
        // Torn cloth hanging
        setColor(0.2f, 0.22f, 0.15f);
        glPushMatrix();
        glTranslatef(0.06f, -0.1f, 0.06f);
        glRotatef(sin(animPhase * 2) * 10, 1, 0, 0);
        drawBox(0.08f, 0.2f, 0.03f);
        glPopMatrix();
        
        // Exposed flesh on calf
        setColor(0.45f, 0.25f, 0.2f);
        glPushMatrix();
        glTranslatef(-0.05f, -0.15f, 0.05f);
        drawBox(0.06f, 0.15f, 0.04f);
        glPopMatrix();
        
        // Lower leg bone visible
        setColor(0.7f, 0.65f, 0.6f);
        glPushMatrix();
        glTranslatef(0, -0.35f, 0);
        drawBox(0.05f, 0.25f, 0.05f);
        glPopMatrix();
        glPopMatrix();
        
        // Right leg
        glPushMatrix();
        glTranslatef(0.1f, 0.45f, 0);
        glRotatef(-shamble, 1, 0, 0);
        setColor(0.25f, 0.3f, 0.2f);
        drawBox(0.14f, 0.55f, 0.14f);
        // Knee wound
        setColor(0.5f, 0.2f, 0.15f);
        glPushMatrix();
        glTranslatef(0, -0.1f, 0.06f);
        drawBox(0.08f, 0.1f, 0.04f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0, -0.35f, 0);
        setColor(0.4f, 0.45f, 0.35f);
        drawBox(0.12f, 0.3f, 0.12f);
        glPopMatrix();
        glPopMatrix();

        // === TORSO ===
        glPushMatrix();
        glTranslatef(0, 0.9f, 0);
        glRotatef(sin(animPhase * 0.5f) * 8, 0, 0, 1);
        glRotatef(15, 1, 0, 0); // Hunched forward
        
        // Torn shirt/skin
        setColor(0.35f, 0.38f, 0.3f);
        drawBox(0.38f, 0.5f, 0.22f);
        
        // Large chest wound
        setColor(0.5f, 0.18f, 0.12f);
        glPushMatrix();
        glTranslatef(0.08f, 0.08f, 0.1f);
        drawBox(0.15f, 0.18f, 0.06f);
        // Dripping
        setColor(0.4f, 0.1f, 0.08f);
        glTranslatef(0, -0.12f, 0.02f);
        drawBox(0.04f, 0.08f, 0.02f);
        glPopMatrix();
        
        // Exposed ribs
        setColor(0.65f, 0.6f, 0.55f);
        glPushMatrix();
        glTranslatef(-0.1f, 0, 0.1f);
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glTranslatef(0, -0.06f + i * 0.08f, 0);
            glRotatef(10, 0, 0, 1);
            drawBox(0.12f, 0.025f, 0.03f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Spine visible in back
        setColor(0.6f, 0.55f, 0.5f);
        glPushMatrix();
        glTranslatef(0, 0.05f, -0.1f);
        for (int i = 0; i < 5; i++) {
            glPushMatrix();
            glTranslatef(0, -0.12f + i * 0.06f, 0);
            drawBox(0.06f, 0.04f, 0.05f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Intestines hanging
        setColor(0.5f, 0.25f, 0.2f);
        glPushMatrix();
        glTranslatef(0, -0.22f, 0.08f);
        glRotatef(sin(animPhase) * 15, 1, 0, 0);
        drawBox(0.12f, 0.15f, 0.08f);
        setColor(0.45f, 0.2f, 0.18f);
        glTranslatef(0.03f, -0.1f, 0);
        drawBox(0.06f, 0.12f, 0.06f);
        glPopMatrix();
        
        glPopMatrix();

        // === ARMS ===
        // Left arm (reaching forward) - raises during attack
        glPushMatrix();
        glTranslatef(-0.28f, 0.95f, 0.05f);
        glRotatef(-75 + armSwing - attackArmRaise, 1, 0, 0);
        glRotatef(-20, 0, 0, 1);
        
        setColor(0.45f, 0.5f, 0.4f);
        drawBox(0.1f, 0.35f, 0.1f);
        
        // Forearm with bite wound
        glPushMatrix();
        glTranslatef(0, -0.35f, 0);
        drawBox(0.08f, 0.3f, 0.08f);
        setColor(0.5f, 0.2f, 0.15f);
        glTranslatef(0.03f, 0.05f, 0.03f);
        drawBox(0.04f, 0.12f, 0.04f);
        glPopMatrix();
        
        // Hand (claw-like)
        glPushMatrix();
        glTranslatef(0, -0.52f, 0);
        setColor(0.4f, 0.45f, 0.35f);
        drawBox(0.07f, 0.08f, 0.1f);
        // Fingers
        setColor(0.35f, 0.38f, 0.3f);
        for (int f = -1; f <= 1; f++) {
            glPushMatrix();
            glTranslatef(f * 0.025f, -0.06f, 0.03f);
            glRotatef(-20, 1, 0, 0);
            drawBox(0.02f, 0.08f, 0.02f);
            // Dirty nails
            setColor(0.25f, 0.2f, 0.15f);
            glTranslatef(0, -0.05f, 0.01f);
            drawBox(0.018f, 0.03f, 0.015f);
            setColor(0.35f, 0.38f, 0.3f);
            glPopMatrix();
        }
        glPopMatrix();
        glPopMatrix();
        
        // Right arm (hanging/dragging) - also raises during attack
        glPushMatrix();
        glTranslatef(0.28f, 0.95f, 0);
        glRotatef(-20 - armSwing * 0.5f - attackArmRaise * 0.8f, 1, 0, 0);
        glRotatef(15 + sin(animPhase * 0.8f) * 5, 0, 0, 1);
        
        setColor(0.45f, 0.5f, 0.4f);
        drawBox(0.1f, 0.35f, 0.1f);
        
        // Forearm (broken, bent wrong)
        glPushMatrix();
        glTranslatef(0, -0.3f, 0);
        glRotatef(25, 0, 0, 1);
        drawBox(0.08f, 0.3f, 0.08f);
        // Bone protruding
        setColor(0.7f, 0.65f, 0.6f);
        glTranslatef(0.03f, 0.08f, 0);
        glRotatef(30, 0, 0, 1);
        drawBox(0.02f, 0.08f, 0.02f);
        glPopMatrix();
        
        glPopMatrix();

        // === HEAD ===
        glPushMatrix();
        glTranslatef(0, 1.2f, 0.05f);
        glRotatef(headTilt, 0, 0, 1);
        glRotatef(sin(animPhase * 0.4f) * 8, 1, 0, 0);
        
        // Neck (partially exposed)
        setColor(0.4f, 0.45f, 0.35f);
        glPushMatrix();
        glTranslatef(0, -0.12f, 0);
        drawBox(0.1f, 0.1f, 0.1f);
        // Neck wound
        setColor(0.5f, 0.2f, 0.15f);
        glTranslatef(0.04f, 0, 0.04f);
        drawBox(0.04f, 0.08f, 0.03f);
        glPopMatrix();
        
        // Head (decomposed)
        setColor(0.45f, 0.5f, 0.4f);
        drawBox(0.22f, 0.26f, 0.22f);
        
        // Scalp damage/missing hair
        setColor(0.5f, 0.25f, 0.2f);
        glPushMatrix();
        glTranslatef(0.06f, 0.1f, 0.02f);
        drawBox(0.1f, 0.08f, 0.15f);
        // Skull showing
        setColor(0.7f, 0.68f, 0.65f);
        glTranslatef(0, 0.02f, 0);
        drawBox(0.08f, 0.05f, 0.12f);
        glPopMatrix();
        
        // Sunken eyes with glow
        for (int i = -1; i <= 1; i += 2) {
            glPushMatrix();
            glTranslatef(i * 0.06f, 0.03f, 0.1f);
            // Eye socket
            setColor(0.2f, 0.15f, 0.12f);
            drawBox(0.045f, 0.04f, 0.03f);
            // Glowing eye
            setColor(0.9f, 0.2f, 0.1f);
            setEmissive(0.6f + damage * 0.3f, 0.1f, 0.05f);
            glTranslatef(0, 0, 0.02f);
            drawSphere(0.025f, 8);
            clearEmissive();
            glPopMatrix();
        }
        
        // Exposed jaw/teeth
        setColor(0.4f, 0.35f, 0.3f);
        glPushMatrix();
        glTranslatef(0, -0.08f, 0.08f);
        drawBox(0.14f, 0.08f, 0.08f);
        // Teeth
        setColor(0.7f, 0.65f, 0.5f);
        glTranslatef(0, -0.02f, 0.03f);
        for (int t = -2; t <= 2; t++) {
            if (t == 1) continue; // Missing tooth
            glPushMatrix();
            glTranslatef(t * 0.025f, 0, 0);
            drawBox(0.02f, 0.04f, 0.015f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Ear (half missing)
        setColor(0.42f, 0.47f, 0.38f);
        glPushMatrix();
        glTranslatef(-0.12f, 0.02f, 0);
        drawBox(0.03f, 0.05f, 0.04f);
        glPopMatrix();
        
        glPopMatrix();
        
        glPopMatrix();
    }

    // ==================== DEMON ====================
    inline void drawDemonDetailed(float rotY, float animPhase, float attackState) {
        glPushMatrix();
        glRotatef(rotY, 0, 1, 0);
        glScalef(DEMON_SCALE, DEMON_SCALE, DEMON_SCALE);
        
        float breathe = sin(getTime() * 3) * 0.03f;
        float wingFlap = sin(animPhase * 2) * 20.0f;
        float clawExtend = attackState > 0 ? attackState : 0;
        
        // === HOOVES ===
        setColor(0.15f, 0.08f, 0.05f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.15f, 0.08f, 0);
            // Hoof
            drawBox(0.12f, 0.16f, 0.15f);
            // Claws on hoof
            setColor(0.1f, 0.05f, 0.03f);
            for (int c = -1; c <= 1; c += 2) {
                glPushMatrix();
                glTranslatef(c * 0.04f, -0.06f, 0.06f);
                glRotatef(-20, 1, 0, 0);
                drawBox(0.025f, 0.06f, 0.025f);
                glPopMatrix();
            }
            setColor(0.15f, 0.08f, 0.05f);
            glPopMatrix();
        }

        // === LEGS (Digitigrade) ===
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.15f, 0.45f, -0.05f);
            glRotatef(sin(animPhase + side) * 15, 1, 0, 0);
            
            // Upper leg (thick)
            setColor(0.5f, 0.15f, 0.1f);
            drawBox(0.16f, 0.4f, 0.18f);
            
            // Knee spikes
            setColor(0.3f, 0.08f, 0.05f);
            glPushMatrix();
            glTranslatef(side * 0.06f, -0.1f, 0.08f);
            glRotatef(side * -15, 0, 0, 1);
            drawCone(0.04f, 0.12f, 6);
            glPopMatrix();
            
            // Lower leg
            glPushMatrix();
            glTranslatef(0, -0.35f, 0.08f);
            glRotatef(-30, 1, 0, 0);
            setColor(0.5f, 0.15f, 0.1f);
            drawBox(0.12f, 0.35f, 0.14f);
            glPopMatrix();
            
            glPopMatrix();
        }

        // === TORSO ===
        glPushMatrix();
        glTranslatef(0, 1.0f + breathe, 0);
        
        // Main body
        setColor(0.55f, 0.18f, 0.12f);
        drawBox(0.5f, 0.6f, 0.35f);
        
        // Chest muscles
        setColor(0.5f, 0.15f, 0.1f);
        glPushMatrix();
        glTranslatef(0, 0.1f, 0.16f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.1f, 0, 0);
            drawBox(0.15f, 0.2f, 0.06f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Abs
        setColor(0.48f, 0.14f, 0.1f);
        glPushMatrix();
        glTranslatef(0, -0.12f, 0.16f);
        for (int row = 0; row < 3; row++) {
            for (int col = -1; col <= 0; col++) {
                glPushMatrix();
                glTranslatef(col * 0.1f + 0.05f, -row * 0.08f, 0);
                drawBox(0.08f, 0.06f, 0.04f);
                glPopMatrix();
            }
        }
        glPopMatrix();
        
        // Spine ridges (back)
        setColor(0.35f, 0.1f, 0.08f);
        glPushMatrix();
        glTranslatef(0, 0.1f, -0.18f);
        for (int i = 0; i < 6; i++) {
            glPushMatrix();
            glTranslatef(0, -i * 0.08f, 0);
            drawCone(0.03f, 0.08f + i * 0.01f, 6);
            glPopMatrix();
        }
        glPopMatrix();
        
        glPopMatrix();

        // === ARMS ===
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.38f, 1.1f + breathe, 0);
            
            float armRaise = attackState > 0 ? -60 * attackState : 0;
            glRotatef(-30 + sin(animPhase) * 10 + armRaise, 1, 0, 0);
            glRotatef(side * 25, 0, 0, 1);
            
            // Upper arm
            setColor(0.55f, 0.18f, 0.12f);
            drawBox(0.12f, 0.38f, 0.14f);
            
            // Forearm
            glPushMatrix();
            glTranslatef(0, -0.4f, 0);
            drawBox(0.1f, 0.35f, 0.12f);
            
            // Hand
            glPushMatrix();
            glTranslatef(0, -0.25f, 0);
            drawBox(0.1f, 0.12f, 0.1f);
            
            // Claws
            setColor(0.2f, 0.08f, 0.05f);
            for (int f = -1; f <= 1; f++) {
                glPushMatrix();
                glTranslatef(f * 0.03f, -0.1f - clawExtend * 0.1f, 0.02f);
                glRotatef(-30 - clawExtend * 30, 1, 0, 0);
                drawBox(0.02f, 0.12f + clawExtend * 0.05f, 0.02f);
                // Claw tip
                glTranslatef(0, -0.08f, 0);
                drawCone(0.015f, 0.06f, 6);
                glPopMatrix();
            }
            glPopMatrix();
            glPopMatrix();
            
            glPopMatrix();
        }

        // === WINGS (vestigial) ===
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.25f, 1.25f + breathe, -0.15f);
            glRotatef(side * (30 + wingFlap), 0, 0, 1);
            glRotatef(-20, 1, 0, 0);
            
            // Wing arm
            setColor(0.4f, 0.12f, 0.08f);
            drawBox(0.04f, 0.04f, 0.4f);
            
            // Wing membrane
            setColor(0.35f, 0.1f, 0.08f, 0.8f);
            enableTransparency();
            glPushMatrix();
            glTranslatef(side * 0.1f, -0.15f, -0.15f);
            drawBox(0.01f, 0.25f, 0.3f);
            glPopMatrix();
            disableTransparency();
            
            // Wing claws
            setColor(0.2f, 0.06f, 0.04f);
            glPushMatrix();
            glTranslatef(0, 0, -0.22f);
            drawCone(0.02f, 0.06f, 6);
            glPopMatrix();
            
            glPopMatrix();
        }

        // === HEAD ===
        glPushMatrix();
        glTranslatef(0, 1.5f + breathe, 0.05f);
        
        // Neck
        setColor(0.5f, 0.15f, 0.1f);
        glPushMatrix();
        glTranslatef(0, -0.15f, 0);
        drawBox(0.16f, 0.15f, 0.14f);
        glPopMatrix();
        
        // Head
        setColor(0.55f, 0.18f, 0.12f);
        drawBox(0.25f, 0.28f, 0.26f);
        
        // Brow ridge
        setColor(0.45f, 0.12f, 0.08f);
        glPushMatrix();
        glTranslatef(0, 0.08f, 0.1f);
        drawBox(0.24f, 0.06f, 0.1f);
        glPopMatrix();
        
        // HORNS
        setColor(0.25f, 0.1f, 0.08f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.12f, 0.15f, 0);
            glRotatef(side * 25, 0, 0, 1);
            glRotatef(-15, 1, 0, 0);
            
            // Horn base
            drawCylinder(0.045f, 0.15f, 8);
            
            // Horn mid
            glTranslatef(0, 0.15f, 0);
            glRotatef(side * 15, 0, 0, 1);
            drawCylinder(0.035f, 0.15f, 8);
            
            // Horn tip
            glTranslatef(0, 0.15f, 0);
            glRotatef(side * 10, 0, 0, 1);
            drawCone(0.03f, 0.12f, 8);
            
            glPopMatrix();
        }
        
        // Glowing eyes
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.07f, 0.02f, 0.12f);
            setColor(1.0f, 0.8f, 0.1f);
            setEmissive(0.8f, 0.6f, 0.0f);
            drawSphere(0.04f, 10);
            // Slit pupil
            setColor(0.1f, 0.0f, 0.0f);
            clearEmissive();
            glTranslatef(0, 0, 0.025f);
            drawBox(0.008f, 0.05f, 0.01f);
            glPopMatrix();
        }
        
        // Snout
        setColor(0.5f, 0.15f, 0.1f);
        glPushMatrix();
        glTranslatef(0, -0.06f, 0.12f);
        drawBox(0.14f, 0.1f, 0.12f);
        
        // Nostrils
        setColor(0.2f, 0.05f, 0.03f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.04f, 0.02f, 0.06f);
            drawSphere(0.025f, 6);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Mouth/Jaw
        setColor(0.35f, 0.08f, 0.05f);
        glPushMatrix();
        glTranslatef(0, -0.12f, 0.1f);
        float jawOpen = attackState > 0 ? attackState * 20 : sin(getTime() * 2) * 5;
        glRotatef(jawOpen, 1, 0, 0);
        drawBox(0.15f, 0.06f, 0.1f);
        
        // Teeth
        setColor(0.9f, 0.85f, 0.7f);
        glPushMatrix();
        glTranslatef(0, 0.03f, 0.04f);
        // Upper fangs
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.05f, 0, 0);
            drawBox(0.02f, 0.06f, 0.02f);
            glTranslatef(0, -0.04f, 0);
            drawCone(0.015f, 0.04f, 6);
            glPopMatrix();
        }
        glPopMatrix();
        glPopMatrix();
        
        glPopMatrix();
        
        glPopMatrix();
    }

    // ==================== DOOM CYBERDEMON BOSS ====================
    // Classic DOOM-style demon: Massive, muscular, with cybernetic enhancements
    // Features: Goat legs with hooves, rocket launcher arm, massive horns, exposed muscles
    inline void drawBossDetailed(float rotY, float animPhase, float health, float maxHealth) {
        glPushMatrix();
        glRotatef(rotY, 0, 1, 0);
        
        float rage = 1.0f - (health / maxHealth);
        float time = getTime();
        float pulse = sin(time * 3.0f) * 0.15f + 0.85f;
        float breathe = sin(time * 1.5f) * 0.04f;
        float walkBob = sin(animPhase * 2.0f) * 0.05f;
        
        // Scale for MASSIVE boss - towering over player
        glScalef(2.2f, 2.2f, 2.2f);
        glTranslatef(0, walkBob, 0);
        
        // =====================================================
        // GOAT LEGS WITH HOOVES - Classic demon legs
        // =====================================================
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.25f, 0, 0);
            
            float legSwing = sin(animPhase + (side > 0 ? 0 : 3.14159f)) * 20.0f;
            glRotatef(legSwing, 1, 0, 0);
            
            // HOOVES - Cloven demon hooves
            setColor(0.15f, 0.08f, 0.05f);
            glPushMatrix();
            glTranslatef(0, 0.08f, 0.05f);
            // Hoof shape - two parts
            for (int h = -1; h <= 1; h += 2) {
                glPushMatrix();
                glTranslatef(h * 0.04f, 0, 0);
                drawBox(0.06f, 0.1f, 0.12f);
                glPopMatrix();
            }
            glPopMatrix();
            
            // Lower leg - backwards bent like goat
            glPushMatrix();
            glTranslatef(0, 0.35f, -0.1f);
            glRotatef(30, 1, 0, 0);
            setColor(0.55f, 0.25f, 0.2f); // Reddish demon flesh
            drawBox(0.1f, 0.4f, 0.1f);
            
            // Exposed muscle/tendons
            setColor(0.7f, 0.2f, 0.15f);
            glPushMatrix();
            glTranslatef(-0.06f, 0, 0.04f);
            drawBox(0.03f, 0.35f, 0.03f);
            glPopMatrix();
            glPopMatrix();
            
            // Knee joint
            setColor(0.5f, 0.2f, 0.18f);
            glPushMatrix();
            glTranslatef(0, 0.55f, -0.15f);
            drawSphere(0.12f, 8);
            glPopMatrix();
            
            // Upper leg - thick and muscular
            glPushMatrix();
            glTranslatef(0, 0.85f, 0);
            glRotatef(-15, 1, 0, 0);
            setColor(0.6f, 0.28f, 0.22f);
            drawBox(0.15f, 0.4f, 0.14f);
            
            // Muscle definition
            setColor(0.65f, 0.3f, 0.25f);
            glPushMatrix();
            glTranslatef(side * 0.08f, 0.1f, 0.06f);
            drawSphere(0.08f, 6);
            glPopMatrix();
            glPopMatrix();
            
            glPopMatrix();
        }
        
        // =====================================================
        // MASSIVE MUSCULAR TORSO
        // =====================================================
        glPushMatrix();
        glTranslatef(0, 1.4f + breathe, 0);
        
        // Lower torso / abs
        setColor(0.6f, 0.25f, 0.2f);
        glPushMatrix();
        glTranslatef(0, -0.2f, 0);
        drawBox(0.4f, 0.35f, 0.3f);
        
        // Ab muscles
        setColor(0.65f, 0.3f, 0.22f);
        for (int row = 0; row < 3; row++) {
            for (int col = -1; col <= 1; col += 2) {
                glPushMatrix();
                glTranslatef(col * 0.1f, -0.1f + row * 0.1f, 0.16f);
                drawBox(0.08f, 0.08f, 0.04f);
                glPopMatrix();
            }
        }
        glPopMatrix();
        
        // Main chest - HUGE
        setColor(0.58f, 0.25f, 0.2f);
        drawBox(0.5f, 0.45f, 0.35f);
        
        // Pectoral muscles
        setColor(0.65f, 0.3f, 0.22f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.18f, 0.15f, 0.2f);
            glScalef(1.2f, 0.8f, 0.6f);
            drawSphere(0.15f, 10);
            glPopMatrix();
        }
        
        // CYBERNETIC IMPLANTS on chest
        setColorMetallic(0.3f, 0.3f, 0.35f);
        glPushMatrix();
        glTranslatef(0, 0, 0.22f);
        drawBox(0.15f, 0.2f, 0.08f);
        
        // Glowing power core
        setColor(1.0f * pulse, 0.3f * pulse, 0.1f * pulse);
        setEmissive(0.8f * pulse + rage * 0.2f, 0.2f * pulse, 0.05f);
        glTranslatef(0, 0, 0.05f);
        drawSphere(0.08f, 12);
        clearEmissive();
        glPopMatrix();
        
        // Spinal ridges on back
        setColor(0.5f, 0.2f, 0.18f);
        for (int s = 0; s < 5; s++) {
            glPushMatrix();
            glTranslatef(0, 0.2f - s * 0.1f, -0.2f);
            drawBox(0.06f, 0.04f, 0.1f);
            glPopMatrix();
        }
        
        glPopMatrix();
        
        // =====================================================
        // SHOULDERS - Massive with spikes
        // =====================================================
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.55f, 1.6f + breathe, 0);
            
            // Shoulder muscle
            setColor(0.6f, 0.28f, 0.22f);
            drawSphere(0.2f, 10);
            
            // Bone spikes from shoulder
            setColor(0.85f, 0.8f, 0.7f);
            for (int sp = 0; sp < 3; sp++) {
                glPushMatrix();
                glRotatef(sp * 25 - 25, 0, 0, 1);
                glRotatef(side * 60, 0, 0, 1);
                glTranslatef(side * 0.15f, 0.1f, 0);
                drawCone(0.04f, 0.2f + sp * 0.05f, 6);
                glPopMatrix();
            }
            glPopMatrix();
        }
        
        // =====================================================
        // LEFT ARM - Muscular demon arm with claws
        // =====================================================
        glPushMatrix();
        glTranslatef(-0.65f, 1.5f + breathe, 0);
        glRotatef(sin(animPhase * 0.8f) * 15.0f - 20, 1, 0, 0);
        glRotatef(-25, 0, 0, 1);
        
        // Upper arm
        setColor(0.6f, 0.28f, 0.22f);
        drawBox(0.14f, 0.35f, 0.12f);
        
        // Bicep bulge
        setColor(0.65f, 0.32f, 0.25f);
        glPushMatrix();
        glTranslatef(0.06f, 0.1f, 0);
        drawSphere(0.1f, 8);
        glPopMatrix();
        
        // Elbow
        glPushMatrix();
        glTranslatef(0, -0.25f, 0);
        setColor(0.55f, 0.25f, 0.2f);
        drawSphere(0.1f, 8);
        glPopMatrix();
        
        // Forearm
        glPushMatrix();
        glTranslatef(0, -0.5f, 0);
        glRotatef(-20, 1, 0, 0);
        setColor(0.58f, 0.26f, 0.2f);
        drawBox(0.12f, 0.3f, 0.1f);
        
        // MASSIVE CLAWED HAND
        glPushMatrix();
        glTranslatef(0, -0.28f, 0.02f);
        setColor(0.55f, 0.24f, 0.18f);
        drawBox(0.14f, 0.1f, 0.1f);
        
        // Deadly claws
        setColor(0.2f, 0.15f, 0.1f);
        for (int f = -2; f <= 2; f++) {
            glPushMatrix();
            glTranslatef(f * 0.03f, -0.08f, 0.04f);
            glRotatef(-40, 1, 0, 0);
            drawBox(0.02f, 0.06f, 0.02f);
            
            // Claw tip
            glTranslatef(0, -0.06f, 0);
            if (rage > 0.3f) {
                setColor(1.0f, 0.4f, 0.1f);
                setEmissive(0.5f * rage, 0.2f * rage, 0.05f);
            }
            drawCone(0.015f, 0.12f, 6);
            clearEmissive();
            glPopMatrix();
        }
        glPopMatrix();
        
        glPopMatrix();
        glPopMatrix();
        
        // =====================================================
        // RIGHT ARM - CYBERNETIC ROCKET LAUNCHER
        // =====================================================
        glPushMatrix();
        glTranslatef(0.65f, 1.5f + breathe, 0);
        float aimAngle = sin(animPhase * 0.5f) * 10.0f;
        glRotatef(aimAngle - 15, 1, 0, 0);
        glRotatef(25, 0, 0, 1);
        
        // Shoulder mount - mechanical
        setColorMetallic(0.35f, 0.35f, 0.4f);
        drawBox(0.18f, 0.15f, 0.15f);
        
        // Hydraulic joint
        setColorMetallic(0.4f, 0.4f, 0.45f);
        glPushMatrix();
        glTranslatef(0, -0.12f, 0);
        drawCylinder(0.08f, 0.15f, 12);
        glPopMatrix();
        
        // Main launcher arm
        glPushMatrix();
        glTranslatef(0, -0.4f, 0);
        
        // Arm housing
        setColorMetallic(0.3f, 0.3f, 0.35f);
        drawBox(0.14f, 0.35f, 0.12f);
        
        // THE ROCKET LAUNCHER
        glPushMatrix();
        glTranslatef(0, -0.3f, 0.08f);
        glRotatef(-90, 1, 0, 0);
        
        // Main barrel
        setColorMetallic(0.25f, 0.25f, 0.3f);
        drawCylinder(0.1f, 0.5f, 16);
        
        // Barrel opening - glowing when charging
        glPushMatrix();
        glTranslatef(0, 0, 0.5f);
        setColor(0.1f + rage * 0.9f, 0.1f + rage * 0.3f, 0.05f);
        if (rage > 0.2f) setEmissive(rage * 0.8f, rage * 0.3f, 0.1f);
        drawCylinder(0.08f, 0.05f, 12);
        clearEmissive();
        glPopMatrix();
        
        // Side details
        setColorMetallic(0.35f, 0.35f, 0.4f);
        for (int d = 0; d < 3; d++) {
            glPushMatrix();
            glTranslatef(0.1f, 0, 0.1f + d * 0.15f);
            drawBox(0.04f, 0.06f, 0.06f);
            glPopMatrix();
        }
        
        // Ammo feed
        setColorMetallic(0.4f, 0.35f, 0.3f);
        glPushMatrix();
        glTranslatef(-0.12f, 0, 0.2f);
        drawCylinder(0.05f, 0.25f, 8);
        glPopMatrix();
        
        glPopMatrix();
        glPopMatrix();
        glPopMatrix();
        
        // =====================================================
        // NECK - Thick and muscular
        // =====================================================
        glPushMatrix();
        glTranslatef(0, 1.85f + breathe, 0);
        setColor(0.55f, 0.25f, 0.2f);
        drawBox(0.18f, 0.15f, 0.15f);
        
        // Neck muscles
        setColor(0.6f, 0.28f, 0.22f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.12f, 0, 0.05f);
            drawBox(0.05f, 0.12f, 0.06f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // =====================================================
        // DEMONIC HEAD - Terrifying face with massive horns
        // =====================================================
        glPushMatrix();
        glTranslatef(0, 2.1f + breathe, 0.08f);
        
        // Main skull shape
        setColor(0.6f, 0.28f, 0.22f);
        glPushMatrix();
        glScalef(1.0f, 1.1f, 0.95f);
        drawSphere(0.28f, 16);
        glPopMatrix();
        
        // Brow ridge - heavy and menacing
        setColor(0.55f, 0.25f, 0.2f);
        glPushMatrix();
        glTranslatef(0, 0.12f, 0.18f);
        drawBox(0.25f, 0.06f, 0.1f);
        glPopMatrix();
        
        // MASSIVE HORNS - Curving back
        setColor(0.2f, 0.15f, 0.1f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.2f, 0.2f, -0.05f);
            glRotatef(side * -30, 0, 0, 1);
            glRotatef(-20, 1, 0, 0);
            
            // Horn base
            drawCylinder(0.08f, 0.15f, 8);
            
            // Horn middle
            glTranslatef(0, 0, 0.15f);
            glRotatef(side * 15, 0, 1, 0);
            drawCylinder(0.06f, 0.2f, 8);
            
            // Horn tip
            glTranslatef(0, 0, 0.2f);
            glRotatef(side * 10, 0, 1, 0);
            drawCone(0.05f, 0.25f, 8);
            
            glPopMatrix();
        }
        
        // BURNING EYES - Red hellfire
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.1f, 0.08f, 0.22f);
            
            // Eye socket
            setColor(0.15f, 0.05f, 0.05f);
            drawSphere(0.07f, 10);
            
            // Burning eye
            setColor(1.0f, 0.2f + rage * 0.3f, 0.05f);
            setEmissive(1.0f, 0.3f + rage * 0.4f, 0.1f);
            glTranslatef(0, 0, 0.03f);
            drawSphere(0.05f, 10);
            
            // Bright pupil
            setColor(1.0f, 0.8f + rage * 0.2f, 0.3f);
            setEmissive(1.0f, 0.9f, 0.5f);
            drawSphere(0.02f, 6);
            clearEmissive();
            
            glPopMatrix();
        }
        
        // Snout / muzzle
        setColor(0.55f, 0.25f, 0.2f);
        glPushMatrix();
        glTranslatef(0, -0.02f, 0.2f);
        drawBox(0.12f, 0.1f, 0.12f);
        
        // Nostrils - steaming
        setColor(0.2f, 0.1f, 0.08f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.04f, 0.02f, 0.08f);
            drawSphere(0.025f, 6);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Jaw with fangs
        setColor(0.52f, 0.24f, 0.18f);
        glPushMatrix();
        float jawOpen = 5.0f + sin(time * 2.0f) * 3.0f + rage * 10.0f;
        glTranslatef(0, -0.15f, 0.12f);
        glRotatef(jawOpen, 1, 0, 0);
        drawBox(0.15f, 0.08f, 0.12f);
        
        // MASSIVE FANGS
        setColor(0.9f, 0.85f, 0.75f);
        // Upper fangs
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.08f, 0.06f, 0.08f);
            drawBox(0.025f, 0.1f, 0.025f);
            glTranslatef(0, -0.08f, 0);
            drawCone(0.02f, 0.06f, 6);
            glPopMatrix();
        }
        // Row of teeth
        for (int t = -2; t <= 2; t++) {
            glPushMatrix();
            glTranslatef(t * 0.025f, 0.04f, 0.1f);
            drawBox(0.015f, 0.04f, 0.015f);
            glPopMatrix();
        }
        glPopMatrix();
        
        glPopMatrix(); // Head
        
        // =====================================================
        // RAGE AURA - Fire and brimstone when damaged
        // =====================================================
        if (rage > 0.25f) {
            enableGlow();
            
            // Inner fire aura
            glColor4f(1.0f, 0.3f + rage * 0.3f, 0.1f, rage * 0.3f);
            glPushMatrix();
            glTranslatef(0, 1.3f, 0);
            drawSphere(0.9f + sin(time * 5.0f) * 0.1f, 12);
            glPopMatrix();
            
            // Fire particles around body
            for (int p = 0; p < 12; p++) {
                float pAngle = p * 30.0f + time * 80.0f;
                float pRad = pAngle * 3.14159f / 180.0f;
                float pDist = 0.7f + sin(time * 3.0f + p) * 0.2f;
                float pY = 0.8f + sin(time * 4.0f + p * 0.7f) * 0.5f + p * 0.08f;
                
                glPushMatrix();
                glTranslatef(cos(pRad) * pDist, pY, sin(pRad) * pDist);
                glColor4f(1.0f, 0.5f - p * 0.03f, 0.1f, 0.6f - p * 0.04f);
                drawSphere(0.05f + rage * 0.03f, 6);
                glPopMatrix();
            }
            
            disableGlow();
        }
        
        glPopMatrix();
    }

    // Compatibility wrappers
    inline void drawZombie(float rotY, float animPhase = 0.0f) {
        drawZombieDetailed(rotY, animPhase, 100, 100);
    }

    inline void drawDemon(float rotY, float animPhase = 0.0f) {
        drawDemonDetailed(rotY, animPhase, 0);
    }
}

#endif // ENEMY_MODELS_H
