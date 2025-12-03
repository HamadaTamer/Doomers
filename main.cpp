/**
 * DOOMERS - DOOM-Style FPS Game
 * With FBX models loaded via Assimp
 */

#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glut.h>

#include "Engine/Core.hpp"
#include "Engine/Math.hpp"
#include "Engine/InputManager.hpp"
#include "Engine/AssimpLoader.hpp"
#include "Game/AssetManager.hpp"

using namespace Doomers::Math;
using Doomers::InputManager;
using Doomers::MouseButton;
using Doomers::AssetManager;
using Doomers::AnimatedModel;

// ============================================================================
// CONSTANTS
// ============================================================================
const float PLAYER_SPEED = 6.0f;
const float SPRINT_MULT = 1.6f;
const float MOUSE_SENS = 0.12f;
const float JUMP_FORCE = 9.0f;
const float GRAVITY = 22.0f;
const int WEAPON_DAMAGE = 30;
const float FIRE_RATE = 0.15f;

// ============================================================================
// CAMERA
// ============================================================================
struct FPSCamera {
    Vector3 pos{0, 1.7f, 8};
    float yaw = -90.0f, pitch = 0.0f;
    Vector3 front{0, 0, -1}, right{1, 0, 0}, up{0, 1, 0};
    
    void update() {
        float yr = yaw * 0.01745f, pr = pitch * 0.01745f;
        front.x = cosf(pr) * cosf(yr);
        front.y = sinf(pr);
        front.z = cosf(pr) * sinf(yr);
        front = front.normalized();
        right = front.cross(Vector3(0,1,0)).normalized();
        up = right.cross(front).normalized();
    }
    
    void rotate(int dx, int dy) {
        yaw += dx * MOUSE_SENS;
        pitch += dy * MOUSE_SENS;
        if (pitch > 89) pitch = 89;
        if (pitch < -89) pitch = -89;
        update();
    }
    
    Vector3 flatFront() { return Vector3(front.x, 0, front.z).normalized(); }
    Vector3 flatRight() { return Vector3(right.x, 0, right.z).normalized(); }
    
    void apply() {
        auto t = pos + front;
        gluLookAt(pos.x, pos.y, pos.z, t.x, t.y, t.z, up.x, up.y, up.z);
    }
};

// ============================================================================
// PLAYER
// ============================================================================
struct GamePlayer {
    Vector3 pos{0, 0, 8};
    Vector3 vel{0, 0, 0};
    FPSCamera cam;
    
    float hp = 100, maxHp = 100, armor = 0;
    int ammo = 60, maxAmmo = 200, score = 0, kills = 0;
    bool onGround = true, sprinting = false;
    float fireCooldown = 0;
    float damageFlash = 0;
    
    void update(float dt) {
        auto& inp = InputManager::instance();
        Vector3 move{0,0,0};
        
        if (inp.isKeyDown('w')) move = move + cam.flatFront();
        if (inp.isKeyDown('s')) move = move - cam.flatFront();
        if (inp.isKeyDown('d')) move = move + cam.flatRight();
        if (inp.isKeyDown('a')) move = move - cam.flatRight();
        
        if (move.lengthSquared() > 0.01f) move = move.normalized();
        
        sprinting = inp.isShiftDown();
        float spd = PLAYER_SPEED * (sprinting ? SPRINT_MULT : 1.0f);
        vel.x = move.x * spd;
        vel.z = move.z * spd;
        
        if (inp.isKeyDown(' ') && onGround) {
            vel.y = JUMP_FORCE;
            onGround = false;
        }
        
        if (!onGround) vel.y -= GRAVITY * dt;
        
        pos = pos + vel * dt;
        
        if (pos.y <= 0) { pos.y = 0; vel.y = 0; onGround = true; }
        
        // Clamp to level bounds
        float bound = 28.0f;
        if (pos.x < -bound) pos.x = -bound;
        if (pos.x > bound) pos.x = bound;
        if (pos.z < -bound) pos.z = -bound;
        if (pos.z > bound) pos.z = bound;
        
        cam.pos = pos + Vector3(0, 1.7f, 0);
        
        if (fireCooldown > 0) fireCooldown -= dt;
        if (damageFlash > 0) damageFlash -= dt * 3;
    }
    
    bool canFire() { return fireCooldown <= 0 && ammo > 0; }
    void fire() { fireCooldown = FIRE_RATE; ammo--; }
    void takeDamage(float d) { 
        float absorbed = std::min(armor, d * 0.5f);
        armor -= absorbed;
        hp -= (d - absorbed);
        damageFlash = 1.0f;
        if (hp < 0) hp = 0;
    }
    bool alive() { return hp > 0; }
};

// ============================================================================
// ENEMY
// ============================================================================
struct GameEnemy {
    enum Type { ZOMBIE, DEVIL };
    enum State { IDLE, WALK, ATTACK, DYING, DEAD };
    
    Type type;
    State state = IDLE;
    Vector3 pos;
    float yaw = 0;
    float hp, maxHp;
    float speed;
    float damage;
    float atkCooldown = 0;
    float atkRange = 2.2f;
    float deathTimer = 0;
    float animTime = 0;
    
    GameEnemy(Type t, Vector3 p) : type(t), pos(p) {
        if (t == ZOMBIE) {
            hp = maxHp = 80;
            speed = 3.0f;
            damage = 12;
        } else {
            hp = maxHp = 180;
            speed = 4.5f;
            damage = 25;
            atkRange = 2.8f;
        }
    }
    
    bool alive() { return state != DEAD; }
    
    void update(float dt, GamePlayer& player) {
        if (state == DEAD) return;
        
        animTime += dt;
        
        if (state == DYING) {
            deathTimer += dt;
            if (deathTimer > 2.5f) state = DEAD;
            return;
        }
        
        atkCooldown -= dt;
        
        Vector3 toP = player.pos - pos;
        toP.y = 0;
        float dist = toP.length();
        
        if (dist > 0.5f) {
            yaw = atan2f(toP.x, toP.z) * 57.3f;
            
            if (dist <= atkRange) {
                state = ATTACK;
                if (atkCooldown <= 0) {
                    player.takeDamage(damage);
                    atkCooldown = 1.2f;
                }
            } else {
                state = WALK;
                Vector3 dir = toP.normalized();
                pos = pos + dir * speed * dt;
            }
        }
        pos.y = 0;
    }
    
    void takeDamage(float d) {
        hp -= d;
        if (hp <= 0) {
            hp = 0;
            state = DYING;
            deathTimer = 0;
        }
    }
    
    void draw() {
        if (state == DEAD) return;
        
        auto& assets = AssetManager::instance();
        
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        glRotatef(yaw, 0, 1, 0);
        
        // Try to draw FBX model
        AnimatedModel* model = nullptr;
        float scale = 0.018f;
        
        if (type == ZOMBIE) {
            if (state == DYING && assets.zombieDeath) model = assets.zombieDeath;
            else if (state == ATTACK && assets.zombieAttack) model = assets.zombieAttack;
            else if (state == WALK && assets.zombieWalk) model = assets.zombieWalk;
            else if (assets.zombieIdle) model = assets.zombieIdle;
        } else {
            scale = 0.022f;
            if (state == ATTACK && assets.devilAttack) model = assets.devilAttack;
            else if (state == WALK && assets.devilWalk) model = assets.devilWalk;
            else if (assets.devilModel) model = assets.devilModel;
        }
        
        if (model) {
            model->animationTime = animTime;
            glScalef(scale, scale, scale);
            model->draw();
        } else {
            // Fallback: Draw colored humanoid shape
            if (type == ZOMBIE) {
                glColor3f(0.3f, 0.5f, 0.3f);
            } else {
                glColor3f(0.7f, 0.15f, 0.1f);
            }
            
            // Body
            glPushMatrix();
            glTranslatef(0, 1.0f, 0);
            glScalef(0.6f, 1.2f, 0.4f);
            glutSolidCube(1.0f);
            glPopMatrix();
            
            // Head
            glPushMatrix();
            glTranslatef(0, 1.9f, 0);
            glutSolidSphere(0.25f, 12, 12);
            glPopMatrix();
            
            // Arms
            if (state == ATTACK) {
                glColor3f(0.9f, 0.2f, 0.2f);
            }
            glPushMatrix();
            glTranslatef(-0.45f, 1.2f, 0.3f);
            glScalef(0.15f, 0.6f, 0.15f);
            glutSolidCube(1.0f);
            glPopMatrix();
            
            glPushMatrix();
            glTranslatef(0.45f, 1.2f, 0.3f);
            glScalef(0.15f, 0.6f, 0.15f);
            glutSolidCube(1.0f);
            glPopMatrix();
            
            // Legs
            glColor3f(0.25f, 0.25f, 0.25f);
            glPushMatrix();
            glTranslatef(-0.15f, 0.4f, 0);
            glScalef(0.2f, 0.8f, 0.2f);
            glutSolidCube(1.0f);
            glPopMatrix();
            
            glPushMatrix();
            glTranslatef(0.15f, 0.4f, 0);
            glScalef(0.2f, 0.8f, 0.2f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
        
        glPopMatrix();
        
        // Health bar above enemy
        if (state != DYING && state != DEAD) {
            glPushMatrix();
            glTranslatef(pos.x, pos.y + 2.4f, pos.z);
            
            // Billboard - face camera
            glDisable(GL_LIGHTING);
            
            float hpPct = hp / maxHp;
            float barW = 1.2f;
            
            // Background
            glColor4f(0.2f, 0.0f, 0.0f, 0.8f);
            glBegin(GL_QUADS);
            glVertex3f(-barW/2, 0, 0);
            glVertex3f(barW/2, 0, 0);
            glVertex3f(barW/2, 0.12f, 0);
            glVertex3f(-barW/2, 0.12f, 0);
            glEnd();
            
            // Health
            if (hpPct > 0.5f) glColor4f(0.2f, 0.8f, 0.2f, 0.9f);
            else if (hpPct > 0.25f) glColor4f(0.9f, 0.7f, 0.1f, 0.9f);
            else glColor4f(0.9f, 0.2f, 0.1f, 0.9f);
            
            glBegin(GL_QUADS);
            glVertex3f(-barW/2, 0, 0.01f);
            glVertex3f(-barW/2 + barW * hpPct, 0, 0.01f);
            glVertex3f(-barW/2 + barW * hpPct, 0.12f, 0.01f);
            glVertex3f(-barW/2, 0.12f, 0.01f);
            glEnd();
            
            glEnable(GL_LIGHTING);
            glPopMatrix();
        }
    }
};

// ============================================================================
// PICKUP
// ============================================================================
struct Pickup {
    enum Type { HEALTH, AMMO, ARMOR };
    Type type;
    Vector3 pos;
    bool active = true;
    float bobT = 0, spinT = 0;
    int value;
    
    Pickup(Type t, Vector3 p, int v) : type(t), pos(p), value(v) {
        bobT = (float)(rand() % 100) / 10.0f;
    }
    
    void update(float dt) {
        bobT += dt * 2.5f;
        spinT += dt * 80.0f;
    }
    
    void draw() {
        if (!active) return;
        
        float bob = sinf(bobT) * 0.15f;
        
        glPushMatrix();
        glTranslatef(pos.x, pos.y + 0.6f + bob, pos.z);
        glRotatef(spinT, 0, 1, 0);
        
        // Draw pickup based on type
        if (type == HEALTH) {
            // Green cross
            glColor3f(0.2f, 0.9f, 0.3f);
            glPushMatrix();
            glScalef(0.4f, 0.12f, 0.12f);
            glutSolidCube(1.0f);
            glPopMatrix();
            glPushMatrix();
            glScalef(0.12f, 0.4f, 0.12f);
            glutSolidCube(1.0f);
            glPopMatrix();
        } else if (type == AMMO) {
            // Yellow ammo box
            glColor3f(0.9f, 0.75f, 0.1f);
            glScalef(0.35f, 0.25f, 0.2f);
            glutSolidCube(1.0f);
        } else {
            // Blue armor
            glColor3f(0.2f, 0.5f, 0.95f);
            glutSolidSphere(0.22f, 12, 12);
        }
        
        glPopMatrix();
    }
    
    bool checkPickup(Vector3& playerPos) {
        if (!active) return false;
        return (pos - playerPos).length() < 1.5f;
    }
};

// ============================================================================
// GLOBALS
// ============================================================================
enum GameState { MENU, PLAYING, PAUSED, DEAD, WIN, NEXT_LEVEL };
GameState g_state = MENU;
GamePlayer g_player;
std::vector<GameEnemy> g_enemies;
std::vector<Pickup> g_pickups;
int g_level = 1;
int g_winW = 1280, g_winH = 720;
float g_time = 0, g_dt = 0;
bool g_mouseLock = false;
float g_hitMarker = 0;

// ============================================================================
// INIT LEVEL
// ============================================================================
void initLevel(int lvl) {
    g_level = lvl;
    g_enemies.clear();
    g_pickups.clear();
    
    if (lvl == 1) {
        // Research Facility - Zombies
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {-12, 0, -12}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {12, 0, -12}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {0, 0, -18}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {-18, 0, 5}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {18, 0, 5}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {-8, 0, 18}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {8, 0, 18}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {0, 0, -8}));
    } else {
        // Hell Arena - Zombies + Devils
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {-15, 0, -15}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {15, 0, -15}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {-20, 0, 0}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {20, 0, 0}));
        g_enemies.push_back(GameEnemy(GameEnemy::ZOMBIE, {0, 0, 20}));
        // Devils
        g_enemies.push_back(GameEnemy(GameEnemy::DEVIL, {0, 0, -22}));
        g_enemies.push_back(GameEnemy(GameEnemy::DEVIL, {-22, 0, -8}));
        g_enemies.push_back(GameEnemy(GameEnemy::DEVIL, {22, 0, -8}));
        g_enemies.push_back(GameEnemy(GameEnemy::DEVIL, {0, 0, -12}));
    }
    
    // Pickups
    g_pickups.push_back(Pickup(Pickup::HEALTH, {-10, 0, 0}, 30));
    g_pickups.push_back(Pickup(Pickup::HEALTH, {10, 0, 0}, 30));
    g_pickups.push_back(Pickup(Pickup::AMMO, {0, 0, -12}, 25));
    g_pickups.push_back(Pickup(Pickup::AMMO, {0, 0, 12}, 25));
    g_pickups.push_back(Pickup(Pickup::ARMOR, {15, 0, 15}, 40));
    g_pickups.push_back(Pickup(Pickup::ARMOR, {-15, 0, -15}, 40));
    
    if (lvl == 2) {
        g_pickups.push_back(Pickup(Pickup::HEALTH, {-18, 0, 10}, 50));
        g_pickups.push_back(Pickup(Pickup::AMMO, {18, 0, -10}, 40));
    }
}

void startGame(int lvl) {
    g_state = PLAYING;
    g_player = GamePlayer();
    g_player.cam.update();
    initLevel(lvl);
    glutSetCursor(GLUT_CURSOR_NONE);
    g_mouseLock = true;
    glutWarpPointer(g_winW/2, g_winH/2);
}

// ============================================================================
// DRAW LEVEL
// ============================================================================
void drawLevel() {
    float size = 30.0f;
    float wallH = 5.0f;
    
    auto& assets = AssetManager::instance();
    AnimatedModel* envModel = (g_level == 1) ? assets.levelModel : assets.level2Model;
    
    if (envModel) {
        // Draw FBX environment
        glPushMatrix();
        glScalef(0.08f, 0.08f, 0.08f);
        envModel->draw();
        glPopMatrix();
    }
    
    // Always draw floor and walls (even if FBX exists, for bounds)
    
    // Floor
    if (g_level == 1) {
        glColor3f(0.22f, 0.25f, 0.28f);
    } else {
        glColor3f(0.28f, 0.12f, 0.08f);
    }
    
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-size, 0, -size);
    glVertex3f(size, 0, -size);
    glVertex3f(size, 0, size);
    glVertex3f(-size, 0, size);
    glEnd();
    
    // Floor grid
    glColor3f(0.15f, 0.18f, 0.2f);
    glBegin(GL_LINES);
    for (float i = -size; i <= size; i += 3.0f) {
        glVertex3f(i, 0.02f, -size);
        glVertex3f(i, 0.02f, size);
        glVertex3f(-size, 0.02f, i);
        glVertex3f(size, 0.02f, i);
    }
    glEnd();
    
    // Walls
    if (g_level == 1) {
        glColor3f(0.35f, 0.38f, 0.45f);
    } else {
        glColor3f(0.42f, 0.18f, 0.12f);
    }
    
    // North
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(-size, 0, -size);
    glVertex3f(size, 0, -size);
    glVertex3f(size, wallH, -size);
    glVertex3f(-size, wallH, -size);
    glEnd();
    
    // South
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glVertex3f(-size, 0, size);
    glVertex3f(-size, wallH, size);
    glVertex3f(size, wallH, size);
    glVertex3f(size, 0, size);
    glEnd();
    
    // East
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glVertex3f(size, 0, -size);
    glVertex3f(size, 0, size);
    glVertex3f(size, wallH, size);
    glVertex3f(size, wallH, -size);
    glEnd();
    
    // West
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glVertex3f(-size, 0, -size);
    glVertex3f(-size, wallH, -size);
    glVertex3f(-size, wallH, size);
    glVertex3f(-size, 0, size);
    glEnd();
    
    // Ceiling
    if (g_level == 1) {
        glColor3f(0.18f, 0.2f, 0.22f);
    } else {
        glColor3f(0.12f, 0.06f, 0.04f);
    }
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glVertex3f(-size, wallH, -size);
    glVertex3f(size, wallH, -size);
    glVertex3f(size, wallH, size);
    glVertex3f(-size, wallH, size);
    glEnd();
    
    // Pillars
    glColor3f(0.4f, 0.42f, 0.45f);
    float pillars[][2] = {{-15,-10},{15,-10},{-15,10},{15,10},{0,-18},{0,18}};
    for (auto& p : pillars) {
        glPushMatrix();
        glTranslatef(p[0], wallH/2, p[1]);
        glScalef(1.8f, wallH, 1.8f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    
    // Level 2: Lava pools
    if (g_level == 2) {
        static float lavaT = 0;
        lavaT += g_dt;
        float pulse = 0.75f + 0.25f * sinf(lavaT * 3);
        
        glColor3f(1.0f * pulse, 0.35f * pulse, 0.1f);
        float pools[][2] = {{-22,-22},{22,-22},{-22,22},{22,22}};
        for (auto& lp : pools) {
            glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f(lp[0]-4, 0.03f, lp[1]-4);
            glVertex3f(lp[0]+4, 0.03f, lp[1]-4);
            glVertex3f(lp[0]+4, 0.03f, lp[1]+4);
            glVertex3f(lp[0]-4, 0.03f, lp[1]+4);
            glEnd();
        }
    }
}

// ============================================================================
// DRAW WEAPON (First Person)
// ============================================================================
void drawWeapon() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(65.0, (double)g_winW/g_winH, 0.05, 10.0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Position weapon at bottom right of view
    glTranslatef(0.35f, -0.35f, -0.7f);
    glRotatef(-5, 0, 0, 1);
    glRotatef(10, 0, 1, 0);
    
    // Recoil animation
    if (g_player.fireCooldown > 0) {
        float recoil = g_player.fireCooldown / FIRE_RATE;
        glTranslatef(0, 0, recoil * 0.08f);
        glRotatef(-recoil * 8, 1, 0, 0);
    }
    
    auto& assets = AssetManager::instance();
    if (assets.weaponModel) {
        glScalef(0.008f, 0.008f, 0.008f);
        assets.weaponModel->draw();
    } else {
        // Fallback: Simple gun shape
        glColor3f(0.2f, 0.2f, 0.25f);
        
        // Barrel
        glPushMatrix();
        glTranslatef(0, 0, -0.15f);
        glScalef(0.04f, 0.04f, 0.35f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Body
        glPushMatrix();
        glTranslatef(0, -0.02f, 0.1f);
        glScalef(0.06f, 0.08f, 0.2f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Handle
        glColor3f(0.15f, 0.12f, 0.1f);
        glPushMatrix();
        glTranslatef(0, -0.1f, 0.15f);
        glRotatef(15, 1, 0, 0);
        glScalef(0.04f, 0.12f, 0.06f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Laser sight glow
        glColor3f(1.0f, 0.2f, 0.2f);
        glPushMatrix();
        glTranslatef(0, 0.03f, -0.32f);
        glutSolidSphere(0.012f, 8, 8);
        glPopMatrix();
    }
    
    // Muzzle flash when firing
    if (g_player.fireCooldown > FIRE_RATE * 0.7f) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.8f, 0.3f);
        glPushMatrix();
        glTranslatef(0, 0, -0.4f);
        glutSolidSphere(0.05f, 8, 8);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ============================================================================
// DRAW HUD
// ============================================================================
void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, g_winW, 0, g_winH);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Health bar
    float hpPct = g_player.hp / g_player.maxHp;
    glColor4f(0.1f, 0.1f, 0.1f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(25, 25); glVertex2f(245, 25);
    glVertex2f(245, 55); glVertex2f(25, 55);
    glEnd();
    
    if (hpPct > 0.5f) glColor4f(0.15f, 0.85f, 0.25f, 0.95f);
    else if (hpPct > 0.25f) glColor4f(0.9f, 0.75f, 0.1f, 0.95f);
    else glColor4f(0.9f, 0.15f, 0.15f, 0.95f);
    
    glBegin(GL_QUADS);
    glVertex2f(28, 28); glVertex2f(28 + 214*hpPct, 28);
    glVertex2f(28 + 214*hpPct, 52); glVertex2f(28, 52);
    glEnd();
    
    glColor4f(1, 1, 1, 0.9f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(25, 25); glVertex2f(245, 25);
    glVertex2f(245, 55); glVertex2f(25, 55);
    glEnd();
    
    // Health cross icon
    glColor4f(1, 1, 1, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(5, 32); glVertex2f(22, 32); glVertex2f(22, 48); glVertex2f(5, 48);
    glVertex2f(9, 28); glVertex2f(18, 28); glVertex2f(18, 52); glVertex2f(9, 52);
    glEnd();
    
    // Armor bar
    if (g_player.armor > 0) {
        float armorPct = g_player.armor / 100.0f;
        glColor4f(0.1f, 0.1f, 0.1f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(25, 60); glVertex2f(245, 60);
        glVertex2f(245, 80); glVertex2f(25, 80);
        glEnd();
        
        glColor4f(0.2f, 0.5f, 0.95f, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(27, 62); glVertex2f(27 + 216*armorPct, 62);
        glVertex2f(27 + 216*armorPct, 78); glVertex2f(27, 78);
        glEnd();
    }
    
    // Ammo
    glColor4f(0.1f, 0.1f, 0.1f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(g_winW-240, 25); glVertex2f(g_winW-25, 25);
    glVertex2f(g_winW-25, 85); glVertex2f(g_winW-240, 85);
    glEnd();
    
    char txt[64];
    if (g_player.ammo > 10) glColor3f(1, 1, 1);
    else if (g_player.ammo > 0) glColor3f(1, 0.7f, 0);
    else glColor3f(1, 0.2f, 0.2f);
    
    sprintf(txt, "%d / %d", g_player.ammo, g_player.maxAmmo);
    glRasterPos2i(g_winW - 200, 45);
    for (char* c = txt; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    
    glColor3f(0.4f, 0.8f, 1.0f);
    glRasterPos2i(g_winW - 200, 65);
    const char* wep = "LASER RIFLE";
    for (const char* c = wep; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    
    // Crosshair
    float cx = g_winW / 2.0f, cy = g_winH / 2.0f;
    glColor4f(0, 1, 0.85f, 0.95f);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2f(cx, cy + 6); glVertex2f(cx, cy + 16);
    glVertex2f(cx, cy - 6); glVertex2f(cx, cy - 16);
    glVertex2f(cx - 6, cy); glVertex2f(cx - 16, cy);
    glVertex2f(cx + 6, cy); glVertex2f(cx + 16, cy);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(cx-2, cy-2); glVertex2f(cx+2, cy-2);
    glVertex2f(cx+2, cy+2); glVertex2f(cx-2, cy+2);
    glEnd();
    
    // Hit marker
    if (g_hitMarker > 0) {
        glColor4f(1, 1, 1, g_hitMarker);
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        glVertex2f(cx-15, cy-15); glVertex2f(cx-5, cy-5);
        glVertex2f(cx+15, cy-15); glVertex2f(cx+5, cy-5);
        glVertex2f(cx-15, cy+15); glVertex2f(cx-5, cy+5);
        glVertex2f(cx+15, cy+15); glVertex2f(cx+5, cy+5);
        glEnd();
    }
    
    // Score
    glColor4f(0.1f, 0.1f, 0.1f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(cx-90, g_winH-50); glVertex2f(cx+90, g_winH-50);
    glVertex2f(cx+90, g_winH-15); glVertex2f(cx-90, g_winH-15);
    glEnd();
    
    glColor3f(1, 0.95f, 0.3f);
    sprintf(txt, "SCORE: %d  KILLS: %d", g_player.score, g_player.kills);
    glRasterPos2i((int)(cx - 70), g_winH - 38);
    for (char* c = txt; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    
    // Level + enemies
    glColor4f(0.1f, 0.1f, 0.1f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(20, g_winH-50); glVertex2f(200, g_winH-50);
    glVertex2f(200, g_winH-15); glVertex2f(20, g_winH-15);
    glEnd();
    
    glColor3f(0.3f, 0.85f, 1.0f);
    const char* lvlName = (g_level == 1) ? "RESEARCH FACILITY" : "HELL ARENA";
    glRasterPos2i(30, g_winH - 38);
    for (const char* c = lvlName; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    
    int aliveCount = 0;
    for (auto& e : g_enemies) if (e.alive()) aliveCount++;
    
    glColor3f(1, 0.5f, 0.3f);
    sprintf(txt, "ENEMIES: %d", aliveCount);
    glRasterPos2i(30, g_winH - 25);
    for (char* c = txt; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    
    // Damage flash
    if (g_player.damageFlash > 0) {
        glColor4f(0.8f, 0, 0, g_player.damageFlash * 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(g_winW, 0);
        glVertex2f(g_winW, 100); glVertex2f(0, 100);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2f(0, g_winH); glVertex2f(g_winW, g_winH);
        glVertex2f(g_winW, g_winH-100); glVertex2f(0, g_winH-100);
        glEnd();
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ============================================================================
// DRAW MENU SCREEN
// ============================================================================
void drawMenuScreen(const char* title, const char* sub, float r, float g, float b) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, g_winW, 0, g_winH);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Background
    glBegin(GL_QUADS);
    glColor3f(0.05f, 0.05f, 0.08f);
    glVertex2f(0, 0); glVertex2f(g_winW, 0);
    glColor3f(0.12f, 0.08f, 0.15f);
    glVertex2f(g_winW, g_winH); glVertex2f(0, g_winH);
    glEnd();
    
    // Title
    glColor3f(r, g, b);
    glRasterPos2i(g_winW/2 - 100, g_winH/2 + 100);
    for (const char* c = title; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    
    // Subtitle
    glColor3f(1, 1, 1);
    glRasterPos2i(g_winW/2 - 120, g_winH/2 + 30);
    for (const char* c = sub; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    
    glEnable(GL_DEPTH_TEST);
}

// ============================================================================
// RAYCAST
// ============================================================================
GameEnemy* raycast() {
    GameEnemy* hit = nullptr;
    float best = 999;
    
    for (auto& e : g_enemies) {
        if (!e.alive() || e.state == GameEnemy::DEAD) continue;
        
        Vector3 toE = (e.pos + Vector3(0, 1, 0)) - g_player.cam.pos;
        float t = toE.dot(g_player.cam.front);
        if (t < 0 || t > 100) continue;
        
        Vector3 pt = g_player.cam.pos + g_player.cam.front * t;
        Vector3 diff = pt - (e.pos + Vector3(0, 1, 0));
        float d = diff.length();
        
        if (d < 1.3f && t < best) {
            best = t;
            hit = &e;
        }
    }
    return hit;
}

// ============================================================================
// UPDATE
// ============================================================================
void update(float dt) {
    if (g_state != PLAYING) return;
    
    auto& inp = InputManager::instance();
    
    g_player.update(dt);
    
    // Shooting
    if (inp.isMouseButtonDown(MouseButton::Left) && g_player.canFire()) {
        g_player.fire();
        
        GameEnemy* hit = raycast();
        if (hit) {
            hit->takeDamage(WEAPON_DAMAGE);
            g_hitMarker = 1.0f;
            
            if (!hit->alive()) {
                g_player.score += (hit->type == GameEnemy::DEVIL) ? 200 : 100;
                g_player.kills++;
            }
        }
    }
    
    if (g_hitMarker > 0) g_hitMarker -= dt * 5;
    
    // Update enemies
    for (auto& e : g_enemies) {
        e.update(dt, g_player);
    }
    
    // Update pickups
    for (auto& p : g_pickups) {
        p.update(dt);
        if (p.checkPickup(g_player.pos)) {
            if (p.type == Pickup::HEALTH) g_player.hp = std::min(g_player.hp + p.value, g_player.maxHp);
            else if (p.type == Pickup::AMMO) g_player.ammo = std::min(g_player.ammo + p.value, g_player.maxAmmo);
            else g_player.armor = std::min(g_player.armor + p.value, 100.0f);
            p.active = false;
        }
    }
    
    // Check win/lose
    if (!g_player.alive()) {
        g_state = DEAD;
        glutSetCursor(GLUT_CURSOR_INHERIT);
        g_mouseLock = false;
    }
    
    int alive = 0;
    for (auto& e : g_enemies) if (e.alive()) alive++;
    
    if (alive == 0) {
        if (g_level == 1) {
            g_state = NEXT_LEVEL;
        } else {
            g_state = WIN;
        }
        glutSetCursor(GLUT_CURSOR_INHERIT);
        g_mouseLock = false;
    }
}

// ============================================================================
// RENDER
// ============================================================================
void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (g_state == MENU) {
        drawMenuScreen("D O O M E R S", "Press SPACE to Start", 0.9f, 0.2f, 0.2f);
        
        glColor3f(0.7f, 0.7f, 0.7f);
        glRasterPos2i(g_winW/2 - 180, g_winH/2 - 50);
        const char* ctrl = "WASD:Move  Mouse:Look  LMB:Shoot  Space:Jump  Shift:Sprint";
        for (const char* c = ctrl; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        
    } else if (g_state == DEAD) {
        drawMenuScreen("YOU DIED", "Press SPACE to Restart", 0.8f, 0.1f, 0.1f);
        
        char txt[64];
        sprintf(txt, "Score: %d  Kills: %d", g_player.score, g_player.kills);
        glColor3f(0.8f, 0.8f, 0.8f);
        glRasterPos2i(g_winW/2 - 80, g_winH/2 - 50);
        for (char* c = txt; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        
    } else if (g_state == NEXT_LEVEL) {
        drawMenuScreen("LEVEL COMPLETE!", "Press SPACE for HELL ARENA", 0.2f, 0.9f, 0.3f);
        
        char txt[64];
        sprintf(txt, "Score: %d  Kills: %d", g_player.score, g_player.kills);
        glColor3f(0.8f, 0.8f, 0.8f);
        glRasterPos2i(g_winW/2 - 80, g_winH/2 - 50);
        for (char* c = txt; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        
    } else if (g_state == WIN) {
        drawMenuScreen("V I C T O R Y !", "You escaped the Doomers!", 1.0f, 0.85f, 0.2f);
        
        char txt[64];
        sprintf(txt, "Final Score: %d  Total Kills: %d", g_player.score, g_player.kills);
        glColor3f(1, 1, 1);
        glRasterPos2i(g_winW/2 - 100, g_winH/2 - 50);
        for (char* c = txt; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        
        glColor3f(0.7f, 0.7f, 0.7f);
        glRasterPos2i(g_winW/2 - 90, g_winH/2 - 90);
        const char* r = "Press SPACE to Play Again";
        for (const char* c = r; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        
    } else if (g_state == PLAYING || g_state == PAUSED) {
        // 3D View
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(75.0, (double)g_winW/g_winH, 0.1, 500.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        g_player.cam.apply();
        
        // Lighting
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        
        float lpos[] = {0, 12, 0, 1};
        float lamb[] = {0.35f, 0.35f, 0.4f, 1};
        float ldif[] = {0.8f, 0.78f, 0.72f, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, lpos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lamb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, ldif);
        
        // Flashlight
        glEnable(GL_LIGHT1);
        float fp[] = {g_player.cam.pos.x, g_player.cam.pos.y, g_player.cam.pos.z, 1};
        float fd[] = {g_player.cam.front.x, g_player.cam.front.y, g_player.cam.front.z};
        float fc[] = {1.0f, 0.95f, 0.85f, 1};
        glLightfv(GL_LIGHT1, GL_POSITION, fp);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, fd);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, fc);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 22.0f);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 15.0f);
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.02f);
        
        drawLevel();
        
        for (auto& e : g_enemies) e.draw();
        for (auto& p : g_pickups) p.draw();
        
        glDisable(GL_LIGHTING);
        drawWeapon();
        drawHUD();
        
        if (g_state == PAUSED) {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, g_winW, 0, g_winH);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            
            glEnable(GL_BLEND);
            glColor4f(0, 0, 0, 0.6f);
            glBegin(GL_QUADS);
            glVertex2f(0, 0); glVertex2f(g_winW, 0);
            glVertex2f(g_winW, g_winH); glVertex2f(0, g_winH);
            glEnd();
            
            glColor3f(1, 1, 0.3f);
            glRasterPos2i(g_winW/2 - 50, g_winH/2);
            const char* p = "PAUSED";
            for (const char* c = p; *c; c++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
            glDisable(GL_BLEND);
        }
    }
    
    glutSwapBuffers();
}

// ============================================================================
// CALLBACKS
// ============================================================================
void display() { render(); }

void reshape(int w, int h) {
    g_winW = w; g_winH = h;
    glViewport(0, 0, w, h);
}

void idle() {
    float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    g_dt = t - g_time;
    g_time = t;
    if (g_dt > 0.1f) g_dt = 0.1f;
    
    update(g_dt);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {
        if (g_state == PLAYING) {
            g_state = PAUSED;
            glutSetCursor(GLUT_CURSOR_INHERIT);
            g_mouseLock = false;
        } else if (g_state == PAUSED) {
            g_state = PLAYING;
            glutSetCursor(GLUT_CURSOR_NONE);
            g_mouseLock = true;
            glutWarpPointer(g_winW/2, g_winH/2);
        } else {
            exit(0);
        }
    }
    
    if (key == ' ') {
        if (g_state == MENU || g_state == DEAD) {
            startGame(1);
        } else if (g_state == NEXT_LEVEL) {
            int sc = g_player.score, k = g_player.kills;
            startGame(2);
            g_player.score = sc;
            g_player.kills = k;
        } else if (g_state == WIN) {
            startGame(1);
        }
    }
}

void keyboardUp(unsigned char k, int x, int y) {}
void special(int k, int x, int y) {}
void specialUp(int k, int x, int y) {}
void mouse(int b, int s, int x, int y) {}

void motion(int x, int y) {
    if (g_mouseLock && g_state == PLAYING) {
        int cx = g_winW/2, cy = g_winH/2;
        int dx = x - cx, dy = cy - y;
        if (dx || dy) {
            g_player.cam.rotate(dx, dy);
            glutWarpPointer(cx, cy);
        }
    }
}

void passiveMotion(int x, int y) { motion(x, y); }

// ============================================================================
// MAIN
// ============================================================================
int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "   D O O M E R S - Escape the Horde    " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD       - Move" << std::endl;
    std::cout << "  Mouse      - Look around" << std::endl;
    std::cout << "  Left Click - Shoot" << std::endl;
    std::cout << "  Space      - Jump" << std::endl;
    std::cout << "  Shift      - Sprint" << std::endl;
    std::cout << "  ESC        - Pause / Quit" << std::endl;
    std::cout << std::endl;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(g_winW, g_winH);
    glutCreateWindow("DOOMERS - Escape the Horde");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.06f, 0.06f, 0.1f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Try to detect asset path
    // First check if running from project root, then from Debug folder
    FILE* test = fopen("assets/scary-zombie-pack/zombie idle.fbx", "rb");
    if (test) {
        fclose(test);
        AssetManager::instance().setBasePath("");
        std::cout << "Assets found in current directory" << std::endl;
    } else {
        test = fopen("../assets/scary-zombie-pack/zombie idle.fbx", "rb");
        if (test) {
            fclose(test);
            AssetManager::instance().setBasePath("../");
            std::cout << "Assets found in parent directory" << std::endl;
        } else {
            // Last resort - absolute path
            AssetManager::instance().setBasePath("c:/Users/youss/Desktop/doomers/Doomers/");
            std::cout << "Using absolute path for assets" << std::endl;
        }
    }
    AssetManager::instance().loadAll();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passiveMotion);
    
    std::cout << "Game ready! Press SPACE in the window to start." << std::endl;
    
    glutMainLoop();
    return 0;
}
