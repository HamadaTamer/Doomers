// ============================================================================
// DOOMERS - ModelUtils.h
// Utility functions for procedural model rendering
// ============================================================================
#ifndef MODEL_UTILS_H
#define MODEL_UTILS_H

#include "../Vector3.h"
#include "../GameConfig.h"
#include <glut.h>
#include <math.h>

namespace ModelUtils {

    // Global animation time
    static float globalTime = 0.0f;
    
    inline void updateTime(float dt) { 
        globalTime += dt; 
    }
    
    inline float getTime() {
        return globalTime;
    }

    // ==================== MATERIAL FUNCTIONS ====================
    inline void setColor(float r, float g, float b, float a = 1.0f) {
        float ambient[] = {r * 0.4f, g * 0.4f, b * 0.4f, a};
        float diffuse[] = {r, g, b, a};
        float specular[] = {0.3f, 0.3f, 0.3f, a};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30.0f);
        glColor4f(r, g, b, a);
    }

    inline void setColorMetallic(float r, float g, float b) {
        float ambient[] = {r * 0.3f, g * 0.3f, b * 0.3f, 1.0f};
        float diffuse[] = {r, g, b, 1.0f};
        float specular[] = {0.8f, 0.8f, 0.8f, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 80.0f);
        glColor3f(r, g, b);
    }

    inline void setEmissive(float r, float g, float b) {
        float emission[] = {r, g, b, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    }

    inline void clearEmissive() {
        float emission[] = {0, 0, 0, 1.0f};
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    }

    // ==================== PRIMITIVE SHAPES ====================
    inline void drawCube(float size) { 
        glutSolidCube(size); 
    }

    inline void drawBox(float sx, float sy, float sz) {
        glPushMatrix();
        glScalef(sx, sy, sz);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    inline void drawCylinder(float radius, float height, int slices = 16) {
        GLUquadric* quad = gluNewQuadric();
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        gluCylinder(quad, radius, radius, height, slices, 1);
        // Cap the cylinder
        glPushMatrix();
        glTranslatef(0, 0, height);
        gluDisk(quad, 0, radius, slices, 1);
        glPopMatrix();
        glRotatef(180, 1, 0, 0);
        gluDisk(quad, 0, radius, slices, 1);
        glPopMatrix();
        gluDeleteQuadric(quad);
    }

    inline void drawCone(float radius, float height, int slices = 16) {
        GLUquadric* quad = gluNewQuadric();
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(radius, height, slices, 4);
        glPopMatrix();
        gluDeleteQuadric(quad);
    }

    inline void drawSphere(float radius, int slices = 16) {
        glutSolidSphere(radius, slices, slices);
    }

    inline void drawTorus(float innerRadius, float outerRadius, int sides = 16, int rings = 16) {
        glutSolidTorus(innerRadius, outerRadius, sides, rings);
    }

    // ==================== EFFECT HELPERS ====================
    inline void enableGlow() {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    inline void disableGlow() {
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    inline void enableTransparency() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    inline void disableTransparency() {
        glDisable(GL_BLEND);
    }
}

#endif // MODEL_UTILS_H
