// ============================================================================
// DOOMERS - Vector3.h
// 3D Vector mathematics for game physics and transformations
// ============================================================================
#ifndef VECTOR3_H
#define VECTOR3_H

#include <math.h>

class Vector3 {
public:
    float x, y, z;

    // Constructors
    Vector3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) 
        : x(_x), y(_y), z(_z) {}

    // Basic operations
    Vector3 operator+(const Vector3& v) const {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }

    Vector3 operator-(const Vector3& v) const {
        return Vector3(x - v.x, y - v.y, z - v.z);
    }

    Vector3 operator*(float n) const {
        return Vector3(x * n, y * n, z * n);
    }

    Vector3 operator/(float n) const {
        if (n != 0) return Vector3(x / n, y / n, z / n);
        return Vector3();
    }

    // Compound assignment
    Vector3& operator+=(const Vector3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }

    Vector3& operator*=(float n) {
        x *= n; y *= n; z *= n;
        return *this;
    }

    // Negation
    Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }

    // Comparison
    bool operator==(const Vector3& v) const {
        return (x == v.x && y == v.y && z == v.z);
    }

    // Vector operations
    float length() const {
        return sqrt(x * x + y * y + z * z);
    }

    float lengthSquared() const {
        return x * x + y * y + z * z;
    }

    Vector3 unit() const {
        float len = length();
        if (len > 0.0001f) return *this / len;
        return Vector3();
    }

    Vector3 normalize() const {
        return unit();
    }

    Vector3 normalized() const {
        return unit();
    }

    float dot(const Vector3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    Vector3 cross(const Vector3& v) const {
        return Vector3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    // Distance
    float distanceTo(const Vector3& v) const {
        return (*this - v).length();
    }

    float distanceSquaredTo(const Vector3& v) const {
        return (*this - v).lengthSquared();
    }

    // Interpolation
    Vector3 lerp(const Vector3& v, float t) const {
        return *this + (v - *this) * t;
    }

    // Reflection
    Vector3 reflect(const Vector3& normal) const {
        return *this - normal * 2.0f * this->dot(normal);
    }

    // Rotation around Y axis (common for FPS games)
    Vector3 rotateY(float angle) const {
        float rad = angle * 0.0174532925f;
        float cosA = cos(rad);
        float sinA = sin(rad);
        return Vector3(
            x * cosA + z * sinA,
            y,
            -x * sinA + z * cosA
        );
    }

    // Rotation around X axis
    Vector3 rotateX(float angle) const {
        float rad = angle * 0.0174532925f;
        float cosA = cos(rad);
        float sinA = sin(rad);
        return Vector3(
            x,
            y * cosA - z * sinA,
            y * sinA + z * cosA
        );
    }

    // Set values
    void set(float _x, float _y, float _z) {
        x = _x; y = _y; z = _z;
    }

    // Zero check
    bool isZero() const {
        return (x == 0 && y == 0 && z == 0);
    }

    // Static helpers
    static Vector3 zero() { return Vector3(0, 0, 0); }
    static Vector3 one() { return Vector3(1, 1, 1); }
    static Vector3 up() { return Vector3(0, 1, 0); }
    static Vector3 down() { return Vector3(0, -1, 0); }
    static Vector3 forward() { return Vector3(0, 0, -1); }
    static Vector3 back() { return Vector3(0, 0, 1); }
    static Vector3 left() { return Vector3(-1, 0, 0); }
    static Vector3 right() { return Vector3(1, 0, 0); }
};

// Scalar multiplication from left side
inline Vector3 operator*(float n, const Vector3& v) {
    return v * n;
}

#endif // VECTOR3_H
