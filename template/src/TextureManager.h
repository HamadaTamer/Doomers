// ============================================================================
// DOOMERS - TextureManager.h
// Texture loading and management using SOIL library
// ============================================================================
#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <windows.h>
#include <glut.h>
#include <stdio.h>
#include <string.h>
#include "../Dependencies/soil/include/SOIL.h"

// Texture IDs for all game textures
enum TextureID {
    // Floor/Ground textures
    TEX_FLOOR_LAB = 0,
    TEX_FLOOR_TILE,
    TEX_FLOOR_TILE2,
    TEX_FLOOR_METAL,
    TEX_LAVA,
    TEX_LAVA_GLOW,
    
    // Wall textures
    TEX_WALL_GREY,
    TEX_WALL_BLUE,
    TEX_WALL_PANEL,
    TEX_WALL_ORANGE_WARNING,
    
    // Object textures
    TEX_CRATE,
    TEX_CRATE_SCIFI,
    TEX_PLATFORM,
    TEX_PLATFORM_LAVA,
    TEX_PARKOUR,
    
    // Environment textures
    TEX_ROCK,
    TEX_PILLAR,
    
    // Character/Enemy textures
    TEX_PLAYER,
    TEX_ENEMY_ZOMBIE,
    TEX_ENEMY_DEMON,
    TEX_ENEMY_BOSS,
    
    // Collectible textures
    TEX_HEALTHPACK,
    TEX_AMMO,
    
    // Effect textures
    TEX_MUZZLE_FLASH,
    TEX_CROSSHAIR,
    
    // Weapon textures
    TEX_WEAPON_METAL,      // For gun metal parts
    TEX_WEAPON_GRIP,       // For grips/handles
    
    // Alien textures (for Level 2)
    TEX_ALIEN_01,
    TEX_ALIEN_02,
    TEX_ALIEN_03,
    TEX_ALIEN_04,
    TEX_ALIEN_05,
    TEX_ALIEN_06,
    TEX_ALIEN_07,      // For demons
    TEX_ALIEN_15,      // For zombies
    
    // Player armor texture
    TEX_GALVANIZED_BLUE,  // For player torso
    
    // Lava zone environment textures (from 3D model folder)
    TEX_LAVA_TERRAIN,      // TerrainGEN_3LAVAColor.png
    TEX_LAVA_TERRAIN_NORMAL,  // TerrainGEN_3Normal.png
    
    // Skybox textures - TitanMoon
    TEX_SKYBOX_FRONT,
    TEX_SKYBOX_BACK,
    TEX_SKYBOX_LEFT,
    TEX_SKYBOX_RIGHT,
    TEX_SKYBOX_TOP,
    TEX_SKYBOX_BOTTOM,
    
    // Total count
    TEX_COUNT
};

class TextureManager {
private:
    static GLuint textures[TEX_COUNT];
    static bool initialized;
    static bool texturesLoaded[TEX_COUNT];
    
    // Helper to load a single texture
    static GLuint loadTexture(const char* filepath, bool repeat = true) {
        printf("Loading texture: %s\n", filepath);
        
        // Use SOIL to load with power-of-2 resizing for compatibility
        unsigned int flags = SOIL_FLAG_INVERT_Y | SOIL_FLAG_MIPMAPS | SOIL_FLAG_POWER_OF_TWO;
        
        GLuint texID = SOIL_load_OGL_texture(
            filepath,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            flags
        );
        
        if (texID == 0) {
            printf("ERROR: Failed to load texture: %s\n", filepath);
            printf("SOIL error: %s\n", SOIL_last_result());
            return 0;
        }
        
        // Set texture parameters
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        if (repeat) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } else {
            // Use GL_CLAMP for older OpenGL compatibility (instead of GL_CLAMP_TO_EDGE)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }
        
        printf("SUCCESS: Loaded texture %s (ID: %d)\n", filepath, texID);
        return texID;
    }
    
public:
    static void init() {
        if (initialized) return;
        
        printf("=== TextureManager: Initializing textures ===\n");
        
        // Initialize all to 0 (not loaded)
        for (int i = 0; i < TEX_COUNT; i++) {
            textures[i] = 0;
            texturesLoaded[i] = false;
        }
        
        // Get absolute path based on exe location
        char exePath[512];
        GetModuleFileNameA(NULL, exePath, 512);
        // Remove exe filename to get directory
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash) *lastSlash = '\0';
        
        // Build base path: exePath\..\res\Textures\ 
        char basePath[512];
        sprintf(basePath, "%s\\..\\res\\Textures\\", exePath);
        printf("Texture base path: %s\n", basePath);
        
        char filepath[512];
        
        // ==================== FLOOR TEXTURES ====================
        sprintf(filepath, "%slab-floor.png", basePath);
        textures[TEX_FLOOR_LAB] = loadTexture(filepath);
        texturesLoaded[TEX_FLOOR_LAB] = (textures[TEX_FLOOR_LAB] != 0);
        
        sprintf(filepath, "%stile006_lab1.png", basePath);
        textures[TEX_FLOOR_TILE] = loadTexture(filepath);
        texturesLoaded[TEX_FLOOR_TILE] = (textures[TEX_FLOOR_TILE] != 0);
        
        sprintf(filepath, "%stile021.png", basePath);
        textures[TEX_FLOOR_TILE2] = loadTexture(filepath);
        texturesLoaded[TEX_FLOOR_TILE2] = (textures[TEX_FLOOR_TILE2] != 0);
        
        sprintf(filepath, "%sfloor-grey.png", basePath);
        textures[TEX_FLOOR_METAL] = loadTexture(filepath);
        texturesLoaded[TEX_FLOOR_METAL] = (textures[TEX_FLOOR_METAL] != 0);
        
        sprintf(filepath, "%sLava-4.png", basePath);
        textures[TEX_LAVA] = loadTexture(filepath);
        texturesLoaded[TEX_LAVA] = (textures[TEX_LAVA] != 0);
        printf("Lava texture loaded: %s (ID: %d)\n", texturesLoaded[TEX_LAVA] ? "YES" : "NO", textures[TEX_LAVA]);
        
        sprintf(filepath, "%slava-effect.png", basePath);
        textures[TEX_LAVA_GLOW] = loadTexture(filepath);
        texturesLoaded[TEX_LAVA_GLOW] = (textures[TEX_LAVA_GLOW] != 0);
        
        // ==================== WALL TEXTURES ====================
        sprintf(filepath, "%sstoned-wall-grey.png", basePath);
        textures[TEX_WALL_GREY] = loadTexture(filepath);
        texturesLoaded[TEX_WALL_GREY] = (textures[TEX_WALL_GREY] != 0);
        
        sprintf(filepath, "%sstoned-wall-blue.png", basePath);
        textures[TEX_WALL_BLUE] = loadTexture(filepath);
        texturesLoaded[TEX_WALL_BLUE] = (textures[TEX_WALL_BLUE] != 0);
        
        sprintf(filepath, "%stile040.png", basePath);
        textures[TEX_WALL_PANEL] = loadTexture(filepath);
        texturesLoaded[TEX_WALL_PANEL] = (textures[TEX_WALL_PANEL] != 0);
        
        sprintf(filepath, "%sorange-warning.png", basePath);
        textures[TEX_WALL_ORANGE_WARNING] = loadTexture(filepath);
        texturesLoaded[TEX_WALL_ORANGE_WARNING] = (textures[TEX_WALL_ORANGE_WARNING] != 0);
        
        // ==================== OBJECT TEXTURES ====================
        sprintf(filepath, "%scubeTex.png", basePath);
        textures[TEX_CRATE] = loadTexture(filepath);
        texturesLoaded[TEX_CRATE] = (textures[TEX_CRATE] != 0);
        
        sprintf(filepath, "%stile061.png", basePath);
        textures[TEX_CRATE_SCIFI] = loadTexture(filepath);
        texturesLoaded[TEX_CRATE_SCIFI] = (textures[TEX_CRATE_SCIFI] != 0);
        
        sprintf(filepath, "%stile062.png", basePath);
        textures[TEX_PLATFORM] = loadTexture(filepath);
        texturesLoaded[TEX_PLATFORM] = (textures[TEX_PLATFORM] != 0);
        
        sprintf(filepath, "%stile139.png", basePath);
        textures[TEX_PLATFORM_LAVA] = loadTexture(filepath);
        texturesLoaded[TEX_PLATFORM_LAVA] = (textures[TEX_PLATFORM_LAVA] != 0);
        
        sprintf(filepath, "%sparkour-texture.png", basePath);
        textures[TEX_PARKOUR] = loadTexture(filepath);
        texturesLoaded[TEX_PARKOUR] = (textures[TEX_PARKOUR] != 0);
        
        // ==================== ROCK TEXTURE ====================
        sprintf(filepath, "%s\\..\\res\\Models3D\\Rock\\TexturesCom_RockSharp0009_1_seamless_S.jpg.001.jpg", exePath);
        textures[TEX_ROCK] = loadTexture(filepath);
        texturesLoaded[TEX_ROCK] = (textures[TEX_ROCK] != 0);
        
        sprintf(filepath, "%stile068.png", basePath);
        textures[TEX_PILLAR] = loadTexture(filepath);
        texturesLoaded[TEX_PILLAR] = (textures[TEX_PILLAR] != 0);
        
        // ==================== CHARACTER TEXTURES ====================
        // Player uses galvanized blue texture
        sprintf(filepath, "%sgalvanized_blue.jpg", basePath);
        textures[TEX_PLAYER] = loadTexture(filepath);
        texturesLoaded[TEX_PLAYER] = (textures[TEX_PLAYER] != 0);
        
        // ZOMBIE - Use object-green.png for GREEN zombies
        sprintf(filepath, "%sobject-green.png", basePath);
        textures[TEX_ENEMY_ZOMBIE] = loadTexture(filepath);
        texturesLoaded[TEX_ENEMY_ZOMBIE] = (textures[TEX_ENEMY_ZOMBIE] != 0);
        
        // DEMON - Use weird alien texture (red/orange demon skin)
        sprintf(filepath, "%sweird_alien_textures\\alien_09.jpg", basePath);
        textures[TEX_ENEMY_DEMON] = loadTexture(filepath);
        texturesLoaded[TEX_ENEMY_DEMON] = (textures[TEX_ENEMY_DEMON] != 0);
        
        // BOSS - Use devil.png texture from devil model folder
        sprintf(filepath, "%s\\..\\res\\Models3D\\devil\\devil.png", exePath);
        textures[TEX_ENEMY_BOSS] = loadTexture(filepath);
        texturesLoaded[TEX_ENEMY_BOSS] = (textures[TEX_ENEMY_BOSS] != 0);
        
        // ==================== COLLECTIBLE TEXTURES ====================
        sprintf(filepath, "%sHealthIcon.png", basePath);
        textures[TEX_HEALTHPACK] = loadTexture(filepath);
        texturesLoaded[TEX_HEALTHPACK] = (textures[TEX_HEALTHPACK] != 0);
        
        // Use AK-47 magazine texture for ammo pickups
        sprintf(filepath, "%s..\\Models3D\\ak-47-magazine\\textures\\ak_47_round_BaseColor.jpeg", basePath);
        textures[TEX_AMMO] = loadTexture(filepath);
        texturesLoaded[TEX_AMMO] = (textures[TEX_AMMO] != 0);
        
        // ==================== EFFECT TEXTURES ====================
        sprintf(filepath, "%smuzzleFlash.png", basePath);
        textures[TEX_MUZZLE_FLASH] = loadTexture(filepath);
        texturesLoaded[TEX_MUZZLE_FLASH] = (textures[TEX_MUZZLE_FLASH] != 0);
        
        sprintf(filepath, "%scrossHair.png", basePath);
        textures[TEX_CROSSHAIR] = loadTexture(filepath);
        texturesLoaded[TEX_CROSSHAIR] = (textures[TEX_CROSSHAIR] != 0);
        
        // ==================== WEAPON TEXTURES ====================
        sprintf(filepath, "%sarmy-grey.png", basePath);
        textures[TEX_WEAPON_METAL] = loadTexture(filepath);
        texturesLoaded[TEX_WEAPON_METAL] = (textures[TEX_WEAPON_METAL] != 0);
        
        sprintf(filepath, "%sarmy-grey2.png", basePath);
        textures[TEX_WEAPON_GRIP] = loadTexture(filepath);
        texturesLoaded[TEX_WEAPON_GRIP] = (textures[TEX_WEAPON_GRIP] != 0);
        
        // ==================== ALIEN TEXTURES (Level 2) ====================
        sprintf(filepath, "%sweird_alien_textures/alien_01.jpg", basePath);
        textures[TEX_ALIEN_01] = loadTexture(filepath);
        texturesLoaded[TEX_ALIEN_01] = (textures[TEX_ALIEN_01] != 0);
        
        sprintf(filepath, "%sweird_alien_textures/alien_02.jpg", basePath);
        textures[TEX_ALIEN_02] = loadTexture(filepath);
        texturesLoaded[TEX_ALIEN_02] = (textures[TEX_ALIEN_02] != 0);
        
        sprintf(filepath, "%sweird_alien_textures/alien_03.jpg", basePath);
        textures[TEX_ALIEN_03] = loadTexture(filepath);
        texturesLoaded[TEX_ALIEN_03] = (textures[TEX_ALIEN_03] != 0);
        
        sprintf(filepath, "%sweird_alien_textures/alien_04.jpg", basePath);
        textures[TEX_ALIEN_04] = loadTexture(filepath);
        texturesLoaded[TEX_ALIEN_04] = (textures[TEX_ALIEN_04] != 0);
        
        sprintf(filepath, "%sweird_alien_textures/alien_05.jpg", basePath);
        textures[TEX_ALIEN_05] = loadTexture(filepath);
        texturesLoaded[TEX_ALIEN_05] = (textures[TEX_ALIEN_05] != 0);
        
        sprintf(filepath, "%sweird_alien_textures/alien_06.jpg", basePath);
        textures[TEX_ALIEN_06] = loadTexture(filepath);
        texturesLoaded[TEX_ALIEN_06] = (textures[TEX_ALIEN_06] != 0);
        
        // Alien 07 for DEMON body texture
        sprintf(filepath, "%sweird_alien_textures/alien_07.jpg", basePath);
        textures[TEX_ALIEN_07] = loadTexture(filepath);
        texturesLoaded[TEX_ALIEN_07] = (textures[TEX_ALIEN_07] != 0);
        
        // Alien 15 for ZOMBIE body texture
        sprintf(filepath, "%sweird_alien_textures/alien_15.jpg", basePath);
        textures[TEX_ALIEN_15] = loadTexture(filepath);
        texturesLoaded[TEX_ALIEN_15] = (textures[TEX_ALIEN_15] != 0);
        
        // GALVANIZED BLUE for player armor
        sprintf(filepath, "%sgalvanized_blue.jpg", basePath);
        textures[TEX_GALVANIZED_BLUE] = loadTexture(filepath);
        texturesLoaded[TEX_GALVANIZED_BLUE] = (textures[TEX_GALVANIZED_BLUE] != 0);
        
        // ==================== LAVA TERRAIN TEXTURES (from 3D model folder) ====================
        // Build model path: exePath\..\res\Models3D\ 
        char modelPath[512];
        sprintf(modelPath, "%s\\..\\res\\Models3D\\free-lava-zone-environment\\textures\\", exePath);
        
        sprintf(filepath, "%sTerrainGEN_3LAVAColor_8bit.png", modelPath);
        textures[TEX_LAVA_TERRAIN] = loadTexture(filepath);
        texturesLoaded[TEX_LAVA_TERRAIN] = (textures[TEX_LAVA_TERRAIN] != 0);
        
        sprintf(filepath, "%sTerrainGEN_3Normal_8bit.png", modelPath);
        textures[TEX_LAVA_TERRAIN_NORMAL] = loadTexture(filepath);
        texturesLoaded[TEX_LAVA_TERRAIN_NORMAL] = (textures[TEX_LAVA_TERRAIN_NORMAL] != 0);
        
        // ==================== SKYBOX TEXTURES (TitanMoon) ====================
        // Use backslashes for Windows paths
        printf("=== Loading Skybox Textures ===\n");
        
        sprintf(filepath, "%sSkyboxes\\TitanMoon\\front.png", basePath);
        printf("Skybox front path: %s\n", filepath);
        textures[TEX_SKYBOX_FRONT] = loadTexture(filepath, false);
        texturesLoaded[TEX_SKYBOX_FRONT] = (textures[TEX_SKYBOX_FRONT] != 0);
        printf("Skybox front loaded: %s (ID: %d)\n", texturesLoaded[TEX_SKYBOX_FRONT] ? "YES" : "NO", textures[TEX_SKYBOX_FRONT]);
        
        sprintf(filepath, "%sSkyboxes\\TitanMoon\\back.png", basePath);
        textures[TEX_SKYBOX_BACK] = loadTexture(filepath, false);
        texturesLoaded[TEX_SKYBOX_BACK] = (textures[TEX_SKYBOX_BACK] != 0);
        
        sprintf(filepath, "%sSkyboxes\\TitanMoon\\left.png", basePath);
        textures[TEX_SKYBOX_LEFT] = loadTexture(filepath, false);
        texturesLoaded[TEX_SKYBOX_LEFT] = (textures[TEX_SKYBOX_LEFT] != 0);
        
        sprintf(filepath, "%sSkyboxes\\TitanMoon\\right.png", basePath);
        textures[TEX_SKYBOX_RIGHT] = loadTexture(filepath, false);
        texturesLoaded[TEX_SKYBOX_RIGHT] = (textures[TEX_SKYBOX_RIGHT] != 0);
        
        sprintf(filepath, "%sSkyboxes\\TitanMoon\\top.png", basePath);
        textures[TEX_SKYBOX_TOP] = loadTexture(filepath, false);
        texturesLoaded[TEX_SKYBOX_TOP] = (textures[TEX_SKYBOX_TOP] != 0);
        
        sprintf(filepath, "%sSkyboxes\\TitanMoon\\bottom.png", basePath);
        textures[TEX_SKYBOX_BOTTOM] = loadTexture(filepath, false);
        texturesLoaded[TEX_SKYBOX_BOTTOM] = (textures[TEX_SKYBOX_BOTTOM] != 0);
        
        printf("=== Skybox loading complete ===\n");
        
        initialized = true;
        
        // Print summary
        int loaded = 0;
        for (int i = 0; i < TEX_COUNT; i++) {
            if (texturesLoaded[i]) loaded++;
        }
        printf("=== TextureManager: Initialized %d/%d textures ===\n", loaded, TEX_COUNT);
    }
    
    static GLuint get(TextureID id) {
        if (!initialized) init();
        if (id < 0 || id >= TEX_COUNT) return 0;
        return textures[id];
    }
    
    static bool isLoaded(TextureID id) {
        if (!initialized) init();
        if (id < 0 || id >= TEX_COUNT) return false;
        return texturesLoaded[id];
    }
    
    // Bind texture with automatic fallback handling
    static void bind(TextureID id) {
        if (!initialized) init();
        GLuint tex = get(id);
        if (tex > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, tex);
        } else {
            glDisable(GL_TEXTURE_2D);
        }
    }
    
    static void unbind() {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
    
    // Draw a textured quad (for floors, walls, etc.)
    static void drawTexturedQuad(TextureID texID, float x, float y, float z, 
                                  float width, float height, float depth,
                                  float texScale = 1.0f, bool vertical = false) {
        bind(texID);
        
        glColor3f(1.0f, 1.0f, 1.0f);  // Full brightness
        
        if (!vertical) {
            // Horizontal quad (floor/ceiling)
            float u = width * texScale;
            float v = depth * texScale;
            
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glTexCoord2f(0, 0); glVertex3f(x - width/2, y, z - depth/2);
            glTexCoord2f(u, 0); glVertex3f(x + width/2, y, z - depth/2);
            glTexCoord2f(u, v); glVertex3f(x + width/2, y, z + depth/2);
            glTexCoord2f(0, v); glVertex3f(x - width/2, y, z + depth/2);
            glEnd();
        } else {
            // Vertical quad (wall)
            float u = width * texScale;
            float v = height * texScale;
            
            glBegin(GL_QUADS);
            glNormal3f(0, 0, 1);
            glTexCoord2f(0, 0); glVertex3f(x - width/2, y, z);
            glTexCoord2f(u, 0); glVertex3f(x + width/2, y, z);
            glTexCoord2f(u, v); glVertex3f(x + width/2, y + height, z);
            glTexCoord2f(0, v); glVertex3f(x - width/2, y + height, z);
            glEnd();
        }
        
        unbind();
    }
    
    // Draw textured box (6 sides)
    static void drawTexturedBox(TextureID texID, float x, float y, float z,
                                 float sizeX, float sizeY, float sizeZ,
                                 float texScale = 1.0f) {
        bind(texID);
        glColor3f(1.0f, 1.0f, 1.0f);
        
        float hx = sizeX / 2;
        float hy = sizeY / 2;
        float hz = sizeZ / 2;
        
        // Calculate texture coordinates based on scale
        float u = sizeX * texScale;
        float v = sizeY * texScale;
        float w = sizeZ * texScale;
        
        glPushMatrix();
        glTranslatef(x, y, z);
        
        glBegin(GL_QUADS);
        
        // Front face (+Z)
        glNormal3f(0, 0, 1);
        glTexCoord2f(0, 0); glVertex3f(-hx, -hy, hz);
        glTexCoord2f(u, 0); glVertex3f(hx, -hy, hz);
        glTexCoord2f(u, v); glVertex3f(hx, hy, hz);
        glTexCoord2f(0, v); glVertex3f(-hx, hy, hz);
        
        // Back face (-Z)
        glNormal3f(0, 0, -1);
        glTexCoord2f(0, 0); glVertex3f(hx, -hy, -hz);
        glTexCoord2f(u, 0); glVertex3f(-hx, -hy, -hz);
        glTexCoord2f(u, v); glVertex3f(-hx, hy, -hz);
        glTexCoord2f(0, v); glVertex3f(hx, hy, -hz);
        
        // Left face (-X)
        glNormal3f(-1, 0, 0);
        glTexCoord2f(0, 0); glVertex3f(-hx, -hy, -hz);
        glTexCoord2f(w, 0); glVertex3f(-hx, -hy, hz);
        glTexCoord2f(w, v); glVertex3f(-hx, hy, hz);
        glTexCoord2f(0, v); glVertex3f(-hx, hy, -hz);
        
        // Right face (+X)
        glNormal3f(1, 0, 0);
        glTexCoord2f(0, 0); glVertex3f(hx, -hy, hz);
        glTexCoord2f(w, 0); glVertex3f(hx, -hy, -hz);
        glTexCoord2f(w, v); glVertex3f(hx, hy, -hz);
        glTexCoord2f(0, v); glVertex3f(hx, hy, hz);
        
        // Top face (+Y)
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, 0); glVertex3f(-hx, hy, -hz);
        glTexCoord2f(u, 0); glVertex3f(-hx, hy, hz);
        glTexCoord2f(u, w); glVertex3f(hx, hy, hz);
        glTexCoord2f(0, w); glVertex3f(hx, hy, -hz);
        
        // Bottom face (-Y)
        glNormal3f(0, -1, 0);
        glTexCoord2f(0, 0); glVertex3f(-hx, -hy, hz);
        glTexCoord2f(u, 0); glVertex3f(-hx, -hy, -hz);
        glTexCoord2f(u, w); glVertex3f(hx, -hy, -hz);
        glTexCoord2f(0, w); glVertex3f(hx, -hy, hz);
        
        glEnd();
        
        glPopMatrix();
        
        unbind();
    }
    
    // Draw skybox centered around a position
    static void drawSkybox(float x, float y, float z, float size) {
        if (!initialized) init();
        
        // Check if skybox textures are loaded
        if (!texturesLoaded[TEX_SKYBOX_FRONT]) {
            return;  // Skybox not available
        }
        
        glPushMatrix();
        glTranslatef(x, y, z);
        
        // Save current states
        GLboolean lightingWasEnabled = glIsEnabled(GL_LIGHTING);
        GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);
        GLboolean texture2DWasEnabled = glIsEnabled(GL_TEXTURE_2D);
        GLboolean fogWasEnabled = glIsEnabled(GL_FOG);
        GLboolean cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);
        
        // Disable lighting, fog, and culling for skybox
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FOG);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        
        glColor3f(1.0f, 1.0f, 1.0f);
        
        float s = size / 2;
        
        // Front (+Z)
        glBindTexture(GL_TEXTURE_2D, textures[TEX_SKYBOX_FRONT]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-s, -s, s);
        glTexCoord2f(1, 0); glVertex3f(s, -s, s);
        glTexCoord2f(1, 1); glVertex3f(s, s, s);
        glTexCoord2f(0, 1); glVertex3f(-s, s, s);
        glEnd();
        
        // Back (-Z)
        glBindTexture(GL_TEXTURE_2D, textures[TEX_SKYBOX_BACK]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(s, -s, -s);
        glTexCoord2f(1, 0); glVertex3f(-s, -s, -s);
        glTexCoord2f(1, 1); glVertex3f(-s, s, -s);
        glTexCoord2f(0, 1); glVertex3f(s, s, -s);
        glEnd();
        
        // Left (-X)
        glBindTexture(GL_TEXTURE_2D, textures[TEX_SKYBOX_LEFT]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-s, -s, -s);
        glTexCoord2f(1, 0); glVertex3f(-s, -s, s);
        glTexCoord2f(1, 1); glVertex3f(-s, s, s);
        glTexCoord2f(0, 1); glVertex3f(-s, s, -s);
        glEnd();
        
        // Right (+X)
        glBindTexture(GL_TEXTURE_2D, textures[TEX_SKYBOX_RIGHT]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(s, -s, s);
        glTexCoord2f(1, 0); glVertex3f(s, -s, -s);
        glTexCoord2f(1, 1); glVertex3f(s, s, -s);
        glTexCoord2f(0, 1); glVertex3f(s, s, s);
        glEnd();
        
        // Top (+Y)
        glBindTexture(GL_TEXTURE_2D, textures[TEX_SKYBOX_TOP]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-s, s, s);
        glTexCoord2f(1, 0); glVertex3f(s, s, s);
        glTexCoord2f(1, 1); glVertex3f(s, s, -s);
        glTexCoord2f(0, 1); glVertex3f(-s, s, -s);
        glEnd();
        
        // Bottom (-Y)
        glBindTexture(GL_TEXTURE_2D, textures[TEX_SKYBOX_BOTTOM]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-s, -s, -s);
        glTexCoord2f(1, 0); glVertex3f(s, -s, -s);
        glTexCoord2f(1, 1); glVertex3f(s, -s, s);
        glTexCoord2f(0, 1); glVertex3f(-s, -s, s);
        glEnd();
        
        // Restore states
        glDepthMask(GL_TRUE);
        if (depthTestWasEnabled) glEnable(GL_DEPTH_TEST);
        if (lightingWasEnabled) glEnable(GL_LIGHTING);
        if (fogWasEnabled) glEnable(GL_FOG);
        if (cullFaceWasEnabled) glEnable(GL_CULL_FACE);
        if (!texture2DWasEnabled) glDisable(GL_TEXTURE_2D);
        
        glPopMatrix();
    }
    
    static void cleanup() {
        if (!initialized) return;
        
        for (int i = 0; i < TEX_COUNT; i++) {
            if (textures[i] > 0) {
                glDeleteTextures(1, &textures[i]);
                textures[i] = 0;
                texturesLoaded[i] = false;
            }
        }
        initialized = false;
    }
};

// Static member definitions
GLuint TextureManager::textures[TEX_COUNT] = {0};
bool TextureManager::initialized = false;
bool TextureManager::texturesLoaded[TEX_COUNT] = {false};

#endif // TEXTURE_MANAGER_H
