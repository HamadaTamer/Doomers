# DOOMERS - Technical Documentation Part 3
## Level System, Collision, Audio, Lighting, and HUD

---

## 9. Level System (Level.h)

### 9.1 Level IDs
```cpp
enum LevelID {
    LEVEL_1_FACILITY,    // Research Facility (indoor)
    LEVEL_2_HELL_ARENA   // Hell Arena (outdoor with lava)
};
```

### 9.2 Level Structures

#### Door Structure
```cpp
struct Door {
    Vector3 position;     // World position
    float rotation;       // Y-axis rotation (degrees)
    bool isLocked;        // Requires keycard
    int requiredKeycard;  // Which keycard ID (1=blue, 2=red)
    bool isOpen;          // Currently open
    float openAmount;     // Animation progress (0-1)
    AABB bounds;          // Collision bounds
    
    // Methods
    void updateBounds();
    void update(float deltaTime);  // Animate opening
    void draw();                   // Render with texture
};
```

#### Mystery Box System
```cpp
enum MysteryBoxContent {
    MYSTERY_NOTHING = 0,  // 40% chance
    MYSTERY_HEALTH,       // 35% chance  
    MYSTERY_AMMO          // 25% chance
};

struct Crate {
    Vector3 position;
    float size;
    bool isSciFi;         // Use sci-fi texture
    AABB bounds;
    
    // Mystery box properties
    bool isMysteryBox;
    bool isOpened;
    float openAnimProgress;
    MysteryBoxContent content;
    bool contentCollected;
    float glowPhase;      // Glow animation
    
    // Methods
    void setAsMysteryBox();           // Convert to mystery box
    void update(float deltaTime);     // Update animations
    bool tryOpen();                   // Attempt to open
    MysteryBoxContent collectContent(); // Get contents
    void draw();                      // Render (different for mystery)
    void drawMysteryBox();            // Glowing mystery box effect
};
```

#### Parkour Obstacle
```cpp
struct ParkourObstacle {
    Vector3 position;
    float width;          // Default 3.0
    float height;         // Default 1.2 (vault height)
    float depth;          // Default 0.4
    float rotation;       // Y rotation
    AABB bounds;
    
    // Methods
    void updateBounds();  // Recalculate AABB (handles rotation)
    bool isPlayerNearForVault(Vector3 playerPos, float playerRadius);
    void draw();          // Textured barrier with caution stripes
};
```

#### Exit Door / Victory Portal
```cpp
struct ExitDoor {
    Vector3 position;
    float rotation;
    bool isActive;        // Activated when all enemies killed
    bool isOpen;          // Player triggered
    float openAmount;     // Opening animation
    float lightIntensity; // Light glow
    float lightPhase;     // Animation phase
    AABB bounds;
    
    // Methods
    void activate();      // Called when all enemies dead
    void update(float deltaTime);
    bool tryOpen();       // Returns true if active
    void draw();          // Level 1 sliding door
    void drawAsPortal();  // Level 2 mystical portal
};
```

### 9.3 Level Class

#### Properties
```cpp
class Level {
    LevelID levelID;
    
    // Object arrays
    Enemy enemies[MAX_ENEMIES];              // 50 max
    int numEnemies;
    
    Collectible collectibles[...];           // Health + Ammo + Keycards
    int numCollectibles;
    
    Platform platforms[MAX_PLATFORMS];       // 30 max
    int numPlatforms;
    
    Crate crates[MAX_CRATES];               // 50 max
    int numCrates;
    
    Door doors[MAX_DOORS];                  // 10 max
    int numDoors;
    
    ParkourObstacle parkourObstacles[10];
    int numParkourObstacles;
    
    ExitDoor exitDoor;
    bool allEnemiesKilled;
    bool exitDoorJustActivated;  // For sound trigger
    
    // Boss fight (Level 2)
    bool bossPhaseStarted;
    bool regularEnemiesCleared;
    int bossEnemyIndex;
    bool bossKilledPortalReady;
    
    // Level properties
    Vector3 playerStart;
    Vector3 objective;
    float objectiveRadius;
    bool objectiveReached;
    
    // Environment
    float floorSize;
    float wallHeight;
    bool hasLava;
    float lavaHeight;
    float drawDistance;
    
    float levelTime;
    float maxTime;
};
```

#### Key Methods
```cpp
// Level loading
void reset()           // Clear all objects
void loadLevel1()      // Load Research Facility
void loadLevel2()      // Load Hell Arena

// Game loop
void update(float deltaTime, Vector3 playerPos)
// - Updates all enemies, collectibles, doors
// - Checks enemy count for exit door activation
// - Updates boss phase transitions

// Collision queries
bool checkPlatformCollision(Vector3 pos, float radius, float& groundY)
bool checkCrateCollision(Vector3 pos, float radius, Vector3& pushOut)
bool checkParkourCollision(Vector3 pos, float radius, 
                           int& obstacleIndex, float& obstHeight)
bool isInLava(Vector3 pos)

// Rendering
void draw(Vector3 cameraPos)
void drawEnvironment()   // Floor, walls, skybox
void drawLava()          // Animated lava surface
void drawSkybox()        // 6-sided skybox
```

### 9.4 Level 1 Layout (Research Facility)
```
Floor Size: 80x80 units
Wall Height: 10 units
Time Limit: 360 seconds (6 minutes)

QUADRANTS:
- SW: Security Checkpoint (player start at -32, -32)
- NW: Research Labs 
- SE: Containment Area
- NE: Reactor Core (objective at +25, +25)

ENEMIES: 12 total
- 8 Zombies (50 HP each)
- 4 Demons (80 HP each)

COLLECTIBLES:
- 5 Health Packs
- 6 Ammo Boxes  
- 2 Keycards (Blue=1, Red=2)

DOORS: 3 (2 locked, 1 unlocked)
- Research Lab: Blue keycard
- Reactor Core: Red keycard

PARKOUR OBSTACLES: 4 vaultable barriers

EXIT: Sliding door at (0, 0, -38)
```

### 9.5 Level 2 Layout (Hell Arena)
```
Floor Size: 80x80 units
Wall Height: 0 (outdoor)
Lava: Yes (at y=0)
Time Limit: 480 seconds (8 minutes)

STRUCTURE:
- Main Arena: 35x35 platform at y=1.5
- Corner Platforms: 4 elevated (y=3.0)
- Boss Platform: y=6.0 at (+25, +25)
- Connecting bridges and ramps

ENEMIES: 12+ total
- Regular enemies in arena
- BOSS at index specified (300 HP)

BOSS PHASES:
- Phase 1: >60% HP - Normal attacks
- Phase 2: 30-60% HP - Faster, meteor shower
- Phase 3: <30% HP - Enraged rapid fire

COLLECTIBLES:
- Health/Ammo on platforms
- Mystery boxes scattered
- Powerups: Speed, Damage, Invincibility, Shield

EXIT: Victory Portal at objective position
      (activated after boss killed)
```

---

## 10. Collision System (Collision.h)

### 10.1 Namespace and Primitives
```cpp
namespace Collision {
    // Axis-Aligned Bounding Box
    struct AABB {
        Vector3 min, max;
        
        static AABB fromCenterSize(Vector3 center, Vector3 halfSize);
        bool contains(Vector3 point);
        bool intersects(AABB other);
    };
    
    // Sphere collider
    struct Sphere {
        Vector3 center;
        float radius;
        
        bool contains(Vector3 point);
        bool intersects(Sphere other);
        bool intersects(AABB box);
    };
    
    // Ray for shooting
    struct Ray {
        Vector3 origin;
        Vector3 direction;  // Normalized
        
        bool intersects(AABB box, float& tHit);
        bool intersects(Sphere sphere, float& tHit);
    };
    
    // Platform (special AABB for ground collision)
    struct Platform {
        Vector3 position;   // Center
        Vector3 size;       // Full dimensions
        AABB bounds;
        bool isJumpPad;
        float jumpForce;
        
        Platform();
        Platform(Vector3 pos, Vector3 sz);
        bool isAbove(Vector3 point, float margin);
        float getTopY();
    };
    
    // Collision result
    struct CollisionResult {
        bool hit;
        Vector3 normal;
        float penetration;
        Vector3 point;
    };
}
```

### 10.2 Collision Functions
```cpp
// Sphere vs AABB resolution
CollisionResult resolveSphereAABB(Sphere sphere, AABB box);

// Point in box test
bool pointInAABB(Vector3 point, AABB box);

// Ray intersection tests
bool rayAABBIntersection(Ray ray, AABB box, float& tHit);
bool raySphereIntersection(Ray ray, Sphere sphere, float& tHit);
```

### 10.3 Player Collision Logic (in Game.h)
```cpp
void handlePlayerCollision() {
    Vector3 pos = player.position;
    float radius = 0.5f;
    
    // 1. Platform collision (ground check)
    float groundY = 0.0f;
    bool onPlatform = level.checkPlatformCollision(pos, radius, groundY);
    if (onPlatform && pos.y - PLAYER_HEIGHT <= groundY + 0.5f) {
        player.position.y = groundY + PLAYER_HEIGHT;
        player.velocity.y = 0;
        player.isOnGround = true;
    }
    
    // 2. Crate collision (push out)
    Vector3 pushOut;
    if (level.checkCrateCollision(pos, radius, pushOut)) {
        player.position = player.position + pushOut;
    }
    
    // 3. Parkour obstacle collision
    int obstIndex;
    float obstHeight;
    if (level.checkParkourCollision(pos, radius, obstIndex, obstHeight)) {
        // Can vault if pressing E and height is vaultable
        if (wantInteract && obstHeight <= PARKOUR_MAX_HEIGHT) {
            startParkour(obstIndex);
        } else {
            // Push player out
        }
    }
    
    // 4. Wall/boundary collision
    float boundary = level.floorSize * 0.5f - 1.0f;
    if (pos.x < -boundary) pos.x = -boundary;
    if (pos.x > boundary) pos.x = boundary;
    if (pos.z < -boundary) pos.z = -boundary;
    if (pos.z > boundary) pos.z = boundary;
    
    // 5. Lava check
    if (level.hasLava && level.isInLava(pos)) {
        player.takeLavaDamage(LAVA_DAMAGE);
    }
}
```

### 10.4 Enemy Collision Logic
```cpp
// Enemy vs player (for attacks)
float distToPlayer = enemy.position.distanceTo(player.position);
if (distToPlayer < ENEMY_ATTACK_RANGE && enemy.canAttack()) {
    player.takeDamage(enemy.damage);
    enemy.performAttack();
}

// Boss projectile vs player
if (enemy.type == ENEMY_BOSS) {
    int damage = enemy.checkProjectileHitDamage(player.position, 1.0f);
    if (damage > 0) {
        player.takeDamage(damage);
    }
}
```

### 10.5 Shooting Ray Collision
```cpp
void handleShooting() {
    Ray shootRay;
    shootRay.origin = player.getShootOrigin();
    shootRay.direction = player.getShootDirection();
    
    float closestHit = 999999.0f;
    int hitEnemy = -1;
    
    // Check all enemies
    for (int i = 0; i < level.numEnemies; i++) {
        if (!level.enemies[i].active) continue;
        
        Sphere enemyBounds;
        enemyBounds.center = level.enemies[i].position + Vector3(0, 1, 0);
        enemyBounds.radius = 1.0f;  // Hit sphere size
        
        float tHit;
        if (shootRay.intersects(enemyBounds, tHit)) {
            if (tHit < closestHit) {
                closestHit = tHit;
                hitEnemy = i;
            }
        }
    }
    
    if (hitEnemy >= 0) {
        int damage = PLAYER_DAMAGE * player.getDamageMultiplier();
        level.enemies[hitEnemy].takeDamage(damage);
        player.addScore(10);  // Hit bonus
    }
}
```

---

## 11. Audio System (Sound.h)

### 11.1 Sound Class
```cpp
class Sound {
    char backgroundMusic[256];  // Current BGM path
    bool musicPlaying;
    
    void playSound(const char* filename, bool loop = false);
    void stopSound(const char* alias);
    void playBackgroundMusic(const char* filename);
    void stopBackgroundMusic();
    void setMusicVolume(int volume);  // 0-1000
};
```

### 11.2 Windows MCI Implementation
```cpp
void playSound(const char* filename, bool loop) {
    char command[512];
    char alias[64];
    
    // Generate unique alias
    sprintf(alias, "sound_%d", aliasCounter++);
    
    // Open the audio file
    sprintf(command, "open \"%s\" type mpegvideo alias %s", filename, alias);
    mciSendString(command, NULL, 0, NULL);
    
    // Play (with optional loop)
    if (loop) {
        sprintf(command, "play %s repeat", alias);
    } else {
        sprintf(command, "play %s", alias);
    }
    mciSendString(command, NULL, 0, NULL);
}

void stopBackgroundMusic() {
    mciSendString("stop bgm", NULL, 0, NULL);
    mciSendString("close bgm", NULL, 0, NULL);
    musicPlaying = false;
}
```

### 11.3 Sound Usage in Game
```cpp
// Background music per level
if (level.levelID == LEVEL_1_FACILITY) {
    sound.playBackgroundMusic("res/Audio/level1_bgm.mp3");
} else if (level.levelID == LEVEL_2_HELL_ARENA) {
    sound.playBackgroundMusic("res/Audio/level2_bgm.mp3");
}

// Sound effects (one-shot)
sound.playSound("res/Audio/gunshot.wav");
sound.playSound("res/Audio/enemy_hurt.wav");
sound.playSound("res/Audio/pickup.wav");
sound.playSound("res/Audio/door_open.wav");
sound.playSound("res/Audio/level_complete.wav");
```

---

## 12. Lighting System (Lighting.h)

### 12.1 Light Class
```cpp
class Light {
    GLenum lightID;          // GL_LIGHT0, GL_LIGHT1, etc.
    
    float ambient[4];        // Ambient color RGBA
    float diffuse[4];        // Diffuse color RGBA
    float specular[4];       // Specular color RGBA
    float position[4];       // Position (w=0 for directional)
    
    bool enabled;
    
    // Methods
    void setAmbient(float r, float g, float b, float a = 1.0f);
    void setDiffuse(float r, float g, float b, float a = 1.0f);
    void setSpecular(float r, float g, float b, float a = 1.0f);
    void setPosition(float x, float y, float z, float w = 1.0f);
    void enable();
    void disable();
    void apply();
};
```

### 12.2 SceneLighting Class
```cpp
class SceneLighting {
    Light sunLight;           // Main directional light
    Light fillLight;          // Secondary fill light
    Light playerLight;        // Follows player
    
    float globalAmbient[4];   // Scene ambient
    
    // Methods
    void init();
    void setupForLevel1();    // Facility lighting (cool blue)
    void setupForLevel2();    // Hell lighting (warm red/orange)
    void updatePlayerLight(Vector3 playerPos);
    void apply();
};
```

### 12.3 Level 1 Lighting Setup
```cpp
void setupForLevel1() {
    // Cool, artificial indoor lighting
    globalAmbient[0] = 0.15f;
    globalAmbient[1] = 0.18f;
    globalAmbient[2] = 0.25f;
    globalAmbient[3] = 1.0f;
    
    // Main overhead light (directional)
    sunLight.setPosition(0.3f, 1.0f, 0.2f, 0.0f);
    sunLight.setDiffuse(0.7f, 0.75f, 0.85f);
    sunLight.setAmbient(0.1f, 0.12f, 0.15f);
    
    // Fill light from below
    fillLight.setPosition(-0.2f, -0.5f, 0.3f, 0.0f);
    fillLight.setDiffuse(0.15f, 0.2f, 0.25f);
}
```

### 12.4 Level 2 Lighting Setup
```cpp
void setupForLevel2() {
    // Warm, hellish outdoor lighting
    globalAmbient[0] = 0.25f;
    globalAmbient[1] = 0.12f;
    globalAmbient[2] = 0.08f;
    globalAmbient[3] = 1.0f;
    
    // Main light (fiery sun)
    sunLight.setPosition(0.5f, 0.8f, 0.3f, 0.0f);
    sunLight.setDiffuse(1.0f, 0.6f, 0.3f);
    sunLight.setAmbient(0.3f, 0.15f, 0.1f);
    
    // Lava glow from below
    fillLight.setPosition(0.0f, -1.0f, 0.0f, 0.0f);
    fillLight.setDiffuse(0.8f, 0.3f, 0.1f);
}
```

---

## 13. HUD System (HUD.h)

### 13.1 StyledText Namespace
```cpp
namespace StyledText {
    // Draw text with drop shadow
    void drawTextWithShadow(float x, float y, const char* text, void* font,
                            float r, float g, float b, float shadowOffset = 2.0f);
    
    // Draw text with 8-direction outline
    void drawTextWithOutline(float x, float y, const char* text, void* font,
                             float r, float g, float b,
                             float outlineR, float outlineG, float outlineB);
    
    // Draw text with glow effect (additive blending)
    void drawTextWithGlow(float x, float y, const char* text, void* font,
                          float r, float g, float b, float glowIntensity = 0.5f);
    
    // Get pixel width of text string
    int getTextWidth(const char* text, void* font);
}
```

### 13.2 HUD Class Properties
```cpp
class HUD {
    int screenWidth;
    int screenHeight;
    float damageFlash;     // Red flash intensity
    float lowHealthPulse;  // Pulse animation
    float animTime;        // Animation timer
    
    // Fonts (GLUT bitmap fonts)
    void* fontLarge;   // GLUT_BITMAP_TIMES_ROMAN_24
    void* fontMedium;  // GLUT_BITMAP_HELVETICA_18
    void* fontSmall;   // GLUT_BITMAP_HELVETICA_12
};
```

### 13.3 HUD Components

#### Crosshair
```cpp
void drawCrosshair(float spread = 0.0f, bool enemyInSight = false);
// - Dynamic size based on spread
// - Red tint when enemy in crosshair
// - 4 lines + center dot
```

#### Health Bar
```cpp
void drawHealthBar(int health, int maxHealth);
// - Position: Top-left (25, screenHeight - 55)
// - Size: 240x24 pixels
// - Colors:
//   - Green: >60% HP
//   - Yellow: 30-60% HP
//   - Red (pulsing): <30% HP
// - Gradient fill + highlight stripe
// - Corner accents (cyan)
```

#### Shield Bar
```cpp
void drawShieldBar(float shieldHealth, float maxShieldHealth);
// - Only shown when shield > 0
// - Position: Below health bar
// - Cyan gradient with glow effect
```

#### Ammo Counter
```cpp
void drawAmmoCounter(int ammo, int maxAmmo);
// - Position: Top-right (screenWidth - 260)
// - Bullet icon
// - Current/Max display
// - Visual bar
// - Glow effect when low ammo
```

#### Score Display
```cpp
void drawScore(int score);
// - Position: Top-center
// - Format: "SCORE: 00000000"
// - Cyan glow effect
```

#### Timer
```cpp
void drawTimer(int seconds);
// - Position: Below score
// - Format: "MM:SS"
// - Red flash when <30 seconds
// - Yellow when <60 seconds
```

#### Level Indicator
```cpp
void drawLevelIndicator(int level, const char* levelName = NULL);
// - Position: Bottom-left (25, 40)
// - Shows "LEVEL X"
// - Shows level name below
```

#### Powerup Indicators
```cpp
void drawPowerupIndicators(float speedBoostTime, float damageBoostTime, 
                           float invincibilityTime);
// - Position: Right side (screenWidth - 200)
// - Progress bars showing remaining time
// - Icons for each powerup type
// - Pulsing colors
```

### 13.4 Screen Effects

#### Damage Overlay
```cpp
void drawDamageOverlay(float intensity);
// - Red screen tint
// - Vignette at edges (darker borders)
// - Decays over time
```

#### Near-Death Overlay
```cpp
void drawNearDeathOverlay(float healthPercent);
// Thresholds from GameConfig.h:
// - NEARDEATH_THRESHOLD (0.30f): Start grey overlay
// - NEARDEATH_HEARTBEAT_THRESHOLD (0.15f): Heartbeat flash
// - NEARDEATH_CRITICAL_THRESHOLD (0.10f): Red pulsing border

// Effects:
// - Grey/red desaturation
// - Heavy vignette (darker edges)
// - Heartbeat red flash at critical
// - Pulsing red border when dying
```

#### Invincibility Overlay
```cpp
// Golden screen glow when invincibility active
if (invincibilityTime > 0) {
    float pulse = sin(animTime * 10.0f) * 0.1f + 0.1f;
    glColor4f(1.0f, 0.85f, 0.2f, pulse);
    // Full screen quad
}
```

### 13.5 UI Dialogs

#### Message Box
```cpp
void drawMessageBox(const char* title, const char* message, 
                    bool showPressKey = true);
// - Centered 500x220 box
// - Gradient background
// - Glowing border
// - Title with glow
// - Message with shadow
// - "Press SPACE to continue" prompt
```

#### Interaction Prompt
```cpp
void drawInteractionPrompt(const char* action);
// - Position: Below crosshair
// - Shows "Press E to [action]"
// - Dark box with yellow border
```

### 13.6 Main Draw Functions
```cpp
// Basic draw
void draw(int health, int maxHealth, int ammo, int maxAmmo, 
          int score, int timeSeconds, int level,
          float speedBoostTime, float damageBoostTime, float invincibilityTime,
          float shieldHealth, float maxShieldHealth);

// With interaction prompt
void drawWithPrompt(..., const char* interactionPrompt, ...);

// Full draw with objective distance
void drawFull(..., float objectiveDist, float spread, bool enemyInSight, ...);
```

---

*Continue to Part 4 for Models, Textures, Animation, and the Game Loop...*
