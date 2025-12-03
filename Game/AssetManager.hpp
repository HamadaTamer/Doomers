/**
 * DOOMERS - Asset Manager
 * Loads and manages all FBX models
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/AssimpLoader.hpp"
#include <string>
#include <map>

namespace Doomers {

class AssetManager {
public:
    // Loaded models
    AnimatedModel* zombieIdle = nullptr;
    AnimatedModel* zombieWalk = nullptr;
    AnimatedModel* zombieAttack = nullptr;
    AnimatedModel* zombieDeath = nullptr;
    
    AnimatedModel* devilModel = nullptr;
    AnimatedModel* devilWalk = nullptr;
    AnimatedModel* devilAttack = nullptr;
    
    AnimatedModel* weaponModel = nullptr;
    AnimatedModel* levelModel = nullptr;
    AnimatedModel* level2Model = nullptr;
    
    bool loaded = false;
    std::string basePath;
    
    void setBasePath(const std::string& path) {
        basePath = path;
        // Ensure trailing slash
        if (!basePath.empty() && basePath.back() != '/' && basePath.back() != '\\') {
            basePath += "/";
        }
    }
    
    std::string getPath(const std::string& relativePath) {
        return basePath + relativePath;
    }
    
    void loadAll() {
        if (loaded) return;
        
        std::cout << "Loading game assets..." << std::endl;
        
        // Zombie - try X Bot as base mesh, or use zombie files
        std::cout << "  Loading zombie models..." << std::endl;
        // Try the base mesh first
        zombieIdle = AssimpLoader::loadModel(getPath("assets/scary-zombie-pack/X Bot.fbx"));
        if (!zombieIdle) {
            zombieIdle = AssimpLoader::loadModel(getPath("assets/scary-zombie-pack/zombie idle.fbx"));
        }
        zombieWalk = AssimpLoader::loadModel(getPath("assets/scary-zombie-pack/zombie walk.fbx"));
        zombieAttack = AssimpLoader::loadModel(getPath("assets/scary-zombie-pack/zombie attack.fbx"));
        zombieDeath = AssimpLoader::loadModel(getPath("assets/scary-zombie-pack/zombie death.fbx"));
        
        if (zombieIdle) std::cout << "    [OK] Zombie mesh" << std::endl;
        else std::cout << "    [FAIL] Zombie mesh" << std::endl;
        
        if (zombieWalk) std::cout << "    [OK] Zombie walk" << std::endl;
        if (zombieAttack) std::cout << "    [OK] Zombie attack" << std::endl;
        if (zombieDeath) std::cout << "    [OK] Zombie death" << std::endl;
        
        // Devil
        std::cout << "  Loading devil models..." << std::endl;
        devilModel = AssimpLoader::loadModel(getPath("assets/devil/devil.fbx"));
        devilWalk = AssimpLoader::loadModel(getPath("assets/devil/Mutant Walking.fbx"));
        devilAttack = AssimpLoader::loadModel(getPath("assets/devil/Standing Melee Kick.fbx"));
        
        if (devilModel) std::cout << "    [OK] Devil model" << std::endl;
        else std::cout << "    [FAIL] Devil model" << std::endl;
        
        // Weapon
        std::cout << "  Loading weapon..." << std::endl;
        weaponModel = AssimpLoader::loadModel(getPath("assets/sci-fi-laser-rifle/source/toSketchfab.fbx"));
        if (weaponModel) std::cout << "    [OK] Laser rifle" << std::endl;
        else std::cout << "    [FAIL] Laser rifle" << std::endl;
        
        // Levels
        std::cout << "  Loading level environments..." << std::endl;
        levelModel = AssimpLoader::loadModel(getPath("assets/sci-fi-interior-pack-lowpoly/source/_CombinedAssets_DisplayPack.fbx"));
        level2Model = AssimpLoader::loadModel(getPath("assets/free-lava-zone-environment/source/TerrainGEN_3Model.fbx"));
        
        if (levelModel) std::cout << "    [OK] Sci-Fi Interior" << std::endl;
        else std::cout << "    [FAIL] Sci-Fi Interior" << std::endl;
        
        if (level2Model) std::cout << "    [OK] Lava Zone" << std::endl;
        else std::cout << "    [FAIL] Lava Zone" << std::endl;
        
        loaded = true;
        std::cout << "Asset loading complete!" << std::endl;
    }
    
    static AssetManager& instance() {
        static AssetManager inst;
        return inst;
    }
    
private:
    AssetManager() {}
};

} // namespace Doomers
