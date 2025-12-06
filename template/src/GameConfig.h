// ============================================================================
// DOOMERS - GameConfig.h
// Core configuration and constants for the game
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
#define GRAVITY 0.018f

// ==================== PLAYER SETTINGS ====================
#define PLAYER_HEIGHT 1.8f
#define PLAYER_SPEED 0.15f
#define PLAYER_SPRINT_MULTIPLIER 1.8f
#define PLAYER_JUMP_FORCE 0.25f
#define PLAYER_MAX_HEALTH 100
#define PLAYER_COLLISION_RADIUS 0.5f

// ==================== ENEMY SETTINGS ====================
#define MAX_ENEMIES 20
#define ENEMY_SPEED 0.08f
#define ENEMY_DAMAGE 10
#define ENEMY_ATTACK_RANGE 2.5f
#define ENEMY_DETECT_RANGE 20.0f

// ==================== WEAPON SETTINGS ====================
#define WEAPON_DAMAGE 25
#define WEAPON_RANGE 50.0f
#define WEAPON_FIRE_RATE 0.2f
#define MAX_AMMO 100
#define AMMO_PICKUP_AMOUNT 20

// ==================== COLLECTIBLES ====================
#define MAX_HEALTH_PACKS 10
#define MAX_AMMO_BOXES 15
#define MAX_KEYCARDS 5
#define HEALTH_PACK_HEAL 25

// ==================== LIGHTING ====================
#define MAX_POINT_LIGHTS 4
#define FLASHLIGHT_RANGE 20.0f
#define FLASHLIGHT_ANGLE 30.0f

// ==================== PARTICLES ====================
#define MAX_PARTICLES 100
#define MAX_MUZZLE_FLASHES 10

// ==================== PLATFORMS & OBSTACLES ====================
#define MAX_PLATFORMS 20
#define MAX_CRATES 30
#define MAX_DOORS 10

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
