/**
 * DOOMERS - Game Level System
 * 
 * Level management using FBX environments:
 * - Level 1: Sci-Fi Interior (research facility)
 * - Level 2: Lava Zone (hell arena)
 * 
 * Each level manages:
 * - Environment rendering (FBX models)
 * - Enemy spawning
 * - Pickup placement
 * - Collision geometry
 * - Lighting/atmosphere
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/AssimpLoader.hpp"
#include "../Engine/Physics.hpp"
#include "GameAssets.hpp"
#include "AnimatedEnemy.hpp"
#include <vector>
#include <memory>

namespace Doomers {

// ============================================================================
// Pickup Item
// ============================================================================
struct GamePickup {
    enum class Type { Health, Ammo };
    
    Type type;
    Math::Vector3 position;
    int value;
    bool active = true;
    float bobTimer = 0.0f;
    float spinAngle = 0.0f;
    
    void update(float dt) {
        if (!active) return;
        bobTimer += dt * 2.0f;
        spinAngle += dt * 90.0f;
    }
    
    void render() {
        if (!active) return;
        
        float bob = sinf(bobTimer) * 0.2f;
        
        glPushMatrix();
        glTranslatef(position.x, position.y + 0.5f + bob, position.z);
        glRotatef(spinAngle, 0, 1, 0);
        
        // Simple colored cube representation
        if (type == Type::Health) {
            glColor3f(0.2f, 0.8f, 0.2f); // Green for health
        } else {
            glColor3f(0.8f, 0.6f, 0.2f); // Orange for ammo
        }
        
        // Draw a simple box
        float s = 0.3f;
        glBegin(GL_QUADS);
            // Front
            glNormal3f(0, 0, 1);
            glVertex3f(-s, -s, s); glVertex3f(s, -s, s);
            glVertex3f(s, s, s); glVertex3f(-s, s, s);
            // Back
            glNormal3f(0, 0, -1);
            glVertex3f(s, -s, -s); glVertex3f(-s, -s, -s);
            glVertex3f(-s, s, -s); glVertex3f(s, s, -s);
            // Top
            glNormal3f(0, 1, 0);
            glVertex3f(-s, s, -s); glVertex3f(-s, s, s);
            glVertex3f(s, s, s); glVertex3f(s, s, -s);
            // Bottom
            glNormal3f(0, -1, 0);
            glVertex3f(-s, -s, s); glVertex3f(-s, -s, -s);
            glVertex3f(s, -s, -s); glVertex3f(s, -s, s);
            // Right
            glNormal3f(1, 0, 0);
            glVertex3f(s, -s, s); glVertex3f(s, -s, -s);
            glVertex3f(s, s, -s); glVertex3f(s, s, s);
            // Left
            glNormal3f(-1, 0, 0);
            glVertex3f(-s, -s, -s); glVertex3f(-s, -s, s);
            glVertex3f(-s, s, s); glVertex3f(-s, s, -s);
        glEnd();
        
        glPopMatrix();
    }
    
    void collect() {
        active = false;
    }
};

// ============================================================================
// Game Level Base Class
// ============================================================================
class GameLevel {
public:
    std::string name;
    bool loaded = false;
    
    // Environment model
    AnimatedModel* environment = nullptr;
    float environmentScale = 1.0f;
    Math::Vector3 environmentOffset;
    
    // Enemies
    std::vector<std::unique_ptr<AnimatedEnemy>> enemies;
    
    // Pickups
    std::vector<GamePickup> pickups;
    
    // Spawn points
    Math::Vector3 playerSpawn;
    float playerSpawnYaw = 0.0f;
    
    // Level bounds (for simple collision)
    Math::Vector3 boundsMin;
    Math::Vector3 boundsMax;
    
    // Lighting
    Math::Vector3 ambientColor;
    Math::Vector3 fogColor;
    float fogStart = 20.0f;
    float fogEnd = 100.0f;
    bool fogEnabled = true;
    
    // Directional light (sun/moon)
    Math::Vector3 lightDirection;
    Math::Vector3 lightColor;
    float lightIntensity = 1.0f;
    
    // Floor level
    float floorY = 0.0f;
    
    virtual ~GameLevel() { unload(); }
    
    // ========================================================================
    // Load / Unload
    // ========================================================================
    virtual bool load() {
        LOG_INFO("Loading level: " << name);
        loaded = true;
        return true;
    }
    
    virtual void unload() {
        enemies.clear();
        pickups.clear();
        loaded = false;
    }
    
    // ========================================================================
    // Update
    // ========================================================================
    virtual void update(float dt, Math::Vector3& playerPos) {
        // Update enemies
        for (auto& enemy : enemies) {
            if (enemy->isActive()) {
                enemy->setTarget(&playerPos);
                enemy->update(dt);
            }
        }
        
        // Update pickups
        for (auto& pickup : pickups) {
            pickup.update(dt);
        }
    }
    
    // ========================================================================
    // Render
    // ========================================================================
    virtual void render() {
        // Setup level lighting
        setupLighting();
        
        // Render environment
        renderEnvironment();
        
        // Render simple floor if no environment
        if (!environment) {
            renderSimpleFloor();
        }
        
        // Render enemies
        for (auto& enemy : enemies) {
            if (enemy->isActive()) {
                enemy->render();
            }
        }
        
        // Render pickups
        for (auto& pickup : pickups) {
            pickup.render();
        }
    }
    
    void setupLighting() {
        // Fog
        if (fogEnabled) {
            glEnable(GL_FOG);
            glFogi(GL_FOG_MODE, GL_LINEAR);
            GLfloat fogCol[] = { fogColor.x, fogColor.y, fogColor.z, 1.0f };
            glFogfv(GL_FOG_COLOR, fogCol);
            glFogf(GL_FOG_START, fogStart);
            glFogf(GL_FOG_END, fogEnd);
        } else {
            glDisable(GL_FOG);
        }
        
        // Ambient
        GLfloat ambient[] = { ambientColor.x, ambientColor.y, ambientColor.z, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
        
        // Directional light (GL_LIGHT0)
        glEnable(GL_LIGHT0);
        GLfloat lightDir[] = { lightDirection.x, lightDirection.y, lightDirection.z, 0.0f };
        GLfloat lightCol[] = { 
            lightColor.x * lightIntensity, 
            lightColor.y * lightIntensity, 
            lightColor.z * lightIntensity, 
            1.0f 
        };
        glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
    }
    
    void renderEnvironment() {
        if (!environment) return;
        
        glPushMatrix();
        glTranslatef(environmentOffset.x, environmentOffset.y, environmentOffset.z);
        glScalef(environmentScale, environmentScale, environmentScale);
        
        environment->drawStatic(); // Environment doesn't animate
        
        glPopMatrix();
    }
    
    void renderSimpleFloor() {
        // Render a simple grid floor as fallback
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.3f, 0.3f, 0.35f);
        
        float size = 50.0f;
        float gridSize = 2.0f;
        
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        for (float x = -size; x < size; x += gridSize) {
            for (float z = -size; z < size; z += gridSize) {
                // Checkerboard pattern
                float c = ((int)(x / gridSize) + (int)(z / gridSize)) % 2 == 0 ? 0.25f : 0.35f;
                glColor3f(c, c, c + 0.05f);
                
                glVertex3f(x, floorY, z);
                glVertex3f(x + gridSize, floorY, z);
                glVertex3f(x + gridSize, floorY, z + gridSize);
                glVertex3f(x, floorY, z + gridSize);
            }
        }
        glEnd();
    }
    
    // ========================================================================
    // Enemy Management
    // ========================================================================
    AnimatedEnemy* spawnZombie(const Math::Vector3& pos) {
        auto enemy = std::make_unique<AnimatedEnemy>(AnimEnemyType::Zombie);
        enemy->position = pos;
        enemy->initialize();
        AnimatedEnemy* ptr = enemy.get();
        enemies.push_back(std::move(enemy));
        return ptr;
    }
    
    AnimatedEnemy* spawnCrawler(const Math::Vector3& pos) {
        auto enemy = std::make_unique<AnimatedEnemy>(AnimEnemyType::CrawlingZombie);
        enemy->position = pos;
        enemy->initialize();
        AnimatedEnemy* ptr = enemy.get();
        enemies.push_back(std::move(enemy));
        return ptr;
    }
    
    AnimatedEnemy* spawnDevil(const Math::Vector3& pos) {
        auto enemy = std::make_unique<AnimatedEnemy>(AnimEnemyType::Devil);
        enemy->position = pos;
        enemy->initialize();
        AnimatedEnemy* ptr = enemy.get();
        enemies.push_back(std::move(enemy));
        return ptr;
    }
    
    void removeDeadEnemies() {
        enemies.erase(
            std::remove_if(enemies.begin(), enemies.end(),
                [](const std::unique_ptr<AnimatedEnemy>& e) { 
                    return !e->isActive(); 
                }),
            enemies.end()
        );
    }
    
    int getAliveEnemyCount() const {
        int count = 0;
        for (const auto& enemy : enemies) {
            if (enemy->isAlive()) count++;
        }
        return count;
    }
    
    // ========================================================================
    // Pickup Management
    // ========================================================================
    void spawnHealthPack(const Math::Vector3& pos, int value = 25) {
        GamePickup pickup;
        pickup.type = GamePickup::Type::Health;
        pickup.position = pos;
        pickup.value = value;
        pickups.push_back(pickup);
    }
    
    void spawnAmmoPack(const Math::Vector3& pos, int value = 30) {
        GamePickup pickup;
        pickup.type = GamePickup::Type::Ammo;
        pickup.position = pos;
        pickup.value = value;
        pickups.push_back(pickup);
    }
    
    // ========================================================================
    // Collision Helpers
    // ========================================================================
    bool isInBounds(const Math::Vector3& pos) const {
        return pos.x >= boundsMin.x && pos.x <= boundsMax.x &&
               pos.z >= boundsMin.z && pos.z <= boundsMax.z;
    }
    
    Math::Vector3 clampToBounds(const Math::Vector3& pos) const {
        Math::Vector3 result = pos;
        if (result.x < boundsMin.x) result.x = boundsMin.x;
        if (result.x > boundsMax.x) result.x = boundsMax.x;
        if (result.z < boundsMin.z) result.z = boundsMin.z;
        if (result.z > boundsMax.z) result.z = boundsMax.z;
        return result;
    }
};

// ============================================================================
// Level 1: Sci-Fi Research Facility
// ============================================================================
class FacilityGameLevel : public GameLevel {
public:
    FacilityGameLevel() {
        name = "Research Facility";
        
        // Lighting - dim, eerie indoor lighting
        ambientColor = Math::Vector3(0.1f, 0.08f, 0.12f);
        fogColor = Math::Vector3(0.02f, 0.02f, 0.05f);
        fogStart = 5.0f;
        fogEnd = 40.0f;
        fogEnabled = true;
        
        lightDirection = Math::Vector3(0.3f, -0.8f, 0.5f).normalized();
        lightColor = Math::Vector3(0.4f, 0.4f, 0.5f);
        lightIntensity = 0.6f;
        
        // Bounds
        boundsMin = Math::Vector3(-30, 0, -30);
        boundsMax = Math::Vector3(30, 10, 30);
        
        // Player spawn
        playerSpawn = Math::Vector3(0, 0, 0);
        playerSpawnYaw = 0;
        
        floorY = 0.0f;
    }
    
    bool load() override {
        LOG_INFO("Loading Facility Level...");
        
        // Try to load environment FBX
        environment = GameAssets::instance().loadLevel1Environment();
        if (environment) {
            environmentScale = 0.01f; // Scale down FBX models
            environmentOffset = Math::Vector3(0, 0, 0);
            LOG_INFO("Facility environment loaded");
        } else {
            LOG_WARN("Facility environment not found - using procedural");
        }
        
        // Spawn enemies in a pattern around the facility
        spawnZombie(Math::Vector3(5, 0, 10));
        spawnZombie(Math::Vector3(-5, 0, 10));
        spawnZombie(Math::Vector3(10, 0, 5));
        spawnZombie(Math::Vector3(-10, 0, 5));
        spawnZombie(Math::Vector3(8, 0, -8));
        spawnCrawler(Math::Vector3(-8, 0, -8));
        spawnZombie(Math::Vector3(0, 0, 15));
        spawnZombie(Math::Vector3(15, 0, 0));
        
        // Spawn pickups
        spawnHealthPack(Math::Vector3(3, 0, 3));
        spawnAmmoPack(Math::Vector3(-3, 0, 3));
        spawnHealthPack(Math::Vector3(10, 0, -5));
        spawnAmmoPack(Math::Vector3(-10, 0, -5));
        
        loaded = true;
        LOG_INFO("Facility Level loaded with " << enemies.size() << " enemies");
        return true;
    }
};

// ============================================================================
// Level 2: Lava Zone Hell Arena
// ============================================================================
class LavaArenaGameLevel : public GameLevel {
public:
    float lavaGlow = 0.0f;
    
    LavaArenaGameLevel() {
        name = "Hell Arena";
        
        // Lighting - fiery, hellish
        ambientColor = Math::Vector3(0.2f, 0.05f, 0.02f);
        fogColor = Math::Vector3(0.15f, 0.03f, 0.01f);
        fogStart = 10.0f;
        fogEnd = 60.0f;
        fogEnabled = true;
        
        lightDirection = Math::Vector3(0.0f, -0.5f, 0.5f).normalized();
        lightColor = Math::Vector3(1.0f, 0.5f, 0.2f); // Orange-red
        lightIntensity = 0.8f;
        
        // Bounds - larger arena
        boundsMin = Math::Vector3(-40, 0, -40);
        boundsMax = Math::Vector3(40, 15, 40);
        
        // Player spawn
        playerSpawn = Math::Vector3(0, 0, -20);
        playerSpawnYaw = 0;
        
        floorY = 0.0f;
    }
    
    bool load() override {
        LOG_INFO("Loading Lava Arena Level...");
        
        // Try to load environment FBX
        environment = GameAssets::instance().loadLevel2Environment();
        if (environment) {
            environmentScale = 0.02f;
            environmentOffset = Math::Vector3(0, -2, 0);
            LOG_INFO("Lava environment loaded");
        } else {
            LOG_WARN("Lava environment not found - using procedural");
        }
        
        // Spawn more enemies - this is the hard level
        // Wave 1: Zombies around the arena
        spawnZombie(Math::Vector3(10, 0, 10));
        spawnZombie(Math::Vector3(-10, 0, 10));
        spawnZombie(Math::Vector3(10, 0, -10));
        spawnZombie(Math::Vector3(-10, 0, -10));
        
        // Wave 2: More zombies and crawlers
        spawnZombie(Math::Vector3(20, 0, 0));
        spawnZombie(Math::Vector3(-20, 0, 0));
        spawnZombie(Math::Vector3(0, 0, 20));
        spawnCrawler(Math::Vector3(15, 0, 15));
        spawnCrawler(Math::Vector3(-15, 0, 15));
        spawnCrawler(Math::Vector3(15, 0, -15));
        
        // Boss: The Devil
        AnimatedEnemy* devil = spawnDevil(Math::Vector3(0, 0, 25));
        if (devil) {
            devil->aiState = EnemyAIState::Idle; // Devil waits for player
        }
        
        // Pickups scattered around
        spawnHealthPack(Math::Vector3(5, 0, -15));
        spawnHealthPack(Math::Vector3(-5, 0, -15));
        spawnAmmoPack(Math::Vector3(15, 0, 0));
        spawnAmmoPack(Math::Vector3(-15, 0, 0));
        spawnHealthPack(Math::Vector3(0, 0, 15), 50); // Big health pack
        spawnAmmoPack(Math::Vector3(20, 0, 20), 60); // Big ammo pack
        
        loaded = true;
        LOG_INFO("Lava Arena Level loaded with " << enemies.size() << " enemies (including boss)");
        return true;
    }
    
    void update(float dt, Math::Vector3& playerPos) override {
        GameLevel::update(dt, playerPos);
        
        // Pulsing lava glow effect
        lavaGlow += dt;
        float pulse = sinf(lavaGlow * 2.0f) * 0.1f + 0.9f;
        ambientColor = Math::Vector3(0.2f * pulse, 0.05f * pulse, 0.02f);
    }
    
    void render() override {
        // Add extra lava floor rendering
        GameLevel::render();
        
        // Render lava pools (simple red/orange patches)
        renderLavaPools();
    }
    
    void renderLavaPools() {
        // Render some glowing lava pools around the arena edges
        float glow = sinf(lavaGlow * 3.0f) * 0.2f + 0.8f;
        
        glDisable(GL_LIGHTING);
        glColor3f(1.0f * glow, 0.3f * glow, 0.1f);
        
        // Simple lava pools at edges
        float poolRadius = 3.0f;
        float arenaSize = 35.0f;
        
        std::vector<Math::Vector3> poolCenters = {
            {arenaSize, -0.1f, arenaSize},
            {-arenaSize, -0.1f, arenaSize},
            {arenaSize, -0.1f, -arenaSize},
            {-arenaSize, -0.1f, -arenaSize}
        };
        
        for (const auto& center : poolCenters) {
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(center.x, center.y, center.z);
            for (int i = 0; i <= 16; i++) {
                float angle = (float)i / 16.0f * 6.28318f;
                glVertex3f(
                    center.x + cosf(angle) * poolRadius,
                    center.y,
                    center.z + sinf(angle) * poolRadius
                );
            }
            glEnd();
        }
        
        glEnable(GL_LIGHTING);
    }
};

// ============================================================================
// Level Manager
// ============================================================================
class LevelManager {
public:
    static LevelManager& instance() {
        static LevelManager inst;
        return inst;
    }
    
    int currentLevelIndex = 0;
    GameLevel* currentLevel = nullptr;
    
    bool loadLevel(int index) {
        // Unload current
        if (currentLevel) {
            currentLevel->unload();
            delete currentLevel;
            currentLevel = nullptr;
        }
        
        currentLevelIndex = index;
        
        // Create new level
        switch (index) {
            case 0:
                currentLevel = new FacilityGameLevel();
                break;
            case 1:
                currentLevel = new LavaArenaGameLevel();
                break;
            default:
                return false; // Victory!
        }
        
        return currentLevel->load();
    }
    
    void unloadCurrent() {
        if (currentLevel) {
            currentLevel->unload();
            delete currentLevel;
            currentLevel = nullptr;
        }
    }
    
    bool nextLevel() {
        return loadLevel(currentLevelIndex + 1);
    }
    
    GameLevel* getCurrent() { return currentLevel; }
    
private:
    LevelManager() = default;
    ~LevelManager() { unloadCurrent(); }
};

// Convenience accessor
inline LevelManager& Levels() {
    return LevelManager::instance();
}

} // namespace Doomers
