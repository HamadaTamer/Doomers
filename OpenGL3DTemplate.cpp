#include "Mesh.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glut.h>

float rotAng;
Mesh gunMesh;
unsigned int gunTexture = 0;

unsigned int loadTexture(const char* filename) {
    int w, h, ch;
    unsigned char* data = stbi_load(filename, &w, &h, &ch, 0);
    if (!data) {
        printf("Failed to load texture: %s\n", filename);
        return 0;
    }

    GLenum format = GL_RGB;
    if (ch == 4) format = GL_RGBA;

    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexImage2D(GL_TEXTURE_2D, 0, format,
        w, h, 0,
        format, GL_UNSIGNED_BYTE, data);

    // NO glGenerateMipmap here

    // simple filtering (no mipmaps)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // optional: wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);
    return texID;
}


void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw textured gun
    glPushMatrix();
    glRotatef(rotAng, 0, 1, 0);
    glScalef(0.05f, 0.05f, 0.05f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gunTexture);
    glColor3f(1.0f, 1.0f, 1.0f); // important: white so texture not tinted
    gunMesh.draw(true);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    // you can still draw other stuff if you want
    /*
    glPushMatrix();
    glRotatef(-rotAng, 0, 1, 0);
    glTranslatef(2, 0, 0);
    glRotatef(rotAng, 1, 0, 0);
    glColor3f(0.5f, 0.5f, 0.5f);
    glutSolidSphere(0.5, 25, 25);
    glPopMatrix();
    */

    glFlush();
}

void Anim() {
    rotAng += 0.01f;
    glutPostRedisplay();
}

void main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitWindowSize(300, 300);
    glutInitWindowPosition(150, 150);

    glutCreateWindow("OpenGL - 3D Template");
    glutDisplayFunc(Display);
    glutIdleFunc(Anim);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); // helpful for scaled models

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 300.0f / 300.0f, 0.1f, 300.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f, 2.0f, 5.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f);

    // load gun mesh
    gunMesh = loadOBJ("assets/AR/source/083412fa5dba4c75a3bdc3bc77dd0ed5/Gun.obj");

    // load albedo texture
    gunTexture = loadTexture("assets/AR/textures/GAP_Examen_Gun_albedo_DriesDeryckere.tga.png");

    glutMainLoop();
}
