/**
 * DOOMERS - Procedural Models
 * 
 * Procedural geometry generation for:
 * - Placeholders (before OBJ loading)
 * - Simple primitives
 * - Environmental objects
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include <GL/gl.h>
#include <cmath>

namespace Doomers {

// ============================================================================
// Procedural Model Drawing Functions
// ============================================================================
namespace ProceduralModels {

// Draw a unit cube centered at origin
inline void drawCube() {
    glBegin(GL_QUADS);
    // Front
    glNormal3f(0, 0, 1);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    
    // Back
    glNormal3f(0, 0, -1);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    
    // Top
    glNormal3f(0, 1, 0);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    
    // Bottom
    glNormal3f(0, -1, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    
    // Right
    glNormal3f(1, 0, 0);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    
    // Left
    glNormal3f(-1, 0, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd();
}

// Draw a cube with specified size
inline void drawBox(float width, float height, float depth) {
    glPushMatrix();
    glScalef(width, height, depth);
    drawCube();
    glPopMatrix();
}

// Draw a sphere
inline void drawSphere(float radius, int segments = 16) {
    for (int i = 0; i < segments; ++i) {
        float lat0 = Math::PI * (-0.5f + (float)i / segments);
        float lat1 = Math::PI * (-0.5f + (float)(i + 1) / segments);
        float y0 = sinf(lat0);
        float y1 = sinf(lat1);
        float r0 = cosf(lat0);
        float r1 = cosf(lat1);
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; ++j) {
            float lng = 2 * Math::PI * (float)j / segments;
            float x = cosf(lng);
            float z = sinf(lng);
            
            glNormal3f(x * r0, y0, z * r0);
            glVertex3f(radius * x * r0, radius * y0, radius * z * r0);
            
            glNormal3f(x * r1, y1, z * r1);
            glVertex3f(radius * x * r1, radius * y1, radius * z * r1);
        }
        glEnd();
    }
}

// Draw a cylinder
inline void drawCylinder(float radius, float height, int segments = 16) {
    float halfHeight = height * 0.5f;
    
    // Side
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * Math::PI * i / segments;
        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;
        
        glNormal3f(cosf(angle), 0, sinf(angle));
        glVertex3f(x, -halfHeight, z);
        glVertex3f(x, halfHeight, z);
    }
    glEnd();
    
    // Top cap
    glNormal3f(0, 1, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, halfHeight, 0);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * Math::PI * i / segments;
        glVertex3f(cosf(angle) * radius, halfHeight, sinf(angle) * radius);
    }
    glEnd();
    
    // Bottom cap
    glNormal3f(0, -1, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, -halfHeight, 0);
    for (int i = segments; i >= 0; --i) {
        float angle = 2.0f * Math::PI * i / segments;
        glVertex3f(cosf(angle) * radius, -halfHeight, sinf(angle) * radius);
    }
    glEnd();
}

// Draw a capsule (for characters)
inline void drawCapsule(float radius, float height, int segments = 12) {
    float cylinderHeight = height - 2 * radius;
    if (cylinderHeight < 0) cylinderHeight = 0;
    
    // Draw cylinder body
    glPushMatrix();
    drawCylinder(radius, cylinderHeight, segments);
    glPopMatrix();
    
    // Top hemisphere
    glPushMatrix();
    glTranslatef(0, cylinderHeight * 0.5f, 0);
    for (int i = 0; i < segments / 2; ++i) {
        float lat0 = Math::PI * (float)i / segments;
        float lat1 = Math::PI * (float)(i + 1) / segments;
        float y0 = cosf(lat0);
        float y1 = cosf(lat1);
        float r0 = sinf(lat0);
        float r1 = sinf(lat1);
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; ++j) {
            float lng = 2 * Math::PI * j / segments;
            float x = cosf(lng);
            float z = sinf(lng);
            
            glNormal3f(x * r0, y0, z * r0);
            glVertex3f(radius * x * r0, radius * y0, radius * z * r0);
            
            glNormal3f(x * r1, y1, z * r1);
            glVertex3f(radius * x * r1, radius * y1, radius * z * r1);
        }
        glEnd();
    }
    glPopMatrix();
    
    // Bottom hemisphere
    glPushMatrix();
    glTranslatef(0, -cylinderHeight * 0.5f, 0);
    for (int i = segments / 2; i < segments; ++i) {
        float lat0 = Math::PI * (float)i / segments;
        float lat1 = Math::PI * (float)(i + 1) / segments;
        float y0 = cosf(lat0);
        float y1 = cosf(lat1);
        float r0 = sinf(lat0);
        float r1 = sinf(lat1);
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; ++j) {
            float lng = 2 * Math::PI * j / segments;
            float x = cosf(lng);
            float z = sinf(lng);
            
            glNormal3f(x * r0, y0, z * r0);
            glVertex3f(radius * x * r0, radius * y0, radius * z * r0);
            
            glNormal3f(x * r1, y1, z * r1);
            glVertex3f(radius * x * r1, radius * y1, radius * z * r1);
        }
        glEnd();
    }
    glPopMatrix();
}

// Draw a cone
inline void drawCone(float radius, float height, int segments = 16) {
    // Base
    glNormal3f(0, -1, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, 0);
    for (int i = segments; i >= 0; --i) {
        float angle = 2.0f * Math::PI * i / segments;
        glVertex3f(cosf(angle) * radius, 0, sinf(angle) * radius);
    }
    glEnd();
    
    // Side
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);  // Simplified normal
    glVertex3f(0, height, 0);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * Math::PI * i / segments;
        float x = cosf(angle);
        float z = sinf(angle);
        // Calculate proper normal
        Math::Vector3 normal(x, radius / height, z);
        normal = normal.normalized();
        glNormal3f(normal.x, normal.y, normal.z);
        glVertex3f(x * radius, 0, z * radius);
    }
    glEnd();
}

// Draw a floor plane
inline void drawFloor(float size, int tiles = 10) {
    float tileSize = size / tiles;
    float halfSize = size * 0.5f;
    
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    
    for (int i = 0; i < tiles; ++i) {
        for (int j = 0; j < tiles; ++j) {
            float x = -halfSize + i * tileSize;
            float z = -halfSize + j * tileSize;
            
            // Checkerboard pattern
            if ((i + j) % 2 == 0) {
                glColor3f(0.4f, 0.4f, 0.4f);
            } else {
                glColor3f(0.3f, 0.3f, 0.3f);
            }
            
            glVertex3f(x, 0, z);
            glVertex3f(x + tileSize, 0, z);
            glVertex3f(x + tileSize, 0, z + tileSize);
            glVertex3f(x, 0, z + tileSize);
        }
    }
    glEnd();
}

// Draw a wall
inline void drawWall(float width, float height, float depth = 0.2f) {
    drawBox(width, height, depth);
}

// Draw a door frame
inline void drawDoorFrame(float width, float height, float depth) {
    float frameThickness = 0.1f;
    
    // Left frame
    glPushMatrix();
    glTranslatef(-width * 0.5f + frameThickness * 0.5f, 0, 0);
    drawBox(frameThickness, height, depth);
    glPopMatrix();
    
    // Right frame
    glPushMatrix();
    glTranslatef(width * 0.5f - frameThickness * 0.5f, 0, 0);
    drawBox(frameThickness, height, depth);
    glPopMatrix();
    
    // Top frame
    glPushMatrix();
    glTranslatef(0, height * 0.5f - frameThickness * 0.5f, 0);
    drawBox(width, frameThickness, depth);
    glPopMatrix();
}

// Draw crosshair
inline void drawCrosshair(float size = 0.02f, float gap = 0.005f) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glColor3f(1, 1, 1);
    glLineWidth(2.0f);
    
    glBegin(GL_LINES);
    // Horizontal
    glVertex2f(-size, 0);
    glVertex2f(-gap, 0);
    glVertex2f(gap, 0);
    glVertex2f(size, 0);
    
    // Vertical
    glVertex2f(0, -size);
    glVertex2f(0, -gap);
    glVertex2f(0, gap);
    glVertex2f(0, size);
    glEnd();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// Draw an arrow (useful for debugging)
inline void drawArrow(float length, float headSize = 0.2f) {
    // Shaft
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, -length);
    glEnd();
    
    // Head
    glPushMatrix();
    glTranslatef(0, 0, -length);
    glRotatef(180, 0, 1, 0);
    drawCone(headSize * 0.3f, headSize, 8);
    glPopMatrix();
}

// Draw axis helper
inline void drawAxes(float length = 1.0f) {
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    
    glBegin(GL_LINES);
    // X - Red
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(length, 0, 0);
    
    // Y - Green
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, length, 0);
    
    // Z - Blue
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, length);
    glEnd();
    
    glEnable(GL_LIGHTING);
}

// Draw a simple humanoid placeholder
inline void drawHumanoid(float height = 1.8f) {
    float scale = height / 1.8f;
    
    // Body
    glPushMatrix();
    glTranslatef(0, 1.0f * scale, 0);
    drawBox(0.4f * scale, 0.5f * scale, 0.2f * scale);
    glPopMatrix();
    
    // Head
    glPushMatrix();
    glTranslatef(0, 1.5f * scale, 0);
    drawSphere(0.15f * scale, 8);
    glPopMatrix();
    
    // Left arm
    glPushMatrix();
    glTranslatef(-0.3f * scale, 1.0f * scale, 0);
    drawBox(0.1f * scale, 0.4f * scale, 0.1f * scale);
    glPopMatrix();
    
    // Right arm
    glPushMatrix();
    glTranslatef(0.3f * scale, 1.0f * scale, 0);
    drawBox(0.1f * scale, 0.4f * scale, 0.1f * scale);
    glPopMatrix();
    
    // Left leg
    glPushMatrix();
    glTranslatef(-0.1f * scale, 0.35f * scale, 0);
    drawBox(0.12f * scale, 0.7f * scale, 0.12f * scale);
    glPopMatrix();
    
    // Right leg
    glPushMatrix();
    glTranslatef(0.1f * scale, 0.35f * scale, 0);
    drawBox(0.12f * scale, 0.7f * scale, 0.12f * scale);
    glPopMatrix();
}

// Draw a crate/box collectible
inline void drawCrate(float size = 0.5f) {
    glPushMatrix();
    glTranslatef(0, size * 0.5f, 0);
    drawCube();
    glScalef(size, size, size);
    glPopMatrix();
}

// Draw a health pack shape
inline void drawHealthPack(float size = 0.3f) {
    // White box with red cross
    glColor3f(1, 1, 1);
    
    glPushMatrix();
    glTranslatef(0, size * 0.5f, 0);
    glScalef(size, size * 0.3f, size);
    drawCube();
    glPopMatrix();
    
    // Red cross on top
    glColor3f(1, 0, 0);
    
    // Horizontal bar
    glPushMatrix();
    glTranslatef(0, size * 0.7f + 0.01f, 0);
    glScalef(size * 0.6f, 0.02f, size * 0.2f);
    drawCube();
    glPopMatrix();
    
    // Vertical bar
    glPushMatrix();
    glTranslatef(0, size * 0.7f + 0.01f, 0);
    glScalef(size * 0.2f, 0.02f, size * 0.6f);
    drawCube();
    glPopMatrix();
}

// Draw ammo box
inline void drawAmmoBox(float size = 0.25f) {
    glColor3f(0.3f, 0.4f, 0.2f);  // Military green
    
    glPushMatrix();
    glTranslatef(0, size * 0.5f, 0);
    glScalef(size * 1.5f, size, size);
    drawCube();
    glPopMatrix();
}

} // namespace ProceduralModels

} // namespace Doomers
