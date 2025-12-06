// ============================================================================
// DOOMERS - HUD.h
// Professional Heads-Up Display with styled text rendering
// Uses bitmap fonts with visual enhancements for a polished look
// ============================================================================
#ifndef HUD_H
#define HUD_H

#include "GameConfig.h"
#include <glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// ============================================================================
// STYLED TEXT RENDERER
// Renders text with shadows, outlines, and glow effects
// ============================================================================
namespace StyledText {
    
    // Draw text with shadow effect
    inline void drawTextWithShadow(float x, float y, const char* text, void* font,
                                   float r, float g, float b, float shadowOffset = 2.0f) {
        // Shadow (dark, offset down-right)
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glRasterPos2f(x + shadowOffset, y - shadowOffset);
        for (int i = 0; text[i] != '\0'; i++) {
            glutBitmapCharacter(font, text[i]);
        }
        
        // Main text
        glColor3f(r, g, b);
        glRasterPos2f(x, y);
        for (int i = 0; text[i] != '\0'; i++) {
            glutBitmapCharacter(font, text[i]);
        }
    }
    
    // Draw text with outline effect (multiple draws)
    inline void drawTextWithOutline(float x, float y, const char* text, void* font,
                                    float r, float g, float b,
                                    float outlineR = 0.0f, float outlineG = 0.0f, float outlineB = 0.0f) {
        // Outline (draw in 8 directions)
        glColor3f(outlineR, outlineG, outlineB);
        float offsets[8][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}, {-1,-1}, {-1,1}, {1,-1}, {1,1}};
        for (int d = 0; d < 8; d++) {
            glRasterPos2f(x + offsets[d][0], y + offsets[d][1]);
            for (int i = 0; text[i] != '\0'; i++) {
                glutBitmapCharacter(font, text[i]);
            }
        }
        
        // Main text
        glColor3f(r, g, b);
        glRasterPos2f(x, y);
        for (int i = 0; text[i] != '\0'; i++) {
            glutBitmapCharacter(font, text[i]);
        }
    }
    
    // Draw text with glow effect
    inline void drawTextWithGlow(float x, float y, const char* text, void* font,
                                 float r, float g, float b, float glowIntensity = 0.5f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        // Glow layers (larger offsets, more transparent)
        for (int layer = 3; layer >= 1; layer--) {
            float alpha = glowIntensity / (layer * 2);
            glColor4f(r, g, b, alpha);
            float offset = layer * 1.5f;
            float offsets[8][2] = {{-offset,0}, {offset,0}, {0,-offset}, {0,offset},
                                   {-offset,-offset}, {-offset,offset}, {offset,-offset}, {offset,offset}};
            for (int d = 0; d < 8; d++) {
                glRasterPos2f(x + offsets[d][0], y + offsets[d][1]);
                for (int i = 0; text[i] != '\0'; i++) {
                    glutBitmapCharacter(font, text[i]);
                }
            }
        }
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Main text
        glColor3f(r, g, b);
        glRasterPos2f(x, y);
        for (int i = 0; text[i] != '\0'; i++) {
            glutBitmapCharacter(font, text[i]);
        }
        
        glDisable(GL_BLEND);
    }
    
    // Get text width
    inline int getTextWidth(const char* text, void* font) {
        int width = 0;
        for (int i = 0; text[i] != '\0'; i++) {
            width += glutBitmapWidth(font, text[i]);
        }
        return width;
    }
}

// ============================================================================
// HUD CLASS
// ============================================================================
class HUD {
public:
    int screenWidth;
    int screenHeight;
    float damageFlash;
    float lowHealthPulse;
    float animTime;
    
    // Font choices - using larger bitmap fonts
    void* fontLarge;
    void* fontMedium;
    void* fontSmall;
    
    HUD() {
        screenWidth = WINDOW_WIDTH;
        screenHeight = WINDOW_HEIGHT;
        damageFlash = 0.0f;
        lowHealthPulse = 0.0f;
        animTime = 0.0f;
        
        // Set up fonts (GLUT bitmap fonts)
        fontLarge = GLUT_BITMAP_TIMES_ROMAN_24;
        fontMedium = GLUT_BITMAP_HELVETICA_18;
        fontSmall = GLUT_BITMAP_HELVETICA_12;
    }
    
    void setScreenSize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
    }
    
    void setDamageFlash(float intensity) {
        damageFlash = intensity;
    }
    
    void update(float deltaTime) {
        animTime += deltaTime;
        
        // Decay damage flash
        if (damageFlash > 0) {
            damageFlash -= deltaTime * 2.0f;
            if (damageFlash < 0) damageFlash = 0;
        }
    }
    
    void beginHUD() {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, screenWidth, 0, screenHeight);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
    }
    
    void endHUD() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    // ========================================================================
    // CROSSHAIR - Dynamic with spread indicator
    // ========================================================================
    void drawCrosshair(float spread = 0.0f, bool enemyInSight = false) {
        float cx = screenWidth / 2.0f;
        float cy = screenHeight / 2.0f;
        float baseSize = 12.0f;
        float size = baseSize + spread * 20.0f;
        float gap = 4.0f + spread * 8.0f;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Outer glow
        if (enemyInSight) {
            glColor4f(1.0f, 0.2f, 0.2f, 0.3f);
        } else {
            glColor4f(0.0f, 0.8f, 1.0f, 0.2f);
        }
        glLineWidth(4.0f);
        glBegin(GL_LINES);
        glVertex2f(cx, cy + gap); glVertex2f(cx, cy + size);
        glVertex2f(cx, cy - gap); glVertex2f(cx, cy - size);
        glVertex2f(cx - gap, cy); glVertex2f(cx - size, cy);
        glVertex2f(cx + gap, cy); glVertex2f(cx + size, cy);
        glEnd();
        
        // Main crosshair
        if (enemyInSight) {
            glColor4f(1.0f, 0.3f, 0.3f, 0.9f);
        } else {
            glColor4f(1.0f, 1.0f, 1.0f, 0.85f);
        }
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(cx, cy + gap); glVertex2f(cx, cy + size);
        glVertex2f(cx, cy - gap); glVertex2f(cx, cy - size);
        glVertex2f(cx - gap, cy); glVertex2f(cx - size, cy);
        glVertex2f(cx + gap, cy); glVertex2f(cx + size, cy);
        glEnd();
        
        // Center dot
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        glVertex2f(cx, cy);
        glEnd();
        
        glDisable(GL_BLEND);
        glLineWidth(1.0f);
    }
    
    // ========================================================================
    // HEALTH BAR - Styled with glow and animations
    // ========================================================================
    void drawHealthBar(int health, int maxHealth) {
        float barWidth = 240.0f;
        float barHeight = 24.0f;
        float x = 25.0f;
        float y = screenHeight - 55.0f;
        float healthPercent = (float)health / maxHealth;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Background with border
        // Dark inner background
        glColor4f(0.05f, 0.0f, 0.0f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + barWidth, y);
        glVertex2f(x + barWidth, y + barHeight);
        glVertex2f(x, y + barHeight);
        glEnd();
        
        // Health fill with gradient
        float fillWidth = barWidth * healthPercent;
        
        // Color based on health percentage
        float r, g, b;
        if (healthPercent > 0.6f) {
            r = 0.2f; g = 0.9f; b = 0.3f;  // Green
        } else if (healthPercent > 0.3f) {
            r = 0.95f; g = 0.75f; b = 0.2f;  // Yellow/Orange
        } else {
            // Red with pulse effect
            float pulse = sin(animTime * 8) * 0.3f + 0.7f;
            r = 0.95f * pulse; g = 0.15f * pulse; b = 0.15f * pulse;
        }
        
        // Gradient fill (top lighter, bottom darker)
        glBegin(GL_QUADS);
        glColor4f(r * 0.7f, g * 0.7f, b * 0.7f, 0.95f);
        glVertex2f(x + 3, y + 3);
        glVertex2f(x + fillWidth - 3, y + 3);
        glColor4f(r, g, b, 0.95f);
        glVertex2f(x + fillWidth - 3, y + barHeight - 3);
        glVertex2f(x + 3, y + barHeight - 3);
        glEnd();
        
        // Highlight stripe
        glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
        glBegin(GL_QUADS);
        glVertex2f(x + 3, y + barHeight - 6);
        glVertex2f(x + fillWidth - 3, y + barHeight - 6);
        glVertex2f(x + fillWidth - 3, y + barHeight - 3);
        glVertex2f(x + 3, y + barHeight - 3);
        glEnd();
        
        // Border
        glColor4f(0.5f, 0.5f, 0.55f, 0.9f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + barWidth, y);
        glVertex2f(x + barWidth, y + barHeight);
        glVertex2f(x, y + barHeight);
        glEnd();
        
        // Corner accents
        glColor4f(0.0f, 0.8f, 1.0f, 0.8f);
        float cornerSize = 6.0f;
        // Top-left
        glBegin(GL_LINES);
        glVertex2f(x, y + barHeight); glVertex2f(x, y + barHeight - cornerSize);
        glVertex2f(x, y + barHeight); glVertex2f(x + cornerSize, y + barHeight);
        // Bottom-right
        glVertex2f(x + barWidth, y); glVertex2f(x + barWidth, y + cornerSize);
        glVertex2f(x + barWidth, y); glVertex2f(x + barWidth - cornerSize, y);
        glEnd();
        
        glDisable(GL_BLEND);
        
        // Health text with shadow
        char healthText[32];
        sprintf(healthText, "HEALTH");
        StyledText::drawTextWithShadow(x, y - 18, healthText, fontMedium, 0.9f, 0.9f, 0.9f);
        
        // Value with glow if low
        char valueText[16];
        sprintf(valueText, "%d", health);
        if (healthPercent < 0.3f) {
            StyledText::drawTextWithGlow(x + barWidth - 40, y + 4, valueText, fontLarge, 1.0f, 0.3f, 0.3f, 0.6f);
        } else {
            StyledText::drawTextWithShadow(x + barWidth - 40, y + 4, valueText, fontLarge, 1.0f, 1.0f, 1.0f);
        }
        
        glLineWidth(1.0f);
    }
    
    // ========================================================================
    // AMMO COUNTER - Military style
    // ========================================================================
    void drawAmmoCounter(int ammo, int maxAmmo) {
        float x = screenWidth - 260.0f;
        float y = screenHeight - 55.0f;
        float boxWidth = 230.0f;
        float boxHeight = 45.0f;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Background box
        glColor4f(0.08f, 0.08f, 0.1f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(x, y - 5);
        glVertex2f(x + boxWidth, y - 5);
        glVertex2f(x + boxWidth, y + boxHeight - 5);
        glVertex2f(x, y + boxHeight - 5);
        glEnd();
        
        // Border
        glColor4f(0.4f, 0.45f, 0.5f, 0.8f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y - 5);
        glVertex2f(x + boxWidth, y - 5);
        glVertex2f(x + boxWidth, y + boxHeight - 5);
        glVertex2f(x, y + boxHeight - 5);
        glEnd();
        
        // Ammo icon (bullet shape)
        glColor4f(0.9f, 0.7f, 0.2f, 0.9f);
        float iconX = x + 15;
        float iconY = y + 12;
        
        // Bullet casing
        glBegin(GL_QUADS);
        glVertex2f(iconX, iconY);
        glVertex2f(iconX + 8, iconY);
        glVertex2f(iconX + 8, iconY + 20);
        glVertex2f(iconX, iconY + 20);
        glEnd();
        
        // Bullet tip
        glColor4f(0.7f, 0.5f, 0.15f, 0.9f);
        glBegin(GL_TRIANGLES);
        glVertex2f(iconX, iconY + 20);
        glVertex2f(iconX + 8, iconY + 20);
        glVertex2f(iconX + 4, iconY + 28);
        glEnd();
        
        glDisable(GL_BLEND);
        
        // Ammo text
        StyledText::drawTextWithShadow(x + 35, y - 18, "AMMUNITION", fontSmall, 0.7f, 0.7f, 0.75f);
        
        // Current ammo - large
        char ammoText[16];
        sprintf(ammoText, "%d", ammo);
        
        // Glow if low ammo
        if (ammo < maxAmmo * 0.25f && ammo > 0) {
            float pulse = sin(animTime * 6) * 0.3f + 0.7f;
            StyledText::drawTextWithGlow(x + 40, y + 8, ammoText, fontLarge, 0.95f * pulse, 0.6f * pulse, 0.2f * pulse, 0.5f);
        } else if (ammo == 0) {
            StyledText::drawTextWithGlow(x + 40, y + 8, ammoText, fontLarge, 0.9f, 0.2f, 0.2f, 0.7f);
        } else {
            StyledText::drawTextWithShadow(x + 40, y + 8, ammoText, fontLarge, 0.95f, 0.85f, 0.4f);
        }
        
        // Separator
        glColor3f(0.5f, 0.5f, 0.5f);
        glRasterPos2f(x + 90, y + 8);
        glutBitmapCharacter(fontLarge, '/');
        
        // Max ammo
        char maxText[16];
        sprintf(maxText, "%d", maxAmmo);
        StyledText::drawTextWithShadow(x + 105, y + 8, maxText, fontMedium, 0.6f, 0.6f, 0.65f);
        
        // Ammo bar visual
        float barX = x + 150;
        float barY = y + 8;
        float barW = 70.0f;
        float barH = 18.0f;
        float ammoPercent = (float)ammo / maxAmmo;
        
        glEnable(GL_BLEND);
        
        // Bar background
        glColor4f(0.15f, 0.15f, 0.18f, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(barX, barY);
        glVertex2f(barX + barW, barY);
        glVertex2f(barX + barW, barY + barH);
        glVertex2f(barX, barY + barH);
        glEnd();
        
        // Bar fill
        glColor4f(0.9f, 0.7f, 0.2f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(barX + 2, barY + 2);
        glVertex2f(barX + (barW - 4) * ammoPercent + 2, barY + 2);
        glVertex2f(barX + (barW - 4) * ammoPercent + 2, barY + barH - 2);
        glVertex2f(barX + 2, barY + barH - 2);
        glEnd();
        
        glDisable(GL_BLEND);
        glLineWidth(1.0f);
    }
    
    // ========================================================================
    // SCORE - Arcade style with glow
    // ========================================================================
    void drawScore(int score) {
        float x = screenWidth / 2.0f;
        float y = screenHeight - 45.0f;
        
        char scoreText[32];
        sprintf(scoreText, "SCORE: %08d", score);
        
        int textWidth = StyledText::getTextWidth(scoreText, fontLarge);
        
        StyledText::drawTextWithGlow(x - textWidth / 2.0f, y, scoreText, fontLarge, 0.0f, 0.85f, 1.0f, 0.4f);
    }
    
    // ========================================================================
    // TIMER - Digital clock style
    // ========================================================================
    void drawTimer(int seconds) {
        float x = screenWidth / 2.0f;
        float y = screenHeight - 75.0f;
        
        int mins = seconds / 60;
        int secs = seconds % 60;
        
        char timeText[16];
        sprintf(timeText, "%02d:%02d", mins, secs);
        
        int textWidth = StyledText::getTextWidth(timeText, fontLarge);
        
        // Warning flash if low time
        if (seconds < 30) {
            float flash = sin(animTime * 8) * 0.4f + 0.6f;
            StyledText::drawTextWithGlow(x - textWidth / 2.0f, y, timeText, fontLarge, 0.95f * flash, 0.2f * flash, 0.2f * flash, 0.6f);
        } else if (seconds < 60) {
            StyledText::drawTextWithShadow(x - textWidth / 2.0f, y, timeText, fontLarge, 0.95f, 0.7f, 0.2f);
        } else {
            StyledText::drawTextWithShadow(x - textWidth / 2.0f, y, timeText, fontMedium, 0.75f, 0.75f, 0.8f);
        }
    }
    
    // ========================================================================
    // LEVEL INDICATOR - Sci-fi style
    // ========================================================================
    void drawLevelIndicator(int level, const char* levelName = NULL) {
        float x = 25.0f;
        float y = 40.0f;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Background panel
        glColor4f(0.05f, 0.1f, 0.15f, 0.75f);
        glBegin(GL_QUADS);
        glVertex2f(x - 5, y - 10);
        glVertex2f(x + 180, y - 10);
        glVertex2f(x + 180, y + 40);
        glVertex2f(x - 5, y + 40);
        glEnd();
        
        // Accent line
        glColor4f(0.0f, 0.8f, 1.0f, 0.9f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2f(x - 5, y + 40);
        glVertex2f(x + 180, y + 40);
        glEnd();
        
        glDisable(GL_BLEND);
        
        // Level number
        char levelText[16];
        sprintf(levelText, "LEVEL %d", level);
        StyledText::drawTextWithOutline(x, y + 18, levelText, fontLarge, 0.0f, 0.9f, 1.0f, 0.0f, 0.0f, 0.0f);
        
        // Level name
        if (levelName != NULL) {
            StyledText::drawTextWithShadow(x, y - 2, levelName, fontSmall, 0.6f, 0.65f, 0.7f);
        } else if (level == 1) {
            StyledText::drawTextWithShadow(x, y - 2, "RESEARCH FACILITY", fontSmall, 0.6f, 0.65f, 0.7f);
        } else if (level == 2) {
            StyledText::drawTextWithShadow(x, y - 2, "HELL ARENA", fontSmall, 0.8f, 0.4f, 0.3f);
        }
        
        glLineWidth(1.0f);
    }
    
    // ========================================================================
    // DAMAGE OVERLAY - Full screen effect
    // ========================================================================
    void drawDamageOverlay(float intensity) {
        if (intensity <= 0) return;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Vignette style damage effect
        float alpha = intensity * 0.6f;
        
        // Full screen tint
        glBegin(GL_QUADS);
        glColor4f(0.9f, 0.0f, 0.0f, alpha * 0.3f);
        glVertex2f(0, 0);
        glVertex2f(screenWidth, 0);
        glVertex2f(screenWidth, screenHeight);
        glVertex2f(0, screenHeight);
        glEnd();
        
        // Edge vignette (darker at edges)
        float edgeSize = 150.0f;
        
        // Left edge
        glBegin(GL_QUADS);
        glColor4f(0.8f, 0.0f, 0.0f, alpha);
        glVertex2f(0, 0);
        glColor4f(0.8f, 0.0f, 0.0f, 0.0f);
        glVertex2f(edgeSize, 0);
        glVertex2f(edgeSize, screenHeight);
        glColor4f(0.8f, 0.0f, 0.0f, alpha);
        glVertex2f(0, screenHeight);
        glEnd();
        
        // Right edge
        glBegin(GL_QUADS);
        glColor4f(0.8f, 0.0f, 0.0f, 0.0f);
        glVertex2f(screenWidth - edgeSize, 0);
        glColor4f(0.8f, 0.0f, 0.0f, alpha);
        glVertex2f(screenWidth, 0);
        glVertex2f(screenWidth, screenHeight);
        glColor4f(0.8f, 0.0f, 0.0f, 0.0f);
        glVertex2f(screenWidth - edgeSize, screenHeight);
        glEnd();
        
        // Top edge
        glBegin(GL_QUADS);
        glColor4f(0.8f, 0.0f, 0.0f, 0.0f);
        glVertex2f(0, screenHeight - edgeSize);
        glVertex2f(screenWidth, screenHeight - edgeSize);
        glColor4f(0.8f, 0.0f, 0.0f, alpha);
        glVertex2f(screenWidth, screenHeight);
        glVertex2f(0, screenHeight);
        glEnd();
        
        // Bottom edge
        glBegin(GL_QUADS);
        glColor4f(0.8f, 0.0f, 0.0f, alpha);
        glVertex2f(0, 0);
        glVertex2f(screenWidth, 0);
        glColor4f(0.8f, 0.0f, 0.0f, 0.0f);
        glVertex2f(screenWidth, edgeSize);
        glVertex2f(0, edgeSize);
        glEnd();
        
        glDisable(GL_BLEND);
    }
    
    // ========================================================================
    // MESSAGE BOX - For game over, level complete, etc.
    // ========================================================================
    void drawMessageBox(const char* title, const char* message, bool showPressKey = true) {
        float boxWidth = 500.0f;
        float boxHeight = 220.0f;
        float x = (screenWidth - boxWidth) / 2.0f;
        float y = (screenHeight - boxHeight) / 2.0f;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Box background with gradient
        glBegin(GL_QUADS);
        glColor4f(0.05f, 0.08f, 0.12f, 0.95f);
        glVertex2f(x, y);
        glVertex2f(x + boxWidth, y);
        glColor4f(0.1f, 0.12f, 0.18f, 0.95f);
        glVertex2f(x + boxWidth, y + boxHeight);
        glVertex2f(x, y + boxHeight);
        glEnd();
        
        // Glowing border
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float pulse = sin(animTime * 3) * 0.2f + 0.8f;
        glColor4f(0.0f * pulse, 0.6f * pulse, 1.0f * pulse, 0.8f);
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + boxWidth, y);
        glVertex2f(x + boxWidth, y + boxHeight);
        glVertex2f(x, y + boxHeight);
        glEnd();
        
        // Inner border
        glColor4f(0.0f, 0.4f, 0.6f, 0.5f);
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 10, y + 10);
        glVertex2f(x + boxWidth - 10, y + 10);
        glVertex2f(x + boxWidth - 10, y + boxHeight - 10);
        glVertex2f(x + 10, y + boxHeight - 10);
        glEnd();
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);
        
        // Title with glow
        int titleWidth = StyledText::getTextWidth(title, fontLarge);
        StyledText::drawTextWithGlow(
            (screenWidth - titleWidth) / 2.0f, y + boxHeight - 50, 
            title, fontLarge, 1.0f, 0.85f, 0.2f, 0.5f
        );
        
        // Message
        int msgWidth = StyledText::getTextWidth(message, fontMedium);
        StyledText::drawTextWithShadow(
            (screenWidth - msgWidth) / 2.0f, y + boxHeight / 2.0f,
            message, fontMedium, 1.0f, 1.0f, 1.0f
        );
        
        // Press key prompt (animated)
        if (showPressKey) {
            float promptAlpha = sin(animTime * 4) * 0.3f + 0.7f;
            const char* prompt = "Press SPACE to continue";
            int promptWidth = StyledText::getTextWidth(prompt, fontSmall);
            
            glColor4f(0.5f, 0.6f, 0.7f, promptAlpha);
            glRasterPos2f((screenWidth - promptWidth) / 2.0f, y + 35);
            for (int i = 0; prompt[i] != '\0'; i++) {
                glutBitmapCharacter(fontSmall, prompt[i]);
            }
        }
        
        glLineWidth(1.0f);
    }
    
    // ========================================================================
    // OBJECTIVE INDICATOR
    // ========================================================================
    void drawObjectiveIndicator(float distanceToObjective, const char* objectiveText = NULL) {
        float x = screenWidth - 220.0f;
        float y = 40.0f;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Background
        glColor4f(0.05f, 0.08f, 0.12f, 0.75f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + 195, y);
        glVertex2f(x + 195, y + 50);
        glVertex2f(x, y + 50);
        glEnd();
        
        // Accent
        float pulse = sin(animTime * 2) * 0.2f + 0.8f;
        glColor4f(0.9f * pulse, 0.7f * pulse, 0.1f * pulse, 0.9f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(x, y + 50);
        glVertex2f(x + 195, y + 50);
        glEnd();
        
        glDisable(GL_BLEND);
        
        StyledText::drawTextWithShadow(x + 5, y + 30, "OBJECTIVE", fontSmall, 0.9f, 0.7f, 0.1f);
        
        char distText[32];
        sprintf(distText, "%.0fm", distanceToObjective);
        StyledText::drawTextWithShadow(x + 5, y + 8, distText, fontMedium, 0.95f, 0.95f, 0.95f);
        
        if (objectiveText != NULL) {
            StyledText::drawTextWithShadow(x + 60, y + 8, objectiveText, fontSmall, 0.7f, 0.7f, 0.75f);
        }
        
        glLineWidth(1.0f);
    }
    
    // ========================================================================
    // INTERACTION PROMPT - Shows "Press E" when near interactable objects
    // ========================================================================
    void drawInteractionPrompt(const char* action) {
        if (!action || action[0] == '\0') return;
        
        float cx = screenWidth / 2.0f;
        float cy = screenHeight / 2.0f - 80.0f;  // Below crosshair
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Background box
        int textWidth = StyledText::getTextWidth(action, GLUT_BITMAP_HELVETICA_18);
        float boxWidth = textWidth + 40.0f;
        float boxHeight = 35.0f;
        float boxX = cx - boxWidth / 2.0f;
        float boxY = cy - boxHeight / 2.0f;
        
        // Dark background
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(boxX, boxY);
        glVertex2f(boxX + boxWidth, boxY);
        glVertex2f(boxX + boxWidth, boxY + boxHeight);
        glVertex2f(boxX, boxY + boxHeight);
        glEnd();
        
        // Border
        glColor4f(1.0f, 0.85f, 0.0f, 0.9f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(boxX, boxY);
        glVertex2f(boxX + boxWidth, boxY);
        glVertex2f(boxX + boxWidth, boxY + boxHeight);
        glVertex2f(boxX, boxY + boxHeight);
        glEnd();
        glLineWidth(1.0f);
        
        // Text
        StyledText::drawTextWithOutline(cx - textWidth / 2.0f, cy - 5.0f, action, 
                                        GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 0.8f);
        
        glDisable(GL_BLEND);
    }
    
    // ========================================================================
    // MAIN DRAW FUNCTION
    // ========================================================================
    void draw(int health, int maxHealth, int ammo, int maxAmmo, int score, int timeSeconds, int level) {
        beginHUD();
        
        // Damage overlay first (behind everything)
        if (damageFlash > 0) {
            drawDamageOverlay(damageFlash);
        }
        
        // HUD elements
        drawHealthBar(health, maxHealth);
        drawAmmoCounter(ammo, maxAmmo);
        drawScore(score);
        drawTimer(timeSeconds);
        drawLevelIndicator(level);
        drawCrosshair();
        
        endHUD();
    }
    
    // Full draw with interaction prompt
    void drawWithPrompt(int health, int maxHealth, int ammo, int maxAmmo, int score, 
                        int timeSeconds, int level, const char* interactionPrompt) {
        beginHUD();
        
        if (damageFlash > 0) {
            drawDamageOverlay(damageFlash);
        }
        
        drawHealthBar(health, maxHealth);
        drawAmmoCounter(ammo, maxAmmo);
        drawScore(score);
        drawTimer(timeSeconds);
        drawLevelIndicator(level);
        drawCrosshair();
        
        if (interactionPrompt && interactionPrompt[0] != '\0') {
            drawInteractionPrompt(interactionPrompt);
        }
        
        endHUD();
    }
    
    // Full draw with more info
    void drawFull(int health, int maxHealth, int ammo, int maxAmmo, int score, 
                  int timeSeconds, int level, float objectiveDist, float spread = 0.0f, bool enemyInSight = false) {
        beginHUD();
        
        if (damageFlash > 0) {
            drawDamageOverlay(damageFlash);
        }
        
        drawHealthBar(health, maxHealth);
        drawAmmoCounter(ammo, maxAmmo);
        drawScore(score);
        drawTimer(timeSeconds);
        drawLevelIndicator(level);
        drawObjectiveIndicator(objectiveDist);
        drawCrosshair(spread, enemyInSight);
        
        endHUD();
    }
};

#endif // HUD_H
