# DOOMERS - Complete Code Reference Guide
**GUC DMET 502 Computer Graphics Project - Spring 2025**

> **Purpose:** Quick reference for manual code editing during evaluation. All methods listed are **verified** to exist in the actual source files.

---

## 📁 Project Structure

```
template/
├── Doomers.cpp          # Main entry point, GLUT callbacks, global Game instance
└── src/
    ├── GameConfig.h     # All #define constants and enums
    ├── Game.h           # Core game loop, state management
    ├── Player.h         # Player movement, shooting, powerups
    ├── Camera.h         # FPS/TPS camera system
    ├── Enemy.h          # Enemy AI, Boss attacks
    ├── Level.h          # Level loading, platforms, obstacles
    ├── Collectible.h    # Pickups (health, ammo, powerups)
    ├── Collision.h      # AABB, Sphere, Ray collision
    ├── Lighting.h       # OpenGL lighting, flashlight
    ├── HUD.h            # Health bars, ammo counter, UI
    ├── Menu.h           # Main menu, pause, game over screens
    ├── Sound.h          # Windows MCI audio system
    ├── TextureManager.h # SOIL texture loading
    ├── ModelLoader.h    # Assimp 3D model loading
    ├── LowPolyModels.h  # Procedural OpenGL shapes
    └── Vector3.h        # 3D vector math class
```

---

## 🔧 GameConfig.h - Master Constants

### Player Settings
```cpp
#define PLAYER_HEIGHT 1.8f
#define PLAYER_SPEED 0.15f
#define PLAYER_SPRINT_MULTIPLIER 1.8f
#define PLAYER_JUMP_FORCE 0.25f
#define PLAYER_MAX_HEALTH 100
#define PLAYER_COLLISION_RADIUS 0.5f
#define PLAYER_INVINCIBILITY_TIME 0.5f
```

### Weapon Settings
```cpp
#define WEAPON_DAMAGE 25
#define WEAPON_RANGE 50.0f
#define WEAPON_FIRE_RATE 0.2f
#define MAX_AMMO 100
#define AMMO_PICKUP_AMOUNT 20
#define WEAPON_HEADSHOT_MULTIPLIER 2.0f
```

### Enemy Settings
```cpp
#define MAX_ENEMIES 40
#define ENEMY_SPEED 0.08f
#define ENEMY_DAMAGE 10
#define ENEMY_ATTACK_RANGE 2.5f
#define ENEMY_DETECT_RANGE 20.0f
#define ZOMBIE_HEALTH 50
#define DEMON_HEALTH 80
```

### Boss Settings
```cpp
#define BOSS_HEALTH 300
#define BOSS_DAMAGE 25
#define BOSS_FIREBALL_DAMAGE 15
#define BOSS_METEOR_DAMAGE 20
#define BOSS_GROUNDSLAM_DAMAGE 25
#define BOSS_ROCKET_COOLDOWN 5.0f
#define BOSS_CHARGE_COOLDOWN 15.0f
#define BOSS_GROUNDSLAM_COOLDOWN 12.0f
#define BOSS_METEOR_COOLDOWN 20.0f
```

### Collectibles & Powerups
```cpp
#define HEALTH_PACK_HEAL 25
#define POWERUP_DURATION 12.0f
#define SPEED_BOOST_MULTIPLIER 1.7f
#define DAMAGE_BOOST_MULTIPLIER 2.5f
#define INVINCIBILITY_DURATION 10.0f
```

### Game States (enum GameState)
```cpp
STATE_MAIN_MENU, STATE_INSTRUCTIONS, STATE_PLAYING,
STATE_PAUSED, STATE_LEVEL_COMPLETE, STATE_GAME_OVER, STATE_WIN
```

### Camera Modes (enum CameraMode)
```cpp
CAMERA_FIRST_PERSON, CAMERA_THIRD_PERSON
```

### Level IDs (enum LevelID)
```cpp
LEVEL_MENU = 0, LEVEL_1_FACILITY = 1, LEVEL_2_HELL_ARENA = 2
```

---

## 🎮 Game.h - Core Game Logic (1882 lines)

### Key Members
```cpp
Player player;              // The player instance
Camera camera;              // Camera system
Level currentLevel;         // Active level data
HUD hud;                    // Heads-up display
Menu menu;                  // Menu system
LightingSystem lighting;    // Lighting
SoundSystem sound;          // Audio

GameState state;            // Current game state
int currentLevelNum;        // 1 or 2
float deltaTime;            // Frame time
bool mouseCaptured;         // Mouse look active

// Particles & Effects
Particle particles[MAX_PARTICLES];
LaserBullet laserBullets[20];
float muzzleFlashTime;

// Level Transition
bool isTransitioning;
float transitionTime;
int transitionPhase;
```

### Core Methods
```cpp
void init()                 // Initialize OpenGL, load assets, start at main menu
void startGame()            // Begin Level 1, capture mouse, play music
void loadLevel(int levelNum)// Load Level 1 or 2, reset player, setup lighting
void update()               // Main game loop - calls updateGameplay() if playing
void render()               // Draw scene based on game state
```

### Gameplay Methods
```cpp
void updateGameplay()       // Update player, enemies, collision, win/lose checks
void collectItem(Collectible& item)  // Apply pickup effects (health/ammo/powerups)
void shoot()                // Raycast hit detection, deal damage, spawn particles
float checkBulletWallCollision(const Ray& ray, float maxDist)  // Wall hit check
bool hasLineOfSight(const Vector3& from, const Vector3& to)    // LOS check
```

### Interaction Methods
```cpp
void handleInteraction()    // E key: mystery boxes, parkour vault, exit door
void spawnParticle(...)     // Create particle at position with velocity/color
void spawnLaserBullet(...)  // Create laser projectile
void updateParticles()      // Animate and remove dead particles
void updateLaserBullets()   // Update laser projectiles
```

### State Transition Methods
```cpp
void onGameOver()           // Show game over, play sound
void onLevelComplete()      // Show level complete screen
void nextLevel()            // Transition to Level 2 or show win screen
void startLevelTransition(int targetLevel)  // Begin transition effect
void updateLevelTransition()                // Update transition animation
void startVictoryShake()    // Boss defeat screen shake
void updateVictoryShake()   // Update shake effect
```

### Rendering Methods
```cpp
void renderGameScene()      // Draw level, enemies, player, collectibles
void renderParticles()      // Draw particle effects
void renderLaserBullets()   // Draw laser projectiles
void renderMuzzleFlash()    // Draw gun flash effect
void renderHUD()            // Draw health, ammo, score, crosshair
void drawLevelTransitionEffects()  // Transition animation
```

### Input Handlers
```cpp
void onKeyDown(unsigned char key)   // WASD, Space, E, F, Escape, etc.
void onKeyUp(unsigned char key)
void onSpecialKeyDown(int key)      // Arrow keys for menu
void onSpecialKeyUp(int key)
void onMouseMove(int x, int y)      // Mouse look
void onMouseButton(int button, int state, int x, int y)  // Shoot, toggle camera
void onResize(int w, int h)         // Window resize
void captureMouse(bool capture)     // Enable/disable mouse capture
```

---

## 👤 Player.h - Player System (734 lines)

### Key Members
```cpp
Vector3 position, velocity;
float rotationY, speed;
bool isSprinting, isOnGround;
int health, maxHealth, ammo, maxAmmo;
int score, enemiesKilled;

// Weapon State
float weaponRecoil, weaponBob;
float lastFireTime;
bool isFiring;
float muzzleFlashTimer;

// Damage & Effects
float damageFlash;
float invincibilityTime;
float lavaDamageTimer;
bool isInLava;

// Powerups
float speedBoostTime, damageBoostTime, invincibilityPowerupTime;
bool hasSpeedBoost, hasDamageBoost, hasInvincibility;
float shieldHealth, maxShieldHealth;
bool hasShield;

// Parkour
bool isDoingParkour;
float parkourProgress;
Vector3 parkourStartPos, parkourEndPos;

// Weapon Light
bool weaponLightOn;
float weaponLightIntensity;

// Input Flags
bool moveForward, moveBackward, moveLeft, moveRight;
bool wantJump, wantSprint;

Camera* camera;
```

### Core Methods
```cpp
void reset()                    // Reset all stats for new game
void update(float deltaTime)    // Movement, gravity, input processing
void setPosition(const Vector3& pos)
void setCamera(Camera* cam)
```

### Combat Methods
```cpp
bool canFire(float currentTime)       // Check fire rate cooldown
void fire(float currentTime)          // Trigger weapon animation
void takeDamage(int damage, Vector3 attackDir)  // Damage with shield/knockback
void takeLavaDamage(int damage)       // Lava-specific damage
void updateLavaState(bool inLava, float deltaTime)
bool isInLavaInvincible()
float getDamageMultiplier()           // Returns 1.0 or DAMAGE_BOOST_MULTIPLIER
bool canTakeDamage()                  // Check invincibility
bool isDead()                         // health <= 0
```

### Pickup Methods
```cpp
void heal(int amount)
void addAmmo(int amount)
void setMaxAmmo()                     // Set ammo to MAX_AMMO
void addScore(int points)
```

### Powerup Methods
```cpp
void activateSpeedBoost(float duration)
void activateDamageBoost(float duration)
void activateInvincibility(float duration)
void activateShield(float amount)
```

### Parkour Methods
```cpp
void startParkour(const Vector3& obstaclePos, float obstacleHeight, 
                  float obstacleDepth, float obstacleRotation)
void updateParkour(float deltaTime)   // Smooth vault animation
```

### Rendering & Animation
```cpp
void draw()                           // FPS: weapon only, TPS: full model
void updateWeaponAnimation(float deltaTime)  // Bob and recoil
void toggleWeaponLight()              // Toggle flashlight
```

### Shooting Helpers
```cpp
Vector3 getShootDirection()           // Camera look direction
Vector3 getShootOrigin()              // Gun muzzle position
```

---

## 📷 Camera.h - Camera System (329 lines)

### Key Members
```cpp
Vector3 eye, center, up;              // gluLookAt parameters
float pitch, yaw;                     // Rotation angles
CameraMode mode;                      // FPS or TPS
float sensitivity;                    // Mouse sensitivity

// Third Person Settings
float distance;                       // Distance behind player
float height;                         // Height above player
float lookAheadDist;

// Smoothing
Vector3 smoothEye, smoothCenter;
float smoothSpeed;

// Camera Shake
float shakeIntensity, shakeDuration, shakeTimer;

// Parkour Tilt
float parkourTilt;
```

### Methods
```cpp
void setMode(CameraMode newMode)
void toggleMode()                     // Switch FPS <-> TPS
void rotate(float deltaX, float deltaY)  // Mouse look with pitch clamping

void update(const Vector3& playerPos, float playerRotY, float deltaTime)
void apply()                          // Call gluLookAt

Vector3 getForward()                  // Forward direction (no Y)
Vector3 getRight()                    // Right direction
Vector3 getLookDirection()            // Full look direction with pitch

void addShake(float intensity, float duration)
Vector3 getShakeOffset()              // Random shake offset

void setParkourTilt(float tilt)
void clearParkourTilt()

float getYaw()
float getPitch()
void setYaw(float newYaw)
void adjustDistance(float delta)      // TPS zoom in/out
```

---

## 👹 Enemy.h - Enemy AI System (1002 lines)

### Enums
```cpp
enum EnemyType { ENEMY_ZOMBIE, ENEMY_DEMON, ENEMY_BOSS };
enum EnemyState { ENEMY_IDLE, ENEMY_PATROL, ENEMY_CHASE, ENEMY_ATTACK, ENEMY_HURT, ENEMY_DEAD };
```

### Key Members
```cpp
Vector3 position, velocity;
Vector3 patrolStart, patrolEnd;
float rotationY, speed;
float animPhase;
float hurtTimer, attackCooldown, deathTimer;
int health, maxHealth, damage, scoreValue;
EnemyType type;
EnemyState state;
bool active;
bool isActiveAttacker;
bool showHealthBar;
float damageFlashTimer;

// Boss-specific
BossProjectile projectiles[20];
float specialAbilityCooldown;
float teleportCooldown, groundSlamCooldown, meteorShowerCooldown;
int currentPhase;                     // 1, 2, or 3 based on health
bool isCharging;
float chargeTimer;
Vector3 chargeTarget;
```

### Core Methods
```cpp
void reset()
void init(EnemyType enemyType, const Vector3& pos, 
          const Vector3& patrolA, const Vector3& patrolB)
void update(float deltaTime, const Vector3& playerPos)  // AI state machine
void draw()
void drawHealthBar()
```

### AI Behavior
```cpp
void patrol(float deltaTime)
void chase(float deltaTime, const Vector3& playerPos)
void lookAt(const Vector3& target)
bool canAttack()
void performAttack()
```

### Combat
```cpp
void takeDamage(int dmg)
bool isDead()
```

### Boss Methods
```cpp
void chaseBoss(float deltaTime, const Vector3& playerPos)
void updateBossAbilities(float deltaTime, const Vector3& playerPos)
void fireProjectile(const Vector3& target, int projectileType)
void updateProjectiles(float deltaTime)
void drawProjectiles()
bool checkProjectileHit(const Vector3& playerPos, float hitRadius)
int checkProjectileHitDamage(const Vector3& playerPos, float hitRadius)
```

---

## 🏗️ Level.h - Level Management (3660 lines)

### Key Structures

#### Door
```cpp
struct Door {
    Vector3 position;
    float rotation;
    bool isLocked;
    int requiredKeycard;
    bool isOpen;
    float openAmount;
    AABB bounds;
    
    void updateBounds();
    void update(float deltaTime);
    void draw();
};
```

#### Crate (includes Mystery Boxes)
```cpp
struct Crate {
    Vector3 position;
    float size;
    bool isSciFi;
    AABB bounds;
    
    bool isMysteryBox;
    bool isOpened;
    float openAnimProgress;
    MysteryBoxContent content;  // MYSTERY_NOTHING, MYSTERY_HEALTH, MYSTERY_AMMO
    
    void updateBounds();
    void setAsMysteryBox();
    void update(float deltaTime);
    bool tryOpen();
    MysteryBoxContent collectContent();
    void draw();
    void drawMysteryBox();
};
```

#### ParkourObstacle
```cpp
struct ParkourObstacle {
    Vector3 position;
    float width, height, depth;
    float rotation;
    AABB bounds;
    
    void updateBounds();
    bool isPlayerNearForVault(const Vector3& playerPos, float playerRadius);
    void draw();
};
```

#### ExitDoor
```cpp
struct ExitDoor {
    Vector3 position;
    float rotation;
    bool isActive;
    bool isOpen;
    float openAmount;
    AABB bounds;
    
    void updateBounds();
    void activate();
    void update(float deltaTime);
    bool tryOpen();
    void draw();
};
```

### Level Class Members
```cpp
LevelID levelID;

Enemy enemies[MAX_ENEMIES];
int numEnemies;

Collectible collectibles[...];
int numCollectibles;

Platform platforms[MAX_PLATFORMS];
int numPlatforms;

Crate crates[MAX_CRATES];
int numCrates;

Door doors[MAX_DOORS];
int numDoors;

ParkourObstacle parkourObstacles[10];
int numParkourObstacles;

ExitDoor exitDoor;
bool allEnemiesKilled;
bool exitDoorJustActivated;

// Boss phase
bool bossPhaseStarted;
bool regularEnemiesCleared;
int bossEnemyIndex;

Vector3 playerStart;
Vector3 objective;
float floorSize;
float wallHeight;
bool hasLava;
float lavaHeight;
```

### Level Methods
```cpp
void reset()
void loadLevel1()    // Facility: indoor, platforms, security doors
void loadLevel2()    // Hell Arena: lava, boss fight, floating platforms
void update(float deltaTime, const Vector3& playerPos)
void draw()
void drawEnvironment()
void drawSkybox()

// Collision Queries
bool checkPlatformCollision(...)
bool checkCrateCollision(...)
int getEnemyCount()
int getActiveEnemyCount()
bool areAllEnemiesKilled()
```

---

## 📦 Collectible.h - Pickup System (621 lines)

### Collectible Types
```cpp
enum CollectibleType {
    COLLECT_HEALTH,
    COLLECT_AMMO,
    COLLECT_KEYCARD,
    COLLECT_SPEED_BOOST,
    COLLECT_DAMAGE_BOOST,
    COLLECT_INVINCIBILITY,
    COLLECT_MAX_AMMO,
    COLLECT_SHIELD
};
```

### Collectible Class
```cpp
class Collectible {
public:
    Vector3 position;
    CollectibleType type;
    bool active;
    float rotation;
    float bobPhase;
    float pickupScale;
    bool beingCollected;
    int value;
    int keycardID;
    
    void init(CollectibleType t, const Vector3& pos, int val = 0);
    void update(float deltaTime);
    bool checkCollection(const Vector3& playerPos, float collectRadius = 2.5f);
    void draw();
    void drawGlow();
    
    // Type-specific drawing
    void drawTexturedHealthPack();
    void drawTexturedAmmoBox();
    void drawKeycard();
    void drawSpeedBoost();
    void drawDamageBoost();
    void drawInvincibility();
    void drawMaxAmmo();
    void drawShield();
};
```

---

## 💥 Collision.h - Collision Detection (374 lines)

### AABB (Axis-Aligned Bounding Box)
```cpp
struct AABB {
    Vector3 minPoint, maxPoint;
    
    static AABB fromCenter(const Vector3& center, const Vector3& halfExtents);
    static AABB fromCenterSize(const Vector3& center, const Vector3& halfExtents);
    Vector3 center() const;
    Vector3 halfExtents() const;
    bool contains(const Vector3& point) const;
    bool intersects(const AABB& other) const;
};
```

### Sphere
```cpp
struct Sphere {
    Vector3 center;
    float radius;
    
    bool contains(const Vector3& point) const;
    bool intersects(const Sphere& other) const;
    bool intersects(const AABB& box) const;
};
```

### Ray
```cpp
struct Ray {
    Vector3 origin;
    Vector3 direction;
    
    Vector3 pointAt(float t) const;
    Vector3 getPoint(float t) const;
    bool intersects(const AABB& box, float& tMin, float& tMax) const;
    float intersects(const Sphere& sphere) const;  // Returns distance or -1
    bool intersects(const Sphere& sphere, float& t) const;
};
```

### Platform
```cpp
struct Platform {
    AABB bounds;
    Vector3 center, size;
    bool isMoving;
    float moveProgress, moveSpeed;
    
    Platform(const Vector3& c, const Vector3& s);
    void update(float dt);
    bool isPlayerOnTop(const Vector3& playerPos, float playerRadius, float& groundHeight) const;
};
```

### Helper Functions
```cpp
Vector3 resolveAABBCollision(const AABB& moving, const AABB& stationary);
bool isOnGround(const AABB& player, const AABB& ground);
CollisionResult resolveSphereAABB(const Sphere& sphere, const AABB& box);
```

---

## 💡 Lighting.h - OpenGL Lighting (343 lines)

### Light Class
```cpp
class Light {
public:
    Vector3 position, direction;
    float ambient[4], diffuse[4], specular[4];
    bool isSpotlight;
    float spotCutoff, spotExponent;
    float constantAtt, linearAtt, quadraticAtt;
    bool enabled;
    int lightID;
    
    void setAsPointLight(const Vector3& pos, float r, float g, float b);
    void setAsSpotlight(const Vector3& pos, const Vector3& dir, float cutoff = 30.0f);
    void setColor(float r, float g, float b);
    void apply();
};
```

### LightingSystem Class
```cpp
class LightingSystem {
public:
    Light flashlight;
    Light emergencyLights[4];
    Light ambientLight;
    Light thirdPersonLight;
    float emergencyPhase;
    float dayNightCycle;
    bool isFirstPerson;
    
    void setFirstPersonMode(bool firstPerson);
    void setupForLevel(int levelID);
    void updateDayNightCycle(float progress);
    void update(float deltaTime, const Vector3& playerPos, const Vector3& lookDir);
    void apply();
    void setFlashlightEnabled(bool enabled);
    void toggleFlashlight();
    void setThirdPersonLight(bool enabled);
};
```

---

## 📊 HUD.h - User Interface (1252 lines)

### StyledText Namespace
```cpp
namespace StyledText {
    void drawTextWithShadow(float x, float y, const char* text, void* font,
                            float r, float g, float b, float shadowOffset = 2.0f);
    void drawTextWithOutline(float x, float y, const char* text, void* font,
                             float r, float g, float b, float outlineR, float outlineG, float outlineB);
    void drawTextWithGlow(float x, float y, const char* text, void* font,
                          float r, float g, float b, float glowIntensity = 0.5f);
    int getTextWidth(const char* text, void* font);
}
```

### HUD Class
```cpp
class HUD {
public:
    int screenWidth, screenHeight;
    float damageFlash;
    float lowHealthPulse;
    float animTime;
    void* fontLarge;   // GLUT_BITMAP_TIMES_ROMAN_24
    void* fontMedium;  // GLUT_BITMAP_HELVETICA_18
    void* fontSmall;   // GLUT_BITMAP_HELVETICA_12
    
    void setScreenSize(int width, int height);
    void setDamageFlash(float intensity);
    void update(float deltaTime);
    void beginHUD();                      // Setup 2D projection
    void endHUD();                        // Restore 3D projection
    
    void drawCrosshair(float spread = 0.0f, bool enemyInSight = false);
    void drawHealthBar(int health, int maxHealth);
    void drawShieldBar(float shieldHealth, float maxShieldHealth);
    void drawAmmoCounter(int ammo, int maxAmmo);
    void drawScore(int score);
    void drawTimer(int seconds);
    void drawLevelIndicator(int level, const char* levelName = NULL);
    void drawPowerupIndicators(float speedBoostTime, float damageBoostTime, float invincibilityTime);
    void drawDamageOverlay(float intensity);
    void drawNearDeathEffect(float healthPercent);
    void drawBossHealthBar(const char* name, int health, int maxHealth);
};
```

---

## 📜 Menu.h - Menu System (451 lines)

### Menu Types
```cpp
enum MenuType {
    MENU_MAIN, MENU_PAUSE, MENU_INSTRUCTIONS,
    MENU_GAME_OVER, MENU_LEVEL_COMPLETE, MENU_WIN
};
```

### Menu Class
```cpp
class Menu {
public:
    int screenWidth, screenHeight;
    int selectedOption, maxOptions;
    float animTime;
    MenuType currentMenu;
    int finalScore, enemiesKilled;
    float timeElapsed;
    
    void setScreenSize(int width, int height);
    void update(float deltaTime);
    void selectNext();
    void selectPrev();
    int getSelected() const;
    void setMenu(MenuType type);
    
    void beginMenu();
    void endMenu();
    void draw();
    
    void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18);
    void drawTextCentered(float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18);
    void drawBackground();
    void drawTitle();
    void drawMenuOption(int index, const char* text, float y);
    
    void drawMainMenu();
    void drawPauseMenu();
    void drawInstructions();
    void drawGameOver();
    void drawLevelComplete();
    void drawWinScreen();
};
```

---

## 🔊 Sound.h - Audio System (210 lines)

### SoundSystem Class
```cpp
class SoundSystem {
public:
    bool musicEnabled, sfxEnabled;
    bool musicPlaying;
    char currentMusicPath[512];
    
    void getFullPath(const char* relativePath, char* fullPath, int maxLen);
    void playMusic(const char* filename);    // MCI looping music
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void playSound(const char* filename);    // PlaySound (WAV)
    void playSoundMP3(const char* filename); // MCI (MP3)
    void stopWinLoseSound();
    void playSoundOverlap(const char* filename);  // Multiple sounds at once
    void setMusicVolume(int volume);
    void toggleMusic();
    void toggleSFX();
    void cleanup();
};
```

### Sound Paths (namespace Sounds)
```cpp
// Music
MUSIC_MENU = "res/Audio/MainMenu.mp3"
MUSIC_INGAME = "res/Audio/InGame.mp3"

// Sound Effects
SFX_AR_FIRE = "res/Audio/AR_Fired.wav"
SFX_RELOAD = "res/Audio/Reloading.wav"
SFX_ENEMY_HIT = "res/Audio/EnemyHit.wav"
SFX_ENEMY_DEATH = "res/Audio/EnemyDead.wav"
SFX_PLAYER_HURT = "res/Audio/Hurt.wav"
SFX_FLASHLIGHT_ON = "res/Audio/FlashLightOn.wav"
SFX_WIN = "res/Audio/WIN_SOUND.mp3"
SFX_LOSE = "res/Audio/LOSE_SOUND.mp3"
```

---

## 🔢 Vector3.h - 3D Math

```cpp
struct Vector3 {
    float x, y, z;
    
    Vector3();
    Vector3(float x, float y, float z);
    
    Vector3 operator+(const Vector3& v) const;
    Vector3 operator-(const Vector3& v) const;
    Vector3 operator*(float s) const;
    Vector3 operator/(float s) const;
    
    float length() const;
    float lengthSquared() const;
    Vector3 normalized() const;
    void normalize();
    
    float dot(const Vector3& v) const;
    Vector3 cross(const Vector3& v) const;
    
    static float distance(const Vector3& a, const Vector3& b);
    static Vector3 lerp(const Vector3& a, const Vector3& b, float t);
};
```

---

## ⌨️ Controls Reference

| Action | Key/Mouse |
|--------|-----------|
| Move | W/A/S/D |
| Jump | SPACE |
| Sprint | SHIFT (hold) |
| Shoot | Left Mouse |
| Toggle Camera (FPS/TPS) | Right Mouse or V |
| Flashlight | F |
| Interact (parkour, mystery box, exit) | E |
| Pause | ESC or P |
| Menu Navigation | Arrow Keys + ENTER |

---

## 🎯 Quick Edit Locations

### To change player stats:
`GameConfig.h` → `PLAYER_*` defines

### To change enemy damage/health:
`GameConfig.h` → `ZOMBIE_*`, `DEMON_*`, `BOSS_*` defines

### To change weapon damage:
`GameConfig.h` → `WEAPON_DAMAGE`, `WEAPON_FIRE_RATE`

### To add a new collectible type:
1. `Collectible.h` → Add to `CollectibleType` enum
2. Add draw method in Collectible class
3. `Game.h` → Handle in `collectItem()` method

### To modify level layout:
`Level.h` → `loadLevel1()` or `loadLevel2()` methods

### To change lighting:
`Lighting.h` → `setupForLevel()` or `updateDayNightCycle()`

### To modify HUD elements:
`HUD.h` → `drawHealthBar()`, `drawAmmoCounter()`, etc.

---

## 🔨 Build Instructions

1. Open `OpenGL3DTemplate.sln` in Visual Studio 2022
2. Set configuration to **Debug | x86**
3. Build and run (F5)

Required libraries (already included):
- OpenGL + GLUT
- SOIL (textures)
- Assimp (3D models)
- Windows MCI (audio)

---

*Document generated for DMET 502 evaluation preparation*
