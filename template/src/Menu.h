// ============================================================================
// DOOMERS - Menu.h
// Main menu and pause menu system
// ============================================================================
#ifndef MENU_H
#define MENU_H

#include "GameConfig.h"
#include <glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

enum MenuType {
    MENU_MAIN,
    MENU_PAUSE,
    MENU_INSTRUCTIONS,
    MENU_GAME_OVER,
    MENU_LEVEL_COMPLETE,
    MENU_WIN
};

class Menu {
public:
    int screenWidth;
    int screenHeight;
    int selectedOption;
    int maxOptions;
    float animTime;
    MenuType currentMenu;
    
    // Stats for end screens
    int finalScore;
    int enemiesKilled;
    float timeElapsed;
    
    Menu() {
        screenWidth = WINDOW_WIDTH;
        screenHeight = WINDOW_HEIGHT;
        selectedOption = 0;
        maxOptions = 3;
        animTime = 0.0f;
        currentMenu = MENU_MAIN;
        finalScore = 0;
        enemiesKilled = 0;
        timeElapsed = 0.0f;
    }
    
    void setScreenSize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
    }
    
    void update(float deltaTime) {
        animTime += deltaTime;
    }
    
    void selectNext() {
        selectedOption++;
        if (selectedOption >= maxOptions) selectedOption = 0;
    }
    
    void selectPrev() {
        selectedOption--;
        if (selectedOption < 0) selectedOption = maxOptions - 1;
    }
    
    int getSelected() const {
        return selectedOption;
    }
    
    void setMenu(MenuType type) {
        currentMenu = type;
        selectedOption = 0;
        
        switch (type) {
            case MENU_MAIN:
                maxOptions = 3;
                break;
            case MENU_PAUSE:
                maxOptions = 3;
                break;
            case MENU_INSTRUCTIONS:
                maxOptions = 1;
                break;
            case MENU_GAME_OVER:
            case MENU_LEVEL_COMPLETE:
            case MENU_WIN:
                maxOptions = 2;
                break;
        }
    }
    
    void beginMenu() {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, screenWidth, 0, screenHeight);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
    }
    
    void endMenu() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
        glRasterPos2f(x, y);
        int len = strlen(text);
        for (int i = 0; i < len; i++) {
            glutBitmapCharacter(font, text[i]);
        }
    }
    
    void drawTextCentered(float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
        int len = strlen(text);
        int width = 0;
        for (int i = 0; i < len; i++) {
            width += glutBitmapWidth(font, text[i]);
        }
        float x = (screenWidth - width) / 2.0f;
        drawText(x, y, text, font);
    }
    
    void drawBackground() {
        // Animated dark gradient background
        glBegin(GL_QUADS);
        
        float pulse = sin(animTime) * 0.05f + 0.1f;
        
        glColor3f(0.05f + pulse, 0.02f, 0.08f + pulse);
        glVertex2f(0, 0);
        glVertex2f(screenWidth, 0);
        
        glColor3f(0.02f, 0.02f, 0.05f);
        glVertex2f(screenWidth, screenHeight);
        glVertex2f(0, screenHeight);
        glEnd();
        
        // Draw some animated particles/stars
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 50; i++) {
            float x = (float)((i * 137) % screenWidth);
            float y = (float)((i * 97 + (int)(animTime * 20)) % screenHeight);
            float brightness = sin(animTime * 2.0f + i) * 0.3f + 0.5f;
            glColor3f(brightness, brightness, brightness * 1.2f);
            glVertex2f(x, y);
        }
        glEnd();
    }
    
    void drawTitle() {
        // Draw animated title
        float bounce = sin(animTime * 2.0f) * 10.0f;
        
        // Shadow
        glColor3f(0.0f, 0.0f, 0.0f);
        drawTextCentered(screenHeight - 140 + bounce + 4, "D O O M E R S", GLUT_BITMAP_TIMES_ROMAN_24);
        drawTextCentered(screenHeight - 140 + bounce + 3, "D O O M E R S", GLUT_BITMAP_TIMES_ROMAN_24);
        
        // Main title with color animation
        float r = sin(animTime * 1.5f) * 0.2f + 0.8f;
        float g = sin(animTime * 1.5f + 2.0f) * 0.1f + 0.2f;
        float b = sin(animTime * 1.5f + 4.0f) * 0.1f + 0.1f;
        glColor3f(r, g, b);
        drawTextCentered(screenHeight - 140 + bounce, "D O O M E R S", GLUT_BITMAP_TIMES_ROMAN_24);
        
        // Subtitle
        glColor3f(0.6f, 0.6f, 0.7f);
        drawTextCentered(screenHeight - 180, "Escape the Demon-Infested Facility");
    }
    
    void drawMenuOption(int index, const char* text, float y) {
        bool isSelected = (index == selectedOption);
        
        if (isSelected) {
            // Highlight box
            float pulse = sin(animTime * 5.0f) * 0.1f + 0.9f;
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            float boxWidth = 300.0f;
            float boxHeight = 40.0f;
            float x = (screenWidth - boxWidth) / 2.0f;
            
            glColor4f(0.0f, 0.5f * pulse, 0.8f * pulse, 0.3f);
            glBegin(GL_QUADS);
            glVertex2f(x, y - 10);
            glVertex2f(x + boxWidth, y - 10);
            glVertex2f(x + boxWidth, y + boxHeight - 10);
            glVertex2f(x, y + boxHeight - 10);
            glEnd();
            
            // Border
            glColor3f(0.0f, 0.8f * pulse, 1.0f * pulse);
            glLineWidth(2.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y - 10);
            glVertex2f(x + boxWidth, y - 10);
            glVertex2f(x + boxWidth, y + boxHeight - 10);
            glVertex2f(x, y + boxHeight - 10);
            glEnd();
            
            // Arrow indicator
            drawText(x - 30, y + 5, ">", GLUT_BITMAP_TIMES_ROMAN_24);
            
            glDisable(GL_BLEND);
            
            // Selected text color
            glColor3f(1.0f, 1.0f, 1.0f);
        } else {
            glColor3f(0.6f, 0.6f, 0.6f);
        }
        
        drawTextCentered(y + 5, text, GLUT_BITMAP_HELVETICA_18);
        glLineWidth(1.0f);
    }
    
    void drawMainMenu() {
        drawBackground();
        drawTitle();
        
        float startY = screenHeight / 2.0f + 40;
        float spacing = 60.0f;
        
        drawMenuOption(0, "START GAME", startY);
        drawMenuOption(1, "INSTRUCTIONS", startY - spacing);
        drawMenuOption(2, "EXIT", startY - spacing * 2);
        
        // Footer
        glColor3f(0.4f, 0.4f, 0.4f);
        drawTextCentered(50, "Use UP/DOWN arrows to select, ENTER to confirm");
        drawTextCentered(30, "GUC - Computer Graphics Project 2025");
    }
    
    void drawPauseMenu() {
        // Semi-transparent overlay
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(screenWidth, 0);
        glVertex2f(screenWidth, screenHeight);
        glVertex2f(0, screenHeight);
        glEnd();
        
        glDisable(GL_BLEND);
        
        // Pause title
        glColor3f(1.0f, 0.8f, 0.0f);
        drawTextCentered(screenHeight - 200, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
        
        float startY = screenHeight / 2.0f + 20;
        float spacing = 60.0f;
        
        drawMenuOption(0, "RESUME", startY);
        drawMenuOption(1, "RESTART", startY - spacing);
        drawMenuOption(2, "QUIT TO MENU", startY - spacing * 2);
    }
    
    void drawInstructions() {
        drawBackground();
        
        glColor3f(1.0f, 0.8f, 0.0f);
        drawTextCentered(screenHeight - 100, "HOW TO PLAY", GLUT_BITMAP_TIMES_ROMAN_24);
        
        float y = screenHeight - 180;
        float spacing = 35.0f;
        
        glColor3f(0.0f, 0.8f, 1.0f);
        drawTextCentered(y, "=== MOVEMENT ===");
        y -= spacing;
        
        glColor3f(1.0f, 1.0f, 1.0f);
        drawTextCentered(y, "W/A/S/D - Move Forward/Left/Backward/Right");
        y -= spacing;
        drawTextCentered(y, "SPACE - Jump");
        y -= spacing;
        drawTextCentered(y, "SHIFT - Sprint");
        y -= spacing;
        drawTextCentered(y, "Mouse - Look Around");
        y -= spacing * 1.5f;
        
        glColor3f(0.0f, 0.8f, 1.0f);
        drawTextCentered(y, "=== COMBAT ===");
        y -= spacing;
        
        glColor3f(1.0f, 1.0f, 1.0f);
        drawTextCentered(y, "Left Mouse - Shoot");
        y -= spacing;
        drawTextCentered(y, "Right Mouse - Toggle First/Third Person View");
        y -= spacing;
        drawTextCentered(y, "F - Toggle Flashlight");
        y -= spacing * 1.5f;
        
        glColor3f(0.0f, 0.8f, 1.0f);
        drawTextCentered(y, "=== OBJECTIVES ===");
        y -= spacing;
        
        glColor3f(1.0f, 1.0f, 1.0f);
        drawTextCentered(y, "- Kill all enemies");
        y -= spacing;
        drawTextCentered(y, "- Collect health packs and ammo");
        y -= spacing;
        drawTextCentered(y, "- Find keycards to unlock doors");
        y -= spacing;
        drawTextCentered(y, "- Reach the portal/objective to complete each level");
        
        y -= spacing * 2;
        drawMenuOption(0, "BACK TO MENU", y);
    }
    
    void drawGameOver() {
        drawBackground();
        
        // Title
        glColor3f(0.8f, 0.0f, 0.0f);
        drawTextCentered(screenHeight - 150, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
        
        // Stats
        float y = screenHeight / 2.0f + 50;
        char text[64];
        
        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(text, "Final Score: %d", finalScore);
        drawTextCentered(y, text);
        y -= 40;
        
        sprintf(text, "Enemies Killed: %d", enemiesKilled);
        drawTextCentered(y, text);
        y -= 40;
        
        int mins = (int)timeElapsed / 60;
        int secs = (int)timeElapsed % 60;
        sprintf(text, "Time Survived: %02d:%02d", mins, secs);
        drawTextCentered(y, text);
        
        y -= 80;
        drawMenuOption(0, "TRY AGAIN", y);
        drawMenuOption(1, "MAIN MENU", y - 60);
    }
    
    void drawLevelComplete() {
        drawBackground();
        
        // Title
        glColor3f(0.0f, 1.0f, 0.5f);
        drawTextCentered(screenHeight - 150, "LEVEL COMPLETE!", GLUT_BITMAP_TIMES_ROMAN_24);
        
        // Stats
        float y = screenHeight / 2.0f + 50;
        char text[64];
        
        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(text, "Score: %d", finalScore);
        drawTextCentered(y, text);
        y -= 40;
        
        sprintf(text, "Enemies Killed: %d", enemiesKilled);
        drawTextCentered(y, text);
        
        y -= 80;
        drawMenuOption(0, "CONTINUE", y);
        drawMenuOption(1, "MAIN MENU", y - 60);
    }
    
    void drawWinScreen() {
        drawBackground();
        
        // Animated victory title
        float bounce = sin(animTime * 3.0f) * 5.0f;
        float r = sin(animTime * 2.0f) * 0.3f + 0.7f;
        float g = sin(animTime * 2.0f + 2.0f) * 0.3f + 0.7f;
        float b = sin(animTime * 2.0f + 4.0f) * 0.3f + 0.7f;
        
        glColor3f(r, g, b);
        drawTextCentered(screenHeight - 130 + bounce, "VICTORY!", GLUT_BITMAP_TIMES_ROMAN_24);
        
        glColor3f(0.8f, 0.8f, 0.8f);
        drawTextCentered(screenHeight - 170, "You escaped the facility!");
        
        // Final stats
        float y = screenHeight / 2.0f + 70;
        char text[64];
        
        glColor3f(1.0f, 0.8f, 0.0f);
        sprintf(text, "FINAL SCORE: %d", finalScore);
        drawTextCentered(y, text, GLUT_BITMAP_TIMES_ROMAN_24);
        y -= 50;
        
        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(text, "Total Enemies Killed: %d", enemiesKilled);
        drawTextCentered(y, text);
        y -= 35;
        
        int mins = (int)timeElapsed / 60;
        int secs = (int)timeElapsed % 60;
        sprintf(text, "Total Time: %02d:%02d", mins, secs);
        drawTextCentered(y, text);
        
        y -= 80;
        drawMenuOption(0, "PLAY AGAIN", y);
        drawMenuOption(1, "MAIN MENU", y - 60);
    }
    
    void draw() {
        beginMenu();
        
        switch (currentMenu) {
            case MENU_MAIN:
                drawMainMenu();
                break;
            case MENU_PAUSE:
                drawPauseMenu();
                break;
            case MENU_INSTRUCTIONS:
                drawInstructions();
                break;
            case MENU_GAME_OVER:
                drawGameOver();
                break;
            case MENU_LEVEL_COMPLETE:
                drawLevelComplete();
                break;
            case MENU_WIN:
                drawWinScreen();
                break;
        }
        
        endMenu();
    }
};

#endif // MENU_H
