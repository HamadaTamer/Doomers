# DOOMERS - Complete Technical Documentation

## Table of Contents
1. [Project Overview](#1-project-overview)
2. [Architecture & Module Integration](#2-architecture--module-integration)
3. [Core Systems](#3-core-systems)
4. [Rendering & Models](#4-rendering--models)
5. [Game Objects](#5-game-objects)
6. [Level System](#6-level-system)
7. [Camera System](#7-camera-system)
8. [Animation System](#8-animation-system)
9. [Collision System](#9-collision-system)
10. [Audio System](#10-audio-system)
11. [UI/HUD System](#11-uihud-system)
12. [Implementation Guide](#12-implementation-guide)

---

## 1. Project Overview

### 1.1 What is DOOMERS?
DOOMERS is a fast-paced sci-fi shooter combining First-Person (FPS) and Third-Person (TPS) perspectives, built with OpenGL and GLUT for the GUC Computer Graphics Project 2025.

### 1.2 Technology Stack
| Component | Technology |
|-----------|------------|
| Graphics API | OpenGL 1.x (Fixed Function Pipeline) |
| Windowing | GLUT (freeglut) |
| 3D Models | Assimp Library |
| Textures | SOIL Library |
| Audio | Windows MCI / PlaySound API |
| Platform | Windows (Win32) |

### 1.3 Project Structure
```
Doomers-V2/
├── template/
│   ├── Doomers.cpp           # Main entry point
│   ├── glut.h                # GLUT header
│   ├── OpenGL3DTemplate.sln  # Visual Studio solution
│   ├── Debug/                # Build output
│   ├── Dependencies/
│   │   ├── assimp/           # 3D model loading library
│   │   └── soil/             # Texture loading library
│   ├── res/
│   │   ├── Audio/            # Sound files (.wav, .mp3)
│   │   ├── Models3D/         # 3D model files (.obj, .fbx)
│   │   └── Textures/         # Image textures (.png, .jpg)
│   └── src/
│       ├── GameConfig.h      # All game constants/settings
│       ├── Vector3.h         # 3D vector math
│       ├── Camera.h          # Camera system (FPS/TPS)
│       ├── Player.h          # Player controller
│       ├── Enemy.h           # Enemy AI and boss
│       ├── Level.h           # Level data and rendering
│       ├── Collectible.h     # Pickups and powerups
│       ├── Collision.h       # Collision detection
│       ├── Lighting.h        # Lighting system
│       ├── HUD.h             # UI rendering
│       ├── Menu.h            # Menu system
│       ├── Sound.h           # Audio system
│       ├── TextureManager.h  # Texture loading/management
│       ├── ModelLoader.h     # 3D model loading
│       ├── LowPolyModels.h   # Procedural model wrapper
│       └── models/
│           ├── ModelUtils.h       # Rendering utilities
│           ├── PlayerModel.h      # Player model
│           ├── WeaponModel.h      # Weapon models
│           ├── EnemyModels.h      # Enemy models
│           ├── EnvironmentModels.h # Props and environment
│           └── EffectsModels.h    # Visual effects
```

---

## 2. Architecture & Module Integration

### 2.1 System Dependency Graph
```
                    ┌─────────────┐
                    │ Doomers.cpp │  (Entry Point)
                    │   main()    │
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │   Game.h    │  (Main Controller)
                    └──────┬──────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
  ┌─────▼─────┐     ┌──────▼──────┐    ┌─────▼─────┐
  │ Player.h  │     │   Level.h   │    │  Menu.h   │
  │           │     │             │    │           │
  └─────┬─────┘     └──────┬──────┘    └───────────┘
        │                  │
  ┌─────▼─────┐     ┌──────▼──────┐
  │ Camera.h  │     │  Enemy.h    │
  │           │     │Collectible.h│
  └───────────┘     └─────────────┘
        
        ┌─────────────────────────────────────┐
        │         SUPPORT SYSTEMS             │
        ├─────────────────────────────────────┤
        │ Vector3.h      - Math operations    │
        │ Collision.h    - Physics            │
        │ Lighting.h     - Light management   │
        │ Sound.h        - Audio playback     │
        │ HUD.h          - UI rendering       │
        │ TextureManager.h - Texture loading  │
        │ ModelLoader.h  - 3D model loading   │
        │ LowPolyModels.h - Procedural models │
        │ GameConfig.h   - All constants      │
        └─────────────────────────────────────┘
```

### 2.2 Data Flow
```
Input (GLUT Callbacks)
        │
        ▼
   Game::update()
        │
        ├──► Player::update()  ──► Camera::update()
        │
        ├──► Level::update()   ──► Enemy::update()
        │                      ──► Collectible::update()
        │
        ├──► Collision checks
        │
        └──► State transitions

   Game::render()
        │
        ├──► Lighting::apply()
        │
        ├──► Level::draw()     ──► Environment
        │                      ──► Enemies
        │                      ──► Collectibles
        │
        ├──► Player::draw()
        │
        └──► HUD::draw()
```

### 2.3 Game States
Defined in `GameConfig.h`:
```cpp
enum GameState {
    STATE_MAIN_MENU,      // Main menu screen
    STATE_INSTRUCTIONS,   // How to play screen
    STATE_PLAYING,        // Active gameplay
    STATE_PAUSED,         // Pause menu
    STATE_LEVEL_COMPLETE, // Level beaten
    STATE_GAME_OVER,      // Player died
    STATE_WIN             // Game completed
};
```

### 2.4 Level IDs
```cpp
enum LevelID {
    LEVEL_MENU = 0,           // Menu background
    LEVEL_1_FACILITY = 1,     // Indoor sci-fi facility
    LEVEL_2_HELL_ARENA = 2    // Outdoor lava arena with boss
};
```

---

## 3. Core Systems

### 3.1 GameConfig.h - Master Configuration
This file contains ALL tweakable game values. **Modify here to balance the game.**

#### Window Settings
```cpp
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "DOOMERS - Escape the Facility"
```

#### Math Constants
```cpp
#define PI 3.14159265358979323846f
#define DEG2RAD(a) ((a) * 0.0174532925f)
#define RAD2DEG(a) ((a) * 57.2957795131f)
```

#### World Settings
```cpp
#define FLOOR_SIZE 60.0f        // Level floor dimensions
#define WALL_HEIGHT 20.0f       // Wall height
#define BOUNDARY 38.0f          // Level 1 boundary
#define BOUNDARY_LEVEL2 38.0f   // Level 2 boundary
#define GRAVITY 0.018f          // Gravity strength
```

#### Player Settings
```cpp
#define PLAYER_HEIGHT 1.8f              // Eye height
#define PLAYER_SPEED 0.15f              // Base movement speed
#define PLAYER_SPRINT_MULTIPLIER 1.8f   // Sprint speed multiplier
#define PLAYER_JUMP_FORCE 0.25f         // Jump velocity
#define PLAYER_MAX_HEALTH 100           // Starting health
#define PLAYER_COLLISION_RADIUS 0.5f    // Collision cylinder radius
#define PLAYER_INVINCIBILITY_TIME 0.5f  // I-frames after hit
```

#### Weapon Settings
```cpp
#define WEAPON_DAMAGE 25                // Base damage per shot
#define WEAPON_RANGE 50.0f              // Max shooting distance
#define WEAPON_FIRE_RATE 0.2f           // Seconds between shots
#define MAX_AMMO 100                    // Maximum ammo capacity
#define AMMO_PICKUP_AMOUNT 20           // Ammo from pickups
#define WEAPON_HEADSHOT_MULTIPLIER 2.0f // Headshot damage bonus
```

#### Enemy Settings
```cpp
// Regular enemies
#define MAX_ENEMIES 40
#define ENEMY_SPEED 0.08f
#define ENEMY_DAMAGE 10
#define ENEMY_ATTACK_RANGE 2.5f
#define ENEMY_DETECT_RANGE 20.0f

// Zombie specific
#define ZOMBIE_HEALTH 50
#define ZOMBIE_DAMAGE 10
#define ZOMBIE_SCORE 100

// Demon specific
#define DEMON_HEALTH 80
#define DEMON_DAMAGE 20
#define DEMON_SCORE 200

// Boss settings
#define BOSS_HEALTH 300
#define BOSS_DAMAGE 25
#define BOSS_SCORE 2000
#define BOSS_DAMAGE_RESISTANCE 0.5f     // Takes 50% less damage
```

#### Boss Attack Cooldowns
```cpp
#define BOSS_ROCKET_COOLDOWN 5.0f       // Rocket barrage interval
#define BOSS_CHARGE_COOLDOWN 15.0f      // Charge attack interval
#define BOSS_GROUNDSLAM_COOLDOWN 12.0f  // Ground slam interval
#define BOSS_METEOR_COOLDOWN 20.0f      // Meteor shower interval (Phase 2+)
```

#### Collectibles & Powerups
```cpp
#define MAX_HEALTH_PACKS 10
#define MAX_AMMO_BOXES 15
#define HEALTH_PACK_HEAL 25
#define POWERUP_DURATION 12.0f
#define SPEED_BOOST_MULTIPLIER 1.7f
#define DAMAGE_BOOST_MULTIPLIER 2.5f
#define INVINCIBILITY_DURATION 10.0f
```

#### Lava/Hazard Settings
```cpp
#define LAVA_DAMAGE 5               // Damage per tick
#define LAVA_DAMAGE_INTERVAL 0.3f   // Seconds between ticks
#define LAVA_KNOCKBACK 0.3f         // Upward push
```

### 3.2 Vector3.h - 3D Math
The `Vector3` class provides all vector operations needed for 3D games.

#### Constructor
```cpp
Vector3(float x = 0, float y = 0, float z = 0)
```

#### Operators
```cpp
Vector3 operator+(const Vector3& v)   // Addition
Vector3 operator-(const Vector3& v)   // Subtraction
Vector3 operator*(float n)            // Scalar multiply
Vector3 operator/(float n)            // Scalar divide
Vector3& operator+=(const Vector3& v) // Compound add
Vector3& operator-=(const Vector3& v) // Compound subtract
Vector3 operator-()                   // Negation
bool operator==(const Vector3& v)     // Equality
```

#### Methods
```cpp
float length()                    // Get magnitude
float lengthSquared()             // Magnitude squared (faster)
Vector3 unit() / normalize()      // Get unit vector
float dot(const Vector3& v)       // Dot product
Vector3 cross(const Vector3& v)   // Cross product
float distanceTo(const Vector3& v)// Distance to another point
Vector3 lerp(const Vector3& v, float t) // Linear interpolation
Vector3 reflect(const Vector3& normal)  // Reflection
Vector3 rotateY(float angle)      // Rotate around Y axis
Vector3 rotateX(float angle)      // Rotate around X axis
```

#### Static Helpers
```cpp
Vector3::zero()    // (0, 0, 0)
Vector3::one()     // (1, 1, 1)
Vector3::up()      // (0, 1, 0)
Vector3::down()    // (0, -1, 0)
Vector3::forward() // (0, 0, -1)
Vector3::back()    // (0, 0, 1)
Vector3::left()    // (-1, 0, 0)
Vector3::right()   // (1, 0, 0)
```

---

## 4. Rendering & Models

### 4.1 Rendering Pipeline Overview
The game uses OpenGL's **Fixed Function Pipeline** (OpenGL 1.x):

```cpp
// In Game::init()
glEnable(GL_DEPTH_TEST);      // Depth buffering
glEnable(GL_CULL_FACE);       // Backface culling
glEnable(GL_LIGHTING);        // Lighting calculations
glEnable(GL_LIGHT0);          // Main light
glEnable(GL_COLOR_MATERIAL);  // Material colors
glEnable(GL_NORMALIZE);       // Normal normalization
glEnable(GL_FOG);             // Distance fog
```

### 4.2 Model System Architecture
```
LowPolyModels.h (Unified Interface)
        │
        ├── ModelUtils.h        - Primitives & materials
        ├── PlayerModel.h       - Player character
        ├── WeaponModel.h       - Weapons
        ├── EnemyModels.h       - Enemies & boss
        ├── EnvironmentModels.h - Props & level geometry
        └── EffectsModels.h     - Visual effects

ModelLoader.h (3D File Loading)
        │
        └── Uses Assimp library for .obj/.fbx files
```

### 4.3 ModelUtils.h - Rendering Primitives

#### Material Functions
```cpp
// Set diffuse material color
void setColor(float r, float g, float b, float a = 1.0f)

// Set metallic/shiny material
void setColorMetallic(float r, float g, float b)

// Set emissive glow
void setEmissive(float r, float g, float b)
void clearEmissive()
```

#### Primitive Shapes
```cpp
void drawCube(float size)                          // Unit cube
void drawBox(float sx, float sy, float sz)         // Rectangular box
void drawTexturedBox(float sx, sy, sz, GLuint tex) // With texture
void drawCylinder(float radius, float height, int slices = 16)
void drawCone(float radius, float height, int slices = 16)
void drawSphere(float radius, int slices = 16)
void drawTorus(float inner, float outer, int sides, int rings)
```

#### Effect Helpers
```cpp
void enableGlow()        // Additive blending mode
void disableGlow()       // Normal rendering
void enableTransparency()  // Alpha blending
void disableTransparency()
```

### 4.4 LowPolyModels.h - Main Interface
This is the **unified interface** for all procedural models.

#### Player
```cpp
void drawPlayer(float rotY, float walkPhase, float armAimAngle, 
                bool isRunning, float weaponRecoil, bool isFiring, 
                bool weaponLightOn)
```

#### Weapon
```cpp
void drawWeapon(float recoil, bool firing, bool weaponLightOn, 
                float flashlightIntensity)
void drawWeaponFirstPerson(float recoil, float bob, bool firing, 
                           bool weaponLightOn, bool aimDownSights)
```

#### Enemies
```cpp
void drawZombie(float rotY, float animPhase, float health, 
                float maxHealth, float attackPhase)
void drawDemon(float rotY, float animPhase, float attackPhase)
void drawBoss(float rotY, float animPhase, float health, 
              float maxHealth, bool isEnraged)
```

#### Environment
```cpp
void drawCrate(float size = 1.0f)
void drawSciFiCrate(float size = 1.0f)
void drawHealthPack()
void drawAmmoBox()
void drawDoor(bool isOpen, float openAmount)
void drawPlatform(float sizeX, float sizeY, float sizeZ)
void drawLavaRock(float size = 1.0f)
void drawObelisk(float glowIntensity = 1.0f)
void drawPillar(float height)
```

#### Level Geometry
```cpp
void drawFloorTile(float x, float z, float size)
void drawWallSegment(float x, float z, float rotation, 
                     float width, float height)
void drawLevelFloor(float width, float depth)
void drawCeiling(float width, float depth, float height)
void drawArenaWalls(float width, float depth, float height)
```

#### Effects
```cpp
void drawBulletTracer(Vector3 start, Vector3 end, float r, g, b, alpha)
void drawLaserBullet(float length = 2.5f)
void drawMuzzleFlash(float intensity, float size)
void drawExplosion(float progress, float size)
void drawBloodSplatter(float progress, float size)
void drawCrosshair(float spread, bool targetInSight)
```

### 4.5 ModelLoader.h - 3D File Loading
Loads external 3D models using the Assimp library.

#### Model IDs
```cpp
enum ModelID {
    MODEL_CRATE = 0,      // Wooden crate
    MODEL_HEALTHPACK,     // Health pickup
    MODEL_AR_GUN,         // Assault rifle
    MODEL_AMMO_MAGAZINE,  // Ammo pickup
    MODEL_COUNT
};
```

#### Usage
```cpp
// Check if model loaded
if (ModelLoader::isLoaded(MODEL_AR_GUN)) {
    ModelLoader::draw(MODEL_AR_GUN, scale);
}

// Draw at specific position
ModelLoader::drawAt(MODEL_CRATE, x, y, z, scale, rotY);

// Draw grounded (feet at Y=0)
ModelLoader::drawGrounded(MODEL_HEALTHPACK, scale);
```

### 4.6 TextureManager.h - Texture System

#### Texture IDs
```cpp
enum TextureID {
    // Floor textures
    TEX_FLOOR_LAB, TEX_FLOOR_TILE, TEX_FLOOR_METAL,
    TEX_LAVA, TEX_LAVA_GLOW,
    
    // Wall textures
    TEX_WALL_GREY, TEX_WALL_BLUE, TEX_WALL_PANEL,
    
    // Object textures
    TEX_CRATE, TEX_CRATE_SCIFI, TEX_PLATFORM, TEX_ROCK,
    
    // Character textures
    TEX_ENEMY_ZOMBIE, TEX_ENEMY_DEMON, TEX_ENEMY_BOSS,
    
    // Collectible textures
    TEX_HEALTHPACK, TEX_AMMO,
    
    // Skybox (6 faces)
    TEX_SKYBOX_FRONT, TEX_SKYBOX_BACK, TEX_SKYBOX_LEFT,
    TEX_SKYBOX_RIGHT, TEX_SKYBOX_TOP, TEX_SKYBOX_BOTTOM,
    
    TEX_COUNT
};
```

#### Usage
```cpp
// Bind texture for rendering
TextureManager::bind(TEX_CRATE);
// ... render geometry with texture coords ...
TextureManager::unbind();

// Check if loaded
if (TextureManager::isLoaded(TEX_LAVA)) { ... }

// Draw textured primitives
TextureManager::drawTexturedQuad(TEX_FLOOR_LAB, x, y, z, 
                                  width, height, depth, texScale);
TextureManager::drawTexturedBox(TEX_CRATE, x, y, z, 
                                 sizeX, sizeY, sizeZ);

// Draw skybox
TextureManager::drawSkybox(playerX, playerY, playerZ, size);
```

---

*Continue to Part 2 for Player, Enemy, and Camera systems...*
