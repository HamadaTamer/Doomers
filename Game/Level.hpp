/**
 * DOOMERS - Level System
 * 
 * Manages level loading, geometry, spawning, and objectives
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/Entity.hpp"
#include "../Engine/Physics.hpp"
#include "../Engine/ResourceManager.hpp"
#include "Enemy.hpp"
#include "Player.hpp"

namespace Doomers {

// ============================================================================
// Spawn Point
// ============================================================================
struct SpawnPoint {
    Math::Vector3 position;
    float rotation;
    enum Type { Player, Enemy, Pickup } type;
    int subType; // enemy type, pickup type, etc.
};

// ============================================================================
// Level Segment - A piece of the level
// ============================================================================
struct LevelSegment {
    Math::Vector3 position;
    float rotation;
    Math::Vector3 scale;
    Mesh* mesh;
    unsigned int textureId;
    
    LevelSegment()
        : position(0, 0, 0)
        , rotation(0)
        , scale(1, 1, 1)
        , mesh(nullptr)
        , textureId(0)
    {}
    
    void render() const {
        if (!mesh) return;
        
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
};

// ============================================================================
// Level - Complete level with geometry and entities
// ============================================================================
class Level {
public:
    Level()
        : name("Unnamed")
        , loaded(false)
        , ambientColor(0.1f, 0.1f, 0.15f)
        , fogColor(0.05f, 0.05f, 0.1f)
        , fogStart(20.0f)
        , fogEnd(100.0f)
    {}
    
    virtual ~Level() {
        unload();
    }
    
    // ========================================================================
    // Level Management
    // ========================================================================
    virtual void load() {
        LOG_INFO("Loading level: " << name);
        loaded = true;
    }
    
    virtual void unload() {
        segments.clear();
        props.clear();
        enemies.clear();
        pickups.clear();
        spawnPoints.clear();
        PhysicsWorld::instance().clearColliders();
        loaded = false;
    }
    
    virtual void update(float deltaTime) {
        // Update all enemies
        for (auto& enemy : enemies) {
            if (enemy.isActive()) {
                enemy.update(deltaTime);
            }
        }
        
        // Update all pickups
        for (auto& pickup : pickups) {
            if (pickup.isActive()) {
                pickup.update(deltaTime);
            }
        }
        
        // Update props
        for (auto& prop : props) {
            if (prop.isActive()) {
                prop.update(deltaTime);
            }
        }
    }
    
    virtual void render() {
        // Set up lighting
        setupLighting();
        
        // Render level geometry
        for (const auto& segment : segments) {
            segment.render();
        }
        
        // Render props
        for (auto& prop : props) {
            if (prop.isVisible()) {
                prop.render();
            }
        }
        
        // Render pickups
        for (auto& pickup : pickups) {
            if (pickup.isActive()) {
                pickup.render();
            }
        }
        
        // Render enemies
        for (auto& enemy : enemies) {
            if (enemy.isActive()) {
                enemy.render();
            }
        }
    }
    
    // ========================================================================
    // Entity Management
    // ========================================================================
    void addSegment(const LevelSegment& segment) {
        segments.push_back(segment);
    }
    
    void addProp(const Prop& prop) {
        props.push_back(prop);
    }
    
    Enemy* spawnEnemy(EnemyType type, const Math::Vector3& pos, Entity* target = nullptr) {
        enemies.push_back(Enemy());
        Enemy& enemy = enemies.back();
        enemy.initialize(type, pos);
        enemy.setTarget(target);
        return &enemy;
    }
    
    Pickup* spawnPickup(PickupType type, int value, const Math::Vector3& pos) {
        pickups.push_back(Pickup());
        Pickup& pickup = pickups.back();
        pickup.initialize(type, value, pos);
        return &pickup;
    }
    
    void addSpawnPoint(const SpawnPoint& sp) {
        spawnPoints.push_back(sp);
    }
    
    void addCollider(const Math::AABB& aabb) {
        PhysicsWorld::instance().addCollider(aabb);
    }
    
    void addWall(const Math::Vector3& min, const Math::Vector3& max) {
        PhysicsWorld::instance().addWall(min, max);
    }
    
    // ========================================================================
    // Collision Setup Helpers
    // ========================================================================
    void createCorridorColliders(float width, float height, float length, 
                                  const Math::Vector3& start) {
        float halfWidth = width * 0.5f;
        
        // Left wall
        addWall(
            Math::Vector3(-halfWidth - 0.5f, 0, start.z - length),
            Math::Vector3(-halfWidth, height, start.z)
        );
        
        // Right wall
        addWall(
            Math::Vector3(halfWidth, 0, start.z - length),
            Math::Vector3(halfWidth + 0.5f, height, start.z)
        );
        
        // Floor (implicit at y=0)
    }
    
    void createRoom(const Math::Vector3& center, const Math::Vector3& size) {
        float halfX = size.x * 0.5f;
        float halfZ = size.z * 0.5f;
        
        // Four walls
        // North wall
        addWall(
            Math::Vector3(center.x - halfX, 0, center.z - halfZ - 0.5f),
            Math::Vector3(center.x + halfX, size.y, center.z - halfZ)
        );
        // South wall
        addWall(
            Math::Vector3(center.x - halfX, 0, center.z + halfZ),
            Math::Vector3(center.x + halfX, size.y, center.z + halfZ + 0.5f)
        );
        // West wall
        addWall(
            Math::Vector3(center.x - halfX - 0.5f, 0, center.z - halfZ),
            Math::Vector3(center.x - halfX, size.y, center.z + halfZ)
        );
        // East wall
        addWall(
            Math::Vector3(center.x + halfX, 0, center.z - halfZ),
            Math::Vector3(center.x + halfX + 0.5f, size.y, center.z + halfZ)
        );
    }
    
    // ========================================================================
    // Lighting
    // ========================================================================
    virtual void setupLighting() {
        // Ambient light
        Renderer::instance().setAmbientLight(ambientColor);
        
        // Set fog
        Renderer::instance().setFog(true, fogColor, fogStart, fogEnd);
    }
    
    // ========================================================================
    // Player Interaction
    // ========================================================================
    void checkPickups(Player* player) {
        if (!player) return;
        
        Math::Vector3 playerPos = player->getPosition();
        
        for (auto& pickup : pickups) {
            if (!pickup.isActive()) continue;
            
            float dist = pickup.distanceTo(playerPos);
            if (dist < 1.5f) {
                switch (pickup.getPickupType()) {
                    case PickupType::Health:
                        player->collectHealth(pickup.getValue());
                        break;
                    case PickupType::Ammo:
                        player->collectAmmo(pickup.getValue());
                        break;
                    default:
                        break;
                }
                pickup.collect();
            }
        }
    }
    
    void setEnemyTargets(Entity* target) {
        for (auto& enemy : enemies) {
            enemy.setTarget(target);
        }
    }
    
    std::vector<Entity*> getEnemyPointers() {
        std::vector<Entity*> result;
        for (auto& enemy : enemies) {
            if (enemy.isActive()) {
                result.push_back(&enemy);
            }
        }
        return result;
    }
    
    // ========================================================================
    // Spawn Points
    // ========================================================================
    SpawnPoint* getPlayerSpawn() {
        for (auto& sp : spawnPoints) {
            if (sp.type == SpawnPoint::Player) {
                return &sp;
            }
        }
        return nullptr;
    }
    
    // ========================================================================
    // Statistics
    // ========================================================================
    int getEnemyCount() const {
        int count = 0;
        for (const auto& enemy : enemies) {
            if (enemy.isActive() && !enemy.isDead()) {
                count++;
            }
        }
        return count;
    }
    
    int getKillCount() const {
        int count = 0;
        for (const auto& enemy : enemies) {
            if (enemy.isDead()) {
                count++;
            }
        }
        return count;
    }
    
    // ========================================================================
    // Getters/Setters
    // ========================================================================
    const std::string& getName() const { return name; }
    bool isLoaded() const { return loaded; }
    
    void setName(const std::string& n) { name = n; }
    void setAmbientColor(const Math::Color& c) { ambientColor = c; }
    void setFog(const Math::Color& c, float start, float end) {
        fogColor = c;
        fogStart = start;
        fogEnd = end;
    }
    
    // For setting up meshes on pickups and enemies
    void setEnemyMesh(Mesh* m, EnemyType type = EnemyType::Zombie) {
        for (auto& enemy : enemies) {
            if (enemy.getEnemyType() == type) {
                enemy.setMesh(m);
            }
        }
    }
    
    void setPickupMesh(Mesh* m, PickupType type) {
        for (auto& pickup : pickups) {
            if (pickup.getPickupType() == type) {
                pickup.setMesh(m);
            }
        }
    }
    
    void setPickupTexture(unsigned int tex, PickupType type) {
        for (auto& pickup : pickups) {
            if (pickup.getPickupType() == type) {
                pickup.setTexture(tex);
            }
        }
    }
    
    // Get enemy pointers for iteration
    std::vector<Enemy*> getEnemies() {
        std::vector<Enemy*> result;
        for (auto& enemy : enemies) {
            result.push_back(&enemy);
        }
        return result;
    }
    
    // Get pickup pointers for iteration
    std::vector<Pickup*> getPickups() {
        std::vector<Pickup*> result;
        for (auto& pickup : pickups) {
            result.push_back(&pickup);
        }
        return result;
    }
    
    const std::vector<SpawnPoint>& getSpawnPoints() const {
        return spawnPoints;
    }
    
protected:
    std::string name;
    bool loaded;
    
    Math::Color ambientColor;
    Math::Color fogColor;
    float fogStart;
    float fogEnd;
    
    std::vector<LevelSegment> segments;
    std::vector<Prop> props;
    std::vector<Enemy> enemies;
    std::vector<Pickup> pickups;
    std::vector<SpawnPoint> spawnPoints;
};

// ============================================================================
// Level 1: Research Facility
// ============================================================================
class FacilityLevel : public Level {
public:
    FacilityLevel() {
        name = "Abandoned Research Facility";
        ambientColor = Math::Color(0.1f, 0.1f, 0.15f);
        fogColor = Math::Color(0.02f, 0.02f, 0.05f);
        fogStart = 15.0f;
        fogEnd = 80.0f;
    }
    
    void load() override {
        Level::load();
        
        // Corridor dimensions
        float corridorWidth = 6.0f;
        float corridorHeight = 4.0f;
        float corridorLength = 100.0f;
        
        // Create corridor colliders
        createCorridorColliders(corridorWidth, corridorHeight, corridorLength, 
                                Math::Vector3(0, 0, 5));
        
        // Add player spawn
        SpawnPoint playerSpawn;
        playerSpawn.position = Math::Vector3(0, 0, 0);
        playerSpawn.rotation = 0;
        playerSpawn.type = SpawnPoint::Player;
        addSpawnPoint(playerSpawn);
        
        // Add some cover (crates)
        addCrate(Math::Vector3(-1.5f, 0, -10.0f));
        addCrate(Math::Vector3(1.5f, 0, -15.0f));
        addCrate(Math::Vector3(-1.0f, 0, -25.0f));
        addCrate(Math::Vector3(0.0f, 0, -35.0f));
        addCrate(Math::Vector3(1.5f, 0, -45.0f));
        
        // Spawn enemies
        spawnEnemy(EnemyType::Zombie, Math::Vector3(0, 0, -20.0f));
        spawnEnemy(EnemyType::Zombie, Math::Vector3(-1.5f, 0, -40.0f));
        spawnEnemy(EnemyType::Zombie, Math::Vector3(1.5f, 0, -55.0f));
        spawnEnemy(EnemyType::Demon, Math::Vector3(0, 0, -70.0f));
        
        // Spawn pickups
        spawnPickup(PickupType::Health, 25, Math::Vector3(1.0f, 0.5f, -12.0f));
        spawnPickup(PickupType::Ammo, 15, Math::Vector3(-1.0f, 0.5f, -30.0f));
        spawnPickup(PickupType::Health, 25, Math::Vector3(0.0f, 0.5f, -50.0f));
        spawnPickup(PickupType::Ammo, 15, Math::Vector3(1.5f, 0.5f, -65.0f));
        
        LOG_INFO("Facility level loaded");
    }
    
private:
    void addCrate(const Math::Vector3& pos) {
        // Add collision
        PhysicsWorld::instance().addBox(
            pos + Math::Vector3(0, 0.6f, 0),
            Math::Vector3(1.2f, 1.2f, 1.2f)
        );
    }
};

// ============================================================================
// Level 2: Hell Arena
// ============================================================================
class ArenaLevel : public Level {
public:
    ArenaLevel() {
        name = "Rooftop Hell Arena";
        ambientColor = Math::Color(0.2f, 0.1f, 0.05f);
        fogColor = Math::Color(0.1f, 0.05f, 0.02f);
        fogStart = 30.0f;
        fogEnd = 150.0f;
    }
    
    void load() override {
        Level::load();
        
        // Large open arena
        float arenaSize = 50.0f;
        
        // Create arena walls
        createRoom(Math::Vector3(0, 0, -arenaSize/2), 
                  Math::Vector3(arenaSize, 10.0f, arenaSize));
        
        // Player spawn
        SpawnPoint playerSpawn;
        playerSpawn.position = Math::Vector3(0, 0, -5.0f);
        playerSpawn.rotation = 0;
        playerSpawn.type = SpawnPoint::Player;
        addSpawnPoint(playerSpawn);
        
        // Multiple enemy waves
        spawnEnemy(EnemyType::Zombie, Math::Vector3(-10, 0, -30));
        spawnEnemy(EnemyType::Zombie, Math::Vector3(10, 0, -30));
        spawnEnemy(EnemyType::Zombie, Math::Vector3(-15, 0, -40));
        spawnEnemy(EnemyType::Zombie, Math::Vector3(15, 0, -40));
        spawnEnemy(EnemyType::Demon, Math::Vector3(0, 0, -35));
        spawnEnemy(EnemyType::Demon, Math::Vector3(-8, 0, -45));
        spawnEnemy(EnemyType::Demon, Math::Vector3(8, 0, -45));
        
        // Boss in the center
        spawnEnemy(EnemyType::Boss, Math::Vector3(0, 0, -60));
        
        // Pickups scattered around
        spawnPickup(PickupType::Health, 25, Math::Vector3(-12, 0.5f, -20));
        spawnPickup(PickupType::Health, 25, Math::Vector3(12, 0.5f, -20));
        spawnPickup(PickupType::Ammo, 30, Math::Vector3(-20, 0.5f, -35));
        spawnPickup(PickupType::Ammo, 30, Math::Vector3(20, 0.5f, -35));
        spawnPickup(PickupType::Health, 50, Math::Vector3(0, 0.5f, -50));
        
        LOG_INFO("Arena level loaded");
    }
};

} // namespace Doomers
