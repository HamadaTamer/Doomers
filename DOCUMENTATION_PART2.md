# DOOMERS - Technical Documentation Part 2
## Game Objects: Player, Enemy, Camera, Collectibles

---

## 5. Player System (Player.h)

### 5.1 Player Class Overview
The `Player` class manages the playable character including movement, shooting, health, and powerups.

### 5.2 Player Properties

#### Position & Movement
```cpp
Vector3 position;          // Current world position (eye level)
Vector3 velocity;          // Current velocity vector
float rotationY;           // Facing direction (synced with camera)
float speed;               // Current movement speed
bool isSprinting;          // Sprint state
bool isOnGround;           // Ground contact flag
```

#### Stats
```cpp
int health;                // Current health (0-100)
int maxHealth;             // Maximum health
int ammo;                  // Current ammo count
int maxAmmo;               // Maximum ammo capacity
int score;                 // Player score
int enemiesKilled;         // Kill counter
```

#### Weapon State
```cpp
float weaponRecoil;        // Current recoil animation value
float weaponBob;           // Weapon bobbing while walking
float lastFireTime;        // Time of last shot (for fire rate)
bool isFiring;             // Currently firing
float muzzleFlashTimer;    // Muzzle flash duration
```

#### Combat State
```cpp
float damageFlash;         // Red screen flash intensity
float invincibilityTime;   // I-frame timer after damage
float lavaDamageTimer;     // Lava damage cooldown
float lavaInvincibilityTime; // Lava-specific I-frames
bool isInLava;             // Currently in lava
```

#### Powerup States
```cpp
float speedBoostTime;      // Speed boost remaining time
float damageBoostTime;     // Damage boost remaining time
float invincibilityPowerupTime; // Invincibility remaining time
bool hasSpeedBoost;        // Speed boost active
bool hasDamageBoost;       // Damage boost active
bool hasInvincibility;     // Invincibility active

// Shield system
float shieldHealth;        // Current shield HP
float maxShieldHealth;     // Max shield (100)
bool hasShield;            // Shield active
float shieldFlashTime;     // Shield hit flash
```

#### Parkour System
```cpp
bool isDoingParkour;       // Currently vaulting
float parkourProgress;     // Animation progress (0-1)
Vector3 parkourStartPos;   // Vault start position
Vector3 parkourEndPos;     // Vault end position
float parkourHeight;       // Obstacle height
Vector3 parkourDirection;  // Vault direction
```

#### Input State
```cpp
bool moveForward, moveBackward, moveLeft, moveRight;
bool wantJump, wantSprint;
```

### 5.3 Key Player Methods

#### Initialization
```cpp
void reset()           // Reset all stats to defaults
void setCamera(Camera* cam)  // Link to camera
void setPosition(const Vector3& pos)
```

#### Movement & Update
```cpp
void update(float deltaTime)
// - Processes input
// - Applies gravity
// - Updates velocity and position
// - Handles boundary collision
// - Updates powerup timers
// - Updates animations
```

#### Parkour (Vault)
```cpp
void startParkour(Vector3 obstaclePos, float obstacleHeight, 
                  float obstacleDepth, float obstacleRotation)
void updateParkour(float deltaTime)
// Vector-style vault animation:
// Phase 1 (0.0-0.15): Approach run
// Phase 2 (0.15-0.35): Hands plant, body lifts
// Phase 3 (0.35-0.65): Body slides over obstacle
// Phase 4 (0.65-1.0): Drop and land
```

#### Combat
```cpp
bool canFire(float currentTime)  // Check fire rate
void fire(float currentTime)     // Shoot weapon

void takeDamage(int damage, Vector3 attackDir = Vector3(0,0,0))
// - Checks invincibility powerup
// - Shield absorbs damage first
// - Applies knockback
// - Sets I-frames

void takeLavaDamage(int damage)
// - Lava-specific damage with upward boost
```

#### Healing & Powerups
```cpp
void heal(int amount)
void addAmmo(int amount)
void addScore(int points)
void setMaxAmmo()

void activateSpeedBoost(float duration)
void activateDamageBoost(float duration)
void activateInvincibility(float duration)
void activateShield(float amount)
```

#### Shooting
```cpp
Vector3 getShootDirection()  // Get aim direction
Vector3 getShootOrigin()     // Get shot origin point
float getDamageMultiplier()  // 1.0 or 2.5x with powerup
```

#### Rendering
```cpp
void draw()
// - In first person: draws weapon only
// - In third person: draws full player model
// - Handles parkour poses
```

### 5.4 Movement Physics
```cpp
// Base movement calculation
Vector3 moveDir = Vector3(0, 0, 0);
if (moveForward)  moveDir += camera->getForward();
if (moveBackward) moveDir -= camera->getForward();
if (moveRight)    moveDir += camera->getRight();
if (moveLeft)     moveDir -= camera->getRight();

// Normalize diagonal movement
if (moveDir.lengthSquared() > 0.01f) {
    moveDir = moveDir.normalize();
}

// Apply speed modifiers
float currentSpeed = PLAYER_SPEED;
if (isSprinting)    currentSpeed *= PLAYER_SPRINT_MULTIPLIER;
if (hasSpeedBoost)  currentSpeed *= SPEED_BOOST_MULTIPLIER;

// Apply to velocity (horizontal only)
velocity.x = moveDir.x * currentSpeed;
velocity.z = moveDir.z * currentSpeed;

// Gravity (when airborne)
if (!isOnGround) {
    velocity.y -= GRAVITY;
}

// Update position
position = position + velocity;
```

---

## 6. Enemy System (Enemy.h)

### 6.1 Enemy Types
```cpp
enum EnemyType {
    ENEMY_ZOMBIE,   // Basic enemy, slow but durable
    ENEMY_DEMON,    // Faster, more dangerous
    ENEMY_BOSS      // Level 2 boss with special attacks
};
```

### 6.2 Enemy States
```cpp
enum EnemyState {
    ENEMY_IDLE,     // Standing still
    ENEMY_PATROL,   // Walking patrol route
    ENEMY_CHASE,    // Chasing player
    ENEMY_ATTACK,   // Attacking player
    ENEMY_HURT,     // Stunned from damage
    ENEMY_DEAD      // Death animation
};
```

### 6.3 Enemy Properties

#### Basic Properties
```cpp
Vector3 position;          // World position
Vector3 velocity;          // Movement velocity
Vector3 patrolStart;       // Patrol point A
Vector3 patrolEnd;         // Patrol point B
float rotationY;           // Facing direction
float speed;               // Movement speed
float animPhase;           // Animation timer
float hurtTimer;           // Stun duration
float attackCooldown;      // Time until next attack
float deathTimer;          // Death animation timer
float deathScale;          // Shrink during death
bool patrolToEnd;          // Patrol direction
```

#### Combat Properties
```cpp
int health;                // Current health
int maxHealth;             // Maximum health
int damage;                // Damage dealt to player
int scoreValue;            // Points when killed
bool isActiveAttacker;     // One-at-a-time attack system
float damageFlashTimer;    // Red flash when hit
```

#### Boss-Specific Properties
```cpp
// Projectile system
static const int MAX_BOSS_PROJECTILES = 20;
BossProjectile projectiles[MAX_BOSS_PROJECTILES];

// Ability cooldowns
float specialAbilityCooldown;  // Rocket attack
float teleportCooldown;        // Charge attack  
float groundSlamCooldown;      // Ground slam
float meteorShowerCooldown;    // Meteor rain

// Phase system
int currentPhase;              // 1, 2, or 3
float phaseTransitionTimer;    // Phase change animation

// Charge attack
bool isCharging;
float chargeTimer;
Vector3 chargeTarget;

// Animation
float hoverHeight;             // Boss floats
float wingFlapPhase;           // Wing animation
bool hasGravity;               // Falls from platforms
float verticalVelocity;

// Kick attack
bool isKicking;
float kickTimer;
bool kickDamageDealt;
```

### 6.4 Boss Projectile Structure
```cpp
struct BossProjectile {
    Vector3 position;
    Vector3 velocity;
    float lifetime;
    bool active;
    int type;    // 0=fireball, 1=meteor, 2=shockwave
    float size;
};
```

### 6.5 Key Enemy Methods

#### Initialization
```cpp
void init(EnemyType type, Vector3 pos, Vector3 patrolA, Vector3 patrolB)
// Sets stats based on type:
// - ZOMBIE:  50 HP, 100 pts, 1.0x speed
// - DEMON:   80 HP, 200 pts, 1.3x speed  
// - BOSS:   300 HP, 2000 pts, special abilities
```

#### AI Update
```cpp
void update(float deltaTime, const Vector3& playerPos)
// State machine:
// 1. Check hurt/death states
// 2. Update boss abilities and phase
// 3. Calculate distance to player
// 4. Transition states based on range
// 5. Execute current behavior
// 6. Apply movement and gravity
```

#### Behaviors
```cpp
void patrol(float deltaTime)
// Move between patrolStart and patrolEnd

void chase(float deltaTime, const Vector3& playerPos)
// Move toward player position

void chaseBoss(float deltaTime, const Vector3& playerPos)
// Boss chase with charge attacks

void lookAt(const Vector3& target)
// Rotate to face target
```

#### Boss Abilities
```cpp
void updateBossAbilities(float deltaTime, const Vector3& playerPos)
// Manages all boss attacks:

// ROCKET BARRAGE (when medium distance)
// Fires projectiles at player position

// CHARGE ATTACK (when far)
// Rush toward player at high speed

// GROUND SLAM (when close)
// Creates circular shockwave

// METEOR SHOWER (Phase 2+)
// Rains projectiles from above

// PHASE 3 ENRAGE
// Rapid fire mode at low health
```

#### Boss Phases
```cpp
// Phase thresholds (% of max health)
BOSS_PHASE2_THRESHOLD = 0.60f  // Enter phase 2 at 60% HP
BOSS_PHASE3_THRESHOLD = 0.30f  // Enter phase 3 at 30% HP

// Speed multipliers per phase
BOSS_PHASE2_SPEED_MULT = 1.5f
BOSS_PHASE3_SPEED_MULT = 1.8f
```

#### Combat
```cpp
void takeDamage(int dmg)
// - Boss takes 50% reduced damage
// - Sets hurt state and timer
// - Applies knockback (less for boss)
// - Triggers death at 0 HP

bool canAttack()
// Check if attackCooldown <= 0

void performAttack()
// Reset attack cooldown

// Boss kick attack
bool checkKickHit(Vector3 playerPos, float playerRadius)
int getKickDamage()
```

#### Projectiles
```cpp
void fireProjectile(Vector3 target, int type)
// Create new boss projectile

void updateProjectiles(float deltaTime)
// Move and expire projectiles

bool checkProjectileHit(Vector3 playerPos, float hitRadius)
// Check player collision

int checkProjectileHitDamage(Vector3 playerPos, float hitRadius)
// Returns damage amount based on projectile type:
// - Fireball: 15 damage
// - Meteor:   20 damage
// - Shockwave: 25 damage
```

### 6.6 One-at-a-Time Attack System
Only the **closest enemy** to the player can attack at once. This prevents overwhelming the player.

```cpp
// In Game::updateGameplay()
int closestEnemy = -1;
float closestDist = ENEMY_DETECT_RANGE + 1.0f;

for (int i = 0; i < numEnemies; i++) {
    if (!enemies[i].active || enemies[i].isDead()) {
        enemies[i].isActiveAttacker = false;
        continue;
    }
    
    float dist = enemies[i].position.distanceTo(player.position);
    if (dist < closestDist) {
        closestDist = dist;
        closestEnemy = i;
    }
    enemies[i].isActiveAttacker = false;
}

// Only closest becomes active attacker
if (closestEnemy >= 0 && closestDist < ENEMY_DETECT_RANGE) {
    enemies[closestEnemy].isActiveAttacker = true;
}
```

---

## 7. Camera System (Camera.h)

### 7.1 Camera Modes
```cpp
enum CameraMode {
    CAMERA_FIRST_PERSON,   // FPS view
    CAMERA_THIRD_PERSON    // Over-the-shoulder TPS view
};
```

### 7.2 Camera Properties
```cpp
Vector3 eye;        // Camera position
Vector3 center;     // Look-at point
Vector3 up;         // Up vector (0, 1, 0)

float pitch;        // Vertical rotation (degrees)
float yaw;          // Horizontal rotation (degrees)

// Third person settings
float distance;     // Distance behind player (6.0)
float height;       // Height above player (3.5)
float lookAheadDist; // How far ahead to look (8.0)

// Smooth movement
Vector3 smoothEye;
Vector3 smoothCenter;
float smoothSpeed;

// Effects
float shakeIntensity;
float shakeDuration;
float shakeTimer;
float parkourTilt;  // Tilt during vault

float sensitivity;  // Mouse sensitivity (0.12)
CameraMode mode;
```

### 7.3 Key Camera Methods

#### Mode Control
```cpp
void setMode(CameraMode newMode)
void toggleMode()  // Switch between FPS/TPS
```

#### Mouse Look
```cpp
void rotate(float deltaX, float deltaY)
// Apply mouse movement to yaw/pitch
// Different sensitivity per mode
// Clamp pitch:
//   - Third person: -35° to +25°
//   - First person: -85° to +85°
```

#### Camera Shake
```cpp
void addShake(float intensity, float duration)
Vector3 getShakeOffset()  // Get random shake offset
```

#### Update
```cpp
void update(const Vector3& playerPos, float playerRotY, float deltaTime)
// FIRST PERSON:
//   - Eye at player eye level
//   - Look direction from yaw/pitch
//   - Reduced shake effect

// THIRD PERSON:
//   - Over-shoulder positioning
//   - Dynamic height follows jumps
//   - Wall/boundary collision
//   - Smooth interpolation (35% lerp)
//   - Pitch affects aim point
```

#### Direction Getters
```cpp
Vector3 getForward()        // Horizontal forward (for movement)
Vector3 getRight()          // Horizontal right (for strafing)
Vector3 getLookDirection()  // Full 3D aim direction (for shooting)
```

#### Apply Transform
```cpp
void apply()
// Call gluLookAt with current eye, center, up
```

### 7.4 Third Person Camera Math
```cpp
// Calculate ideal camera position
float radYaw = DEG2RAD(yaw);
float radPitch = DEG2RAD(pitch);

// Behind and above player, offset right for over-shoulder
float shoulderOffset = 1.0f;
float idealCamX = playerPos.x - sin(radYaw) * distance 
                               + cos(radYaw) * shoulderOffset;
float idealCamY = playerPos.y + height;
float idealCamZ = playerPos.z + cos(radYaw) * distance 
                               + sin(radYaw) * shoulderOffset;

// Smooth interpolation
smoothEye.x += (idealCamX - smoothEye.x) * 0.35f;
smoothEye.y += (idealCamY - smoothEye.y) * 0.35f;
smoothEye.z += (idealCamZ - smoothEye.z) * 0.35f;

// Look at player + pitch offset
float lookX = playerPos.x + sin(radYaw) * lookAheadDist;
float lookY = playerPos.y + sin(radPitch) * 3.0f;
float lookZ = playerPos.z - cos(radYaw) * lookAheadDist;
```

---

## 8. Collectible System (Collectible.h)

### 8.1 Collectible Types
```cpp
enum CollectibleType {
    COLLECT_HEALTH,         // Health pack (+25 HP)
    COLLECT_AMMO,           // Ammo box (+20 ammo)
    COLLECT_KEYCARD,        // Door keycard
    COLLECT_SPEED_BOOST,    // 12s speed boost
    COLLECT_DAMAGE_BOOST,   // 12s damage boost
    COLLECT_INVINCIBILITY,  // 10s invincibility
    COLLECT_MAX_AMMO,       // Full ammo refill
    COLLECT_SHIELD          // Energy shield
};
```

### 8.2 Collectible Properties
```cpp
Vector3 position;          // World position
CollectibleType type;      // Type of pickup
bool active;               // Currently in world
float rotation;            // Spin animation
float bobPhase;            // Bobbing animation
float pickupScale;         // Collection animation scale
bool beingCollected;       // Currently being picked up
int value;                 // Amount (HP, ammo, etc.)
int keycardID;             // For keycards: which door
```

### 8.3 Key Methods
```cpp
void init(CollectibleType t, Vector3 pos, int val = 0)
// Set type, position, and value

void update(float deltaTime)
// - Rotate 60°/sec
// - Bob up/down (sine wave)
// - Animate collection (scale up, spin, fade)

bool checkCollection(Vector3 playerPos, float collectRadius = 2.5f)
// Check if player is close enough
// Uses horizontal distance + vertical tolerance

void draw()
// Render pickup with glow effect
// Different visuals per type:
// - Health: White box with red cross
// - Ammo: Magazine model or box
// - Keycard: Colored card
// - Speed: Lightning bolt with cyan glow
// - Damage: Fiery orb with spikes
// - Invincibility: Golden star shield
// - Shield: Blue energy sphere with rings
```

### 8.4 Collecting Items (in Game.h)
```cpp
void collectItem(Collectible& item) {
    switch (item.type) {
        case COLLECT_HEALTH:
            player.heal(item.value);      // +25 HP
            player.addScore(10);
            break;
        case COLLECT_AMMO:
            player.addAmmo(item.value);   // +20 ammo
            player.addScore(10);
            break;
        case COLLECT_KEYCARD:
            keycards[numKeycards++] = item.keycardID;
            player.addScore(50);
            break;
        case COLLECT_SPEED_BOOST:
            player.activateSpeedBoost(POWERUP_DURATION);
            player.addScore(25);
            break;
        case COLLECT_DAMAGE_BOOST:
            player.activateDamageBoost(POWERUP_DURATION);
            player.addScore(25);
            break;
        case COLLECT_INVINCIBILITY:
            player.activateInvincibility(INVINCIBILITY_DURATION);
            player.addScore(50);
            break;
        case COLLECT_MAX_AMMO:
            player.setMaxAmmo();
            player.addScore(25);
            break;
        case COLLECT_SHIELD:
            player.activateShield(PLAYER_SHIELD_MAX);
            player.addScore(30);
            break;
    }
}
```

---

*Continue to Part 3 for Level System, Collision, Lighting, Audio, and HUD...*
