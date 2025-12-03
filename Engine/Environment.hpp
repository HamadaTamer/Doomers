/**
 * DOOMERS - Environment System
 * 
 * Environment management for:
 * - Skybox rendering
 * - Fog effects
 * - Ambient lighting
 * - Environmental hazards
 * - Level atmosphere
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>

namespace Doomers {

// ============================================================================
// Environment Types
// ============================================================================
enum class EnvironmentType {
    Lab,           // Indoor sci-fi lab
    Hell,          // Outdoor hell arena
    Corridor,      // Dark corridors
    Custom
};

// ============================================================================
// Fog Settings
// ============================================================================
struct FogSettings {
    bool enabled = true;
    Math::Color color{0.1f, 0.1f, 0.15f, 1.0f};
    float density = 0.02f;
    float start = 5.0f;
    float end = 50.0f;
    int mode = GL_EXP2;  // GL_LINEAR, GL_EXP, GL_EXP2
    
    void apply() const {
        if (!enabled) {
            glDisable(GL_FOG);
            return;
        }
        
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, mode);
        
        GLfloat fogColor[] = { color.r, color.g, color.b, color.a };
        glFogfv(GL_FOG_COLOR, fogColor);
        
        if (mode == GL_LINEAR) {
            glFogf(GL_FOG_START, start);
            glFogf(GL_FOG_END, end);
        } else {
            glFogf(GL_FOG_DENSITY, density);
        }
        
        glHint(GL_FOG_HINT, GL_NICEST);
    }
};

// ============================================================================
// Ambient Light Settings
// ============================================================================
struct AmbientSettings {
    Math::Color color{0.2f, 0.2f, 0.25f, 1.0f};
    float intensity = 0.3f;
    
    void apply() const {
        GLfloat ambient[] = {
            color.r * intensity,
            color.g * intensity,
            color.b * intensity,
            1.0f
        };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    }
};

// ============================================================================
// Skybox (simple color gradient for now)
// ============================================================================
class Skybox {
public:
    Math::Color topColor{0.1f, 0.1f, 0.2f, 1.0f};
    Math::Color horizonColor{0.2f, 0.15f, 0.1f, 1.0f};
    Math::Color bottomColor{0.05f, 0.05f, 0.05f, 1.0f};
    
    void setLabPreset() {
        topColor = {0.05f, 0.05f, 0.1f, 1.0f};
        horizonColor = {0.1f, 0.1f, 0.15f, 1.0f};
        bottomColor = {0.02f, 0.02f, 0.05f, 1.0f};
    }
    
    void setHellPreset() {
        topColor = {0.1f, 0.02f, 0.02f, 1.0f};
        horizonColor = {0.4f, 0.1f, 0.05f, 1.0f};
        bottomColor = {0.05f, 0.02f, 0.02f, 1.0f};
    }
    
    void draw(const Math::Vector3& cameraPos) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_TEXTURE_2D);
        
        float size = 500.0f;
        
        glPushMatrix();
        glTranslatef(cameraPos.x, cameraPos.y, cameraPos.z);
        
        // Draw gradient dome
        int segments = 32;
        int rings = 16;
        
        for (int i = 0; i < rings; ++i) {
            float t0 = (float)i / rings;
            float t1 = (float)(i + 1) / rings;
            
            float y0 = cosf(t0 * Math::PI * 0.5f);
            float y1 = cosf(t1 * Math::PI * 0.5f);
            float r0 = sinf(t0 * Math::PI * 0.5f);
            float r1 = sinf(t1 * Math::PI * 0.5f);
            
            // Interpolate color
            Math::Color c0 = lerpColor(horizonColor, topColor, t0);
            Math::Color c1 = lerpColor(horizonColor, topColor, t1);
            
            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= segments; ++j) {
                float angle = 2.0f * Math::PI * j / segments;
                float x = cosf(angle);
                float z = sinf(angle);
                
                glColor3f(c0.r, c0.g, c0.b);
                glVertex3f(x * r0 * size, y0 * size, z * r0 * size);
                
                glColor3f(c1.r, c1.g, c1.b);
                glVertex3f(x * r1 * size, y1 * size, z * r1 * size);
            }
            glEnd();
        }
        
        // Bottom hemisphere
        for (int i = 0; i < rings; ++i) {
            float t0 = (float)i / rings;
            float t1 = (float)(i + 1) / rings;
            
            float y0 = -cosf(t0 * Math::PI * 0.5f);
            float y1 = -cosf(t1 * Math::PI * 0.5f);
            float r0 = sinf(t0 * Math::PI * 0.5f);
            float r1 = sinf(t1 * Math::PI * 0.5f);
            
            Math::Color c0 = lerpColor(horizonColor, bottomColor, t0);
            Math::Color c1 = lerpColor(horizonColor, bottomColor, t1);
            
            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= segments; ++j) {
                float angle = 2.0f * Math::PI * j / segments;
                float x = cosf(angle);
                float z = sinf(angle);
                
                glColor3f(c0.r, c0.g, c0.b);
                glVertex3f(x * r0 * size, y0 * size, z * r0 * size);
                
                glColor3f(c1.r, c1.g, c1.b);
                glVertex3f(x * r1 * size, y1 * size, z * r1 * size);
            }
            glEnd();
        }
        
        glPopMatrix();
        glPopAttrib();
    }
    
private:
    Math::Color lerpColor(const Math::Color& a, const Math::Color& b, float t) {
        return Math::Color{
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        };
    }
};

// ============================================================================
// Environment Manager
// ============================================================================
class Environment {
public:
    EnvironmentType type = EnvironmentType::Lab;
    FogSettings fog;
    AmbientSettings ambient;
    Skybox skybox;
    
    // Clear color
    Math::Color clearColor{0.05f, 0.05f, 0.08f, 1.0f};
    
    // Main light (sun/ceiling light)
    struct MainLight {
        Math::Vector3 direction{-0.5f, -1.0f, -0.3f};
        Math::Color diffuse{0.8f, 0.75f, 0.7f, 1.0f};
        Math::Color specular{1.0f, 0.95f, 0.9f, 1.0f};
        float intensity = 1.0f;
    } mainLight;
    
    Environment() = default;
    
    void setEnvironmentType(EnvironmentType t) {
        type = t;
        
        switch (t) {
            case EnvironmentType::Lab:
                setLabEnvironment();
                break;
            case EnvironmentType::Hell:
                setHellEnvironment();
                break;
            case EnvironmentType::Corridor:
                setCorridorEnvironment();
                break;
            default:
                break;
        }
    }
    
    void setLabEnvironment() {
        // Cool, sterile lab atmosphere
        clearColor = {0.02f, 0.02f, 0.05f, 1.0f};
        
        fog.enabled = true;
        fog.color = {0.05f, 0.05f, 0.1f, 1.0f};
        fog.density = 0.015f;
        fog.mode = GL_EXP2;
        
        ambient.color = {0.3f, 0.35f, 0.4f, 1.0f};
        ambient.intensity = 0.25f;
        
        mainLight.direction = {-0.3f, -1.0f, -0.2f};
        mainLight.diffuse = {0.9f, 0.95f, 1.0f, 1.0f};  // Cool white
        mainLight.intensity = 0.8f;
        
        skybox.setLabPreset();
    }
    
    void setHellEnvironment() {
        // Hot, fiery hell atmosphere
        clearColor = {0.05f, 0.02f, 0.02f, 1.0f};
        
        fog.enabled = true;
        fog.color = {0.15f, 0.05f, 0.02f, 1.0f};
        fog.density = 0.02f;
        fog.mode = GL_EXP2;
        
        ambient.color = {0.5f, 0.2f, 0.1f, 1.0f};
        ambient.intensity = 0.3f;
        
        mainLight.direction = {0.2f, -0.8f, 0.3f};
        mainLight.diffuse = {1.0f, 0.6f, 0.3f, 1.0f};  // Warm orange
        mainLight.intensity = 1.0f;
        
        skybox.setHellPreset();
    }
    
    void setCorridorEnvironment() {
        // Dark, claustrophobic corridors
        clearColor = {0.01f, 0.01f, 0.02f, 1.0f};
        
        fog.enabled = true;
        fog.color = {0.02f, 0.02f, 0.03f, 1.0f};
        fog.density = 0.03f;
        fog.mode = GL_EXP2;
        
        ambient.color = {0.15f, 0.15f, 0.2f, 1.0f};
        ambient.intensity = 0.15f;
        
        mainLight.direction = {0, -1.0f, 0};
        mainLight.diffuse = {0.5f, 0.5f, 0.6f, 1.0f};
        mainLight.intensity = 0.5f;
        
        skybox.setLabPreset();  // Won't be visible anyway
    }
    
    void apply() {
        // Set clear color
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        
        // Apply fog
        fog.apply();
        
        // Apply ambient
        ambient.apply();
        
        // Apply main light
        applyMainLight();
    }
    
    void applyMainLight() {
        Math::Vector3 dir = mainLight.direction.normalized();
        GLfloat position[] = { -dir.x, -dir.y, -dir.z, 0.0f };  // Directional
        GLfloat diffuse[] = {
            mainLight.diffuse.r * mainLight.intensity,
            mainLight.diffuse.g * mainLight.intensity,
            mainLight.diffuse.b * mainLight.intensity,
            1.0f
        };
        GLfloat specular[] = {
            mainLight.specular.r * mainLight.intensity,
            mainLight.specular.g * mainLight.intensity,
            mainLight.specular.b * mainLight.intensity,
            1.0f
        };
        
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    }
    
    void drawSkybox(const Math::Vector3& cameraPos) {
        skybox.draw(cameraPos);
    }
    
    // Transition between environments (for level changes)
    void transitionTo(EnvironmentType target, float t) {
        // Store current settings and lerp to new ones
        // For now, just instant switch
        if (t >= 1.0f) {
            setEnvironmentType(target);
        }
    }
};

// ============================================================================
// Environmental Hazard
// ============================================================================
struct EnvironmentalHazard {
    Math::Vector3 position;
    Math::Vector3 size;
    float damagePerSecond = 10.0f;
    std::string type = "generic";  // "fire", "acid", "electricity"
    bool active = true;
    
    bool containsPoint(const Math::Vector3& point) const {
        Math::Vector3 halfSize = size * 0.5f;
        Math::Vector3 min = position - halfSize;
        Math::Vector3 max = position + halfSize;
        
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
};

// ============================================================================
// Global Environment Instance
// ============================================================================
inline Environment& getEnvironment() {
    static Environment env;
    return env;
}

} // namespace Doomers
