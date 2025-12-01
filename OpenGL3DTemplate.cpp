#include "Mesh.hpp"
#include "Model.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <cmath>
#include <glut.h>

enum ViewMode { VIEW_FPS, VIEW_TPS };

ViewMode viewMode = VIEW_FPS;

// player position + orientation
float playerX = 0.0f;
float playerY = 0.0f;   // feet on ground
float playerZ = 0.0f;
float playerYaw = 0.0f; // degrees

float playerVelY = 0.0f;
bool  isGrounded = true;

const float GRAVITY = -0.3f;  // tweak
const float JUMP_VELOCITY = 0.61f;   // tweak

// camera pitch (for looking up/down)
float camPitch = 0.0f;


const float MOVE_SPEED = 0.3f;

const float cutDiff = 40;

// rotation for testing
float rotAng = 0.0f;


const float PLAYER_R = 0.4f;   // collision radius
const float CORRIDOR_HALF_WIDTH = 1.8f;   // corridor lane: x in [-1.8, 1.8]

const float Z_FRONT_LIMIT = -80.0f;  // far end (more negative)
const float Z_BACK_LIMIT = 5.0f;   // behind start (positive)

const float JUMP_OVER_HEIGHT = 0.4f; // if playerY >= this, crates no longer block

const float CRATE_HEIGHT = 1.2f;       // how high the crates are
const float STEP_EPS = 0.1f;       // how much higher you must be to step up

const float Z_MIN = 0.0f;   // start point
const float Z_MAX = 80.0f;  // far end (adjust to your scene)



int   playerHealth = 100;
int   playerAmmo = 30;
int   playerScore = 0;

bool  isFiring = false;  // just for animation if you want
float gunRecoil = 0.0f;   // degrees
float gunRecoilDecay = 0.8f;   // how fast it goes back to 0
float muzzleFlashTime = 0.0f;   // frames or seconds, we’ll just decay it

// Zombie
int   zombieHealth = 100;
bool  zombieAlive = true;

// Shooting
const float SHOOT_RANGE = 50.0f;
const float ZOMBIE_RADIUS = 1.2f;   // approximate

void drawText(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    while (*s) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *s++);
    }
}




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


enum PickupType {
    PICKUP_NONE = -1,
    PICKUP_HEALTH = 0,
    PICKUP_AMMO = 1
};

struct GameObject {
    float x, y, z;
    float sx, sy, sz;
    float ry;
    Mesh* mesh = nullptr;
    Model* model = nullptr;
    unsigned int texId = 0;

    // extra fields (safe for all uses)
    PickupType pickupType = PICKUP_NONE;
    bool collected = false;

    void draw() const {
        if (collected && pickupType != PICKUP_NONE) return; // don't draw collected pickups

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


struct Vec3 {
    float x, y, z;
};

Vec3 gCamPos = { 0,0,0 };
Vec3 gCamDir = { 0,0,-1 };
Vec3 gCamRight = { 1,0,0 };
Vec3 gCamUp = { 0,1,0 };

Vec3 makeVec(float x, float y, float z) { return { x,y,z }; }
Vec3 add(const Vec3& a, const Vec3& b) { return { a.x + b.x,a.y + b.y,a.z + b.z }; }
Vec3 mul(const Vec3& a, float s) { return { a.x * s,a.y * s,a.z * s }; }

Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 normalize(const Vec3& v) {
    float len2 = v.x * v.x + v.y * v.y + v.z * v.z;
    if (len2 <= 1e-6f) return { 0,0,-1 };
    float inv = 1.0f / sqrtf(len2);
    return { v.x * inv, v.y * inv, v.z * inv };
}




bool  showBulletRay = false;
Vec3  bulletStart;
Vec3  bulletEnd;
float bulletRayTime = 0.0f;      // remaining time to show


bool rayHitsZombie(float& outHitDist) {
    if (!zombieAlive || enemies.empty()) return false;

    const GameObject& z = enemies[0];

    float zombieCenterY = z.y + 1.0f;  // tweak if needed

    Vec3 center = { z.x, zombieCenterY, z.z };

    // oc = vector from camera to zombie center
    Vec3 oc = { center.x - gCamPos.x,
                center.y - gCamPos.y,
                center.z - gCamPos.z };

    // projection length t along ray
    float t = oc.x * gCamDir.x + oc.y * gCamDir.y + oc.z * gCamDir.z;
    if (t < 0.0f || t > SHOOT_RANGE) return false;

    // closest point on ray to center
    Vec3 closest = {
        gCamPos.x + gCamDir.x * t,
        gCamPos.y + gCamDir.y * t,
        gCamPos.z + gCamDir.z * t
    };

    Vec3 diff = { center.x - closest.x,
                  center.y - closest.y,
                  center.z - closest.z };

    float dist2 = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

    if (dist2 <= ZOMBIE_RADIUS * ZOMBIE_RADIUS) {
        outHitDist = t;   // where the bullet hit
        return true;
    }
    return false;
}

void tryShoot() {
    if (playerAmmo <= 0) return;

    playerAmmo--;
    isFiring = true;
    gunRecoil = 8.0f;
    muzzleFlashTime = 0.1f;

    float hitDist = SHOOT_RANGE;
    float tHit = 0.0f;

    // keep your logic, just make sure rayHitsZombie uses gCamPos/gCamDir or same direction
    if (rayHitsZombie(tHit)) {
        hitDist = tHit;
        zombieHealth -= 34;
        playerScore += 20;
        if (zombieHealth <= 0) {
            zombieAlive = false;
            playerScore += 50;
        }
    }

    // === VISUAL RAY FROM MUZZLE ===
    showBulletRay = true;
    bulletRayTime = 0.08f;  // short flash

    // offsets in CAMERA space (approx muzzle position)
    float muzzleRight = 0.15f;  // to the right of camera
    float muzzleUp = -0.10f;  // slightly down (because gun is lower)
    float muzzleForward = 0.6f;   // in front of camera

    // transform that to world space: P_muzzle = CamPos + R*x + U*y + D*z
    Vec3 offset =
        add(
            add(mul(gCamRight, muzzleRight),
                mul(gCamUp, muzzleUp)),
            mul(gCamDir, muzzleForward)
        );

    bulletStart = add(gCamPos, offset);
    bulletEnd = add(bulletStart, mul(gCamDir, hitDist));
}








struct Pickup {
    GameObject obj;
    PickupType type;
    bool collected = false;
};




// initial collisions structs

struct AABB {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
};

std::vector<AABB> worldColliders;



bool pointInsideAABB(float x, float y, float z, const AABB& b) {
    return (x >= b.minX && x <= b.maxX &&
        y >= b.minY && y <= b.maxY &&
        z >= b.minZ && z <= b.maxZ);
}


bool circleIntersectsAABB(float px, float pz, float radius, const AABB& b) {
    // clamp circle center to box
    float cx = std::max(b.minX, std::min(px, b.maxX));
    float cz = std::max(b.minZ, std::min(pz, b.maxZ));

    float dx = px - cx;
    float dz = pz - cz;
    return (dx * dx + dz * dz) < (radius * radius);
}

bool inCrateZone(float z) {
    // crates baked into corridor, roughly these Z intervals (negative because forward = -Z)
    if (z < -11.0f && z > -14.0f)   return true;  // block 1
    if (z < -27.0f && z > -29.5f)   return true;  // block 2
    if (z < -42.0f && z > -50.0f)   return true;  // block 3 (tweak end)

    return false;
}

float getGroundHeightAt(float x, float z) {
    // only in the lane, crates form a raised floor
    if (fabs(x) < CORRIDOR_HALF_WIDTH && inCrateZone(z)) {
        return CRATE_HEIGHT;
    }
    // default floor
    return 0.0f;
}


bool collidesWithWalls(float newX, float newZ) {
    // left wall
    if (newX - PLAYER_R < -CORRIDOR_HALF_WIDTH) return true;
    // right wall
    if (newX + PLAYER_R > CORRIDOR_HALF_WIDTH) return true;

    // front limit (far end)
    if (newZ - PLAYER_R < Z_MIN) return true;
    // back limit (behind player spawn)
    if (newZ + PLAYER_R > Z_MAX) return true;

    return false;
}



bool canMoveTo(float newX, float newZ) {
    // 1) corridor walls in X
    if (newX - PLAYER_R < -CORRIDOR_HALF_WIDTH) return false;
    if (newX + PLAYER_R > CORRIDOR_HALF_WIDTH) return false;

    // 2) corridor limits in Z
    if (newZ - PLAYER_R < Z_FRONT_LIMIT) return false; // too far forward (-Z)
    if (newZ + PLAYER_R > Z_BACK_LIMIT)  return false; // too far back

    // 3) stepping logic: compare current vs next ground height
    float currentGround = getGroundHeightAt(playerX, playerZ);
    float nextGround = getGroundHeightAt(newX, newZ);

    // going DOWN is always fine (fall or step down)
    if (nextGround <= currentGround) {
        return true;
    }

    // going UP: require that your body is already high enough to clear the step
    // i.e., you have jumped. If you're basically still at current ground,
    // block the movement so you "bump" into the crate.
    float requiredHeightToStep = currentGround + CRATE_HEIGHT * 0.5f; // or currentGround + STEP_EPS;

    if (playerY <= requiredHeightToStep) {
        return false; // not high enough to get onto crate
    }

    // in the air and high enough -> allow, you'll land on top via Anim()
    return true;
}










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



void movePlayer(float forwardDelta, float rightDelta) {
    float yawRad = playerYaw * 3.14159265f / 180.0f;

    float dirX = sinf(yawRad);
    float dirZ = -cosf(yawRad);  // forward is -Z
    float rightX = cosf(yawRad);
    float rightZ = sinf(yawRad);

    float newX = playerX + dirX * forwardDelta + rightX * rightDelta;
    float newZ = playerZ + dirZ * forwardDelta + rightZ * rightDelta;

    if (canMoveTo(newX, newZ)) {
        playerX = newX;
        playerZ = newZ;
    }
}





void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': case 'W': movePlayer(MOVE_SPEED, 0.0f); break;
    case 's': case 'S': movePlayer(-MOVE_SPEED, 0.0f); break;
    case 'a': case 'A': movePlayer(0.0f, -MOVE_SPEED); break;
    case 'd': case 'D': movePlayer(0.0f, MOVE_SPEED); break;

    case ' ':
    if (isGrounded) {
        isGrounded = false;
        playerVelY = JUMP_VELOCITY;
    }
    break;

    }
}


void SpecialKeys(int key, int x, int y) {
    float angleStep = 20.0f;

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
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        viewMode = (viewMode == VIEW_FPS) ? VIEW_TPS : VIEW_FPS;
    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // only shoot in FPS (up to you)
        if (viewMode == VIEW_FPS) {
            tryShoot();
        }
    }
}


void drawSimpleHands() {
    glDisable(GL_TEXTURE_2D);      // just solid color
    glColor3f(0.8f, 0.7f, 0.6f);   // skin-ish

    // Right forearm
    glPushMatrix();
    glTranslatef(0.20f, -0.05f, -0.25f);  // tweak these
    glScalef(0.20f, 0.10f, 0.45f);        // length, thickness
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left forearm
    glPushMatrix();
    glTranslatef(-0.10f, -0.05f, -0.20f);
    glScalef(0.18f, 0.10f, 0.40f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // (Optional: small block as “hand grip” on the gun)
    glPushMatrix();
    glTranslatef(0.05f, -0.02f, -0.32f);
    glScalef(0.10f, 0.08f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void drawBetterHands() {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.8f, 0.7f, 0.6f);  // skin-ish for hands

    // Right forearm
    glPushMatrix();
    glTranslatef(0.15f, -0.05f, -0.1f); // shift to right
    glRotatef(-20.0f, 1, 0, 0);        // slight bend
    glScalef(0.12f, 0.12f, 0.4f);      // long in Z
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right hand (fist)
    glPushMatrix();
    glTranslatef(0.15f, -0.12f, -0.35f); // in front of gun
    glScalef(0.13f, 0.13f, 0.13f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left forearm
    glPushMatrix();
    glTranslatef(-0.05f, -0.03f, -0.15f); // slightly left and back
    glRotatef(-15.0f, 1, 0, 0);
    glScalef(0.10f, 0.10f, 0.35f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left hand (supporting)
    glPushMatrix();
    glTranslatef(-0.02f, -0.10f, -0.32f);
    glScalef(0.11f, 0.11f, 0.11f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(1, 1, 1);
}


void applyCamera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float eyeHeight = 1.6f;

    // choose camera position (FPS or TPS)
    if (viewMode == VIEW_FPS) {
        gCamPos.x = playerX;
        gCamPos.y = playerY + eyeHeight;
        gCamPos.z = playerZ;
    }
    else {
        float yawRad = playerYaw * 3.14159265f / 180.0f;
        float fx = sinf(yawRad);
        float fz = -cosf(yawRad);

        float camDistBack = 5.0f;
        float camHeight = 2.5f;

        gCamPos.x = playerX - fx * camDistBack;
        gCamPos.y = playerY + camHeight;
        gCamPos.z = playerZ - fz * camDistBack;
    }

    // forward direction from yaw + pitch
    float yawRad = playerYaw * 3.14159265f / 180.0f;
    float pitchRad = camPitch * 3.14159265f / 180.0f;

    gCamDir.x = cosf(pitchRad) * sinf(yawRad);
    gCamDir.y = sinf(pitchRad);
    gCamDir.z = -cosf(pitchRad) * cosf(yawRad);
    gCamDir = normalize(gCamDir);

    // build camera basis: right / up
    Vec3 worldUp = { 0,1,0 };
    gCamRight = normalize(cross(gCamDir, worldUp));
    gCamUp = normalize(cross(gCamRight, gCamDir));

    // target point for gluLookAt
    float cx = gCamPos.x + gCamDir.x;
    float cy = gCamPos.y + gCamDir.y;
    float cz = gCamPos.z + gCamDir.z;

    gluLookAt(
        gCamPos.x, gCamPos.y, gCamPos.z,
        cx, cy, cz,
        0.0f, 1.0f, 0.0f
    );
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



    for (auto& c : crates)  c.draw();
    if (zombieAlive && !enemies.empty()) {
        enemies[0].draw();
    }

    for (auto& p : pickups) {
        if (p.collected || p.pickupType == PICKUP_NONE) continue;

        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);

        // spin + bob
        glRotatef(rotAng * 50.0f, 0, 1, 0);
        glTranslatef(0.0f, 0.1f * sinf(rotAng * 3.0f), 0.0f);

        glScalef(p.sx, p.sy, p.sz);

        if (p.mesh) {
            if (p.texId) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, p.texId);
                glColor3f(1, 1, 1);
                p.mesh->draw(true);
                glDisable(GL_TEXTURE_2D);
            }
            else {
                glDisable(GL_TEXTURE_2D);
                glColor3f(0.7f, 0.7f, 0.7f);
                p.mesh->draw(false);
            }
        }
        glPopMatrix();
    }




    // 2) bullet ray in world
    if (showBulletRay && bulletRayTime > 0.0f) {
        glDisable(GL_TEXTURE_2D);
        // you can leave depth test ON so it can disappear behind walls,
        // or turn it off if you want "always visible":
        // glDisable(GL_DEPTH_TEST);

        glLineWidth(3.0f);
        glColor3f(1.0f, 0.9f, 0.3f);

        glBegin(GL_LINES);
        glVertex3f(bulletStart.x- 0.03f, bulletStart.y, bulletStart.z);
        glVertex3f(bulletEnd.x, bulletEnd.y, bulletEnd.z);
        glEnd();

        glColor3f(1, 1, 1);
        // glEnable(GL_DEPTH_TEST);  // if you disabled it above
    }

    // 3) FPS gun overlay (hands + gun, no ray here anymore)
    if (viewMode == VIEW_FPS) {
        glDisable(GL_DEPTH_TEST);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glTranslatef(0.2f, -0.18f, -0.75f);
        glRotatef(gunRecoil, 1, 0, 0);
        glRotatef(180.0f, 0, 1, 0);
        glRotatef(5.0f, 0, 1, 0);

        drawBetterHands();

        glPushMatrix();
        glScalef(SCALE_GUN, SCALE_GUN, SCALE_GUN);
        drawTextured(gunMesh, gunTexture);
        glPopMatrix();

        // muzzle flash as before...

        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
    }



    if (viewMode == VIEW_TPS) {
        playerVisual.x = playerX;
        playerVisual.y = playerY;
        playerVisual.z = playerZ;
        playerVisual.ry = playerYaw;
        playerVisual.draw();
    }

 // --- HUD overlay ---
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);  // simple normalized 2D

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    char buf[128];
    snprintf(buf, sizeof(buf), "HP: %d   Ammo: %d   Score: %d", playerHealth, playerAmmo, playerScore);
    glColor3f(1, 1, 1);
    drawText(0.05f, 0.95f, buf);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_LIGHTING); // if you had it

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);



    glFlush();
}


void Anim() {
    // other stuff like rotAng, animations...
    rotAng += 0.01f;

    // --- vertical motion ---
    float groundY = getGroundHeightAt(playerX, playerZ);

    if (!isGrounded) {
        playerY += playerVelY;
        playerVelY += GRAVITY;

        // did we hit the ground (floor or crate top)?
        if (playerY <= groundY) {
            playerY = groundY;
            playerVelY = 0.0f;
            isGrounded = true;
        }
    }
    else {
        // just to be safe: if ground moved (e.g. walked off a crate)
        if (playerY > groundY + 0.01f) {
            isGrounded = false;  // start falling
        }
        else {
            playerY = groundY;   // stick to surface
        }
    }

    // --- pickup logic ---
    for (auto& p : pickups) {
        if (p.collected) continue;
        if (p.pickupType == PICKUP_NONE) continue;

        float dx = playerX - p.x;
        float dz = playerZ - p.z;
        float dist2 = dx * dx + dz * dz;

        if (dist2 < 1.0f) { // within 1 unit
            p.collected = true;

            if (p.pickupType == PICKUP_HEALTH) {
                playerHealth = std::min(100, playerHealth + 25);
                playerScore += 10;
            }
            else if (p.pickupType == PICKUP_AMMO) {
                playerAmmo += 15;
                playerScore += 5;
            }
        }
    }


    // recoil decay
    if (gunRecoil > 0.0f) {
        gunRecoil *= gunRecoilDecay;   // exponential decay
        if (gunRecoil < 0.1f) gunRecoil = 0.0f;
    }

    // muzzle flash time decay
    if (muzzleFlashTime > 0.0f) {
        muzzleFlashTime -= 0.02f;  // tweak based on your frame rate
        if (muzzleFlashTime < 0.0f) muzzleFlashTime = 0.0f;
    }

    if (bulletRayTime > 0.0f) {
        bulletRayTime -= 0.02f;  // tweak
        if (bulletRayTime <= 0.0f) {
            bulletRayTime = 0.0f;
            showBulletRay = false;
        }
    }

    showBulletRay = true; // for temporary debug: always true



    // draw bullet ray if active
    if (showBulletRay && bulletRayTime > 0.0f) {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glLineWidth(3.0f);
        glColor3f(1.0f, 0.9f, 0.3f);   // same color as muzzle flash

        glBegin(GL_LINES);
        glVertex3f(bulletStart.x , bulletStart.y, bulletStart.z);
        glVertex3f(bulletEnd.x, bulletEnd.y, bulletEnd.z);
        glEnd();

        glColor3f(1, 1, 1);
        // glEnable(GL_LIGHTING); // if you had it enabled
    }


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
    // crate 1 at (-2.0, 0, -8.0)
    crates.push_back({
        -2.0f, 0.0f, -8.0f,
        SCALE_CRATE, SCALE_CRATE, SCALE_CRATE,
        0.0f,
        &crateMesh, nullptr,
        crateTexture
        });
    {
        auto& c = crates.back();
        float hw = 1.0f;  // half-width (tweak)
        float hd = 1.0f;  // half-depth

        worldColliders.push_back({
            c.x - hw, c.x + hw,
            c.z - hd, c.z + hd
            });
    }

    // crate 2 at (2.0, 0, -12.0)
    crates.push_back({
        2.0f, 0.0f, -12.0f,
        SCALE_CRATE, SCALE_CRATE, SCALE_CRATE,
        15.0f,
        &crateMesh, nullptr,
        crateTexture
        });
    {
        auto& c = crates.back();
        float hw = 1.0f;
        float hd = 1.0f;

        worldColliders.push_back({
            c.x - hw, c.x + hw,
            c.z - hd, c.z + hd
            });
    }





   // Ammo on top of second crate
    {
        GameObject ammo;
        ammo.x = 1.0f;
        ammo.y = 1.0f;
        ammo.z = -16.0f;
        ammo.sx = ammo.sy = ammo.sz = SCALE_AMMO;
        ammo.ry = 0.0f;
        ammo.mesh = &ammoMesh;
        ammo.model = nullptr;
        ammo.texId = ammoTexture;
        ammo.pickupType = PICKUP_AMMO;
        ammo.collected = false;

        pickups.push_back(ammo);
    } 

    // Health on the ground near first crate
    {
        GameObject hp;
        hp.x = 1.0f;
        hp.y = 0.5f;
        hp.z = -11.5f;
        hp.sx = hp.sy = hp.sz = SCALE_HEALTH;
        hp.ry = 0.0f;
        hp.mesh = &healthMesh;
        hp.model = nullptr;
        hp.texId = healthTexture;
        hp.pickupType = PICKUP_HEALTH;
        hp.collected = false;

        pickups.push_back(hp);
    }


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



    // left wall: everything with x <= -2.5 is blocked
    worldColliders.push_back({
        -1000.0f, -30.0f,
        -1000.0f,  1000.0f
        });

    // right wall: everything with x >= 2.5 is blocked
    worldColliders.push_back({
         30.0f,  1000.0f,
        -1000.0f, 1000.0f
        });



   

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
    c0.x = -0.2f;
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
