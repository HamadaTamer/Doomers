/**
 * DOOMERS - Game World / Level System
 * Loads FBX environments and manages game state
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/AssimpLoader.hpp"
#include "FPSGame.hpp"
#include <vector>

namespace Doomers {

class GameWorld {
public:
    int currentLevel = 1;
    AnimatedModel* environmentModel = nullptr;
    std::vector<GameEnemy> enemies;
    std::vector<GamePickup> pickups;
    
    // Level bounds
    float levelWidth = 60.0f;
    float levelLength = 60.0f;
    float wallHeight = 6.0f;
    
    // Level colors
    struct LevelTheme {
        Math::Color floor, wall, ceiling, ambient;
    } theme;
    
    bool loadLevel(int levelNum) {
        currentLevel = levelNum;
        enemies.clear();
        pickups.clear();
        
        // Try to load FBX environment
        const char* path = (levelNum == 1) ? GameConfig::LEVEL1_FBX : GameConfig::LEVEL2_FBX;
        environmentModel = AssimpLoader::loadModel(path);
        
        if (environmentModel) {
            LOG_INFO("Loaded FBX environment: " + std::string(path));
        } else {
            LOG_WARN("FBX not found, using procedural level");
        }
        
        // Set theme
        if (levelNum == 1) {
            // Sci-Fi Research Facility
            theme.floor = {0.25f, 0.28f, 0.32f};
            theme.wall = {0.35f, 0.4f, 0.48f};
            theme.ceiling = {0.18f, 0.2f, 0.25f};
            theme.ambient = {0.4f, 0.45f, 0.55f};
            levelWidth = 50.0f;
            levelLength = 50.0f;
        } else {
            // Hell Arena (Lava Zone)
            theme.floor = {0.3f, 0.15f, 0.1f};
            theme.wall = {0.45f, 0.2f, 0.15f};
            theme.ceiling = {0.15f, 0.08f, 0.05f};
            theme.ambient = {0.6f, 0.3f, 0.2f};
            levelWidth = 70.0f;
            levelLength = 70.0f;
        }
        
        spawnEnemies(levelNum);
        spawnPickups(levelNum);
        
        return true;
    }
    
    void spawnEnemies(int level) {
        if (level == 1) {
            // Level 1: Zombies only
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(-12, 0, -12));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(12, 0, -12));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(0, 0, -18));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(-18, 0, 0));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(18, 0, 0));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(-8, 0, 15));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(8, 0, 15));
        } else {
            // Level 2: Zombies + Devils
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(-15, 0, -15));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(15, 0, -15));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(-20, 0, 0));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(20, 0, 0));
            enemies.emplace_back(GameEnemy::Type::Zombie, Math::Vector3(0, 0, 20));
            // Devils
            enemies.emplace_back(GameEnemy::Type::Devil, Math::Vector3(0, 0, -25));
            enemies.emplace_back(GameEnemy::Type::Devil, Math::Vector3(-25, 0, -10));
            enemies.emplace_back(GameEnemy::Type::Devil, Math::Vector3(25, 0, -10));
        }
    }
    
    void spawnPickups(int level) {
        pickups.emplace_back(GamePickup::Type::Health, Math::Vector3(-10, 0, 0), 25);
        pickups.emplace_back(GamePickup::Type::Health, Math::Vector3(10, 0, 0), 25);
        pickups.emplace_back(GamePickup::Type::Ammo, Math::Vector3(0, 0, -10), 20);
        pickups.emplace_back(GamePickup::Type::Ammo, Math::Vector3(0, 0, 10), 20);
        pickups.emplace_back(GamePickup::Type::Armor, Math::Vector3(15, 0, 15), 50);
        
        if (level == 2) {
            pickups.emplace_back(GamePickup::Type::Health, Math::Vector3(-20, 0, 10), 50);
            pickups.emplace_back(GamePickup::Type::Ammo, Math::Vector3(20, 0, -10), 30);
        }
    }
    
    void update(float dt, GamePlayer& player) {
        // Update enemies
        for (auto& e : enemies) {
            e.update(dt, player.position);
        }
        
        // Update pickups
        for (auto& p : pickups) {
            p.update(dt);
            if (p.checkCollision(player.position)) {
                switch (p.type) {
                    case GamePickup::Type::Health: player.addHealth((float)p.value); break;
                    case GamePickup::Type::Ammo: player.addAmmo(p.value); break;
                    case GamePickup::Type::Armor: player.addArmor((float)p.value); break;
                }
                p.active = false;
            }
        }
    }
    
    int getAliveEnemyCount() const {
        int count = 0;
        for (const auto& e : enemies) {
            if (e.isAlive()) count++;
        }
        return count;
    }
    
    void draw() {
        if (environmentModel) {
            // Draw FBX environment
            glPushMatrix();
            glScalef(0.05f, 0.05f, 0.05f); // Scale FBX
            environmentModel->draw();
            glPopMatrix();
        } else {
            // Draw procedural level
            drawProceduralLevel();
        }
        
        // Draw enemies
        for (auto& e : enemies) {
            e.draw();
        }
        
        // Draw pickups
        for (auto& p : pickups) {
            p.draw();
        }
    }
    
    void drawProceduralLevel() {
        float hw = levelWidth / 2;
        float hl = levelLength / 2;
        
        // Floor
        glColor3f(theme.floor.r, theme.floor.g, theme.floor.b);
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-hw, 0, -hl);
        glVertex3f(hw, 0, -hl);
        glVertex3f(hw, 0, hl);
        glVertex3f(-hw, 0, hl);
        glEnd();
        
        // Grid pattern
        glColor3f(theme.floor.r * 0.7f, theme.floor.g * 0.7f, theme.floor.b * 0.7f);
        glBegin(GL_LINES);
        for (float x = -hw; x <= hw; x += 3.0f) {
            glVertex3f(x, 0.02f, -hl);
            glVertex3f(x, 0.02f, hl);
        }
        for (float z = -hl; z <= hl; z += 3.0f) {
            glVertex3f(-hw, 0.02f, z);
            glVertex3f(hw, 0.02f, z);
        }
        glEnd();
        
        // Walls
        glColor3f(theme.wall.r, theme.wall.g, theme.wall.b);
        
        // North
        glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glVertex3f(-hw, 0, -hl);
        glVertex3f(hw, 0, -hl);
        glVertex3f(hw, wallHeight, -hl);
        glVertex3f(-hw, wallHeight, -hl);
        glEnd();
        
        // South
        glBegin(GL_QUADS);
        glNormal3f(0, 0, -1);
        glVertex3f(-hw, 0, hl);
        glVertex3f(-hw, wallHeight, hl);
        glVertex3f(hw, wallHeight, hl);
        glVertex3f(hw, 0, hl);
        glEnd();
        
        // East
        glBegin(GL_QUADS);
        glNormal3f(-1, 0, 0);
        glVertex3f(hw, 0, -hl);
        glVertex3f(hw, 0, hl);
        glVertex3f(hw, wallHeight, hl);
        glVertex3f(hw, wallHeight, -hl);
        glEnd();
        
        // West
        glBegin(GL_QUADS);
        glNormal3f(1, 0, 0);
        glVertex3f(-hw, 0, -hl);
        glVertex3f(-hw, wallHeight, -hl);
        glVertex3f(-hw, wallHeight, hl);
        glVertex3f(-hw, 0, hl);
        glEnd();
        
        // Ceiling
        glColor3f(theme.ceiling.r, theme.ceiling.g, theme.ceiling.b);
        glBegin(GL_QUADS);
        glNormal3f(0, -1, 0);
        glVertex3f(-hw, wallHeight, -hl);
        glVertex3f(hw, wallHeight, -hl);
        glVertex3f(hw, wallHeight, hl);
        glVertex3f(-hw, wallHeight, hl);
        glEnd();
        
        // Add some pillars for cover
        drawPillars();
        
        // Level 2: Add lava pools
        if (currentLevel == 2) {
            drawLavaPools();
        }
    }
    
    void drawPillars() {
        float positions[][2] = {
            {-15, -10}, {15, -10}, {-15, 10}, {15, 10},
            {0, -15}, {0, 15}
        };
        
        glColor3f(theme.wall.r * 0.8f, theme.wall.g * 0.8f, theme.wall.b * 0.8f);
        
        for (const auto& pos : positions) {
            glPushMatrix();
            glTranslatef(pos[0], wallHeight / 2, pos[1]);
            glScalef(1.5f, wallHeight, 1.5f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }
    
    void drawLavaPools() {
        float pools[][3] = {
            {-20, 0.05f, -20}, {20, 0.05f, -20},
            {-20, 0.05f, 20}, {20, 0.05f, 20}
        };
        
        // Animated lava color
        static float lavaTime = 0;
        lavaTime += 0.016f;
        float pulse = 0.8f + 0.2f * sinf(lavaTime * 3);
        
        glColor3f(1.0f * pulse, 0.3f * pulse, 0.1f);
        
        for (const auto& pool : pools) {
            glPushMatrix();
            glTranslatef(pool[0], pool[1], pool[2]);
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f(-5, 0, -5);
            glVertex3f(5, 0, -5);
            glVertex3f(5, 0, 5);
            glVertex3f(-5, 0, 5);
            glEnd();
            glPopMatrix();
        }
    }
};

} // namespace Doomers
