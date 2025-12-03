/**
 * DOOMERS - HUD System
 * 
 * Professional UI with animations, transitions, and visual effects:
 * - Animated health/ammo bars
 * - Damage indicators (directional)
 * - Hit markers
 * - Kill feed
 * - Smooth transitions
 * - Screen effects (vignette, blood overlay)
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Renderer.hpp"
#include "../Engine/Animation.hpp"

namespace Doomers {

// Forward declaration
class Player;

// ============================================================================
// Damage Indicator - Shows damage direction
// ============================================================================
struct DamageIndicator {
    float angle;      // Direction damage came from
    float intensity;  // 0-1 strength
    float lifetime;
    float age;
    
    DamageIndicator(float ang, float intens = 1.0f)
        : angle(ang), intensity(intens), lifetime(1.5f), age(0) {}
    
    bool isAlive() const { return age < lifetime; }
    float getAlpha() const {
        float fadeStart = lifetime * 0.5f;
        if (age > fadeStart) {
            return intensity * (1.0f - (age - fadeStart) / (lifetime - fadeStart));
        }
        return intensity;
    }
};

// ============================================================================
// Hit Marker - Crosshair feedback on hit
// ============================================================================
struct HitMarker {
    float lifetime;
    float age;
    bool isKill;
    
    HitMarker(bool kill = false) : lifetime(0.3f), age(0), isKill(kill) {}
    bool isAlive() const { return age < lifetime; }
    float getScale() const {
        float t = age / lifetime;
        return 1.0f + Anim::Ease::OutBack(t) * 0.5f;
    }
    float getAlpha() const {
        return 1.0f - Anim::Ease::InQuad(age / lifetime);
    }
};

// ============================================================================
// Kill Feed Entry
// ============================================================================
struct KillFeedEntry {
    std::string text;
    Math::Color color;
    float lifetime;
    float age;
    
    KillFeedEntry(const std::string& t, const Math::Color& c)
        : text(t), color(c), lifetime(4.0f), age(0) {}
    
    bool isAlive() const { return age < lifetime; }
    float getAlpha() const {
        if (age > lifetime - 0.5f) {
            return (lifetime - age) / 0.5f;
        }
        if (age < 0.3f) {
            return Anim::Ease::OutBack(age / 0.3f);
        }
        return 1.0f;
    }
};

// ============================================================================
// Floating Text - Score popups, damage numbers
// ============================================================================
struct FloatingText {
    std::string text;
    Math::Vector2 position;
    Math::Vector2 velocity;
    Math::Color color;
    float lifetime;
    float age;
    float scale;
    
    FloatingText(const std::string& t, const Math::Vector2& pos, const Math::Color& c)
        : text(t), position(pos), velocity(0, -50), color(c), lifetime(1.5f), age(0), scale(1.0f) {}
    
    void update(float dt) {
        age += dt;
        position = position + velocity * dt;
        velocity.y *= 0.95f; // Slow down
    }
    
    bool isAlive() const { return age < lifetime; }
    float getAlpha() const {
        if (age > lifetime * 0.7f) {
            return (lifetime - age) / (lifetime * 0.3f);
        }
        return 1.0f;
    }
    float getScale() const {
        if (age < 0.1f) {
            return scale * Anim::Ease::OutBack(age / 0.1f);
        }
        return scale;
    }
};

// ============================================================================
// Enhanced HUD Class
// ============================================================================
class EnhancedHUD {
public:
    EnhancedHUD()
        : screenWidth(1280)
        , screenHeight(720)
        , showCrosshair(true)
        , showDebug(false)
        , targetHealth(100)
        , displayedHealth(100)
        , targetAmmo(30)
        , displayedAmmo(30)
        , damageVignetteIntensity(0)
        , lowHealthPulse(1.5f, 0.85f, 1.15f)
        , ammoLowPulse(3.0f, 0.9f, 1.1f)
        , crosshairSpread(0)
        , targetCrosshairSpread(0)
    {}
    
    void initialize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
    }
    
    void resize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
    }
    
    void update(float deltaTime) {
        // Smooth health bar animation
        displayedHealth = Anim::lerp(displayedHealth, targetHealth, deltaTime * 8.0f);
        displayedAmmo = Anim::lerp(displayedAmmo, targetAmmo, deltaTime * 12.0f);
        
        // Update damage vignette
        if (damageVignetteIntensity > 0) {
            damageVignetteIntensity -= deltaTime * 2.0f;
            if (damageVignetteIntensity < 0) damageVignetteIntensity = 0;
        }
        
        // Update damage indicators
        for (auto& indicator : damageIndicators) {
            indicator.age += deltaTime;
        }
        damageIndicators.erase(
            std::remove_if(damageIndicators.begin(), damageIndicators.end(),
                          [](const DamageIndicator& d) { return !d.isAlive(); }),
            damageIndicators.end()
        );
        
        // Update hit markers
        for (auto& marker : hitMarkers) {
            marker.age += deltaTime;
        }
        hitMarkers.erase(
            std::remove_if(hitMarkers.begin(), hitMarkers.end(),
                          [](const HitMarker& h) { return !h.isAlive(); }),
            hitMarkers.end()
        );
        
        // Update kill feed
        for (auto& entry : killFeed) {
            entry.age += deltaTime;
        }
        killFeed.erase(
            std::remove_if(killFeed.begin(), killFeed.end(),
                          [](const KillFeedEntry& k) { return !k.isAlive(); }),
            killFeed.end()
        );
        
        // Update floating texts
        for (auto& text : floatingTexts) {
            text.update(deltaTime);
        }
        floatingTexts.erase(
            std::remove_if(floatingTexts.begin(), floatingTexts.end(),
                          [](const FloatingText& f) { return !f.isAlive(); }),
            floatingTexts.end()
        );
        
        // Update messages
        for (auto& msg : messages) {
            msg.age += deltaTime;
        }
        messages.erase(
            std::remove_if(messages.begin(), messages.end(),
                          [](const HUDMessage& m) { return !m.isAlive(); }),
            messages.end()
        );
        
        // Update effects
        lowHealthPulse.update(deltaTime);
        ammoLowPulse.update(deltaTime);
        
        // Smooth crosshair spread
        crosshairSpread = Anim::lerp(crosshairSpread, targetCrosshairSpread, deltaTime * 15.0f);
    }
    
    void render(const Player* player) {
        Renderer& renderer = Renderer::instance();
        renderer.begin2D();
        
        // Draw screen effects first (behind everything)
        drawScreenEffects();
        
        // Draw damage indicators
        drawDamageIndicators();
        
        // Draw crosshair with hit markers
        if (showCrosshair) {
            drawCrosshair();
            drawHitMarkers();
        }
        
        // Draw HUD elements
        if (player) {
            setTargetHealth((float)player->getHealth());
            setTargetAmmo((float)player->getWeapon().getCurrentAmmo());
            
            drawHealthBar(player);
            drawAmmoDisplay(player);
            drawWeaponInfo(player);
            drawScore(player);
        }
        
        // Draw kill feed
        drawKillFeed();
        
        // Draw floating texts
        drawFloatingTexts();
        
        // Draw messages
        drawMessages();
        
        // Draw minimap placeholder
        drawMinimap();
        
        // Debug info
        if (showDebug && player) {
            drawDebugInfo(player);
        }
        
        renderer.end2D();
    }
    
    // ========================================================================
    // Events
    // ========================================================================
    void onDamage(float angle, float intensity = 1.0f) {
        damageIndicators.push_back(DamageIndicator(angle, intensity));
        damageVignetteIntensity = fminf(1.0f, damageVignetteIntensity + intensity * 0.5f);
    }
    
    void onHit(bool isKill = false) {
        hitMarkers.push_back(HitMarker(isKill));
    }
    
    void onKill(const std::string& enemyName) {
        killFeed.insert(killFeed.begin(), 
            KillFeedEntry("Killed " + enemyName, Math::Color(1.0f, 0.3f, 0.3f)));
        if (killFeed.size() > 5) killFeed.pop_back();
    }
    
    void addFloatingText(const std::string& text, const Math::Vector2& pos, const Math::Color& color) {
        floatingTexts.push_back(FloatingText(text, pos, color));
    }
    
    void addScorePopup(int score, const Math::Vector2& worldPos) {
        Math::Vector2 screenPos(screenWidth * 0.5f + (rand() % 100 - 50), 
                                screenHeight * 0.4f);
        Math::Color color = score > 50 ? Math::Color::yellow() : Math::Color::white();
        FloatingText ft("+" + std::to_string(score), screenPos, color);
        ft.scale = score > 50 ? 1.5f : 1.0f;
        floatingTexts.push_back(ft);
    }
    
    void setCrosshairSpread(float spread) {
        targetCrosshairSpread = spread;
    }
    
    // ========================================================================
    // Message System
    // ========================================================================
    struct HUDMessage {
        std::string text;
        Math::Color color;
        float lifetime;
        float age;
        bool isLarge;
        
        HUDMessage(const std::string& t, const Math::Color& c, float life = 3.0f, bool large = false)
            : text(t), color(c), lifetime(life), age(0), isLarge(large) {}
        
        bool isAlive() const { return age < lifetime; }
        float getAlpha() const {
            if (age > lifetime - 0.5f) {
                return (lifetime - age) / 0.5f;
            }
            if (age < 0.2f) {
                return Anim::Ease::OutQuad(age / 0.2f);
            }
            return 1.0f;
        }
    };
    
    void addMessage(const std::string& text, const Math::Color& color = Math::Color::white()) {
        messages.push_back(HUDMessage(text, color));
    }
    
    void showLevelMessage(const std::string& levelName) {
        messages.insert(messages.begin(), HUDMessage(levelName, Math::Color::cyan(), 5.0f, true));
    }
    
    void showPickupMessage(const std::string& item) {
        addMessage("+ " + item, Math::Color::green());
    }
    
    void showKillMessage() {
        onKill("Enemy");
    }
    
    void showDamageMessage(int damage) {
        addMessage("-" + std::to_string(damage) + " HP", Math::Color(1.0f, 0.3f, 0.3f));
    }
    
    // ========================================================================
    // Setters & Getters
    // ========================================================================
    void setShowCrosshair(bool show) { showCrosshair = show; }
    void setShowDebug(bool show) { showDebug = show; }
    void toggleDebug() { showDebug = !showDebug; }
    bool isShowDebug() const { return showDebug; }
    
    void setTargetHealth(float h) { targetHealth = h; }
    void setTargetAmmo(float a) { targetAmmo = a; }
    
private:
    // ========================================================================
    // Drawing Functions
    // ========================================================================
    void drawScreenEffects() {
        // Damage vignette
        if (damageVignetteIntensity > 0.01f) {
            drawVignette(Math::Color(0.8f, 0.0f, 0.0f, damageVignetteIntensity * 0.6f));
        }
        
        // Low health vignette
        if (targetHealth < 30) {
            float intensity = (30.0f - targetHealth) / 30.0f;
            float pulse = lowHealthPulse.getAlpha(0.3f, 0.6f);
            drawVignette(Math::Color(0.5f, 0.0f, 0.0f, intensity * pulse * 0.4f));
        }
    }
    
    void drawVignette(const Math::Color& color) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        
        float cx = screenWidth * 0.5f;
        float cy = screenHeight * 0.5f;
        float radius = sqrtf(cx * cx + cy * cy);
        
        // Draw radial gradient using triangle fan
        int segments = 32;
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(0, 0, 0, 0); // Center transparent
        glVertex2f(cx, cy);
        
        for (int i = 0; i <= segments; i++) {
            float angle = (float)i / segments * Math::PI * 2.0f;
            float x = cx + cosf(angle) * radius;
            float y = cy + sinf(angle) * radius;
            glColor4f(color.r, color.g, color.b, color.a);
            glVertex2f(x, y);
        }
        glEnd();
    }
    
    void drawDamageIndicators() {
        float cx = screenWidth * 0.5f;
        float cy = screenHeight * 0.5f;
        float indicatorDist = 100.0f;
        float indicatorSize = 40.0f;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);
        
        for (const auto& indicator : damageIndicators) {
            float alpha = indicator.getAlpha();
            float angle = indicator.angle;
            
            float x = cx + cosf(angle) * indicatorDist;
            float y = cy + sinf(angle) * indicatorDist;
            
            glPushMatrix();
            glTranslatef(x, y, 0);
            glRotatef(angle * Math::RAD_TO_DEG + 90, 0, 0, 1);
            
            // Draw arrow pointing toward damage source
            glColor4f(1.0f, 0.0f, 0.0f, alpha * 0.8f);
            glBegin(GL_TRIANGLES);
            glVertex2f(0, -indicatorSize * 0.5f);
            glVertex2f(-indicatorSize * 0.3f, indicatorSize * 0.3f);
            glVertex2f(indicatorSize * 0.3f, indicatorSize * 0.3f);
            glEnd();
            
            glPopMatrix();
        }
    }
    
    void drawCrosshair() {
        float cx = screenWidth * 0.5f;
        float cy = screenHeight * 0.5f;
        float baseSize = 10.0f;
        float gap = 4.0f + crosshairSpread * 20.0f;
        float thickness = 2.0f;
        
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(thickness);
        
        // Shadow
        glColor4f(0, 0, 0, 0.5f);
        drawCrosshairLines(cx + 1, cy + 1, baseSize, gap);
        
        // Main crosshair (dynamic color based on health/state)
        Math::Color crossColor = Math::Color::white();
        if (targetHealth < 30) {
            crossColor = Math::Color(1.0f, 0.5f, 0.5f);
        }
        
        glColor4f(crossColor.r, crossColor.g, crossColor.b, 0.9f);
        drawCrosshairLines(cx, cy, baseSize, gap);
        
        // Center dot
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        glColor4f(1, 1, 1, 0.8f);
        glVertex2f(cx, cy);
        glEnd();
    }
    
    void drawCrosshairLines(float cx, float cy, float size, float gap) {
        glBegin(GL_LINES);
        // Top
        glVertex2f(cx, cy - gap);
        glVertex2f(cx, cy - gap - size);
        // Bottom
        glVertex2f(cx, cy + gap);
        glVertex2f(cx, cy + gap + size);
        // Left
        glVertex2f(cx - gap, cy);
        glVertex2f(cx - gap - size, cy);
        // Right
        glVertex2f(cx + gap, cy);
        glVertex2f(cx + gap + size, cy);
        glEnd();
    }
    
    void drawHitMarkers() {
        if (hitMarkers.empty()) return;
        
        float cx = screenWidth * 0.5f;
        float cy = screenHeight * 0.5f;
        
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glLineWidth(2.0f);
        
        for (const auto& marker : hitMarkers) {
            float scale = marker.getScale();
            float alpha = marker.getAlpha();
            float size = 15.0f * scale;
            float offset = 8.0f * scale;
            
            Math::Color color = marker.isKill ? Math::Color(1.0f, 0.2f, 0.2f) : Math::Color::white();
            glColor4f(color.r, color.g, color.b, alpha);
            
            glBegin(GL_LINES);
            // X pattern
            glVertex2f(cx - offset, cy - offset);
            glVertex2f(cx - offset - size * 0.5f, cy - offset - size * 0.5f);
            
            glVertex2f(cx + offset, cy - offset);
            glVertex2f(cx + offset + size * 0.5f, cy - offset - size * 0.5f);
            
            glVertex2f(cx - offset, cy + offset);
            glVertex2f(cx - offset - size * 0.5f, cy + offset + size * 0.5f);
            
            glVertex2f(cx + offset, cy + offset);
            glVertex2f(cx + offset + size * 0.5f, cy + offset + size * 0.5f);
            glEnd();
        }
    }
    
    void drawHealthBar(const Player* player) {
        float barX = 30;
        float barY = screenHeight - 70;
        float barWidth = 220;
        float barHeight = 24;
        
        float healthPercent = displayedHealth / (float)player->getMaxHealth();
        float actualPercent = (float)player->getHealth() / (float)player->getMaxHealth();
        
        // Apply pulse for low health
        float pulseScale = 1.0f;
        if (actualPercent < 0.3f) {
            pulseScale = lowHealthPulse.getScale();
        }
        
        // Background panel
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        drawRoundedRect(barX - 10, barY - 10, barWidth + 20, barHeight + 45, 5);
        
        // Health bar background
        glColor4f(0.2f, 0.0f, 0.0f, 0.9f);
        drawRect(barX, barY, barWidth, barHeight);
        
        // Delayed health (shows recent damage in different color)
        if (displayedHealth > player->getHealth()) {
            float delayedWidth = barWidth * (displayedHealth / player->getMaxHealth());
            glColor4f(0.8f, 0.4f, 0.0f, 0.8f);
            drawRect(barX, barY, delayedWidth, barHeight);
        }
        
        // Current health
        float healthWidth = barWidth * actualPercent * pulseScale;
        
        // Health color gradient (green -> yellow -> red)
        Math::Color healthColor;
        if (actualPercent > 0.6f) {
            healthColor = Math::Color(0.2f, 0.9f, 0.2f);
        } else if (actualPercent > 0.3f) {
            float t = (actualPercent - 0.3f) / 0.3f;
            healthColor = Anim::lerp(Math::Color(1.0f, 0.8f, 0.0f), Math::Color(0.2f, 0.9f, 0.2f), t);
        } else {
            float t = actualPercent / 0.3f;
            healthColor = Anim::lerp(Math::Color(1.0f, 0.1f, 0.1f), Math::Color(1.0f, 0.8f, 0.0f), t);
        }
        
        glColor4f(healthColor.r, healthColor.g, healthColor.b, 0.95f);
        drawRect(barX, barY, healthWidth, barHeight);
        
        // Shine effect
        glColor4f(1, 1, 1, 0.15f);
        drawRect(barX, barY, healthWidth, barHeight * 0.4f);
        
        // Border
        glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
        glLineWidth(2.0f);
        drawRectOutline(barX, barY, barWidth, barHeight);
        
        // Health text
        char buf[32];
        snprintf(buf, sizeof(buf), "%d / %d", player->getHealth(), player->getMaxHealth());
        
        // Shadow
        glColor4f(0, 0, 0, 0.8f);
        drawText(buf, barX + 3, barY + barHeight + 18);
        // Text
        glColor4f(1, 1, 1, 1);
        drawText(buf, barX + 2, barY + barHeight + 17);
        
        // Health icon with pulse
        glColor4f(1.0f, 0.3f, 0.3f, 1);
        drawText("+", barX - 18, barY + 6);
    }
    
    void drawAmmoDisplay(const Player* player) {
        float x = screenWidth - 200;
        float y = screenHeight - 70;
        
        const auto& weapon = player->getWeapon();
        int currentAmmo = weapon.getCurrentAmmo();
        int reserveAmmo = weapon.getReserveAmmo();
        int maxAmmo = weapon.getMaxAmmo();
        
        // Background panel
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        drawRoundedRect(x - 15, y - 15, 195, 80, 5);
        
        // Ammo count with color based on amount
        char buf[64];
        snprintf(buf, sizeof(buf), "%d", currentAmmo);
        
        Math::Color ammoColor = Math::Color::white();
        float scale = 1.0f;
        
        if (currentAmmo == 0) {
            ammoColor = Math::Color(1.0f, 0.2f, 0.2f);
            scale = ammoLowPulse.getScale();
        } else if (currentAmmo <= maxAmmo * 0.3f) {
            ammoColor = Math::Color(1.0f, 0.7f, 0.2f);
        }
        
        glColor4f(ammoColor.r, ammoColor.g, ammoColor.b, 1);
        drawTextLarge(buf, x + 5, y + 20);
        
        // Reserve ammo
        snprintf(buf, sizeof(buf), "/ %d", reserveAmmo);
        glColor4f(0.6f, 0.6f, 0.6f, 1);
        drawText(buf, x + 70, y + 15);
        
        // Ammo bar
        float barWidth = 150;
        float barHeight = 6;
        float barY = y + 35;
        
        glColor4f(0.2f, 0.2f, 0.2f, 0.9f);
        drawRect(x, barY, barWidth, barHeight);
        
        float ammoPercent = (float)currentAmmo / maxAmmo;
        glColor4f(ammoColor.r * 0.8f, ammoColor.g * 0.8f, ammoColor.b * 0.8f, 0.9f);
        drawRect(x, barY, barWidth * ammoPercent, barHeight);
        
        // Reload indicator
        if (weapon.getIsReloading()) {
            float progress = weapon.getReloadProgress();
            
            glColor4f(1, 1, 0, 0.3f);
            drawRect(x, barY - 15, barWidth * progress, 10);
            
            glColor4f(1, 1, 0, 1);
            drawText("RELOADING", x, barY - 5);
        }
        
        // Label
        glColor4f(0.5f, 0.5f, 0.5f, 1);
        drawText("AMMO", x, y + 55);
    }
    
    void drawWeaponInfo(const Player* player) {
        // Weapon name in bottom right
        float x = screenWidth - 150;
        float y = screenHeight - 110;
        
        const char* weaponName = "ASSAULT RIFLE";
        
        glColor4f(0.7f, 0.7f, 0.7f, 0.8f);
        drawText(weaponName, x, y);
    }
    
    void drawScore(const Player* player) {
        float x = screenWidth * 0.5f;
        float y = 30;
        
        char buf[32];
        snprintf(buf, sizeof(buf), "SCORE: %d", player->getScore());
        
        // Center
        float textWidth = strlen(buf) * 9.0f;
        x -= textWidth * 0.5f;
        
        // Background
        glColor4f(0, 0, 0, 0.5f);
        drawRoundedRect(x - 15, y - 8, textWidth + 30, 30, 5);
        
        glColor4f(1, 1, 1, 0.95f);
        drawText(buf, x, y + 10);
    }
    
    void drawKillFeed() {
        float x = screenWidth - 250;
        float y = 50;
        
        for (const auto& entry : killFeed) {
            float alpha = entry.getAlpha();
            
            // Background
            glColor4f(0, 0, 0, alpha * 0.6f);
            drawRoundedRect(x - 10, y - 5, 240, 25, 3);
            
            // Text
            glColor4f(entry.color.r, entry.color.g, entry.color.b, alpha);
            drawText(entry.text, x, y + 10);
            
            y += 30;
        }
    }
    
    void drawFloatingTexts() {
        for (const auto& ft : floatingTexts) {
            float alpha = ft.getAlpha();
            float scale = ft.getScale();
            
            glColor4f(ft.color.r, ft.color.g, ft.color.b, alpha);
            // Simple scaling by adjusting position slightly
            drawText(ft.text, ft.position.x, ft.position.y);
        }
    }
    
    void drawMessages() {
        float y = screenHeight * 0.35f;
        
        for (const auto& msg : messages) {
            float alpha = msg.getAlpha();
            
            // Center horizontally
            float textWidth = msg.text.length() * (msg.isLarge ? 12.0f : 9.0f);
            float x = (screenWidth - textWidth) * 0.5f;
            
            if (msg.isLarge) {
                // Large level title style
                glColor4f(0, 0, 0, alpha * 0.7f);
                drawRoundedRect(x - 20, y - 15, textWidth + 40, 45, 8);
                
                glColor4f(msg.color.r, msg.color.g, msg.color.b, alpha);
                drawTextLarge(msg.text, x, y + 15);
            } else {
                glColor4f(msg.color.r, msg.color.g, msg.color.b, alpha);
                drawText(msg.text, x, y);
            }
            
            y += msg.isLarge ? 50 : 22;
        }
    }
    
    void drawMinimap() {
        // Minimap placeholder in top-left
        float size = 120;
        float x = 20;
        float y = 20;
        
        // Background
        glColor4f(0, 0, 0, 0.6f);
        drawRoundedRect(x, y, size, size, 5);
        
        // Border
        glColor4f(0.5f, 0.5f, 0.5f, 0.8f);
        glLineWidth(1.0f);
        drawRectOutline(x, y, size, size);
        
        // Player indicator (center)
        float cx = x + size * 0.5f;
        float cy = y + size * 0.5f;
        
        glColor4f(0, 1, 0, 1);
        glPointSize(6.0f);
        glBegin(GL_POINTS);
        glVertex2f(cx, cy);
        glEnd();
        
        // Direction indicator
        glBegin(GL_LINES);
        glVertex2f(cx, cy);
        glVertex2f(cx, cy - 10);
        glEnd();
    }
    
    void drawDebugInfo(const Player* player) {
        float x = 10;
        float y = 160;
        
        glColor4f(0, 1, 0, 0.9f);
        
        char buf[128];
        
        // Position
        Math::Vector3 pos = player->getPosition();
        snprintf(buf, sizeof(buf), "Pos: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
        drawText(buf, x, y); y += 15;
        
        // Rotation
        snprintf(buf, sizeof(buf), "Yaw: %.1f", player->getYaw());
        drawText(buf, x, y); y += 15;
        
        // State
        const char* stateStr = "Unknown";
        switch (player->getState()) {
            case PlayerState::Idle: stateStr = "Idle"; break;
            case PlayerState::Walking: stateStr = "Walking"; break;
            case PlayerState::Running: stateStr = "Running"; break;
            case PlayerState::Jumping: stateStr = "Jumping"; break;
            case PlayerState::Falling: stateStr = "Falling"; break;
            case PlayerState::Dead: stateStr = "Dead"; break;
        }
        snprintf(buf, sizeof(buf), "State: %s", stateStr);
        drawText(buf, x, y); y += 15;
        
        // Camera mode
        const char* camMode = player->getCamera().isFirstPerson() ? "FPS" : "TPS";
        snprintf(buf, sizeof(buf), "Camera: %s", camMode);
        drawText(buf, x, y); y += 15;
        
        // Grounded
        snprintf(buf, sizeof(buf), "Grounded: %s", player->isOnGround() ? "Yes" : "No");
        drawText(buf, x, y); y += 15;
    }
    
    // ========================================================================
    // Helper Drawing Functions
    // ========================================================================
    void drawRect(float x, float y, float w, float h) {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }
    
    void drawRoundedRect(float x, float y, float w, float h, float radius) {
        // Simple approximation with regular rect for now
        // A proper implementation would use GL_TRIANGLE_FAN for corners
        drawRect(x, y, w, h);
    }
    
    void drawRectOutline(float x, float y, float w, float h) {
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }
    
    void drawText(const std::string& text, float x, float y) {
        glRasterPos2f(x, y);
        for (char c : text) {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
        }
    }
    
    void drawTextLarge(const std::string& text, float x, float y) {
        glRasterPos2f(x, y);
        for (char c : text) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }
    
    // ========================================================================
    // Member Variables
    // ========================================================================
    int screenWidth;
    int screenHeight;
    bool showCrosshair;
    bool showDebug;
    
    // Animated values
    float targetHealth;
    float displayedHealth;
    float targetAmmo;
    float displayedAmmo;
    
    // Screen effects
    float damageVignetteIntensity;
    
    // Effects
    Anim::PulseEffect lowHealthPulse;
    Anim::PulseEffect ammoLowPulse;
    
    // Crosshair
    float crosshairSpread;
    float targetCrosshairSpread;
    
    // Indicators
    std::vector<DamageIndicator> damageIndicators;
    std::vector<HitMarker> hitMarkers;
    std::vector<KillFeedEntry> killFeed;
    std::vector<FloatingText> floatingTexts;
    std::vector<HUDMessage> messages;
};

} // namespace Doomers
