/**
 * DOOMERS - Input Manager
 * 
 * Handles keyboard, mouse input with support for:
 * - Key states (pressed, held, released)
 * - Mouse position and delta
 * - Mouse lock for FPS controls
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include <set>

namespace Doomers {

// ============================================================================
// Key Codes
// ============================================================================
enum class Key {
    // Letters
    A = 'a', B = 'b', C = 'c', D = 'd', E = 'e', F = 'f', G = 'g', H = 'h',
    I = 'i', J = 'j', K = 'k', L = 'l', M = 'm', N = 'n', O = 'o', P = 'p',
    Q = 'q', R = 'r', S = 's', T = 't', U = 'u', V = 'v', W = 'w', X = 'x',
    Y = 'y', Z = 'z',
    
    // Numbers
    Num0 = '0', Num1 = '1', Num2 = '2', Num3 = '3', Num4 = '4',
    Num5 = '5', Num6 = '6', Num7 = '7', Num8 = '8', Num9 = '9',
    
    // Special
    Space = ' ',
    Escape = 27,
    Tab = '\t',
    Enter = '\r',
    Backspace = '\b',
    
    // Function keys (special handling)
    F1 = 256 + GLUT_KEY_F1,
    F2 = 256 + GLUT_KEY_F2,
    F3 = 256 + GLUT_KEY_F3,
    F4 = 256 + GLUT_KEY_F4,
    F5 = 256 + GLUT_KEY_F5,
    F6 = 256 + GLUT_KEY_F6,
    F7 = 256 + GLUT_KEY_F7,
    F8 = 256 + GLUT_KEY_F8,
    F9 = 256 + GLUT_KEY_F9,
    F10 = 256 + GLUT_KEY_F10,
    F11 = 256 + GLUT_KEY_F11,
    F12 = 256 + GLUT_KEY_F12,
    
    // Arrow keys
    Up = 256 + GLUT_KEY_UP,
    Down = 256 + GLUT_KEY_DOWN,
    Left = 256 + GLUT_KEY_LEFT,
    Right = 256 + GLUT_KEY_RIGHT,
    
    // Modifiers
    Shift = 256 + 100,
    Ctrl = 256 + 101,
    Alt = 256 + 102
};

enum class MouseButton {
    Left = GLUT_LEFT_BUTTON,
    Middle = GLUT_MIDDLE_BUTTON,
    Right = GLUT_RIGHT_BUTTON
};

// ============================================================================
// Input Manager - Singleton
// ============================================================================
class InputManager {
public:
    static InputManager& instance() {
        static InputManager instance;
        return instance;
    }
    
    // ========================================================================
    // Initialization
    // ========================================================================
    void initialize(int windowWidth, int windowHeight) {
        screenWidth = windowWidth;
        screenHeight = windowHeight;
        screenCenterX = windowWidth / 2;
        screenCenterY = windowHeight / 2;
        
        // Register GLUT callbacks
        glutKeyboardFunc(keyboardCallback);
        glutKeyboardUpFunc(keyboardUpCallback);
        glutSpecialFunc(specialCallback);
        glutSpecialUpFunc(specialUpCallback);
        glutMouseFunc(mouseButtonCallback);
        glutPassiveMotionFunc(mouseMotionCallback);
        glutMotionFunc(mouseMotionCallback);
        
        LOG_INFO("Input Manager initialized");
    }
    
    void resize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
        screenCenterX = width / 2;
        screenCenterY = height / 2;
    }
    
    // ========================================================================
    // Update (call once per frame, at the end)
    // ========================================================================
    void update() {
        // Copy current keys to previous
        previousKeys = currentKeys;
        previousMouseButtons = currentMouseButtons;
        
        // Reset deltas
        mouseDeltaX = 0;
        mouseDeltaY = 0;
        mouseWheelDelta = 0;
        
        // Handle mouse lock
        if (mouseLocked) {
            // Warp cursor back to center
            glutWarpPointer(screenCenterX, screenCenterY);
            mouseX = screenCenterX;
            mouseY = screenCenterY;
        }
    }
    
    // ========================================================================
    // Keyboard Queries
    // ========================================================================
    bool isKeyDown(Key key) const {
        return currentKeys.count((int)key) > 0;
    }
    
    bool isKeyDown(char key) const {
        // Handle both upper and lower case
        int k = tolower(key);
        return currentKeys.count(k) > 0 || currentKeys.count(toupper(key)) > 0;
    }
    
    bool isKeyPressed(Key key) const {
        int k = (int)key;
        return currentKeys.count(k) > 0 && previousKeys.count(k) == 0;
    }
    
    bool isKeyPressed(char key) const {
        int k = tolower(key);
        bool current = currentKeys.count(k) > 0 || currentKeys.count(toupper(key)) > 0;
        bool previous = previousKeys.count(k) > 0 || previousKeys.count(toupper(key)) > 0;
        return current && !previous;
    }
    
    bool isKeyReleased(Key key) const {
        int k = (int)key;
        return currentKeys.count(k) == 0 && previousKeys.count(k) > 0;
    }
    
    bool isShiftDown() const {
        return modifierShift;
    }
    
    bool isCtrlDown() const {
        return modifierCtrl;
    }
    
    bool isAltDown() const {
        return modifierAlt;
    }
    
    // ========================================================================
    // Mouse Queries
    // ========================================================================
    bool isMouseButtonDown(MouseButton button) const {
        return currentMouseButtons.count((int)button) > 0;
    }
    
    bool isMouseButtonPressed(MouseButton button) const {
        int b = (int)button;
        return currentMouseButtons.count(b) > 0 && previousMouseButtons.count(b) == 0;
    }
    
    bool isMouseButtonReleased(MouseButton button) const {
        int b = (int)button;
        return currentMouseButtons.count(b) == 0 && previousMouseButtons.count(b) > 0;
    }
    
    int getMouseX() const { return mouseX; }
    int getMouseY() const { return mouseY; }
    int getMouseDeltaX() const { return mouseDeltaX; }
    int getMouseDeltaY() const { return mouseDeltaY; }
    int getMouseWheelDelta() const { return mouseWheelDelta; }
    
    Math::Vector2 getMousePosition() const {
        return Math::Vector2((float)mouseX, (float)mouseY);
    }
    
    Math::Vector2 getMouseDelta() const {
        return Math::Vector2((float)mouseDeltaX, (float)mouseDeltaY);
    }
    
    // Normalized mouse position (0-1)
    Math::Vector2 getNormalizedMousePosition() const {
        return Math::Vector2(
            (float)mouseX / (float)screenWidth,
            (float)mouseY / (float)screenHeight
        );
    }
    
    // ========================================================================
    // Mouse Lock (for FPS camera)
    // ========================================================================
    void setMouseLock(bool locked) {
        mouseLocked = locked;
        if (locked) {
            glutSetCursor(GLUT_CURSOR_NONE);
            glutWarpPointer(screenCenterX, screenCenterY);
            mouseX = screenCenterX;
            mouseY = screenCenterY;
        }
        else {
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
    }
    
    bool isMouseLocked() const { return mouseLocked; }
    
    void toggleMouseLock() {
        setMouseLock(!mouseLocked);
    }
    
    // ========================================================================
    // Movement Helpers
    // ========================================================================
    Math::Vector2 getMovementInput() const {
        Math::Vector2 input(0, 0);
        
        if (isKeyDown('w') || isKeyDown(Key::Up)) input.y += 1.0f;
        if (isKeyDown('s') || isKeyDown(Key::Down)) input.y -= 1.0f;
        if (isKeyDown('a') || isKeyDown(Key::Left)) input.x -= 1.0f;
        if (isKeyDown('d') || isKeyDown(Key::Right)) input.x += 1.0f;
        
        // Normalize diagonal movement
        if (input.lengthSquared() > 1.0f) {
            input.normalize();
        }
        
        return input;
    }
    
    bool isJumpPressed() const {
        return isKeyPressed(Key::Space);
    }
    
    bool isSprintDown() const {
        return isShiftDown();
    }
    
    bool isFireDown() const {
        return isMouseButtonDown(MouseButton::Left);
    }
    
    bool isFirePressed() const {
        return isMouseButtonPressed(MouseButton::Left);
    }
    
    bool isAimDown() const {
        return isMouseButtonDown(MouseButton::Right);
    }
    
    bool isReloadPressed() const {
        return isKeyPressed('r');
    }
    
    bool isInteractPressed() const {
        return isKeyPressed('e');
    }
    
    // ========================================================================
    // Direct State Setters (for external GLUT callbacks in main.cpp)
    // ========================================================================
    void setKeyPressed(unsigned char key, bool pressed) {
        if (pressed) {
            currentKeys.insert(key);
            currentKeys.insert(tolower(key));
        } else {
            currentKeys.erase(key);
            currentKeys.erase(tolower(key));
            currentKeys.erase(toupper(key));
        }
    }
    
    void setSpecialKeyPressed(int key, bool pressed) {
        if (pressed) {
            currentKeys.insert(256 + key);
        } else {
            currentKeys.erase(256 + key);
        }
    }
    
    void setMouseButton(int button, bool pressed) {
        if (pressed) {
            currentMouseButtons.insert(button);
        } else {
            currentMouseButtons.erase(button);
        }
    }
    
    void updateMousePosition(int x, int y) {
        if (mouseLocked) {
            mouseDeltaX = x - screenCenterX;
            mouseDeltaY = y - screenCenterY;
        } else {
            mouseDeltaX = x - mouseX;
            mouseDeltaY = y - mouseY;
        }
        mouseX = x;
        mouseY = y;
    }
    
    void setMouseLocked(bool locked) {
        mouseLocked = locked;
        if (locked) {
            glutSetCursor(GLUT_CURSOR_NONE);
            glutWarpPointer(screenCenterX, screenCenterY);
            mouseX = screenCenterX;
            mouseY = screenCenterY;
        } else {
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
    }
    
    // Query for just pressed (single frame)
    bool isKeyJustPressed(unsigned char key) const {
        int k = tolower(key);
        bool current = currentKeys.count(k) > 0 || currentKeys.count(toupper(key)) > 0;
        bool previous = previousKeys.count(k) > 0 || previousKeys.count(toupper(key)) > 0;
        return current && !previous;
    }
    
    bool isMouseButtonPressed(int button) const {
        return currentMouseButtons.count(button) > 0 && 
               previousMouseButtons.count(button) == 0;
    }
    
private:
    InputManager() 
        : mouseX(0), mouseY(0)
        , mouseDeltaX(0), mouseDeltaY(0)
        , mouseWheelDelta(0)
        , mouseLocked(false)
        , modifierShift(false), modifierCtrl(false), modifierAlt(false)
        , screenWidth(1280), screenHeight(720)
        , screenCenterX(640), screenCenterY(360)
    {}
    
    // GLUT Callbacks (static)
    static void keyboardCallback(unsigned char key, int x, int y) {
        instance().currentKeys.insert(key);
        instance().currentKeys.insert(tolower(key)); // Store both cases
        // Cache modifiers (can only call glutGetModifiers inside callback)
        int mods = glutGetModifiers();
        instance().modifierShift = (mods & GLUT_ACTIVE_SHIFT) != 0;
        instance().modifierCtrl = (mods & GLUT_ACTIVE_CTRL) != 0;
        instance().modifierAlt = (mods & GLUT_ACTIVE_ALT) != 0;
    }
    
    static void keyboardUpCallback(unsigned char key, int x, int y) {
        instance().currentKeys.erase(key);
        instance().currentKeys.erase(tolower(key));
        instance().currentKeys.erase(toupper(key));
        // Cache modifiers
        int mods = glutGetModifiers();
        instance().modifierShift = (mods & GLUT_ACTIVE_SHIFT) != 0;
        instance().modifierCtrl = (mods & GLUT_ACTIVE_CTRL) != 0;
        instance().modifierAlt = (mods & GLUT_ACTIVE_ALT) != 0;
    }
    
    static void specialCallback(int key, int x, int y) {
        instance().currentKeys.insert(256 + key);
        // Cache modifiers
        int mods = glutGetModifiers();
        instance().modifierShift = (mods & GLUT_ACTIVE_SHIFT) != 0;
        instance().modifierCtrl = (mods & GLUT_ACTIVE_CTRL) != 0;
        instance().modifierAlt = (mods & GLUT_ACTIVE_ALT) != 0;
    }
    
    static void specialUpCallback(int key, int x, int y) {
        instance().currentKeys.erase(256 + key);
        // Cache modifiers
        int mods = glutGetModifiers();
        instance().modifierShift = (mods & GLUT_ACTIVE_SHIFT) != 0;
        instance().modifierCtrl = (mods & GLUT_ACTIVE_CTRL) != 0;
        instance().modifierAlt = (mods & GLUT_ACTIVE_ALT) != 0;
    }
    
    static void mouseButtonCallback(int button, int state, int x, int y) {
        if (state == GLUT_DOWN) {
            instance().currentMouseButtons.insert(button);
        }
        else {
            instance().currentMouseButtons.erase(button);
        }
        instance().mouseX = x;
        instance().mouseY = y;
    }
    
    static void mouseMotionCallback(int x, int y) {
        InputManager& im = instance();
        
        if (im.mouseLocked) {
            im.mouseDeltaX = x - im.screenCenterX;
            im.mouseDeltaY = y - im.screenCenterY;
        }
        else {
            im.mouseDeltaX = x - im.mouseX;
            im.mouseDeltaY = y - im.mouseY;
        }
        
        im.mouseX = x;
        im.mouseY = y;
    }
    
    // State
    std::set<int> currentKeys;
    std::set<int> previousKeys;
    std::set<int> currentMouseButtons;
    std::set<int> previousMouseButtons;
    
    int mouseX, mouseY;
    int mouseDeltaX, mouseDeltaY;
    int mouseWheelDelta;
    bool mouseLocked;
    
    // Cached modifier states (since glutGetModifiers only works in callbacks)
    bool modifierShift;
    bool modifierCtrl;
    bool modifierAlt;
    
    int screenWidth, screenHeight;
    int screenCenterX, screenCenterY;
};

// Convenience accessor
inline InputManager& Input() {
    return InputManager::instance();
}

} // namespace Doomers
