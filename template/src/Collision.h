#pragma once
// Collision.h - Collision detection for Doomers
// AABB, Sphere, and Ray collision detection

// Undefine Windows min/max macros to prevent conflicts
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "Vector3.h"
#include "GameConfig.h"
#include <cmath>
#include <algorithm>

namespace Collision {
    
    // Axis-Aligned Bounding Box
    struct AABB {
        Vector3 minPoint;
        Vector3 maxPoint;
        
        AABB() : minPoint(0, 0, 0), maxPoint(1, 1, 1) {}
        AABB(const Vector3& minP, const Vector3& maxP) : minPoint(minP), maxPoint(maxP) {}
        
        // Create AABB from center and half-extents
        static AABB fromCenter(const Vector3& center, const Vector3& halfExtents) {
            return AABB(center - halfExtents, center + halfExtents);
        }
        
        // Alias for compatibility
        static AABB fromCenterSize(const Vector3& center, const Vector3& halfExtents) {
            return fromCenter(center, halfExtents);
        }
        
        Vector3 center() const {
            return (minPoint + maxPoint) * 0.5f;
        }
        
        Vector3 halfExtents() const {
            return (maxPoint - minPoint) * 0.5f;
        }
        
        bool contains(const Vector3& point) const {
            return point.x >= minPoint.x && point.x <= maxPoint.x &&
                   point.y >= minPoint.y && point.y <= maxPoint.y &&
                   point.z >= minPoint.z && point.z <= maxPoint.z;
        }
        
        bool intersects(const AABB& other) const {
            return minPoint.x <= other.maxPoint.x && maxPoint.x >= other.minPoint.x &&
                   minPoint.y <= other.maxPoint.y && maxPoint.y >= other.minPoint.y &&
                   minPoint.z <= other.maxPoint.z && maxPoint.z >= other.minPoint.z;
        }
    };
    
    // Sphere collider
    struct Sphere {
        Vector3 center;
        float radius;
        
        Sphere() : center(0, 0, 0), radius(1.0f) {}
        Sphere(const Vector3& c, float r) : center(c), radius(r) {}
        
        bool contains(const Vector3& point) const {
            return (point - center).length() <= radius;
        }
        
        bool intersects(const Sphere& other) const {
            float dist = (center - other.center).length();
            return dist <= (radius + other.radius);
        }
        
        bool intersects(const AABB& box) const {
            // Find closest point on AABB to sphere center
            float closestX = std::max(box.minPoint.x, std::min(center.x, box.maxPoint.x));
            float closestY = std::max(box.minPoint.y, std::min(center.y, box.maxPoint.y));
            float closestZ = std::max(box.minPoint.z, std::min(center.z, box.maxPoint.z));
            
            Vector3 closest(closestX, closestY, closestZ);
            float dist = (center - closest).length();
            return dist <= radius;
        }
    };
    
    // Ray for shooting/raycasting
    struct Ray {
        Vector3 origin;
        Vector3 direction;
        
        Ray() : origin(0, 0, 0), direction(0, 0, -1) {}
        Ray(const Vector3& o, const Vector3& d) : origin(o), direction(d.normalized()) {}
        
        Vector3 pointAt(float t) const {
            return origin + direction * t;
        }
        
        // Alias for pointAt
        Vector3 getPoint(float t) const {
            return pointAt(t);
        }
        
        // Ray-AABB intersection
        bool intersects(const AABB& box, float& tMin, float& tMax) const {
            tMin = 0.0f;
            tMax = 1000000.0f;
            
            // X axis
            if (std::abs(direction.x) > 0.0001f) {
                float t1 = (box.minPoint.x - origin.x) / direction.x;
                float t2 = (box.maxPoint.x - origin.x) / direction.x;
                if (t1 > t2) std::swap(t1, t2);
                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);
                if (tMin > tMax) return false;
            } else if (origin.x < box.minPoint.x || origin.x > box.maxPoint.x) {
                return false;
            }
            
            // Y axis
            if (std::abs(direction.y) > 0.0001f) {
                float t1 = (box.minPoint.y - origin.y) / direction.y;
                float t2 = (box.maxPoint.y - origin.y) / direction.y;
                if (t1 > t2) std::swap(t1, t2);
                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);
                if (tMin > tMax) return false;
            } else if (origin.y < box.minPoint.y || origin.y > box.maxPoint.y) {
                return false;
            }
            
            // Z axis
            if (std::abs(direction.z) > 0.0001f) {
                float t1 = (box.minPoint.z - origin.z) / direction.z;
                float t2 = (box.maxPoint.z - origin.z) / direction.z;
                if (t1 > t2) std::swap(t1, t2);
                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);
                if (tMin > tMax) return false;
            } else if (origin.z < box.minPoint.z || origin.z > box.maxPoint.z) {
                return false;
            }
            
            return tMin <= tMax && tMax >= 0;
        }
        
        // Ray-Sphere intersection (returns hit distance, -1 if no hit)
        float intersects(const Sphere& sphere) const {
            Vector3 oc = origin - sphere.center;
            float a = direction.dot(direction);
            float b = 2.0f * oc.dot(direction);
            float c = oc.dot(oc) - sphere.radius * sphere.radius;
            float discriminant = b * b - 4 * a * c;
            
            if (discriminant < 0) return -1.0f;
            
            float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
            if (t < 0) {
                t = (-b + std::sqrt(discriminant)) / (2.0f * a);
            }
            return (t >= 0) ? t : -1.0f;
        }
        
        // Ray-Sphere intersection with output parameter
        bool intersects(const Sphere& sphere, float& t) const {
            Vector3 oc = origin - sphere.center;
            float a = direction.dot(direction);
            float b = 2.0f * oc.dot(direction);
            float c = oc.dot(oc) - sphere.radius * sphere.radius;
            float discriminant = b * b - 4 * a * c;
            
            if (discriminant < 0) return false;
            
            t = (-b - std::sqrt(discriminant)) / (2.0f * a);
            if (t < 0) {
                t = (-b + std::sqrt(discriminant)) / (2.0f * a);
            }
            return t >= 0;
        }
    };
    
    // Wall structure with collision
    struct Wall {
        AABB bounds;
        bool isDestructible;
        int health;
        
        Wall() : isDestructible(false), health(100) {}
        Wall(const Vector3& minP, const Vector3& maxP, bool destructible = false) 
            : bounds(minP, maxP), isDestructible(destructible), health(100) {}
    };
    
    // Platform structure
    struct Platform {
        AABB bounds;
        bool isMoving;
        Vector3 moveStart;
        Vector3 moveEnd;
        float moveSpeed;
        float moveProgress;
        Vector3 center;
        Vector3 size;
        
        Platform() : isMoving(false), moveSpeed(1.0f), moveProgress(0.0f), center(0,0,0), size(1,1,1) {}
        
        // Constructor with center and size
        Platform(const Vector3& c, const Vector3& s) 
            : center(c), size(s), isMoving(false), moveSpeed(1.0f), moveProgress(0.0f) {
            Vector3 halfSize = s * 0.5f;
            bounds = AABB(c - halfSize, c + halfSize);
        }
        
        // Accessor for position (alias for center)
        const Vector3& position() const { return center; }
        
        void update(float dt) {
            if (!isMoving) return;
            
            moveProgress += moveSpeed * dt;
            if (moveProgress > 1.0f) {
                moveProgress = 1.0f;
                moveSpeed = -moveSpeed; // Reverse direction
            } else if (moveProgress < 0.0f) {
                moveProgress = 0.0f;
                moveSpeed = -moveSpeed;
            }
            
            Vector3 currentCenter = moveStart + (moveEnd - moveStart) * moveProgress;
            Vector3 halfExt = bounds.halfExtents();
            bounds.minPoint = currentCenter - halfExt;
            bounds.maxPoint = currentCenter + halfExt;
            center = currentCenter;
        }
        
        // Check if player is standing on this platform
        bool isPlayerOnTop(const Vector3& playerPos, float playerRadius, float& groundHeight) const {
            // Check if player is within horizontal bounds
            if (playerPos.x + playerRadius < bounds.minPoint.x || 
                playerPos.x - playerRadius > bounds.maxPoint.x ||
                playerPos.z + playerRadius < bounds.minPoint.z || 
                playerPos.z - playerRadius > bounds.maxPoint.z) {
                return false;
            }
            
            // Check if player is at platform height
            float platformTop = bounds.maxPoint.y;
            if (playerPos.y >= platformTop - 0.5f && playerPos.y <= platformTop + 2.0f) {
                groundHeight = platformTop;
                return true;
            }
            return false;
        }
    };
    
    // Door structure
    struct Door {
        AABB bounds;
        AABB openBounds;
        AABB closedBounds;
        bool isOpen;
        bool requiresKey;
        int keyType; // 0 = red, 1 = blue, 2 = yellow
        float openProgress;
        
        Door() : isOpen(false), requiresKey(false), keyType(0), openProgress(0.0f) {}
        
        void update(float dt) {
            float targetProgress = isOpen ? 1.0f : 0.0f;
            if (openProgress < targetProgress) {
                openProgress += dt * 2.0f;
                if (openProgress > targetProgress) openProgress = targetProgress;
            } else if (openProgress > targetProgress) {
                openProgress -= dt * 2.0f;
                if (openProgress < targetProgress) openProgress = targetProgress;
            }
            
            // Interpolate bounds
            bounds.minPoint = closedBounds.minPoint + (openBounds.minPoint - closedBounds.minPoint) * openProgress;
            bounds.maxPoint = closedBounds.maxPoint + (openBounds.maxPoint - closedBounds.maxPoint) * openProgress;
        }
        
        bool tryOpen(bool hasRedKey, bool hasBlueKey, bool hasYellowKey) {
            if (isOpen) return true;
            
            if (requiresKey) {
                if (keyType == 0 && !hasRedKey) return false;
                if (keyType == 1 && !hasBlueKey) return false;
                if (keyType == 2 && !hasYellowKey) return false;
            }
            
            isOpen = true;
            return true;
        }
    };
    
    // Collision response helper
    inline Vector3 resolveAABBCollision(const AABB& moving, const AABB& stationary) {
        Vector3 resolution(0, 0, 0);
        
        Vector3 movingCenter = moving.center();
        Vector3 stationaryCenter = stationary.center();
        Vector3 diff = movingCenter - stationaryCenter;
        
        Vector3 movingHalf = moving.halfExtents();
        Vector3 stationaryHalf = stationary.halfExtents();
        
        float overlapX = (movingHalf.x + stationaryHalf.x) - std::abs(diff.x);
        float overlapY = (movingHalf.y + stationaryHalf.y) - std::abs(diff.y);
        float overlapZ = (movingHalf.z + stationaryHalf.z) - std::abs(diff.z);
        
        if (overlapX > 0 && overlapY > 0 && overlapZ > 0) {
            // Find minimum overlap axis
            if (overlapX <= overlapY && overlapX <= overlapZ) {
                resolution.x = (diff.x > 0) ? overlapX : -overlapX;
            } else if (overlapY <= overlapX && overlapY <= overlapZ) {
                resolution.y = (diff.y > 0) ? overlapY : -overlapY;
            } else {
                resolution.z = (diff.z > 0) ? overlapZ : -overlapZ;
            }
        }
        
        return resolution;
    }
    
    // Check if player can stand on platform
    inline bool isOnGround(const AABB& player, const AABB& ground) {
        // Check if player is just above the ground
        float playerBottom = player.minPoint.y;
        float groundTop = ground.maxPoint.y;
        
        if (std::abs(playerBottom - groundTop) > 0.1f) return false;
        
        // Check horizontal overlap
        return player.minPoint.x < ground.maxPoint.x && player.maxPoint.x > ground.minPoint.x &&
               player.minPoint.z < ground.maxPoint.z && player.maxPoint.z > ground.minPoint.z;
    }
    
    // Collision result structure
    struct CollisionResult {
        bool hit;
        Vector3 normal;
        float penetration;
        Vector3 point;
        
        CollisionResult() : hit(false), penetration(0.0f) {}
    };
    
    // Resolve sphere vs AABB collision
    inline CollisionResult resolveSphereAABB(const Sphere& sphere, const AABB& box) {
        CollisionResult result;
        
        // Find closest point on AABB to sphere center
        float closestX = std::max(box.minPoint.x, std::min(sphere.center.x, box.maxPoint.x));
        float closestY = std::max(box.minPoint.y, std::min(sphere.center.y, box.maxPoint.y));
        float closestZ = std::max(box.minPoint.z, std::min(sphere.center.z, box.maxPoint.z));
        
        Vector3 closest(closestX, closestY, closestZ);
        result.point = closest;
        
        Vector3 diff = sphere.center - closest;
        float dist = diff.length();
        
        if (dist < sphere.radius && dist > 0.0001f) {
            result.hit = true;
            result.normal = diff.normalized();
            result.penetration = sphere.radius - dist;
        }
        
        return result;
    }
}
