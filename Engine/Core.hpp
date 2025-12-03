/**
 * DOOMERS - A DOOM-style Sci-Fi Shooter
 * Core Engine Header
 * 
 * This is the main include file for the engine.
 * OpenGL 2.1 compatible (using fixed-function pipeline with some extensions)
 */

#pragma once

// Standard library includes
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>

// OpenGL includes (legacy/fixed-function compatible)
#include <glut.h>

// Engine version
#define DOOMERS_VERSION_MAJOR 1
#define DOOMERS_VERSION_MINOR 0
#define DOOMERS_VERSION_PATCH 0

// Debug macros
#ifdef _DEBUG
    #define DOOMERS_LOG(msg) std::cout << "[DOOMERS] " << msg << std::endl
    #define DOOMERS_ERROR(msg) std::cerr << "[DOOMERS ERROR] " << msg << std::endl
    #define DOOMERS_WARN(msg) std::cerr << "[DOOMERS WARN] " << msg << std::endl
#else
    #define DOOMERS_LOG(msg)
    #define DOOMERS_ERROR(msg)
    #define DOOMERS_WARN(msg)
#endif

// Always print errors
#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
#define LOG_WARN(msg) std::cerr << "[WARN] " << msg << std::endl

// Game constants
namespace GameConstants {
    // Window settings
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr const char* WINDOW_TITLE = "DOOMERS - Escape the Facility";
    
    // Physics constants
    constexpr float GRAVITY = -20.0f;
    constexpr float GROUND_LEVEL = 0.0f;
    
    // Player constants
    constexpr float PLAYER_HEIGHT = 1.8f;
    constexpr float PLAYER_EYE_HEIGHT = 1.65f;
    constexpr float PLAYER_RADIUS = 0.4f;
    constexpr float PLAYER_WALK_SPEED = 5.0f;
    constexpr float PLAYER_RUN_SPEED = 8.5f;
    constexpr float PLAYER_JUMP_VELOCITY = 8.0f;
    constexpr int PLAYER_MAX_HEALTH = 100;
    constexpr int PLAYER_START_AMMO = 30;
    
    // Camera constants
    constexpr float CAMERA_SENSITIVITY = 0.15f;
    constexpr float CAMERA_PITCH_MIN = -85.0f;
    constexpr float CAMERA_PITCH_MAX = 85.0f;
    constexpr float TPS_CAMERA_DISTANCE = 4.0f;
    constexpr float TPS_CAMERA_HEIGHT = 2.0f;
    
    // Combat constants
    constexpr float WEAPON_RANGE = 100.0f;
    constexpr int WEAPON_DAMAGE = 25;
    constexpr float WEAPON_FIRE_RATE = 0.15f; // seconds between shots
    
    // Enemy constants
    constexpr int ZOMBIE_HEALTH = 100;
    constexpr float ZOMBIE_SPEED = 2.5f;
    constexpr float ZOMBIE_ATTACK_RANGE = 2.0f;
    constexpr int ZOMBIE_DAMAGE = 15;
    constexpr float ZOMBIE_ATTACK_COOLDOWN = 1.5f;
    
    // Pickup values
    constexpr int HEALTH_PACK_VALUE = 25;
    constexpr int AMMO_PACK_VALUE = 15;
    
    // Score values
    constexpr int SCORE_ZOMBIE_KILL = 100;
    constexpr int SCORE_HEALTH_PICKUP = 10;
    constexpr int SCORE_AMMO_PICKUP = 5;
}

// Forward declarations
namespace Doomers {
    class Engine;
    class Renderer;
    class ResourceManager;
    class InputManager;
    class AudioManager;
    class Camera;
    class Entity;
    class Player;
    class Enemy;
    class Weapon;
    class Level;
    class HUD;
    class PhysicsWorld;
}

// Utility functions
namespace Doomers {
namespace Utils {
    inline float degToRad(float degrees) {
        return degrees * 3.14159265358979f / 180.0f;
    }
    
    inline float radToDeg(float radians) {
        return radians * 180.0f / 3.14159265358979f;
    }
    
    inline float clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    
    inline float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }
    
    inline float smoothstep(float edge0, float edge1, float x) {
        float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }
    
    // Random number generation
    inline float randomFloat(float min, float max) {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }
    
    inline int randomInt(int min, int max) {
        return min + rand() % (max - min + 1);
    }
}
}
