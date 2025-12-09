# DOOMERS - Technical Documentation Part 5
## Implementation Guide: Adding Features & Modifying Systems

---

## 20. Adding New Enemies

### 20.1 Step 1: Add Enemy Type
```cpp
// In GameConfig.h
enum EnemyType {
    ENEMY_ZOMBIE,
    ENEMY_DEMON,
    ENEMY_BOSS,
    ENEMY_SPIDER,  // NEW: Add your enemy type here
    ENEMY_COUNT
};
```

### 20.2 Step 2: Configure Enemy Stats
```cpp
// In Enemy.h - init() method
void init(EnemyType type, Vector3 pos, Vector3 patrolA, Vector3 patrolB) {
    position = pos;
    patrolStart = patrolA;
    patrolEnd = patrolB;
    
    switch (type) {
        case ENEMY_ZOMBIE:
            health = 50; maxHealth = 50;
            damage = 10; speed = ENEMY_SPEED;
            scoreValue = 100;
            break;
        case ENEMY_DEMON:
            health = 80; maxHealth = 80;
            damage = 15; speed = ENEMY_SPEED * 1.3f;
            scoreValue = 200;
            break;
        case ENEMY_SPIDER:  // NEW
            health = 30; maxHealth = 30;
            damage = 5; speed = ENEMY_SPEED * 1.8f;  // Fast but weak
            scoreValue = 150;
            break;
        // ...
    }
}
```

### 20.3 Step 3: Create Visual Model
```cpp
// In models/EnemyModels.h
namespace EnemyModels {
    void drawSpider(float rotY, float animPhase, float health, float maxHealth) {
        glPushMatrix();
        glRotatef(rotY, 0, 1, 0);
        
        // Body
        ModelUtils::setColor(0.3f, 0.2f, 0.1f);
        glTranslatef(0, 0.3f, 0);
        ModelUtils::drawSphere(0.4f, 8, 8);
        
        // Legs (8 legs with animation)
        for (int i = 0; i < 8; i++) {
            float angle = i * 45.0f;
            float legPhase = animPhase + i * 0.5f;
            float legBend = sin(legPhase) * 20.0f;
            
            glPushMatrix();
            glRotatef(angle, 0, 1, 0);
            glTranslatef(0.3f, 0, 0);
            glRotatef(45 + legBend, 0, 0, 1);
            ModelUtils::drawCylinder(0.05f, 0.5f, 6);
            glPopMatrix();
        }
        
        glPopMatrix();
    }
}
```

### 20.4 Step 4: Add to LowPolyModels Interface
```cpp
// In LowPolyModels.h
inline void drawSpider(float rotY, float animPhase, float health, float maxHealth) {
    EnemyModels::drawSpider(rotY, animPhase, health, maxHealth);
}
```

### 20.5 Step 5: Update Enemy::draw()
```cpp
// In Enemy.h - draw() method
void draw() {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    
    switch (type) {
        case ENEMY_ZOMBIE:
            LowPolyModels::drawZombie(rotationY, animPhase, health, maxHealth);
            break;
        case ENEMY_DEMON:
            LowPolyModels::drawDemon(rotationY, animPhase, attackPhase);
            break;
        case ENEMY_SPIDER:  // NEW
            LowPolyModels::drawSpider(rotationY, animPhase, health, maxHealth);
            break;
        // ...
    }
    
    glPopMatrix();
}
```

### 20.6 Step 6: Add to Level
```cpp
// In Level.h - loadLevel1() or loadLevel2()
enemies[numEnemies].init(ENEMY_SPIDER, 
    Vector3(10, 0, 10),      // Position
    Vector3(5, 0, 10),       // Patrol point A
    Vector3(15, 0, 10));     // Patrol point B
numEnemies++;
```

---

## 21. Adding New Collectibles

### 21.1 Step 1: Add Collectible Type
```cpp
// In Collectible.h
enum CollectibleType {
    COLLECT_HEALTH,
    COLLECT_AMMO,
    COLLECT_KEYCARD,
    COLLECT_SPEED_BOOST,
    COLLECT_DAMAGE_BOOST,
    COLLECT_INVINCIBILITY,
    COLLECT_MAX_AMMO,
    COLLECT_SHIELD,
    COLLECT_MEGA_HEALTH,  // NEW: Add here
    COLLECTIBLE_TYPE_COUNT
};
```

### 21.2 Step 2: Create Visual
```cpp
// In Collectible.h - draw() method, add case:
case COLLECT_MEGA_HEALTH:
    // Golden health cross
    glPushMatrix();
    glRotatef(rotation, 0, 1, 0);
    glTranslatef(0, bobOffset, 0);
    
    // Glow effect
    EffectsModels::drawPickupGlow(1.0f, 0.8f, 0.0f, 1.5f);
    
    // Golden cross
    ModelUtils::setColor(1.0f, 0.85f, 0.0f);
    ModelUtils::setEmissive(0.3f, 0.25f, 0.0f);
    ModelUtils::drawBox(0.4f, 0.15f, 0.15f);  // Horizontal
    ModelUtils::drawBox(0.15f, 0.4f, 0.15f);  // Vertical
    ModelUtils::clearEmissive();
    
    glPopMatrix();
    break;
```

### 21.3 Step 3: Handle Collection in Game
```cpp
// In Game.h - collectItem() method
case COLLECT_MEGA_HEALTH:
    player.health = player.maxHealth * 2;  // Double max!
    if (player.health > 200) player.health = 200;
    player.addScore(100);
    break;
```

### 21.4 Step 4: Add to Level
```cpp
// In Level.h
collectibles[numCollectibles].init(COLLECT_MEGA_HEALTH, 
    Vector3(0, 0.5f, 0),  // Position (raised slightly)
    0);                   // Value (not used for mega health)
numCollectibles++;
```

---

## 22. Adding New Weapons

### 22.1 Step 1: Add Weapon Type
```cpp
// In GameConfig.h (create if not exists)
enum WeaponType {
    WEAPON_ASSAULT_RIFLE = 0,
    WEAPON_SHOTGUN,
    WEAPON_PLASMA,
    WEAPON_COUNT
};

// Weapon stats
const int WEAPON_DAMAGE[WEAPON_COUNT] = {20, 60, 30};
const float WEAPON_FIRE_RATE[WEAPON_COUNT] = {0.12f, 0.8f, 0.3f};
const int WEAPON_AMMO_COST[WEAPON_COUNT] = {1, 3, 2};
```

### 22.2 Step 2: Add to Player
```cpp
// In Player.h
class Player {
    WeaponType currentWeapon;
    int weaponAmmo[WEAPON_COUNT];
    
    void switchWeapon(WeaponType weapon);
    int getCurrentDamage();
    float getCurrentFireRate();
};
```

### 22.3 Step 3: Create Weapon Model
```cpp
// In models/WeaponModel.h
namespace WeaponModel {
    void drawShotgun(float recoil, bool firing, float lightIntensity) {
        glPushMatrix();
        
        // Apply recoil (stronger for shotgun)
        glTranslatef(0, 0, -recoil * 0.3f);
        glRotatef(recoil * 2.0f, 1, 0, 0);
        
        // Stock
        ModelUtils::setColorMetallic(0.3f, 0.2f, 0.15f);  // Wood color
        glPushMatrix();
        glTranslatef(0, -0.05f, 0.2f);
        ModelUtils::drawBox(0.08f, 0.08f, 0.3f);
        glPopMatrix();
        
        // Barrel (double barrel)
        ModelUtils::setColorMetallic(0.2f, 0.2f, 0.22f);
        glPushMatrix();
        glTranslatef(0.03f, 0, -0.3f);
        ModelUtils::drawCylinder(0.025f, 0.6f, 8);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-0.03f, 0, -0.3f);
        ModelUtils::drawCylinder(0.025f, 0.6f, 8);
        glPopMatrix();
        
        glPopMatrix();
    }
}
```

### 22.4 Step 4: Update Weapon Drawing
```cpp
// In LowPolyModels.h
inline void drawCurrentWeapon(WeaponType weapon, float recoil, bool firing, 
                               bool lightOn, float intensity) {
    switch (weapon) {
        case WEAPON_ASSAULT_RIFLE:
            WeaponModel::drawAssaultRifleDetailed(recoil, firing, intensity);
            break;
        case WEAPON_SHOTGUN:
            WeaponModel::drawShotgun(recoil, firing, intensity);
            break;
        // ...
    }
}
```

### 22.5 Step 5: Update Shooting Logic
```cpp
// In Game.h - shoot()
void shoot() {
    WeaponType weapon = player.currentWeapon;
    
    if (!player.canFire(currentTime)) return;
    if (player.ammo < WEAPON_AMMO_COST[weapon]) return;
    
    player.fire(currentTime);
    player.ammo -= WEAPON_AMMO_COST[weapon];
    
    // Different behavior per weapon
    switch (weapon) {
        case WEAPON_ASSAULT_RIFLE:
            // Single ray
            shootRay(player.getShootOrigin(), player.getShootDirection(),
                     WEAPON_DAMAGE[weapon]);
            sound.playSound(Sounds::SFX_AR_FIRE);
            break;
            
        case WEAPON_SHOTGUN:
            // Multiple pellets with spread
            for (int i = 0; i < 8; i++) {
                Vector3 spread = player.getShootDirection();
                spread.x += (rand() % 100 - 50) / 500.0f;
                spread.y += (rand() % 100 - 50) / 500.0f;
                spread.z += (rand() % 100 - 50) / 500.0f;
                spread = spread.normalize();
                
                shootRay(player.getShootOrigin(), spread, 
                         WEAPON_DAMAGE[weapon] / 8);
            }
            sound.playSound(Sounds::SFX_SHOTGUN_FIRE);
            break;
    }
}
```

---

## 23. Adding New Levels

### 23.1 Step 1: Add Level ID
```cpp
// In GameConfig.h
enum LevelID {
    LEVEL_1_FACILITY,
    LEVEL_2_HELL_ARENA,
    LEVEL_3_SPACE_STATION,  // NEW
    LEVEL_COUNT
};
```

### 23.2 Step 2: Create Load Function
```cpp
// In Level.h
void loadLevel3() {
    reset();
    levelID = LEVEL_3_SPACE_STATION;
    
    // Environment settings
    floorSize = 100.0f;
    wallHeight = 15.0f;
    hasLava = false;
    maxTime = 420.0f;  // 7 minutes
    
    // Player start
    playerStart = Vector3(0, PLAYER_HEIGHT, -40);
    objective = Vector3(0, 0, 40);
    
    // ===== ADD ENEMIES =====
    numEnemies = 0;
    enemies[numEnemies].init(ENEMY_DEMON, 
        Vector3(10, 0, 0), Vector3(5, 0, 0), Vector3(15, 0, 0));
    numEnemies++;
    // ... add more enemies
    
    // ===== ADD PLATFORMS =====
    numPlatforms = 0;
    platforms[numPlatforms] = Platform(Vector3(0, 2, 0), Vector3(10, 0.5f, 10));
    numPlatforms++;
    // ... add more platforms
    
    // ===== ADD COLLECTIBLES =====
    numCollectibles = 0;
    collectibles[numCollectibles].init(COLLECT_HEALTH, Vector3(5, 0.5f, 5), 25);
    numCollectibles++;
    // ... add more collectibles
    
    // ===== ADD CRATES =====
    numCrates = 0;
    crates[numCrates].position = Vector3(-10, 0, 10);
    crates[numCrates].size = 1.2f;
    crates[numCrates].isSciFi = true;
    crates[numCrates].updateBounds();
    numCrates++;
    // ... add more crates
    
    // ===== EXIT DOOR =====
    exitDoor.position = Vector3(0, 0, 45);
    exitDoor.rotation = 0.0f;
    exitDoor.isActive = false;
    exitDoor.updateBounds();
}
```

### 23.3 Step 3: Update Game Loader
```cpp
// In Game.h - loadLevel()
void loadLevel(int levelNum) {
    player.reset();
    numKeycards = 0;
    
    switch (levelNum) {
        case 1:
            currentLevel.loadLevel1();
            lighting.setupForLevel(LEVEL_1_FACILITY);
            break;
        case 2:
            currentLevel.loadLevel2();
            lighting.setupForLevel(LEVEL_2_HELL_ARENA);
            break;
        case 3:  // NEW
            currentLevel.loadLevel3();
            lighting.setupForLevel(LEVEL_3_SPACE_STATION);
            break;
    }
    
    player.setPosition(currentLevel.playerStart);
}
```

### 23.4 Step 4: Set Up Lighting
```cpp
// In Lighting.h
void setupForLevel(LevelID level) {
    switch (level) {
        case LEVEL_1_FACILITY:
            // Cool blue indoor lighting
            setGlobalAmbient(0.15f, 0.18f, 0.25f);
            sunLight.setDiffuse(0.7f, 0.75f, 0.85f);
            break;
            
        case LEVEL_2_HELL_ARENA:
            // Warm red/orange hell lighting
            setGlobalAmbient(0.25f, 0.12f, 0.08f);
            sunLight.setDiffuse(1.0f, 0.6f, 0.3f);
            break;
            
        case LEVEL_3_SPACE_STATION:  // NEW
            // Cold, stark white with purple accent
            setGlobalAmbient(0.2f, 0.2f, 0.25f);
            sunLight.setDiffuse(0.9f, 0.9f, 1.0f);
            break;
    }
}
```

### 23.5 Step 5: Add New Skybox (Optional)
```cpp
// In TextureManager.h - Add texture IDs
TEX_SKYBOX3_FRONT,
TEX_SKYBOX3_BACK,
TEX_SKYBOX3_LEFT,
TEX_SKYBOX3_RIGHT,
TEX_SKYBOX3_TOP,
TEX_SKYBOX3_BOTTOM,

// In init() - Load textures
loadTexture(TEX_SKYBOX3_FRONT, "res/Textures/Skyboxes/Space/front.png");
// ... load other faces

// In Level::drawSkybox() - Use based on level
if (levelID == LEVEL_3_SPACE_STATION) {
    drawSkyboxFace(TEX_SKYBOX3_FRONT, ...);
    // ...
}
```

---

## 24. Adding Powerups

### 24.1 Quick Powerup Template
```cpp
// 1. Add to CollectibleType enum (Collectible.h)
COLLECT_DOUBLE_JUMP,

// 2. Add visual in Collectible::draw()
case COLLECT_DOUBLE_JUMP:
    // Purple boot icon
    ModelUtils::setColor(0.6f, 0.2f, 0.8f);
    ModelUtils::setEmissive(0.2f, 0.1f, 0.3f);
    // Draw boot shape...
    ModelUtils::clearEmissive();
    break;

// 3. Add to Player class
bool hasDoubleJump;
float doubleJumpTime;
bool usedDoubleJump;

void activateDoubleJump(float duration) {
    hasDoubleJump = true;
    doubleJumpTime = duration;
    usedDoubleJump = false;
}

// 4. Handle in Player::update()
if (hasDoubleJump) {
    doubleJumpTime -= deltaTime;
    if (doubleJumpTime <= 0) hasDoubleJump = false;
}

// In jump logic:
if (wantJump) {
    if (isOnGround) {
        velocity.y = JUMP_FORCE;
        isOnGround = false;
        usedDoubleJump = false;
    } else if (hasDoubleJump && !usedDoubleJump) {
        velocity.y = JUMP_FORCE * 0.8f;  // Slightly weaker
        usedDoubleJump = true;
    }
}

// 5. Add collection handling in Game::collectItem()
case COLLECT_DOUBLE_JUMP:
    player.activateDoubleJump(15.0f);  // 15 seconds
    player.addScore(40);
    break;

// 6. Add HUD indicator in HUD::drawPowerupIndicators()
if (doubleJumpTime > 0) {
    // Draw progress bar and icon
}
```

---

## 25. Modifying Existing Systems

### 25.1 Changing Player Speed
```cpp
// In GameConfig.h
const float PLAYER_SPEED = 8.0f;           // Was 6.0f
const float PLAYER_SPRINT_MULTIPLIER = 1.8f; // Was 1.5f

// Affects:
// - Player::update() - base movement speed
// - Sprint multiplier applied on top
```

### 25.2 Changing Weapon Damage
```cpp
// In GameConfig.h
const int PLAYER_DAMAGE = 25;  // Was 20

// Affects:
// - Game::shoot() - enemy.takeDamage(PLAYER_DAMAGE * multiplier)
```

### 25.3 Changing Enemy AI Behavior
```cpp
// In Enemy.h - update()

// Change detection range
const float DETECT_RANGE = 25.0f;  // Was ENEMY_DETECT_RANGE (15.0f)

// Change attack frequency
attackCooldown = 1.0f;  // Was 2.0f - attacks twice as fast

// Change chase speed
float chaseSpeed = speed * 1.5f;  // Was speed * 1.0f
```

### 25.4 Changing Jump Height
```cpp
// In GameConfig.h
const float JUMP_FORCE = 0.3f;  // Was 0.25f - higher jumps

// Or in Player.h for finer control:
if (wantJump && isOnGround) {
    velocity.y = 0.35f;  // Direct value
    isOnGround = false;
}
```

### 25.5 Changing Camera Sensitivity
```cpp
// In Camera.h
float sensitivity = 0.15f;  // Was 0.12f - faster camera movement

// Or make it adjustable:
void setSensitivity(float newSens) {
    sensitivity = newSens;
    if (sensitivity < 0.05f) sensitivity = 0.05f;
    if (sensitivity > 0.5f) sensitivity = 0.5f;
}
```

---

## 26. Adding Sound Effects

### 26.1 Step 1: Add Sound Enum
```cpp
// In Sound.h
namespace Sounds {
    enum SoundID {
        SFX_AR_FIRE,
        SFX_ENEMY_HURT,
        SFX_PLAYER_HURT,
        SFX_BUTTON_CLICK,
        SFX_FOOTSTEPS_WALK,
        SFX_FOOTSTEPS_RUN,
        SFX_EXPLOSION,      // NEW
        SFX_LASER_CHARGE,   // NEW
        MUSIC_MENU,
        MUSIC_INGAME,
        SOUND_COUNT
    };
}
```

### 26.2 Step 2: Add Sound Files
Place audio files in `res/Audio/`:
- `explosion.wav`
- `laser_charge.wav`

### 26.3 Step 3: Load and Play
```cpp
// In Sound.h
void playSound(SoundID id) {
    const char* paths[] = {
        "res/Audio/ar_fire.wav",
        "res/Audio/enemy_hurt.wav",
        // ...
        "res/Audio/explosion.wav",      // NEW
        "res/Audio/laser_charge.wav",   // NEW
    };
    
    if (id < SOUND_COUNT) {
        playSoundFile(paths[id], false);
    }
}

// Usage:
sound.playSound(Sounds::SFX_EXPLOSION);
```

---

## 27. Performance Tips

### 27.1 Reducing Draw Calls
```cpp
// Bad: Many small draw calls
for (int i = 0; i < 1000; i++) {
    glPushMatrix();
    glTranslatef(x[i], y[i], z[i]);
    drawSmallObject();
    glPopMatrix();
}

// Better: Batch similar objects
glBegin(GL_TRIANGLES);
for (int i = 0; i < 1000; i++) {
    // Add all vertices at once
    glVertex3f(x[i], y[i], z[i]);
    glVertex3f(x[i]+1, y[i], z[i]);
    glVertex3f(x[i], y[i]+1, z[i]);
}
glEnd();
```

### 27.2 Distance Culling
```cpp
// In Level::draw()
float dist = (object.position - cameraPos).length();
if (dist > drawDistance) continue;  // Skip far objects
```

### 27.3 Reduce Polygon Count
```cpp
// For spheres far away, use fewer segments
float dist = (pos - cameraPos).length();
int segments = (dist < 20) ? 16 : 8;
glutSolidSphere(radius, segments, segments);
```

---

## 28. Debugging Tips

### 28.1 Debug Logging
```cpp
// Use DEBUG_LOG macro
DEBUG_LOG("Player position: %.2f, %.2f, %.2f\n", 
          player.position.x, player.position.y, player.position.z);
```

### 28.2 Visual Debug
```cpp
// Draw collision bounds
void drawAABBDebug(AABB box) {
    glDisable(GL_LIGHTING);
    glColor3f(1, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(box.min.x, box.min.y, box.min.z);
    glVertex3f(box.max.x, box.min.y, box.min.z);
    glVertex3f(box.max.x, box.max.y, box.min.z);
    glVertex3f(box.min.x, box.max.y, box.min.z);
    glEnd();
    // ... draw all faces
    glEnable(GL_LIGHTING);
}
```

### 28.3 Common Issues

| Issue | Likely Cause | Solution |
|-------|-------------|----------|
| Objects not visible | Wrong position or scale | Check coordinates, add debug draw |
| Collision not working | AABB not updated | Call `updateBounds()` after position change |
| Texture missing | Wrong path or not loaded | Check console for SOIL errors |
| Model upside down | Coordinate system mismatch | Add `glRotatef(180, 0, 1, 0)` |
| Performance drop | Too many draw calls | Batch rendering, add culling |

---

## 29. File Reference Quick Guide

| File | Purpose | Key Classes/Namespaces |
|------|---------|----------------------|
| `GameConfig.h` | All constants and enums | - |
| `Vector3.h` | 3D math | `Vector3` |
| `Camera.h` | View control | `Camera` |
| `Player.h` | Player character | `Player` |
| `Enemy.h` | Enemy AI | `Enemy`, `BossProjectile` |
| `Collectible.h` | Pickups | `Collectible` |
| `Collision.h` | Physics | `Collision::AABB`, `Sphere`, `Ray` |
| `Level.h` | Level data | `Level`, `Door`, `Crate`, `ParkourObstacle` |
| `Game.h` | Main loop | `Game` |
| `HUD.h` | UI rendering | `HUD`, `StyledText` |
| `Menu.h` | Menu screens | `Menu` |
| `Sound.h` | Audio | `Sound`, `Sounds::` |
| `Lighting.h` | Lights | `Light`, `SceneLighting` |
| `TextureManager.h` | Textures | `TextureManager` |
| `ModelLoader.h` | 3D models | `ModelLoader`, `AnimationLoader` |
| `LowPolyModels.h` | Model wrapper | `LowPolyModels` |
| `models/*.h` | Model details | Various namespaces |

---

## 30. Summary

This documentation covers the complete DOOMERS codebase:

1. **Project Overview** - Architecture, file structure, build system
2. **Core Systems** - GameConfig, Vector3, collision math
3. **Game Objects** - Player, Enemy, Camera, Collectibles
4. **Level System** - Doors, crates, parkour, platforms
5. **Collision** - AABB, sphere, ray intersection
6. **Audio** - Windows MCI sound system
7. **Lighting** - OpenGL lighting setup per level
8. **HUD** - Health bars, ammo, effects, overlays
9. **Models** - Procedural and 3D model loading
10. **Textures** - SOIL texture management
11. **Animation** - Frame-based FBX animations
12. **Game Loop** - State machine, input, rendering
13. **Implementation Guide** - How to add/modify features

For any additions:
1. Add enums/constants to `GameConfig.h`
2. Implement logic in appropriate system file
3. Add visuals in model files
4. Connect in `Game.h` for gameplay
5. Update HUD if needed
6. Test thoroughly!

---

*End of DOOMERS Technical Documentation*
*Total: 5 Parts*
