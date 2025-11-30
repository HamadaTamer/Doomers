#include "Mesh.hpp"
#include "Model.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cmath>
#include <glut.h>

enum ViewMode { VIEW_FPS, VIEW_TPS };

ViewMode viewMode = VIEW_FPS;

// player position + orientation
float playerX = 0.0f;
float playerY = 0.0f;   // feet on ground
float playerZ = 0.0f;
float playerYaw = 0.0f; // degrees

// camera pitch (for looking up/down)
float camPitch = 0.0f;




// rotation for testing
float rotAng = 0.0f;

// Meshes
Mesh gunMesh;
Mesh crateMesh;
Mesh healthMesh;
Mesh ammoMesh;
Mesh corridorMesh;
Mesh gateMesh;

// Textures
unsigned int gunTexture = 0;
unsigned int crateTexture = 0;
unsigned int healthTexture = 0;
unsigned int ammoTexture = 0;
unsigned int soldierTexture = 0;
unsigned int zombieTexture = 0;
unsigned int corridorTexture = 0;


Model soldierModel;
Model playerModel;
Model zombieModel;



struct GameObject {
    enum Type { CRATE, AMMO, HEALTH, ENEMY, PLAYER, GATE } type;
    float x, y, z;
    float sx, sy, sz;
    float ry;        // rotation around Y
    Mesh* mesh;     // for corridor, crates, etc.
    Model* model;    // for soldier, zombie
    unsigned int texId;

    void draw() const {
        glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(ry, 0, 1, 0);
        glScalef(sx, sy, sz);

        if (model) {
            model->draw();
        }
        else if (mesh) {
            if (texId) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, texId);
                glColor3f(1, 1, 1);
                mesh->draw(true);
                glDisable(GL_TEXTURE_2D);
            }
            else {
                glColor3f(0.7f, 0.7f, 0.7f);
                mesh->draw(false);
            }
        }
        glPopMatrix();
    }
};


std::vector<GameObject> crates;
std::vector<GameObject> pickups;  // ammo + health
std::vector<GameObject> enemies;  // zombies
GameObject playerObject;


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

void drawTextured(const Mesh& mesh, unsigned int texId, bool useTex = true) {
    if (texId && useTex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texId);
        glColor3f(1.0f, 1.0f, 1.0f); // so texture not tinted
        mesh.draw(true);
        glDisable(GL_TEXTURE_2D);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.7f, 0.7f, 0.7f);
        mesh.draw(false);
    }
}

void Keyboard(unsigned char key, int x, int y) {
    float moveStep = 0.2f;

    float yawRad = playerYaw * 3.14159265f / 180.0f;
    float forwardX = sinf(yawRad);
    float forwardZ = -cosf(yawRad);
    float rightX = cosf(yawRad);
    float rightZ = sinf(yawRad);

    switch (key) {
    case 'w':
    case 'W':
        playerX += forwardX * moveStep;
        playerZ += forwardZ * moveStep;
        break;
    case 's':
    case 'S':
        playerX -= forwardX * moveStep;
        playerZ -= forwardZ * moveStep;
        break;
    case 'a':
    case 'A':
        playerX -= rightX * moveStep;
        playerZ -= rightZ * moveStep;
        break;
    case 'd':
    case 'D':
        playerX += rightX * moveStep;
        playerZ += rightZ * moveStep;
        break;
    case 27: // ESC
        exit(0);
        break;
    }

    glutPostRedisplay();
}


void SpecialKeys(int key, int x, int y) {
    float angleStep = 2.0f;

    switch (key) {
    case GLUT_KEY_LEFT:
        playerYaw -= angleStep;
        break;
    case GLUT_KEY_RIGHT:
        playerYaw += angleStep;
        break;
    case GLUT_KEY_UP:
        camPitch += angleStep;
        if (camPitch > 89.0f) camPitch = 89.0f;
        break;
    case GLUT_KEY_DOWN:
        camPitch -= angleStep;
        if (camPitch < -89.0f) camPitch = -89.0f;
        break;
    }

    glutPostRedisplay();
}


void Mouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return;

    if (button == GLUT_RIGHT_BUTTON) {
        if (viewMode == VIEW_FPS) viewMode = VIEW_TPS;
        else                      viewMode = VIEW_FPS;
    }

    if (button == GLUT_LEFT_BUTTON) {
        // TODO: shooting logic
    }

    glutPostRedisplay();
}


void applyCamera() {
    float eyeHeight = 1.7f;     // player eye level
    float yawRad = playerYaw * 3.14159265f / 180.0f;
    float pitchRad = camPitch * 3.14159265f / 180.0f;

    // forward direction from yaw + pitch
    float dirX = cosf(pitchRad) * sinf(yawRad);
    float dirY = sinf(pitchRad);
    float dirZ = -cosf(pitchRad) * cosf(yawRad);

    float camX, camY, camZ;
    float targetX, targetY, targetZ;

    if (viewMode == VIEW_FPS) {
        // camera at player eyes
        camX = playerX;
        camY = playerY + eyeHeight;
        camZ = playerZ;

        targetX = camX + dirX;
        targetY = camY + dirY;
        targetZ = camZ + dirZ;
    }
    else { // VIEW_TPS
        float distBehind = 4.0f;
        float height = 2.0f;

        // place camera behind player along -forward
        camX = playerX - sinf(yawRad) * distBehind;
        camZ = playerZ + cosf(yawRad) * distBehind;
        camY = playerY + height;

        targetX = playerX;
        targetY = playerY + eyeHeight;
        targetZ = playerZ;
    }

    glLoadIdentity();
    gluLookAt(camX, camY, camZ,
        targetX, targetY, targetZ,
        0.0f, 1.0f, 0.0f);
}



void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    applyCamera();  

    gluLookAt(0.0f, 2.0f, 6.0f,
        0.0f, 1.5f, 0.0f,
        0.0f, 1.0f, 0.0f);

    //// 1) Corridor (environment base)
    //glPushMatrix();
    //glScalef(0.05f, 0.05f, 0.05f);            // corridor is usually huge
    //glRotatef(rotAng * 10.0f, 0, 1, 0);   // use rotAng here
    //drawTextured(corridorMesh, corridorTexture);
    //glPopMatrix();

    // 2) Gate at the end of the corridor
    //glPushMatrix();
    //glTranslatef(0.0f, 0.0f, -5.0f);          // move along -Z
    //glScalef(0.01f, 0.01f, 0.01f);
    //glColor3f(0.3f, 0.6f, 0.9f);              // sci-fi bluish gate
    //gateMesh.draw(false);
    //glPopMatrix();

    // 3) A couple of crates as obstacles
    glPushMatrix();
    glTranslatef(-1.5f, 0.0f, -2.0f);
    glScalef(0.005f, 0.005f, 0.005f);
    drawTextured(crateMesh, crateTexture);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.5f, 0.0f, -3.5f);
    glScalef(0.01f, 0.01f, 0.01f);
    drawTextured(crateMesh, crateTexture);
    glPopMatrix();

    // 4) Ammo box on top of a crate
    glPushMatrix();
    glTranslatef(1.5f, 0.5f, -3.5f);
    glScalef(0.01f, 0.01f, 0.01f);
    drawTextured(ammoMesh, ammoTexture);
    glPopMatrix();

    // 5) Health pack somewhere on the floor
    glPushMatrix();
    glTranslatef(-1.0f, 0.0f, -4.5f);
    glScalef(0.01f, 0.01f, 0.01f);
    drawTextured(healthMesh, healthTexture);
    glPopMatrix();

    // 6) Zombie enemy in the corridor
    // Player (third-person placement)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 2.0f);   // tweak
    glScalef(0.015f, 0.015f, 0.015f);     // tweak scale until it looks right
    playerModel.draw();
    glPopMatrix();

    // Zombie enemy
    glPushMatrix();
    glTranslatef(1.0f, 0.0f, 5.0f);   // in the corridor
    glScalef(0.015f, 0.015f, 0.015f);
    glRotatef(180.0f, 0, 1, 0);        // face player
    zombieModel.draw();
    glPopMatrix();


    // 7) Player soldier (for third-person view, just test placement)
   /* glPushMatrix();
    glTranslatef(0.0f, 0.0f, -3.0f);
    glScalef(0.01f, 0.01f, 0.01f);
    soldierModel.draw();
    glPopMatrix();*/


    // 8) Gun in front of camera (like FPS weapon)
    glPushMatrix();
    // place it slightly in front/right/down of camera
    glTranslatef(0.4f, -0.6f, 1.0f);
    glRotatef(rotAng*-10.0f, 0, 1, 0);
    glScalef(0.01f, 0.01f, 0.01f);
    drawTextured(gunMesh, gunTexture);
    glPopMatrix();

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
    /*glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);*/

    GLfloat lightPos[] = { 0.0f, 5.0f, 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 300.0f / 300.0f, 0.1f, 300.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f, 2.0f, 5.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f);

    // load meshes
    gunMesh = loadOBJ("assets/AR/source/083412fa5dba4c75a3bdc3bc77dd0ed5/Gun.obj");
    crateMesh = loadOBJ("assets/gart130-crate/source/L_Crate_2fbx.obj");
    healthMesh = loadOBJ("assets/health-pack/source/HealthPack/Healthpack Textured.Obj");
    ammoMesh = loadOBJ("assets/sci-fi-ammo-box/source/Box_final/Box_final.obj");
    corridorMesh = loadOBJ("assets/sci-fi-corridor-texturing-challenge/source/sci-fi-corridor-texturing-challenge-model/corridor.obj");
    gateMesh = loadOBJ("assets/sci-fi-gate/source/sci fi gate/sci fi gate.obj");

    // load textures
    gunTexture = loadTexture("assets/AR/textures/GAP_Examen_Gun_albedo_DriesDeryckere.tga.png");
    crateTexture = loadTexture("assets/gart130-crate/textures/L_Crate.2fbx_lambert5_BaseColor.png");
    healthTexture = loadTexture("assets/health-pack/textures/Healthpack Textured_Albedo.png");
    ammoTexture = loadTexture("assets/sci-fi-ammo-box/textures/BOX_full_albedo.png");
    corridorTexture = loadTexture("assets/sci-fi-corridor-texturing-challenge/textures/scene_1001_BaseColor.png");

    soldierModel = loadOBJWithMTL(
        "assets/Soldier/Soldier.obj",          // adjust to your real path
        "assets/Soldier"                       // base dir where Soldier.mtl + _Body_Low.png live
    );

    playerModel = loadOBJWithMTL(
        "assets/military-man-army-man-soldier/source/Army man/Army man.obj",
        "assets/military-man-army-man-soldier/source/Army man"
    );

    zombieModel = loadOBJWithMTL(
        "assets/zombie/source/obj/obj/Zombie001.obj",
        "assets/zombie/source/obj/obj"
    );


    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutSpecialFunc(SpecialKeys);

    glutMainLoop();
}
