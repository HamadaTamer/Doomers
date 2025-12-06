// ============================================================================
// DOOMERS - Main Entry Point
// A fast-paced sci-fi shooter combining FPS and TPS perspectives
// GUC - Computer Graphics Project 2025
// ============================================================================

#include <windows.h>
#include <glut.h>
#include <stdio.h>
#include "src/Game.h"

// ============================================================================
// FORCE DEDICATED GPU (NVIDIA/AMD) - This tells Windows to use your gaming GPU
// ============================================================================
extern "C" {
    // For NVIDIA GPUs - forces high performance GPU
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    // For AMD GPUs - forces high performance GPU
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Debug file
FILE* g_debugFile = NULL;

void initDebugLog() {
    g_debugFile = fopen("C:\\Users\\youss\\Desktop\\doomers_crash.txt", "w");
    if (g_debugFile) {
        fprintf(g_debugFile, "=== DOOMERS DEBUG LOG ===\n");
        fflush(g_debugFile);
    }
}

void debugLog(const char* msg) {
    if (g_debugFile) {
        fprintf(g_debugFile, "%s\n", msg);
        fflush(g_debugFile);
    }
}

// Global game instance
Game game;

// ==================== GLUT CALLBACKS ====================

void display() {
    game.render();
}

void idle() {
    game.update();
}

void reshape(int width, int height) {
    game.onResize(width, height);
}

void keyboard(unsigned char key, int x, int y) {
    game.onKeyDown(key);
}

void keyboardUp(unsigned char key, int x, int y) {
    game.onKeyUp(key);
}

void specialKeys(int key, int x, int y) {
    game.onSpecialKeyDown(key);
}

void specialKeysUp(int key, int x, int y) {
    game.onSpecialKeyUp(key);
}

void mouseMotion(int x, int y) {
    game.onMouseMove(x, y);
}

void passiveMouseMotion(int x, int y) {
    game.onMouseMove(x, y);
}

void mouseButton(int button, int state, int x, int y) {
    game.onMouseButton(button, state, x, y);
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

// ==================== MAIN ====================

int main(int argc, char** argv) {
    // Initialize debug logging
    initDebugLog();
    debugLog("Starting DOOMERS...");
    
    // Initialize GLUT
    debugLog("Initializing GLUT...");
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 50);
    glutCreateWindow(WINDOW_TITLE);
    debugLog("GLUT window created");
    
    // Register callbacks
    debugLog("Registering callbacks...");
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(passiveMouseMotion);
    glutMouseFunc(mouseButton);
    
    // Setup timer for consistent frame rate
    glutTimerFunc(0, timer, 0);
    
    // Initialize game
    debugLog("Initializing game...");
    game.init();
    debugLog("Game initialized, entering main loop");
    
    // Enter main loop
    glutMainLoop();
    
    return 0;
}
