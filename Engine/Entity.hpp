/**
 * DOOMERS - Entity System
 * 
 * Base classes for all game objects: player, enemies, pickups, props
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include "ResourceManager.hpp"
#include "Renderer.hpp"

namespace Doomers {

// ============================================================================
// Entity Types
// ============================================================================
enum class EntityType {
    None,
    Player,
    Enemy,
    Pickup,
    Prop,
    Projectile,
    Trigger
};

// ============================================================================
// Entity - Base class for all game objects
// ============================================================================
class Entity {
public:
    Entity()
        : type(EntityType::None)
        , active(true)
        , visible(true)
        , position(Math::Vector3::zero())
        , rotation(0)
        , scale(Math::Vector3::one())
        , velocity(Math::Vector3::zero())
        , mesh(nullptr)
        , textureId(0)
    {
        static int nextId = 0;
        id = nextId++;
    }
    
    virtual ~Entity() {}
    
    // ========================================================================
    // Core Methods (Virtual)
    // ========================================================================
    virtual void update(float deltaTime) {
        // Default: apply velocity
        position += velocity * deltaTime;
    }
    
    virtual void render() {
        if (!visible || !mesh) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation, 0, 1, 0);
        glScalef(scale.x, scale.y, scale.z);
        
        if (textureId > 0) {
            mesh->drawWithTexture(textureId);
        } else {
            mesh->draw();
        }
        
        glPopMatrix();
    }
    
    virtual void onCollision(Entity* other) {}
    virtual void onTriggerEnter(Entity* other) {}
    virtual void onTriggerExit(Entity* other) {}
    virtual void takeDamage(int damage) {}
    virtual void kill() { active = false; }
    
    // ========================================================================
    // Getters/Setters
    // ========================================================================
    int getId() const { return id; }
    EntityType getType() const { return type; }
    bool isActive() const { return active; }
    bool isVisible() const { return visible; }
    
    Math::Vector3 getPosition() const { return position; }
    float getRotation() const { return rotation; }
    Math::Vector3 getScale() const { return scale; }
    Math::Vector3 getVelocity() const { return velocity; }
    
    void setPosition(const Math::Vector3& pos) { position = pos; }
    void setRotation(float rot) { rotation = rot; }
    void setScale(const Math::Vector3& s) { scale = s; }
    void setScale(float s) { scale = Math::Vector3(s, s, s); }
    void setVelocity(const Math::Vector3& vel) { velocity = vel; }
    void setActive(bool a) { active = a; }
    void setVisible(bool v) { visible = v; }
    
    void setMesh(Mesh* m) { mesh = m; }
    void setTexture(unsigned int tex) { textureId = tex; }
    
    // ========================================================================
    // Direction Helpers
    // ========================================================================
    Math::Vector3 getForward() const {
        float rad = rotation * Math::DEG_TO_RAD;
        return Math::Vector3(sinf(rad), 0, -cosf(rad));
    }
    
    Math::Vector3 getRight() const {
        float rad = rotation * Math::DEG_TO_RAD;
        return Math::Vector3(cosf(rad), 0, sinf(rad));
    }
    
    void lookAt(const Math::Vector3& target) {
        Math::Vector3 dir = target - position;
        dir.y = 0; // Only rotate on Y axis
        if (dir.lengthSquared() > 0.0001f) {
            dir.normalize();
            rotation = atan2f(dir.x, -dir.z) * Math::RAD_TO_DEG;
        }
    }
    
    // ========================================================================
    // Collision Helpers
    // ========================================================================
    Math::AABB getBounds() const {
        if (mesh && mesh->boundsComputed) {
            Math::AABB bounds = mesh->bounds;
            // Transform bounds by scale and position
            bounds.min = bounds.min * scale + position;
            bounds.max = bounds.max * scale + position;
            return bounds;
        }
        // Default bounds if no mesh
        float radius = 0.5f;
        return Math::AABB(
            position - Math::Vector3(radius, 0, radius),
            position + Math::Vector3(radius, 2.0f, radius)
        );
    }
    
    Math::Sphere getBoundingSphere() const {
        if (mesh && mesh->boundsComputed) {
            Math::Vector3 center = mesh->bounds.center() * scale + position;
            float radius = mesh->bounds.extents().length() * std::max({scale.x, scale.y, scale.z});
            return Math::Sphere(center, radius);
        }
        return Math::Sphere(position + Math::Vector3(0, 1, 0), 1.0f);
    }
    
    float distanceTo(const Entity* other) const {
        return Math::Vector3::distance(position, other->position);
    }
    
    float distanceTo(const Math::Vector3& point) const {
        return Math::Vector3::distance(position, point);
    }
    
protected:
    int id;
    EntityType type;
    bool active;
    bool visible;
    
    Math::Vector3 position;
    float rotation;
    Math::Vector3 scale;
    Math::Vector3 velocity;
    
    Mesh* mesh;
    unsigned int textureId;
};

// ============================================================================
// Pickup Types
// ============================================================================
enum class PickupType {
    None,
    Health,
    Ammo,
    Keycard
};

// ============================================================================
// Pickup - Collectible items
// ============================================================================
class Pickup : public Entity {
public:
    Pickup()
        : pickupType(PickupType::None)
        , value(0)
        , bobTimer(0)
        , spinSpeed(90.0f)
        , bobAmount(0.2f)
        , bobSpeed(2.0f)
        , baseY(0)
    {
        type = EntityType::Pickup;
    }
    
    void initialize(PickupType pType, int val, const Math::Vector3& pos) {
        pickupType = pType;
        value = val;
        position = pos;
        baseY = pos.y;
    }
    
    void update(float deltaTime) override {
        if (!active) return;
        
        // Spin animation
        rotation += spinSpeed * deltaTime;
        if (rotation >= 360.0f) rotation -= 360.0f;
        
        // Bob animation
        bobTimer += deltaTime * bobSpeed;
        position.y = baseY + sinf(bobTimer) * bobAmount;
    }
    
    void render() override {
        if (!visible || !mesh) return;
        
        // Add a glow effect based on type
        Math::Color glowColor;
        switch (pickupType) {
            case PickupType::Health: glowColor = Math::Color(0.2f, 1.0f, 0.3f); break;
            case PickupType::Ammo: glowColor = Math::Color(1.0f, 0.8f, 0.2f); break;
            case PickupType::Keycard: glowColor = Math::Color(0.3f, 0.5f, 1.0f); break;
            default: glowColor = Math::Color::white(); break;
        }
        
        // Draw the mesh
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation, 0, 1, 0);
        glScalef(scale.x, scale.y, scale.z);
        
        // Apply slight emissive color
        GLfloat emission[] = { glowColor.r * 0.3f, glowColor.g * 0.3f, glowColor.b * 0.3f, 1.0f };
        glMaterialfv(GL_FRONT, GL_EMISSION, emission);
        
        if (textureId > 0) {
            mesh->drawWithTexture(textureId);
        } else {
            glColor3f(glowColor.r, glowColor.g, glowColor.b);
            mesh->draw();
        }
        
        // Reset emission
        GLfloat noEmission[] = { 0, 0, 0, 1 };
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
        
        glPopMatrix();
    }
    
    void collect() {
        active = false;
        visible = false;
        // Play collection effect/sound here
    }
    
    PickupType getPickupType() const { return pickupType; }
    int getValue() const { return value; }
    
    // Convenience method to get type as string
    std::string getType() const {
        switch (pickupType) {
            case PickupType::Health: return "health";
            case PickupType::Ammo: return "ammo";
            case PickupType::Keycard: return "keycard";
            default: return "unknown";
        }
    }
    
private:
    PickupType pickupType;
    int value;
    float bobTimer;
    float spinSpeed;
    float bobAmount;
    float bobSpeed;
    float baseY;
};

// ============================================================================
// Prop - Static or dynamic environmental objects
// ============================================================================
class Prop : public Entity {
public:
    Prop()
        : isStatic(true)
        , isSolid(true)
        , health(-1) // -1 = indestructible
    {
        type = EntityType::Prop;
    }
    
    void setStatic(bool s) { isStatic = s; }
    void setSolid(bool s) { isSolid = s; }
    void setHealth(int h) { health = h; }
    
    bool getIsStatic() const { return isStatic; }
    bool getIsSolid() const { return isSolid; }
    int getHealth() const { return health; }
    
    void takeDamage(int damage) override {
        if (health < 0) return; // Indestructible
        
        health -= damage;
        if (health <= 0) {
            kill();
            // Spawn destruction particles here
        }
    }
    
private:
    bool isStatic;
    bool isSolid;
    int health;
};

// ============================================================================
// Trigger - Invisible collision zone for events
// ============================================================================
class Trigger : public Entity {
public:
    using TriggerCallback = std::function<void(Entity*)>;
    
    Trigger()
        : triggered(false)
        , oneShot(true)
        , radius(2.0f)
    {
        type = EntityType::Trigger;
        visible = false;
    }
    
    void setBounds(const Math::AABB& b) { bounds = b; }
    void setRadius(float r) { radius = r; }
    void setOneShot(bool o) { oneShot = o; }
    void setCallback(TriggerCallback cb) { callback = cb; }
    
    void checkTrigger(Entity* entity) {
        if (!active || (triggered && oneShot)) return;
        
        // Check if entity is within bounds
        float dist = distanceTo(entity);
        if (dist < radius) {
            triggered = true;
            if (callback) {
                callback(entity);
            }
        }
    }
    
    void reset() { triggered = false; }
    
private:
    bool triggered;
    bool oneShot;
    float radius;
    Math::AABB bounds;
    TriggerCallback callback;
};

// ============================================================================
// Projectile - Bullets, rockets, etc.
// ============================================================================
class Projectile : public Entity {
public:
    Projectile()
        : speed(50.0f)
        , damage(25)
        , lifetime(3.0f)
        , age(0)
        , owner(nullptr)
    {
        type = EntityType::Projectile;
    }
    
    void initialize(const Math::Vector3& pos, const Math::Vector3& dir, Entity* ownerEntity) {
        position = pos;
        velocity = dir.normalized() * speed;
        owner = ownerEntity;
        age = 0;
    }
    
    void update(float deltaTime) override {
        if (!active) return;
        
        position += velocity * deltaTime;
        age += deltaTime;
        
        if (age >= lifetime) {
            kill();
        }
    }
    
    void render() override {
        if (!visible) return;
        
        // Draw as a simple elongated sphere/cylinder in direction of travel
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        
        // Rotate to face velocity direction
        Math::Vector3 dir = velocity.normalized();
        float yaw = atan2f(dir.x, -dir.z) * Math::RAD_TO_DEG;
        float pitch = asinf(dir.y) * Math::RAD_TO_DEG;
        
        glRotatef(yaw, 0, 1, 0);
        glRotatef(-pitch, 1, 0, 0);
        
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0f, 0.9f, 0.3f); // Yellow/orange bullet
        
        // Draw bullet as scaled sphere
        glScalef(0.05f, 0.05f, 0.2f);
        glutSolidSphere(1.0f, 8, 8);
        
        glPopMatrix();
    }
    
    void setSpeed(float s) { speed = s; velocity = velocity.normalized() * speed; }
    void setDamage(int d) { damage = d; }
    void setLifetime(float l) { lifetime = l; }
    
    int getDamage() const { return damage; }
    Entity* getOwner() const { return owner; }
    
private:
    float speed;
    int damage;
    float lifetime;
    float age;
    Entity* owner;
};

} // namespace Doomers
