// ============================================================================
// DOOMERS - Sound.h
// Sound system using Windows PlaySound and MCI for reliable audio
// ============================================================================
#ifndef SOUND_H
#define SOUND_H

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "winmm.lib")

class SoundSystem {
public:
    bool musicEnabled;
    bool sfxEnabled;
    bool musicPlaying;
    char currentMusicPath[512];
    int sfxChannel;
    
    SoundSystem() {
        musicEnabled = true;
        sfxEnabled = true;
        musicPlaying = false;
        currentMusicPath[0] = '\0';
        sfxChannel = 0;
    }
    
    // Get full path from relative path
    void getFullPath(const char* relativePath, char* fullPath, int maxLen) {
        char moduleDir[MAX_PATH];
        GetModuleFileNameA(NULL, moduleDir, MAX_PATH);
        
        // Find last backslash and truncate
        char* lastSlash = strrchr(moduleDir, '\\');
        if (lastSlash) *lastSlash = '\0';
        
        // Go up one directory (from Debug folder)
        lastSlash = strrchr(moduleDir, '\\');
        if (lastSlash) *lastSlash = '\0';
        
        snprintf(fullPath, maxLen, "%s\\%s", moduleDir, relativePath);
        
        // Convert forward slashes to backslashes
        for (int i = 0; fullPath[i]; i++) {
            if (fullPath[i] == '/') fullPath[i] = '\\';
        }
    }
    
    void playMusic(const char* filename) {
        if (!musicEnabled) return;
        
        char fullPath[512];
        getFullPath(filename, fullPath, 512);
        
        // Stop any existing music
        stopMusic();
        
        char command[1024];
        
        // Open the file
        snprintf(command, sizeof(command), "open \"%s\" type mpegvideo alias bgmusic", fullPath);
        MCIERROR err = mciSendStringA(command, NULL, 0, NULL);
        
        if (err == 0) {
            // Play with repeat
            mciSendStringA("play bgmusic repeat", NULL, 0, NULL);
            musicPlaying = true;
            strncpy(currentMusicPath, fullPath, 511);
        }
    }
    
    void stopMusic() {
        mciSendStringA("stop bgmusic", NULL, 0, NULL);
        mciSendStringA("close bgmusic", NULL, 0, NULL);
        musicPlaying = false;
    }
    
    void pauseMusic() {
        mciSendStringA("pause bgmusic", NULL, 0, NULL);
    }
    
    void resumeMusic() {
        if (musicPlaying) {
            mciSendStringA("resume bgmusic", NULL, 0, NULL);
        }
    }
    
    // Play sound effect using PlaySound - most reliable method
    void playSound(const char* filename) {
        if (!sfxEnabled) return;
        
        char fullPath[512];
        getFullPath(filename, fullPath, 512);
        
        // SND_ASYNC: play asynchronously
        // SND_NODEFAULT: don't play default sound if file not found
        PlaySoundA(fullPath, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    }
    
    // Play sound with overlap support using MCI (for rapid fire sounds)
    void playSoundOverlap(const char* filename) {
        if (!sfxEnabled) return;
        
        char fullPath[512];
        getFullPath(filename, fullPath, 512);
        
        char alias[32];
        snprintf(alias, sizeof(alias), "sfx%d", sfxChannel);
        sfxChannel = (sfxChannel + 1) % 8;
        
        char command[1024];
        
        // Close previous sound on this channel
        snprintf(command, sizeof(command), "close %s", alias);
        mciSendStringA(command, NULL, 0, NULL);
        
        // Open new sound
        snprintf(command, sizeof(command), "open \"%s\" type waveaudio alias %s", fullPath, alias);
        if (mciSendStringA(command, NULL, 0, NULL) == 0) {
            snprintf(command, sizeof(command), "play %s", alias);
            mciSendStringA(command, NULL, 0, NULL);
        }
    }
    
    void setMusicVolume(int volume) {
        char command[64];
        snprintf(command, sizeof(command), "setaudio bgmusic volume to %d", volume * 10);
        mciSendStringA(command, NULL, 0, NULL);
    }
    
    void toggleMusic() {
        musicEnabled = !musicEnabled;
        if (!musicEnabled) {
            stopMusic();
        }
    }
    
    void toggleSFX() {
        sfxEnabled = !sfxEnabled;
    }
    
    void cleanup() {
        mciSendStringA("close all", NULL, 0, NULL);
    }
};

// Sound file paths - mapped to actual audio files in res/Audio
namespace Sounds {
    // Music
    const char* MUSIC_MENU = "res/Audio/MainMenu.mp3";
    const char* MUSIC_INGAME = "res/Audio/InGame.mp3";
    
    // Sound effects
    const char* SFX_AR_FIRE = "res/Audio/AR_Fired.wav";
    const char* SFX_RELOAD = "res/Audio/Reloading.wav";
    const char* SFX_ENEMY_HIT = "res/Audio/EnemyHit.wav";
    const char* SFX_ENEMY_DEATH = "res/Audio/EnemyDead.wav";
    const char* SFX_ENEMY_DEATH_2 = "res/Audio/EnemyDead2.wav";
    const char* SFX_PLAYER_HURT = "res/Audio/Hurt.wav";
    const char* SFX_PLAYER_DEAD = "res/Audio/Dead.wav";
    const char* SFX_BUTTON_CLICK = "res/Audio/ButtonClicked.wav";
    const char* SFX_BUTTON_HOVER = "res/Audio/ButtonHovered.wav";
    const char* SFX_FOOTSTEPS_WALK = "res/Audio/FootstepsWalk.wav";
    const char* SFX_FOOTSTEPS_RUN = "res/Audio/FootstepsRun.wav";
    const char* SFX_FLASHLIGHT_ON = "res/Audio/FlashLightOn.wav";
    const char* SFX_FLASHLIGHT_OFF = "res/Audio/FlashLightOff.wav";
    const char* SFX_SHOCKWAVE = "res/Audio/Shockwave.wav";
    const char* SFX_THUNDER = "res/Audio/Thunder.wav";
}

#endif // SOUND_H
