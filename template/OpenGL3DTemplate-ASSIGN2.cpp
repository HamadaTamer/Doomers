#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <glut.h>
#pragma comment(lib, "winmm.lib")

// ==================== LAB 6 CAMERA SYSTEM (Required by Assignment) ====================
// MODIFICATION: Renamed Vector3f to Vector3 for consistency throughout the codebase
#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

class Vector3
{
public:
	float x, y, z;

	Vector3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3 operator+(Vector3 &v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 &v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(float n)
	{
		return Vector3(x * n, y * n, z * n);
	}

	Vector3 operator/(float n)
	{
		return Vector3(x / n, y / n, z / n);
	}

	Vector3 unit()
	{
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3 cross(Vector3 v)
	{
		return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera
{
public:
	Vector3 eye, center, up;

	Camera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f,
		   float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f,
		   float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f)
	{
		eye = Vector3(eyeX, eyeY, eyeZ);
		center = Vector3(centerX, centerY, centerZ);
		up = Vector3(upX, upY, upZ);
	}

	void moveX(float d)
	{
		Vector3 right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d)
	{
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d)
	{
		Vector3 view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a)
	{
		Vector3 view = (center - eye).unit();
		Vector3 right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a)
	{
		Vector3 view = (center - eye).unit();
		Vector3 right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look()
	{
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z);
	}
};

// Lab 6 Camera instance (used for free camera mode)
Camera camera;

// ==================== CONSTANTS ====================
#define PI 3.14159265
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

// Boundaries
#define FLOOR_SIZE 50.0f
#define WALL_HEIGHT 15.0f
#define CEILING_HEIGHT 12.0f
#define BOUNDARY 25.0f
#define PLATFORM_COUNT 8
#define MAX_ENEMIES 4
#define MAX_STARS 200
#define MAX_PARTICLES 100
#define MAX_MYSTERY_BOXES 5

// ==================== DATA STRUCTURES ====================
// Note: Vector3 class is defined above (from Lab 6)

// Forward declarations
float distance3D(Vector3 a, Vector3 b);
void spawnParticle(Vector3 pos, Vector3 vel, float r, float g, float b);
void initializeStars();

struct GameObject
{
	Vector3 pos;
	bool active;
	GameObject(float x = 0, float y = 0, float z = 0, bool isActive = false)
		: pos(x, y, z), active(isActive) {}
};

struct Platform
{
	Vector3 pos;
	Vector3 size;
	float colorR, colorG, colorB;
	Platform(float x = 0, float y = 0, float z = 0, float sx = 2, float sy = 0.3f, float sz = 2)
		: pos(x, y, z), size(sx, sy, sz), colorR(0.3f), colorG(0.3f), colorB(0.4f) {}
};

struct Enemy
{
	Vector3 pos;
	Vector3 vel;
	float rotY;
	bool active;
	float patrolMin, patrolMax;
	bool patrolAxis; // true = X, false = Z
	Enemy(float x = 0, float y = 0, float z = 0)
		: pos(x, y, z), vel(0.05f, 0, 0), rotY(0), active(true),
		  patrolMin(x - 3), patrolMax(x + 3), patrolAxis(true) {}
};

struct Star
{
	Vector3 pos;
	float size;
	float brightness;
	float twinkleSpeed;
	Star() : pos(0, 0, 0), size(0.05f), brightness(1.0f), twinkleSpeed(0.02f) {}
};

struct Planet
{
	Vector3 pos;
	float size;
	float r, g, b;
	float rotationSpeed;
	Planet() : pos(0, 0, 0), size(1.0f), r(1), g(1), b(1), rotationSpeed(0.1f) {}
};

struct Asteroid
{
	Vector3 pos;
	float size;
	float rotation;
	Asteroid() : pos(0, 0, 0), size(0.3f), rotation(0) {}
};

struct Particle
{
	Vector3 pos;
	Vector3 vel;
	float life;
	float size;
	float r, g, b;
	bool active;
	Particle() : pos(0, 0, 0), vel(0, 0, 0), life(0), size(0.1f),
				 r(1), g(1), b(1), active(false) {}
};

struct MysteryBox
{
	Vector3 pos;
	bool opened;
	bool hasCrystal;
	float rotation;
	float openAngle;
	MysteryBox(float x = 0, float y = 0, float z = 0, bool crystal = false)
		: pos(x, y, z), opened(false), hasCrystal(crystal), rotation(0), openAngle(0) {}
};

// ==================== GAME STATE ====================
enum GameState
{
	PLAYING,
	WIN,
	LOSE,
	INSTRUCTIONS
};
enum CameraMode
{
	FREE_CAM,
	FRONT_VIEW,
	SIDE_VIEW,
	TOP_VIEW
};

GameState gameState = INSTRUCTIONS;
CameraMode cameraMode = FRONT_VIEW;

// Camera position (global for lighting)
float camX = 0, camY = 10, camZ = 15;

// Mouse camera control
int lastMouseX = -1, lastMouseY = -1;
bool mouseControlActive = false;

// ==================== PLAYER ====================
Vector3 playerPos(-20, 0.8f, -20); // Start in safe corner away from enemies (scaled for 50x50)
Vector3 playerVel(0, 0, 0);
float playerRotY = 180.0f;
float playerRotX = 0.0f;
float playerSpeed = 0.25f;
float jumpForce = 0.38f; // Increased from 0.25 to reach higher platforms
float gravity = 0.015f;
bool isOnGround = true;
bool keysPressed[256] = {false};
int playerHealth = 100;
float damageFlash = 0.0f;
float invincibleTime = 0.0f;
bool loseSoundPlayed = false; // Prevent lose sound from playing multiple times
// Platform bookkeeping to avoid landing jitter
int currentPlatform = -1;
float playerBaseHeight = 0.8f;

// ==================== CAMERA ====================
float cameraAngleH = 0.0f;		  // Horizontal angle (used for particle billboarding)
const float CAM_BOUNDARY = 80.0f; // Camera boundary for FREE_CAM (planets visible at ~60-120 units)

// ==================== GAME OBJECTS ====================
// Mix of mystery box crystals (inactive, spawn on box open) and world crystals (active, placed randomly on floor)
GameObject crystals[5] = {
	// Mystery box crystals (inactive - spawn when box opens) - 2 slots
	GameObject(0, 0, 0, false),
	GameObject(0, 0, 0, false),
	// World crystals (active - placed randomly on floor) - 3 total
	GameObject(-12, 0.8f, 8, true), // Random floor position 1
	GameObject(8, 0.8f, -15, true), // Random floor position 2
	GameObject(15, 0.8f, 18, true)	// Random floor position 3
};

Platform platforms[PLATFORM_COUNT] = {
	// ZONE 1: STARTING SAFE AREA (Southwest quadrant) - Raised higher
	Platform(-18, 2.5f, -18, 5.0f, 0.4f, 5.0f), // Spawn landing pad (large & safe)
	Platform(-13, 2.3f, -15, 4.0f, 0.4f, 4.0f), // First step toward center

	// ZONE 2: CENTRAL HUB (Easy jumps, main navigation) - Raised higher
	Platform(-7, 2.8f, -10, 4.5f, 0.4f, 4.5f), // Mid-west platform
	Platform(0, 2.6f, -7, 5.0f, 0.4f, 4.0f),   // Central command platform (large)
	Platform(7, 3.1f, -5, 4.0f, 0.4f, 4.0f),   // East-center elevated

	// ZONE 3: NORTH PATH (Progressive difficulty, enemy territory) - Raised higher
	Platform(0, 3.3f, 5, 3.5f, 0.4f, 3.5f),	  // North-center (requires good jump)
	Platform(10, 2.8f, 10, 4.0f, 0.4f, 4.0f), // Northeast platform (near enemy)
	Platform(-10, 3.5f, 12, 3.5f, 0.4f, 3.5f) // Northwest high platform (challenge)
};

Enemy enemies[MAX_ENEMIES] = {
	// Strategic placement - guards key areas
	Enemy(15, 1.0f, -10), // East guard - blocks eastern route
	Enemy(-15, 1.0f, 10), // West-north patroller
	Enemy(10, 1.0f, 15),  // North-east guardian (guards crystal)
	Enemy(0, 1.0f, 20)	  // Far north sentry (hardest area)
};

Star stars[MAX_STARS];
Particle particles[MAX_PARTICLES];
Planet planets[5];
Asteroid asteroids[30];

MysteryBox mysteryBoxes[MAX_MYSTERY_BOXES] = {
	// ALL BOXES CONTAIN CRYSTALS - equal weight collectables
	// TUTORIAL BOX: First easy crystal at spawn area (teaches mechanics)
	MysteryBox(-13, 2.9f, -15, true), // HAS CRYSTAL - accessible from platform 2 (raised +2.3)

	// CENTRAL HUB: Both contain crystals
	MysteryBox(0, 3.2f, -7, true), // HAS CRYSTAL - on large central platform (raised +1.4)
	MysteryBox(7, 3.7f, -5, true), // HAS CRYSTAL - on elevated platform (raised +1.4)

	// ENEMY TERRITORY: High risk, high reward
	MysteryBox(10, 3.4f, 10, true), // HAS CRYSTAL - near northeast enemy (raised +1.4)
	MysteryBox(-5, 1.9f, 15, true)	// HAS CRYSTAL - north-west danger zone (raised +1.3)
};

// ==================== ANIMATIONS ====================
float crystalRotation = 0.0f;
float crystalBob = 0.0f;
float wallColorTime = 0.0f;
float starTwinkle = 0.0f;
float nebulaShift = 0.0f;

// Animation states for objects
bool consoleAnimating = false;
bool airlockAnimating = false;
bool containerAnimating = false;
bool dishAnimating = false;
bool tankAnimating = false;

float consoleScale = 1.0f;
float airlockDoorOffset = 0.0f;
float containerLidAngle = 0.0f;
float dishRotation = 0.0f;
float tankRotation = 0.0f;
float tankScale = 1.0f;

// Animation directions
int airlockDirection = 1;
int containerDirection = 1;

// Interactive object positions (VISIBLE - placed around the station at various heights)
Vector3 consolePos(-15, 1.5f, 0);	 // West wall area - standing console
Vector3 airlockPos(0, 4.0f, -20);	 // North wall - elevated airlock door
Vector3 containerPos(20, 1.2f, -15); // East area - storage container on ground
Vector3 dishPos(-18, 2.5f, 18);		 // Southwest corner - elevated communication dish
Vector3 tankPos(15, 1.8f, 20);		 // Southeast corner - fuel tank standing

// ==================== TIMER ====================
int gameTime = 90;
int lastTime = 0;
int collectedCrystals = 0;
int score = 0;

// ==================== SOUND SYSTEM ====================
const char *SND_BACKGROUND = "songs\\BACKGROUND_MUSIC.mp3";
const char *SND_COLLECT = "songs\\CRYSTAL_COLLECT.mp3";
const char *SND_JUMP = "songs\\AMBIENT_BEEP.mp3";
const char *SND_HIT = "songs\\AMBIENT_BEEP.mp3";
const char *SND_WIN = "songs\\WIN_SOUND.mp3";
const char *SND_LOSE = "songs\\LOSE_SOUND.mp3";
const char *SND_MYSTERY = "songs\\CONTAINER_OPEN.mp3";

void playSound(const char *filename, bool loop = false)
{
	static int soundId = 0;
	char alias[256];
	char command[512];

	if (loop)
	{
		// Background music - use MCI with repeat
		sprintf(alias, "bgmusic");
		sprintf(command, "close %s", alias);
		mciSendStringA(command, NULL, 0, NULL); // Close if already open

		sprintf(command, "open \"%s\" type mpegvideo alias %s", filename, alias);
		if (mciSendStringA(command, NULL, 0, NULL) == 0)
		{
			sprintf(command, "play %s repeat", alias);
			mciSendStringA(command, NULL, 0, NULL);
		}
	}
	else
	{
		// Sound effects - use MCI for simultaneous playback
		sprintf(alias, "sfx%d", soundId);

		// Close previous sound with this alias
		sprintf(command, "close %s", alias);
		mciSendStringA(command, NULL, 0, NULL);

		// Open and play new sound
		sprintf(command, "open \"%s\" type mpegvideo alias %s", filename, alias);
		if (mciSendStringA(command, NULL, 0, NULL) == 0)
		{
			sprintf(command, "play %s", alias);
			mciSendStringA(command, NULL, 0, NULL);
		}

		// Cycle through aliases
		soundId = (soundId + 1) % 15;
	}
}

void print(float x, float y, const char *string)
{
	glRasterPos2f(x, y);
	int len = strlen(string);
	for (int i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
	}
}

void printLarge(float x, float y, const char *string)
{
	glRasterPos2f(x, y);
	int len = strlen(string);
	for (int i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

float distance3D(Vector3 a, Vector3 b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return sqrt(dx * dx + dy * dy + dz * dz);
}

void initializeStars()
{
	for (int i = 0; i < MAX_STARS; i++)
	{
		stars[i].pos.x = ((float)rand() / RAND_MAX) * 100.0f - 50.0f; // Scale to 50x50 world
		stars[i].pos.y = ((float)rand() / RAND_MAX) * 30.0f;
		stars[i].pos.z = ((float)rand() / RAND_MAX) * 100.0f - 50.0f; // Scale to 50x50 world
		stars[i].size = ((float)rand() / RAND_MAX) * 0.08f + 0.02f;
		stars[i].brightness = ((float)rand() / RAND_MAX) * 0.5f + 0.5f;
		stars[i].twinkleSpeed = ((float)rand() / RAND_MAX) * 0.04f + 0.01f;
	}

	// Initialize planets CLOSER and HIGHER - visible within camera bounds
	planets[0].pos = Vector3(-35, 30, -45); // Closer and much higher
	planets[0].size = 6.0f;					// Slightly smaller for better visibility
	planets[0].r = 1.0f;
	planets[0].g = 0.6f;
	planets[0].b = 0.3f; // Orange gas giant
	planets[0].rotationSpeed = 0.05f;

	planets[1].pos = Vector3(40, 35, -50); // Closer and much higher
	planets[1].size = 4.0f;
	planets[1].r = 0.4f;
	planets[1].g = 0.6f;
	planets[1].b = 0.9f; // Blue ice planet
	planets[1].rotationSpeed = 0.08f;

	planets[2].pos = Vector3(-30, 25, 50); // Closer and much higher
	planets[2].size = 3.0f;
	planets[2].r = 0.8f;
	planets[2].g = 0.3f;
	planets[2].b = 0.3f; // Red rocky planet
	planets[2].rotationSpeed = 0.12f;

	planets[3].pos = Vector3(35, 40, 45); // Closer and much higher
	planets[3].size = 7.0f;
	planets[3].r = 0.9f;
	planets[3].g = 0.8f;
	planets[3].b = 0.5f; // Yellow ringed planet
	planets[3].rotationSpeed = 0.03f;

	planets[4].pos = Vector3(0, 50, -55); // Closer and highest - visible at top
	planets[4].size = 5.0f;
	planets[4].r = 0.6f;
	planets[4].g = 0.9f;
	planets[4].b = 0.7f; // Green mysterious planet
	planets[4].rotationSpeed = 0.07f;

	// Initialize asteroids scattered around
	for (int i = 0; i < 30; i++)
	{
		asteroids[i].pos.x = ((float)rand() / RAND_MAX) * 140.0f - 70.0f;
		asteroids[i].pos.y = ((float)rand() / RAND_MAX) * 40.0f - 5.0f;
		asteroids[i].pos.z = ((float)rand() / RAND_MAX) * 140.0f - 70.0f;
		asteroids[i].size = ((float)rand() / RAND_MAX) * 0.6f + 0.2f;
		asteroids[i].rotation = ((float)rand() / RAND_MAX) * 360.0f;
	}
}

void spawnParticle(Vector3 pos, Vector3 vel, float r, float g, float b)
{
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (!particles[i].active)
		{
			particles[i].pos = pos;
			particles[i].vel = vel;
			particles[i].life = 1.0f;
			particles[i].size = 0.15f;
			particles[i].r = r;
			particles[i].g = g;
			particles[i].b = b;
			particles[i].active = true;
			break;
		}
	}
}

// ==================== DRAWING FUNCTIONS ====================

void drawPlanets()
{
	glEnable(GL_LIGHTING);
	for (int i = 0; i < 5; i++)
	{
		glPushMatrix();
		glTranslatef(planets[i].pos.x, planets[i].pos.y, planets[i].pos.z);
		glRotatef(starTwinkle * planets[i].rotationSpeed, 0, 1, 0);
		glColor3f(planets[i].r, planets[i].g, planets[i].b);
		glutSolidSphere(planets[i].size, 30, 30);

		// Add ring to planet 3 (yellow ringed planet)
		if (i == 3)
		{
			glPushMatrix();
			glRotatef(75, 1, 0, 0);
			glColor4f(0.8f, 0.7f, 0.4f, 0.7f);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glutSolidTorus(0.5f, planets[i].size * 1.5f, 20, 40);
			glDisable(GL_BLEND);
			glPopMatrix();
		}
		glPopMatrix();
	}
}

void drawAsteroids()
{
	glDisable(GL_LIGHTING);
	glColor3f(0.5f, 0.5f, 0.5f);
	for (int i = 0; i < 30; i++)
	{
		glPushMatrix();
		glTranslatef(asteroids[i].pos.x, asteroids[i].pos.y, asteroids[i].pos.z);
		glRotatef(asteroids[i].rotation + starTwinkle * 0.2f, 1, 1, 0);
		glScalef(asteroids[i].size, asteroids[i].size * 0.8f, asteroids[i].size * 1.1f);
		glutSolidDodecahedron();
		glPopMatrix();
	}
	glEnable(GL_LIGHTING);
}

void drawStarfield()
{
	glDisable(GL_LIGHTING);
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	for (int i = 0; i < MAX_STARS; i++)
	{
		float twinkle = sin(starTwinkle * stars[i].twinkleSpeed + i) * 0.3f + 0.7f;
		float brightness = stars[i].brightness * twinkle;
		glColor3f(brightness, brightness, brightness * 1.1f);
		glVertex3f(stars[i].pos.x, stars[i].pos.y, stars[i].pos.z);
	}
	glEnd();
	glEnable(GL_LIGHTING);
}

void drawNebula()
{
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Purple nebula clouds in background
	for (int i = 0; i < 8; i++)
	{
		float angle = i * 45.0f;
		float rad = angle * PI / 180.0f;
		float x = cos(rad + nebulaShift) * 18.0f;
		float z = sin(rad + nebulaShift) * 18.0f;
		float pulse = sin(nebulaShift * 0.5f + i) * 0.2f + 0.8f;

		glPushMatrix();
		glTranslatef(x, 8.0f, z);
		glRotatef(angle, 0, 1, 0);

		// Multiple overlapping quads for cloud effect
		for (int j = 0; j < 3; j++)
		{
			float offset = j * 0.3f;
			glColor4f(0.4f + offset * 0.2f, 0.1f, 0.6f + offset * 0.1f, 0.08f * pulse);
			glBegin(GL_QUADS);
			glVertex3f(-4 - offset, -3 - offset, 0);
			glVertex3f(4 + offset, -3 - offset, 0);
			glVertex3f(4 + offset, 3 + offset, 0);
			glVertex3f(-4 - offset, 3 + offset, 0);
			glEnd();
		}
		glPopMatrix();
	}

	// Blue energy wisps
	for (int i = 0; i < 6; i++)
	{
		float angle = i * 60.0f + nebulaShift * 20.0f;
		float rad = angle * PI / 180.0f;
		float x = cos(rad) * 15.0f;
		float z = sin(rad) * 15.0f;

		glPushMatrix();
		glTranslatef(x, 5.0f, z);
		glColor4f(0.1f, 0.4f, 0.8f, 0.12f);
		glBegin(GL_QUADS);
		glVertex3f(-2, -2, 0);
		glVertex3f(2, -2, 0);
		glVertex3f(2, 2, 0);
		glVertex3f(-2, 2, 0);
		glEnd();
		glPopMatrix();
	}

	glEnable(GL_LIGHTING);
}

void drawParticles()
{
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (particles[i].active && particles[i].life > 0)
		{
			glPushMatrix();
			glTranslatef(particles[i].pos.x, particles[i].pos.y, particles[i].pos.z);

			// Billboard to face camera
			glRotatef(-cameraAngleH, 0, 1, 0);

			float alpha = particles[i].life;
			glColor4f(particles[i].r, particles[i].g, particles[i].b, alpha);

			float s = particles[i].size * particles[i].life;
			glBegin(GL_QUADS);
			glVertex3f(-s, -s, 0);
			glVertex3f(s, -s, 0);
			glVertex3f(s, s, 0);
			glVertex3f(-s, s, 0);
			glEnd();

			glPopMatrix();
		}
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
}

void drawFloor()
{
	glPushMatrix();
	glTranslatef(0, -0.1f, 0);

	// Main metallic floor with subtle gradient
	glBegin(GL_QUADS);
	glColor3f(0.15f, 0.15f, 0.18f); // Lighter at edges
	glVertex3f(-FLOOR_SIZE / 2, 0, -FLOOR_SIZE / 2);
	glVertex3f(FLOOR_SIZE / 2, 0, -FLOOR_SIZE / 2);
	glColor3f(0.10f, 0.10f, 0.13f); // Darker in center
	glVertex3f(FLOOR_SIZE / 2, 0, FLOOR_SIZE / 2);
	glVertex3f(-FLOOR_SIZE / 2, 0, FLOOR_SIZE / 2);
	glEnd();

	glDisable(GL_LIGHTING);

	// Large hexagonal panels with depth
	glLineWidth(3.0f);
	float gridSize = 2.5f;
	for (float x = -FLOOR_SIZE / 2; x < FLOOR_SIZE / 2; x += gridSize)
	{
		for (float z = -FLOOR_SIZE / 2; z < FLOOR_SIZE / 2; z += gridSize)
		{
			float distFromCenter = sqrt(x * x + z * z);
			float pulse = sin(wallColorTime + distFromCenter * 0.15f) * 0.25f + 0.75f;

			// Outer glow
			glColor3f(0.0f, 0.4f * pulse, 0.7f * pulse);
			glBegin(GL_LINE_LOOP);
			for (int i = 0; i < 6; i++)
			{
				float angle = i * 60.0f * PI / 180.0f;
				float hx = x + cos(angle) * 1.0f;
				float hz = z + sin(angle) * 1.0f;
				glVertex3f(hx, 0.01f, hz);
			}
			glEnd();

			// Inner detail
			glLineWidth(1.5f);
			glColor3f(0.2f * pulse, 0.6f * pulse, 1.0f * pulse);
			glBegin(GL_LINE_LOOP);
			for (int i = 0; i < 6; i++)
			{
				float angle = i * 60.0f * PI / 180.0f;
				float hx = x + cos(angle) * 0.6f;
				float hz = z + sin(angle) * 0.6f;
				glVertex3f(hx, 0.015f, hz);
			}
			glEnd();
			glLineWidth(3.0f);
		}
	}

	// Glowing energy conduits
	glLineWidth(3.5f);
	for (int i = -7; i <= 7; i++)
	{
		float offset = i * 2.0f;
		float pulse = sin(wallColorTime * 3 + i * 0.8f) * 0.3f + 0.7f;

		// Outer glow
		glColor4f(0.0f, 0.5f * pulse, 0.8f * pulse, 0.4f);
		glBegin(GL_LINES);
		glVertex3f(offset, 0.025f, -FLOOR_SIZE / 2);
		glVertex3f(offset, 0.025f, FLOOR_SIZE / 2);
		glEnd();

		// Bright core
		glLineWidth(1.5f);
		glColor3f(0.4f, 1.0f * pulse, 1.0f);
		glBegin(GL_LINES);
		glVertex3f(offset, 0.03f, -FLOOR_SIZE / 2);
		glVertex3f(offset, 0.03f, FLOOR_SIZE / 2);
		glEnd();
		glLineWidth(3.5f);
	}

	// Cross energy lines
	glLineWidth(3.0f);
	for (int i = -7; i <= 7; i++)
	{
		float offset = i * 2.0f;
		float pulse = sin(wallColorTime * 3 + i * 0.8f + 1.5f) * 0.3f + 0.7f;

		glColor4f(0.0f, 0.5f * pulse, 0.8f * pulse, 0.4f);
		glBegin(GL_LINES);
		glVertex3f(-FLOOR_SIZE / 2, 0.025f, offset);
		glVertex3f(FLOOR_SIZE / 2, 0.025f, offset);
		glEnd();

		glLineWidth(1.5f);
		glColor3f(0.4f, 1.0f * pulse, 1.0f);
		glBegin(GL_LINES);
		glVertex3f(-FLOOR_SIZE / 2, 0.03f, offset);
		glVertex3f(FLOOR_SIZE / 2, 0.03f, offset);
		glEnd();
		glLineWidth(3.0f);
	}

	// Tech corner markers
	for (float x = -FLOOR_SIZE / 2 + 2; x < FLOOR_SIZE / 2; x += 4.0f)
	{
		for (float z = -FLOOR_SIZE / 2 + 2; z < FLOOR_SIZE / 2; z += 4.0f)
		{
			float pulse = sin(wallColorTime * 4 + x + z) * 0.5f + 0.5f;
			glPointSize(4.0f);
			glBegin(GL_POINTS);
			glColor3f(1.0f, 0.8f * pulse, 0.3f * pulse);
			glVertex3f(x, 0.035f, z);
			glEnd();
		}
	}

	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void drawWall(float x, float z, float rotY, float r, float g, float b)
{
	glPushMatrix();
	glTranslatef(x, WALL_HEIGHT / 2, z);
	glRotatef(rotY, 0, 1, 0);

	// Layered wall panels for depth
	// Back layer - VISIBLE ANIMATED COLORS
	glPushMatrix();
	glScalef(FLOOR_SIZE, WALL_HEIGHT, 0.3f);
	glColor3f(r * 0.6f, g * 0.6f, b * 0.8f); // Much brighter, visible animation
	glutSolidCube(1.0f);
	glPopMatrix();

	// Mid layer - paneling effect with ANIMATED COLORS
	for (int i = -6; i <= 6; i++)
	{
		float px = i * 2.3f;
		glPushMatrix();
		glTranslatef(px, 0, 0.2f);
		glScalef(2.0f, WALL_HEIGHT - 0.5f, 0.25f);
		float shade = (i % 2 == 0) ? 0.15f : 0.18f;
		glColor3f(shade + r * 0.5f, shade + g * 0.5f, shade + b * 0.6f); // Brighter animation
		glutSolidCube(1.0f);
		glPopMatrix();

		// Panel borders
		glDisable(GL_LIGHTING);
		glColor3f(0.15f, 0.18f, 0.25f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		glVertex3f(px - 1.0f, -WALL_HEIGHT / 2 + 0.25f, 0.35f);
		glVertex3f(px + 1.0f, -WALL_HEIGHT / 2 + 0.25f, 0.35f);
		glVertex3f(px + 1.0f, WALL_HEIGHT / 2 - 0.25f, 0.35f);
		glVertex3f(px - 1.0f, WALL_HEIGHT / 2 - 0.25f, 0.35f);
		glEnd();
		glEnable(GL_LIGHTING);
	}

	glDisable(GL_LIGHTING);

	// Large viewport windows
	for (int i = -1; i <= 1; i++)
	{
		float wx = i * 8.0f;
		float pulse = sin(wallColorTime * 0.5f + i) * 0.2f + 0.8f;

		// Window frame with beveled edges
		glColor3f(0.25f, 0.28f, 0.35f);
		glLineWidth(4.0f);
		glBegin(GL_LINE_LOOP);
		glVertex3f(wx - 2.5f, 1.0f, 0.4f);
		glVertex3f(wx + 2.5f, 1.0f, 0.4f);
		glVertex3f(wx + 2.5f, 5.0f, 0.4f);
		glVertex3f(wx - 2.5f, 5.0f, 0.4f);
		glEnd();

		// Inner frame
		glColor3f(0.35f, 0.38f, 0.45f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		glVertex3f(wx - 2.3f, 1.2f, 0.41f);
		glVertex3f(wx + 2.3f, 1.2f, 0.41f);
		glVertex3f(wx + 2.3f, 4.8f, 0.41f);
		glVertex3f(wx - 2.3f, 4.8f, 0.41f);
		glEnd();

		// Window "glass" with space reflection
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.05f, 0.15f, 0.35f * pulse, 0.4f);
		glBegin(GL_QUADS);
		glVertex3f(wx - 2.3f, 1.2f, 0.42f);
		glVertex3f(wx + 2.3f, 1.2f, 0.42f);
		glVertex3f(wx + 2.3f, 4.8f, 0.42f);
		glVertex3f(wx - 2.3f, 4.8f, 0.42f);
		glEnd();

		// Window divider (cross)
		glColor3f(0.3f, 0.35f, 0.4f);
		glLineWidth(3.0f);
		glBegin(GL_LINES);
		glVertex3f(wx, 1.2f, 0.43f);
		glVertex3f(wx, 4.8f, 0.43f);
		glVertex3f(wx - 2.3f, 3.0f, 0.43f);
		glVertex3f(wx + 2.3f, 3.0f, 0.43f);
		glEnd();

		// Glowing accent around window
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glColor4f(0.2f * pulse, 0.6f * pulse, 1.0f * pulse, 0.3f);
		glLineWidth(3.0f);
		glBegin(GL_LINE_LOOP);
		glVertex3f(wx - 2.6f, 0.9f, 0.44f);
		glVertex3f(wx + 2.6f, 0.9f, 0.44f);
		glVertex3f(wx + 2.6f, 5.1f, 0.44f);
		glVertex3f(wx - 2.6f, 5.1f, 0.44f);
		glEnd();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Control panels with indicator lights
	for (int i = -5; i <= 5; i++)
	{
		if (i >= -1 && i <= 1)
			continue; // Skip window areas

		float px = i * 2.5f;
		float lightPulse = sin(wallColorTime * 4 + i * 1.2f) * 0.5f + 0.5f;

		// Panel housing
		glColor3f(0.18f, 0.20f, 0.25f);
		glBegin(GL_QUADS);
		glVertex3f(px - 0.7f, -3.0f, 0.45f);
		glVertex3f(px + 0.7f, -3.0f, 0.45f);
		glVertex3f(px + 0.7f, -1.0f, 0.45f);
		glVertex3f(px - 0.7f, -1.0f, 0.45f);
		glEnd();

		// Panel screen effect
		glColor3f(0.05f, 0.15f * lightPulse, 0.25f * lightPulse);
		glBegin(GL_QUADS);
		glVertex3f(px - 0.6f, -2.8f, 0.46f);
		glVertex3f(px + 0.6f, -2.8f, 0.46f);
		glVertex3f(px + 0.6f, -1.2f, 0.46f);
		glVertex3f(px - 0.6f, -1.2f, 0.46f);
		glEnd();

		// Status indicator lights
		float statusColors[3][3] = {
			{0.2f, 1.0f, 0.3f}, // Green
			{1.0f, 0.8f, 0.2f}, // Amber
			{1.0f, 0.3f, 0.3f}	// Red
		};
		int colorIdx = abs(i) % 3;

		glPointSize(6.0f);
		glBegin(GL_POINTS);
		if (lightPulse > 0.5f)
		{
			glColor3f(statusColors[colorIdx][0], statusColors[colorIdx][1], statusColors[colorIdx][2]);
		}
		else
		{
			glColor3f(statusColors[colorIdx][0] * 0.3f, statusColors[colorIdx][1] * 0.3f, statusColors[colorIdx][2] * 0.3f);
		}
		glVertex3f(px, -1.5f, 0.47f);
		glEnd();

		// Holographic panel grid
		glColor4f(0.3f * lightPulse, 0.7f * lightPulse, 1.0f * lightPulse, 0.3f);
		glLineWidth(1.0f);
		for (float gy = -2.7f; gy < -1.3f; gy += 0.3f)
		{
			glBegin(GL_LINES);
			glVertex3f(px - 0.55f, gy, 0.465f);
			glVertex3f(px + 0.55f, gy, 0.465f);
			glEnd();
		}
	}

	// Decorative rivet details
	glPointSize(3.0f);
	glColor3f(0.35f, 0.38f, 0.42f);
	glBegin(GL_POINTS);
	for (float px = -FLOOR_SIZE / 2 + 1; px < FLOOR_SIZE / 2; px += 1.5f)
	{
		glVertex3f(px, WALL_HEIGHT / 2 - 0.5f, 0.45f);
		glVertex3f(px, -WALL_HEIGHT / 2 + 0.5f, 0.45f);
	}
	glEnd();

	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void drawPlayer()
{
	glPushMatrix();
	glTranslatef(playerPos.x, playerPos.y, playerPos.z);
	glRotatef(playerRotY, 0, 1, 0);
	glRotatef(playerRotX, 1, 0, 0);
	glTranslatef(0, 0.5f, 0);

	// Apply damage flash effect
	float flashMult = 1.0f - damageFlash * 0.7f;

	// ===== REDESIGNED ASTRONAUT - Dark Blue Space Suit =====

	// Define color scheme - Professional space suit colors
	float suitBlue[3] = {0.15f * flashMult, 0.25f * flashMult, 0.45f * flashMult};	// Dark navy blue
	float suitGray[3] = {0.35f * flashMult, 0.38f * flashMult, 0.42f * flashMult};	// Metal gray
	float visorTint[3] = {0.05f, 0.15f * flashMult, 0.25f * flashMult};				// Dark tinted visor
	float accentOrange[3] = {0.9f * flashMult, 0.5f * flashMult, 0.1f * flashMult}; // Safety orange accents

	// HELMET - Gray metallic sphere (larger, better proportions)
	glPushMatrix();
	glTranslatef(0, 0.85f, 0);
	glColor3f(suitGray[0], suitGray[1], suitGray[2]);
	glutSolidSphere(0.28f, 20, 20);
	glPopMatrix();

	// VISOR - Large dark curved glass
	glPushMatrix();
	glTranslatef(0, 0.85f, 0.22f);
	glScalef(1.0f, 0.9f, 0.5f);
	glColor3f(visorTint[0], visorTint[1], visorTint[2]);
	glutSolidSphere(0.22f, 16, 16);
	glPopMatrix();

	// NECK RING - Orange safety ring
	glPushMatrix();
	glTranslatef(0, 0.58f, 0);
	glRotatef(90, 1, 0, 0);
	glColor3f(accentOrange[0], accentOrange[1], accentOrange[2]);
	glutSolidTorus(0.05f, 0.25f, 12, 16);
	glPopMatrix();

	// TORSO - Dark blue suit body (wider, more realistic)
	glPushMatrix();
	glTranslatef(0, 0.15f, 0);
	glScalef(0.6f, 0.75f, 0.45f);
	glColor3f(suitBlue[0], suitBlue[1], suitBlue[2]);
	glutSolidCube(1.0f);
	glPopMatrix();

	// CHEST CONTROL PANEL - Small gray rectangle
	glPushMatrix();
	glTranslatef(0, 0.25f, 0.24f);
	glScalef(0.25f, 0.2f, 0.05f);
	glColor3f(suitGray[0] * 1.2f, suitGray[1] * 1.2f, suitGray[2] * 1.2f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// CHEST INDICATOR LIGHTS - Small colored dots
	glDisable(GL_LIGHTING);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	glColor3f(0.2f, 1.0f, 0.3f); // Green light
	glVertex3f(-0.08f, 0.28f, 0.27f);
	glColor3f(1.0f, 0.8f, 0.2f); // Amber light
	glVertex3f(0.0f, 0.28f, 0.27f);
	glColor3f(0.3f, 0.6f, 1.0f); // Blue light
	glVertex3f(0.08f, 0.28f, 0.27f);
	glEnd();
	glEnable(GL_LIGHTING);

	// BACKPACK - Large gray life support system
	glPushMatrix();
	glTranslatef(0, 0.25f, -0.3f);
	glScalef(0.5f, 0.65f, 0.25f);
	glColor3f(suitGray[0], suitGray[1], suitGray[2]);
	glutSolidCube(1.0f);
	glPopMatrix();

	// BACKPACK TANKS - Two cylindrical oxygen tanks
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.12f : 0.12f;

		glPushMatrix();
		glTranslatef(side, 0.35f, -0.35f);
		glRotatef(90, 1, 0, 0);
		GLUquadricObj *tank = gluNewQuadric();
		glColor3f(suitGray[0] * 1.3f, suitGray[1] * 1.3f, suitGray[2] * 1.3f);
		gluCylinder(tank, 0.08f, 0.08f, 0.4f, 12, 5);
		// Tank caps
		glColor3f(accentOrange[0], accentOrange[1], accentOrange[2]);
		gluDisk(tank, 0, 0.08f, 12, 1);
		glTranslatef(0, 0, 0.4f);
		gluDisk(tank, 0, 0.08f, 12, 1);
		gluDeleteQuadric(tank);
		glPopMatrix();
	}

	// SHOULDERS - Dark blue rounded
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.42f : 0.42f;

		glPushMatrix();
		glTranslatef(side, 0.4f, 0);
		glColor3f(suitBlue[0], suitBlue[1], suitBlue[2]);
		glutSolidSphere(0.16f, 12, 12);
		glPopMatrix();
	}

	// ARMS - Dark blue with orange stripes
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.45f : 0.45f;
		float rotDir = (i == 0) ? 90 : -90;

		// Upper arm
		glPushMatrix();
		glTranslatef(side, 0.28f, 0);
		glRotatef(rotDir, 0, 0, 1);
		GLUquadricObj *arm = gluNewQuadric();
		glColor3f(suitBlue[0], suitBlue[1], suitBlue[2]);
		gluCylinder(arm, 0.11f, 0.09f, 0.35f, 12, 5);

		// Orange stripe on arm
		glTranslatef(0, 0, 0.15f);
		glColor3f(accentOrange[0], accentOrange[1], accentOrange[2]);
		gluCylinder(arm, 0.12f, 0.10f, 0.06f, 12, 5);

		// Forearm
		glTranslatef(0, 0, 0.06f);
		glColor3f(suitBlue[0] * 0.9f, suitBlue[1] * 0.9f, suitBlue[2] * 0.9f);
		gluCylinder(arm, 0.09f, 0.08f, 0.3f, 12, 5);

		gluDeleteQuadric(arm);
		glPopMatrix();
	}

	// GLOVES - Gray with orange trim
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.45f : 0.45f;
		float rotDir = (i == 0) ? 90 : -90;

		glPushMatrix();
		glTranslatef(side, 0.28f, 0);
		glRotatef(rotDir, 0, 0, 1);
		glTranslatef(0, 0, 0.72f);

		// Glove base
		glColor3f(suitGray[0], suitGray[1], suitGray[2]);
		glutSolidSphere(0.1f, 12, 12);

		// Orange wrist ring
		glColor3f(accentOrange[0], accentOrange[1], accentOrange[2]);
		glutSolidTorus(0.02f, 0.09f, 8, 12);
		glPopMatrix();
	}

	// WAIST BELT - Orange utility belt
	glPushMatrix();
	glTranslatef(0, -0.22f, 0);
	glRotatef(90, 1, 0, 0);
	glColor3f(accentOrange[0], accentOrange[1], accentOrange[2]);
	glutSolidTorus(0.06f, 0.32f, 12, 16);
	glPopMatrix();

	// BELT POUCHES - Small gray boxes
	for (int i = -1; i <= 1; i += 2)
	{
		glPushMatrix();
		glTranslatef(i * 0.25f, -0.22f, 0.18f);
		glScalef(0.12f, 0.12f, 0.1f);
		glColor3f(suitGray[0] * 0.8f, suitGray[1] * 0.8f, suitGray[2] * 0.8f);
		glutSolidCube(1.0f);
		glPopMatrix();
	}

	// LEGS - Dark blue with knee joints (NOT WHITE!)
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.18f : 0.18f;

		// Upper thigh (dark blue)
		glPushMatrix();
		glTranslatef(side, -0.22f, 0); // Start just below belt
		glScalef(0.13f, 0.35f, 0.13f); // Vertical cylinder shape
		glColor3f(suitBlue[0], suitBlue[1], suitBlue[2]);
		glutSolidCube(1.0f);
		glPopMatrix();

		// Knee joint (orange sphere)
		glPushMatrix();
		glTranslatef(side, -0.57f, 0); // Middle of leg
		glColor3f(accentOrange[0], accentOrange[1], accentOrange[2]);
		glutSolidSphere(0.13f, 12, 12);
		glPopMatrix();

		// Lower leg (darker blue)
		glPushMatrix();
		glTranslatef(side, -0.75f, 0); // Below knee
		glScalef(0.12f, 0.3f, 0.12f);  // Vertical cylinder shape
		glColor3f(suitBlue[0] * 0.8f, suitBlue[1] * 0.8f, suitBlue[2] * 0.8f);
		glutSolidCube(1.0f);
		glPopMatrix();
	}

	// BOOTS - Gray magnetic boots
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.18f : 0.18f;

		glPushMatrix();
		glTranslatef(side, -0.96f, 0.08f);
		glScalef(0.16f, 0.12f, 0.25f);
		glColor3f(suitGray[0] * 0.7f, suitGray[1] * 0.7f, suitGray[2] * 0.7f);
		glutSolidCube(1.0f);
		glPopMatrix();

		// Boot sole (orange tread)
		glPushMatrix();
		glTranslatef(side, -1.02f, 0.08f);
		glScalef(0.18f, 0.03f, 0.27f);
		glColor3f(accentOrange[0] * 0.6f, accentOrange[1] * 0.6f, accentOrange[2] * 0.6f);
		glutSolidCube(1.0f);
		glPopMatrix();
	}

	glPopMatrix();
}

void drawCrystal(float x, float y, float z, bool active)
{
	if (!active)
		return;

	glPushMatrix();
	glTranslatef(x, y + sin(crystalBob) * 0.4f, z);
	glRotatef(crystalRotation, 0, 1, 0);

	// Multi-layered energy crystal
	// Outer glow shell
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	float pulse = sin(crystalBob * 2) * 0.3f + 0.7f;

	glPushMatrix();
	glColor4f(0.2f, 0.8f, 1.0f, 0.3f * pulse);
	glutSolidSphere(0.6f, 20, 20);
	glPopMatrix();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Core crystal (octahedron)
	glPushMatrix();
	glRotatef(crystalRotation * 0.5f, 1, 1, 0);
	glScalef(0.5f, 0.5f, 0.5f);
	glColor3f(0.3f, 1.0f, 1.0f);
	glutSolidOctahedron();
	glPopMatrix();

	// Inner energy sphere
	glPushMatrix();
	glColor4f(0.5f, 0.9f, 1.0f, 0.6f);
	glutSolidSphere(0.35f, 18, 18);
	glPopMatrix();

	// Rotating energy rings
	for (int i = 0; i < 3; i++)
	{
		glPushMatrix();
		glRotatef(crystalRotation + i * 120, 0, 1, 0);
		glRotatef(45, 1, 0, 0);
		glColor3f(0.0f, 0.7f + i * 0.1f, 1.0f);
		glutSolidTorus(0.05f, 0.5f + i * 0.1f, 12, 24);
		glPopMatrix();
	}

	glDisable(GL_LIGHTING);
	// Energy particles spiraling around
	for (int i = 0; i < 8; i++)
	{
		float angle = (crystalRotation * 2 + i * 45) * PI / 180.0f;
		float height = sin(crystalBob + i) * 0.5f;
		float radius = 0.7f;
		float px = cos(angle) * radius;
		float pz = sin(angle) * radius;

		glPushMatrix();
		glTranslatef(px, height, pz);
		glColor3f(0.5f, 1.0f, 1.0f);
		glutSolidSphere(0.08f, 10, 10);
		glPopMatrix();
	}
	glEnable(GL_LIGHTING);

	// Holographic base platform
	glPushMatrix();
	glTranslatef(0, -0.7f, 0);
	glRotatef(-crystalRotation * 0.3f, 0, 1, 0);

	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.2f, 0.6f, 0.8f, 0.5f);

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, 0);
	for (int i = 0; i <= 12; i++)
	{
		float angle = i * 30 * PI / 180.0f;
		glVertex3f(cos(angle) * 0.5f, 0, sin(angle) * 0.5f);
	}
	glEnd();

	glEnable(GL_LIGHTING);
	glPopMatrix();

	glPopMatrix();
}

void drawMysteryBox(MysteryBox &box)
{
	glPushMatrix();
	glTranslatef(box.pos.x, box.pos.y, box.pos.z);
	glRotatef(box.rotation, 0, 1, 0);

	float bobHeight = sin(crystalBob * 0.5f) * 0.1f;
	glTranslatef(0, bobHeight, 0);

	// Main box body
	glPushMatrix();
	glScalef(1.0f, 1.0f, 1.0f);

	if (box.opened)
	{
		// Empty/opened box color
		glColor3f(0.3f, 0.3f, 0.35f);
	}
	else if (box.hasCrystal)
	{
		// Crystal box - glowing cyan
		float glow = sin(crystalBob * 2) * 0.3f + 0.7f;
		glColor3f(0.2f * glow, 0.8f * glow, 1.0f * glow);
	}
	else
	{
		// Empty box - dull gray
		glColor3f(0.4f, 0.4f, 0.45f);
	}
	glutSolidCube(1.0f);
	glPopMatrix();

	// Lid (opens upward if opened)
	glPushMatrix();
	if (box.opened)
	{
		glTranslatef(0, 0.5f, -0.5f);
		glRotatef(box.openAngle, 1, 0, 0);
		glTranslatef(0, 0, 0.5f);
	}
	else
	{
		glTranslatef(0, 0.5f, 0);
	}

	glScalef(1.05f, 0.1f, 1.05f);

	if (box.hasCrystal && !box.opened)
	{
		float glow = sin(crystalBob * 2) * 0.2f + 0.8f;
		glColor3f(0.3f * glow, 0.9f * glow, 1.0f * glow);
	}
	else
	{
		glColor3f(0.5f, 0.5f, 0.55f);
	}
	glutSolidCube(1.0f);
	glPopMatrix();

	// Question mark hologram on top (if unopened)
	if (!box.opened)
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		glPushMatrix();
		glTranslatef(0, 0.8f, 0);
		glRotatef(-box.rotation, 0, 1, 0); // Face camera

		float alpha = (sin(crystalBob * 3) + 1) / 2 * 0.7f + 0.3f;

		if (box.hasCrystal)
		{
			glColor4f(0.3f, 1.0f, 1.0f, alpha);
		}
		else
		{
			glColor4f(0.7f, 0.7f, 0.7f, alpha * 0.6f);
		}

		// Draw ? using primitives
		glutSolidSphere(0.15f, 15, 15);
		glPopMatrix();

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LIGHTING);
	}

	// Energy particles around mystery boxes with crystals
	if (box.hasCrystal && !box.opened)
	{
		glDisable(GL_LIGHTING);
		for (int i = 0; i < 6; i++)
		{
			float angle = (crystalRotation * 2 + i * 60) * PI / 180.0f;
			float px = cos(angle) * 0.8f;
			float pz = sin(angle) * 0.8f;
			float py = sin(crystalBob * 2 + i) * 0.3f;

			glPushMatrix();
			glTranslatef(px, py, pz);
			glColor3f(0.4f, 1.0f, 1.0f);
			glutSolidSphere(0.06f, 8, 8);
			glPopMatrix();
		}
		glEnable(GL_LIGHTING);
	}

	// Base platform with tech details
	glPushMatrix();
	glTranslatef(0, -0.6f, 0);
	glRotatef(-90, 1, 0, 0);
	GLUquadricObj *quad = gluNewQuadric();
	glColor3f(0.2f, 0.2f, 0.25f);
	gluCylinder(quad, 0.6f, 0.5f, 0.1f, 16, 5);
	gluDeleteQuadric(quad);
	glPopMatrix();

	// Corner lights
	for (int i = 0; i < 4; i++)
	{
		float angle = i * 90 * PI / 180.0f;
		float lx = cos(angle) * 0.5f;
		float lz = sin(angle) * 0.5f;

		glPushMatrix();
		glTranslatef(lx, 0, lz);

		if (box.hasCrystal && !box.opened)
		{
			float pulse = sin(crystalBob * 4 + i) * 0.5f + 0.5f;
			glColor3f(0.2f, 0.8f * pulse, 1.0f * pulse);
		}
		else
		{
			glColor3f(0.2f, 0.2f, 0.2f);
		}
		glutSolidSphere(0.08f, 10, 10);
		glPopMatrix();
	}

	glPopMatrix();
}

void drawConsole()
{
	glPushMatrix();
	glTranslatef(consolePos.x, consolePos.y, consolePos.z);

	// Scale up entire object for visibility
	glScalef(2.0f, 2.0f, 2.0f);

	float scale = consoleScale;

	// Base (cube) - DARK GRAY METAL
	glPushMatrix();
	glScalef(0.8f, 0.5f, 0.6f);
	glColor3f(0.3f, 0.3f, 0.35f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Screen (thin cube) - BRIGHT GREEN HOLOGRAPHIC
	glPushMatrix();
	glTranslatef(0, 0.4f, 0);
	glScalef(0.7f * scale, 0.5f * scale, 0.05f);
	glColor3f(0.0f, 1.0f * scale, 0.3f * scale);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Screen frame (4 small cubes) - BLACK
	glColor3f(0.1f, 0.1f, 0.12f);
	glPushMatrix();
	glTranslatef(-0.35f, 0.65f, 0);
	glScalef(0.05f, 0.05f, 0.1f);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.35f, 0.65f, 0);
	glScalef(0.05f, 0.05f, 0.1f);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.35f, 0.15f, 0);
	glScalef(0.05f, 0.05f, 0.1f);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.35f, 0.15f, 0);
	glScalef(0.05f, 0.05f, 0.1f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Antenna (cone)
	glPushMatrix();
	glTranslatef(0, 0.7f, 0);
	glRotatef(-90, 1, 0, 0);
	glColor3f(0.5f, 0.5f, 0.55f);
	glutSolidCone(0.05f, 0.3f, 10, 10);
	glPopMatrix();

	// Support (cylinder)
	glPushMatrix();
	glTranslatef(0, -0.5f, 0);
	glRotatef(-90, 1, 0, 0);
	GLUquadricObj *quad = gluNewQuadric();
	glColor3f(0.4f, 0.4f, 0.45f);
	gluCylinder(quad, 0.15f, 0.15f, 0.5f, 10, 5);
	gluDeleteQuadric(quad);
	glPopMatrix();

	glPopMatrix();
}

void drawAirlock()
{
	glPushMatrix();
	glTranslatef(airlockPos.x, airlockPos.y, airlockPos.z);

	// Scale up for visibility
	glScalef(1.5f, 1.5f, 1.5f);

	// Left door - DARK METALLIC GREEN
	glPushMatrix();
	glTranslatef(-0.5f - airlockDoorOffset, 0, 0);
	glScalef(1.0f, 2.5f, 0.3f);
	glColor3f(0.2f, 0.4f, 0.3f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Right door - DARK METALLIC GREEN
	glPushMatrix();
	glTranslatef(0.5f + airlockDoorOffset, 0, 0);
	glScalef(1.0f, 2.5f, 0.3f);
	glColor3f(0.2f, 0.4f, 0.3f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Frame top - DARK BLUE
	glPushMatrix();
	glTranslatef(0, 1.5f, 0);
	glScalef(2.5f, 0.2f, 0.4f);
	glColor3f(0.2f, 0.3f, 0.5f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Frame left
	glPushMatrix();
	glTranslatef(-1.25f, 0, 0);
	glScalef(0.2f, 3.0f, 0.4f);
	glColor3f(0.3f, 0.3f, 0.35f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Frame right
	glPushMatrix();
	glTranslatef(1.25f, 0, 0);
	glScalef(0.2f, 3.0f, 0.4f);
	glColor3f(0.3f, 0.3f, 0.35f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Warning light
	glPushMatrix();
	glTranslatef(0, 1.8f, 0);
	glColor3f(1.0f, 0.0f, 0.0f);
	glutSolidSphere(0.15f, 15, 15);
	glPopMatrix();

	// Left hydraulic
	glPushMatrix();
	glTranslatef(-0.8f, 0, 0.2f);
	glRotatef(90, 0, 1, 0);
	GLUquadricObj *quad1 = gluNewQuadric();
	glColor3f(0.4f, 0.4f, 0.45f);
	gluCylinder(quad1, 0.08f, 0.08f, 0.3f, 10, 5);
	gluDeleteQuadric(quad1);
	glPopMatrix();

	// Right hydraulic
	glPushMatrix();
	glTranslatef(0.8f, 0, 0.2f);
	glRotatef(90, 0, 1, 0);
	GLUquadricObj *quad2 = gluNewQuadric();
	glColor3f(0.4f, 0.4f, 0.45f);
	gluCylinder(quad2, 0.08f, 0.08f, 0.3f, 10, 5);
	gluDeleteQuadric(quad2);
	glPopMatrix();

	glPopMatrix();
}

void drawContainer()
{
	glPushMatrix();
	glTranslatef(containerPos.x, containerPos.y, containerPos.z);

	// Scale up for visibility
	glScalef(2.0f, 2.0f, 2.0f);

	// Main body (cube) - DARK GREEN MILITARY STYLE
	glPushMatrix();
	glScalef(1.0f, 1.0f, 0.8f);
	glColor3f(0.2f, 0.4f, 0.2f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Hazard stripe - YELLOW
	glPushMatrix();
	glTranslatef(0, 0, 0.45f);
	glScalef(1.05f, 0.3f, 0.05f);
	glColor3f(1.0f, 0.9f, 0.0f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Lid (thin cube - rotates) - DARKER GREEN
	glPushMatrix();
	glTranslatef(0, 0.5f, -0.4f);
	glRotatef(containerLidAngle, 1, 0, 0);
	glTranslatef(0, 0, 0.4f);
	glScalef(1.05f, 0.1f, 0.85f);
	glColor3f(0.15f, 0.35f, 0.15f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Lock mechanism
	glPushMatrix();
	glTranslatef(0, 0.3f, 0.45f);
	glScalef(0.2f, 0.3f, 0.1f);
	glColor3f(0.3f, 0.3f, 0.35f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Handle (torus)
	glPushMatrix();
	glTranslatef(0.6f, 0, 0);
	glRotatef(90, 0, 1, 0);
	glColor3f(0.5f, 0.5f, 0.55f);
	glutSolidTorus(0.08f, 0.2f, 10, 15);
	glPopMatrix();

	glPopMatrix();
}

void drawDish()
{
	glPushMatrix();
	glTranslatef(dishPos.x, dishPos.y, dishPos.z);

	// Scale up for visibility
	glScalef(2.5f, 2.5f, 2.5f);
	glRotatef(dishRotation, 0, 1, 0);

	// Main cargo crate body - BROWN/TAN
	glPushMatrix();
	glScalef(1.2f, 1.0f, 1.2f);
	glColor3f(0.6f, 0.4f, 0.2f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Metal bands (horizontal straps)
	for (int i = -1; i <= 1; i++)
	{
		glPushMatrix();
		glTranslatef(0, i * 0.3f, 0);
		glScalef(1.25f, 0.08f, 1.25f);
		glColor3f(0.3f, 0.3f, 0.3f);
		glutSolidCube(1.0f);
		glPopMatrix();
	}

	// Warning markings - YELLOW
	glPushMatrix();
	glTranslatef(0, 0.3f, 0.61f);
	glScalef(0.4f, 0.4f, 0.02f);
	glColor3f(1.0f, 0.9f, 0.0f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Caution symbol base
	glPushMatrix();
	glTranslatef(0, 0.3f, 0.62f);
	glScalef(0.3f, 0.3f, 0.01f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glutSolidCube(1.0f);
	glPopMatrix();
	glutSolidSphere(0.15f, 15, 15);
	glPopMatrix();

	glPopMatrix();
}

void drawTank()
{
	glPushMatrix();
	glTranslatef(tankPos.x, tankPos.y, tankPos.z);

	// Scale up for visibility
	glScalef(2.0f, 2.0f, 2.0f);
	glRotatef(tankRotation, 0, 1, 0);
	glScalef(tankScale, tankScale, tankScale);

	// Tank body (cylinder) - BRIGHT ORANGE
	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	GLUquadricObj *quad = gluNewQuadric();
	glColor3f(1.0f, 0.6f, 0.1f);
	gluCylinder(quad, 0.3f, 0.3f, 1.5f, 20, 10);
	gluDeleteQuadric(quad);
	glPopMatrix();

	// Top cap (cone) - DARK ORANGE
	glPushMatrix();
	glTranslatef(0, 1.5f, 0);
	glRotatef(-90, 1, 0, 0);
	glColor3f(0.8f, 0.5f, 0.0f);
	glutSolidCone(0.3f, 0.3f, 20, 10);
	glPopMatrix();

	// Bottom cap (cone inverted)
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glColor3f(0.8f, 0.8f, 0.85f);
	glutSolidCone(0.3f, 0.3f, 20, 10);
	glPopMatrix();

	// Valve (small cylinder)
	glPushMatrix();
	glTranslatef(0, 1.8f, 0);
	glRotatef(-90, 1, 0, 0);
	GLUquadricObj *quad2 = gluNewQuadric();
	glColor3f(0.6f, 0.6f, 0.65f);
	gluCylinder(quad2, 0.05f, 0.05f, 0.2f, 10, 5);
	gluDeleteQuadric(quad2);
	glPopMatrix();

	// Warning stripe
	glPushMatrix();
	glTranslatef(0, 0.75f, 0);
	glRotatef(-90, 1, 0, 0);
	glColor3f(1.0f, 0.0f, 0.0f);
	GLUquadricObj *quad3 = gluNewQuadric();
	gluCylinder(quad3, 0.31f, 0.31f, 0.2f, 20, 5);
	gluDeleteQuadric(quad3);
	glPopMatrix();

	// Gauge (small cube)
	glPushMatrix();
	glTranslatef(0.32f, 1.0f, 0);
	glScalef(0.1f, 0.2f, 0.2f);
	glColor3f(0.2f, 0.2f, 0.25f);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPopMatrix();
}

void drawPlatform(Platform &plat)
{
	glPushMatrix();
	glTranslatef(plat.pos.x, plat.pos.y, plat.pos.z);

	// Main platform body - DARK PURPLE METALLIC
	glPushMatrix();
	glScalef(plat.size.x, plat.size.y, plat.size.z);
	glColor3f(0.3f, 0.15f, 0.4f); // Dark purple metallic
	glutSolidCube(1.0f);
	glPopMatrix();

	// Platform edge glow - BRIGHT CYAN (NOT WHITE)
	glPushMatrix();
	glTranslatef(0, plat.size.y / 2 + 0.05f, 0);
	glScalef(plat.size.x * 1.05f, 0.05f, plat.size.z * 1.05f);
	glColor3f(0.0f, 0.8f, 1.0f); // Bright cyan glow
	glutSolidCube(1.0f);
	glPopMatrix();

	// Support pillars (4 corners)
	float hx = plat.size.x / 2 - 0.15f;
	float hz = plat.size.z / 2 - 0.15f;
	glColor3f(0.2f, 0.2f, 0.25f);

	for (int i = 0; i < 4; i++)
	{
		float px = (i % 2 == 0) ? -hx : hx;
		float pz = (i < 2) ? -hz : hz;
		glPushMatrix();
		glTranslatef(px, -plat.size.y / 2, pz);
		glRotatef(-90, 1, 0, 0);
		GLUquadricObj *quad = gluNewQuadric();
		gluCylinder(quad, 0.1f, 0.1f, plat.pos.y - plat.size.y / 2, 8, 5);
		gluDeleteQuadric(quad);
		glPopMatrix();
	}

	glPopMatrix();
}

void drawEnemy(Enemy &enemy)
{
	if (!enemy.active)
		return;

	glPushMatrix();
	glTranslatef(enemy.pos.x, enemy.pos.y, enemy.pos.z);
	glRotatef(enemy.rotY, 0, 1, 0);

	// Vary colors per enemy for visual distinction
	int enemyIndex = (&enemy - enemies) % MAX_ENEMIES;
	float baseR = 0.8f, baseG = 0.1f, baseB = 0.2f;
	float accentR = 1.0f, accentG = 0.3f, accentB = 0.0f;

	switch (enemyIndex)
	{
	case 0: // Red/Orange aggressive
		baseR = 0.9f;
		baseG = 0.15f;
		baseB = 0.1f;
		accentR = 1.0f;
		accentG = 0.4f;
		accentB = 0.0f;
		break;
	case 1: // Purple/Magenta menacing
		baseR = 0.7f;
		baseG = 0.1f;
		baseB = 0.8f;
		accentR = 0.9f;
		accentG = 0.2f;
		accentB = 1.0f;
		break;
	case 2: // Dark red/crimson
		baseR = 0.6f;
		baseG = 0.05f;
		baseB = 0.1f;
		accentR = 0.9f;
		accentG = 0.1f;
		accentB = 0.15f;
		break;
	case 3: // Orange/Yellow aggressive
		baseR = 0.9f;
		baseG = 0.4f;
		baseB = 0.1f;
		accentR = 1.0f;
		accentG = 0.6f;
		accentB = 0.0f;
		break;
	}

	// Pulsing aggression effect
	float aggroPulse = sin(crystalBob * 2.5f) * 0.15f + 0.85f;

	// ===== TORSO (menacing angular design) =====
	// Main body chassis
	glPushMatrix();
	glTranslatef(0, 0.1f, 0);
	glScalef(0.65f, 0.75f, 0.55f);
	glColor3f(baseR * aggroPulse, baseG * aggroPulse, baseB * aggroPulse);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Chest armor plates (angular overlays)
	glPushMatrix();
	glTranslatef(0, 0.15f, 0.28f);
	glRotatef(10, 1, 0, 0);
	glScalef(0.55f, 0.6f, 0.15f);
	glColor3f(baseR * 0.7f, baseG * 0.7f, baseB * 0.7f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Spine/back plates
	for (int i = 0; i < 3; i++)
	{
		glPushMatrix();
		glTranslatef(0, 0.35f - i * 0.2f, -0.35f);
		glRotatef(45, 1, 0, 0);
		glScalef(0.25f, 0.15f, 0.08f);
		glColor3f(baseR * 0.6f, baseG * 0.6f, baseB * 0.6f);
		glutSolidCube(1.0f);
		glPopMatrix();
	}

	// Glowing core/weak spot
	glPushMatrix();
	glTranslatef(0, 0.2f, 0.3f);
	float coreGlow = sin(crystalBob * 3) * 0.4f + 0.6f;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glColor4f(accentR, accentG * 1.5f, accentB, 0.7f * coreGlow);
	glutSolidSphere(0.15f, 16, 16);
	glDisable(GL_BLEND);
	glPopMatrix();

	// ===== HEAD (alien/robotic skull design) =====
	// Main head structure (elongated back)
	glPushMatrix();
	glTranslatef(0, 0.65f, 0);
	glScalef(0.8f, 0.9f, 1.2f);
	glColor3f(baseR * 0.85f, baseG * 0.85f, baseB * 0.85f);
	glutSolidSphere(0.35f, 20, 20);
	glPopMatrix();

	// Face plate (angular menacing)
	glPushMatrix();
	glTranslatef(0, 0.65f, 0.28f);
	glScalef(0.6f, 0.7f, 0.2f);
	glColor3f(baseR * 0.5f, baseG * 0.5f, baseB * 0.5f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Glowing eyes (two menacing slits)
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.15f : 0.15f;
		glPushMatrix();
		glTranslatef(side, 0.68f, 0.35f);
		float eyeGlow = (sin(crystalBob * 4 + i) + 1) / 2;
		glColor3f(accentR * eyeGlow, accentG * eyeGlow * 0.5f, 0.0f);
		glScalef(1.8f, 0.6f, 1.0f);
		glutSolidSphere(0.08f, 12, 12);
		glPopMatrix();
	}

	// Horns/antenna (menacing protrusions)
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.2f : 0.2f;
		glPushMatrix();
		glTranslatef(side, 0.95f, -0.05f);
		glRotatef(-75 + (i * 30), 0, 0, 1);
		glRotatef(20, 1, 0, 0);
		GLUquadricObj *horn = gluNewQuadric();
		glColor3f(baseR * 0.4f, baseG * 0.4f, baseB * 0.4f);
		gluCylinder(horn, 0.06f, 0.02f, 0.4f, 10, 5);
		gluDeleteQuadric(horn);
		glPopMatrix();
	}

	// Jaw/mandibles
	glPushMatrix();
	glTranslatef(0, 0.52f, 0.32f);
	glScalef(0.45f, 0.25f, 0.15f);
	glColor3f(baseR * 0.6f, baseG * 0.6f, baseB * 0.6f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// ===== ARMS (powerful clawed limbs) =====
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.45f : 0.45f;
		float rotDir = (i == 0) ? 90 : -90;

		// Shoulder joint (sphere)
		glPushMatrix();
		glTranslatef(side, 0.35f, 0);
		glColor3f(baseR * 0.7f, baseG * 0.7f, baseB * 0.7f);
		glutSolidSphere(0.16f, 14, 14);
		glPopMatrix();

		// Shoulder spike
		glPushMatrix();
		glTranslatef(side * 1.1f, 0.45f, 0);
		glRotatef(rotDir, 0, 0, 1);
		GLUquadricObj *spike = gluNewQuadric();
		glColor3f(baseR * 0.5f, baseG * 0.5f, baseB * 0.5f);
		gluCylinder(spike, 0.08f, 0.0f, 0.25f, 8, 5);
		gluDeleteQuadric(spike);
		glPopMatrix();

		// Upper arm (thick and muscular)
		glPushMatrix();
		glTranslatef(side, 0.2f, 0);
		glRotatef(rotDir, 0, 0, 1);
		GLUquadricObj *upperArm = gluNewQuadric();
		glColor3f(baseR * aggroPulse, baseG * aggroPulse, baseB * aggroPulse);
		gluCylinder(upperArm, 0.13f, 0.11f, 0.45f, 12, 5);
		gluDeleteQuadric(upperArm);
		glPopMatrix();

		// Elbow joint
		glPushMatrix();
		glTranslatef(side > 0 ? side + 0.45f : side - 0.45f, 0.2f, 0);
		glColor3f(baseR * 0.6f, baseG * 0.6f, baseB * 0.6f);
		glutSolidSphere(0.11f, 12, 12);
		glPopMatrix();

		// Forearm (tapered)
		glPushMatrix();
		glTranslatef(side > 0 ? side + 0.45f : side - 0.45f, 0.2f, 0);
		glRotatef(rotDir, 0, 0, 1);
		GLUquadricObj *forearm = gluNewQuadric();
		glColor3f(baseR * 0.8f * aggroPulse, baseG * 0.8f * aggroPulse, baseB * 0.8f * aggroPulse);
		gluCylinder(forearm, 0.11f, 0.09f, 0.4f, 12, 5);
		gluDeleteQuadric(forearm);
		glPopMatrix();

		// Claw hand (menacing)
		glPushMatrix();
		glTranslatef(side > 0 ? side + 0.85f : side - 0.85f, 0.2f, 0);
		glScalef(0.15f, 0.2f, 0.12f);
		glColor3f(baseR * 0.5f, baseG * 0.5f, baseB * 0.5f);
		glutSolidCube(1.0f);
		glPopMatrix();

		// Claw talons (3 sharp points)
		for (int j = 0; j < 3; j++)
		{
			glPushMatrix();
			glTranslatef(side > 0 ? side + 0.92f : side - 0.92f, 0.2f + (j - 1) * 0.08f, 0);
			glRotatef(rotDir, 0, 0, 1);
			glRotatef(20 * (j - 1), 0, 1, 0);
			GLUquadricObj *talon = gluNewQuadric();
			glColor3f(0.3f, 0.3f, 0.35f);
			gluCylinder(talon, 0.03f, 0.0f, 0.15f, 6, 3);
			gluDeleteQuadric(talon);
			glPopMatrix();
		}
	}

	// ===== LEGS (powerful digitigrade stance) =====
	for (int i = 0; i < 2; i++)
	{
		float side = (i == 0) ? -0.2f : 0.2f;

		// Hip joint
		glPushMatrix();
		glTranslatef(side, -0.35f, 0);
		glColor3f(baseR * 0.6f, baseG * 0.6f, baseB * 0.6f);
		glutSolidSphere(0.14f, 12, 12);
		glPopMatrix();

		// Thigh (thick upper leg)
		glPushMatrix();
		glTranslatef(side, -0.5f, 0);
		glRotatef(-90, 1, 0, 0);
		GLUquadricObj *thigh = gluNewQuadric();
		glColor3f(baseR * aggroPulse, baseG * aggroPulse, baseB * aggroPulse);
		gluCylinder(thigh, 0.14f, 0.12f, 0.45f, 12, 5);
		gluDeleteQuadric(thigh);
		glPopMatrix();

		// Knee armor (spike)
		glPushMatrix();
		glTranslatef(side, -0.95f, 0.1f);
		glRotatef(-60, 1, 0, 0);
		GLUquadricObj *kneeSpike = gluNewQuadric();
		glColor3f(baseR * 0.5f, baseG * 0.5f, baseB * 0.5f);
		gluCylinder(kneeSpike, 0.08f, 0.0f, 0.2f, 8, 4);
		gluDeleteQuadric(kneeSpike);
		glPopMatrix();

		// Lower leg (reverse joint like raptor)
		glPushMatrix();
		glTranslatef(side, -1.0f, 0);
		glRotatef(-90, 1, 0, 0);
		glRotatef(20, 0, 1, 0);
		GLUquadricObj *shin = gluNewQuadric();
		glColor3f(baseR * 0.75f * aggroPulse, baseG * 0.75f * aggroPulse, baseB * 0.75f * aggroPulse);
		gluCylinder(shin, 0.12f, 0.08f, 0.5f, 12, 5);
		gluDeleteQuadric(shin);
		glPopMatrix();

		// Foot claw (three-toed)
		glPushMatrix();
		glTranslatef(side, -1.5f, 0.15f);
		glScalef(0.18f, 0.1f, 0.3f);
		glColor3f(baseR * 0.4f, baseG * 0.4f, baseB * 0.4f);
		glutSolidCube(1.0f);
		glPopMatrix();

		// Toe claws
		for (int j = 0; j < 3; j++)
		{
			glPushMatrix();
			glTranslatef(side, -1.5f, 0.3f + j * 0.08f);
			glRotatef(90, 1, 0, 0);
			glRotatef((j - 1) * 15, 0, 1, 0);
			GLUquadricObj *toeClaw = gluNewQuadric();
			glColor3f(0.25f, 0.25f, 0.3f);
			gluCylinder(toeClaw, 0.03f, 0.0f, 0.12f, 6, 3);
			gluDeleteQuadric(toeClaw);
			glPopMatrix();
		}
	}

	// Energy field when aggressive (close to player)
	float distToPlayer = distance3D(enemy.pos, playerPos);
	if (distToPlayer < 10.0f)
	{
		glPushMatrix();
		glTranslatef(0, 0.3f, 0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		float fieldPulse = sin(crystalBob * 5) * 0.3f + 0.3f;
		glColor4f(accentR, accentG * 0.5f, 0.0f, fieldPulse * 0.4f);
		glutSolidSphere(0.9f, 20, 20);
		glDisable(GL_BLEND);
		glPopMatrix();
	}

	glPopMatrix();
}

// ==================== CAMERA FUNCTIONS ====================

void setupCamera()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Use global camera position variables for lighting
	const float WALL_MARGIN = 2.0f; // Keep camera this far from walls

	switch (cameraMode)
	{
	case FREE_CAM:
		// LAB 6 CAMERA: Free-flying camera with full control (Assignment requirement)
		// Use Lab 6's Camera class for movement and rotation
		camera.look();
		// Update global cam variables for lighting (if needed elsewhere)
		camX = camera.eye.x;
		camY = camera.eye.y;
		camZ = camera.eye.z;
		break;

	case FRONT_VIEW:
		// Follow player from behind and slightly above, clamped
		camX = playerPos.x;
		camY = playerPos.y + 6.0f;
		camZ = playerPos.z + 12.0f;
		if (camZ > BOUNDARY - WALL_MARGIN)
			camZ = BOUNDARY - WALL_MARGIN;

		gluLookAt(camX, camY, camZ,
				  playerPos.x, playerPos.y + 1.0f, playerPos.z,
				  0, 1, 0);
		break;

	case SIDE_VIEW:
		// Side view offset, clamped to not go through walls
		camX = playerPos.x + 18.0f;
		if (camX > BOUNDARY - WALL_MARGIN)
			camX = BOUNDARY - WALL_MARGIN;
		if (camX < -BOUNDARY + WALL_MARGIN)
			camX = -BOUNDARY + WALL_MARGIN;

		gluLookAt(camX, playerPos.y + 6.0f, playerPos.z,
				  playerPos.x, playerPos.y + 1.0f, playerPos.z,
				  0, 1, 0);
		break;

	case TOP_VIEW:
		// Top-down view looking straight down
		gluLookAt(playerPos.x, playerPos.y + 22.0f, playerPos.z,
				  playerPos.x, playerPos.y, playerPos.z,
				  0, 0, -1);
		break;
	}
}

// Setup simple basic lighting (not a requirement, but helps visibility)
void setupLighting()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Single simple white light from above - REDUCED AMBIENT for better color visibility
	GLfloat ambient[] = {0.2f, 0.2f, 0.2f, 1.0f}; // Reduced from 0.5 to avoid washing out colors
	GLfloat diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f}; // Slightly reduced diffuse
	GLfloat pos[] = {0.0f, 30.0f, 0.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	// Enable color material so glColor3f works
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

// ==================== GAME LOGIC ====================

bool checkPlatformCollision(Vector3 pos, Platform &plat)
{
	float halfX = plat.size.x / 2;
	float halfZ = plat.size.z / 2;
	float platTop = plat.pos.y + plat.size.y / 2;

	return (pos.x >= plat.pos.x - halfX && pos.x <= plat.pos.x + halfX &&
			pos.z >= plat.pos.z - halfZ && pos.z <= plat.pos.z + halfZ &&
			pos.y >= platTop - 0.3f && pos.y <= platTop + 0.5f);
}

void updatePlayer()
{
	// Decrease damage flash and invincibility
	if (damageFlash > 0)
		damageFlash -= 0.05f;
	if (invincibleTime > 0)
		invincibleTime -= 0.016f;

	// Handle FREE_CAM separately (dynamic third-person camera controls)
	if (cameraMode == FREE_CAM)
	{
		// FREE_CAM now controls camera angle and zoom, not position
		// I/K = rotate vertically (tilt up/down)
		// LAB 6 CAMERA CONTROLS (I/K/J/L/U/O for movement)
		// Arrow keys for rotation are handled in SpecialInput function
		float moveSpeed = 0.3f;

		if (keysPressed['i'] || keysPressed['I'])
		{
			camera.moveZ(moveSpeed); // Move forward
		}
		if (keysPressed['k'] || keysPressed['K'])
		{
			camera.moveZ(-moveSpeed); // Move backward
		}
		if (keysPressed['j'] || keysPressed['J'])
		{
			camera.moveX(moveSpeed); // Move left
		}
		if (keysPressed['l'] || keysPressed['L'])
		{
			camera.moveX(-moveSpeed); // Move right
		}
		if (keysPressed['u'] || keysPressed['U'])
		{
			camera.moveY(moveSpeed); // Move up
		}
		if (keysPressed['o'] || keysPressed['O'])
		{
			camera.moveY(-moveSpeed); // Move down
		}

		// In FREE_CAM, player doesn't move (only camera moves)
		return;
	}

	// Movement input with camera-relative controls
	bool moving = false;
	Vector3 moveDir(0, 0, 0);

	// SIMPLE movement - same controls for all views, world-space
	// W/S = forward/back in Z axis, A/D = left/right in X axis
	if (keysPressed['w'] || keysPressed['W'])
	{
		moveDir.z -= 1;
		moving = true;
	}
	if (keysPressed['s'] || keysPressed['S'])
	{
		moveDir.z += 1;
		moving = true;
	}
	if (keysPressed['a'] || keysPressed['A'])
	{
		moveDir.x -= 1;
		moving = true;
	}
	if (keysPressed['d'] || keysPressed['D'])
	{
		moveDir.x += 1;
		moving = true;
	}

	// Normalize diagonal movement
	if (moveDir.x != 0 && moveDir.z != 0)
	{
		moveDir.x *= 0.707f;
		moveDir.z *= 0.707f;
	}

	// Apply movement with rotation to face movement direction
	if (moving)
	{
		// Calculate angle based on movement direction
		// W/Up (z=-1) = 0° (North, facing away from camera)
		// S/Down (z=+1) = 180° (South, facing toward camera)
		// D/Right (x=+1) = 90° (East)
		// A/Left (x=-1) = 270° (West)
		float angle = atan2(moveDir.x, moveDir.z) * 180.0f / PI;

		// ASSIGNMENT REQUIREMENT: Rotation behavior based on ground state
		if (isOnGround)
		{
			// On ground: Face direction of motion (y-axis only)
			playerRotY = angle;
			playerRotX = 0;
		}
		else
		{
			// In air: Keep x-axis tilt AND update y-axis to match direction
			playerRotY = angle; // Still rotate to face direction
			playerRotX = 45.0f; // Maintain forward tilt (astronaut swimming)
		}

		playerPos.x += moveDir.x * playerSpeed * 0.016f * 60.0f; // Frame-rate independent
		playerPos.z += moveDir.z * playerSpeed * 0.016f * 60.0f;
	}

	// Jump with sound
	if (keysPressed[' '] && isOnGround)
	{
		playerVel.y = jumpForce;
		isOnGround = false;
		keysPressed[' '] = false; // Prevent holding
		playSound(SND_JUMP);
	}

	// Apply gravity and rotation when not on ground
	if (!isOnGround)
	{
		playerVel.y -= gravity;
		// X-axis tilt is now handled above based on movement
		// If not moving in air, maintain tilt from last movement
		if (!moving && playerRotX == 0)
		{
			playerRotX = 45.0f; // Default air tilt
		}
	}
	else
	{
		// On ground: reset tilt
		playerRotX = 0;
		playerVel.y = 0; // Zero velocity when grounded
	}

	// Apply vertical velocity
	playerPos.y += playerVel.y;

	// Reset ground state before checking
	isOnGround = false;
	bool onPlatform = false;

	// Check floor collision first
	if (playerPos.y <= playerBaseHeight)
	{
		playerPos.y = playerBaseHeight;
		playerVel.y = 0;
		isOnGround = true;
		currentPlatform = -1;
	}

	// Check platform collisions
	for (int i = 0; i < PLATFORM_COUNT; i++)
	{
		float halfX = platforms[i].size.x / 2;
		float halfZ = platforms[i].size.z / 2;
		float platTop = platforms[i].pos.y + platforms[i].size.y / 2;

		// Check if player is horizontally over the platform
		bool overPlatform = (playerPos.x >= platforms[i].pos.x - halfX &&
							 playerPos.x <= platforms[i].pos.x + halfX &&
							 playerPos.z >= platforms[i].pos.z - halfZ &&
							 playerPos.z <= platforms[i].pos.z + halfZ);

		if (overPlatform)
		{
			float distAbovePlatform = playerPos.y - platTop;

			// Land on platform if close enough and falling/stationary
			if (distAbovePlatform >= 0 && distAbovePlatform <= playerBaseHeight + 0.2f && playerVel.y <= 0)
			{
				playerPos.y = platTop + playerBaseHeight;
				playerVel.y = 0;
				isOnGround = true;
				onPlatform = true;
				currentPlatform = i;
				platforms[i].colorR = 0.2f;
				platforms[i].colorG = 0.8f;
				platforms[i].colorB = 0.4f;
				break; // Stop checking other platforms
			}
			else
			{
				// Near platform but not landing
				platforms[i].colorR = 0.35f;
				platforms[i].colorG = 0.35f;
				platforms[i].colorB = 0.45f;
			}
		}
		else
		{
			// Reset platform color when not over it
			platforms[i].colorR = 0.3f;
			platforms[i].colorG = 0.3f;
			platforms[i].colorB = 0.4f;
		}
	}

	// If we're not on any platform, clear current platform
	if (!onPlatform && currentPlatform != -1)
	{
		currentPlatform = -1;
	}

	// Boundary collision
	if (playerPos.x > BOUNDARY - 0.5f)
		playerPos.x = BOUNDARY - 0.5f;
	if (playerPos.x < -BOUNDARY + 0.5f)
		playerPos.x = -BOUNDARY + 0.5f;
	if (playerPos.z > BOUNDARY - 0.5f)
		playerPos.z = BOUNDARY - 0.5f;
	if (playerPos.z < -BOUNDARY + 0.5f)
		playerPos.z = -BOUNDARY + 0.5f;
	if (playerPos.y > CEILING_HEIGHT)
	{
		playerPos.y = CEILING_HEIGHT;
		playerVel.y = 0;
	}

	// Death if fall too far
	if (playerPos.y < -2.0f)
	{
		playerHealth = 0;
	}
}

void checkCollisions()
{
	// Crystal collection - only need 3 crystals to win
	for (int i = 0; i < 5; i++)
	{
		if (crystals[i].active)
		{
			float dist = distance3D(playerPos, crystals[i].pos);
			if (dist < 1.2f)
			{
				crystals[i].active = false;
				collectedCrystals++;
				score += 50; // All crystals worth same value

				// Spawn collection particles
				for (int j = 0; j < 20; j++)
				{
					Vector3 vel(
						((float)rand() / RAND_MAX - 0.5f) * 0.2f,
						((float)rand() / RAND_MAX) * 0.15f + 0.05f,
						((float)rand() / RAND_MAX - 0.5f) * 0.2f);
					spawnParticle(crystals[i].pos, vel, 0.3f, 1.0f, 1.0f);
				}

				playSound(SND_COLLECT);

				if (collectedCrystals >= 3)
				{ // Only need 3 crystals to win
					gameState = WIN;
					score += gameTime * 10;
					playSound(SND_WIN);
				}
			}
		}
	}

	// Mystery box interaction
	for (int i = 0; i < MAX_MYSTERY_BOXES; i++)
	{
		if (!mysteryBoxes[i].opened)
		{
			float dist = distance3D(playerPos, mysteryBoxes[i].pos);
			if (dist < 1.5f && keysPressed['e'])
			{
				mysteryBoxes[i].opened = true;
				keysPressed['e'] = false;

				if (mysteryBoxes[i].hasCrystal)
				{
					// Spawn a crystal above the box (use first 2 slots for mystery box crystals)
					playSound(SND_MYSTERY);
					for (int j = 0; j < 2; j++)
					{
						if (!crystals[j].active)
						{
							crystals[j].pos = Vector3(
								mysteryBoxes[i].pos.x,
								mysteryBoxes[i].pos.y + 2.0f,
								mysteryBoxes[i].pos.z);
							crystals[j].active = true;
							score += 50;

							// Spawn reveal particles
							for (int k = 0; k < 30; k++)
							{
								Vector3 vel(
									((float)rand() / RAND_MAX - 0.5f) * 0.25f,
									((float)rand() / RAND_MAX) * 0.2f + 0.1f,
									((float)rand() / RAND_MAX - 0.5f) * 0.25f);
								spawnParticle(mysteryBoxes[i].pos, vel, 0.5f, 1.0f, 1.0f);
							}
							break;
						}
					}
					playSound(SND_COLLECT);
				}
				else
				{
					// Empty box - small penalty
					score -= 5;
					playSound(SND_HIT);

					// Spawn disappointment particles
					for (int k = 0; k < 10; k++)
					{
						Vector3 vel(
							((float)rand() / RAND_MAX - 0.5f) * 0.1f,
							((float)rand() / RAND_MAX) * 0.08f,
							((float)rand() / RAND_MAX - 0.5f) * 0.1f);
						spawnParticle(mysteryBoxes[i].pos, vel, 0.5f, 0.5f, 0.5f);
					}
				}
			}
		}
	}

	// Hazards removed - enemies are the only danger now

	// Enemy collision with better invincibility window
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		if (enemies[i].active)
		{
			float dist = distance3D(playerPos, enemies[i].pos);
			// Larger collision radius so damage actually registers
			if (dist < 1.5f && invincibleTime <= 0)
			{
				playerHealth -= 15;
				damageFlash = 1.0f;
				invincibleTime = 1.5f; // 1.5 second invincibility after hit
				score -= 10;
				playSound(SND_HIT);

				// Spawn combat particles
				for (int j = 0; j < 12; j++)
				{
					Vector3 vel(
						((float)rand() / RAND_MAX - 0.5f) * 0.18f,
						((float)rand() / RAND_MAX) * 0.12f,
						((float)rand() / RAND_MAX - 0.5f) * 0.18f);
					spawnParticle(enemies[i].pos, vel, 0.8f, 0.2f, 0.2f);
				}

				// Knockback from enemy (stronger pushback)
				Vector3 knockDir(
					playerPos.x - enemies[i].pos.x,
					0.15f,
					playerPos.z - enemies[i].pos.z);
				float len = sqrt(knockDir.x * knockDir.x + knockDir.z * knockDir.z);
				if (len > 0)
				{
					playerVel.x = (knockDir.x / len) * 0.35f; // Stronger knockback
					playerVel.y = knockDir.y;
					playerVel.z = (knockDir.z / len) * 0.35f;
					isOnGround = false;
				}

				if (playerHealth <= 0)
				{
					gameState = LOSE;
					if (!loseSoundPlayed)
					{
						playSound(SND_LOSE);
						loseSoundPlayed = true;
					}
				}
			}
		}
	}

	// Apply knockback velocity
	playerPos.x += playerVel.x;
	playerPos.z += playerVel.z;
	playerVel.x *= 0.9f;
	playerVel.z *= 0.9f;
}

void updateAnimations()
{
	// Continuous animations
	crystalRotation += 2.5f;
	if (crystalRotation > 360)
		crystalRotation -= 360;

	crystalBob += 0.1f;
	if (crystalBob > 2 * PI)
		crystalBob -= 2 * PI;

	wallColorTime += 0.01f;
	if (wallColorTime > 2 * PI)
		wallColorTime -= 2 * PI;

	starTwinkle += 0.05f;
	if (starTwinkle > 2 * PI)
		starTwinkle -= 2 * PI;

	nebulaShift += 0.008f;
	if (nebulaShift > 2 * PI)
		nebulaShift -= 2 * PI;

	// Update particles
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (particles[i].active)
		{
			particles[i].pos.x += particles[i].vel.x;
			particles[i].pos.y += particles[i].vel.y;
			particles[i].pos.z += particles[i].vel.z;

			particles[i].vel.y -= 0.008f; // Gravity
			particles[i].life -= 0.02f;

			if (particles[i].life <= 0)
			{
				particles[i].active = false;
			}
		}
	}

	// Update mystery boxes
	for (int i = 0; i < MAX_MYSTERY_BOXES; i++)
	{
		mysteryBoxes[i].rotation += 0.5f;
		if (mysteryBoxes[i].rotation > 360)
			mysteryBoxes[i].rotation -= 360;

		if (mysteryBoxes[i].opened && mysteryBoxes[i].openAngle < 120)
		{
			mysteryBoxes[i].openAngle += 3.0f;
		}
	}

	// Update enemies with improved AI
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		if (enemies[i].active)
		{
			float distToPlayer = distance3D(enemies[i].pos, playerPos);

			// AGGRESSIVE: Chase player if within large detection range
			if (distToPlayer < 20.0f && distToPlayer > 1.0f)
			{
				// Direct chase toward player
				Vector3 dir(
					playerPos.x - enemies[i].pos.x,
					0,
					playerPos.z - enemies[i].pos.z);
				float len = sqrt(dir.x * dir.x + dir.z * dir.z);
				if (len > 0)
				{
					// Faster chase speed for more challenge
					float chaseSpeed = 0.08f;
					// Speed up when very close for urgency
					if (distToPlayer < 5.0f)
						chaseSpeed = 0.12f;

					enemies[i].pos.x += (dir.x / len) * chaseSpeed;
					enemies[i].pos.z += (dir.z / len) * chaseSpeed;
					enemies[i].rotY = atan2(dir.x, -dir.z) * 180.0f / PI;
				}
			}
			else
			{
				// Patrol when player is far away
				if (enemies[i].patrolAxis)
				{
					enemies[i].pos.x += enemies[i].vel.x;
					if (enemies[i].pos.x > enemies[i].patrolMax)
					{
						enemies[i].pos.x = enemies[i].patrolMax;
						enemies[i].vel.x = -fabs(enemies[i].vel.x);
						enemies[i].rotY = 90;
					}
					else if (enemies[i].pos.x < enemies[i].patrolMin)
					{
						enemies[i].pos.x = enemies[i].patrolMin;
						enemies[i].vel.x = fabs(enemies[i].vel.x);
						enemies[i].rotY = -90;
					}
				}
				else
				{
					enemies[i].pos.z += enemies[i].vel.z;
					if (enemies[i].pos.z > enemies[i].patrolMax)
					{
						enemies[i].pos.z = enemies[i].patrolMax;
						enemies[i].vel.z = -fabs(enemies[i].vel.z);
						enemies[i].rotY = 180;
					}
					else if (enemies[i].pos.z < enemies[i].patrolMin)
					{
						enemies[i].pos.z = enemies[i].patrolMin;
						enemies[i].vel.z = fabs(enemies[i].vel.z);
						enemies[i].rotY = 0;
					}
				}
			}
		}
	}

	// Console animation - PULSING SCALE
	if (consoleAnimating)
	{
		consoleScale = 1.0f + sin(crystalBob * 2) * 0.4f; // Larger pulse (0.6 to 1.4)
	}
	else
	{
		consoleScale = 1.0f;
	}

	// Airlock animation - DOORS OPENING/CLOSING
	if (airlockAnimating)
	{
		airlockDoorOffset += 0.05f * airlockDirection; // Faster movement
		if (airlockDoorOffset > 1.5f)
		{ // Open wider
			airlockDoorOffset = 1.5f;
			airlockDirection = -1;
		}
		if (airlockDoorOffset < 0.0f)
		{
			airlockDoorOffset = 0.0f;
			airlockDirection = 1;
		}
	}

	// Container animation - LID OPENING/CLOSING
	if (containerAnimating)
	{
		containerLidAngle += 3.5f * containerDirection; // Faster
		if (containerLidAngle > 110)
		{ // Open more
			containerLidAngle = 110;
			containerDirection = -1;
		}
		if (containerLidAngle < 0)
		{
			containerLidAngle = 0;
			containerDirection = 1;
		}
	}

	// Dish animation - ROTATION
	if (dishAnimating)
	{
		dishRotation += 3.0f; // Faster spin
		if (dishRotation > 360)
			dishRotation -= 360;
	}

	// Tank animation - ROTATION + PULSING
	if (tankAnimating)
	{
		tankRotation += 2.5f; // Faster rotation
		if (tankRotation > 360)
			tankRotation -= 360;
		tankScale = 1.0f + sin(crystalBob) * 0.15f; // Larger pulse
	}
	else
	{
		tankScale = 1.0f;
	}
}

void updateTimer()
{
	if (gameState != PLAYING)
		return;

	int currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000;
	if (currentTime > lastTime)
	{
		lastTime = currentTime;
		gameTime--;
		if (gameTime <= 0)
		{
			gameState = LOSE;
			if (!loseSoundPlayed)
			{
				playSound(SND_LOSE);
				loseSoundPlayed = true;
			}
		}
	}
}

// ==================== UI FUNCTIONS ====================

void drawHUD()
{
	// Switch to 2D
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	// Damage flash overlay
	if (damageFlash > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.0f, 0.0f, 0.0f, damageFlash * 0.4f);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(WINDOW_WIDTH, 0);
		glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
		glVertex2f(0, WINDOW_HEIGHT);
		glEnd();
	}

	// Dark semi-transparent background panel for HUD
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
	glBegin(GL_QUADS);
	glVertex2f(20, WINDOW_HEIGHT - 20);
	glVertex2f(320, WINDOW_HEIGHT - 20);
	glVertex2f(320, WINDOW_HEIGHT - 180);
	glVertex2f(20, WINDOW_HEIGHT - 180);
	glEnd();

	// Health bar background
	glColor3f(0.15f, 0.15f, 0.15f);
	glBegin(GL_QUADS);
	glVertex2f(40, WINDOW_HEIGHT - 40);
	glVertex2f(300, WINDOW_HEIGHT - 40);
	glVertex2f(300, WINDOW_HEIGHT - 65);
	glVertex2f(40, WINDOW_HEIGHT - 65);
	glEnd();

	// Health bar fill
	if (playerHealth > 60)
		glColor3f(0.1f, 0.9f, 0.2f);
	else if (playerHealth > 30)
		glColor3f(1.0f, 0.7f, 0.0f);
	else
		glColor3f(1.0f, 0.1f, 0.0f);

	float healthWidth = (playerHealth / 100.0f) * 255.0f;
	glBegin(GL_QUADS);
	glVertex2f(42, WINDOW_HEIGHT - 42);
	glVertex2f(42 + healthWidth, WINDOW_HEIGHT - 42);
	glVertex2f(42 + healthWidth, WINDOW_HEIGHT - 63);
	glVertex2f(42, WINDOW_HEIGHT - 63);
	glEnd();

	// Health text
	glColor3f(1.0f, 1.0f, 1.0f);
	char healthStr[64];
	sprintf(healthStr, "HEALTH: %d / 100", playerHealth);
	printLarge(50, WINDOW_HEIGHT - 58, healthStr);

	// Timer with background
	glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
	glBegin(GL_QUADS);
	glVertex2f(40, WINDOW_HEIGHT - 80);
	glVertex2f(180, WINDOW_HEIGHT - 80);
	glVertex2f(180, WINDOW_HEIGHT - 105);
	glVertex2f(40, WINDOW_HEIGHT - 105);
	glEnd();

	if (gameTime < 20)
		glColor3f(1.0f, 0.3f, 0.3f);
	else
		glColor3f(0.2f, 1.0f, 1.0f);
	char timeStr[64];
	sprintf(timeStr, "TIME: %d s", gameTime);
	printLarge(50, WINDOW_HEIGHT - 98, timeStr);

	// Score with background
	glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
	glBegin(GL_QUADS);
	glVertex2f(40, WINDOW_HEIGHT - 115);
	glVertex2f(200, WINDOW_HEIGHT - 115);
	glVertex2f(200, WINDOW_HEIGHT - 140);
	glVertex2f(40, WINDOW_HEIGHT - 140);
	glEnd();

	glColor3f(1.0f, 1.0f, 0.4f);
	char scoreStr[64];
	sprintf(scoreStr, "SCORE: %d", score);
	printLarge(50, WINDOW_HEIGHT - 133, scoreStr);

	// Crystals with background
	glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
	glBegin(GL_QUADS);
	glVertex2f(40, WINDOW_HEIGHT - 150);
	glVertex2f(220, WINDOW_HEIGHT - 150);
	glVertex2f(220, WINDOW_HEIGHT - 175);
	glVertex2f(40, WINDOW_HEIGHT - 175);
	glEnd();

	if (collectedCrystals == 3)
		glColor3f(0.2f, 1.0f, 0.3f);
	else
		glColor3f(0.3f, 1.0f, 1.0f);
	char crystalStr[64];
	sprintf(crystalStr, "CRYSTALS: %d / 3", collectedCrystals);
	printLarge(50, WINDOW_HEIGHT - 168, crystalStr);

	// Mystery box interaction prompt (center of screen with large background)
	for (int i = 0; i < MAX_MYSTERY_BOXES; i++)
	{
		if (!mysteryBoxes[i].opened)
		{
			float dist = distance3D(playerPos, mysteryBoxes[i].pos);
			if (dist < 1.5f)
			{
				float pulse = (sin(crystalBob * 4) + 1) / 2;

				// Background box
				glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
				glBegin(GL_QUADS);
				glVertex2f(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 10);
				glVertex2f(WINDOW_WIDTH / 2 + 200, WINDOW_HEIGHT / 2 - 10);
				glVertex2f(WINDOW_WIDTH / 2 + 200, WINDOW_HEIGHT / 2 + 30);
				glVertex2f(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 + 30);
				glEnd();

				if (mysteryBoxes[i].hasCrystal)
				{
					glColor3f(0.3f + pulse * 0.7f, 1.0f, 1.0f);
					printLarge(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 + 5, "[ E ] OPEN MYSTERY BOX - Contains Crystal!");
				}
				else
				{
					glColor3f(0.9f, 0.9f, 0.9f);
					printLarge(WINDOW_WIDTH / 2 - 140, WINDOW_HEIGHT / 2 + 5, "[ E ] OPEN MYSTERY BOX");
				}
				break;
			}
		}
	}

	// Controls hint panel (bottom right with dark background)
	glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
	glBegin(GL_QUADS);
	glVertex2f(WINDOW_WIDTH - 520, 10);
	glVertex2f(WINDOW_WIDTH - 10, 10);
	glVertex2f(WINDOW_WIDTH - 10, 130); // Taller for animation status
	glVertex2f(WINDOW_WIDTH - 520, 130);
	glEnd();

	glColor3f(0.9f, 0.9f, 0.9f);
	printLarge(WINDOW_WIDTH - 510, 105, "CONTROLS:");
	glColor3f(0.6f, 0.9f, 1.0f);
	print(WINDOW_WIDTH - 510, 85, "WASD/Arrows: Move   SPACE: Jump   E: Open Box");
	print(WINDOW_WIDTH - 510, 67, "F/T/V/C: Camera Views   1-5: Animations   Mouse: Look");
	print(WINDOW_WIDTH - 510, 49, "R: Restart   Free Cam (C): I/K/J/L/U/O + Move Mouse");

	// Animation status indicators
	glColor3f(0.8f, 0.8f, 0.2f);
	print(WINDOW_WIDTH - 510, 31, "Animations:");
	char animStatus[128];
	sprintf(animStatus, "1:%s 2:%s 3:%s 4:%s 5:%s",
			consoleAnimating ? "ON" : "off",
			airlockAnimating ? "ON" : "off",
			containerAnimating ? "ON" : "off",
			dishAnimating ? "ON" : "off",
			tankAnimating ? "ON" : "off");
	glColor3f(consoleAnimating || airlockAnimating || containerAnimating ||
					  dishAnimating || tankAnimating
				  ? 0.2f
				  : 0.5f,
			  consoleAnimating || airlockAnimating || containerAnimating ||
					  dishAnimating || tankAnimating
				  ? 1.0f
				  : 0.5f,
			  0.3f);
	print(WINDOW_WIDTH - 510, 13, animStatus);

	glEnable(GL_DEPTH_TEST);

	// Restore 3D
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawWinScreen()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	// Semi-transparent overlay
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.2f, 0.1f, 0.8f);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(WINDOW_WIDTH, 0);
	glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
	glVertex2f(0, WINDOW_HEIGHT);
	glEnd();

	// Win text
	glColor3f(0.2f, 1.0f, 0.3f);
	printLarge(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 + 120, "MISSION ACCOMPLISHED!");

	glColor3f(0.9f, 0.9f, 0.9f);
	print(WINDOW_WIDTH / 2 - 130, WINDOW_HEIGHT / 2 + 70, "All Crystals Collected!");

	char timeStr[64];
	sprintf(timeStr, "Time Remaining: %d seconds", gameTime);
	print(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 + 30, timeStr);

	char scoreStr[64];
	sprintf(scoreStr, "Final Score: %d", score);
	print(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 10, scoreStr);

	char healthStr[64];
	sprintf(healthStr, "Health: %d%%", playerHealth);
	print(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 50, healthStr);

	float pulse = (sin(crystalBob * 2) + 1) / 2;
	glColor3f(pulse, pulse, 0.3f);
	print(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 100, "Press R to Restart");

	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawLoseScreen()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	// Semi-transparent overlay
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.2f, 0.0f, 0.0f, 0.8f);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(WINDOW_WIDTH, 0);
	glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
	glVertex2f(0, WINDOW_HEIGHT);
	glEnd();

	// Lose text
	glColor3f(1.0f, 0.3f, 0.2f);
	printLarge(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 + 120, "MISSION FAILED");

	glColor3f(0.9f, 0.9f, 0.9f);
	if (playerHealth <= 0)
	{
		print(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 + 70, "You Died!");
	}
	else
	{
		print(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 + 70, "Time Expired");
	}

	char crystalStr[64];
	sprintf(crystalStr, "Crystals Collected: %d / 3", collectedCrystals);
	print(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 30, crystalStr);

	char scoreStr[64];
	sprintf(scoreStr, "Score: %d", score);
	print(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 10, scoreStr);

	float pulse = (sin(crystalBob * 2) + 1) / 2;
	glColor3f(pulse, pulse * 0.5f, 0.2f);
	print(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 80, "Press R to Restart");

	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawInstructionsScreen()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	// Background
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.1f, 0.15f, 0.95f);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(WINDOW_WIDTH, 0);
	glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
	glVertex2f(0, WINDOW_HEIGHT);
	glEnd();

	// Title
	glColor3f(0.2f, 1.0f, 1.0f);
	printLarge(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT - 100, "SPACE STATION: ORBITAL RESCUE");

	// Mission briefing
	glColor3f(1.0f, 1.0f, 1.0f);
	print(100, WINDOW_HEIGHT - 160, "MISSION BRIEFING:");
	print(100, WINDOW_HEIGHT - 190, "You're aboard a damaged orbital station under siege!");
	print(100, WINDOW_HEIGHT - 215, "Find 3 ENERGY CRYSTALS hidden in MYSTERY BOXES throughout the station.");
	print(100, WINDOW_HEIGHT - 240, "Navigate platforms, avoid deadly hazards & hostile defense drones!");
	print(100, WINDOW_HEIGHT - 265, "Not all boxes contain crystals - choose wisely or waste time!");

	// Controls
	glColor3f(0.3f, 1.0f, 0.3f);
	print(100, WINDOW_HEIGHT - 315, "MOVEMENT CONTROLS:");
	glColor3f(0.9f, 0.9f, 0.9f);
	print(120, WINDOW_HEIGHT - 340, "WASD or Arrow Keys - Move (North/South/West/East)");
	print(120, WINDOW_HEIGHT - 365, "SPACE              - Jump Between Platforms");
	print(120, WINDOW_HEIGHT - 390, "E                  - Open Mystery Boxes");

	glColor3f(1.0f, 1.0f, 0.3f);
	print(600, WINDOW_HEIGHT - 315, "CAMERA VIEWS:");
	glColor3f(0.9f, 0.9f, 0.9f);
	print(620, WINDOW_HEIGHT - 340, "F - Follow View (Recommended)");
	print(620, WINDOW_HEIGHT - 365, "T - Top View");
	print(620, WINDOW_HEIGHT - 390, "V - Side View");
	print(620, WINDOW_HEIGHT - 415, "C - Free Camera (Lab 6)");

	glColor3f(0.6f, 0.8f, 1.0f);
	print(100, WINDOW_HEIGHT - 440, "FREE CAMERA MODE (Press C):");
	glColor3f(0.8f, 0.8f, 0.8f);
	print(120, WINDOW_HEIGHT - 465, "I/K - Forward/Backward   |   J/L - Strafe Left/Right");
	print(120, WINDOW_HEIGHT - 490, "U/O - Up/Down            |   Move Mouse (anywhere) - Look Around");

	glColor3f(1.0f, 0.8f, 0.3f);
	print(100, WINDOW_HEIGHT - 540, "OBJECT ANIMATIONS (Press keys to toggle):");
	glColor3f(0.8f, 0.8f, 0.8f);
	print(120, WINDOW_HEIGHT - 565, "1: Console   2: Airlock   3: Container   4: Dish   5: Tank");

	// Warnings
	glColor3f(1.0f, 0.3f, 0.2f);
	print(100, 150, "DANGER: Defense drones deal 15 HP damage!");
	print(100, 125, "TIP: 3 crystals on floor, 2 hidden in mystery boxes (2 with crystals, 3 empty)");

	// Start prompt
	float pulse = (sin(crystalBob * 3) + 1) / 2;
	glColor3f(pulse, pulse * 0.8f, 0.2f);
	printLarge(WINDOW_WIDTH / 2 - 150, 50, "PRESS ENTER TO START");

	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

// ==================== RENDERING ====================
void Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setupLighting();
	setupCamera();

	// Draw space environment first
	drawPlanets();
	drawAsteroids();
	drawStarfield();
	// Nebula removed - was creating flying rectangles

	// Draw scene
	drawFloor();

	// Draw platforms
	for (int i = 0; i < PLATFORM_COUNT; i++)
	{
		drawPlatform(platforms[i]);
	}

	// ASSIGNMENT REQUIREMENT: Draw walls with animated colors that change over time
	// Much larger color ranges for VISIBLE animation (0.2 to 0.8 range)
	float wallR = sin(wallColorTime * 0.5f) * 0.3f + 0.5f;
	float wallG = sin(wallColorTime * 0.7f + 1) * 0.3f + 0.5f;
	float wallB = sin(wallColorTime * 0.9f + 2) * 0.3f + 0.5f;

	drawWall(0, -BOUNDARY, 0, wallR, wallG, wallB);				   // Front
	drawWall(0, BOUNDARY, 180, wallR * 0.7f, wallG * 0.7f, wallB); // Back (darker)
	drawWall(-BOUNDARY, 0, 90, wallG, wallB, wallR);			   // Left (shifted hue)
	drawWall(BOUNDARY, 0, 270, wallB, wallR, wallG);			   // Right (shifted hue)

	drawPlayer();

	// Draw mystery boxes
	for (int i = 0; i < MAX_MYSTERY_BOXES; i++)
	{
		drawMysteryBox(mysteryBoxes[i]);
	}

	// Draw crystals (5 total: 2 from boxes + 3 on floor)
	for (int i = 0; i < 5; i++)
	{
		drawCrystal(crystals[i].pos.x, crystals[i].pos.y, crystals[i].pos.z, crystals[i].active);
	}

	// Draw enemies
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		drawEnemy(enemies[i]);
	}

	// ASSIGNMENT REQUIREMENT: Draw 5 animated objects (toggleable with keys 1-5)
	drawConsole();
	drawAirlock();
	drawContainer();
	drawDish();
	drawTank();

	// Draw particles last
	drawParticles();

	drawHUD();

	if (gameState == WIN)
	{
		drawWinScreen();
	}
	else if (gameState == LOSE)
	{
		drawLoseScreen();
	}
	else if (gameState == INSTRUCTIONS)
	{
		drawInstructionsScreen();
	}

	glFlush();
	glutSwapBuffers();
}

// ==================== INPUT HANDLERS ====================

void Keyboard(unsigned char key, int x, int y)
{
	keysPressed[key] = true;

	// Start game from instructions
	if (gameState == INSTRUCTIONS && (key == 13 || key == ' '))
	{
		gameState = PLAYING;
		lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000;
		return;
	}

	// Camera mode switching
	if (key == 'c' || key == 'C')
		cameraMode = FREE_CAM;
	if (key == 'f' || key == 'F')
		cameraMode = FRONT_VIEW;
	if (key == 't' || key == 'T')
		cameraMode = TOP_VIEW;
	if (key == 'v' || key == 'V')
		cameraMode = SIDE_VIEW;

	// Animation toggles
	if (key == '1')
	{
		consoleAnimating = !consoleAnimating;
		printf("Console animation: %s\n", consoleAnimating ? "ON" : "OFF");
	}
	if (key == '2')
	{
		airlockAnimating = !airlockAnimating;
		printf("Airlock animation: %s\n", airlockAnimating ? "ON" : "OFF");
	}
	if (key == '3')
	{
		containerAnimating = !containerAnimating;
		printf("Container animation: %s\n", containerAnimating ? "ON" : "OFF");
	}
	if (key == '4')
	{
		dishAnimating = !dishAnimating;
		printf("Dish animation: %s\n", dishAnimating ? "ON" : "OFF");
	}
	if (key == '5')
	{
		tankAnimating = !tankAnimating;
		printf("Tank animation: %s\n", tankAnimating ? "ON" : "OFF");
	}

	// Restart
	if (key == 'r' || key == 'R')
	{
		gameState = PLAYING;
		gameTime = 90;
		collectedCrystals = 0;
		score = 0;
		playerHealth = 100;
		damageFlash = 0;
		invincibleTime = 0;
		loseSoundPlayed = false;			 // Reset lose sound flag
		playerPos = Vector3(-20, 0.8f, -20); // Safe corner (scaled for 50x50)
		playerVel = Vector3(0, 0, 0);
		playerRotY = 180.0f;
		currentPlatform = -1;
		cameraMode = FRONT_VIEW;
		cameraAngleH = 0.0f;
		consoleAnimating = false;
		airlockAnimating = false;
		containerAnimating = false;
		dishAnimating = false;
		tankAnimating = false;
		airlockDoorOffset = 0;
		containerLidAngle = 0;

		// Reset crystals (they spawn from mystery boxes now, set inactive initially)
		for (int i = 0; i < 3; i++)
		{
			crystals[i].active = false;
		}

		// Reset enemies far from starting position (scaled for 50x50)
		enemies[0] = Enemy(17, 1.0f, -17);
		enemies[0].patrolAxis = true;
		enemies[0].vel = Vector3(0.03f, 0, 0);
		enemies[0].patrolMin = 13;
		enemies[0].patrolMax = 20;

		enemies[1] = Enemy(-13, 1.0f, 17);
		enemies[1].patrolAxis = true;
		enemies[1].vel = Vector3(0.03f, 0, 0);
		enemies[1].patrolMin = -17;
		enemies[1].patrolMax = -10;

		enemies[2] = Enemy(20, 1.0f, 8);
		enemies[2].patrolAxis = false;
		enemies[2].vel = Vector3(0, 0, 0.03f);
		enemies[2].patrolMin = 4;
		enemies[2].patrolMax = 12;

		enemies[3] = Enemy(0, 1.0f, 13);
		enemies[3].patrolAxis = false;
		enemies[3].vel = Vector3(0, 0, 0.03f);
		enemies[3].patrolMin = 10;
		enemies[3].patrolMax = 16;

		// Reset mystery boxes
		for (int i = 0; i < MAX_MYSTERY_BOXES; i++)
		{
			mysteryBoxes[i].opened = false;
			mysteryBoxes[i].openAngle = 0;
			mysteryBoxes[i].rotation = 0;
		}
		// Re-randomize which boxes have crystals (3 out of 5)
		for (int i = 0; i < MAX_MYSTERY_BOXES; i++)
		{
			mysteryBoxes[i].hasCrystal = false;
		}
		int boxesWithCrystals = 0;
		while (boxesWithCrystals < 3)
		{
			int idx = rand() % MAX_MYSTERY_BOXES;
			if (!mysteryBoxes[idx].hasCrystal)
			{
				mysteryBoxes[idx].hasCrystal = true;
				boxesWithCrystals++;
			}
		}

		// Reset particles
		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			particles[i].active = false;
		}

		// Reset animation variables
		starTwinkle = 0;
		nebulaShift = 0;

		lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000;
	}

	// Free camera movement keys (add to keysPressed for continuous movement)
	if (key == 'i' || key == 'I')
		keysPressed['i'] = true;
	if (key == 'k' || key == 'K')
		keysPressed['k'] = true;
	if (key == 'j' || key == 'J')
		keysPressed['j'] = true;
	if (key == 'l' || key == 'L')
		keysPressed['l'] = true;
	if (key == 'u' || key == 'U')
		keysPressed['u'] = true;
	if (key == 'o' || key == 'O')
		keysPressed['o'] = true;

	// Exit
	// if (key == 27) {
	// 	PostQuitMessage(0);
	// }
}

void KeyboardUp(unsigned char key, int x, int y)
{
	keysPressed[key] = false;
}

void SpecialInput(int key, int x, int y)
{
	// LAB 6 CAMERA: Arrow keys for rotation in FREE_CAM mode
	if (cameraMode == FREE_CAM)
	{
		float a = 2.0f; // Rotation speed
		switch (key)
		{
		case GLUT_KEY_UP:
			camera.rotateX(a);
			break;
		case GLUT_KEY_DOWN:
			camera.rotateX(-a);
			break;
		case GLUT_KEY_LEFT:
			camera.rotateY(a);
			break;
		case GLUT_KEY_RIGHT:
			camera.rotateY(-a);
			break;
		}
	}
	else
	{
		// For other camera modes, use for player movement
		switch (key)
		{
		case GLUT_KEY_UP:
			keysPressed['w'] = true;
			break;
		case GLUT_KEY_DOWN:
			keysPressed['s'] = true;
			break;
		case GLUT_KEY_LEFT:
			keysPressed['a'] = true;
			break;
		case GLUT_KEY_RIGHT:
			keysPressed['d'] = true;
			break;
		}
	}
}

void SpecialInputUp(int key, int x, int y)
{
	// Only handle key releases for player movement (not needed for camera rotation in FREE_CAM)
	if (cameraMode != FREE_CAM)
	{
		switch (key)
		{
		case GLUT_KEY_UP:
			keysPressed['w'] = false;
			break;
		case GLUT_KEY_DOWN:
			keysPressed['s'] = false;
			break;
		case GLUT_KEY_LEFT:
			keysPressed['a'] = false;
			break;
		case GLUT_KEY_RIGHT:
			keysPressed['d'] = false;
			break;
		}
	}
}

// ASSIGNMENT REQUIREMENT: Mouse camera movement (works on drag)
void MouseMotion(int x, int y)
{
	if (cameraMode != FREE_CAM)
		return; // Only works in FREE_CAM mode
	if (!mouseControlActive)
		return; // Only when button is held

	if (lastMouseX != -1 && lastMouseY != -1)
	{
		int dx = x - lastMouseX;
		int dy = y - lastMouseY;

		// Mouse sensitivity
		float sensitivity = 0.3f;

		// Rotate camera based on mouse movement
		camera.rotateY(dx * sensitivity);
		camera.rotateX(-dy * sensitivity);

		cameraAngleH += dx * sensitivity; // Update for particle billboarding
	}

	lastMouseX = x;
	lastMouseY = y;
}

// ASSIGNMENT REQUIREMENT: Passive mouse movement (works without button)
void PassiveMouseMotion(int x, int y)
{
	if (cameraMode != FREE_CAM)
		return; // Only works in FREE_CAM mode

	if (lastMouseX != -1 && lastMouseY != -1)
	{
		int dx = x - lastMouseX;
		int dy = y - lastMouseY;

		// Mouse sensitivity (slightly lower for passive)
		float sensitivity = 0.2f;

		// Rotate camera based on mouse movement
		camera.rotateY(dx * sensitivity);
		camera.rotateX(-dy * sensitivity);

		cameraAngleH += dx * sensitivity;
	}

	lastMouseX = x;
	lastMouseY = y;
}

void MouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			mouseControlActive = true;
			lastMouseX = x;
			lastMouseY = y;
		}
		else
		{
			mouseControlActive = false;
		}
	}
}

// ==================== MAIN LOOP ====================

void Timer(int value)
{
	if (gameState == PLAYING)
	{
		updatePlayer();
		checkCollisions();
		updateTimer();
	}

	updateAnimations();

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}

void main(int argc, char **argv)
{
	srand(time(NULL));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Space Station: Orbital Rescue");

	glutDisplayFunc(Display);
	glutTimerFunc(0, Timer, 0);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp);
	glutSpecialFunc(SpecialInput);
	glutSpecialUpFunc(SpecialInputUp);
	// ASSIGNMENT REQUIREMENT: Mouse camera control (drag and passive)
	glutMotionFunc(MouseMotion);			   // Mouse drag with button
	glutPassiveMotionFunc(PassiveMouseMotion); // Mouse move without button
	glutMouseFunc(MouseButton);

	glClearColor(0.02f, 0.02f, 0.08f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);

	// Initialize space environment
	initializeStars();

	// Initialize particles to inactive
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		particles[i].active = false;
	}

	lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000;

	// Play background music
	playSound(SND_BACKGROUND, true);

	glutMainLoop();
}