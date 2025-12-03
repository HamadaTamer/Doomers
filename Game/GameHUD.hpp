/**
 * DOOMERS - Game HUD (CS:GO / Valorant Style)
 */

#pragma once

#include "../Engine/Core.hpp"
#include <glut.h>
#include <cstdio>
#include <cmath>
#include <string>
#include <deque>

namespace Doomers {

struct KillEntry {
    std::string text;
    float timer;
    KillEntry(const std::string& t) : text(t), timer(5.0f) {}
};

class GameHUD {
public:
    int width = 1280;
    int height = 720;
    std::deque<KillEntry> killFeed;
    float damageAlpha = 0;
    float hitAlpha = 0;
    float lowHealthPulse = 0;
    
    void init(int w, int h) { width = w; height = h; }
    
    void update(float dt) {
        for (auto& k : killFeed) k.timer -= dt;
        while (!killFeed.empty() && killFeed.front().timer <= 0) killFeed.pop_front();
        if (damageAlpha > 0) damageAlpha -= dt * 3.0f;
        if (hitAlpha > 0) hitAlpha -= dt * 5.0f;
        lowHealthPulse += dt * 4.0f;
    }
    
    void addKill(const std::string& name) {
        killFeed.push_back(KillEntry("Killed " + name));
        if (killFeed.size() > 5) killFeed.pop_front();
    }
    
    void showDamage() { damageAlpha = 0.6f; }
    void showHit() { hitAlpha = 1.0f; }
    
    void draw(float hp, float maxHp, float armor, int ammo, int maxAmmo, 
              int score, int kills, int level, int enemiesLeft) {
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, width, 0, height);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        drawHealthBar(hp, maxHp);
        drawArmorBar(armor);
        drawAmmoCounter(ammo, maxAmmo);
        drawCrosshair();
        drawMinimap();
        drawKillFeed();
        drawScore(score, kills);
        drawLevelInfo(level, enemiesLeft);
        drawDamageOverlay(hp, maxHp);
        drawHitMarker();
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
private:
    void drawHealthBar(float hp, float maxHp) {
        float x = 30, y = 30, w = 220, h = 28;
        float pct = hp / maxHp;
        
        // Background
        glColor4f(0.1f, 0.1f, 0.1f, 0.85f);
        rect(x, y, w, h);
        
        // Health color
        if (pct > 0.6f) glColor4f(0.2f, 0.9f, 0.3f, 0.95f);
        else if (pct > 0.3f) glColor4f(0.95f, 0.8f, 0.1f, 0.95f);
        else {
            float p = 0.7f + 0.3f * sinf(lowHealthPulse);
            glColor4f(0.95f, 0.15f, 0.15f, p);
        }
        rect(x + 3, y + 3, (w - 6) * pct, h - 6);
        
        // Border
        glColor4f(0.9f, 0.9f, 0.9f, 0.9f);
        rectOutline(x, y, w, h);
        
        // Health cross icon
        glColor4f(1, 1, 1, 0.95f);
        rect(x - 28, y + 9, 18, 10);
        rect(x - 24, y + 5, 10, 18);
        
        // Text
        char txt[16];
        sprintf_s(txt, "%.0f", hp);
        text(x + w/2 - 12, y + 8, txt, 1, 1, 1);
    }
    
    void drawArmorBar(float armor) {
        if (armor <= 0) return;
        float x = 30, y = 65, w = 220, h = 18;
        float pct = armor / 100.0f;
        
        glColor4f(0.1f, 0.1f, 0.1f, 0.8f);
        rect(x, y, w, h);
        
        glColor4f(0.2f, 0.5f, 0.95f, 0.9f);
        rect(x + 2, y + 2, (w - 4) * pct, h - 4);
        
        glColor4f(0.5f, 0.7f, 1.0f, 0.9f);
        rectOutline(x, y, w, h);
    }
    
    void drawAmmoCounter(int ammo, int maxAmmo) {
        float x = width - 250, y = 30;
        
        // Panel
        glColor4f(0.1f, 0.1f, 0.1f, 0.8f);
        rect(x, y, 220, 70);
        glColor4f(0.4f, 0.4f, 0.4f, 0.8f);
        rectOutline(x, y, 220, 70);
        
        // Ammo number
        char txt[32];
        if (ammo > 10) glColor4f(1, 1, 1, 1);
        else if (ammo > 0) glColor4f(1, 0.7f, 0, 1);
        else glColor4f(1, 0.2f, 0.2f, 1);
        
        sprintf_s(txt, "%d", ammo);
        textLarge(x + 25, y + 25, txt);
        
        // Reserve
        glColor4f(0.6f, 0.6f, 0.6f, 1);
        sprintf_s(txt, "/ %d", maxAmmo);
        text(x + 100, y + 30, txt, 0.6f, 0.6f, 0.6f);
        
        // Weapon name
        text(x + 25, y + 52, "LASER RIFLE", 0.4f, 0.8f, 1.0f);
    }
    
    void drawCrosshair() {
        float cx = width / 2.0f, cy = height / 2.0f;
        float gap = 5, len = 10, th = 2;
        
        // Cyan crosshair
        glColor4f(0, 1, 0.85f, 0.95f);
        rect(cx - th/2, cy + gap, th, len);
        rect(cx - th/2, cy - gap - len, th, len);
        rect(cx - gap - len, cy - th/2, len, th);
        rect(cx + gap, cy - th/2, len, th);
        
        // Center dot
        glColor4f(0, 1, 0.85f, 0.8f);
        rect(cx - 1.5f, cy - 1.5f, 3, 3);
        
        // Outline
        glColor4f(0, 0, 0, 0.5f);
        rectOutline(cx - th/2 - 1, cy + gap - 1, th + 2, len + 2);
        rectOutline(cx - th/2 - 1, cy - gap - len - 1, th + 2, len + 2);
        rectOutline(cx - gap - len - 1, cy - th/2 - 1, len + 2, th + 2);
        rectOutline(cx + gap - 1, cy - th/2 - 1, len + 2, th + 2);
    }
    
    void drawHitMarker() {
        if (hitAlpha <= 0) return;
        float cx = width / 2.0f, cy = height / 2.0f;
        float sz = 14;
        
        glColor4f(1, 1, 1, hitAlpha);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        // X shape
        glVertex2f(cx - sz, cy - sz); glVertex2f(cx - sz/3, cy - sz/3);
        glVertex2f(cx + sz, cy - sz); glVertex2f(cx + sz/3, cy - sz/3);
        glVertex2f(cx - sz, cy + sz); glVertex2f(cx - sz/3, cy + sz/3);
        glVertex2f(cx + sz, cy + sz); glVertex2f(cx + sz/3, cy + sz/3);
        glEnd();
        glLineWidth(1.0f);
    }
    
    void drawMinimap() {
        float x = width - 170, y = height - 170, sz = 140;
        float cx = x + sz/2, cy = y + sz/2;
        
        // Background circle
        glColor4f(0.1f, 0.1f, 0.1f, 0.65f);
        circle(cx, cy, sz/2, 32);
        
        // Border
        glColor4f(0.5f, 0.5f, 0.5f, 0.8f);
        circleOutline(cx, cy, sz/2, 32);
        
        // Player arrow
        glColor4f(0, 1, 0.5f, 1);
        glBegin(GL_TRIANGLES);
        glVertex2f(cx, cy + 10);
        glVertex2f(cx - 6, cy - 6);
        glVertex2f(cx + 6, cy - 6);
        glEnd();
        
        // N indicator
        text(cx - 4, y + sz - 18, "N", 1, 1, 1);
    }
    
    void drawKillFeed() {
        float x = width - 320, y = height - 200;
        int i = 0;
        for (const auto& k : killFeed) {
            float a = std::min(1.0f, k.timer);
            glColor4f(0, 0, 0, 0.6f * a);
            rect(x, y - i * 28, 290, 25);
            text(x + 8, y - i * 28 + 7, k.text.c_str(), 1, 0.85f, 0.2f);
            i++;
        }
    }
    
    void drawScore(int score, int kills) {
        float cx = width / 2.0f, y = height - 45;
        
        glColor4f(0.1f, 0.1f, 0.1f, 0.75f);
        rect(cx - 90, y - 8, 180, 40);
        
        char txt[64];
        sprintf_s(txt, "SCORE: %d", score);
        text(cx - 55, y + 12, txt, 1, 0.95f, 0.3f);
        
        sprintf_s(txt, "KILLS: %d", kills);
        text(cx - 40, y - 5, txt, 0.85f, 0.85f, 0.85f);
    }
    
    void drawLevelInfo(int level, int enemies) {
        float x = 25, y = height - 45;
        
        const char* name = (level == 1) ? "RESEARCH FACILITY" : "HELL ARENA";
        
        glColor4f(0.1f, 0.1f, 0.1f, 0.75f);
        rect(x, y - 8, 200, 40);
        
        text(x + 12, y + 12, name, 0.3f, 0.85f, 1.0f);
        
        char txt[32];
        sprintf_s(txt, "ENEMIES: %d", enemies);
        text(x + 12, y - 5, txt, 1, 0.5f, 0.3f);
    }
    
    void drawDamageOverlay(float hp, float maxHp) {
        float alpha = damageAlpha;
        if (hp < maxHp * 0.3f) {
            alpha = std::max(alpha, 0.2f + 0.15f * sinf(lowHealthPulse));
        }
        if (alpha <= 0) return;
        
        // Vignette edges
        float edge = 100;
        
        // Top
        glBegin(GL_QUADS);
        glColor4f(0.85f, 0, 0, alpha * 0.6f);
        glVertex2f(0, height); glVertex2f(width, height);
        glColor4f(0.85f, 0, 0, 0);
        glVertex2f(width, height - edge); glVertex2f(0, height - edge);
        glEnd();
        
        // Bottom
        glBegin(GL_QUADS);
        glColor4f(0.85f, 0, 0, alpha * 0.6f);
        glVertex2f(0, 0); glVertex2f(width, 0);
        glColor4f(0.85f, 0, 0, 0);
        glVertex2f(width, edge); glVertex2f(0, edge);
        glEnd();
        
        // Left
        glBegin(GL_QUADS);
        glColor4f(0.85f, 0, 0, alpha * 0.6f);
        glVertex2f(0, 0); glVertex2f(0, height);
        glColor4f(0.85f, 0, 0, 0);
        glVertex2f(80, height); glVertex2f(80, 0);
        glEnd();
        
        // Right
        glBegin(GL_QUADS);
        glColor4f(0.85f, 0, 0, alpha * 0.6f);
        glVertex2f(width, 0); glVertex2f(width, height);
        glColor4f(0.85f, 0, 0, 0);
        glVertex2f(width - 80, height); glVertex2f(width - 80, 0);
        glEnd();
    }
    
    // Helpers
    void rect(float x, float y, float w, float h) {
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x+w, y);
        glVertex2f(x+w, y+h); glVertex2f(x, y+h);
        glEnd();
    }
    
    void rectOutline(float x, float y, float w, float h) {
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y); glVertex2f(x+w, y);
        glVertex2f(x+w, y+h); glVertex2f(x, y+h);
        glEnd();
    }
    
    void circle(float cx, float cy, float r, int seg) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= seg; i++) {
            float a = 6.28318f * i / seg;
            glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
        }
        glEnd();
    }
    
    void circleOutline(float cx, float cy, float r, int seg) {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < seg; i++) {
            float a = 6.28318f * i / seg;
            glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
        }
        glEnd();
    }
    
    void text(float x, float y, const char* str, float r, float g, float b) {
        glColor4f(r, g, b, 1);
        glRasterPos2f(x, y);
        while (*str) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *str++);
    }
    
    void textLarge(float x, float y, const char* str) {
        glRasterPos2f(x, y);
        while (*str) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str++);
    }
};

} // namespace Doomers
