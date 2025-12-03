/**
 * DOOMERS - Game Assets Manager
 * 
 * Central management of all FBX animated models and textures.
 * Handles loading, caching, and providing access to game assets.
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include "../Engine/AssimpLoader.hpp"
#include "../Engine/ResourceManager.hpp"
#include <map>
#include <string>
#include <memory>

namespace Doomers {

// ============================================================================
// Animation Names Constants (for easy reference)
// ============================================================================
namespace PlayerAnimations {
    // Idle animations
    constexpr const char* IDLE = "idle.fbx";
    constexpr const char* IDLE_AIMING = "idle aiming.fbx";
    constexpr const char* IDLE_CROUCHING = "idle crouching.fbx";
    constexpr const char* IDLE_CROUCHING_AIMING = "idle crouching aiming.fbx";
    
    // Walk animations
    constexpr const char* WALK_FORWARD = "walk forward.fbx";
    constexpr const char* WALK_BACKWARD = "walk backward.fbx";
    constexpr const char* WALK_LEFT = "walk left.fbx";
    constexpr const char* WALK_RIGHT = "walk right.fbx";
    constexpr const char* WALK_FORWARD_LEFT = "walk forward left.fbx";
    constexpr const char* WALK_FORWARD_RIGHT = "walk forward right.fbx";
    constexpr const char* WALK_BACKWARD_LEFT = "walk backward left.fbx";
    constexpr const char* WALK_BACKWARD_RIGHT = "walk backward right.fbx";
    
    // Crouch walk
    constexpr const char* WALK_CROUCH_FORWARD = "walk crouching forward.fbx";
    constexpr const char* WALK_CROUCH_BACKWARD = "walk crouching backward.fbx";
    constexpr const char* WALK_CROUCH_LEFT = "walk crouching left.fbx";
    constexpr const char* WALK_CROUCH_RIGHT = "walk crouching right.fbx";
    
    // Run animations
    constexpr const char* RUN_FORWARD = "run forward.fbx";
    constexpr const char* RUN_BACKWARD = "run backward.fbx";
    constexpr const char* RUN_LEFT = "run left.fbx";
    constexpr const char* RUN_RIGHT = "run right.fbx";
    constexpr const char* RUN_FORWARD_LEFT = "run forward left.fbx";
    constexpr const char* RUN_FORWARD_RIGHT = "run forward right.fbx";
    
    // Sprint animations
    constexpr const char* SPRINT_FORWARD = "sprint forward.fbx";
    constexpr const char* SPRINT_BACKWARD = "sprint backward.fbx";
    constexpr const char* SPRINT_LEFT = "sprint left.fbx";
    constexpr const char* SPRINT_RIGHT = "sprint right.fbx";
    
    // Jump animations
    constexpr const char* JUMP_UP = "jump up.fbx";
    constexpr const char* JUMP_LOOP = "jump loop.fbx";
    constexpr const char* JUMP_DOWN = "jump down.fbx";
    
    // Turn animations
    constexpr const char* TURN_LEFT = "turn 90 left.fbx";
    constexpr const char* TURN_RIGHT = "turn 90 right.fbx";
    constexpr const char* CROUCH_TURN_LEFT = "crouching turn 90 left.fbx";
    constexpr const char* CROUCH_TURN_RIGHT = "crouching turn 90 right.fbx";
    
    // Death animations
    constexpr const char* DEATH_FRONT = "death from the front.fbx";
    constexpr const char* DEATH_BACK = "death from the back.fbx";
    constexpr const char* DEATH_RIGHT = "death from right.fbx";
    constexpr const char* DEATH_HEADSHOT_FRONT = "death from front headshot.fbx";
    constexpr const char* DEATH_HEADSHOT_BACK = "death from back headshot.fbx";
    constexpr const char* DEATH_CROUCH_HEADSHOT = "death crouching headshot front.fbx";
}

namespace ZombieAnimations {
    constexpr const char* IDLE = "zombie idle.fbx";
    constexpr const char* WALK = "zombie walk.fbx";
    constexpr const char* RUN = "zombie run.fbx";
    constexpr const char* ATTACK = "zombie attack.fbx";
    constexpr const char* BITING = "zombie biting.fbx";
    constexpr const char* BITING_2 = "zombie biting (2).fbx";
    constexpr const char* NECK_BITE = "zombie neck bite.fbx";
    constexpr const char* DEATH = "zombie death.fbx";
    constexpr const char* DYING = "zombie dying.fbx";
    constexpr const char* SCREAM = "zombie scream.fbx";
    constexpr const char* CRAWL = "zombie crawl.fbx";
    constexpr const char* RUNNING_CRAWL = "running crawl.fbx";
}

namespace DevilAnimations {
    constexpr const char* BASE = "devil.fbx";
    constexpr const char* WALK = "Mutant Walking.fbx";
    constexpr const char* DROP_KICK = "Drop Kick.fbx";
    constexpr const char* MELEE_KICK = "Standing Melee Kick.fbx";
}

// ============================================================================
// Asset Paths
// ============================================================================
namespace AssetPaths {
    // Character models base
    constexpr const char* PLAYER_BASE = "assets/pro-rifle-pack/";
    constexpr const char* ZOMBIE_BASE = "assets/scary-zombie-pack/";
    constexpr const char* DEVIL_BASE = "assets/devil/";
    
    // Weapon models
    constexpr const char* LASER_RIFLE = "assets/sci-fi-laser-rifle/source/toSketchfab.fbx";
    constexpr const char* LASER_RIFLE_TEXTURES = "assets/sci-fi-laser-rifle/textures/";
    
    // Environment
    constexpr const char* SCI_FI_INTERIOR = "assets/sci-fi-interior-pack-lowpoly/source/_CombinedAssets_DisplayPack.fbx";
    constexpr const char* LAVA_ZONE = "assets/free-lava-zone-environment/source/TerrainGEN_3Model.fbx";
    
    // Props
    constexpr const char* HEALTH_PACK = "assets/health-pack/";
    constexpr const char* AMMO_BOX = "assets/sci-fi-ammo-box/";
    constexpr const char* CRATE = "assets/gart130-crate/";
}

// ============================================================================
// Character Model with Multiple Animations
// ============================================================================
class CharacterModel {
public:
    // Base model (skeleton/mesh)
    AnimatedModel* baseModel = nullptr;
    
    // Animation clips loaded from separate FBX files
    std::map<std::string, AnimationClip> animationLibrary;
    
    // Current animation state
    std::string currentAnimation;
    std::string nextAnimation;
    float blendTime = 0.0f;
    float blendDuration = 0.2f;
    bool isBlending = false;
    
    ~CharacterModel() {
        if (baseModel) delete baseModel;
    }
    
    bool isLoaded() const { return baseModel != nullptr; }
    
    void setAnimation(const std::string& name, float blendTime = 0.2f) {
        if (currentAnimation == name) return;
        
        auto it = animationLibrary.find(name);
        if (it != animationLibrary.end()) {
            if (blendTime > 0 && !currentAnimation.empty()) {
                // Start blending
                nextAnimation = name;
                this->blendDuration = blendTime;
                this->blendTime = 0;
                isBlending = true;
            } else {
                // Instant switch
                currentAnimation = name;
                if (baseModel) {
                    // Copy animation data to base model
                    baseModel->animations.clear();
                    baseModel->animations.push_back(it->second);
                    baseModel->setAnimation(0);
                }
            }
        }
    }
    
    void update(float dt) {
        if (!baseModel) return;
        
        if (isBlending) {
            blendTime += dt;
            if (blendTime >= blendDuration) {
                // Finish blend
                currentAnimation = nextAnimation;
                auto it = animationLibrary.find(currentAnimation);
                if (it != animationLibrary.end()) {
                    baseModel->animations.clear();
                    baseModel->animations.push_back(it->second);
                    baseModel->setAnimation(0);
                }
                isBlending = false;
            }
        }
        
        baseModel->update(dt);
    }
    
    void draw() {
        if (baseModel) {
            baseModel->draw();
        }
    }
    
    void drawStatic() {
        if (baseModel) {
            baseModel->drawStatic();
        }
    }
};

// ============================================================================
// Game Assets Manager - Singleton
// ============================================================================
class GameAssets {
public:
    static GameAssets& instance() {
        static GameAssets inst;
        return inst;
    }
    
    // ========================================================================
    // Initialization
    // ========================================================================
    bool initialize() {
        LOG_INFO("GameAssets: Initializing...");
        
        initialized = false;
        
        // We'll load assets lazily or on-demand
        // For now, just mark as ready
        initialized = true;
        
        LOG_INFO("GameAssets: Initialization complete");
        return true;
    }
    
    void shutdown() {
        // Clean up all loaded models
        playerModel.reset();
        zombieModel.reset();
        devilModel.reset();
        
        if (weaponModel) {
            delete weaponModel;
            weaponModel = nullptr;
        }
        if (environmentLevel1) {
            delete environmentLevel1;
            environmentLevel1 = nullptr;
        }
        if (environmentLevel2) {
            delete environmentLevel2;
            environmentLevel2 = nullptr;
        }
        
        initialized = false;
    }
    
    // ========================================================================
    // Player Model Loading
    // ========================================================================
    CharacterModel* loadPlayerModel() {
        if (playerModel && playerModel->isLoaded()) {
            return playerModel.get();
        }
        
        LOG_INFO("Loading player model and animations...");
        
        playerModel = std::make_unique<CharacterModel>();
        
        // Load base model (X Bot with skeleton)
        std::string basePath = AssetPaths::PLAYER_BASE;
        playerModel->baseModel = AssimpLoader::loadModel(basePath + "X Bot.fbx");
        
        if (!playerModel->baseModel) {
            LOG_ERROR("Failed to load player base model");
            return nullptr;
        }
        
        // Load animations from separate FBX files
        loadPlayerAnimation(PlayerAnimations::IDLE);
        loadPlayerAnimation(PlayerAnimations::IDLE_AIMING);
        loadPlayerAnimation(PlayerAnimations::WALK_FORWARD);
        loadPlayerAnimation(PlayerAnimations::WALK_BACKWARD);
        loadPlayerAnimation(PlayerAnimations::WALK_LEFT);
        loadPlayerAnimation(PlayerAnimations::WALK_RIGHT);
        loadPlayerAnimation(PlayerAnimations::RUN_FORWARD);
        loadPlayerAnimation(PlayerAnimations::RUN_BACKWARD);
        loadPlayerAnimation(PlayerAnimations::SPRINT_FORWARD);
        loadPlayerAnimation(PlayerAnimations::JUMP_UP);
        loadPlayerAnimation(PlayerAnimations::JUMP_LOOP);
        loadPlayerAnimation(PlayerAnimations::JUMP_DOWN);
        loadPlayerAnimation(PlayerAnimations::DEATH_FRONT);
        loadPlayerAnimation(PlayerAnimations::DEATH_BACK);
        
        // Set default animation
        playerModel->setAnimation(PlayerAnimations::IDLE, 0);
        
        LOG_INFO("Player model loaded with " << playerModel->animationLibrary.size() << " animations");
        
        return playerModel.get();
    }
    
    // ========================================================================
    // Zombie Model Loading
    // ========================================================================
    CharacterModel* loadZombieModel() {
        if (zombieModel && zombieModel->isLoaded()) {
            return zombieModel.get();
        }
        
        LOG_INFO("Loading zombie model and animations...");
        
        zombieModel = std::make_unique<CharacterModel>();
        
        // Load base model
        std::string basePath = AssetPaths::ZOMBIE_BASE;
        zombieModel->baseModel = AssimpLoader::loadModel(basePath + "X Bot.fbx");
        
        if (!zombieModel->baseModel) {
            LOG_ERROR("Failed to load zombie base model");
            return nullptr;
        }
        
        // Load zombie animations
        loadZombieAnimation(ZombieAnimations::IDLE);
        loadZombieAnimation(ZombieAnimations::WALK);
        loadZombieAnimation(ZombieAnimations::RUN);
        loadZombieAnimation(ZombieAnimations::ATTACK);
        loadZombieAnimation(ZombieAnimations::BITING);
        loadZombieAnimation(ZombieAnimations::DEATH);
        loadZombieAnimation(ZombieAnimations::DYING);
        loadZombieAnimation(ZombieAnimations::SCREAM);
        loadZombieAnimation(ZombieAnimations::CRAWL);
        
        // Set default animation
        zombieModel->setAnimation(ZombieAnimations::IDLE, 0);
        
        LOG_INFO("Zombie model loaded with " << zombieModel->animationLibrary.size() << " animations");
        
        return zombieModel.get();
    }
    
    // ========================================================================
    // Devil/Boss Model Loading
    // ========================================================================
    CharacterModel* loadDevilModel() {
        if (devilModel && devilModel->isLoaded()) {
            return devilModel.get();
        }
        
        LOG_INFO("Loading devil model and animations...");
        
        devilModel = std::make_unique<CharacterModel>();
        
        // Load base model
        std::string basePath = AssetPaths::DEVIL_BASE;
        devilModel->baseModel = AssimpLoader::loadModel(basePath + "devil.fbx");
        
        if (!devilModel->baseModel) {
            LOG_ERROR("Failed to load devil base model");
            return nullptr;
        }
        
        // Load devil texture
        devilTexture = ResourceManager::instance().loadTexture(basePath + "devil.png");
        if (devilTexture > 0) {
            devilModel->baseModel->textureId = devilTexture;
        }
        
        // Load devil animations
        loadDevilAnimation(DevilAnimations::WALK);
        loadDevilAnimation(DevilAnimations::DROP_KICK);
        loadDevilAnimation(DevilAnimations::MELEE_KICK);
        
        LOG_INFO("Devil model loaded with " << devilModel->animationLibrary.size() << " animations");
        
        return devilModel.get();
    }
    
    // ========================================================================
    // Weapon Model Loading
    // ========================================================================
    AnimatedModel* loadWeaponModel() {
        if (weaponModel) return weaponModel;
        
        LOG_INFO("Loading weapon model...");
        
        weaponModel = AssimpLoader::loadModel(AssetPaths::LASER_RIFLE);
        
        if (!weaponModel) {
            LOG_ERROR("Failed to load weapon model");
            return nullptr;
        }
        
        // Load weapon textures
        std::string texPath = AssetPaths::LASER_RIFLE_TEXTURES;
        unsigned int weaponTex = ResourceManager::instance().loadTexture(texPath + "Base_Color.png");
        if (weaponTex > 0) {
            weaponModel->textureId = weaponTex;
        }
        
        LOG_INFO("Weapon model loaded");
        return weaponModel;
    }
    
    // ========================================================================
    // Environment Loading
    // ========================================================================
    AnimatedModel* loadLevel1Environment() {
        if (environmentLevel1) return environmentLevel1;
        
        LOG_INFO("Loading Level 1 environment (Sci-Fi Interior)...");
        
        environmentLevel1 = AssimpLoader::loadModel(AssetPaths::SCI_FI_INTERIOR);
        
        if (!environmentLevel1) {
            LOG_ERROR("Failed to load Level 1 environment");
            return nullptr;
        }
        
        LOG_INFO("Level 1 environment loaded");
        return environmentLevel1;
    }
    
    AnimatedModel* loadLevel2Environment() {
        if (environmentLevel2) return environmentLevel2;
        
        LOG_INFO("Loading Level 2 environment (Lava Zone)...");
        
        environmentLevel2 = AssimpLoader::loadModel(AssetPaths::LAVA_ZONE);
        
        if (!environmentLevel2) {
            LOG_ERROR("Failed to load Level 2 environment");
            return nullptr;
        }
        
        LOG_INFO("Level 2 environment loaded");
        return environmentLevel2;
    }
    
    // ========================================================================
    // Accessors
    // ========================================================================
    CharacterModel* getPlayerModel() { return playerModel.get(); }
    CharacterModel* getZombieModel() { return zombieModel.get(); }
    CharacterModel* getDevilModel() { return devilModel.get(); }
    AnimatedModel* getWeaponModel() { return weaponModel; }
    AnimatedModel* getLevel1Environment() { return environmentLevel1; }
    AnimatedModel* getLevel2Environment() { return environmentLevel2; }
    
    bool isInitialized() const { return initialized; }
    
private:
    GameAssets() = default;
    ~GameAssets() { shutdown(); }
    
    // No copying
    GameAssets(const GameAssets&) = delete;
    GameAssets& operator=(const GameAssets&) = delete;
    
    // Helper: Load animation from FBX file and add to library
    void loadPlayerAnimation(const char* animName) {
        std::string fullPath = std::string(AssetPaths::PLAYER_BASE) + animName;
        AnimatedModel* animModel = AssimpLoader::loadModel(fullPath);
        if (animModel && !animModel->animations.empty()) {
            playerModel->animationLibrary[animName] = animModel->animations[0];
            delete animModel;
        }
    }
    
    void loadZombieAnimation(const char* animName) {
        std::string fullPath = std::string(AssetPaths::ZOMBIE_BASE) + animName;
        AnimatedModel* animModel = AssimpLoader::loadModel(fullPath);
        if (animModel && !animModel->animations.empty()) {
            zombieModel->animationLibrary[animName] = animModel->animations[0];
            delete animModel;
        }
    }
    
    void loadDevilAnimation(const char* animName) {
        std::string fullPath = std::string(AssetPaths::DEVIL_BASE) + animName;
        AnimatedModel* animModel = AssimpLoader::loadModel(fullPath);
        if (animModel && !animModel->animations.empty()) {
            devilModel->animationLibrary[animName] = animModel->animations[0];
            delete animModel;
        }
    }
    
    // State
    bool initialized = false;
    
    // Character models
    std::unique_ptr<CharacterModel> playerModel;
    std::unique_ptr<CharacterModel> zombieModel;
    std::unique_ptr<CharacterModel> devilModel;
    
    // Static models
    AnimatedModel* weaponModel = nullptr;
    AnimatedModel* environmentLevel1 = nullptr;
    AnimatedModel* environmentLevel2 = nullptr;
    
    // Textures
    unsigned int devilTexture = 0;
};

// ============================================================================
// Convenience accessor
// ============================================================================
inline GameAssets& Assets() {
    return GameAssets::instance();
}

} // namespace Doomers
