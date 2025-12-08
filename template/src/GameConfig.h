// ============================================================================
// DOOMERS - GameConfig.h
// MASTER CONFIGURATION FILE - All tweakable game values in one place!
// Modify these values to balance the game without touching code
// ============================================================================
#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// ==================== WINDOW SETTINGS ====================
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "DOOMERS - Escape the Facility"

// ==================== MATH CONSTANTS ====================
#define PI 3.14159265358979323846f
#define DEG2RAD(a) ((a) * 0.0174532925f)
#define RAD2DEG(a) ((a) * 57.2957795131f)
#define GLUT_KEY_ESCAPE 27

// ==================== WORLD SETTINGS ====================
#define FLOOR_SIZE 60.0f
#define WALL_HEIGHT 20.0f
#define CEILING_HEIGHT 10.0f
#define BOUNDARY 38.0f
#define BOUNDARY_LEVEL2 38.0f  // Compact arena for Level 2
#define GRAVITY 0.018f

// ==================== PLAYER SETTINGS ====================
#define PLAYER_HEIGHT 1.8f
#define PLAYER_SPEED 0.15f
#define PLAYER_SPRINT_MULTIPLIER 1.8f
#define PLAYER_JUMP_FORCE 0.25f
#define PLAYER_MAX_HEALTH 100
#define PLAYER_COLLISION_RADIUS 0.5f
#define PLAYER_INVINCIBILITY_TIME 0.5f      // Seconds of invincibility after being hit

// ==================== WEAPON SETTINGS ====================
#define WEAPON_DAMAGE 25
#define WEAPON_RANGE 50.0f
#define WEAPON_FIRE_RATE 0.2f               // Seconds between shots
#define MAX_AMMO 100
#define AMMO_PICKUP_AMOUNT 20
#define WEAPON_HEADSHOT_MULTIPLIER 2.0f     // Damage multiplier for headshots

// ==================== REGULAR ENEMY SETTINGS ====================
#define MAX_ENEMIES 40
#define ENEMY_SPEED 0.08f
#define ENEMY_DAMAGE 10
#define ENEMY_ATTACK_RANGE 2.5f
#define ENEMY_DETECT_RANGE 20.0f
#define ENEMY_ATTACK_COOLDOWN 1.0f          // Seconds between enemy attacks

// --- Zombie Stats ---
#define ZOMBIE_HEALTH 50
#define ZOMBIE_DAMAGE 10
#define ZOMBIE_SPEED_MULT 1.0f              // Multiplier of base ENEMY_SPEED
#define ZOMBIE_SCORE 100

// --- Demon Stats ---
#define DEMON_HEALTH 80
#define DEMON_DAMAGE 20
#define DEMON_SPEED_MULT 1.3f
#define DEMON_SCORE 200

// ==================== BOSS SETTINGS (THE BIG ONE!) ====================
#define BOSS_HEALTH 300                    // How tanky the boss is
#define BOSS_DAMAGE 25                      // Melee damage (lowered from 40)
#define BOSS_SPEED_MULT 1.0f                // Movement speed multiplier (slower)
#define BOSS_SCORE 2000
#define BOSS_DAMAGE_RESISTANCE 0.5f         // Takes 50% less damage

// --- Boss Projectile Damage ---
#define BOSS_FIREBALL_DAMAGE 15             // Damage from fireballs/rockets
#define BOSS_METEOR_DAMAGE 20               // Damage from meteors
#define BOSS_GROUNDSLAM_DAMAGE 25           // Damage from ground slam projectiles

// --- Boss Attack Cooldowns (in seconds) - HIGHER = EASIER ---
#define BOSS_ROCKET_COOLDOWN 5.0f           // Time between rocket barrages
#define BOSS_CHARGE_COOLDOWN 15.0f          // Time between charge attacks
#define BOSS_GROUNDSLAM_COOLDOWN 12.0f      // Time between ground slams
#define BOSS_METEOR_COOLDOWN 20.0f          // Time between meteor showers (Phase 2+)
#define BOSS_ENRAGE_FIRE_RATE 1.0f          // Phase 3 rapid fire cooldown

// --- Boss Attack Parameters ---
#define BOSS_ROCKET_COUNT 1                 // Rockets per barrage (was 1 + phase)
#define BOSS_GROUNDSLAM_PROJECTILES 4       // Shockwave projectiles (was 6-12)
#define BOSS_METEOR_COUNT 3                 // Meteors per shower (was 4-8)
#define BOSS_PROJECTILE_SPEED 20.0f         // Fireball speed (was 25)
#define BOSS_METEOR_SPEED 15.0f             // Meteor fall speed (was 20)
#define BOSS_GROUNDSLAM_SPEED 10.0f         // Shockwave speed (was 12)

// --- Boss Phase Thresholds (% of max health) ---
#define BOSS_PHASE2_THRESHOLD 0.60f         // Enter phase 2 at 60% health
#define BOSS_PHASE3_THRESHOLD 0.30f         // Enter phase 3 at 30% health

// --- Boss Charge Attack ---
#define BOSS_CHARGE_DURATION 1.5f           // How long the charge lasts
#define BOSS_CHARGE_SPEED_MULT 3.0f         // Speed multiplier during charge
#define BOSS_PHASE2_SPEED_MULT 1.5f         // Speed boost at phase 2
#define BOSS_PHASE3_SPEED_MULT 1.8f         // Speed boost at phase 3 (enraged)

// ==================== COLLECTIBLES ====================
#define MAX_HEALTH_PACKS 10
#define MAX_AMMO_BOXES 15
#define MAX_KEYCARDS 5
#define HEALTH_PACK_HEAL 25

// ==================== POWERUPS ====================
#define MAX_POWERUPS 15
#define POWERUP_DURATION 12.0f
#define SPEED_BOOST_MULTIPLIER 1.7f
#define DAMAGE_BOOST_MULTIPLIER 2.5f
#define INVINCIBILITY_DURATION 10.0f

// ==================== LAVA/HAZARD DAMAGE ====================
#define LAVA_DAMAGE 5                       // Damage per tick when in lava
#define LAVA_DAMAGE_INTERVAL 0.3f           // Seconds between lava damage ticks
#define LAVA_KNOCKBACK 0.3f                 // Upward push when in lava
#define LAVA_INVINCIBILITY_TIME 0.8f        // Invincibility after lava damage

// ==================== PLAYER COMBAT ====================
#define PLAYER_INVINCIBILITY_TIME 0.5f      // Seconds of invincibility after being hit
#define PLAYER_KNOCKBACK_FORCE 0.5f         // How far player gets knocked back
#define PLAYER_KNOCKBACK_DURATION 0.3f      // How long knockback lasts
#define PLAYER_SHIELD_MAX 100.0f            // Max shield health

// ==================== NEAR-DEATH EFFECT ====================
#define NEARDEATH_THRESHOLD 0.5f            // Start showing at 50% health
#define NEARDEATH_HEARTBEAT_THRESHOLD 0.25f // Heartbeat starts at 25%
#define NEARDEATH_CRITICAL_THRESHOLD 0.10f  // Red border at 10%

// ==================== LIGHTING ====================
#define MAX_POINT_LIGHTS 4
#define FLASHLIGHT_RANGE 20.0f
#define FLASHLIGHT_ANGLE 30.0f

// ==================== PARTICLES ====================
#define MAX_PARTICLES 100
#define MAX_MUZZLE_FLASHES 10

// ==================== PLATFORMS & OBSTACLES ====================
#define MAX_PLATFORMS 80
#define MAX_CRATES 80
#define MAX_DOORS 10

// ==================== SPAWN SETTINGS ====================
#define LEVEL2_ENEMY_SPAWN_DISTANCE 15.0f   // How far from player spawn enemies appear

// ==================== GAME STATES ====================
enum GameState {
    STATE_MAIN_MENU,
    STATE_INSTRUCTIONS,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_LEVEL_COMPLETE,
    STATE_GAME_OVER,
    STATE_WIN
};

// ==================== CAMERA MODES ====================
enum CameraMode {
    CAMERA_FIRST_PERSON,
    CAMERA_THIRD_PERSON
};

// ==================== LEVEL IDS ====================
enum LevelID {
    LEVEL_MENU = 0,
    LEVEL_1_FACILITY = 1,
    LEVEL_2_HELL_ARENA = 2
};

// ==================== COLORS (Low-poly style) ====================
namespace Colors {
    // Primary colors
    const float RED[3] = {0.8f, 0.2f, 0.2f};
    const float GREEN[3] = {0.2f, 0.8f, 0.2f};
    const float BLUE[3] = {0.2f, 0.4f, 0.8f};
    
    // Environment colors - Level 1 (Facility)
    const float FACILITY_FLOOR[3] = {0.15f, 0.15f, 0.18f};
    const float FACILITY_WALL[3] = {0.25f, 0.25f, 0.28f};
    const float FACILITY_CEILING[3] = {0.12f, 0.12f, 0.15f};
    const float FACILITY_METAL[3] = {0.4f, 0.42f, 0.45f};
    const float FACILITY_GLOW[3] = {0.0f, 0.6f, 0.9f};
    
    // Environment colors - Level 2 (Hell Arena)
    const float HELL_FLOOR[3] = {0.3f, 0.1f, 0.05f};
    const float HELL_ROCK[3] = {0.35f, 0.15f, 0.1f};
    const float HELL_LAVA[3] = {1.0f, 0.3f, 0.0f};
    const float HELL_SKY[3] = {0.6f, 0.15f, 0.05f};
    
    // UI Colors
    const float UI_HEALTH[3] = {0.8f, 0.2f, 0.2f};
    const float UI_AMMO[3] = {0.9f, 0.7f, 0.2f};
    const float UI_TEXT[3] = {1.0f, 1.0f, 1.0f};
    const float UI_HIGHLIGHT[3] = {0.0f, 0.8f, 1.0f};
    
    // Enemy colors
    const float ENEMY_ZOMBIE[3] = {0.4f, 0.5f, 0.35f};
    const float ENEMY_DEMON[3] = {0.6f, 0.15f, 0.1f};
    const float ENEMY_BOSS[3] = {0.5f, 0.0f, 0.0f};
}

#endif // GAME_CONFIG_H
