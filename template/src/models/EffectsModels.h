// ============================================================================
// DOOMERS - EffectsModels.h
// Visual effects: laser bullets, muzzle flash, particles, explosions
// ============================================================================
#ifndef EFFECTS_MODELS_H
#define EFFECTS_MODELS_H

#include "ModelUtils.h"
#include <vector>
#include <cmath>

namespace EffectsModels {

    using namespace ModelUtils;

    // ==================== BULLET TRACER - HIGHLY VISIBLE ====================
    inline void drawBulletTracer(const Vector3& start, const Vector3& end, float r, float g, float b, float alpha) {
        glPushMatrix();
        
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);  // Draw on top of everything
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow
        
        // Calculate direction and length for oriented particles
        Vector3 dir = end - start;
        float length = dir.length();
        
        // Super bright core line
        glLineWidth(6.0f);
        glBegin(GL_LINES);
        glColor4f(1.0f, 1.0f, 1.0f, alpha);  // White hot core
        glVertex3f(start.x, start.y, start.z);
        glVertex3f(end.x, end.y, end.z);
        glEnd();
        
        // Main tracer line - bright color
        glLineWidth(4.0f);
        glBegin(GL_LINES);
        glColor4f(r, g, b, alpha);
        glVertex3f(start.x, start.y, start.z);
        glVertex3f(end.x, end.y, end.z);
        glEnd();
        
        // Outer glow - thicker, dimmer
        glLineWidth(10.0f);
        glBegin(GL_LINES);
        glColor4f(r * 0.7f, g * 0.7f, b * 0.7f, alpha * 0.5f);
        glVertex3f(start.x, start.y, start.z);
        glVertex3f(end.x, end.y, end.z);
        glEnd();
        
        // Even outer glow
        glLineWidth(18.0f);
        glBegin(GL_LINES);
        glColor4f(r * 0.4f, g * 0.4f, b * 0.4f, alpha * 0.25f);
        glVertex3f(start.x, start.y, start.z);
        glVertex3f(end.x, end.y, end.z);
        glEnd();
        
        glLineWidth(1.0f);
        
        // Muzzle point glow
        glColor4f(1.0f, 0.9f, 0.5f, alpha);
        glPushMatrix();
        glTranslatef(start.x, start.y, start.z);
        glutSolidSphere(0.15f * alpha, 8, 8);
        glPopMatrix();
        
        // Impact point glow - bigger and brighter
        glColor4f(r, g, b, alpha * 0.9f);
        glPushMatrix();
        glTranslatef(end.x, end.y, end.z);
        glutSolidSphere(0.2f * alpha, 10, 10);
        // Impact flash
        glColor4f(1.0f, 0.8f, 0.3f, alpha * 0.6f);
        glutSolidSphere(0.35f * alpha, 8, 8);
        glPopMatrix();
        
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        glPopMatrix();
    }

    // ==================== LASER BULLET ====================
    inline void drawLaserBullet(float length = 2.5f) {
        float pulse = sin(getTime() * 25) * 0.15f + 0.85f;
        
        glPushMatrix();
        enableGlow();
        
        // Core beam - bright cyan
        glColor4f(0.4f, 1.0f, 1.0f, 1.0f);
        setEmissive(0.3f, 0.8f, 0.8f);
        glPushMatrix();
        glRotatef(90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), 0.04f, 0.04f, length, 8, 1);
        glPopMatrix();
        
        // Inner glow layer
        glColor4f(0.2f, 0.9f * pulse, 1.0f * pulse, 0.7f);
        glPushMatrix();
        glRotatef(90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), 0.08f, 0.08f, length, 8, 1);
        glPopMatrix();
        
        // Outer glow layer
        glColor4f(0.0f, 0.5f * pulse, 0.8f * pulse, 0.3f);
        glPushMatrix();
        glRotatef(90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), 0.15f, 0.15f, length, 8, 1);
        glPopMatrix();
        
        // Front tip glow
        glColor4f(0.7f, 1.0f, 1.0f, 0.9f);
        glPushMatrix();
        glTranslatef(0, 0, 0);
        drawSphere(0.1f * pulse, 8);
        glPopMatrix();
        
        // Trail particles
        glColor4f(0.2f, 0.7f, 1.0f, 0.4f);
        for (int i = 0; i < 5; i++) {
            glPushMatrix();
            float offset = i * 0.4f;
            float size = 0.05f + (float)i * 0.01f;
            glTranslatef(
                sin(getTime() * 20 + i) * 0.05f,
                -offset - length,
                cos(getTime() * 20 + i) * 0.05f
            );
            drawSphere(size, 6);
            glPopMatrix();
        }
        
        clearEmissive();
        disableGlow();
        glPopMatrix();
    }

    // ==================== MUZZLE FLASH - COMPACT AND VISIBLE ====================
    inline void drawMuzzleFlash(float intensity = 1.0f, float size = 1.0f) {
        if (intensity < 0.01f) return;
        
        // Reduce overall size for a more compact effect
        size *= 0.6f;
        
        glPushMatrix();
        enableGlow();
        
        // Central flash - smaller
        glColor4f(1.0f, 0.95f, 0.6f, intensity);
        drawSphere(0.1f * size, 8);
        
        // Fewer, smaller flash spikes
        int numSpikes = 6;
        for (int i = 0; i < numSpikes; i++) {
            float angle = (360.0f / numSpikes) * i;
            float spikeLen = (0.15f + (i % 2) * 0.08f) * size;
            
            glPushMatrix();
            glRotatef(angle, 0, 0, 1);
            
            // Inner spike
            glColor4f(1.0f, 0.9f, 0.5f, intensity * 0.9f);
            glBegin(GL_TRIANGLES);
            glVertex3f(-0.02f * size, 0, 0);
            glVertex3f(0.02f * size, 0, 0);
            glVertex3f(0, spikeLen, 0);
            glEnd();
            
            glPopMatrix();
        }
        
        // Forward flash cone - smaller
        glColor4f(1.0f, 0.85f, 0.4f, intensity * 0.6f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), 0.05f * size, 0.12f * size, 0.2f * size, 10, 1);
        glPopMatrix();
        
        // Smaller outer glow
        glColor4f(1.0f, 0.5f, 0.1f, intensity * 0.2f);
        drawSphere(0.2f * size, 10);
        
        disableGlow();
        glPopMatrix();
    }

    // ==================== EXPLOSION ====================
    inline void drawExplosion(float progress, float size = 1.0f) {
        // progress: 0.0 = start, 1.0 = end
        if (progress < 0 || progress > 1) return;
        
        float currentSize = size * (0.2f + progress * 2.5f);
        float alpha = 1.0f - progress * progress;
        
        glPushMatrix();
        enableGlow();
        
        // Core fireball
        float coreR = 1.0f;
        float coreG = 0.9f - progress * 0.5f;
        float coreB = 0.3f - progress * 0.3f;
        glColor4f(coreR, coreG, coreB, alpha);
        drawSphere(currentSize * 0.4f, 12);
        
        // Inner fire layer
        glColor4f(1.0f, 0.6f - progress * 0.3f, 0.1f, alpha * 0.7f);
        drawSphere(currentSize * 0.6f, 12);
        
        // Outer fire layer
        glColor4f(1.0f, 0.3f, 0.05f, alpha * 0.4f);
        drawSphere(currentSize * 0.85f, 12);
        
        // Smoke layer (appears later)
        if (progress > 0.3f) {
            float smokeAlpha = (progress - 0.3f) * 0.6f;
            glColor4f(0.3f, 0.3f, 0.3f, smokeAlpha);
            drawSphere(currentSize * 1.1f, 10);
        }
        
        // Debris particles
        int numDebris = 12;
        for (int i = 0; i < numDebris; i++) {
            float debrisAngle = (360.0f / numDebris) * i;
            float debrisHeight = sin(debrisAngle * 3.14159f / 180.0f * 3) * 0.3f;
            float debrisDist = currentSize * 0.7f * progress;
            
            glPushMatrix();
            glRotatef(debrisAngle, 0, 1, 0);
            glTranslatef(debrisDist, debrisHeight, 0);
            
            glColor4f(1.0f, 0.5f - progress * 0.3f, 0.1f, alpha * 0.8f);
            drawSphere(0.08f * size * (1.0f - progress * 0.5f), 6);
            glPopMatrix();
        }
        
        // Shockwave ring
        if (progress > 0.1f && progress < 0.6f) {
            float ringProgress = (progress - 0.1f) / 0.5f;
            float ringSize = currentSize * (1.0f + ringProgress * 2.0f);
            float ringAlpha = (1.0f - ringProgress) * 0.4f;
            
            glColor4f(1.0f, 0.7f, 0.3f, ringAlpha);
            glPushMatrix();
            glRotatef(90, 1, 0, 0);
            gluDisk(gluNewQuadric(), ringSize * 0.9f, ringSize, 24, 1);
            glPopMatrix();
        }
        
        disableGlow();
        glPopMatrix();
    }

    // ==================== BLOOD SPLATTER ====================
    inline void drawBloodSplatter(float progress, float size = 1.0f) {
        if (progress < 0 || progress > 1) return;
        
        float alpha = 1.0f - progress;
        float currentSize = size * (0.3f + progress * 0.7f);
        
        glPushMatrix();
        enableGlow();
        
        // Main splatter
        glColor4f(0.7f, 0.05f, 0.02f, alpha * 0.8f);
        drawSphere(currentSize * 0.3f, 8);
        
        // Blood droplets
        int numDrops = 8;
        for (int i = 0; i < numDrops; i++) {
            float angle = (360.0f / numDrops) * i + i * 17.0f;
            float dist = currentSize * progress * 0.6f;
            float dropSize = 0.05f * size * (1.0f - progress * 0.7f);
            
            glPushMatrix();
            glRotatef(angle, 0, 1, 0);
            glRotatef(30 + i * 10, 1, 0, 0);
            glTranslatef(0, dist, 0);
            
            glColor4f(0.6f, 0.02f, 0.01f, alpha * 0.9f);
            drawSphere(dropSize, 6);
            glPopMatrix();
        }
        
        disableGlow();
        glPopMatrix();
    }

    // ==================== PARTICLE SYSTEM ====================
    struct Particle {
        float x, y, z;
        float vx, vy, vz;
        float life;
        float maxLife;
        float size;
        float r, g, b, a;
    };

    inline void drawParticle(const Particle& p) {
        if (p.life <= 0) return;
        
        float lifeFactor = p.life / p.maxLife;
        float alpha = p.a * lifeFactor;
        float size = p.size * (1.0f - (1.0f - lifeFactor) * 0.5f);
        
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        
        enableGlow();
        glColor4f(p.r, p.g, p.b, alpha);
        drawSphere(size, 6);
        disableGlow();
        
        glPopMatrix();
    }

    // ==================== DAMAGE INDICATOR ====================
    inline void drawDamageIndicator(float angle, float intensity) {
        // Screen-space damage direction indicator
        glPushMatrix();
        enableGlow();
        
        glRotatef(angle, 0, 0, 1);
        
        float alpha = intensity * 0.6f;
        glColor4f(1.0f, 0.1f, 0.05f, alpha);
        
        // Arrow pointing to damage source
        glBegin(GL_TRIANGLES);
        glVertex3f(0, 0.8f, 0);
        glVertex3f(-0.15f, 0.5f, 0);
        glVertex3f(0.15f, 0.5f, 0);
        glEnd();
        
        disableGlow();
        glPopMatrix();
    }

    // ==================== CROSSHAIR ====================
    inline void drawCrosshair(float spread = 0.0f, bool targetInSight = false) {
        enableGlow();
        
        float baseSpread = 0.03f + spread * 0.02f;
        float lineLen = 0.02f;
        
        if (targetInSight) {
            glColor4f(1.0f, 0.2f, 0.1f, 0.9f);
        } else {
            glColor4f(0.2f, 1.0f, 0.3f, 0.8f);
        }
        
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        // Top
        glVertex3f(0, baseSpread + lineLen, 0);
        glVertex3f(0, baseSpread, 0);
        // Bottom
        glVertex3f(0, -baseSpread - lineLen, 0);
        glVertex3f(0, -baseSpread, 0);
        // Left
        glVertex3f(-baseSpread - lineLen, 0, 0);
        glVertex3f(-baseSpread, 0, 0);
        // Right
        glVertex3f(baseSpread + lineLen, 0, 0);
        glVertex3f(baseSpread, 0, 0);
        glEnd();
        
        // Center dot
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        glVertex3f(0, 0, 0);
        glEnd();
        
        disableGlow();
    }

    // ==================== PICKUP GLOW ====================
    inline void drawPickupGlow(float r, float g, float b, float intensity = 1.0f) {
        float pulse = sin(getTime() * 4) * 0.2f + 0.8f;
        float size = 0.6f + pulse * 0.15f;
        
        glPushMatrix();
        enableGlow();
        
        // Inner glow
        glColor4f(r, g, b, intensity * 0.5f * pulse);
        drawSphere(size * 0.5f, 12);
        
        // Outer glow
        glColor4f(r, g, b, intensity * 0.2f * pulse);
        drawSphere(size, 12);
        
        // Rising particles
        for (int i = 0; i < 6; i++) {
            float particleT = fmod(getTime() * 0.5f + i * 0.15f, 1.0f);
            float particleY = particleT * 1.5f;
            float particleAlpha = (1.0f - particleT) * intensity * 0.5f;
            float angle = i * 60.0f + getTime() * 30;
            
            glPushMatrix();
            glRotatef(angle, 0, 1, 0);
            glTranslatef(0.3f, particleY, 0);
            glColor4f(r, g, b, particleAlpha);
            drawSphere(0.05f, 6);
            glPopMatrix();
        }
        
        disableGlow();
        glPopMatrix();
    }

    // ==================== WEAPON LIGHT CONE ====================
    inline void drawWeaponLightCone(float range, float angle, float intensity) {
        if (intensity < 0.01f) return;
        
        glPushMatrix();
        enableGlow();
        
        // Main light cone
        float coneRadius = range * tan(angle * 3.14159f / 180.0f);
        
        // Volumetric light effect (multiple transparent cones)
        for (int layer = 0; layer < 4; layer++) {
            float layerAlpha = intensity * 0.08f * (1.0f - layer * 0.2f);
            float layerRadius = coneRadius * (1.0f + layer * 0.15f);
            
            glColor4f(1.0f, 0.98f, 0.85f, layerAlpha);
            glPushMatrix();
            glRotatef(-90, 1, 0, 0);
            gluCylinder(gluNewQuadric(), 0.02f + layer * 0.02f, layerRadius, range, 16, 1);
            glPopMatrix();
        }
        
        // Light spot at end
        glColor4f(1.0f, 0.95f, 0.8f, intensity * 0.15f);
        glPushMatrix();
        glTranslatef(0, range, 0);
        glRotatef(90, 1, 0, 0);
        gluDisk(gluNewQuadric(), 0, coneRadius * 0.8f, 16, 1);
        glPopMatrix();
        
        disableGlow();
        glPopMatrix();
    }

    // ==================== FOOTSTEP DUST ====================
    inline void drawFootstepDust(float progress, float size = 0.3f) {
        if (progress < 0 || progress > 1) return;
        
        float alpha = (1.0f - progress) * 0.4f;
        float expand = 1.0f + progress * 2.0f;
        
        glPushMatrix();
        enableGlow();
        
        // Dust cloud particles
        for (int i = 0; i < 6; i++) {
            float angle = i * 60.0f;
            float dist = size * expand * 0.5f;
            float pSize = size * 0.15f * (1.0f - progress * 0.5f);
            float height = progress * 0.2f;
            
            glPushMatrix();
            glRotatef(angle, 0, 1, 0);
            glTranslatef(dist, height, 0);
            
            glColor4f(0.6f, 0.55f, 0.45f, alpha);
            drawSphere(pSize, 6);
            glPopMatrix();
        }
        
        disableGlow();
        glPopMatrix();
    }

    // ==================== ENERGY SHIELD (for boss) ====================
    inline void drawEnergyShield(float health, float maxHealth) {
        float healthPercent = health / maxHealth;
        float pulse = sin(getTime() * 5) * 0.1f + 0.9f;
        
        glPushMatrix();
        enableGlow();
        
        // Shield color based on health
        float r = 1.0f - healthPercent;
        float g = 0.2f + healthPercent * 0.3f;
        float b = healthPercent * 0.8f;
        
        // Hexagonal pattern (simplified as sphere layers)
        for (int layer = 0; layer < 3; layer++) {
            float layerSize = 2.5f + layer * 0.1f;
            float layerAlpha = 0.15f * pulse * (1.0f - layer * 0.3f);
            
            glColor4f(r, g, b, layerAlpha);
            
            // Draw wireframe sphere
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            drawSphere(layerSize, 12);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        // Impact sparks (when hit)
        if (healthPercent < 1.0f) {
            float sparkIntensity = (1.0f - healthPercent) * pulse;
            glColor4f(1.0f, 1.0f, 1.0f, sparkIntensity * 0.5f);
            
            for (int i = 0; i < 8; i++) {
                glPushMatrix();
                glRotatef(i * 45 + getTime() * 100, 0, 1, 0);
                glRotatef(30, 1, 0, 0);
                glTranslatef(2.5f, 0, 0);
                drawSphere(0.1f * sparkIntensity, 6);
                glPopMatrix();
            }
        }
        
        disableGlow();
        glPopMatrix();
    }

    // ==================== TELEPORT EFFECT ====================
    inline void drawTeleportEffect(float progress, bool appearing) {
        if (progress < 0 || progress > 1) return;
        
        float effectProgress = appearing ? progress : (1.0f - progress);
        float alpha = effectProgress;
        
        glPushMatrix();
        enableGlow();
        
        // Vertical beam
        glColor4f(0.5f, 0.2f, 0.9f, alpha * 0.6f);
        glPushMatrix();
        glTranslatef(0, 1.5f, 0);
        glRotatef(90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), 0.6f * effectProgress, 0.6f * effectProgress, 3.0f, 12, 1);
        glPopMatrix();
        
        // Rings
        int numRings = 5;
        for (int i = 0; i < numRings; i++) {
            float ringT = fmod((getTime() * 2 + i * 0.2f), 1.0f);
            float ringY = ringT * 3.0f;
            float ringAlpha = (1.0f - ringT) * alpha * 0.5f;
            float ringSize = 0.8f + ringT * 0.5f;
            
            glPushMatrix();
            glTranslatef(0, ringY, 0);
            glRotatef(90, 1, 0, 0);
            glColor4f(0.6f, 0.3f, 1.0f, ringAlpha);
            gluDisk(gluNewQuadric(), ringSize * 0.8f, ringSize, 16, 1);
            glPopMatrix();
        }
        
        // Ground effect
        glColor4f(0.5f, 0.2f, 0.9f, alpha * 0.4f);
        glPushMatrix();
        glRotatef(90, 1, 0, 0);
        gluDisk(gluNewQuadric(), 0, 1.2f * effectProgress, 16, 1);
        glPopMatrix();
        
        disableGlow();
        glPopMatrix();
    }
}

#endif // EFFECTS_MODELS_H
