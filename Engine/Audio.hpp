/**
 * DOOMERS - Audio Manager (Stub)
 * 
 * Placeholder audio system for future implementation.
 * Currently just logs audio events - ready for integration
 * with OpenAL, FMOD, or SDL_mixer.
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"

#include <unordered_map>
#include <string>
#include <iostream>

namespace Doomers {

// ============================================================================
// Sound Effect IDs
// ============================================================================
enum class SoundEffect {
    // Weapons
    Pistol_Fire,
    Rifle_Fire,
    Shotgun_Fire,
    Weapon_Reload,
    Weapon_Empty,
    
    // Player
    Player_Footstep,
    Player_Jump,
    Player_Land,
    Player_Hurt,
    Player_Death,
    
    // Enemy
    Enemy_Alert,
    Enemy_Attack,
    Enemy_Hurt,
    Enemy_Death,
    Zombie_Moan,
    Demon_Roar,
    
    // Pickups
    Pickup_Health,
    Pickup_Ammo,
    Pickup_Weapon,
    Pickup_Key,
    
    // Environment
    Door_Open,
    Door_Close,
    Elevator,
    Trigger,
    Explosion,
    
    // UI
    UI_Select,
    UI_Confirm,
    UI_Back,
    
    // Count
    COUNT
};

// ============================================================================
// Music Track IDs
// ============================================================================
enum class MusicTrack {
    Menu,
    Level1_Facility,
    Level2_Arena,
    Boss_Fight,
    Victory,
    GameOver,
    
    COUNT
};

// ============================================================================
// AudioManager - Singleton audio system (stub)
// ============================================================================
class AudioManager {
public:
    static AudioManager& instance() {
        static AudioManager inst;
        return inst;
    }
    
    // ========================================================================
    // Initialization
    // ========================================================================
    bool initialize() {
        std::cout << "[AudioManager] Initialized (stub mode - no audio output)\n";
        
        // In a real implementation, initialize OpenAL/FMOD/SDL_mixer here
        initialized = true;
        
        // Set default volumes
        masterVolume = 1.0f;
        sfxVolume = 0.8f;
        musicVolume = 0.6f;
        
        return true;
    }
    
    void shutdown() {
        if (!initialized) return;
        
        std::cout << "[AudioManager] Shutdown\n";
        initialized = false;
    }
    
    // ========================================================================
    // Sound Effects
    // ========================================================================
    void playSound(SoundEffect sound, float volume = 1.0f) {
        if (!initialized || !sfxEnabled) return;
        
        // Stub: just log the sound
        if (debugMode) {
            std::cout << "[Audio] Play sound: " << getSoundName(sound) 
                      << " (vol: " << volume * sfxVolume * masterVolume << ")\n";
        }
        
        // TODO: Play actual sound effect
    }
    
    void playSound3D(SoundEffect sound, const Math::Vector3& position, float volume = 1.0f) {
        if (!initialized || !sfxEnabled) return;
        
        // Calculate volume based on distance from listener
        float dist = (position - listenerPosition).length();
        float attenuation = 1.0f / (1.0f + dist * 0.1f);
        float finalVolume = volume * attenuation;
        
        if (debugMode) {
            std::cout << "[Audio] Play 3D sound: " << getSoundName(sound)
                      << " at " << position.x << "," << position.y << "," << position.z
                      << " (atten vol: " << finalVolume << ")\n";
        }
        
        // TODO: Play positional audio
    }
    
    // ========================================================================
    // Music
    // ========================================================================
    void playMusic(MusicTrack track, bool loop = true) {
        if (!initialized || !musicEnabled) return;
        
        currentTrack = track;
        
        if (debugMode) {
            std::cout << "[Audio] Play music: " << getMusicName(track) 
                      << (loop ? " (looping)" : " (once)") << "\n";
        }
        
        // TODO: Play actual music
    }
    
    void stopMusic() {
        if (!initialized) return;
        
        if (debugMode) {
            std::cout << "[Audio] Stop music\n";
        }
        
        currentTrack = MusicTrack::COUNT;
    }
    
    void pauseMusic() {
        if (!initialized) return;
        musicPaused = true;
    }
    
    void resumeMusic() {
        if (!initialized) return;
        musicPaused = false;
    }
    
    // ========================================================================
    // Listener (player/camera position)
    // ========================================================================
    void setListenerPosition(const Math::Vector3& pos) {
        listenerPosition = pos;
    }
    
    void setListenerOrientation(const Math::Vector3& forward, const Math::Vector3& up) {
        listenerForward = forward;
        listenerUp = up;
    }
    
    // ========================================================================
    // Volume Control
    // ========================================================================
    void setMasterVolume(float vol) { masterVolume = clamp(vol, 0.0f, 1.0f); }
    void setSFXVolume(float vol) { sfxVolume = clamp(vol, 0.0f, 1.0f); }
    void setMusicVolume(float vol) { musicVolume = clamp(vol, 0.0f, 1.0f); }
    
    float getMasterVolume() const { return masterVolume; }
    float getSFXVolume() const { return sfxVolume; }
    float getMusicVolume() const { return musicVolume; }
    
    // ========================================================================
    // Enable/Disable
    // ========================================================================
    void setSFXEnabled(bool enabled) { sfxEnabled = enabled; }
    void setMusicEnabled(bool enabled) { musicEnabled = enabled; }
    bool isSFXEnabled() const { return sfxEnabled; }
    bool isMusicEnabled() const { return musicEnabled; }
    
    void setDebugMode(bool debug) { debugMode = debug; }
    
private:
    AudioManager() 
        : initialized(false)
        , masterVolume(1.0f)
        , sfxVolume(0.8f)
        , musicVolume(0.6f)
        , sfxEnabled(true)
        , musicEnabled(true)
        , musicPaused(false)
        , currentTrack(MusicTrack::COUNT)
        , debugMode(false)
    {}
    
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    
    // ========================================================================
    // Helper - Get sound names for debug
    // ========================================================================
    const char* getSoundName(SoundEffect sound) {
        switch (sound) {
            case SoundEffect::Pistol_Fire: return "Pistol_Fire";
            case SoundEffect::Rifle_Fire: return "Rifle_Fire";
            case SoundEffect::Shotgun_Fire: return "Shotgun_Fire";
            case SoundEffect::Weapon_Reload: return "Weapon_Reload";
            case SoundEffect::Weapon_Empty: return "Weapon_Empty";
            case SoundEffect::Player_Footstep: return "Player_Footstep";
            case SoundEffect::Player_Jump: return "Player_Jump";
            case SoundEffect::Player_Land: return "Player_Land";
            case SoundEffect::Player_Hurt: return "Player_Hurt";
            case SoundEffect::Player_Death: return "Player_Death";
            case SoundEffect::Enemy_Alert: return "Enemy_Alert";
            case SoundEffect::Enemy_Attack: return "Enemy_Attack";
            case SoundEffect::Enemy_Hurt: return "Enemy_Hurt";
            case SoundEffect::Enemy_Death: return "Enemy_Death";
            case SoundEffect::Zombie_Moan: return "Zombie_Moan";
            case SoundEffect::Demon_Roar: return "Demon_Roar";
            case SoundEffect::Pickup_Health: return "Pickup_Health";
            case SoundEffect::Pickup_Ammo: return "Pickup_Ammo";
            case SoundEffect::Pickup_Weapon: return "Pickup_Weapon";
            case SoundEffect::Pickup_Key: return "Pickup_Key";
            case SoundEffect::Door_Open: return "Door_Open";
            case SoundEffect::Door_Close: return "Door_Close";
            case SoundEffect::Elevator: return "Elevator";
            case SoundEffect::Trigger: return "Trigger";
            case SoundEffect::Explosion: return "Explosion";
            case SoundEffect::UI_Select: return "UI_Select";
            case SoundEffect::UI_Confirm: return "UI_Confirm";
            case SoundEffect::UI_Back: return "UI_Back";
            default: return "Unknown";
        }
    }
    
    const char* getMusicName(MusicTrack track) {
        switch (track) {
            case MusicTrack::Menu: return "Menu";
            case MusicTrack::Level1_Facility: return "Level1_Facility";
            case MusicTrack::Level2_Arena: return "Level2_Arena";
            case MusicTrack::Boss_Fight: return "Boss_Fight";
            case MusicTrack::Victory: return "Victory";
            case MusicTrack::GameOver: return "GameOver";
            default: return "Unknown";
        }
    }
    
    float clamp(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    
    bool initialized;
    
    float masterVolume;
    float sfxVolume;
    float musicVolume;
    
    bool sfxEnabled;
    bool musicEnabled;
    bool musicPaused;
    
    MusicTrack currentTrack;
    
    Math::Vector3 listenerPosition;
    Math::Vector3 listenerForward;
    Math::Vector3 listenerUp;
    
    bool debugMode;
};

// ============================================================================
// Convenience Functions
// ============================================================================
inline void PlaySound(SoundEffect sound, float volume = 1.0f) {
    AudioManager::instance().playSound(sound, volume);
}

inline void PlaySound3D(SoundEffect sound, const Math::Vector3& pos, float volume = 1.0f) {
    AudioManager::instance().playSound3D(sound, pos, volume);
}

inline void PlayMusic(MusicTrack track, bool loop = true) {
    AudioManager::instance().playMusic(track, loop);
}

inline void StopMusic() {
    AudioManager::instance().stopMusic();
}

} // namespace Doomers
