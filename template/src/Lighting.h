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
        
        // Interpolate between sunset orange and dark blue
        float sunsetR = 0.8f - progress * 0.6f;
        float sunsetG = 0.4f - progress * 0.35f;
        float sunsetB = 0.2f + progress * 0.3f;
        
        float globalAmbient[] = {sunsetR * 0.2f, sunsetG * 0.2f, sunsetB * 0.2f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
        
        // Directional "sky" light
        ambientLight.setColor(sunsetR, sunsetG, sunsetB);
        ambientLight.enabled = true;
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
