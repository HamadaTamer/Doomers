/**
 * DOOMERS - Animation & Interpolation System
 * 
 * Provides smooth interpolation, easing functions, and animation utilities
 * for professional-quality motion and transitions.
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include <functional>

namespace Doomers {
namespace Anim {

// ============================================================================
// Easing Functions - For smooth, professional animations
// ============================================================================
namespace Ease {
    // Linear (no easing)
    inline float Linear(float t) { return t; }
    
    // Quadratic
    inline float InQuad(float t) { return t * t; }
    inline float OutQuad(float t) { return t * (2.0f - t); }
    inline float InOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }
    
    // Cubic
    inline float InCubic(float t) { return t * t * t; }
    inline float OutCubic(float t) { float t1 = t - 1.0f; return t1 * t1 * t1 + 1.0f; }
    inline float InOutCubic(float t) {
        return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
    }
    
    // Quartic
    inline float InQuart(float t) { return t * t * t * t; }
    inline float OutQuart(float t) { float t1 = t - 1.0f; return 1.0f - t1 * t1 * t1 * t1; }
    inline float InOutQuart(float t) {
        float t1 = t - 1.0f;
        return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - 8.0f * t1 * t1 * t1 * t1;
    }
    
    // Quintic
    inline float InQuint(float t) { return t * t * t * t * t; }
    inline float OutQuint(float t) { float t1 = t - 1.0f; return 1.0f + t1 * t1 * t1 * t1 * t1; }
    inline float InOutQuint(float t) {
        float t1 = t - 1.0f;
        return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f + 16.0f * t1 * t1 * t1 * t1 * t1;
    }
    
    // Sinusoidal
    inline float InSine(float t) { return 1.0f - cosf(t * Math::PI * 0.5f); }
    inline float OutSine(float t) { return sinf(t * Math::PI * 0.5f); }
    inline float InOutSine(float t) { return 0.5f * (1.0f - cosf(Math::PI * t)); }
    
    // Exponential
    inline float InExpo(float t) { return t == 0.0f ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f)); }
    inline float OutExpo(float t) { return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t); }
    inline float InOutExpo(float t) {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        if (t < 0.5f) return 0.5f * powf(2.0f, 20.0f * t - 10.0f);
        return 1.0f - 0.5f * powf(2.0f, -20.0f * t + 10.0f);
    }
    
    // Circular
    inline float InCirc(float t) { return 1.0f - sqrtf(1.0f - t * t); }
    inline float OutCirc(float t) { float t1 = t - 1.0f; return sqrtf(1.0f - t1 * t1); }
    inline float InOutCirc(float t) {
        if (t < 0.5f) return 0.5f * (1.0f - sqrtf(1.0f - 4.0f * t * t));
        float t1 = 2.0f * t - 2.0f;
        return 0.5f * (sqrtf(1.0f - t1 * t1) + 1.0f);
    }
    
    // Back (overshoots)
    inline float InBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return c3 * t * t * t - c1 * t * t;
    }
    inline float OutBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        float t1 = t - 1.0f;
        return 1.0f + c3 * t1 * t1 * t1 + c1 * t1 * t1;
    }
    inline float InOutBack(float t) {
        const float c1 = 1.70158f;
        const float c2 = c1 * 1.525f;
        if (t < 0.5f) {
            return 0.5f * (4.0f * t * t * ((c2 + 1.0f) * 2.0f * t - c2));
        }
        float t1 = 2.0f * t - 2.0f;
        return 0.5f * (t1 * t1 * ((c2 + 1.0f) * t1 + c2) + 2.0f);
    }
    
    // Elastic (bouncy spring)
    inline float InElastic(float t) {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        return -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * (2.0f * Math::PI / 3.0f));
    }
    inline float OutElastic(float t) {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * (2.0f * Math::PI / 3.0f)) + 1.0f;
    }
    inline float InOutElastic(float t) {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        const float c5 = (2.0f * Math::PI) / 4.5f;
        if (t < 0.5f) {
            return -0.5f * powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * c5);
        }
        return powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * c5) * 0.5f + 1.0f;
    }
    
    // Bounce
    inline float OutBounce(float t) {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;
        if (t < 1.0f / d1) {
            return n1 * t * t;
        } else if (t < 2.0f / d1) {
            t -= 1.5f / d1;
            return n1 * t * t + 0.75f;
        } else if (t < 2.5f / d1) {
            t -= 2.25f / d1;
            return n1 * t * t + 0.9375f;
        } else {
            t -= 2.625f / d1;
            return n1 * t * t + 0.984375f;
        }
    }
    inline float InBounce(float t) { return 1.0f - OutBounce(1.0f - t); }
    inline float InOutBounce(float t) {
        return t < 0.5f
            ? (1.0f - OutBounce(1.0f - 2.0f * t)) * 0.5f
            : (1.0f + OutBounce(2.0f * t - 1.0f)) * 0.5f;
    }
}

// Easing function type
using EaseFunc = float(*)(float);

// ============================================================================
// Interpolation utilities
// ============================================================================
inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline Math::Vector3 lerp(const Math::Vector3& a, const Math::Vector3& b, float t) {
    return Math::Vector3(
        lerp(a.x, b.x, t),
        lerp(a.y, b.y, t),
        lerp(a.z, b.z, t)
    );
}

inline Math::Color lerp(const Math::Color& a, const Math::Color& b, float t) {
    return Math::Color(
        lerp(a.r, b.r, t),
        lerp(a.g, b.g, t),
        lerp(a.b, b.b, t),
        lerp(a.a, b.a, t)
    );
}

// Smooth damp - Unity-style smooth following
inline float smoothDamp(float current, float target, float& velocity, float smoothTime, float maxSpeed, float deltaTime) {
    smoothTime = fmaxf(0.0001f, smoothTime);
    float omega = 2.0f / smoothTime;
    float x = omega * deltaTime;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    float originalTo = target;
    
    float maxChange = maxSpeed * smoothTime;
    change = fmaxf(-maxChange, fminf(maxChange, change));
    target = current - change;
    
    float temp = (velocity + omega * change) * deltaTime;
    velocity = (velocity - omega * temp) * exp;
    float output = target + (change + temp) * exp;
    
    if (originalTo - current > 0.0f == output > originalTo) {
        output = originalTo;
        velocity = (output - originalTo) / deltaTime;
    }
    
    return output;
}

inline Math::Vector3 smoothDamp(const Math::Vector3& current, const Math::Vector3& target, 
                                 Math::Vector3& velocity, float smoothTime, float maxSpeed, float deltaTime) {
    return Math::Vector3(
        smoothDamp(current.x, target.x, velocity.x, smoothTime, maxSpeed, deltaTime),
        smoothDamp(current.y, target.y, velocity.y, smoothTime, maxSpeed, deltaTime),
        smoothDamp(current.z, target.z, velocity.z, smoothTime, maxSpeed, deltaTime)
    );
}

// ============================================================================
// Tween - Animated value over time
// ============================================================================
template<typename T>
class Tween {
public:
    Tween() : duration(1.0f), elapsed(0.0f), easeFunc(Ease::Linear), playing(false), loop(false) {}
    
    void start(const T& from, const T& to, float dur, EaseFunc ease = Ease::Linear) {
        startValue = from;
        endValue = to;
        currentValue = from;
        duration = dur;
        elapsed = 0.0f;
        easeFunc = ease;
        playing = true;
    }
    
    void update(float deltaTime) {
        if (!playing) return;
        
        elapsed += deltaTime;
        float t = elapsed / duration;
        
        if (t >= 1.0f) {
            if (loop) {
                elapsed = fmodf(elapsed, duration);
                t = elapsed / duration;
            } else {
                t = 1.0f;
                playing = false;
            }
        }
        
        float easedT = easeFunc(t);
        currentValue = lerp(startValue, endValue, easedT);
    }
    
    const T& getValue() const { return currentValue; }
    bool isPlaying() const { return playing; }
    bool isComplete() const { return !playing && elapsed >= duration; }
    void setLoop(bool l) { loop = l; }
    void stop() { playing = false; }
    void reset() { elapsed = 0.0f; currentValue = startValue; }
    float getProgress() const { return elapsed / duration; }
    
private:
    T startValue;
    T endValue;
    T currentValue;
    float duration;
    float elapsed;
    EaseFunc easeFunc;
    bool playing;
    bool loop;
};

// ============================================================================
// Animation Sequence - Chain multiple animations
// ============================================================================
class AnimationSequence {
public:
    struct Step {
        std::function<void()> onStart;
        std::function<void(float)> onUpdate;
        std::function<void()> onComplete;
        float duration;
    };
    
    AnimationSequence() : currentStep(0), stepTime(0), playing(false) {}
    
    void addStep(float duration, 
                 std::function<void()> onStart = nullptr,
                 std::function<void(float)> onUpdate = nullptr,
                 std::function<void()> onComplete = nullptr) {
        steps.push_back({onStart, onUpdate, onComplete, duration});
    }
    
    void play() {
        if (steps.empty()) return;
        currentStep = 0;
        stepTime = 0;
        playing = true;
        if (steps[0].onStart) steps[0].onStart();
    }
    
    void update(float deltaTime) {
        if (!playing || currentStep >= steps.size()) return;
        
        stepTime += deltaTime;
        Step& step = steps[currentStep];
        
        float progress = step.duration > 0 ? stepTime / step.duration : 1.0f;
        if (step.onUpdate) step.onUpdate(fminf(progress, 1.0f));
        
        if (stepTime >= step.duration) {
            if (step.onComplete) step.onComplete();
            currentStep++;
            stepTime = 0;
            
            if (currentStep < steps.size()) {
                if (steps[currentStep].onStart) steps[currentStep].onStart();
            } else {
                playing = false;
            }
        }
    }
    
    bool isPlaying() const { return playing; }
    void stop() { playing = false; }
    void clear() { steps.clear(); currentStep = 0; stepTime = 0; playing = false; }
    
private:
    std::vector<Step> steps;
    size_t currentStep;
    float stepTime;
    bool playing;
};

// ============================================================================
// Spring Physics - For juicy, responsive animations
// ============================================================================
class Spring {
public:
    float current;  // Public for direct access
    float target;
    float velocity;
    float stiffness;
    float damping;
    
    Spring(float stiff = 150.0f, float damp = 10.0f)
        : current(0), target(0), velocity(0)
        , stiffness(stiff), damping(damp) {}
    
    void setTarget(float t) { target = t; }
    void setValue(float v) { current = v; velocity = 0; }
    void impulse(float force) { velocity += force; }
    
    void update(float deltaTime) {
        float springForce = (target - current) * stiffness;
        float dampingForce = velocity * damping;
        float acceleration = springForce - dampingForce;
        
        velocity += acceleration * deltaTime;
        current += velocity * deltaTime;
    }
    
    float getValue() const { return current; }
    bool isSettled(float threshold = 0.001f) const {
        return fabsf(current - target) < threshold && fabsf(velocity) < threshold;
    }
};

class Spring3D {
public:
    Math::Vector3 current;  // Public for direct access
    Math::Vector3 target;
    Math::Vector3 velocity;
    float stiffness;
    float damping;
    
    Spring3D(float stiff = 150.0f, float damp = 10.0f)
        : stiffness(stiff), damping(damp) {}
    
    void setTarget(const Math::Vector3& t) { target = t; }
    void setValue(const Math::Vector3& v) { current = v; velocity = Math::Vector3(); }
    void impulse(const Math::Vector3& force) { velocity = velocity + force; }
    
    void update(float deltaTime) {
        Math::Vector3 springForce = (target - current) * stiffness;
        Math::Vector3 dampingForce = velocity * damping;
        Math::Vector3 acceleration = springForce - dampingForce;
        
        velocity = velocity + acceleration * deltaTime;
        current = current + velocity * deltaTime;
    }
    
    const Math::Vector3& getValue() const { return current; }
};

// ============================================================================
// Timer utilities
// ============================================================================
class Timer {
public:
    Timer(float duration = 1.0f) : duration(duration), elapsed(0), running(false) {}
    
    void start(float dur = -1.0f) {
        if (dur > 0) duration = dur;
        elapsed = 0;
        running = true;
    }
    
    void update(float deltaTime) {
        if (running) {
            elapsed += deltaTime;
            if (elapsed >= duration) {
                elapsed = duration;
                running = false;
            }
        }
    }
    
    bool isRunning() const { return running; }
    bool isComplete() const { return elapsed >= duration; }
    float getProgress() const { return duration > 0 ? elapsed / duration : 1.0f; }
    float getRemaining() const { return fmaxf(0.0f, duration - elapsed); }
    void reset() { elapsed = 0; running = false; }
    
private:
    float duration;
    float elapsed;
    bool running;
};

// ============================================================================
// Flicker/Flash effect for damage, pickups, etc.
// ============================================================================
class FlickerEffect {
public:
    FlickerEffect() : active(false), timer(0), frequency(10.0f), duration(0.5f) {}
    
    void start(float dur = 0.5f, float freq = 10.0f) {
        duration = dur;
        frequency = freq;
        timer = dur;
        active = true;
    }
    
    void update(float deltaTime) {
        if (!active) return;
        timer -= deltaTime;
        if (timer <= 0) {
            active = false;
            timer = 0;
        }
    }
    
    bool isVisible() const {
        if (!active) return true;
        return (int)(timer * frequency * 2.0f) % 2 == 0;
    }
    
    bool isActive() const { return active; }
    float getAlpha() const {
        if (!active) return 1.0f;
        return isVisible() ? 1.0f : 0.3f;
    }
    
private:
    bool active;
    float timer;
    float frequency;
    float duration;
};

// ============================================================================
// Pulse effect for UI elements
// ============================================================================
class PulseEffect {
public:
    PulseEffect(float freq = 2.0f, float minScale = 0.9f, float maxScale = 1.1f)
        : frequency(freq), minScale(minScale), maxScale(maxScale), timer(0) {}
    
    void update(float deltaTime) {
        timer += deltaTime * frequency;
    }
    
    float getScale() const {
        float t = (sinf(timer * Math::PI * 2.0f) + 1.0f) * 0.5f;
        return minScale + (maxScale - minScale) * t;
    }
    
    float getAlpha(float min = 0.7f, float max = 1.0f) const {
        float t = (sinf(timer * Math::PI * 2.0f) + 1.0f) * 0.5f;
        return min + (max - min) * t;
    }
    
private:
    float frequency;
    float minScale, maxScale;
    float timer;
};

} // namespace Anim

// Type aliases for backward compatibility (Animation:: -> Anim::)
namespace Animation = Anim;

// ============================================================================
// Free functions for lerp and clamp (available globally in Doomers namespace)
// ============================================================================
inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float clamp(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

inline int clamp(int value, int minVal, int maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

} // namespace Doomers
