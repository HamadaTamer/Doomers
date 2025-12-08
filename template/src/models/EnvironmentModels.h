// ============================================================================
// DOOMERS - EnvironmentModels.h
// Environment props, crates, doors, platforms, etc.
// ============================================================================
#ifndef ENVIRONMENT_MODELS_H
#define ENVIRONMENT_MODELS_H

#include "ModelUtils.h"
#include "../TextureManager.h"

namespace EnvironmentModels {

    using namespace ModelUtils;

    // ==================== CRATES ====================
    inline void drawCrate(float size = 1.0f) {
        glPushMatrix();
        glScalef(size, size, size);
        
        // Main body
        setColor(0.48f, 0.38f, 0.28f);
        drawCube(1.0f);
        
        // Corner metal brackets
        setColorMetallic(0.35f, 0.35f, 0.38f);
        float corners[4][2] = {{0.48f, 0.48f}, {-0.48f, 0.48f}, {0.48f, -0.48f}, {-0.48f, -0.48f}};
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glTranslatef(corners[i][0], 0, corners[i][1]);
            drawBox(0.08f, 1.02f, 0.08f);
            glPopMatrix();
        }
        
        // Top edge trim
        setColorMetallic(0.32f, 0.32f, 0.35f);
        glPushMatrix();
        glTranslatef(0, 0.48f, 0);
        drawBox(1.02f, 0.06f, 1.02f);
        glPopMatrix();
        
        // Bottom edge trim  
        glPushMatrix();
        glTranslatef(0, -0.48f, 0);
        drawBox(1.02f, 0.06f, 1.02f);
        glPopMatrix();
        
        // Wood planks texture (horizontal lines)
        setColor(0.4f, 0.3f, 0.2f);
        for (int i = 0; i < 5; i++) {
            glPushMatrix();
            glTranslatef(0, -0.35f + i * 0.17f, 0.51f);
            drawBox(0.85f, 0.02f, 0.01f);
            glPopMatrix();
        }
        
        glPopMatrix();
    }

    inline void drawSciFiCrate(float size = 1.0f) {
        glPushMatrix();
        glScalef(size, size, size);
        
        // Main metallic body
        setColorMetallic(0.32f, 0.35f, 0.4f);
        drawCube(0.95f);
        
        // Edge trim (glowing)
        setColor(0.0f, 0.7f, 0.9f);
        setEmissive(0.0f, 0.25f, 0.35f);
        
        // Vertical edge lights
        float edgePos[4][2] = {{0.48f, 0.48f}, {-0.48f, 0.48f}, {0.48f, -0.48f}, {-0.48f, -0.48f}};
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glTranslatef(edgePos[i][0], 0, edgePos[i][1]);
            drawBox(0.03f, 0.9f, 0.03f);
            glPopMatrix();
        }
        clearEmissive();
        
        // Front panel with display
        setColorMetallic(0.25f, 0.28f, 0.32f);
        glPushMatrix();
        glTranslatef(0, 0, 0.48f);
        drawBox(0.6f, 0.6f, 0.02f);
        
        // Screen
        setColor(0.1f, 0.3f, 0.4f);
        setEmissive(0.05f, 0.15f, 0.2f);
        glTranslatef(0, 0.05f, 0.015f);
        drawBox(0.4f, 0.25f, 0.01f);
        clearEmissive();
        
        // Status indicators
        setColor(0.1f, 0.9f, 0.2f);
        setEmissive(0.05f, 0.4f, 0.1f);
        glTranslatef(-0.15f, -0.2f, 0);
        drawSphere(0.03f, 8);
        clearEmissive();
        glPopMatrix();
        
        // Side vents
        setColorMetallic(0.2f, 0.22f, 0.25f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.48f, 0, 0);
            for (int v = 0; v < 5; v++) {
                glPushMatrix();
                glTranslatef(0, -0.3f + v * 0.15f, 0);
                drawBox(0.02f, 0.08f, 0.5f);
                glPopMatrix();
            }
            glPopMatrix();
        }
        
        glPopMatrix();
    }

    // ==================== COLLECTIBLES ====================
    inline void drawHealthPack() {
        float bob = sin(getTime() * 3) * 0.12f;
        float spin = getTime() * 60;
        float pulse = sin(getTime() * 4) * 0.1f + 0.9f;
        
        glPushMatrix();
        glTranslatef(0, 0.3f + bob, 0);
        glRotatef(spin, 0, 1, 0);
        
        // Main case
        setColor(0.95f, 0.95f, 0.95f);
        drawBox(0.5f, 0.32f, 0.42f);
        
        // Red cross
        setColor(0.95f * pulse, 0.15f, 0.15f);
        setEmissive(0.4f * pulse, 0.05f, 0.05f);
        
        // Horizontal bar
        glPushMatrix();
        glTranslatef(0, 0.165f, 0);
        drawBox(0.32f, 0.025f, 0.1f);
        glPopMatrix();
        
        // Vertical bar
        glPushMatrix();
        glTranslatef(0, 0.165f, 0);
        drawBox(0.1f, 0.025f, 0.32f);
        glPopMatrix();
        
        // Cross on front
        glPushMatrix();
        glTranslatef(0, 0, 0.22f);
        drawBox(0.25f, 0.08f, 0.02f);
        drawBox(0.08f, 0.25f, 0.02f);
        glPopMatrix();
        clearEmissive();
        
        // Handle
        setColorMetallic(0.5f, 0.5f, 0.5f);
        glPushMatrix();
        glTranslatef(0, 0.2f, 0);
        drawBox(0.25f, 0.03f, 0.04f);
        glPopMatrix();
        
        // Latches
        setColorMetallic(0.6f, 0.55f, 0.45f);
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.2f, 0, 0.22f);
            drawBox(0.05f, 0.1f, 0.03f);
            glPopMatrix();
        }
        
        // Glow effect
        enableGlow();
        glColor4f(1.0f, 0.3f, 0.3f, 0.2f * pulse);
        drawSphere(0.45f, 12);
        disableGlow();
        
        glPopMatrix();
    }

    inline void drawAmmoBox() {
        float bob = sin(getTime() * 2.5f + 1) * 0.1f;
        float spin = getTime() * 45;
        float pulse = sin(getTime() * 3) * 0.1f + 0.9f;
        
        glPushMatrix();
        glTranslatef(0, 0.35f + bob, 0);
        glRotatef(spin, 0, 1, 0);
        
        // Enable texturing for magazine
        glEnable(GL_TEXTURE_2D);
        TextureManager::bind(TEX_AMMO);  // AK-47 magazine texture
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor3f(1.0f, 1.0f, 1.0f);
        
        // Draw textured magazine body - curved shape
        // Main magazine body
        glPushMatrix();
        glRotatef(-10, 1, 0, 0);  // Slight curve angle
        
        // Draw magazine as textured box with proper UVs
        float magW = 0.12f, magH = 0.45f, magD = 0.08f;
        glBegin(GL_QUADS);
        // Front face
        glNormal3f(0, 0, 1);
        glTexCoord2f(0, 0); glVertex3f(-magW, -magH, magD);
        glTexCoord2f(1, 0); glVertex3f(magW, -magH, magD);
        glTexCoord2f(1, 1); glVertex3f(magW, magH, magD);
        glTexCoord2f(0, 1); glVertex3f(-magW, magH, magD);
        // Back face
        glNormal3f(0, 0, -1);
        glTexCoord2f(0, 0); glVertex3f(magW, -magH, -magD);
        glTexCoord2f(1, 0); glVertex3f(-magW, -magH, -magD);
        glTexCoord2f(1, 1); glVertex3f(-magW, magH, -magD);
        glTexCoord2f(0, 1); glVertex3f(magW, magH, -magD);
        // Left face
        glNormal3f(-1, 0, 0);
        glTexCoord2f(0, 0); glVertex3f(-magW, -magH, -magD);
        glTexCoord2f(1, 0); glVertex3f(-magW, -magH, magD);
        glTexCoord2f(1, 1); glVertex3f(-magW, magH, magD);
        glTexCoord2f(0, 1); glVertex3f(-magW, magH, -magD);
        // Right face
        glNormal3f(1, 0, 0);
        glTexCoord2f(0, 0); glVertex3f(magW, -magH, magD);
        glTexCoord2f(1, 0); glVertex3f(magW, -magH, -magD);
        glTexCoord2f(1, 1); glVertex3f(magW, magH, -magD);
        glTexCoord2f(0, 1); glVertex3f(magW, magH, magD);
        // Top face
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, 0); glVertex3f(-magW, magH, magD);
        glTexCoord2f(1, 0); glVertex3f(magW, magH, magD);
        glTexCoord2f(1, 1); glVertex3f(magW, magH, -magD);
        glTexCoord2f(0, 1); glVertex3f(-magW, magH, -magD);
        // Bottom face
        glNormal3f(0, -1, 0);
        glTexCoord2f(0, 0); glVertex3f(-magW, -magH, -magD);
        glTexCoord2f(1, 0); glVertex3f(magW, -magH, -magD);
        glTexCoord2f(1, 1); glVertex3f(magW, -magH, magD);
        glTexCoord2f(0, 1); glVertex3f(-magW, -magH, magD);
        glEnd();
        glPopMatrix();
        
        TextureManager::unbind();
        
        // Glow effect
        enableGlow();
        glColor4f(1.0f, 0.8f, 0.2f, 0.25f * pulse);
        drawSphere(0.35f, 12);
        disableGlow();
        
        glPopMatrix();
    }

    inline void drawKeycard(float r, float g, float b) {
        float bob = sin(getTime() * 4) * 0.15f;
        float spin = getTime() * 90;
        float pulse = sin(getTime() * 5) * 0.2f + 0.8f;
        
        glPushMatrix();
        glTranslatef(0, 0.4f + bob, 0);
        glRotatef(spin, 0, 1, 0);
        glRotatef(15, 1, 0, 0);
        
        // Card body
        setColor(0.15f, 0.15f, 0.2f);
        drawBox(0.4f, 0.02f, 0.25f);
        
        // Colored strip
        setColor(r * pulse, g * pulse, b * pulse);
        setEmissive(r * 0.4f * pulse, g * 0.4f * pulse, b * 0.4f * pulse);
        glPushMatrix();
        glTranslatef(0, 0.015f, 0);
        drawBox(0.35f, 0.01f, 0.08f);
        glPopMatrix();
        
        // Chip
        setColorMetallic(0.7f, 0.65f, 0.4f);
        glPushMatrix();
        glTranslatef(-0.1f, 0.015f, 0.05f);
        drawBox(0.08f, 0.01f, 0.06f);
        glPopMatrix();
        clearEmissive();
        
        // Glow
        enableGlow();
        glColor4f(r, g, b, 0.3f * pulse);
        drawSphere(0.35f, 12);
        disableGlow();
        
        glPopMatrix();
    }

    // ==================== DOORS ====================
    inline void drawDoor(bool isOpen, float openAmount) {
        glPushMatrix();
        
        // Door frame
        setColorMetallic(0.35f, 0.35f, 0.38f);
        
        // Left frame
        glPushMatrix();
        glTranslatef(-1.3f, 1.8f, 0);
        drawBox(0.25f, 3.6f, 0.35f);
        glPopMatrix();
        
        // Right frame
        glPushMatrix();
        glTranslatef(1.3f, 1.8f, 0);
        drawBox(0.25f, 3.6f, 0.35f);
        glPopMatrix();
        
        // Top frame
        glPushMatrix();
        glTranslatef(0, 3.7f, 0);
        drawBox(2.85f, 0.25f, 0.35f);
        glPopMatrix();
        
        // Warning stripes on frame
        setColor(0.9f, 0.7f, 0.0f);
        for (int i = 0; i < 6; i++) {
            glPushMatrix();
            glTranslatef(-1.35f, 0.3f + i * 0.6f, 0.18f);
            glRotatef(45, 0, 0, 1);
            drawBox(0.15f, 0.04f, 0.01f);
            glPopMatrix();
        }
        
        // Door panels (sliding)
        setColorMetallic(0.28f, 0.3f, 0.35f);
        
        // Left door panel
        glPushMatrix();
        glTranslatef(-0.55f - openAmount * 0.6f, 1.75f, 0);
        drawBox(1.0f, 3.3f, 0.15f);
        // Panel details
        setColorMetallic(0.22f, 0.24f, 0.28f);
        for (int row = 0; row < 3; row++) {
            glPushMatrix();
            glTranslatef(0, 0.8f - row * 1.0f, 0.08f);
            drawBox(0.7f, 0.7f, 0.02f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Right door panel
        glPushMatrix();
        glTranslatef(0.55f + openAmount * 0.6f, 1.75f, 0);
        setColorMetallic(0.28f, 0.3f, 0.35f);
        drawBox(1.0f, 3.3f, 0.15f);
        setColorMetallic(0.22f, 0.24f, 0.28f);
        for (int row = 0; row < 3; row++) {
            glPushMatrix();
            glTranslatef(0, 0.8f - row * 1.0f, 0.08f);
            drawBox(0.7f, 0.7f, 0.02f);
            glPopMatrix();
        }
        glPopMatrix();
        
        // Status light
        float lightPulse = sin(getTime() * 3) * 0.2f + 0.8f;
        if (isOpen) {
            setColor(0.1f, 0.9f * lightPulse, 0.2f);
            setEmissive(0.05f, 0.5f * lightPulse, 0.1f);
        } else {
            setColor(0.9f * lightPulse, 0.15f, 0.1f);
            setEmissive(0.5f * lightPulse, 0.05f, 0.05f);
        }
        glPushMatrix();
        glTranslatef(0, 3.85f, 0.2f);
        drawSphere(0.12f, 10);
        glPopMatrix();
        clearEmissive();
        
        glPopMatrix();
    }

    // ==================== PLATFORMS ====================
    inline void drawPlatform(float sizeX, float sizeY, float sizeZ) {
        glPushMatrix();
        
        // Main platform surface
        setColorMetallic(0.38f, 0.4f, 0.45f);
        drawBox(sizeX, sizeY, sizeZ);
        
        // Metal grating pattern on top
        setColorMetallic(0.32f, 0.34f, 0.38f);
        int numLines = (int)(sizeX / 0.3f);
        for (int i = 0; i < numLines; i++) {
            glPushMatrix();
            glTranslatef(-sizeX/2 + 0.15f + i * 0.3f, sizeY/2 + 0.005f, 0);
            drawBox(0.03f, 0.01f, sizeZ - 0.1f);
            glPopMatrix();
        }
        
        // Warning edge stripes
        setColor(0.9f, 0.7f, 0.0f);
        glPushMatrix();
        glTranslatef(0, sizeY/2 + 0.01f, sizeZ/2 - 0.1f);
        drawBox(sizeX - 0.15f, 0.02f, 0.15f);
        glPopMatrix();
        
        // Edge trim
        setColorMetallic(0.5f, 0.48f, 0.4f);
        glPushMatrix();
        glTranslatef(0, sizeY/2, sizeZ/2);
        drawBox(sizeX + 0.05f, 0.08f, 0.08f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0, sizeY/2, -sizeZ/2);
        drawBox(sizeX + 0.05f, 0.08f, 0.08f);
        glPopMatrix();
        
        // Support structure underneath
        setColorMetallic(0.3f, 0.3f, 0.32f);
        float legPositions[4][2] = {
            {sizeX/2 - 0.15f, sizeZ/2 - 0.15f},
            {-sizeX/2 + 0.15f, sizeZ/2 - 0.15f},
            {sizeX/2 - 0.15f, -sizeZ/2 + 0.15f},
            {-sizeX/2 + 0.15f, -sizeZ/2 + 0.15f}
        };
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glTranslatef(legPositions[i][0], -sizeY, legPositions[i][1]);
            drawBox(0.12f, sizeY * 1.5f, 0.12f);
            glPopMatrix();
        }
        
        glPopMatrix();
    }

    // ==================== LAVA ROCK ====================
    inline void drawLavaRock(float size = 1.0f) {
        glPushMatrix();
        glScalef(size, size * 0.7f, size);
        
        // Rock body
        setColor(0.38f, 0.2f, 0.15f);
        drawCube(1.0f);
        
        // Irregular shape additions
        setColor(0.35f, 0.18f, 0.12f);
        glPushMatrix();
        glTranslatef(0.3f, 0.2f, 0.1f);
        drawCube(0.5f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-0.2f, -0.1f, 0.25f);
        drawCube(0.4f);
        glPopMatrix();
        
        // Glowing lava cracks
        float pulse = sin(getTime() * 2) * 0.2f + 0.8f;
        setColor(1.0f * pulse, 0.4f * pulse, 0.0f);
        setEmissive(0.6f * pulse, 0.25f * pulse, 0.0f);
        
        glPushMatrix();
        glTranslatef(0.2f, 0.45f, 0);
        drawBox(0.35f, 0.04f, 0.03f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-0.1f, 0.3f, 0.35f);
        drawBox(0.03f, 0.25f, 0.03f);
        glPopMatrix();
        
        clearEmissive();
        glPopMatrix();
    }

    // ==================== OBELISK (Level 2 Objective) ====================
    inline void drawObelisk(float glowIntensity = 1.0f) {
        float pulse = sin(getTime() * 2) * 0.15f + 0.85f;
        glowIntensity *= pulse;
        
        glPushMatrix();
        
        // Base platform
        setColor(0.12f, 0.12f, 0.15f);
        glPushMatrix();
        glTranslatef(0, 0.3f, 0);
        drawBox(2.5f, 0.6f, 2.5f);
        glPopMatrix();
        
        // Steps
        setColor(0.15f, 0.15f, 0.18f);
        glPushMatrix();
        glTranslatef(0, 0.7f, 0);
        drawBox(2.0f, 0.2f, 2.0f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0, 0.95f, 0);
        drawBox(1.5f, 0.15f, 1.5f);
        glPopMatrix();
        
        // Main obelisk
        setColor(0.08f, 0.08f, 0.1f);
        glPushMatrix();
        glTranslatef(0, 3.5f, 0);
        glScalef(1.0f, 1.0f, 1.0f);
        // Tapered shape
        glBegin(GL_TRIANGLES);
        // Front face
        glNormal3f(0, 0.2f, 1);
        glVertex3f(-0.5f, -2.4f, 0.5f);
        glVertex3f(0.5f, -2.4f, 0.5f);
        glVertex3f(0, 2.4f, 0);
        // Back face
        glNormal3f(0, 0.2f, -1);
        glVertex3f(0.5f, -2.4f, -0.5f);
        glVertex3f(-0.5f, -2.4f, -0.5f);
        glVertex3f(0, 2.4f, 0);
        // Left face
        glNormal3f(-1, 0.2f, 0);
        glVertex3f(-0.5f, -2.4f, -0.5f);
        glVertex3f(-0.5f, -2.4f, 0.5f);
        glVertex3f(0, 2.4f, 0);
        // Right face
        glNormal3f(1, 0.2f, 0);
        glVertex3f(0.5f, -2.4f, 0.5f);
        glVertex3f(0.5f, -2.4f, -0.5f);
        glVertex3f(0, 2.4f, 0);
        glEnd();
        // Bottom
        glBegin(GL_QUADS);
        glNormal3f(0, -1, 0);
        glVertex3f(-0.5f, -2.4f, -0.5f);
        glVertex3f(0.5f, -2.4f, -0.5f);
        glVertex3f(0.5f, -2.4f, 0.5f);
        glVertex3f(-0.5f, -2.4f, 0.5f);
        glEnd();
        glPopMatrix();
        
        // Glowing runes
        setColor(glowIntensity * 0.9f, glowIntensity * 0.2f, glowIntensity * 0.9f);
        setEmissive(glowIntensity * 0.5f, glowIntensity * 0.1f, glowIntensity * 0.5f);
        
        // Runes on faces
        for (int face = 0; face < 4; face++) {
            glPushMatrix();
            glTranslatef(0, 2.5f, 0);
            glRotatef(face * 90, 0, 1, 0);
            glTranslatef(0, 0, 0.35f);
            
            for (int i = 0; i < 4; i++) {
                glPushMatrix();
                glTranslatef(0, -0.8f + i * 0.5f, 0);
                drawBox(0.2f, 0.06f, 0.02f);
                if (i % 2 == 0) {
                    drawBox(0.06f, 0.15f, 0.02f);
                }
                glPopMatrix();
            }
            glPopMatrix();
        }
        
        // Floating crystal at top
        glPushMatrix();
        glTranslatef(0, 6.5f + sin(getTime() * 2) * 0.2f, 0);
        glRotatef(getTime() * 30, 0, 1, 0);
        glRotatef(45, 1, 0, 0);
        drawBox(0.5f, 0.5f, 0.5f);
        glPopMatrix();
        
        clearEmissive();
        
        // Glow aura
        enableGlow();
        glColor4f(0.8f, 0.2f, 0.8f, glowIntensity * 0.25f);
        glPushMatrix();
        glTranslatef(0, 3.5f, 0);
        drawSphere(2.0f + sin(getTime() * 1.5f) * 0.3f, 20);
        glPopMatrix();
        disableGlow();
        
        glPopMatrix();
    }

    // ==================== SCI-FI WALL PANEL ====================
    inline void drawWallPanel(float width, float height) {
        glPushMatrix();
        
        // Base panel
        setColorMetallic(0.2f, 0.22f, 0.28f);
        drawBox(width, height, 0.15f);
        
        // Trim
        setColorMetallic(0.35f, 0.35f, 0.4f);
        // Top trim
        glPushMatrix();
        glTranslatef(0, height/2 - 0.05f, 0.08f);
        drawBox(width, 0.08f, 0.02f);
        glPopMatrix();
        // Bottom trim
        glPushMatrix();
        glTranslatef(0, -height/2 + 0.05f, 0.08f);
        drawBox(width, 0.08f, 0.02f);
        glPopMatrix();
        
        // Vertical supports
        setColorMetallic(0.25f, 0.27f, 0.32f);
        int numSupports = (int)(width / 2.0f);
        for (int i = 0; i <= numSupports; i++) {
            glPushMatrix();
            glTranslatef(-width/2 + i * 2.0f, 0, 0.08f);
            drawBox(0.1f, height - 0.1f, 0.02f);
            glPopMatrix();
        }
        
        // Lights
        float lightPulse = sin(getTime() * 2 + width) * 0.1f + 0.9f;
        setColor(0.2f, 0.6f * lightPulse, 0.8f * lightPulse);
        setEmissive(0.1f, 0.3f * lightPulse, 0.4f * lightPulse);
        for (int i = 0; i < numSupports; i++) {
            glPushMatrix();
            glTranslatef(-width/2 + 1.0f + i * 2.0f, height/2 - 0.3f, 0.1f);
            drawBox(1.5f, 0.06f, 0.02f);
            glPopMatrix();
        }
        clearEmissive();
        
        glPopMatrix();
    }

    // ==================== PILLAR ====================
    inline void drawPillar(float height) {
        glPushMatrix();
        
        // Base - use texture if available
        if (TextureManager::isLoaded(TEX_PILLAR)) {
            TextureManager::bind(TEX_PILLAR);
            glEnable(GL_TEXTURE_2D);
            glColor3f(0.8f, 0.8f, 0.85f); // Light gray tint
        } else {
            setColorMetallic(0.35f, 0.35f, 0.4f);
        }
        glPushMatrix();
        glTranslatef(0, 0.2f, 0);
        TextureManager::drawTexturedBox(TEX_PILLAR, 0, 0, 0, 1.2f, 0.4f, 1.2f, 0.5f);
        glPopMatrix();
        
        // Main column - textured
        glPushMatrix();
        glTranslatef(0, height/2, 0);
        TextureManager::drawTexturedBox(TEX_PILLAR, 0, 0, 0, 0.8f, height - 0.8f, 0.8f, 2.0f);
        glPopMatrix();
        
        // Capital - textured
        glPushMatrix();
        glTranslatef(0, height - 0.2f, 0);
        TextureManager::drawTexturedBox(TEX_PILLAR, 0, 0, 0, 1.2f, 0.4f, 1.2f, 0.5f);
        glPopMatrix();
        
        if (TextureManager::isLoaded(TEX_PILLAR)) {
            TextureManager::unbind();
        }
        
        // Light strips
        float pulse = sin(getTime() * 2) * 0.15f + 0.85f;
        setColor(0.2f, 0.5f * pulse, 0.7f * pulse);
        setEmissive(0.1f, 0.25f * pulse, 0.35f * pulse);
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glRotatef(i * 90, 0, 1, 0);
            glTranslatef(0.42f, height/2, 0);
            drawBox(0.03f, height - 1.0f, 0.06f);
            glPopMatrix();
        }
        clearEmissive();
        
        glPopMatrix();
    }
}

#endif // ENVIRONMENT_MODELS_H
