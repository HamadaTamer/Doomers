/**
 * DOOMERS - Damage System
 * 
 * Professional damage handling with:
 * - Health management
 * - Damage flash effects
 * - Invincibility frames
 * - Death callbacks
 * - Armor system
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Animation.hpp"
#include <functional>
#include <string>

namespace Doomers {

// ============================================================================
// Damage Types
// ============================================================================
enum class DamageType {
    Generic,
    Bullet,
    Explosion,
    Fire,
    Acid,
    Melee,
    Fall,
    Environmental
};

// ============================================================================
// Damage Info
// ============================================================================
struct DamageInfo {
    float amount = 0;
    DamageType type = DamageType::Generic;
    Math::Vector3 direction;
    Math::Vector3 hitPoint;
    float knockback = 0;
    bool ignoreArmor = false;
    std::string attackerId;
    
    DamageInfo() = default;
    DamageInfo(float amt, DamageType t = DamageType::Generic)
        : amount(amt), type(t) {}
};

// ============================================================================
// Damageable Component
// ============================================================================
class Damageable {
public:
    // Health
    float maxHealth = 100.0f;
    float currentHealth = 100.0f;
    
    // Armor
    float maxArmor = 100.0f;
    float currentArmor = 0.0f;
    float armorAbsorption = 0.5f;  // 50% damage to armor
    
    // Invincibility
    float invincibilityDuration = 0.5f;
    float invincibilityTimer = 0.0f;
    
    // Visual effects
    float damageFlashDuration = 0.15f;
    float damageFlashTimer = 0.0f;
    Math::Color damageFlashColor{1.0f, 0.0f, 0.0f, 0.5f};
    
    // Animation
    Anim::Spring healthSpring;
    float displayHealth = 100.0f;
    
    // Death
    bool alive = true;
    float deathTime = 0.0f;
    
    // Callbacks
    std::function<void(const DamageInfo&)> onDamaged;
    std::function<void()> onDeath;
    std::function<void()> onHeal;
    
    Damageable() {
        healthSpring.stiffness = 150.0f;
        healthSpring.damping = 15.0f;
    }
    
    void init(float maxHp = 100.0f) {
        maxHealth = maxHp;
        currentHealth = maxHp;
        displayHealth = maxHp;
        currentArmor = 0.0f;
        alive = true;
        invincibilityTimer = 0.0f;
        damageFlashTimer = 0.0f;
    }
    
    void update(float dt) {
        // Update timers
        if (invincibilityTimer > 0) {
            invincibilityTimer -= dt;
        }
        
        if (damageFlashTimer > 0) {
            damageFlashTimer -= dt;
        }
        
        // Smooth health display
        healthSpring.target = currentHealth;
        healthSpring.update(dt);
        displayHealth = healthSpring.position;
    }
    
    // Apply damage with armor consideration
    float takeDamage(const DamageInfo& info) {
        if (!alive) return 0;
        if (isInvincible()) return 0;
        
        float damage = info.amount;
        float actualDamage = damage;
        
        // Apply armor if available
        if (currentArmor > 0 && !info.ignoreArmor) {
            float armorDamage = damage * armorAbsorption;
            float healthDamage = damage * (1.0f - armorAbsorption);
            
            if (currentArmor >= armorDamage) {
                currentArmor -= armorDamage;
                actualDamage = healthDamage;
            } else {
                // Armor depleted
                float remaining = armorDamage - currentArmor;
                currentArmor = 0;
                actualDamage = healthDamage + remaining;
            }
        }
        
        // Apply health damage
        currentHealth -= actualDamage;
        
        // Trigger effects
        damageFlashTimer = damageFlashDuration;
        invincibilityTimer = invincibilityDuration;
        
        if (onDamaged) {
            onDamaged(info);
        }
        
        // Check death
        if (currentHealth <= 0) {
            currentHealth = 0;
            die();
        }
        
        return actualDamage;
    }
    
    // Simple damage overload
    float takeDamage(float amount, DamageType type = DamageType::Generic) {
        return takeDamage(DamageInfo(amount, type));
    }
    
    void heal(float amount) {
        if (!alive) return;
        
        float oldHealth = currentHealth;
        currentHealth = Math::min(currentHealth + amount, maxHealth);
        
        if (currentHealth > oldHealth && onHeal) {
            onHeal();
        }
    }
    
    void addArmor(float amount) {
        currentArmor = Math::min(currentArmor + amount, maxArmor);
    }
    
    void die() {
        if (!alive) return;
        
        alive = false;
        deathTime = 0;
        
        if (onDeath) {
            onDeath();
        }
    }
    
    void revive(float healthPercent = 1.0f) {
        alive = true;
        currentHealth = maxHealth * healthPercent;
        displayHealth = currentHealth;
        currentArmor = 0;
        invincibilityTimer = 1.0f;  // Brief invincibility on respawn
    }
    
    // Queries
    bool isAlive() const { return alive; }
    bool isInvincible() const { return invincibilityTimer > 0; }
    bool isFlashing() const { return damageFlashTimer > 0; }
    
    float getHealthPercent() const { 
        return maxHealth > 0 ? currentHealth / maxHealth : 0; 
    }
    
    float getArmorPercent() const { 
        return maxArmor > 0 ? currentArmor / maxArmor : 0; 
    }
    
    float getDisplayHealthPercent() const {
        return maxHealth > 0 ? displayHealth / maxHealth : 0;
    }
    
    // Rendering helpers
    float getRenderAlpha() const {
        if (!alive) return 0.5f;
        if (isInvincible()) {
            // Flicker effect
            return (sinf(invincibilityTimer * 30.0f) > 0) ? 1.0f : 0.3f;
        }
        return 1.0f;
    }
    
    Math::Color getFlashColor() const {
        if (isFlashing()) {
            float t = damageFlashTimer / damageFlashDuration;
            return Math::Color{
                damageFlashColor.r,
                damageFlashColor.g,
                damageFlashColor.b,
                damageFlashColor.a * t
            };
        }
        return Math::Color{0, 0, 0, 0};
    }
};

// ============================================================================
// Damage Zone (environmental hazards)
// ============================================================================
class DamageZone {
public:
    Math::Vector3 position;
    Math::Vector3 size;
    float damagePerSecond = 10.0f;
    DamageType damageType = DamageType::Environmental;
    bool active = true;
    
    // Tick damage
    float tickRate = 0.5f;
    float tickTimer = 0;
    
    DamageZone() = default;
    DamageZone(Math::Vector3 pos, Math::Vector3 sz, float dps)
        : position(pos), size(sz), damagePerSecond(dps) {}
    
    void update(float dt) {
        if (active) {
            tickTimer += dt;
        }
    }
    
    bool shouldApplyDamage() {
        if (tickTimer >= tickRate) {
            tickTimer = 0;
            return true;
        }
        return false;
    }
    
    float getDamageAmount() const {
        return damagePerSecond * tickRate;
    }
    
    bool containsPoint(const Math::Vector3& point) const {
        Math::Vector3 halfSize = size * 0.5f;
        Math::Vector3 min = position - halfSize;
        Math::Vector3 max = position + halfSize;
        
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
};

// ============================================================================
// Knockback Helper
// ============================================================================
struct KnockbackInfo {
    Math::Vector3 direction;
    float force = 0;
    float duration = 0.3f;
    
    Math::Vector3 getVelocity() const {
        return direction.normalized() * force;
    }
};

inline KnockbackInfo calculateKnockback(
    const Math::Vector3& attackerPos,
    const Math::Vector3& victimPos,
    float force
) {
    KnockbackInfo info;
    info.direction = (victimPos - attackerPos).normalized();
    info.force = force;
    return info;
}

} // namespace Doomers
