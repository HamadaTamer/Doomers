/**
 * DOOMERS - Lighting System
 * 
 * As per description:
 * 1. Emergency lights in lab (level 1):
 *    - Rotating red siren lights
 *    - Periodically change intensity from dim to bright red
 * 
 * 2. Outdoor arena main light (level 2):
 *    - Directional light representing sky
 *    - Color/intensity changes from orange to dark blue (sunset to night)
 */

#pragma once

#include "../Engine/Core.hpp"
#include "../Engine/Math.hpp"
#include <vector>
#include <cmath>

namespace Doomers {

// ============================================================================
// Emergency Siren Light - Rotating red light for lab corridors
// ============================================================================
class EmergencyLight {
public:
    Math::Vector3 position;
    Math::Color color = Math::Color(1.0f, 0.1f, 0.1f);  // Red
    
    float rotationAngle = 0;
    float rotationSpeed = 180.0f;  // Degrees per second
    
    float intensity = 1.0f;
    float minIntensity = 0.3f;
    float maxIntensity = 1.0f;
    float pulseSpeed = 2.0f;  // Pulses per second
    float pulseTimer = 0;
    
    float range = 15.0f;
    int lightIndex = 3;  // GL_LIGHT3 by default
    
    EmergencyLight(const Math::Vector3& pos, int glLightIndex = 3)
        : position(pos), lightIndex(glLightIndex) {}
    
    void update(float dt) {
        // Rotate the light direction
        rotationAngle += rotationSpeed * dt;
        if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;
        
        // Pulse intensity
        pulseTimer += dt * pulseSpeed;
        float pulse = (sinf(pulseTimer * Math::PI * 2.0f) + 1.0f) * 0.5f;
        intensity = minIntensity + (maxIntensity - minIntensity) * pulse;
    }
    
    void render() {
        GLenum light = GL_LIGHT0 + lightIndex;
        glEnable(light);
        
        // Calculate rotating direction (horizontal sweep)
        float radians = rotationAngle * Math::PI / 180.0f;
        Math::Vector3 direction(cosf(radians), -0.3f, sinf(radians));
        direction = direction.normalized();
        
        GLfloat lightPos[] = { position.x, position.y, position.z, 1.0f };
        GLfloat lightDir[] = { direction.x, direction.y, direction.z };
        GLfloat lightDiffuse[] = { 
            color.r * intensity, 
            color.g * intensity, 
            color.b * intensity, 
            1.0f 
        };
        GLfloat lightAmbient[] = { color.r * 0.1f, color.g * 0.1f, color.b * 0.1f, 1.0f };
        
        glLightfv(light, GL_POSITION, lightPos);
        glLightfv(light, GL_SPOT_DIRECTION, lightDir);
        glLightfv(light, GL_DIFFUSE, lightDiffuse);
        glLightfv(light, GL_AMBIENT, lightAmbient);
        
        glLightf(light, GL_SPOT_CUTOFF, 45.0f);  // Wide cone for siren effect
        glLightf(light, GL_SPOT_EXPONENT, 10.0f);
        
        glLightf(light, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(light, GL_LINEAR_ATTENUATION, 0.05f);
        glLightf(light, GL_QUADRATIC_ATTENUATION, 0.01f);
    }
    
    // Render the physical light fixture
    void renderFixture() {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        
        // Base
        glColor3f(0.2f, 0.2f, 0.2f);
        glPushMatrix();
        glScalef(0.15f, 0.05f, 0.15f);
        drawCylinder();
        glPopMatrix();
        
        // Light dome (glowing)
        glColor3f(color.r * intensity, color.g * intensity, color.b * intensity);
        glPushMatrix();
        glTranslatef(0, 0.05f, 0);
        glScalef(0.12f, 0.1f, 0.12f);
        drawHemisphere();
        glPopMatrix();
        
        glPopMatrix();
    }
    
private:
    void drawCylinder() {
        const int segments = 16;
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= segments; ++i) {
            float angle = (float)i / segments * Math::PI * 2.0f;
            float x = cosf(angle);
            float z = sinf(angle);
            glNormal3f(x, 0, z);
            glVertex3f(x, 1, z);
            glVertex3f(x, -1, z);
        }
        glEnd();
    }
    
    void drawHemisphere() {
        const int stacks = 8;
        const int slices = 16;
        for (int i = 0; i < stacks; ++i) {
            float phi1 = (float)i / stacks * Math::PI * 0.5f;
            float phi2 = (float)(i + 1) / stacks * Math::PI * 0.5f;
            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= slices; ++j) {
                float theta = (float)j / slices * Math::PI * 2.0f;
                float x1 = cosf(phi1) * cosf(theta);
                float y1 = sinf(phi1);
                float z1 = cosf(phi1) * sinf(theta);
                float x2 = cosf(phi2) * cosf(theta);
                float y2 = sinf(phi2);
                float z2 = cosf(phi2) * sinf(theta);
                glNormal3f(x1, y1, z1);
                glVertex3f(x1, y1, z1);
                glNormal3f(x2, y2, z2);
                glVertex3f(x2, y2, z2);
            }
            glEnd();
        }
    }
};

// ============================================================================
// Dynamic Sky Light - Sunset to night transition for outdoor level
// ============================================================================
class DynamicSkyLight {
public:
    // Time progression (0 = start/sunset, 1 = end/night)
    float timeProgress = 0;
    float transitionSpeed = 0.02f;  // Progress per second (full transition in ~50 seconds)
    bool autoProgress = true;
    
    // Sunset colors
    Math::Color sunsetSkyTop = Math::Color(0.8f, 0.4f, 0.2f);      // Orange
    Math::Color sunsetSkyHorizon = Math::Color(1.0f, 0.5f, 0.2f);  // Bright orange
    Math::Color sunsetLight = Math::Color(1.0f, 0.6f, 0.3f);       // Warm orange
    float sunsetIntensity = 0.9f;
    
    // Night colors
    Math::Color nightSkyTop = Math::Color(0.02f, 0.02f, 0.08f);    // Dark blue
    Math::Color nightSkyHorizon = Math::Color(0.05f, 0.05f, 0.15f); // Slightly lighter blue
    Math::Color nightLight = Math::Color(0.2f, 0.2f, 0.4f);        // Cool blue
    float nightIntensity = 0.15f;
    
    // Current interpolated values
    Math::Color currentSkyTop;
    Math::Color currentSkyHorizon;
    Math::Color currentLight;
    float currentIntensity;
    
    // Sun direction (goes down over time)
    Math::Vector3 sunDirection;
    float sunStartAngle = 15.0f;   // Degrees above horizon at start
    float sunEndAngle = -30.0f;    // Below horizon at end
    
    void update(float dt) {
        if (autoProgress) {
            timeProgress += transitionSpeed * dt;
            if (timeProgress > 1.0f) timeProgress = 1.0f;
        }
        
        // Interpolate colors using smooth easing
        float t = timeProgress;
        float smoothT = t * t * (3.0f - 2.0f * t);  // Smoothstep
        
        currentSkyTop = Math::Color::lerp(sunsetSkyTop, nightSkyTop, smoothT);
        currentSkyHorizon = Math::Color::lerp(sunsetSkyHorizon, nightSkyHorizon, smoothT);
        currentLight = Math::Color::lerp(sunsetLight, nightLight, smoothT);
        currentIntensity = sunsetIntensity + (nightIntensity - sunsetIntensity) * smoothT;
        
        // Update sun direction
        float sunAngle = sunStartAngle + (sunEndAngle - sunStartAngle) * smoothT;
        float radians = sunAngle * Math::PI / 180.0f;
        sunDirection = Math::Vector3(0.5f, sinf(radians), 0.5f).normalized();
    }
    
    void applyLighting() {
        // Main directional light (sun)
        glEnable(GL_LIGHT0);
        
        GLfloat lightPos[] = { sunDirection.x, sunDirection.y, sunDirection.z, 0.0f };  // w=0 for directional
        GLfloat lightDiffuse[] = { 
            currentLight.r * currentIntensity, 
            currentLight.g * currentIntensity, 
            currentLight.b * currentIntensity, 
            1.0f 
        };
        
        // Ambient based on sky color
        GLfloat lightAmbient[] = { 
            currentSkyTop.r * 0.3f, 
            currentSkyTop.g * 0.3f, 
            currentSkyTop.b * 0.3f, 
            1.0f 
        };
        
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    }
    
    void renderSky() {
        // Render gradient sky dome
        glPushMatrix();
        glDisable(GL_LIGHTING);
        glDepthMask(GL_FALSE);
        
        const int segments = 32;
        const int rings = 16;
        const float radius = 500.0f;
        
        glBegin(GL_TRIANGLE_STRIP);
        for (int ring = 0; ring < rings; ++ring) {
            float t1 = (float)ring / rings;
            float t2 = (float)(ring + 1) / rings;
            
            float pitch1 = (t1 - 0.5f) * Math::PI;
            float pitch2 = (t2 - 0.5f) * Math::PI;
            
            for (int seg = 0; seg <= segments; ++seg) {
                float yaw = (float)seg / segments * Math::PI * 2.0f;
                
                float x1 = cosf(pitch1) * sinf(yaw) * radius;
                float y1 = sinf(pitch1) * radius;
                float z1 = cosf(pitch1) * cosf(yaw) * radius;
                
                float x2 = cosf(pitch2) * sinf(yaw) * radius;
                float y2 = sinf(pitch2) * radius;
                float z2 = cosf(pitch2) * cosf(yaw) * radius;
                
                // Color based on height
                Math::Color c1 = getColorAtHeight(t1);
                Math::Color c2 = getColorAtHeight(t2);
                
                glColor3f(c1.r, c1.g, c1.b);
                glVertex3f(x1, y1, z1);
                
                glColor3f(c2.r, c2.g, c2.b);
                glVertex3f(x2, y2, z2);
            }
        }
        glEnd();
        
        // Render sun/moon glow
        if (sunDirection.y > -0.1f) {
            renderSunGlow();
        }
        
        glDepthMask(GL_TRUE);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
    
    void setTime(float t) {
        timeProgress = Math::clamp(t, 0.0f, 1.0f);
    }
    
    float getTime() const { return timeProgress; }
    
private:
    Math::Color getColorAtHeight(float t) {
        // t = 0 is bottom, t = 1 is top
        if (t < 0.4f) {
            float blend = t / 0.4f;
            Math::Color bottom(currentSkyHorizon.r * 0.5f, currentSkyHorizon.g * 0.5f, currentSkyHorizon.b * 0.5f);
            return Math::Color::lerp(bottom, currentSkyHorizon, blend);
        } else {
            float blend = (t - 0.4f) / 0.6f;
            return Math::Color::lerp(currentSkyHorizon, currentSkyTop, blend);
        }
    }
    
    void renderSunGlow() {
        Math::Vector3 sunPos = sunDirection * 400.0f;
        
        glPushMatrix();
        glTranslatef(sunPos.x, sunPos.y, sunPos.z);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        // Sun disc
        float sunSize = 30.0f * (0.5f + currentIntensity * 0.5f);
        glColor4f(currentLight.r, currentLight.g, currentLight.b, currentIntensity);
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, 0);
        for (int i = 0; i <= 16; ++i) {
            float angle = (float)i / 16.0f * Math::PI * 2.0f;
            glVertex3f(cosf(angle) * sunSize, sinf(angle) * sunSize, 0);
        }
        glEnd();
        
        // Glow
        for (int layer = 0; layer < 3; ++layer) {
            float layerSize = sunSize * (2.0f + layer * 1.5f);
            float alpha = 0.3f * currentIntensity / (layer + 1);
            
            glBegin(GL_TRIANGLE_FAN);
            glColor4f(currentLight.r, currentLight.g, currentLight.b, alpha);
            glVertex3f(0, 0, 0);
            glColor4f(currentLight.r, currentLight.g, currentLight.b, 0);
            for (int i = 0; i <= 16; ++i) {
                float angle = (float)i / 16.0f * Math::PI * 2.0f;
                glVertex3f(cosf(angle) * layerSize, sinf(angle) * layerSize, 0);
            }
            glEnd();
        }
        
        glDisable(GL_BLEND);
        glPopMatrix();
    }
};

// ============================================================================
// Lighting Manager - Handles all lights for both levels
// ============================================================================
class LightingManager {
public:
    // Emergency lights for level 1 (lab)
    std::vector<EmergencyLight> emergencyLights;
    
    // Sky light for level 2 (outdoor)
    DynamicSkyLight skyLight;
    
    // Which level we're in
    int currentLevel = 1;
    
    void setupLevel1() {
        currentLevel = 1;
        emergencyLights.clear();
        
        // Will be populated when level is loaded
        // Example positions would be added by level loader
    }
    
    void setupLevel2() {
        currentLevel = 2;
        emergencyLights.clear();
        
        // Reset sky to sunset
        skyLight.timeProgress = 0;
        skyLight.autoProgress = true;
    }
    
    void addEmergencyLight(const Math::Vector3& position, int lightIndex) {
        emergencyLights.emplace_back(position, lightIndex);
    }
    
    void update(float dt) {
        if (currentLevel == 1) {
            // Update emergency lights
            for (auto& light : emergencyLights) {
                light.update(dt);
            }
        } else if (currentLevel == 2) {
            // Update sky transition
            skyLight.update(dt);
        }
    }
    
    void applyLighting() {
        if (currentLevel == 1) {
            // Dark ambient for lab
            GLfloat ambient[] = { 0.1f, 0.1f, 0.12f, 1.0f };
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
            
            // Render emergency lights
            for (auto& light : emergencyLights) {
                light.render();
            }
        } else if (currentLevel == 2) {
            // Apply sky light
            skyLight.applyLighting();
        }
    }
    
    void renderSky() {
        if (currentLevel == 2) {
            skyLight.renderSky();
        }
    }
    
    void renderLightFixtures() {
        if (currentLevel == 1) {
            for (auto& light : emergencyLights) {
                light.renderFixture();
            }
        }
    }
};

} // namespace Doomers
