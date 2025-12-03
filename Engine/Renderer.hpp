/**
 * DOOMERS - Renderer System
 * 
 * Handles all rendering operations using OpenGL 2.x fixed-function pipeline
 * Features:
 * - Lighting system (flashlight, ambient, directional)
 * - Fog/atmosphere effects
 * - Debug rendering (rays, bounding boxes, etc.)
 * - Post-processing effects (screen flash for damage)
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include "ResourceManager.hpp"

// Forward declaration for Camera
namespace Doomers { class Camera; }

namespace Doomers {

// ============================================================================
// Light - Light source definition
// ============================================================================
enum class LightType {
    Directional,
    Point,
    Spot
};

struct Light {
    LightType type;
    Math::Vector3 position;
    Math::Vector3 direction;
    Math::Color ambient;
    Math::Color diffuse;
    Math::Color specular;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    float spotCutoff;      // in degrees
    float spotExponent;
    bool enabled;
    
    Light() 
        : type(LightType::Point)
        , position(0, 0, 0)
        , direction(0, -1, 0)
        , ambient(0.1f, 0.1f, 0.1f)
        , diffuse(1, 1, 1)
        , specular(1, 1, 1)
        , constantAttenuation(1.0f)
        , linearAttenuation(0.0f)
        , quadraticAttenuation(0.0f)
        , spotCutoff(180.0f)
        , spotExponent(0.0f)
        , enabled(true)
    {}
    
    static Light createDirectional(const Math::Vector3& dir, const Math::Color& color) {
        Light light;
        light.type = LightType::Directional;
        light.direction = dir.normalized();
        light.diffuse = color;
        light.specular = color;
        return light;
    }
    
    static Light createPoint(const Math::Vector3& pos, const Math::Color& color, float range = 10.0f) {
        Light light;
        light.type = LightType::Point;
        light.position = pos;
        light.diffuse = color;
        light.specular = color;
        // Attenuation for range
        light.linearAttenuation = 2.0f / range;
        light.quadraticAttenuation = 1.0f / (range * range);
        return light;
    }
    
    static Light createSpot(const Math::Vector3& pos, const Math::Vector3& dir, 
                           float angle, const Math::Color& color) {
        Light light;
        light.type = LightType::Spot;
        light.position = pos;
        light.direction = dir.normalized();
        light.spotCutoff = angle;
        light.spotExponent = 20.0f;
        light.diffuse = color;
        light.specular = color;
        return light;
    }
    
    static Light createFlashlight(const Math::Vector3& pos, const Math::Vector3& dir) {
        Light light;
        light.type = LightType::Spot;
        light.position = pos;
        light.direction = dir.normalized();
        light.spotCutoff = 25.0f;
        light.spotExponent = 40.0f;
        light.diffuse = Math::Color(1.0f, 0.98f, 0.9f);
        light.specular = Math::Color(1.0f, 0.98f, 0.9f);
        light.constantAttenuation = 1.0f;
        light.linearAttenuation = 0.05f;
        light.quadraticAttenuation = 0.01f;
        return light;
    }
};

// ============================================================================
// DirectionalLight - Simple directional light (for main.cpp compatibility)
// ============================================================================
struct DirectionalLight {
    Math::Vector3 direction;
    Math::Color color;
    float intensity;
    
    DirectionalLight()
        : direction(0.5f, -1.0f, 0.3f)
        , color(1.0f, 1.0f, 1.0f)
        , intensity(1.0f)
    {}
};

// ============================================================================
// Renderer - Main rendering system
// ============================================================================
class Renderer {
public:
    static Renderer& instance() {
        static Renderer instance;
        return instance;
    }
    
    // Initialize OpenGL state
    void initialize() {
        initialize(screenWidth, screenHeight);
    }
    
    void initialize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
        
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        // Enable back-face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Enable texturing
        glEnable(GL_TEXTURE_2D);
        
        // Enable smooth shading
        glShadeModel(GL_SMOOTH);
        
        // Enable lighting
        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE); // Auto-normalize normals after scaling
        
        // Set up color material
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        
        // Background color (dark)
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        
        // Set up projection
        setupProjection();
        
        LOG_INFO("Renderer initialized (" << width << "x" << height << ")");
    }
    
    void resize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
        glViewport(0, 0, width, height);
        setupProjection();
    }
    
    void setupProjection() {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)screenWidth / (float)screenHeight;
        gluPerspective(fov, aspect, nearPlane, farPlane);
        glMatrixMode(GL_MODELVIEW);
    }
    
    // ========================================================================
    // Frame Management
    // ========================================================================
    void beginFrame() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
    
    void clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    void setClearColor(const Math::Color& color) {
        clearColor = color;
        glClearColor(color.r, color.g, color.b, color.a);
    }
    
    // ========================================================================
    // 3D View Setup with Camera
    // ========================================================================
    void begin3D(const Camera& camera) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)screenWidth / (float)screenHeight;
        gluPerspective(camera.getFOV(), aspect, nearPlane, farPlane);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        Math::Vector3 pos = camera.getPosition();
        Math::Vector3 target = pos + camera.getForward();
        Math::Vector3 camUp = camera.getUp();
        
        gluLookAt(pos.x, pos.y, pos.z,
                  target.x, target.y, target.z,
                  camUp.x, camUp.y, camUp.z);
        
        // Store camera info
        cameraPosition = pos;
        cameraDirection = camera.getForward();
        cameraRight = camera.getRight();
        cameraUp = camUp;
    }
    
    void applyLighting() {
        // Re-apply light positions (needed after camera transform)
        glEnable(GL_LIGHTING);
    }
    
    void endFrame() {
        // Draw any screen effects
        if (screenFlashIntensity > 0) {
            drawScreenFlash();
        }
        
        glutSwapBuffers();
    }
    
    // ========================================================================
    // Camera Setup
    // ========================================================================
    void setCamera(const Math::Vector3& position, const Math::Vector3& target, const Math::Vector3& up = Math::Vector3::up()) {
        cameraPosition = position;
        cameraTarget = target;
        cameraUp = up;
        cameraDirection = (target - position).normalized();
        cameraRight = Math::Vector3::cross(cameraDirection, up).normalized();
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(position.x, position.y, position.z,
                  target.x, target.y, target.z,
                  up.x, up.y, up.z);
    }
    
    // ========================================================================
    // Lighting
    // ========================================================================
    void setAmbientLight(const Math::Color& color) {
        GLfloat ambient[] = { color.r, color.g, color.b, color.a };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    }
    
    void setDirectionalLight(const DirectionalLight& light) {
        GLenum lightEnum = GL_LIGHT0;
        glEnable(lightEnum);
        
        Math::Color scaledColor = Math::Color(
            light.color.r * light.intensity,
            light.color.g * light.intensity,
            light.color.b * light.intensity
        );
        
        GLfloat ambient[] = { scaledColor.r * 0.2f, scaledColor.g * 0.2f, scaledColor.b * 0.2f, 1.0f };
        GLfloat diffuse[] = { scaledColor.r, scaledColor.g, scaledColor.b, 1.0f };
        GLfloat specular[] = { scaledColor.r, scaledColor.g, scaledColor.b, 1.0f };
        GLfloat pos[] = { -light.direction.x, -light.direction.y, -light.direction.z, 0.0f };
        
        glLightfv(lightEnum, GL_AMBIENT, ambient);
        glLightfv(lightEnum, GL_DIFFUSE, diffuse);
        glLightfv(lightEnum, GL_SPECULAR, specular);
        glLightfv(lightEnum, GL_POSITION, pos);
        glLightf(lightEnum, GL_SPOT_CUTOFF, 180.0f);
    }
    
    void setLight(int index, const Light& light) {
        if (index < 0 || index >= 8) return;
        
        GLenum lightEnum = GL_LIGHT0 + index;
        
        if (!light.enabled) {
            glDisable(lightEnum);
            return;
        }
        
        glEnable(lightEnum);
        
        GLfloat ambient[] = { light.ambient.r, light.ambient.g, light.ambient.b, 1.0f };
        GLfloat diffuse[] = { light.diffuse.r, light.diffuse.g, light.diffuse.b, 1.0f };
        GLfloat specular[] = { light.specular.r, light.specular.g, light.specular.b, 1.0f };
        
        glLightfv(lightEnum, GL_AMBIENT, ambient);
        glLightfv(lightEnum, GL_DIFFUSE, diffuse);
        glLightfv(lightEnum, GL_SPECULAR, specular);
        
        if (light.type == LightType::Directional) {
            GLfloat pos[] = { -light.direction.x, -light.direction.y, -light.direction.z, 0.0f };
            glLightfv(lightEnum, GL_POSITION, pos);
            glLightf(lightEnum, GL_SPOT_CUTOFF, 180.0f);
        }
        else {
            GLfloat pos[] = { light.position.x, light.position.y, light.position.z, 1.0f };
            glLightfv(lightEnum, GL_POSITION, pos);
            
            glLightf(lightEnum, GL_CONSTANT_ATTENUATION, light.constantAttenuation);
            glLightf(lightEnum, GL_LINEAR_ATTENUATION, light.linearAttenuation);
            glLightf(lightEnum, GL_QUADRATIC_ATTENUATION, light.quadraticAttenuation);
            
            if (light.type == LightType::Spot) {
                GLfloat dir[] = { light.direction.x, light.direction.y, light.direction.z };
                glLightfv(lightEnum, GL_SPOT_DIRECTION, dir);
                glLightf(lightEnum, GL_SPOT_CUTOFF, light.spotCutoff);
                glLightf(lightEnum, GL_SPOT_EXPONENT, light.spotExponent);
            }
            else {
                glLightf(lightEnum, GL_SPOT_CUTOFF, 180.0f);
            }
        }
    }
    
    void disableLight(int index) {
        if (index >= 0 && index < 8) {
            glDisable(GL_LIGHT0 + index);
        }
    }
    
    void enableLighting(bool enable) {
        if (enable) glEnable(GL_LIGHTING);
        else glDisable(GL_LIGHTING);
    }
    
    // ========================================================================
    // Fog
    // ========================================================================
    void setFog(bool enable, const Math::Color& color = Math::Color(0.1f, 0.1f, 0.15f), 
                float start = 10.0f, float end = 80.0f) {
        fogEnabled = enable;
        fogColor = color;
        fogStart = start;
        fogEnd = end;
        
        if (enable) {
            glEnable(GL_FOG);
            glFogi(GL_FOG_MODE, GL_LINEAR);
            GLfloat fc[] = { color.r, color.g, color.b, color.a };
            glFogfv(GL_FOG_COLOR, fc);
            glFogf(GL_FOG_START, start);
            glFogf(GL_FOG_END, end);
        }
        else {
            glDisable(GL_FOG);
        }
    }
    
    void setFogEnabled(bool enable) {
        fogEnabled = enable;
        if (enable) {
            glEnable(GL_FOG);
        } else {
            glDisable(GL_FOG);
        }
    }
    
    void setFogParams(const Math::Color& color, float start, float end) {
        fogColor = color;
        fogStart = start;
        fogEnd = end;
        
        glFogi(GL_FOG_MODE, GL_LINEAR);
        GLfloat fc[] = { color.r, color.g, color.b, color.a };
        glFogfv(GL_FOG_COLOR, fc);
        glFogf(GL_FOG_START, start);
        glFogf(GL_FOG_END, end);
    }
    
    // ========================================================================
    // Mesh Drawing
    // ========================================================================
    void drawMesh(Mesh* mesh, const Math::Transform& transform) {
        if (!mesh) return;
        
        glPushMatrix();
        transform.applyToGL();
        mesh->draw();
        glPopMatrix();
    }
    
    void drawMesh(Mesh* mesh, const Math::Vector3& position, float rotationY = 0.0f, 
                  const Math::Vector3& scale = Math::Vector3::one()) {
        if (!mesh) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotationY, 0, 1, 0);
        glScalef(scale.x, scale.y, scale.z);
        mesh->draw();
        glPopMatrix();
    }
    
    void drawMeshWithTexture(Mesh* mesh, unsigned int textureId, 
                             const Math::Vector3& position, float rotationY = 0.0f,
                             const Math::Vector3& scale = Math::Vector3::one()) {
        if (!mesh) return;
        
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotationY, 0, 1, 0);
        glScalef(scale.x, scale.y, scale.z);
        mesh->drawWithTexture(textureId);
        glPopMatrix();
    }
    
    // ========================================================================
    // Primitive Drawing
    // ========================================================================
    void drawCube(const Math::Vector3& position, const Math::Vector3& size, const Math::Color& color) {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glScalef(size.x, size.y, size.z);
        
        glDisable(GL_TEXTURE_2D);
        color.apply();
        glutSolidCube(1.0f);
        
        glPopMatrix();
    }
    
    void drawSphere(const Math::Vector3& position, float radius, const Math::Color& color) {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        
        glDisable(GL_TEXTURE_2D);
        color.apply();
        glutSolidSphere(radius, 16, 16);
        
        glPopMatrix();
    }
    
    void drawFloor(float size, float y, const Math::Color& color, unsigned int textureId = 0) {
        float halfSize = size * 0.5f;
        
        if (textureId > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glColor3f(1, 1, 1);
        }
        else {
            glDisable(GL_TEXTURE_2D);
            color.apply();
        }
        
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        
        float tileSize = 4.0f;
        float uvScale = size / tileSize;
        
        glTexCoord2f(0, 0);
        glVertex3f(-halfSize, y, -halfSize);
        
        glTexCoord2f(uvScale, 0);
        glVertex3f(halfSize, y, -halfSize);
        
        glTexCoord2f(uvScale, uvScale);
        glVertex3f(halfSize, y, halfSize);
        
        glTexCoord2f(0, uvScale);
        glVertex3f(-halfSize, y, halfSize);
        
        glEnd();
    }
    
    // ========================================================================
    // Debug Drawing
    // ========================================================================
    void drawLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Color& color, float width = 2.0f) {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(width);
        
        color.apply();
        
        glBegin(GL_LINES);
        glVertex3f(start.x, start.y, start.z);
        glVertex3f(end.x, end.y, end.z);
        glEnd();
        
        glEnable(GL_LIGHTING);
    }
    
    void drawRay(const Math::Vector3& origin, const Math::Vector3& direction, float length, 
                 const Math::Color& color, float width = 2.0f) {
        drawLine(origin, origin + direction * length, color, width);
    }
    
    void drawAABB(const Math::AABB& aabb, const Math::Color& color, float lineWidth = 1.0f) {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(lineWidth);
        color.apply();
        
        Math::Vector3 corners[8] = {
            Math::Vector3(aabb.min.x, aabb.min.y, aabb.min.z),
            Math::Vector3(aabb.max.x, aabb.min.y, aabb.min.z),
            Math::Vector3(aabb.max.x, aabb.max.y, aabb.min.z),
            Math::Vector3(aabb.min.x, aabb.max.y, aabb.min.z),
            Math::Vector3(aabb.min.x, aabb.min.y, aabb.max.z),
            Math::Vector3(aabb.max.x, aabb.min.y, aabb.max.z),
            Math::Vector3(aabb.max.x, aabb.max.y, aabb.max.z),
            Math::Vector3(aabb.min.x, aabb.max.y, aabb.max.z)
        };
        
        glBegin(GL_LINES);
        // Bottom face
        for (int i = 0; i < 4; ++i) {
            glVertex3f(corners[i].x, corners[i].y, corners[i].z);
            glVertex3f(corners[(i+1)%4].x, corners[(i+1)%4].y, corners[(i+1)%4].z);
        }
        // Top face
        for (int i = 0; i < 4; ++i) {
            glVertex3f(corners[4+i].x, corners[4+i].y, corners[4+i].z);
            glVertex3f(corners[4+(i+1)%4].x, corners[4+(i+1)%4].y, corners[4+(i+1)%4].z);
        }
        // Vertical edges
        for (int i = 0; i < 4; ++i) {
            glVertex3f(corners[i].x, corners[i].y, corners[i].z);
            glVertex3f(corners[4+i].x, corners[4+i].y, corners[4+i].z);
        }
        glEnd();
        
        glEnable(GL_LIGHTING);
    }
    
    void drawCrosshair(const Math::Color& color = Math::Color::white(), float size = 10.0f) {
        // Switch to 2D overlay mode
        begin2D();
        
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        color.apply();
        
        float cx = screenWidth * 0.5f;
        float cy = screenHeight * 0.5f;
        
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        // Horizontal
        glVertex2f(cx - size, cy);
        glVertex2f(cx - size/3, cy);
        glVertex2f(cx + size/3, cy);
        glVertex2f(cx + size, cy);
        // Vertical
        glVertex2f(cx, cy - size);
        glVertex2f(cx, cy - size/3);
        glVertex2f(cx, cy + size/3);
        glVertex2f(cx, cy + size);
        glEnd();
        
        // Center dot
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        glVertex2f(cx, cy);
        glEnd();
        
        end2D();
    }
    
    // ========================================================================
    // Screen Effects
    // ========================================================================
    void flashScreen(const Math::Color& color, float intensity = 0.5f) {
        screenFlashColor = color;
        screenFlashIntensity = intensity;
    }
    
    void updateScreenEffects(float deltaTime) {
        if (screenFlashIntensity > 0) {
            screenFlashIntensity -= deltaTime * 4.0f; // Fade out
            if (screenFlashIntensity < 0) screenFlashIntensity = 0;
        }
    }
    
    // ========================================================================
    // 2D Overlay Mode
    // ========================================================================
    void begin2D() {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, screenWidth, 0, screenHeight);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
    }
    
    void end2D() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    void drawText(const std::string& text, float x, float y, const Math::Color& color = Math::Color::white()) {
        glDisable(GL_TEXTURE_2D);
        color.apply();
        glRasterPos2f(x, y);
        for (char c : text) {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
        }
    }
    
    void drawTextLarge(const std::string& text, float x, float y, const Math::Color& color = Math::Color::white()) {
        glDisable(GL_TEXTURE_2D);
        color.apply();
        glRasterPos2f(x, y);
        for (char c : text) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }
    
    void drawRect(float x, float y, float width, float height, const Math::Color& color) {
        glDisable(GL_TEXTURE_2D);
        color.apply();
        
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();
    }
    
    // ========================================================================
    // FPS Weapon View (Overlay)
    // ========================================================================
    void beginWeaponView() {
        // Clear depth buffer so weapon is always on top
        glClear(GL_DEPTH_BUFFER_BIT);
        
        // Use a separate projection for the weapon (no far clipping issues)
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        float aspect = (float)screenWidth / (float)screenHeight;
        gluPerspective(fov, aspect, 0.01f, 10.0f);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    
    void endWeaponView() {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        
        glMatrixMode(GL_MODELVIEW);
    }
    
    // Getters
    Math::Vector3 getCameraPosition() const { return cameraPosition; }
    Math::Vector3 getCameraDirection() const { return cameraDirection; }
    Math::Vector3 getCameraRight() const { return cameraRight; }
    Math::Vector3 getCameraUp() const { return cameraUp; }
    int getScreenWidth() const { return screenWidth; }
    int getScreenHeight() const { return screenHeight; }
    
    // Setters
    void setFOV(float fovDegrees) { fov = fovDegrees; setupProjection(); }
    void setNearFar(float near, float far) { nearPlane = near; farPlane = far; setupProjection(); }
    
private:
    Renderer() 
        : screenWidth(1280)
        , screenHeight(720)
        , fov(60.0f)
        , nearPlane(0.1f)
        , farPlane(500.0f)
        , screenFlashIntensity(0)
        , screenFlashColor(Math::Color::red())
        , fogEnabled(false)
        , fogStart(10.0f)
        , fogEnd(80.0f)
        , clearColor(0.02f, 0.02f, 0.05f)
    {}
    
    void drawScreenFlash() {
        begin2D();
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        Math::Color flashColor = screenFlashColor;
        flashColor.a = screenFlashIntensity * 0.5f;
        
        drawRect(0, 0, (float)screenWidth, (float)screenHeight, flashColor);
        
        end2D();
    }
    
    int screenWidth;
    int screenHeight;
    float fov;
    float nearPlane;
    float farPlane;
    
    Math::Vector3 cameraPosition;
    Math::Vector3 cameraTarget;
    Math::Vector3 cameraDirection;
    Math::Vector3 cameraRight;
    Math::Vector3 cameraUp;
    
    float screenFlashIntensity;
    Math::Color screenFlashColor;
    
    bool fogEnabled;
    Math::Color fogColor;
    float fogStart;
    float fogEnd;
    Math::Color clearColor;
};

} // namespace Doomers
