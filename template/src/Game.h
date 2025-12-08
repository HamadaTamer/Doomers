// ============================================================================
// DOOMERS - Game.h
// Main game class that manages all systems
// ============================================================================
#ifndef GAME_H
#define GAME_H

#include "GameConfig.h"
#include "Vector3.h"
#include "Camera.h"
#include "Player.h"
#include "Enemy.h"
#include "Level.h"
#include "Collectible.h"
#include "Collision.h"
#include "Lighting.h"
#include "HUD.h"
#include "Menu.h"
#include "Sound.h"
#include "LowPolyModels.h"
#include <glut.h>
#include <time.h>
#include <windows.h>

// Use global debug function from Doomers.cpp
extern void debugLog(const char* msg);
#define GAME_LOG(msg) debugLog(msg)

// Import types from Collision namespace for convenience
using Collision::Ray;
using Collision::Sphere;

class Game {
public:
    // Core systems
    Player player;
    Camera camera;
    Level currentLevel;
    HUD hud;
    Menu menu;
    LightingSystem lighting;
    SoundSystem sound;
    
    // Game state
    GameState state;
    int currentLevelNum;
    float deltaTime;
    int lastUpdateTime;
    
    // Mouse state
    int lastMouseX;
    int lastMouseY;
    bool mouseCaptured;
    int windowCenterX;
    int windowCenterY;
    
    // Shift key tracking (for sprint)
    bool shiftPressed;
    
    // Player keycards collected
    int keycards[MAX_KEYCARDS];
    int numKeycards;
    
    // Interaction hint
    int nearInteractableType; // 0=none, 1=mystery box, 2=parkour, 3=exit door
    int nearInteractableIndex;
    
    // Footstep sound timer
    float footstepTimer;
    bool wasMoving;
    
    // Particle effects
    struct Particle {
        Vector3 pos;
        Vector3 vel;
        float life;
        float r, g, b;
        bool active;
    };
    Particle particles[MAX_PARTICLES];
    
    // Laser bullets system
    struct LaserBullet {
        Vector3 startPos;
        Vector3 endPos;
        float life;
        float r, g, b;
        bool active;
    };
    static const int MAX_LASER_BULLETS = 20;
    LaserBullet laserBullets[MAX_LASER_BULLETS];
    
    // Muzzle flash
    float muzzleFlashTime;
    
    // Victory shake system (for Level 2 - all enemies killed)
    bool victoryShakeActive;
    float victoryShakeTime;
    float victoryShakeDuration;
    float victoryShakeIntensity;
    
    // Epic level transition system
    bool isTransitioning;
    float transitionTime;
    float transitionDuration;
    int transitionPhase; // 0=fadeOut, 1=effects, 2=fadeIn
    int transitionTargetLevel;
    bool transitionLevelLoaded; // Flag to ensure level loads only once
    Vector3 transitionCameraStart;
    Vector3 transitionCameraEnd;
    float transitionShake;
    
    Game() {
        state = STATE_MAIN_MENU;
        currentLevelNum = 0;
        deltaTime = 0.016f;
        lastUpdateTime = 0;
        lastMouseX = -1;
        lastMouseY = -1;
        mouseCaptured = false;
        windowCenterX = WINDOW_WIDTH / 2;
        windowCenterY = WINDOW_HEIGHT / 2;
        shiftPressed = false;
        numKeycards = 0;
        muzzleFlashTime = 0.0f;
        nearInteractableType = 0;
        nearInteractableIndex = -1;
        footstepTimer = 0.0f;
        wasMoving = false;
        victoryShakeActive = false;
        victoryShakeTime = 0.0f;
        victoryShakeDuration = 2.0f;
        victoryShakeIntensity = 0.0f;
        isTransitioning = false;
        transitionTime = 0.0f;
        transitionDuration = 4.0f;
        transitionPhase = 0;
        transitionTargetLevel = 0;
        transitionLevelLoaded = false;
        transitionShake = 0.0f;
        
        for (int i = 0; i < MAX_PARTICLES; i++) {
            particles[i].active = false;
        }
        
        for (int i = 0; i < MAX_LASER_BULLETS; i++) {
            laserBullets[i].active = false;
        }
        
        srand((unsigned int)time(NULL));
    }
    
    void init() {
        // Setup player-camera link
        player.setCamera(&camera);
        
        // ============================================
        // GPU PERFORMANCE OPTIMIZATIONS
        // ============================================
        
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        // Enable backface culling for better performance
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        
        // Lighting
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_NORMALIZE);
        
        // Optimize rendering hints for quality
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
        
        // Enable smooth shading
        glShadeModel(GL_SMOOTH);
        
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        
        // Setup fog for atmosphere
        glEnable(GL_FOG);
        float fogColor[] = {0.02f, 0.02f, 0.05f, 1.0f};
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, 30.0f);
        glFogf(GL_FOG_END, 80.0f);
        glHint(GL_FOG_HINT, GL_FASTEST); // Fast fog for better FPS
        
        // Set initial state
        state = STATE_MAIN_MENU;
        menu.setMenu(MENU_MAIN);
        
        // Start menu music
        sound.playMusic(Sounds::MUSIC_MENU);
        
        lastUpdateTime = glutGet(GLUT_ELAPSED_TIME);
    }
    
    void startGame() {
        GAME_LOG("Game::startGame START\n");
        // Stop any lingering win/lose sounds from previous game
        sound.stopWinLoseSound();
        currentLevelNum = 1;
        GAME_LOG("Game::startGame calling loadLevel(1)\n");
        loadLevel(currentLevelNum);
        GAME_LOG("Game::startGame loadLevel done\n");
        state = STATE_PLAYING;
        captureMouse(true);
        
        // Start in-game music
        GAME_LOG("Game::startGame playing music\n");
        sound.playMusic(Sounds::MUSIC_INGAME);
        GAME_LOG("Game::startGame COMPLETE\n");
    }
    
    void loadLevel(int levelNum) {
        GAME_LOG("Game::loadLevel START\n");
        player.reset();
        GAME_LOG("Game::loadLevel player reset done\n");
        numKeycards = 0;
        
        if (levelNum == 1) {
            GAME_LOG("Game::loadLevel calling currentLevel.loadLevel1()\n");
            currentLevel.loadLevel1();
            GAME_LOG("Game::loadLevel loadLevel1 done\n");
            // Setup facility lighting
            lighting.setupForLevel(LEVEL_1_FACILITY);
            GAME_LOG("Game::loadLevel lighting setup done\n");
            player.currentBoundary = BOUNDARY; // Indoor level boundary
        } else if (levelNum == 2) {
            GAME_LOG("Game::loadLevel calling currentLevel.loadLevel2()\n");
            currentLevel.loadLevel2();
            // Setup hell arena lighting
            lighting.setupForLevel(LEVEL_2_HELL_ARENA);
            player.currentBoundary = BOUNDARY_LEVEL2; // Larger outdoor level boundary
        }
        
        GAME_LOG("Game::loadLevel setting player position\n");
        player.setPosition(currentLevel.playerStart);
        camera.yaw = 0.0f;
        camera.pitch = 0.0f;
        GAME_LOG("Game::loadLevel COMPLETE\n");
        
        // Set fog based on level
        if (levelNum == 1) {
            float fogColor[] = {0.02f, 0.02f, 0.05f, 1.0f};
            glFogfv(GL_FOG_COLOR, fogColor);
            glFogf(GL_FOG_START, 20.0f);
            glFogf(GL_FOG_END, 60.0f);
        } else {
            // Level 2 - Hell fog that changes with day/night
            float fogColor[] = {0.25f, 0.08f, 0.05f, 1.0f}; // Reddish hell fog
            glFogfv(GL_FOG_COLOR, fogColor);
            glFogf(GL_FOG_START, 60.0f);  // Increased for larger level
            glFogf(GL_FOG_END, 150.0f);   // Much farther for open world
        }
    }
    
    void update() {
        // Calculate delta time
        int currentTime = glutGet(GLUT_ELAPSED_TIME);
        deltaTime = (currentTime - lastUpdateTime) / 1000.0f;
        if (deltaTime > 0.1f) deltaTime = 0.1f; // Cap delta time
        lastUpdateTime = currentTime;
        
        // Update based on state
        switch (state) {
            case STATE_MAIN_MENU:
            case STATE_INSTRUCTIONS:
            case STATE_GAME_OVER:
            case STATE_LEVEL_COMPLETE:
            case STATE_WIN:
                menu.update(deltaTime);
                break;
                
            case STATE_PLAYING:
                updateGameplay();
                break;
                
            case STATE_PAUSED:
                menu.update(deltaTime);
                break;
        }
        
        glutPostRedisplay();
    }
    
    void updateGameplay() {
        // Update level transition first
        if (isTransitioning) {
            updateLevelTransition();
            // During transition, don't update gameplay
            if (transitionPhase == 1) return; // Full blackout - skip gameplay update
        }
        
        // Update model animation time
        LowPolyModels::updateAnimationTime(deltaTime);
        
        // Use tracked shift key state for sprinting (hold to sprint)
        player.wantSprint = shiftPressed;
        
        // GRAVITY FIX: Check if player should be falling BEFORE update
        // If player is above floor and not on a platform, they should fall
        float preCheckGround = currentLevel.checkPlatformCollision(player.position, PLAYER_COLLISION_RADIUS);
        if (preCheckGround <= 0.0f && player.position.y > PLAYER_HEIGHT + 0.1f) {
            // Not on any platform - player should be falling
            player.isOnGround = false;
        }
        
        // Update player (applies gravity if isOnGround is false)
        player.update(deltaTime);
        
        // Platform collision - check if player is on a platform AFTER movement
        float groundHeight = currentLevel.checkPlatformCollision(player.position, PLAYER_COLLISION_RADIUS);
        
        // If ground height is 0 (no platform), player should be on the floor
        // If player was on a platform and walked off, they should fall
        if (groundHeight > 0.0f) {
            // Player is above a platform
            if (player.position.y <= groundHeight + PLAYER_HEIGHT && player.velocity.y <= 0) {
                player.position.y = groundHeight + PLAYER_HEIGHT;
                player.velocity.y = 0;
                player.isOnGround = true;
            }
        } else {
            // No platform below - if player is above floor level and not jumping up, apply gravity
            if (player.position.y > PLAYER_HEIGHT && player.velocity.y <= 0) {
                // Player is in the air without a platform - they should fall
                player.isOnGround = false;
            } else if (player.position.y <= PLAYER_HEIGHT) {
                // Player is on the floor
                player.position.y = PLAYER_HEIGHT;
                player.velocity.y = 0;
                player.isOnGround = true;
            }
        }
        
        // Crate/obstacle collision - blocks player movement
        currentLevel.checkCrateCollision(player.position, PLAYER_COLLISION_RADIUS);
        
        // Parkour obstacle collision (only if not currently vaulting)
        if (!player.isDoingParkour) {
            currentLevel.checkParkourObstacleCollision(player.position, PLAYER_COLLISION_RADIUS);
        }
        
        // Interior wall collision
        currentLevel.checkInteriorWallCollision(player.position, PLAYER_COLLISION_RADIUS);
        
        // Door collision
        currentLevel.checkDoorCollision(player.position, PLAYER_COLLISION_RADIUS, keycards, numKeycards);
        
        // Update camera
        camera.update(player.position, player.rotationY);
        
        // Enable/disable third person light based on camera mode
        lighting.setThirdPersonLight(camera.mode == CAMERA_THIRD_PERSON);
        
        // Update level
        currentLevel.update(deltaTime, player.position);
        
        // Update lighting
        lighting.update(deltaTime, camera.eye, camera.getLookDirection());
        
        // Update HUD damage flash
        hud.setDamageFlash(player.damageFlash);
        
        // Check for nearby interactables (E key)
        nearInteractableType = currentLevel.getNearestInteractable(player.position, nearInteractableIndex);
        
        // Footstep sounds
        bool isMoving = player.moveForward || player.moveBackward || player.moveLeft || player.moveRight;
        if (isMoving && player.isOnGround && !player.isDoingParkour) {
            float footstepInterval = player.isSprinting ? 0.28f : 0.42f;
            footstepTimer += deltaTime;
            if (footstepTimer >= footstepInterval) {
                footstepTimer = 0.0f;
                if (player.isSprinting) {
                    sound.playSound(Sounds::SFX_FOOTSTEPS_RUN);
                } else {
                    sound.playSound(Sounds::SFX_FOOTSTEPS_WALK);
                }
            }
        } else {
            footstepTimer = 0.0f;
        }
        wasMoving = isMoving;
        
        // Check collectibles
        for (int i = 0; i < currentLevel.numCollectibles; i++) {
            if (currentLevel.collectibles[i].checkCollection(player.position)) {
                collectItem(currentLevel.collectibles[i]);
            }
        }
        
        // ONE-AT-A-TIME ATTACK SYSTEM
        // Find closest enemy to player and make them the active attacker
        int closestEnemy = -1;
        float closestDist = ENEMY_DETECT_RANGE + 1.0f;
        
        for (int i = 0; i < currentLevel.numEnemies; i++) {
            if (!currentLevel.enemies[i].active || currentLevel.enemies[i].isDead()) {
                currentLevel.enemies[i].isActiveAttacker = false;
                continue;
            }
            
            float dist = currentLevel.enemies[i].position.distanceTo(player.position);
            if (dist < closestDist) {
                closestDist = dist;
                closestEnemy = i;
            }
            
            // Reset all to non-active first
            currentLevel.enemies[i].isActiveAttacker = false;
        }
        
        // Only the closest enemy becomes the active attacker
        if (closestEnemy >= 0 && closestDist < ENEMY_DETECT_RANGE) {
            currentLevel.enemies[closestEnemy].isActiveAttacker = true;
        }
        
        // Check enemy attacks - only active attacker can attack
        for (int i = 0; i < currentLevel.numEnemies; i++) {
            if (!currentLevel.enemies[i].isActiveAttacker) continue;
            
            if (currentLevel.enemies[i].canAttack()) {
                float dist = currentLevel.enemies[i].position.distanceTo(player.position);
                if (dist < ENEMY_ATTACK_RANGE) {
                    // Calculate knockback direction (from enemy to player)
                    Vector3 attackDir = player.position - currentLevel.enemies[i].position;
                    attackDir.y = 0; // Only horizontal knockback
                    player.takeDamage(currentLevel.enemies[i].damage, attackDir);
                    currentLevel.enemies[i].performAttack();
                    sound.playSound(Sounds::SFX_PLAYER_HURT);
                }
            }
        }
        
        // Check BOSS PROJECTILE hits (uses per-projectile damage from config)
        for (int i = 0; i < currentLevel.numEnemies; i++) {
            if (currentLevel.enemies[i].type == ENEMY_BOSS && currentLevel.enemies[i].active) {
                int projDamage = currentLevel.enemies[i].checkProjectileHitDamage(player.position, 1.2f);
                if (projDamage > 0) {
                    // Hit by boss projectile!
                    Vector3 knockbackDir = player.position - currentLevel.enemies[i].position;
                    knockbackDir.y = 0;
                    player.takeDamage(projDamage, knockbackDir);
                    sound.playSound(Sounds::SFX_PLAYER_HURT);
                }
            }
        }
        
        // Update enemy health bar visibility based on line of sight
        updateEnemyHealthBarVisibility();
        
        // Check if exit door was just activated (all enemies killed)
        if (currentLevel.exitDoorJustActivated) {
            currentLevel.exitDoorJustActivated = false;
            sound.playSound(Sounds::SFX_THUNDER); // Epic sound for exit door reveal
        }
        
        // Update particles
        updateParticles();
        
        // Update laser bullets
        updateLaserBullets();
        
        // Update muzzle flash
        if (muzzleFlashTime > 0) {
            muzzleFlashTime -= deltaTime;
        }
        
        // Check level 2 day/night cycle
        if (currentLevel.levelID == LEVEL_2_HELL_ARENA) {
            float progress = currentLevel.levelTime / currentLevel.maxTime;
            lighting.updateDayNightCycle(progress);
        }
        
        // Check win/lose conditions (but not if we're already transitioning!)
        if (player.isDead()) {
            onGameOver();
        } else if (!isTransitioning && !victoryShakeActive && currentLevel.isComplete()) {
            // For Level 2, trigger victory shake instead of direct win
            if (currentLevel.levelID == LEVEL_2_HELL_ARENA) {
                startVictoryShake();
            } else {
                onLevelComplete();
            }
        } else if (currentLevel.isTimeUp()) {
            onGameOver();
        }
        
        // Update victory shake if active
        if (victoryShakeActive) {
            updateVictoryShake();
        }
        
        // Check lava damage in level 2 with invincibility frames
        if (currentLevel.hasLava) {
            bool inLava = player.position.y < currentLevel.lavaHeight + PLAYER_HEIGHT + 0.5f;
            player.updateLavaState(inLava, deltaTime);
            
            if (inLava && !player.isInLavaInvincible()) {
                player.takeLavaDamage(8); // Damage with invincibility frames and upward boost
                // Spawn lava splash particles
                for (int i = 0; i < 10; i++) {
                    spawnParticle(player.position, 
                        Vector3((rand() % 100 - 50) / 100.0f, 2.0f + (rand() % 100) / 50.0f, (rand() % 100 - 50) / 100.0f),
                        1.0f, 0.3f, 0.0f);
                }
            }
        }
    }
    
    void collectItem(Collectible& item) {
        switch (item.type) {
            case COLLECT_HEALTH:
                player.heal(item.value);
                player.addScore(10);
                break;
            case COLLECT_AMMO:
                player.addAmmo(item.value);
                player.addScore(10);
                break;
            case COLLECT_KEYCARD:
                if (numKeycards < MAX_KEYCARDS) {
                    keycards[numKeycards++] = item.keycardID;
                    player.addScore(50);
                }
                break;
            case COLLECT_SPEED_BOOST:
                player.activateSpeedBoost((float)item.value);
                player.addScore(25);
                break;
            case COLLECT_DAMAGE_BOOST:
                player.activateDamageBoost((float)item.value);
                player.addScore(25);
                break;
            case COLLECT_INVINCIBILITY:
                player.activateInvincibility((float)item.value);
                player.addScore(50);
                break;
            case COLLECT_MAX_AMMO:
                player.setMaxAmmo();
                player.addScore(30);
                break;
            case COLLECT_SHIELD:
                player.activateShield((float)item.value);
                player.addScore(50);
                break;
        }
        sound.playSound(Sounds::SFX_BUTTON_CLICK); // Use as pickup sound
        
        // Spawn particles
        for (int i = 0; i < 10; i++) {
            Vector3 vel(
                ((float)rand() / RAND_MAX - 0.5f) * 2.0f,
                (float)rand() / RAND_MAX * 3.0f,
                ((float)rand() / RAND_MAX - 0.5f) * 2.0f
            );
            spawnParticle(item.position + Vector3(0, 0.5f, 0), vel, 0.2f, 0.8f, 0.2f);
        }
    }
    
    // Check if a bullet ray hits any wall and return the hit distance
    // Returns -1 if no wall hit, otherwise returns closest wall hit distance
    float checkBulletWallCollision(const Ray& ray, float maxDist) {
        float closestHit = maxDist + 1.0f; // Start beyond max range
        bool anyHit = false;
        
        float halfFloor = currentLevel.floorSize / 2.0f;
        float wallH = currentLevel.wallHeight;
        
        // Build wall AABBs (same as in Level's collision check)
        AABB walls[] = {
            // === OUTER ARENA WALLS ===
            // North wall (back)
            AABB::fromCenter(Vector3(0, wallH/2, -halfFloor), Vector3(halfFloor, wallH/2, 1.5f)),
            // South wall (front)
            AABB::fromCenter(Vector3(0, wallH/2, halfFloor), Vector3(halfFloor, wallH/2, 1.5f)),
            // East wall (right)
            AABB::fromCenter(Vector3(halfFloor, wallH/2, 0), Vector3(1.5f, wallH/2, halfFloor)),
            // West wall (left)
            AABB::fromCenter(Vector3(-halfFloor, wallH/2, 0), Vector3(1.5f, wallH/2, halfFloor)),
            
            // === INTERIOR WALLS (Facility Level 1) ===
            // Security (SW) - East wall
            AABB::fromCenter(Vector3(-5, wallH/2, -20), Vector3(1.5f, wallH/2, 12)),
            // Security (SW) - North wall
            AABB::fromCenter(Vector3(-20, wallH/2, -5), Vector3(9, wallH/2, 1.5f)),
            // Research (NW) - South wall  
            AABB::fromCenter(Vector3(-25, wallH/2, 5), Vector3(10, wallH/2, 1.5f)),
            // Research (NW) - East wall
            AABB::fromCenter(Vector3(-5, wallH/2, 20), Vector3(1.5f, wallH/2, 12)),
            // Containment (SE) - West wall
            AABB::fromCenter(Vector3(5, wallH/2, -20), Vector3(1.5f, wallH/2, 12)),
            // Containment (SE) - North wall
            AABB::fromCenter(Vector3(20, wallH/2, -5), Vector3(9, wallH/2, 1.5f)),
            // Reactor (NE) - South wall
            AABB::fromCenter(Vector3(25, wallH/2, 5), Vector3(8, wallH/2, 1.5f)),
            // Reactor (NE) - West wall
            AABB::fromCenter(Vector3(5, wallH/2, 20), Vector3(1.5f, wallH/2, 10))
        };
        
        int numWalls = sizeof(walls) / sizeof(AABB);
        
        // Check ray against each wall
        for (int i = 0; i < numWalls; i++) {
            float tMin, tMax;
            if (ray.intersects(walls[i], tMin, tMax)) {
                if (tMin > 0 && tMin < closestHit && tMin < maxDist) {
                    closestHit = tMin;
                    anyHit = true;
                }
            }
        }
        
        // Also check floor and ceiling (prevent shooting through)
        AABB floor = AABB::fromCenter(Vector3(0, -0.5f, 0), Vector3(halfFloor, 0.5f, halfFloor));
        AABB ceiling = AABB::fromCenter(Vector3(0, wallH + 0.5f, 0), Vector3(halfFloor, 0.5f, halfFloor));
        
        float tMin, tMax;
        if (ray.intersects(floor, tMin, tMax)) {
            if (tMin > 0 && tMin < closestHit && tMin < maxDist) {
                closestHit = tMin;
                anyHit = true;
            }
        }
        if (ray.intersects(ceiling, tMin, tMax)) {
            if (tMin > 0 && tMin < closestHit && tMin < maxDist) {
                closestHit = tMin;
                anyHit = true;
            }
        }
        
        return anyHit ? closestHit : -1.0f;
    }
    
    void shoot() {
        if (!player.canFire((float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f)) return;
        
        player.fire((float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f);
        muzzleFlashTime = 0.08f;
        
        // Play shooting sound
        sound.playSound(Sounds::SFX_AR_FIRE);
        
        // Raycast for hit detection
        Ray shootRay(player.getShootOrigin(), player.getShootDirection());
        
        float closestHit = WEAPON_RANGE;
        int hitEnemy = -1;
        bool hitWall = false;
        
        // ====== CHECK WALL COLLISION FIRST ======
        // This prevents bullets from passing through walls!
        float wallHitDist = checkBulletWallCollision(shootRay, WEAPON_RANGE);
        if (wallHitDist > 0 && wallHitDist < closestHit) {
            closestHit = wallHitDist;
            hitWall = true;
        }
        
        // Check crates/obstacles
        for (int i = 0; i < currentLevel.numCrates; i++) {
            float tMin, tMax;
            if (shootRay.intersects(currentLevel.crates[i].bounds, tMin, tMax)) {
                if (tMin > 0 && tMin < closestHit) {
                    closestHit = tMin;
                    hitWall = true;
                    hitEnemy = -1;
                }
            }
        }
        
        // Check parkour obstacles
        for (int i = 0; i < currentLevel.numParkourObstacles; i++) {
            float tMin, tMax;
            if (shootRay.intersects(currentLevel.parkourObstacles[i].bounds, tMin, tMax)) {
                if (tMin > 0 && tMin < closestHit) {
                    closestHit = tMin;
                    hitWall = true;
                    hitEnemy = -1;
                }
            }
        }
        
        // Check platforms
        for (int i = 0; i < currentLevel.numPlatforms; i++) {
            float tMin, tMax;
            if (shootRay.intersects(currentLevel.platforms[i].bounds, tMin, tMax)) {
                if (tMin > 0 && tMin < closestHit) {
                    closestHit = tMin;
                    hitWall = true;
                    hitEnemy = -1;
                }
            }
        }
        
        // ====== NOW CHECK ENEMIES (only if bullet hasn't already hit a wall closer) ======
        for (int i = 0; i < currentLevel.numEnemies; i++) {
            if (!currentLevel.enemies[i].active || currentLevel.enemies[i].isDead()) continue;
            
            // Create enemy bounding sphere - CENTER MASS (chest/torso area)
            // Position the hitbox at center of body for better hit detection
            float enemyHeight = 1.5f;  // Default enemy height
            float enemyRadius = 1.2f;  // LARGER hitbox for easier hits
            
            if (currentLevel.enemies[i].type == ENEMY_DEMON) {
                enemyHeight = 1.8f;    // Demons are taller
                enemyRadius = 1.4f;    // Larger hitbox
            } else if (currentLevel.enemies[i].type == ENEMY_BOSS) {
                enemyHeight = 2.0f;
                enemyRadius = 1.8f;
            }
            
            // Center the sphere on the enemy's chest, not feet
            Vector3 enemyCenter = currentLevel.enemies[i].position + Vector3(0, enemyHeight, 0);
            
            Sphere enemySphere(enemyCenter, enemyRadius);
            float hitDist = shootRay.intersects(enemySphere);
            
            if (hitDist > 0 && hitDist < closestHit) {
                closestHit = hitDist;
                hitEnemy = i;
                hitWall = false; // Enemy is closer than wall
            }
        }
        
        // Spawn laser bullet effect
        Vector3 laserStart = player.getShootOrigin();
        Vector3 laserEnd = shootRay.getPoint(closestHit);
        spawnLaserBullet(laserStart, laserEnd, 0.0f, 1.0f, 0.3f); // Green laser
        
        // If hit wall/obstacle, spawn spark particles
        if (hitWall && hitEnemy < 0) {
            Vector3 hitPoint = shootRay.getPoint(closestHit);
            for (int i = 0; i < 3; i++) {
                Vector3 vel(
                    ((float)rand() / RAND_MAX - 0.5f) * 2.0f,
                    (float)rand() / RAND_MAX * 1.5f,
                    ((float)rand() / RAND_MAX - 0.5f) * 2.0f
                );
                spawnParticle(hitPoint, vel, 1.0f, 0.8f, 0.2f); // Yellow/orange sparks
            }
        }
        
        if (hitEnemy >= 0) {
            // Apply damage with damage boost multiplier
            int damage = (int)(WEAPON_DAMAGE * player.getDamageMultiplier());
            currentLevel.enemies[hitEnemy].takeDamage(damage);
            
            // Spawn blood particles
            Vector3 hitPoint = shootRay.getPoint(closestHit);
            for (int i = 0; i < 5; i++) {
                Vector3 vel(
                    ((float)rand() / RAND_MAX - 0.5f) * 3.0f,
                    (float)rand() / RAND_MAX * 2.0f,
                    ((float)rand() / RAND_MAX - 0.5f) * 3.0f
                );
                spawnParticle(hitPoint, vel, 0.6f, 0.0f, 0.0f);
            }
            
            // Hit sound
            sound.playSound(Sounds::SFX_ENEMY_HIT);
            
            if (currentLevel.enemies[hitEnemy].isDead()) {
                player.addScore(currentLevel.enemies[hitEnemy].scoreValue);
                player.enemiesKilled++;
                // Use variation in death sounds
                if (rand() % 2 == 0) {
                    sound.playSound(Sounds::SFX_ENEMY_DEATH);
                } else {
                    sound.playSound(Sounds::SFX_ENEMY_DEATH_2);
                }
            }
        }
    }
    
    void spawnParticle(const Vector3& pos, const Vector3& vel, float r, float g, float b) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) {
                particles[i].pos = pos;
                particles[i].vel = vel;
                particles[i].life = 1.0f;
                particles[i].r = r;
                particles[i].g = g;
                particles[i].b = b;
                particles[i].active = true;
                break;
            }
        }
    }
    
    void spawnLaserBullet(const Vector3& start, const Vector3& end, float r, float g, float b) {
        for (int i = 0; i < MAX_LASER_BULLETS; i++) {
            if (!laserBullets[i].active) {
                laserBullets[i].startPos = start;
                laserBullets[i].endPos = end;
                laserBullets[i].life = 0.25f; // Increased - laser visible for 250ms
                laserBullets[i].r = r;
                laserBullets[i].g = g;
                laserBullets[i].b = b;
                laserBullets[i].active = true;
                break;
            }
        }
    }
    
    void updateLaserBullets() {
        for (int i = 0; i < MAX_LASER_BULLETS; i++) {
            if (laserBullets[i].active) {
                laserBullets[i].life -= deltaTime;
                if (laserBullets[i].life <= 0) {
                    laserBullets[i].active = false;
                }
            }
        }
    }
    
    // Check if there's a clear line of sight between player and enemy (no walls blocking)
    bool hasLineOfSight(const Vector3& from, const Vector3& to) {
        // Create a ray from player to enemy
        Vector3 dir = to - from;
        float dist = dir.length();
        if (dist < 0.1f) return true;
        dir = dir.normalize();
        
        Ray ray(from, dir);
        
        // Check against crates (obstacles)
        for (int i = 0; i < currentLevel.numCrates; i++) {
            float tMin, tMax;
            if (ray.intersects(currentLevel.crates[i].bounds, tMin, tMax)) {
                if (tMin > 0 && tMin < dist) {
                    return false;
                }
            }
        }
        
        // Check against parkour obstacles
        for (int i = 0; i < currentLevel.numParkourObstacles; i++) {
            float tMin, tMax;
            if (ray.intersects(currentLevel.parkourObstacles[i].bounds, tMin, tMax)) {
                if (tMin > 0 && tMin < dist) {
                    return false;
                }
            }
        }
        
        // Check against platforms (if enemy is below and player above, or vice versa)
        for (int i = 0; i < currentLevel.numPlatforms; i++) {
            float tMin, tMax;
            if (ray.intersects(currentLevel.platforms[i].bounds, tMin, tMax)) {
                if (tMin > 0 && tMin < dist) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    // Update enemy health bar visibility based on line of sight
    void updateEnemyHealthBarVisibility() {
        for (int i = 0; i < currentLevel.numEnemies; i++) {
            if (!currentLevel.enemies[i].active || currentLevel.enemies[i].isDead()) {
                currentLevel.enemies[i].showHealthBar = false;
                continue;
            }
            
            // Check line of sight from player to enemy
            Vector3 enemyPos = currentLevel.enemies[i].position + Vector3(0, 1.0f, 0); // Enemy center
            currentLevel.enemies[i].showHealthBar = hasLineOfSight(player.position, enemyPos);
        }
    }
    
    void updateParticles() {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].active) {
                particles[i].pos = particles[i].pos + particles[i].vel * deltaTime;
                particles[i].vel.y -= 5.0f * deltaTime; // Gravity
                particles[i].life -= deltaTime * 2.0f;
                
                if (particles[i].life <= 0) {
                    particles[i].active = false;
                }
            }
        }
    }
    
    // Handle E key interaction with mystery boxes, parkour obstacles, and exit door
    void handleInteraction() {
        if (player.isDoingParkour) return; // Can't interact while vaulting
        
        if (nearInteractableType == 0 || nearInteractableIndex < 0) {
            return; // Nothing nearby
        }
        
        switch (nearInteractableType) {
            case 1: // Mystery Box
            {
                Crate& crate = currentLevel.crates[nearInteractableIndex];
                if (!crate.isOpened) {
                    // Open the mystery box
                    crate.tryOpen();
                    sound.playSound(Sounds::SFX_BUTTON_CLICK);
                    
                    // Spawn particles
                    for (int i = 0; i < 8; i++) {
                        Vector3 vel(
                            ((float)rand() / RAND_MAX - 0.5f) * 2.0f,
                            (float)rand() / RAND_MAX * 3.0f + 1.0f,
                            ((float)rand() / RAND_MAX - 0.5f) * 2.0f
                        );
                        spawnParticle(crate.position + Vector3(0, 0.8f, 0), vel, 0.3f, 0.6f, 1.0f);
                    }
                } else if (!crate.contentCollected && crate.openAnimProgress > 0.8f) {
                    // Collect content from opened box
                    MysteryBoxContent content = crate.collectContent();
                    switch (content) {
                        case MYSTERY_HEALTH:
                            player.heal(30);
                            player.addScore(15);
                            sound.playSound(Sounds::SFX_BUTTON_CLICK);
                            // Green particles for health
                            for (int i = 0; i < 6; i++) {
                                Vector3 vel(
                                    ((float)rand() / RAND_MAX - 0.5f) * 1.5f,
                                    (float)rand() / RAND_MAX * 2.0f,
                                    ((float)rand() / RAND_MAX - 0.5f) * 1.5f
                                );
                                spawnParticle(crate.position + Vector3(0, 1.0f, 0), vel, 0.2f, 0.9f, 0.3f);
                            }
                            break;
                        case MYSTERY_AMMO:
                            player.addAmmo(25);
                            player.addScore(15);
                            sound.playSound(Sounds::SFX_RELOAD);
                            // Yellow particles for ammo
                            for (int i = 0; i < 6; i++) {
                                Vector3 vel(
                                    ((float)rand() / RAND_MAX - 0.5f) * 1.5f,
                                    (float)rand() / RAND_MAX * 2.0f,
                                    ((float)rand() / RAND_MAX - 0.5f) * 1.5f
                                );
                                spawnParticle(crate.position + Vector3(0, 1.0f, 0), vel, 0.9f, 0.8f, 0.2f);
                            }
                            break;
                        case MYSTERY_NOTHING:
                        default:
                            // Gray particles for nothing
                            for (int i = 0; i < 4; i++) {
                                Vector3 vel(
                                    ((float)rand() / RAND_MAX - 0.5f) * 1.0f,
                                    (float)rand() / RAND_MAX * 1.5f,
                                    ((float)rand() / RAND_MAX - 0.5f) * 1.0f
                                );
                                spawnParticle(crate.position + Vector3(0, 1.0f, 0), vel, 0.5f, 0.5f, 0.5f);
                            }
                            break;
                    }
                }
                break;
            }
            case 2: // Parkour Obstacle
            {
                ParkourObstacle& obstacle = currentLevel.parkourObstacles[nearInteractableIndex];
                // Start parkour vault animation
                player.startParkour(obstacle.position, obstacle.height, obstacle.depth, obstacle.rotation);
                sound.playSound(Sounds::SFX_FOOTSTEPS_RUN);
                break;
            }
            case 3: // Exit Door
            {
                if (currentLevel.exitDoor.isActive) {
                    currentLevel.exitDoor.tryOpen();
                    sound.playSound(Sounds::SFX_SHOCKWAVE);
                    // DIRECTLY trigger level transition!
                    currentLevel.objectiveReached = true;
                }
                break;
            }
        }
    }
    
    // ============================================
    // VICTORY SHAKE SYSTEM (Level 2 - All Enemies Killed)
    // ============================================
    void startVictoryShake() {
        victoryShakeActive = true;
        victoryShakeTime = 0.0f;
        victoryShakeDuration = 2.5f;  // 2.5 seconds of epic shake
        victoryShakeIntensity = 1.0f;
        
        // Play epic victory sound
        sound.playSound(Sounds::SFX_SHOCKWAVE);
        
        // Start camera shake
        camera.addShake(0.8f, victoryShakeDuration);
        
        // Spawn victory particles around player
        for (int i = 0; i < 50; i++) {
            float angle = (float)i / 50.0f * 6.28318f;
            float radius = 5.0f + (rand() % 100) / 10.0f;
            Vector3 pos = player.position;
            pos.x += cosf(angle) * radius;
            pos.z += sinf(angle) * radius;
            pos.y += (rand() % 100) / 20.0f;
            
            spawnParticle(pos, 
                Vector3(cosf(angle) * 2.0f, 3.0f + (rand() % 100) / 30.0f, sinf(angle) * 2.0f),
                1.0f, 0.8f, 0.2f);
        }
    }
    
    void updateVictoryShake() {
        victoryShakeTime += deltaTime;
        
        // Calculate shake intensity (starts strong, fades out)
        float progress = victoryShakeTime / victoryShakeDuration;
        victoryShakeIntensity = (1.0f - progress) * 0.8f;
        
        // Apply ongoing shake
        if (progress < 1.0f) {
            // Spawn particles throughout the shake
            if (rand() % 5 == 0) {
                float angle = (float)(rand() % 628) / 100.0f;
                float radius = 10.0f + (rand() % 150) / 10.0f;
                Vector3 pos = player.position;
                pos.x += cosf(angle) * radius;
                pos.z += sinf(angle) * radius;
                pos.y += (rand() % 50) / 10.0f;
                
                // Golden victory particles
                spawnParticle(pos, 
                    Vector3(0, 4.0f + (rand() % 100) / 50.0f, 0),
                    1.0f, 0.9f, 0.1f);
            }
        }
        
        // When shake complete, trigger victory
        if (victoryShakeTime >= victoryShakeDuration) {
            victoryShakeActive = false;
            victoryShakeIntensity = 0.0f;
            onLevelComplete();
        }
    }
    
    void onGameOver() {
        state = STATE_GAME_OVER;
        menu.setMenu(MENU_GAME_OVER);
        menu.finalScore = player.score;
        menu.enemiesKilled = player.enemiesKilled;
        menu.timeElapsed = currentLevel.levelTime;
        captureMouse(false);
        // Play lose sound! (MP3 needs MCI)
        sound.stopMusic();
        sound.playSound(Sounds::SFX_PLAYER_DEAD);
        sound.playSoundMP3(Sounds::SFX_LOSE);
    }
    
    void onLevelComplete() {
        // Prevent multiple calls
        if (isTransitioning) return;
        
        if (currentLevelNum == 2) {
            // Game won!
            state = STATE_WIN;
            menu.setMenu(MENU_WIN);
            menu.finalScore = player.score;
            menu.enemiesKilled = player.enemiesKilled;
            menu.timeElapsed = currentLevel.levelTime;
            // Play WIN sound! (MP3 needs MCI)
            sound.stopMusic();
            sound.playSoundMP3(Sounds::SFX_WIN);
            captureMouse(false);
        } else {
            // SKIP THE MENU - Go directly to epic transition animation!
            startLevelTransition(currentLevelNum + 1);
        }
    }
    
    void nextLevel() {
        // Start epic transition animation!
        startLevelTransition(currentLevelNum + 1);
    }
    
    // Begin the epic level transition
    void startLevelTransition(int targetLevel) {
        isTransitioning = true;
        transitionTime = 0.0f;
        transitionDuration = 4.5f; // Total transition time
        transitionPhase = 0;
        transitionTargetLevel = targetLevel;
        transitionLevelLoaded = false; // Reset - level not loaded yet
        transitionCameraStart = camera.eye;
        transitionShake = 0.0f;
        
        // Play epic transition sound
        sound.playSound(Sounds::SFX_SHOCKWAVE);
        
        state = STATE_PLAYING; // Keep playing state during transition
        captureMouse(false); // Release mouse during transition
    }
    
    // Update the epic level transition
    void updateLevelTransition() {
        if (!isTransitioning) return;
        
        transitionTime += deltaTime;
        float progress = transitionTime / transitionDuration;
        
        // Phase 0: Dramatic zoom out + shake + fade (0-40% of time)
        // Phase 1: Hold black + particle effects (40-60%)
        // Phase 2: Fade in new level + zoom in (60-100%)
        
        if (progress < 0.4f) {
            transitionPhase = 0;
            float phaseProgress = progress / 0.4f;
            
            // Exponential shake buildup
            transitionShake = phaseProgress * phaseProgress * 0.5f;
            
            // Camera pulls back dramatically
            float pullback = phaseProgress * phaseProgress * 15.0f;
            camera.eye.y = transitionCameraStart.y + pullback;
            
        } else if (progress < 0.6f) {
            transitionPhase = 1;
            
            // At peak - load the new level (only once!)
            if (!transitionLevelLoaded) {
                transitionLevelLoaded = true;
                
                // Actually load the new level now
                currentLevelNum = transitionTargetLevel;
                if (currentLevelNum > 2) currentLevelNum = 2;
                
                // Keep player stats
                int savedScore = player.score;
                int savedKills = player.enemiesKilled;
                int savedAmmo = player.ammo;
                int savedHealth = player.health;
                
                loadLevel(currentLevelNum);
                
                // Restore stats
                player.score = savedScore;
                player.enemiesKilled = savedKills;
                player.ammo = savedAmmo;
                player.health = savedHealth;
                
                transitionCameraStart = camera.eye;
                
                // Play arrival sound
                sound.playSound(Sounds::SFX_THUNDER);
            }
            
            // Maximum shake
            transitionShake = 0.3f;
            
        } else {
            transitionPhase = 2;
            float phaseProgress = (progress - 0.6f) / 0.4f;
            
            // Shake reduces
            transitionShake = 0.3f * (1.0f - phaseProgress);
            
            // Camera settles down
            float settle = (1.0f - phaseProgress) * 8.0f;
            camera.eye.y = player.position.y + PLAYER_HEIGHT + settle;
        }
        
        // Transition complete
        if (progress >= 1.0f) {
            isTransitioning = false;
            transitionPhase = 0;
            transitionShake = 0.0f;
            captureMouse(true);
            
            // Camera will be reset by normal player update
        }
    }
    
    // Draw the epic transition effects
    void drawLevelTransitionEffects() {
        if (!isTransitioning) return;
        
        float progress = transitionTime / transitionDuration;
        
        // Setup 2D overlay
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float alpha = 0.0f;
        
        if (transitionPhase == 0) {
            // Fade to black with vignette
            float phaseProgress = progress / 0.4f;
            alpha = phaseProgress * phaseProgress;
            
        } else if (transitionPhase == 1) {
            // Full black with particle effects
            alpha = 1.0f;
            
            // Draw swirling particles
            float effectProgress = (progress - 0.4f) / 0.2f;
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            
            int numParticles = 50;
            for (int i = 0; i < numParticles; i++) {
                float angle = (float)i / numParticles * 3.14159f * 6.0f + transitionTime * 5.0f;
                float radius = 150.0f + sin(angle * 0.5f + i) * 100.0f;
                radius *= (0.5f + effectProgress * 0.5f);
                
                float px = WINDOW_WIDTH / 2 + cos(angle) * radius;
                float py = WINDOW_HEIGHT / 2 + sin(angle) * radius * 0.6f;
                
                float particleAlpha = sin(effectProgress * 3.14159f) * 0.8f;
                float size = 5.0f + sin(angle + transitionTime * 3.0f) * 3.0f;
                
                // Blue-green energy particles
                if (i % 3 == 0) {
                    glColor4f(0.2f, 0.8f, 1.0f, particleAlpha);
                } else if (i % 3 == 1) {
                    glColor4f(0.1f, 1.0f, 0.5f, particleAlpha);
                } else {
                    glColor4f(0.8f, 0.9f, 1.0f, particleAlpha);
                }
                
                glBegin(GL_QUADS);
                glVertex2f(px - size, py - size);
                glVertex2f(px + size, py - size);
                glVertex2f(px + size, py + size);
                glVertex2f(px - size, py + size);
                glEnd();
            }
            
            // Central energy burst
            float burstSize = 50.0f + sin(transitionTime * 10.0f) * 30.0f;
            float burstAlpha = sin(effectProgress * 3.14159f) * 0.6f;
            glColor4f(1.0f, 1.0f, 1.0f, burstAlpha);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
            for (int a = 0; a <= 16; a++) {
                float ang = a * 3.14159f * 2.0f / 16.0f;
                glVertex2f(WINDOW_WIDTH / 2 + cos(ang) * burstSize, 
                          WINDOW_HEIGHT / 2 + sin(ang) * burstSize);
            }
            glEnd();
            
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
        } else {
            // Fade from black
            float phaseProgress = (progress - 0.6f) / 0.4f;
            alpha = 1.0f - phaseProgress * phaseProgress;
        }
        
        // Draw main fade overlay
        glColor4f(0.0f, 0.0f, 0.0f, alpha);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glVertex2f(0, WINDOW_HEIGHT);
        glEnd();
        
        // Draw vignette effect (darker edges)
        float vignetteAlpha = 0.3f + alpha * 0.5f;
        for (int ring = 0; ring < 5; ring++) {
            float innerR = ring * 0.2f;
            float outerR = (ring + 1) * 0.2f;
            float innerA = vignetteAlpha * (1.0f - innerR);
            float outerA = vignetteAlpha * (1.0f - outerR);
            
            float cx = WINDOW_WIDTH / 2;
            float cy = WINDOW_HEIGHT / 2;
            float maxDist = sqrt(cx * cx + cy * cy);
            
            glBegin(GL_QUAD_STRIP);
            for (int a = 0; a <= 32; a++) {
                float ang = a * 3.14159f * 2.0f / 32.0f;
                float dx = cos(ang);
                float dy = sin(ang);
                
                glColor4f(0.0f, 0.0f, 0.0f, innerA);
                glVertex2f(cx + dx * maxDist * innerR, cy + dy * maxDist * innerR * WINDOW_HEIGHT / WINDOW_WIDTH);
                glColor4f(0.0f, 0.0f, 0.0f, outerA);
                glVertex2f(cx + dx * maxDist * outerR, cy + dy * maxDist * outerR * WINDOW_HEIGHT / WINDOW_WIDTH);
            }
            glEnd();
        }
        
        // Draw "ENTERING LEVEL X" text during mid-transition
        if (transitionPhase == 1) {
            float textAlpha = sin((progress - 0.4f) / 0.2f * 3.14159f);
            glColor4f(0.2f, 0.9f, 0.4f, textAlpha);
            
            // Simple level indicator
            glRasterPos2f(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 + 50);
            const char* text = (transitionTargetLevel == 2) ? "ENTERING HELL ARENA" : "LEVEL TRANSITION";
            while (*text) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text++);
            }
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    void render() {
        GAME_LOG("Game::render START\n");
        
        // Set clear color based on current level and boss phase
        if (state == STATE_PLAYING && currentLevelNum == 2) {
            // Check if boss is active (Phase 2)
            bool bossActive = currentLevel.bossEnemyIndex >= 0 && 
                              currentLevel.bossEnemyIndex < currentLevel.numEnemies &&
                              currentLevel.enemies[currentLevel.bossEnemyIndex].active;
            
            if (bossActive) {
                // BOSS PHASE - Blue/stormy sky!
                glClearColor(0.02f, 0.05f, 0.15f, 1.0f);
                float fogColor[] = {0.05f, 0.08f, 0.2f, 1.0f};
                glFogfv(GL_FOG_COLOR, fogColor);
                glFogf(GL_FOG_START, 40.0f);
                glFogf(GL_FOG_END, 120.0f);
            } else {
                // HELL ARENA - Red/dark red background
                glClearColor(0.15f, 0.03f, 0.03f, 1.0f);
                float fogColor[] = {0.2f, 0.05f, 0.02f, 1.0f};
                glFogfv(GL_FOG_COLOR, fogColor);
                glFogf(GL_FOG_START, 50.0f);
                glFogf(GL_FOG_END, 150.0f);
            }
        } else {
            // Default dark blue for Level 1 / menus
            glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
            float fogColor[] = {0.02f, 0.02f, 0.05f, 1.0f};
            glFogfv(GL_FOG_COLOR, fogColor);
            glFogf(GL_FOG_START, 30.0f);
            glFogf(GL_FOG_END, 80.0f);
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (state == STATE_MAIN_MENU || state == STATE_INSTRUCTIONS) {
            GAME_LOG("Game::render drawing menu\n");
            menu.draw();
        }
        else if (state == STATE_GAME_OVER || state == STATE_LEVEL_COMPLETE || state == STATE_WIN) {
            // Render darkened game scene
            GAME_LOG("Game::render drawing game over scene\n");
            renderGameScene();
            menu.draw();
        }
        else if (state == STATE_PAUSED) {
            GAME_LOG("Game::render drawing paused scene\n");
            renderGameScene();
            menu.draw();
        }
        else {
            GAME_LOG("Game::render drawing playing scene\n");
            renderGameScene();
            GAME_LOG("Game::render drawing HUD\n");
            if (!isTransitioning) {
                renderHUD();
            }
            // Draw transition effects on top
            drawLevelTransitionEffects();
        }
        
        GAME_LOG("Game::render swapping buffers\n");
        glutSwapBuffers();
        GAME_LOG("Game::render COMPLETE\n");
    }
    
    void renderGameScene() {
        GAME_LOG("Game::renderGameScene START\n");
        // Setup 3D projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 200.0f);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Apply screen shake during transition
        if (isTransitioning && transitionShake > 0) {
            float shakeX = ((float)rand() / RAND_MAX - 0.5f) * transitionShake * 2.0f;
            float shakeY = ((float)rand() / RAND_MAX - 0.5f) * transitionShake * 2.0f;
            glTranslatef(shakeX, shakeY, 0);
        }
        
        // Apply camera
        GAME_LOG("Game::renderGameScene applying camera\n");
        camera.apply();
        
        // Apply lighting
        GAME_LOG("Game::renderGameScene applying lighting\n");
        lighting.apply();
        
        // Draw level
        GAME_LOG("Game::renderGameScene drawing level\n");
        currentLevel.draw();
        GAME_LOG("Game::renderGameScene level drawn\n");
        
        // Draw player (third person only)
        GAME_LOG("Game::renderGameScene drawing player\n");
        player.draw();
        GAME_LOG("Game::renderGameScene player drawn\n");
        
        // Draw particles
        renderParticles();
        
        // Draw laser bullets
        renderLaserBullets();
        
        // Draw muzzle flash
        if (muzzleFlashTime > 0 && camera.mode == CAMERA_FIRST_PERSON) {
            renderMuzzleFlash();
        }
        
        // Draw weapon in first person
        if (camera.mode == CAMERA_FIRST_PERSON) {
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluPerspective(60.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, 10.0f);
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            glDisable(GL_DEPTH_TEST);
            player.draw();
            glEnable(GL_DEPTH_TEST);
            
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
        }
        GAME_LOG("Game::renderGameScene COMPLETE\n");
    }
    
    void renderParticles() {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].active) {
                glPushMatrix();
                glTranslatef(particles[i].pos.x, particles[i].pos.y, particles[i].pos.z);
                
                // Billboard
                float modelview[16];
                glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
                for (int j = 0; j < 3; j++) {
                    for (int k = 0; k < 3; k++) {
                        modelview[j * 4 + k] = (j == k) ? 1.0f : 0.0f;
                    }
                }
                
                float alpha = particles[i].life;
                float size = 0.1f * particles[i].life;
                
                glColor4f(particles[i].r, particles[i].g, particles[i].b, alpha);
                glBegin(GL_QUADS);
                glVertex3f(-size, -size, 0);
                glVertex3f(size, -size, 0);
                glVertex3f(size, size, 0);
                glVertex3f(-size, size, 0);
                glEnd();
                
                glPopMatrix();
            }
        }
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }
    
    void renderLaserBullets() {
        for (int i = 0; i < MAX_LASER_BULLETS; i++) {
            if (laserBullets[i].active) {
                float alpha = laserBullets[i].life / 0.25f; // Match the new life value
                
                // Use the new highly visible bullet tracer
                LowPolyModels::drawBulletTracer(
                    laserBullets[i].startPos,
                    laserBullets[i].endPos,
                    laserBullets[i].r,
                    laserBullets[i].g,
                    laserBullets[i].b,
                    alpha
                );
            }
        }
    }
    
    void renderMuzzleFlash() {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluPerspective(60.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, 10.0f);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_DEPTH_TEST);
        glTranslatef(0.3f, -0.2f, -0.6f);
        float flashIntensity = muzzleFlashTime / 0.08f;
        LowPolyModels::drawMuzzleFlash(flashIntensity, 1.5f);
        glEnable(GL_DEPTH_TEST);
        
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
    
    void renderHUD() {
        // Get interaction prompt based on nearby interactable
        const char* interactionPrompt = "";
        if (nearInteractableType == 1) {
            Crate& crate = currentLevel.crates[nearInteractableIndex];
            if (!crate.isOpened) {
                interactionPrompt = "[E] Open Mystery Box";
            } else if (!crate.contentCollected && crate.openAnimProgress > 0.8f) {
                interactionPrompt = "[E] Collect Item";
            }
        } else if (nearInteractableType == 2) {
            interactionPrompt = "[E] Vault Over";
        } else if (nearInteractableType == 3) {
            interactionPrompt = "[E] Open Exit Door";
        }
        
        // Show "All Enemies Eliminated" message when exit opens
        // Level 2 (boss) has no exit - show different message
        if (currentLevel.areAllEnemiesKilled() && !currentLevel.exitDoor.isOpen) {
            const char* eliminatedMsg;
            if (currentLevelNum == 2) {
                eliminatedMsg = interactionPrompt[0] ? interactionPrompt : "BOSS DEFEATED! VICTORY!";
            } else {
                eliminatedMsg = interactionPrompt[0] ? interactionPrompt : "All Enemies Eliminated! Find the Exit!";
            }
            hud.drawWithPrompt(
                player.health, player.maxHealth,
                player.ammo, player.maxAmmo,
                player.score,
                currentLevel.getRemainingTime(),
                currentLevelNum,
                eliminatedMsg,
                player.speedBoostTime, player.damageBoostTime, player.invincibilityPowerupTime,
                player.shieldHealth, player.maxShieldHealth
            );
        } else if (interactionPrompt[0]) {
            hud.drawWithPrompt(
                player.health, player.maxHealth,
                player.ammo, player.maxAmmo,
                player.score,
                currentLevel.getRemainingTime(),
                currentLevelNum,
                interactionPrompt,
                player.speedBoostTime, player.damageBoostTime, player.invincibilityPowerupTime,
                player.shieldHealth, player.maxShieldHealth
            );
        } else {
            hud.draw(
                player.health, player.maxHealth,
                player.ammo, player.maxAmmo,
                player.score,
                currentLevel.getRemainingTime(),
                currentLevelNum,
                player.speedBoostTime, player.damageBoostTime, player.invincibilityPowerupTime,
                player.shieldHealth, player.maxShieldHealth
            );
        }
    }
    
    void captureMouse(bool capture) {
        mouseCaptured = capture;
        if (capture) {
            glutSetCursor(GLUT_CURSOR_NONE);
            windowCenterX = WINDOW_WIDTH / 2;
            windowCenterY = WINDOW_HEIGHT / 2;
            glutWarpPointer(windowCenterX, windowCenterY);
            lastMouseX = windowCenterX;
            lastMouseY = windowCenterY;
        } else {
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
    }
    
    // Input handlers
    void onKeyDown(unsigned char key) {
        if (state == STATE_PLAYING) {
            // Track shift modifier (glutGetModifiers is safe inside input callbacks)
            int modifiers = glutGetModifiers();
            shiftPressed = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
            
            switch (key) {
                case 'w': case 'W': player.moveForward = true; break;
                case 's': case 'S': player.moveBackward = true; break;
                case 'a': case 'A': player.moveLeft = true; break;
                case 'd': case 'D': player.moveRight = true; break;
                case ' ': player.wantJump = true; break;
                case 'e': case 'E':
                    // Interact with nearby objects
                    handleInteraction();
                    break;
                case 'f': case 'F': 
                    lighting.toggleFlashlight(); 
                    break;
                case 'g': case 'G':
                    // Toggle weapon light
                    player.toggleWeaponLight();
                    if (player.weaponLightOn) {
                        sound.playSound(Sounds::SFX_FLASHLIGHT_ON);
                    } else {
                        sound.playSound(Sounds::SFX_FLASHLIGHT_OFF);
                    }
                    break;
                case 'v': case 'V':
                    // Toggle camera mode (first/third person)
                    camera.toggleMode();
                    break;
                case 27: // Escape
                    state = STATE_PAUSED;
                    menu.setMenu(MENU_PAUSE);
                    captureMouse(false);
                    break;
            }
        }
        else if (state == STATE_MAIN_MENU) {
            if (key == 13) { // Enter
                sound.playSound(Sounds::SFX_BUTTON_CLICK);
                switch (menu.getSelected()) {
                    case 0: startGame(); break;
                    case 1:
                        menu.setMenu(MENU_INSTRUCTIONS);
                        state = STATE_INSTRUCTIONS;
                        break;
                    case 2: exit(0); break;
                }
            }
        }
        else if (state == STATE_INSTRUCTIONS) {
            if (key == 13 || key == 27) {
                sound.playSound(Sounds::SFX_BUTTON_CLICK);
                menu.setMenu(MENU_MAIN);
                state = STATE_MAIN_MENU;
            }
        }
        else if (state == STATE_PAUSED) {
            if (key == 13) {
                sound.playSound(Sounds::SFX_BUTTON_CLICK);
                switch (menu.getSelected()) {
                    case 0: // Resume
                        state = STATE_PLAYING;
                        captureMouse(true);
                        break;
                    case 1: // Restart
                        loadLevel(currentLevelNum);
                        state = STATE_PLAYING;
                        captureMouse(true);
                        break;
                    case 2: // Quit to menu
                        state = STATE_MAIN_MENU;
                        menu.setMenu(MENU_MAIN);
                        break;
                }
            }
            else if (key == 27) {
                state = STATE_PLAYING;
                captureMouse(true);
            }
        }
        else if (state == STATE_GAME_OVER) {
            if (key == 13) {
                sound.playSound(Sounds::SFX_BUTTON_CLICK);
                switch (menu.getSelected()) {
                    case 0: // Try again
                        loadLevel(currentLevelNum);
                        state = STATE_PLAYING;
                        captureMouse(true);
                        break;
                    case 1: // Main menu
                        state = STATE_MAIN_MENU;
                        menu.setMenu(MENU_MAIN);
                        break;
                }
            }
        }
        else if (state == STATE_LEVEL_COMPLETE) {
            if (key == 13) {
                sound.playSound(Sounds::SFX_BUTTON_CLICK);
                switch (menu.getSelected()) {
                    case 0: // Continue
                        nextLevel();
                        break;
                    case 1: // Main menu
                        state = STATE_MAIN_MENU;
                        menu.setMenu(MENU_MAIN);
                        break;
                }
            }
        }
        else if (state == STATE_WIN) {
            if (key == 13) {
                sound.playSound(Sounds::SFX_BUTTON_CLICK);
                switch (menu.getSelected()) {
                    case 0: // Play again
                        currentLevelNum = 1;
                        loadLevel(1);
                        state = STATE_PLAYING;
                        captureMouse(true);
                        break;
                    case 1: // Main menu
                        state = STATE_MAIN_MENU;
                        menu.setMenu(MENU_MAIN);
                        break;
                }
            }
        }
    }
    
    void onKeyUp(unsigned char key) {
        if (state == STATE_PLAYING) {
            // Track shift modifier (glutGetModifiers is safe inside input callbacks)
            int modifiers = glutGetModifiers();
            shiftPressed = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
            
            switch (key) {
                case 'w': case 'W': player.moveForward = false; break;
                case 's': case 'S': player.moveBackward = false; break;
                case 'a': case 'A': player.moveLeft = false; break;
                case 'd': case 'D': player.moveRight = false; break;
            }
        }
    }
    
    void onSpecialKeyDown(int key) {
        if (state == STATE_PLAYING) {
            // Track shift modifier (glutGetModifiers is safe inside input callbacks)
            int modifiers = glutGetModifiers();
            shiftPressed = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
            
            switch (key) {
                case GLUT_KEY_UP: player.moveForward = true; break;
                case GLUT_KEY_DOWN: player.moveBackward = true; break;
                case GLUT_KEY_LEFT: player.moveLeft = true; break;
                case GLUT_KEY_RIGHT: player.moveRight = true; break;
            }
        }
        else {
            // Menu navigation
            switch (key) {
                case GLUT_KEY_UP: 
                    menu.selectPrev(); 
                    sound.playSound(Sounds::SFX_BUTTON_HOVER);
                    break;
                case GLUT_KEY_DOWN: 
                    menu.selectNext(); 
                    sound.playSound(Sounds::SFX_BUTTON_HOVER);
                    break;
            }
        }
    }
    
    void onSpecialKeyUp(int key) {
        if (state == STATE_PLAYING) {
            // Track shift modifier (glutGetModifiers is safe inside input callbacks)
            int modifiers = glutGetModifiers();
            shiftPressed = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
            
            switch (key) {
                case GLUT_KEY_UP: player.moveForward = false; break;
                case GLUT_KEY_DOWN: player.moveBackward = false; break;
                case GLUT_KEY_LEFT: player.moveLeft = false; break;
                case GLUT_KEY_RIGHT: player.moveRight = false; break;
            }
        }
    }
    
    void onMouseMove(int x, int y) {
        if (state == STATE_PLAYING && mouseCaptured) {
            // Note: Don't call glutGetModifiers here - it causes warnings
            // shiftPressed is tracked in keyboard callbacks instead
            
            int deltaX = x - windowCenterX;
            int deltaY = y - windowCenterY;
            
            if (deltaX != 0 || deltaY != 0) {
                camera.rotate((float)deltaX, (float)deltaY);
                glutWarpPointer(windowCenterX, windowCenterY);
            }
        }
    }
    
    void onMouseButton(int button, int buttonState, int x, int y) {
        if (state == STATE_PLAYING) {
            if (button == GLUT_LEFT_BUTTON && buttonState == GLUT_DOWN) {
                shoot();
            }
            else if (button == GLUT_RIGHT_BUTTON && buttonState == GLUT_DOWN) {
                camera.toggleMode();
            }
        }
    }
    
    void onResize(int width, int height) {
        if (height == 0) height = 1;
        
        glViewport(0, 0, width, height);
        
        hud.setScreenSize(width, height);
        menu.setScreenSize(width, height);
        
        windowCenterX = width / 2;
        windowCenterY = height / 2;
    }
};

#endif // GAME_H
