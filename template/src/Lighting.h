// ============================================================================
// DOOMERS - Lighting.h
// Lighting system: flashlight, emergency lights, directional lights
// ============================================================================
#ifndef LIGHTING_H
#define LIGHTING_H

#include "Vector3.h"
#include "GameConfig.h"
#include <glut.h>
#include <math.h>

class Light {
public:
    Vector3 position;
    Vector3 direction;
    float color[4];
    float ambient[4];
    float diffuse[4];
    float specular[4];
    
    bool isSpotlight;
    float spotCutoff;
    float spotExponent;
    
    float constantAtt;
    float linearAtt;
    float quadraticAtt;
    
    bool enabled;
    int lightID;
    
    Light() {
        position = Vector3(0, 5, 0);
        direction = Vector3(0, -1, 0);
        
        color[0] = color[1] = color[2] = 1.0f; color[3] = 1.0f;
        ambient[0] = 0.1f; ambient[1] = 0.1f; ambient[2] = 0.1f; ambient[3] = 1.0f;
        diffuse[0] = 1.0f; diffuse[1] = 1.0f; diffuse[2] = 1.0f; diffuse[3] = 1.0f;
        specular[0] = 0.5f; specular[1] = 0.5f; specular[2] = 0.5f; specular[3] = 1.0f;
        
        isSpotlight = false;
        spotCutoff = 45.0f;
        spotExponent = 20.0f;
        
        constantAtt = 1.0f;
        linearAtt = 0.05f;
        quadraticAtt = 0.01f;
        
        enabled = true;
        lightID = GL_LIGHT0;
    }
    
    void setAsPointLight(const Vector3& pos, float r, float g, float b) {
        position = pos;
        isSpotlight = false;
        setColor(r, g, b);
    }
    
    void setAsSpotlight(const Vector3& pos, const Vector3& dir, float cutoff = 30.0f) {
        position = pos;
        direction = dir;
        isSpotlight = true;
        spotCutoff = cutoff;
    }
    
    void setColor(float r, float g, float b) {
        diffuse[0] = r; diffuse[1] = g; diffuse[2] = b;
        specular[0] = r * 0.5f; specular[1] = g * 0.5f; specular[2] = b * 0.5f;
    }
    
    void apply() {
        if (!enabled) {
            glDisable(lightID);
            return;
        }
        
        glEnable(lightID);
        
        // Position (w=1 for point light, w=0 for directional)
        float pos[4] = {position.x, position.y, position.z, 1.0f};
        glLightfv(lightID, GL_POSITION, pos);
        
        // Colors
        glLightfv(lightID, GL_AMBIENT, ambient);
        glLightfv(lightID, GL_DIFFUSE, diffuse);
        glLightfv(lightID, GL_SPECULAR, specular);
        
        // Attenuation
        glLightf(lightID, GL_CONSTANT_ATTENUATION, constantAtt);
        glLightf(lightID, GL_LINEAR_ATTENUATION, linearAtt);
        glLightf(lightID, GL_QUADRATIC_ATTENUATION, quadraticAtt);
        
        // Spotlight settings
        if (isSpotlight) {
            float dir[4] = {direction.x, direction.y, direction.z, 0.0f};
            glLightfv(lightID, GL_SPOT_DIRECTION, dir);
            glLightf(lightID, GL_SPOT_CUTOFF, spotCutoff);
            glLightf(lightID, GL_SPOT_EXPONENT, spotExponent);
        } else {
            glLightf(lightID, GL_SPOT_CUTOFF, 180.0f);
        }
    }
};

class LightingSystem {
public:
    Light flashlight;
    Light emergencyLights[4];
    Light ambientLight;
    Light thirdPersonLight; // Top light for third person view
    
    float emergencyPhase;
    float dayNightCycle; // For Level 2 sunset effect
    
    LightingSystem() {
        emergencyPhase = 0.0f;
        dayNightCycle = 0.0f;
        
        // Setup flashlight
        flashlight.lightID = GL_LIGHT0;
        flashlight.isSpotlight = true;
        flashlight.spotCutoff = FLASHLIGHT_ANGLE;
        flashlight.spotExponent = 30.0f;
        flashlight.setColor(1.0f, 0.95f, 0.8f);
        flashlight.linearAtt = 0.02f;
        flashlight.quadraticAtt = 0.005f;
        
        // Setup emergency lights
        for (int i = 0; i < 4; i++) {
            emergencyLights[i].lightID = GL_LIGHT1 + i;
            emergencyLights[i].isSpotlight = false;
            emergencyLights[i].setColor(0.8f, 0.1f, 0.1f);
            emergencyLights[i].linearAtt = 0.1f;
            emergencyLights[i].quadraticAtt = 0.02f;
        }
        
        // Ambient light
        ambientLight.lightID = GL_LIGHT5;
        ambientLight.position = Vector3(0, 50, 0);
        ambientLight.isSpotlight = false;
        
        // Third person top-down light (follows player from above)
        thirdPersonLight.lightID = GL_LIGHT6;
        thirdPersonLight.isSpotlight = false;
        thirdPersonLight.setColor(0.8f, 0.8f, 0.9f); // Bright white-blue
        thirdPersonLight.linearAtt = 0.02f;
        thirdPersonLight.quadraticAtt = 0.005f;
        thirdPersonLight.enabled = false;
    }
    
    void setupForLevel(int levelID) {
        if (levelID == LEVEL_1_FACILITY) {
            // Well-lit facility - normal indoor lighting
            float globalAmbient[] = {0.4f, 0.4f, 0.45f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
            
            // Bright ceiling lights in corners
            emergencyLights[0].position = Vector3(-25, 8, -25);
            emergencyLights[1].position = Vector3(25, 8, -25);
            emergencyLights[2].position = Vector3(-25, 8, 25);
            emergencyLights[3].position = Vector3(25, 8, 25);
            
            // Set to white/neutral light instead of red emergency
            for (int i = 0; i < 4; i++) {
                emergencyLights[i].setColor(0.9f, 0.85f, 0.8f);
                emergencyLights[i].enabled = true;
            }
            
            flashlight.enabled = true;
            
        } else if (levelID == LEVEL_2_HELL_ARENA) {
            // Outdoor hell arena with sunset/night transition
            updateDayNightCycle(0.0f);
            
            // Disable indoor lights
            for (int i = 0; i < 4; i++) {
                emergencyLights[i].enabled = false;
            }
            
            // Flashlight still available
            flashlight.enabled = true;
        }
    }
    
    void updateDayNightCycle(float progress) {
        dayNightCycle = progress;
        if (progress > 1.0f) progress = 1.0f;
        
        // Enhanced day-night transition for hell arena
        // Progress 0.0 = Sunset (orange/red sky)
        // Progress 0.3 = Dusk (dark red/purple)
        // Progress 0.6 = Early night (purple/blue)
        // Progress 1.0 = Deep night (dark blue)
        
        float ambientR, ambientG, ambientB;
        float lightR, lightG, lightB;
        
        if (progress < 0.3f) {
            // Sunset - warm orange light
            float t = progress / 0.3f;
            ambientR = 0.35f - t * 0.15f;  // 0.35 -> 0.2
            ambientG = 0.15f - t * 0.08f;  // 0.15 -> 0.07
            ambientB = 0.08f + t * 0.05f;  // 0.08 -> 0.13
            
            lightR = 1.0f - t * 0.3f;      // 1.0 -> 0.7
            lightG = 0.5f - t * 0.25f;     // 0.5 -> 0.25
            lightB = 0.2f + t * 0.1f;      // 0.2 -> 0.3
        } else if (progress < 0.6f) {
            // Dusk to early night - purple tones
            float t = (progress - 0.3f) / 0.3f;
            ambientR = 0.2f - t * 0.1f;    // 0.2 -> 0.1
            ambientG = 0.07f - t * 0.02f;  // 0.07 -> 0.05
            ambientB = 0.13f + t * 0.07f;  // 0.13 -> 0.2
            
            lightR = 0.7f - t * 0.35f;     // 0.7 -> 0.35
            lightG = 0.25f - t * 0.1f;     // 0.25 -> 0.15
            lightB = 0.3f + t * 0.15f;     // 0.3 -> 0.45
        } else {
            // Night - dark blue with lava glow
            float t = (progress - 0.6f) / 0.4f;
            ambientR = 0.1f + t * 0.05f;   // 0.1 -> 0.15 (lava influence)
            ambientG = 0.05f;
            ambientB = 0.2f - t * 0.08f;   // 0.2 -> 0.12
            
            lightR = 0.35f - t * 0.15f;    // 0.35 -> 0.2
            lightG = 0.15f - t * 0.1f;     // 0.15 -> 0.05
            lightB = 0.45f - t * 0.2f;     // 0.45 -> 0.25
        }
        
        float globalAmbient[] = {ambientR, ambientG, ambientB, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
        
        // Directional "sky" light - simulates sun/moon
        ambientLight.setColor(lightR, lightG, lightB);
        ambientLight.enabled = true;
        
        // Add lava glow effect to ambient during night
        if (progress > 0.5f) {
            float lavaInfluence = (progress - 0.5f) * 0.4f;
            // Subtle orange underglow from lava
            emergencyLights[0].position = Vector3(0, -5, 0);
            emergencyLights[0].setColor(0.8f * lavaInfluence, 0.3f * lavaInfluence, 0.05f * lavaInfluence);
            emergencyLights[0].linearAtt = 0.01f;
            emergencyLights[0].quadraticAtt = 0.002f;
            emergencyLights[0].enabled = true;
        }
    }
    
    void update(float deltaTime, const Vector3& playerPos, const Vector3& lookDir) {
        // Update flashlight position and direction
        flashlight.position = playerPos + Vector3(0, -0.1f, 0);
        flashlight.direction = lookDir;
        
        // Update third person light - position above and slightly behind player
        thirdPersonLight.position = playerPos + Vector3(0, 8.0f, 2.0f);
        
        // Update emergency light flicker/rotation
        emergencyPhase += deltaTime * 2.0f;
        
        for (int i = 0; i < 4; i++) {
            if (emergencyLights[i].enabled) {
                // Pulsing intensity
                float pulse = sin(emergencyPhase + i * 1.57f) * 0.3f + 0.7f;
                emergencyLights[i].setColor(0.8f * pulse, 0.1f * pulse, 0.1f * pulse);
            }
        }
    }
    
    void apply() {
        glEnable(GL_LIGHTING);
        
        flashlight.apply();
        
        for (int i = 0; i < 4; i++) {
            emergencyLights[i].apply();
        }
        
        ambientLight.apply();
        thirdPersonLight.apply();
    }
    
    void setFlashlightEnabled(bool enabled) {
        flashlight.enabled = enabled;
    }
    
    void toggleFlashlight() {
        flashlight.enabled = !flashlight.enabled;
    }
    
    void setThirdPersonLight(bool enabled) {
        thirdPersonLight.enabled = enabled;
    }
};

// Global lighting system
extern LightingSystem g_lighting;

#endif // LIGHTING_H
