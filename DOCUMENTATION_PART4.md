# DOOMERS - Technical Documentation Part 4
## Models, Textures, Animation, and Game Loop

---

## 14. Model System Architecture

### 14.1 Model Hierarchy
```
LowPolyModels.h (Main Wrapper)
├── ModelUtils.h       - Primitive drawing utilities
├── PlayerModel.h      - Player character
├── WeaponModel.h      - Weapons and arms
├── EnemyModels.h      - Zombie, Demon, Boss
├── EnvironmentModels.h - Crates, platforms, doors
├── EffectsModels.h    - Particles, tracers, explosions
└── ModelLoader.h      - 3D model loading (Assimp)
    └── AnimationLoader - Frame-based animations
```

### 14.2 LowPolyModels Namespace
The `LowPolyModels` namespace provides a unified interface for all model rendering.

```cpp
namespace LowPolyModels {
    // Time management
    void updateAnimationTime(float deltaTime);
    
    // Player
    void drawPlayer(float rotY, float walkPhase, float armAimAngle, 
                    bool isRunning, float weaponRecoil, bool isFiring, 
                    bool weaponLightOn);
    void drawParkourArmsFirstPerson(float parkourProgress);
    void drawPlayerParkourPose(float rotY, float parkourProgress);
    
    // Weapon
    void drawWeapon(float recoil, bool firing, bool weaponLightOn, 
                    float flashlightIntensity);
    void drawWeaponFirstPerson(float recoil, float bob, bool firing, 
                               bool weaponLightOn, bool aimDownSights);
    
    // Enemies
    void drawZombie(float rotY, float animPhase, float health, 
                    float maxHealth, float attackPhase = 0.0f);
    void drawDemon(float rotY, float animPhase, float attackPhase);
    void drawBoss(float rotY, float animPhase, float health, 
                  float maxHealth, bool isEnraged);
    
    // Environment
    void drawCrate(float size = 1.0f);
    void drawSciFiCrate(float size = 1.0f);
    void drawHealthPack();
    void drawAmmoBox();
    void drawKeycard(float r, float g, float b);
    void drawDoor(bool isOpen, float openAmount);
    void drawPlatform(float sizeX, float sizeY, float sizeZ);
    void drawLavaRock(float size = 1.0f);
    void drawObelisk(float glowIntensity = 1.0f);
    void drawWallPanel(float width, float height);
    void drawPillar(float height);
    
    // Effects
    void drawBulletTracer(Vector3 start, Vector3 end, float r, g, b, alpha);
    void drawLaserBullet(float length = 2.5f);
    void drawMuzzleFlash(float intensity = 1.0f, float size = 1.0f);
    void drawExplosion(float progress, float size = 1.0f);
    void drawBloodSplatter(float progress, float size = 1.0f);
    void drawPickupGlow(float r, g, b, float intensity = 1.0f);
    void drawWeaponLightCone(float range, float angle, float intensity);
    void drawFootstepDust(float progress, float size = 0.3f);
    void drawEnergyShield(float health, float maxHealth);
    void drawTeleportEffect(float progress, bool appearing);
    void drawDamageIndicator(float angle, float intensity);
    
    // Level geometry
    void drawFloorTile(float x, float z, float size);
    void drawFloorTileSimple(float x, float z, float size);
}
```

---

## 15. ModelUtils.h - Primitive Utilities

### 15.1 Material Functions
```cpp
namespace ModelUtils {
    float globalTime;  // Animation timer
    
    void updateTime(float deltaTime);
    float getTime();
    
    // Material setup
    void setColor(float r, float g, float b);
    void setColorMetallic(float r, float g, float b);
    void setColorGlossy(float r, float g, float b);
    void setEmissive(float r, float g, float b);
    void clearEmissive();
    void setTransparency(float alpha);
    
    // Primitive drawing
    void drawBox(float w, float h, float d);
    void drawCylinder(float radius, float height, int segments = 12);
    void drawSphere(float radius, int slices = 8, int stacks = 8);
    void drawCone(float radius, float height, int segments = 12);
    void drawTorus(float innerRadius, float outerRadius, 
                   int sides = 8, int rings = 16);
}
```

### 15.2 How Materials Work
```cpp
void setColorMetallic(float r, float g, float b) {
    GLfloat ambient[] = {r * 0.3f, g * 0.3f, b * 0.3f, 1.0f};
    GLfloat diffuse[] = {r, g, b, 1.0f};
    GLfloat specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0f);
}

void setEmissive(float r, float g, float b) {
    GLfloat emission[] = {r, g, b, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
}

void clearEmissive() {
    GLfloat noEmission[] = {0, 0, 0, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
}
```

---

## 16. 3D Model Loading (ModelLoader.h)

### 16.1 Model IDs
```cpp
enum ModelID {
    MODEL_CRATE = 0,
    MODEL_HEALTHPACK,
    MODEL_AR_GUN,
    MODEL_AMMO_MAGAZINE,
    MODEL_COUNT
};
```

### 16.2 Model3D Structure
```cpp
struct Model3D {
    std::vector<SimpleMesh> meshes;
    bool loaded;
    
    // Bounding box
    float minX, maxX, minY, maxY, minZ, maxZ;
    float centerX, centerY, centerZ;
    float scale;  // Normalizing scale factor
    
    void calculateBounds();
    void draw(float customScale = 1.0f, bool centerModel = true);
};
```

### 16.3 SimpleMesh Structure
```cpp
struct SimpleMesh {
    std::vector<float> vertices;     // x, y, z per vertex
    std::vector<float> normals;      // nx, ny, nz per vertex
    std::vector<float> texCoords;    // u, v per vertex
    std::vector<unsigned int> indices;
    GLuint textureID;
    bool hasTexture;
    float diffuseColor[3];
    
    void draw() const;  // Render using vertex arrays
};
```

### 16.4 ModelLoader Class
```cpp
class ModelLoader {
    static Model3D models[MODEL_COUNT];
    static bool initialized;
    
public:
    static void init();              // Load all models at startup
    static bool isLoaded(ModelID id);
    static const Model3D* get(ModelID id);
    
    static void draw(ModelID id, float scale = 1.0f, bool centerModel = true);
    static void drawGrounded(ModelID id, float scale = 1.0f);  // Feet at Y=0
    static void drawAt(ModelID id, float x, y, z, float scale, 
                       float rotY, rotX, rotZ);
    
    static void cleanup();
    
    // For loading animation frames
    static bool loadModelDirect(const char* filepath, Model3D& model,
                                const char* textureOverride, bool invertY);
};
```

### 16.5 Model Loading Process
```cpp
// Using Assimp library
static bool loadModel(const char* filepath, Model3D& model, 
                      const char* textureOverride = nullptr, 
                      bool invertTextureY = true) {
    
    Assimp::Importer importer;
    
    // Import flags
    unsigned int flags = aiProcess_Triangulate |
                         aiProcess_GenNormals |
                         aiProcess_JoinIdenticalVertices |
                         aiProcess_OptimizeMeshes;
    
    // Only flip UVs for non-FBX files
    bool isFBX = strstr(filepath, ".fbx") != nullptr;
    if (!isFBX) {
        flags |= aiProcess_FlipUVs;
    }
    
    const aiScene* scene = importer.ReadFile(filepath, flags);
    
    // Process scene nodes recursively
    processNode(scene->mRootNode, scene, model, baseDir);
    
    // Apply texture override if specified
    if (textureOverride) {
        GLuint tex = SOIL_load_OGL_texture(textureOverride, ...);
        for (auto& mesh : model.meshes) {
            mesh.textureID = tex;
            mesh.hasTexture = true;
        }
    }
    
    model.calculateBounds();
    model.loaded = true;
    return true;
}
```

### 16.6 Model Paths
```cpp
// Models are loaded from res/Models3D/
// Each model has its own folder with source/ and textures/ subfolders

// Crate
"res/Models3D/gart130-crate/source/L_Crate_2fbx.obj"
"res/Models3D/gart130-crate/textures/L_Crate.2fbx_lambert5_BaseColor.png"

// Health Pack
"res/Models3D/health-pack/source/HealthPack/healthpack.obj"
"res/Models3D/health-pack/textures/Healthpack_Textured_Albedo.png"

// AR Gun
"res/Models3D/AR/source/Gun.obj"
"res/Models3D/AR/textures/GAP_Examen_Gun_albedo_DriesDeryckere.tga.png"

// Ammo Magazine
"res/Models3D/ak-47-magazine/source/ak_47_round.obj"
"res/Models3D/ak-47-magazine/textures/ak_47_round_BaseColor.jpeg"
```

---

## 17. Animation System (AnimationLoader)

### 17.1 Animation Types
```cpp
enum AnimationType {
    ANIM_IDLE = 0,  // Not implemented
    ANIM_WALK,      // Walking cycle
    ANIM_KICK,      // Attack animation
    ANIM_COUNT
};
```

### 17.2 FrameAnimation Structure
```cpp
struct FrameAnimation {
    Model3D frames[MAX_ANIM_FRAMES];  // 32 max frames
    int frameCount;
    float fps;
    bool loaded;
    bool loop;
    
    // Reference values for consistent positioning
    float refScale;
    float refCenterX, refCenterZ;
    float refMinY;
};
```

### 17.3 AnimationLoader Class
```cpp
class AnimationLoader {
    static FrameAnimation animations[ANIM_COUNT];
    static bool initialized;
    
public:
    static void init();
    static bool isLoaded(AnimationType anim);
    static int getFrameCount(AnimationType anim);
    
    // Get frame index from time
    static int getFrameIndex(AnimationType anim, float time);
    
    // Draw animation frame
    static void drawGrounded(AnimationType anim, int frameIndex, float scale);
    static void drawAnimated(AnimationType anim, float time, float scale);
};
```

### 17.4 Frame-Based Animation Workflow
1. Export individual frames as FBX from animation software
2. Place in `res/Models3D/devil/baked/` folder
3. Name frames: `walk_001.fbx`, `walk_009.fbx`, etc.
4. AnimationLoader loads subset of frames at startup
5. At runtime, calculate frame index from time
6. Draw appropriate frame

```cpp
// Loading animation frames
int walkFrames[] = {1, 9, 17, 25, 33, ...};  // Every 8th frame
loadAnimationFrames(ANIM_WALK, bakedPath, "walk", texturePath, walkFrames, 30);

// Drawing at runtime
float animTime = elapsedTime;
int frameIndex = (int)(animTime * fps) % frameCount;
AnimationLoader::drawGrounded(ANIM_WALK, frameIndex, 1.5f);
```

---

## 18. Texture System (TextureManager.h)

### 18.1 Texture IDs
```cpp
enum TextureID {
    TEX_FLOOR = 0,
    TEX_WALL,
    TEX_CEILING,
    TEX_CRATE,
    TEX_CRATE_SCIFI,
    TEX_PLATFORM,
    TEX_LAVA,
    TEX_LAVA_GLOW,
    TEX_ROCK,
    TEX_WALL_PANEL,
    TEX_WALL_ORANGE_WARNING,
    TEX_PARKOUR,
    TEX_SKYBOX_FRONT,
    TEX_SKYBOX_BACK,
    TEX_SKYBOX_LEFT,
    TEX_SKYBOX_RIGHT,
    TEX_SKYBOX_TOP,
    TEX_SKYBOX_BOTTOM,
    // Skybox 2 (Hell)
    TEX_SKYBOX2_FRONT,
    TEX_SKYBOX2_BACK,
    TEX_SKYBOX2_LEFT,
    TEX_SKYBOX2_RIGHT,
    TEX_SKYBOX2_TOP,
    TEX_SKYBOX2_BOTTOM,
    TEXTURE_COUNT
};
```

### 18.2 TextureManager Class
```cpp
class TextureManager {
    static GLuint textures[TEXTURE_COUNT];
    static bool loaded[TEXTURE_COUNT];
    static bool initialized;
    
public:
    static void init();              // Load all textures
    static bool isLoaded(TextureID id);
    static void bind(TextureID id);  // glBindTexture
    static void unbind();            // glBindTexture(0)
    static GLuint get(TextureID id);
    
    // Utility functions
    static void drawTexturedQuad(TextureID id, float x, y, w, h);
    static void drawTexturedBox(TextureID id, float x, y, z, 
                                float w, h, d, float texScale = 1.0f);
    
    static void cleanup();
};
```

### 18.3 Texture Loading with SOIL
```cpp
static void loadTexture(TextureID id, const char* path) {
    GLuint tex = SOIL_load_OGL_texture(
        path,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS
    );
    
    if (tex > 0) {
        textures[id] = tex;
        loaded[id] = true;
        
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
}
```

### 18.4 Texture Paths
```
res/Textures/
├── Floor.png
├── Wall.png
├── Ceiling.png
├── Crate.png
├── CrateSciFi.png
├── Platform.png
├── Lava.png
├── LavaGlow.png
├── Rock.png
├── WallPanel.png
├── WallOrangeWarning.png
├── Parkour.png
└── Skyboxes/
    ├── Space/
    │   ├── front.png
    │   ├── back.png
    │   ├── left.png
    │   ├── right.png
    │   ├── top.png
    │   └── bottom.png
    └── TitanMoon/
        ├── front.png
        ├── back.png
        └── ...
```

---

## 19. Game Class (Game.h)

### 19.1 Game State Machine
```cpp
enum GameState {
    STATE_MAIN_MENU,
    STATE_INSTRUCTIONS,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_LEVEL_COMPLETE,
    STATE_WIN
};
```

### 19.2 Game Class Properties
```cpp
class Game {
    // Core systems
    Player player;
    Camera camera;
    Level currentLevel;
    HUD hud;
    Menu menu;
    LightingSystem lighting;
    SoundSystem sound;
    
    // State
    GameState state;
    int currentLevelNum;
    float deltaTime;
    int lastUpdateTime;
    
    // Mouse
    int lastMouseX, lastMouseY;
    bool mouseCaptured;
    int windowCenterX, windowCenterY;
    
    // Input
    bool shiftPressed;
    
    // Keycards
    int keycards[MAX_KEYCARDS];
    int numKeycards;
    
    // Interaction
    int nearInteractableType;  // 0=none, 1=mystery, 2=parkour, 3=exit
    int nearInteractableIndex;
    
    // Particles
    Particle particles[MAX_PARTICLES];
    LaserBullet laserBullets[MAX_LASER_BULLETS];
    float muzzleFlashTime;
    
    // Victory effects
    bool victoryShakeActive;
    float victoryShakeTime, victoryShakeDuration, victoryShakeIntensity;
    
    // Level transition
    bool isTransitioning;
    float transitionTime, transitionDuration;
    int transitionPhase;  // 0=fadeOut, 1=effects, 2=fadeIn
    int transitionTargetLevel;
};
```

### 19.3 Initialization
```cpp
void init() {
    player.setCamera(&camera);
    
    // Initialize systems
    TextureManager::init();
    ModelLoader::init();
    AnimationLoader::init();
    
    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    
    // Fog for atmosphere
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 30.0f);
    glFogf(GL_FOG_END, 80.0f);
    
    state = STATE_MAIN_MENU;
    sound.playMusic(Sounds::MUSIC_MENU);
}
```

### 19.4 Main Update Loop
```cpp
void update() {
    // Calculate delta time
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = (currentTime - lastUpdateTime) / 1000.0f;
    if (deltaTime > 0.1f) deltaTime = 0.1f;  // Cap
    lastUpdateTime = currentTime;
    
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
```

### 19.5 Gameplay Update
```cpp
void updateGameplay() {
    // Handle level transition
    if (isTransitioning) {
        updateLevelTransition();
        if (transitionPhase == 1) return;
    }
    
    // Update animation time
    LowPolyModels::updateAnimationTime(deltaTime);
    
    // Sprint from shift key
    player.wantSprint = shiftPressed;
    
    // Gravity check
    float groundHeight = currentLevel.checkPlatformCollision(
        player.position, PLAYER_COLLISION_RADIUS);
    if (groundHeight <= 0.0f && player.position.y > PLAYER_HEIGHT + 0.1f) {
        player.isOnGround = false;
    }
    
    // Update player
    player.update(deltaTime);
    
    // Platform collision
    // ... (detailed collision logic)
    
    // Update camera
    camera.update(player.position, player.rotationY);
    
    // Update level
    currentLevel.update(deltaTime, player.position);
    
    // Update lighting
    lighting.update(deltaTime, camera.eye, camera.getLookDirection());
    
    // Enemy attack system (one-at-a-time)
    // ... (find closest enemy, make active attacker)
    
    // Check collectibles
    // ... (iterate and collect items)
    
    // Update particles and effects
    updateParticles();
    updateLaserBullets();
    
    // Check win/lose conditions
    if (player.isDead()) {
        onGameOver();
    } else if (currentLevel.isComplete()) {
        onLevelComplete();
    } else if (currentLevel.isTimeUp()) {
        onGameOver();
    }
    
    // Lava damage
    if (currentLevel.hasLava && player.position.y < currentLevel.lavaHeight + 1.0f) {
        player.takeLavaDamage(LAVA_DAMAGE);
    }
}
```

### 19.6 Shooting System
```cpp
void shoot() {
    if (!player.canFire(currentTime)) return;
    
    player.fire(currentTime);
    muzzleFlashTime = 0.08f;
    sound.playSound(Sounds::SFX_AR_FIRE);
    
    // Create ray from player
    Ray shootRay(player.getShootOrigin(), player.getShootDirection());
    
    float closestHit = WEAPON_RANGE;
    int hitEnemy = -1;
    bool hitWall = false;
    
    // 1. Check walls first (prevents shooting through)
    float wallHitDist = checkBulletWallCollision(shootRay, WEAPON_RANGE);
    if (wallHitDist > 0 && wallHitDist < closestHit) {
        closestHit = wallHitDist;
        hitWall = true;
    }
    
    // 2. Check obstacles (crates, parkour, platforms)
    // ...
    
    // 3. Check enemies (only if bullet hasn't hit closer wall)
    for (int i = 0; i < currentLevel.numEnemies; i++) {
        if (!currentLevel.enemies[i].active) continue;
        
        Vector3 enemyCenter = currentLevel.enemies[i].position + Vector3(0, 1.5f, 0);
        Sphere enemySphere(enemyCenter, 1.2f);
        
        float hitDist = shootRay.intersects(enemySphere);
        if (hitDist > 0 && hitDist < closestHit) {
            closestHit = hitDist;
            hitEnemy = i;
            hitWall = false;
        }
    }
    
    // Spawn laser bullet effect
    Vector3 laserEnd = shootRay.getPoint(closestHit);
    spawnLaserBullet(player.getShootOrigin(), laserEnd, 0.0f, 1.0f, 0.3f);
    
    // Apply damage if hit enemy
    if (hitEnemy >= 0) {
        int damage = PLAYER_DAMAGE * player.getDamageMultiplier();
        currentLevel.enemies[hitEnemy].takeDamage(damage);
        player.addScore(10);
        sound.playSound(Sounds::SFX_ENEMY_HURT);
        
        // Spawn blood particles
        for (int i = 0; i < 5; i++) {
            spawnParticle(laserEnd, randomVelocity(), 0.8f, 0.0f, 0.0f);
        }
    }
}
```

### 19.7 Input Handling
```cpp
void handleKeyDown(unsigned char key) {
    switch (state) {
        case STATE_PLAYING:
            if (key == 'w' || key == 'W') player.moveForward = true;
            if (key == 's' || key == 'S') player.moveBackward = true;
            if (key == 'a' || key == 'A') player.moveLeft = true;
            if (key == 'd' || key == 'D') player.moveRight = true;
            if (key == ' ') player.wantJump = true;
            if (key == 'e' || key == 'E') handleInteraction();
            if (key == 'v' || key == 'V') camera.toggleMode();
            if (key == 27) { state = STATE_PAUSED; menu.setMenu(MENU_PAUSE); }
            break;
            
        case STATE_MAIN_MENU:
        case STATE_PAUSED:
            // Menu navigation
            break;
    }
}

void handleSpecialKeyDown(int key) {
    if (key == GLUT_KEY_SHIFT_L || key == GLUT_KEY_SHIFT_R) {
        shiftPressed = true;
    }
}

void handleMouseClick(int button, int state, int x, int y) {
    if (this->state == STATE_PLAYING && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        shoot();
    }
}

void handleMouseMove(int x, int y) {
    if (!mouseCaptured || state != STATE_PLAYING) return;
    
    int dx = x - windowCenterX;
    int dy = y - windowCenterY;
    
    if (dx != 0 || dy != 0) {
        camera.rotate((float)dx, (float)dy);
        player.rotationY = camera.yaw;
        
        // Re-center mouse
        glutWarpPointer(windowCenterX, windowCenterY);
    }
}
```

### 19.8 Rendering
```cpp
void draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    switch (state) {
        case STATE_MAIN_MENU:
        case STATE_INSTRUCTIONS:
        case STATE_PAUSED:
            drawWorld();
            menu.draw();
            break;
            
        case STATE_PLAYING:
            drawWorld();
            drawHUD();
            break;
            
        case STATE_GAME_OVER:
        case STATE_LEVEL_COMPLETE:
        case STATE_WIN:
            drawWorld();
            menu.draw();
            break;
    }
    
    glutSwapBuffers();
}

void drawWorld() {
    // Setup camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, aspectRatio, NEAR_PLANE, FAR_PLANE);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.apply();
    
    // Apply lighting
    lighting.apply();
    
    // Draw level
    currentLevel.draw(camera.eye);
    
    // Draw enemies
    for (int i = 0; i < currentLevel.numEnemies; i++) {
        if (currentLevel.enemies[i].active) {
            currentLevel.enemies[i].draw();
        }
    }
    
    // Draw collectibles
    for (int i = 0; i < currentLevel.numCollectibles; i++) {
        if (currentLevel.collectibles[i].active) {
            currentLevel.collectibles[i].draw();
        }
    }
    
    // Draw player (third person only)
    if (camera.mode == CAMERA_THIRD_PERSON) {
        player.draw();
    }
    
    // Draw effects
    drawParticles();
    drawLaserBullets();
    if (muzzleFlashTime > 0) drawMuzzleFlash();
    
    // Draw first-person weapon
    if (camera.mode == CAMERA_FIRST_PERSON) {
        drawFirstPersonWeapon();
    }
}
```

---

*Continue to Part 5 for Implementation Guide: How to add features and modify existing systems...*
