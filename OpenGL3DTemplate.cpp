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



const float cutDiff = 15;

// rotation for testing
float rotAng = 0.0f;


// choose these as globals or consts
float SCALE_CORRIDOR = 4.0f / 130.8f;   // ~ 0.03
float SCALE_CRATE = 1.0f / 112.0f;   // ~ 0.009
float SCALE_GATE = 3.0f / 15.0f;    // ~ 0.20
float SCALE_GUN = 0.3f / 37.5f;    // ~ 0.008 (gun appears ~30 cm tall on screen)
float SCALE_HEALTH = 0.4f / 0.37f;    // ~ 1.1  (slightly bigger than source)
float SCALE_AMMO = 0.4f / 1.40f;    // ~ 0.28

// for army man / zombie (we dont have logs yet, but you can pick)
float SCALE_PLAYER = 0.01f;
float SCALE_ZOMBIE = 0.01f;



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
    float x, y, z;
    float sx, sy, sz;
    float ry;                 
    Mesh* mesh = nullptr;
    Model* model = nullptr;
    unsigned int texId = 0;

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
                glDisable(GL_TEXTURE_2D);
                glColor3f(0.7f, 0.7f, 0.7f);
                mesh->draw(false);
            }
        }
        glPopMatrix();
    }
};






std::vector<GameObject> corridorSegments;
std::vector<GameObject> crates;
std::vector<GameObject> pickups;   // health + ammo
std::vector<GameObject> enemies;   // zombies
GameObject playerVisual;           // for TPS soldier model



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
    float moveStep = 1.0f;

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
    float angleStep = 10.0f;

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

void drawCorridorWithClip(const GameObject& c, double localCutX) {
    glPushMatrix();

    glTranslatef(c.x, c.y, c.z);
    glRotatef(c.ry, 0, 1, 0);
    glScalef(c.sx, c.sy, c.sz);

    GLdouble eq[4] = { -1.0, 0.0, 0.0, localCutX }; // keep x <= localCutX

    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, eq);

    if (c.texId) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, c.texId);
        glColor3f(1, 1, 1);
        c.mesh->draw(true);
        glDisable(GL_TEXTURE_2D);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.7f, 0.7f, 0.7f);
        c.mesh->draw(false);
    }

    glDisable(GL_CLIP_PLANE0);
    glPopMatrix();
}




void Display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1) World (uses camera)
    applyCamera();

    double minX = corridorMesh.minX;
    double maxX = corridorMesh.maxX;

    // where we cut the end cap in local space
    double cutX = maxX - cutDiff;  // or whatever margin you liked

    // this is the local length we KEEP (from minX up to cutX)
    double keptLenLocal = cutX - minX;
    for (auto& c : corridorSegments) {
        drawCorridorWithClip(c, cutX);
    }



    //gateObj.draw();
    //for (auto& c : crates)  c.draw();
    //for (auto& p : pickups) p.draw();
    for (auto& e : enemies) e.draw();

    if (viewMode == VIEW_TPS) {
        playerVisual.x = playerX;
        playerVisual.y = playerY;
        playerVisual.z = playerZ;
        playerVisual.ry = playerYaw;
        playerVisual.draw();
    }

    // 2) Gun as view-model (FPS only)
    if (viewMode == VIEW_FPS) {
        glPushMatrix();

        // reset modelview so we are in camera space
        glLoadIdentity();

        // small offset from camera: right, down, forward
        glTranslatef(0.3f, -0.3f, -0.8f);
        glRotatef(5.0f, 0, 1, 0);     // tiny tilt if you like

        glScalef(SCALE_GUN, SCALE_GUN, SCALE_GUN);
        drawTextured(gunMesh, gunTexture);

        glPopMatrix();
    }

   /* crates[0].sx = crates[0].sy = crates[0].sz = 0.05f;
    crates[0].x = 0.0f;
    crates[0].z = -5.0f;*/


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


    

    // Gate at end of corridor, ~25 units away
   /* gateObj = {
        0.0f, 0.0f, -25.0f,
        SCALE_GATE, SCALE_GATE, SCALE_GATE,
        0.0f,
        &gateMesh, nullptr,
        0
    };*/

    // Two crates as cover
    crates.push_back({
        -2.0f, 0.0f, -10.0f,
        SCALE_CRATE, SCALE_CRATE, SCALE_CRATE,
        0.0f,
        &crateMesh, nullptr,
        crateTexture
        });

    crates.push_back({
         2.0f, 0.0f, -12.0f,
         SCALE_CRATE, SCALE_CRATE, SCALE_CRATE,
         15.0f,
         &crateMesh, nullptr,
         crateTexture
        });

    // Ammo on top of second crate
    pickups.push_back({
        2.0f, 1.0f, -12.0f,           // y ≈ 1 so it sits on crate
        SCALE_AMMO, SCALE_AMMO, SCALE_AMMO,
        0.0f,
        &ammoMesh, nullptr,
        ammoTexture
        });

    // Health on the ground near first crate
    pickups.push_back({
        -1.5f, 0.0f, -11.5f,
        SCALE_HEALTH, SCALE_HEALTH, SCALE_HEALTH,
        0.0f,
        &healthMesh, nullptr,
        healthTexture
        });

    // One zombie in the corridor
    enemies.push_back({
        0.5f, 0.0f, -18.0f,
        SCALE_ZOMBIE, SCALE_ZOMBIE, SCALE_ZOMBIE,
        180.0f,               // facing player
        nullptr, &zombieModel,
        0
        });

    // TPS player visual
    playerVisual = {
        playerX, playerY, playerZ,
        SCALE_PLAYER, SCALE_PLAYER, SCALE_PLAYER,
        playerYaw,
        nullptr, &playerModel,
        0
    };

    // keep height ~ 4 units
    const float RAW_CORRIDOR_HEIGHT = 130.847f;
    const float RAW_CORRIDOR_LENGTH = 600.581f; // along X
    const float SCALE_CORRIDOR = 4.0f / RAW_CORRIDOR_HEIGHT; // ~ 0.03

    // after scaling, corridor world length:
    const float corridorLenWorld = RAW_CORRIDOR_LENGTH * SCALE_CORRIDOR;


    double minX = corridorMesh.minX;
    double maxX = corridorMesh.maxX;

    // where we cut the end cap in local space
    double cutX = maxX - 2*cutDiff;  // or whatever margin you liked

    // this is the local length we KEEP (from minX up to cutX)
    double keptLenLocal = cutX - minX;


    float stepWorld = (float)(keptLenLocal * SCALE_CORRIDOR);  // distance between segments in world

    // base segment (rotated so the long X axis becomes Z)
    corridorSegments.clear();

    // base segment
    GameObject c0;
    c0.x = 0.0f;
    c0.y = 0.0f;
    c0.z = 0.0f;
    c0.sx = SCALE_CORRIDOR;
    c0.sy = SCALE_CORRIDOR;
    c0.sz = SCALE_CORRIDOR;
    c0.ry = 90.0f;                 // as you had
    c0.mesh = &corridorMesh;
    c0.model = nullptr;
    c0.texId = corridorTexture;

    corridorSegments.push_back(c0);

    // second and third segments, using kept length as step
    for (int i = 1; i < 3; ++i) {
        GameObject ci = c0;
        ci.z = -i * stepWorld;     // minus because corridor goes “forward” in -Z for you
        corridorSegments.push_back(ci);
    }


    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutSpecialFunc(SpecialKeys);

    glutMainLoop();
}
