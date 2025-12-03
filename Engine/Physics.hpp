/**
 * DOOMERS - Physics and Collision System
 * 
 * Handles collision detection and response for:
 * - Player movement and wall collision
 * - Raycasting for weapons
 * - Trigger volumes
 * - Basic physics (gravity, jumping)
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include "Entity.hpp"
#include <vector>

namespace Doomers {

// ============================================================================
// Collision Layers
// ============================================================================
enum class CollisionLayer {
    None        = 0,
    World       = 1 << 0,
    Player      = 1 << 1,
    Enemy       = 1 << 2,
    Pickup      = 1 << 3,
    Projectile  = 1 << 4,
    Trigger     = 1 << 5,
    All         = 0xFFFFFFFF
};

inline CollisionLayer operator|(CollisionLayer a, CollisionLayer b) {
    return static_cast<CollisionLayer>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool operator&(CollisionLayer a, CollisionLayer b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

// ============================================================================
// Collider - Base collision shape
// ============================================================================
struct Collider {
    enum class Type {
        None,
        AABB,
        Sphere,
        Capsule
    };
    
    Type type;
    CollisionLayer layer;
    bool isTrigger;
    bool isStatic;
    Entity* owner;
    
    // Shape data
    Math::AABB aabb;
    Math::Sphere sphere;
    float capsuleRadius;
    float capsuleHeight;
    
    Collider()
        : type(Type::None)
        , layer(CollisionLayer::World)
        , isTrigger(false)
        , isStatic(true)
        , owner(nullptr)
        , capsuleRadius(0.5f)
        , capsuleHeight(2.0f)
    {}
    
    static Collider createAABB(const Math::Vector3& min, const Math::Vector3& max, 
                               CollisionLayer lay = CollisionLayer::World, bool trigger = false) {
        Collider c;
        c.type = Type::AABB;
        c.aabb = Math::AABB(min, max);
        c.layer = lay;
        c.isTrigger = trigger;
        return c;
    }
    
    static Collider createSphere(const Math::Vector3& center, float radius,
                                 CollisionLayer lay = CollisionLayer::World, bool trigger = false) {
        Collider c;
        c.type = Type::Sphere;
        c.sphere = Math::Sphere(center, radius);
        c.layer = lay;
        c.isTrigger = trigger;
        return c;
    }
    
    static Collider createCapsule(const Math::Vector3& center, float radius, float height,
                                  CollisionLayer lay = CollisionLayer::Player, bool trigger = false) {
        Collider c;
        c.type = Type::Capsule;
        c.sphere.center = center;
        c.capsuleRadius = radius;
        c.capsuleHeight = height;
        c.layer = lay;
        c.isTrigger = trigger;
        c.isStatic = false;
        return c;
    }
    
    Math::AABB getWorldBounds(const Math::Vector3& position = Math::Vector3::zero()) const {
        switch (type) {
            case Type::AABB:
                return Math::AABB(aabb.min + position, aabb.max + position);
            case Type::Sphere:
                return Math::AABB::fromCenterSize(sphere.center + position, 
                                                  Math::Vector3(sphere.radius * 2));
            case Type::Capsule: {
                float halfHeight = capsuleHeight * 0.5f;
                return Math::AABB(
                    position + Math::Vector3(-capsuleRadius, 0, -capsuleRadius),
                    position + Math::Vector3(capsuleRadius, capsuleHeight, capsuleRadius)
                );
            }
            default:
                return Math::AABB();
        }
    }
};

// ============================================================================
// Raycast Result
// ============================================================================
struct RaycastHit {
    bool hit;
    Math::Vector3 point;
    Math::Vector3 normal;
    float distance;
    Entity* entity;
    Collider* collider;
    
    RaycastHit()
        : hit(false)
        , distance(std::numeric_limits<float>::max())
        , entity(nullptr)
        , collider(nullptr)
    {}
};

// ============================================================================
// Physics World - Manages all physics and collision
// ============================================================================
class PhysicsWorld {
public:
    static PhysicsWorld& instance() {
        static PhysicsWorld instance;
        return instance;
    }
    
    // ========================================================================
    // Collider Management
    // ========================================================================
    void addCollider(const Collider& collider) {
        colliders.push_back(collider);
    }
    
    void addCollider(const Math::AABB& aabb, CollisionLayer layer = CollisionLayer::World) {
        colliders.push_back(Collider::createAABB(aabb.min, aabb.max, layer));
    }
    
    void addWall(const Math::Vector3& min, const Math::Vector3& max) {
        addCollider(Math::AABB(min, max), CollisionLayer::World);
    }
    
    void addBox(const Math::Vector3& position, const Math::Vector3& size, 
                CollisionLayer layer = CollisionLayer::World, Entity* owner = nullptr) {
        Collider c = Collider::createAABB(
            position - size * 0.5f,
            position + size * 0.5f,
            layer
        );
        c.owner = owner;
        colliders.push_back(c);
    }
    
    void clearColliders() {
        colliders.clear();
    }
    
    void clearDynamicColliders() {
        colliders.erase(
            std::remove_if(colliders.begin(), colliders.end(), 
                          [](const Collider& c) { return !c.isStatic; }),
            colliders.end()
        );
    }
    
    // ========================================================================
    // Raycasting
    // ========================================================================
    bool raycast(const Math::Ray& ray, RaycastHit& hit, float maxDistance = 1000.0f,
                 CollisionLayer layerMask = CollisionLayer::All) {
        hit = RaycastHit();
        hit.distance = maxDistance;
        
        for (auto& collider : colliders) {
            if (!(collider.layer & layerMask)) continue;
            if (collider.isTrigger) continue;
            
            float tMin, tMax;
            bool intersects = false;
            Math::Vector3 localHitPoint;
            
            switch (collider.type) {
                case Collider::Type::AABB:
                    intersects = collider.aabb.intersectsRay(ray, tMin, tMax);
                    if (intersects && tMin >= 0 && tMin < hit.distance) {
                        hit.hit = true;
                        hit.distance = tMin;
                        hit.point = ray.getPoint(tMin);
                        hit.collider = &collider;
                        hit.entity = collider.owner;
                        // Calculate normal (simplified)
                        hit.normal = calculateAABBNormal(hit.point, collider.aabb);
                    }
                    break;
                    
                case Collider::Type::Sphere:
                    float t;
                    intersects = collider.sphere.intersectsRay(ray, t);
                    if (intersects && t >= 0 && t < hit.distance) {
                        hit.hit = true;
                        hit.distance = t;
                        hit.point = ray.getPoint(t);
                        hit.collider = &collider;
                        hit.entity = collider.owner;
                        hit.normal = (hit.point - collider.sphere.center).normalized();
                    }
                    break;
                    
                default:
                    break;
            }
        }
        
        return hit.hit;
    }
    
    // Raycast against entities
    bool raycastEntities(const Math::Ray& ray, const std::vector<Entity*>& entities,
                         RaycastHit& hit, float maxDistance = 1000.0f,
                         Entity* ignoreEntity = nullptr) {
        hit = RaycastHit();
        hit.distance = maxDistance;
        
        for (Entity* entity : entities) {
            if (!entity || !entity->isActive()) continue;
            if (entity == ignoreEntity) continue;
            
            Math::Sphere bounds = entity->getBoundingSphere();
            float t;
            
            if (bounds.intersectsRay(ray, t) && t >= 0 && t < hit.distance) {
                hit.hit = true;
                hit.distance = t;
                hit.point = ray.getPoint(t);
                hit.entity = entity;
                hit.normal = (hit.point - bounds.center).normalized();
            }
        }
        
        return hit.hit;
    }
    
    // ========================================================================
    // Collision Detection
    // ========================================================================
    bool checkCollision(const Math::AABB& aabb, CollisionLayer layerMask = CollisionLayer::World) {
        for (const auto& collider : colliders) {
            if (!(collider.layer & layerMask)) continue;
            if (collider.isTrigger) continue;
            
            if (collider.type == Collider::Type::AABB) {
                if (aabb.intersects(collider.aabb)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    bool checkCollision(const Math::Sphere& sphere, CollisionLayer layerMask = CollisionLayer::World) {
        for (const auto& collider : colliders) {
            if (!(collider.layer & layerMask)) continue;
            if (collider.isTrigger) continue;
            
            switch (collider.type) {
                case Collider::Type::AABB:
                    if (sphereAABBIntersect(sphere, collider.aabb)) {
                        return true;
                    }
                    break;
                case Collider::Type::Sphere:
                    if (sphere.intersects(collider.sphere)) {
                        return true;
                    }
                    break;
                default:
                    break;
            }
        }
        return false;
    }
    
    // ========================================================================
    // Player Movement with Collision
    // ========================================================================
    Math::Vector3 moveWithCollision(const Math::Vector3& currentPos, 
                                    const Math::Vector3& desiredMove,
                                    float radius, float height,
                                    CollisionLayer layerMask = CollisionLayer::World) {
        // Simple slide collision response
        Math::Vector3 newPos = currentPos + desiredMove;
        
        // Create capsule bounds as AABB for simplicity
        Math::AABB playerBounds(
            newPos + Math::Vector3(-radius, 0.1f, -radius),
            newPos + Math::Vector3(radius, height - 0.1f, radius)
        );
        
        // Check X movement
        Math::AABB testBoundsX(
            currentPos + Math::Vector3(-radius, 0.1f, -radius) + Math::Vector3(desiredMove.x, 0, 0),
            currentPos + Math::Vector3(radius, height - 0.1f, radius) + Math::Vector3(desiredMove.x, 0, 0)
        );
        
        bool blockedX = false;
        bool blockedZ = false;
        
        for (const auto& collider : colliders) {
            if (!(collider.layer & layerMask)) continue;
            if (collider.isTrigger) continue;
            if (collider.type != Collider::Type::AABB) continue;
            
            if (testBoundsX.intersects(collider.aabb)) {
                blockedX = true;
            }
        }
        
        // Check Z movement
        Math::AABB testBoundsZ(
            currentPos + Math::Vector3(-radius, 0.1f, -radius) + Math::Vector3(0, 0, desiredMove.z),
            currentPos + Math::Vector3(radius, height - 0.1f, radius) + Math::Vector3(0, 0, desiredMove.z)
        );
        
        for (const auto& collider : colliders) {
            if (!(collider.layer & layerMask)) continue;
            if (collider.isTrigger) continue;
            if (collider.type != Collider::Type::AABB) continue;
            
            if (testBoundsZ.intersects(collider.aabb)) {
                blockedZ = true;
            }
        }
        
        // Apply movement with sliding
        Math::Vector3 finalMove = desiredMove;
        if (blockedX) finalMove.x = 0;
        if (blockedZ) finalMove.z = 0;
        
        return currentPos + finalMove;
    }
    
    // Ground check for jumping/gravity
    float getGroundHeight(const Math::Vector3& position, float radius) {
        float groundY = 0.0f;
        
        // Cast ray downward
        Math::Ray downRay(position + Math::Vector3(0, 1.0f, 0), Math::Vector3(0, -1, 0));
        
        for (const auto& collider : colliders) {
            if (collider.type != Collider::Type::AABB) continue;
            if (collider.isTrigger) continue;
            
            // Check if position is above this collider
            if (position.x >= collider.aabb.min.x - radius && 
                position.x <= collider.aabb.max.x + radius &&
                position.z >= collider.aabb.min.z - radius && 
                position.z <= collider.aabb.max.z + radius) {
                
                if (collider.aabb.max.y > groundY && collider.aabb.max.y <= position.y + 0.5f) {
                    groundY = collider.aabb.max.y;
                }
            }
        }
        
        return groundY;
    }
    
    // ========================================================================
    // Overlap Tests
    // ========================================================================
    std::vector<Entity*> overlapSphere(const Math::Vector3& center, float radius,
                                        const std::vector<Entity*>& entities) {
        std::vector<Entity*> result;
        Math::Sphere testSphere(center, radius);
        
        for (Entity* entity : entities) {
            if (!entity || !entity->isActive()) continue;
            
            Math::Sphere entitySphere = entity->getBoundingSphere();
            if (testSphere.intersects(entitySphere)) {
                result.push_back(entity);
            }
        }
        
        return result;
    }
    
    // ========================================================================
    // Debug Rendering
    // ========================================================================
    void debugDraw() {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(1.0f);
        
        for (const auto& collider : colliders) {
            if (collider.type == Collider::Type::AABB) {
                if (collider.isTrigger) {
                    glColor3f(0.3f, 0.3f, 1.0f);
                } else {
                    glColor3f(0.0f, 1.0f, 0.0f);
                }
                drawAABBWireframe(collider.aabb);
            }
            else if (collider.type == Collider::Type::Sphere) {
                glColor3f(1.0f, 1.0f, 0.0f);
                drawSphereWireframe(collider.sphere);
            }
        }
        
        glEnable(GL_LIGHTING);
    }
    
    // Overload for compatibility with Renderer parameter
    template<typename T>
    void debugDraw(T& renderer) {
        debugDraw();
    }
    
private:
    PhysicsWorld() {}
    
    std::vector<Collider> colliders;
    
    // Helper functions
    bool sphereAABBIntersect(const Math::Sphere& sphere, const Math::AABB& aabb) {
        // Find closest point on AABB to sphere center
        Math::Vector3 closest;
        closest.x = std::max(aabb.min.x, std::min(sphere.center.x, aabb.max.x));
        closest.y = std::max(aabb.min.y, std::min(sphere.center.y, aabb.max.y));
        closest.z = std::max(aabb.min.z, std::min(sphere.center.z, aabb.max.z));
        
        float distSq = Math::Vector3::distanceSquared(sphere.center, closest);
        return distSq <= sphere.radius * sphere.radius;
    }
    
    Math::Vector3 calculateAABBNormal(const Math::Vector3& point, const Math::AABB& aabb) {
        Math::Vector3 center = aabb.center();
        Math::Vector3 extents = aabb.extents();
        Math::Vector3 local = point - center;
        
        // Find which face we hit
        float minDist = std::numeric_limits<float>::max();
        Math::Vector3 normal = Math::Vector3::up();
        
        float dx = extents.x - fabsf(local.x);
        float dy = extents.y - fabsf(local.y);
        float dz = extents.z - fabsf(local.z);
        
        if (dx < minDist) { minDist = dx; normal = Math::Vector3(local.x > 0 ? 1 : -1, 0, 0); }
        if (dy < minDist) { minDist = dy; normal = Math::Vector3(0, local.y > 0 ? 1 : -1, 0); }
        if (dz < minDist) { minDist = dz; normal = Math::Vector3(0, 0, local.z > 0 ? 1 : -1); }
        
        return normal;
    }
    
    void drawAABBWireframe(const Math::AABB& aabb) {
        Math::Vector3 corners[8] = {
            Math::Vector3(aabb.min.x, aabb.min.y, aabb.min.z),
            Math::Vector3(aabb.max.x, aabb.min.y, aabb.min.z),
            Math::Vector3(aabb.max.x, aabb.max.y, aabb.min.z),
            Math::Vector3(aabb.min.x, aabb.max.y, aabb.min.z),
            Math::Vector3(aabb.min.x, aabb.min.y, aabb.max.z),
            Math::Vector3(aabb.max.x, aabb.min.y, aabb.max.z),
            Math::Vector3(aabb.max.x, aabb.max.y, aabb.max.z),
            Math::Vector3(aabb.min.x, aabb.max.y, aabb.max.z)
        };
        
        glBegin(GL_LINES);
        // Bottom
        for (int i = 0; i < 4; ++i) {
            glVertex3f(corners[i].x, corners[i].y, corners[i].z);
            glVertex3f(corners[(i+1)%4].x, corners[(i+1)%4].y, corners[(i+1)%4].z);
        }
        // Top
        for (int i = 0; i < 4; ++i) {
            glVertex3f(corners[4+i].x, corners[4+i].y, corners[4+i].z);
            glVertex3f(corners[4+(i+1)%4].x, corners[4+(i+1)%4].y, corners[4+(i+1)%4].z);
        }
        // Vertical
        for (int i = 0; i < 4; ++i) {
            glVertex3f(corners[i].x, corners[i].y, corners[i].z);
            glVertex3f(corners[4+i].x, corners[4+i].y, corners[4+i].z);
        }
        glEnd();
    }
    
    void drawSphereWireframe(const Math::Sphere& sphere) {
        glPushMatrix();
        glTranslatef(sphere.center.x, sphere.center.y, sphere.center.z);
        glutWireSphere(sphere.radius, 12, 12);
        glPopMatrix();
    }
};

// Convenience accessor
inline PhysicsWorld& Physics() {
    return PhysicsWorld::instance();
}

} // namespace Doomers
