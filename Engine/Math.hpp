/**
 * DOOMERS - Math Library
 * 
 * Complete vector, matrix, and quaternion math for 3D game development
 * Designed for OpenGL 2.x compatibility
 */

#pragma once

#include <cmath>
#include <cstring>

namespace Doomers {
namespace Math {

// Constants
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 6.28318530717958647692f;
constexpr float HALF_PI = 1.57079632679489661923f;
constexpr float DEG_TO_RAD = 0.01745329251994329577f;
constexpr float RAD_TO_DEG = 57.2957795130823208768f;
constexpr float EPSILON = 0.0001f;

// ============================================================================
// Vector2 - 2D Vector
// ============================================================================
struct Vector2 {
    float x, y;
    
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
    
    Vector2 operator+(const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
    Vector2 operator-(const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
    Vector2 operator*(float s) const { return Vector2(x * s, y * s); }
    Vector2 operator/(float s) const { return Vector2(x / s, y / s); }
    Vector2& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
    Vector2& operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
    Vector2& operator*=(float s) { x *= s; y *= s; return *this; }
    Vector2 operator-() const { return Vector2(-x, -y); }
    
    float length() const { return sqrtf(x*x + y*y); }
    float lengthSquared() const { return x*x + y*y; }
    
    Vector2 normalized() const {
        float len = length();
        if (len > EPSILON) return *this / len;
        return Vector2(0, 0);
    }
    
    void normalize() {
        float len = length();
        if (len > EPSILON) { x /= len; y /= len; }
    }
    
    static float dot(const Vector2& a, const Vector2& b) {
        return a.x * b.x + a.y * b.y;
    }
};

// ============================================================================
// Vector3 - 3D Vector (Core type for positions, directions, velocities)
// ============================================================================
struct Vector3 {
    float x, y, z;
    
    // Constructors
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(float s) : x(s), y(s), z(s) {}
    
    // Static constants
    static Vector3 zero() { return Vector3(0, 0, 0); }
    static Vector3 one() { return Vector3(1, 1, 1); }
    static Vector3 up() { return Vector3(0, 1, 0); }
    static Vector3 down() { return Vector3(0, -1, 0); }
    static Vector3 forward() { return Vector3(0, 0, -1); }
    static Vector3 back() { return Vector3(0, 0, 1); }
    static Vector3 right() { return Vector3(1, 0, 0); }
    static Vector3 left() { return Vector3(-1, 0, 0); }
    
    // Operators
    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator*(const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
    Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }
    Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
    Vector3 operator-() const { return Vector3(-x, -y, -z); }
    bool operator==(const Vector3& v) const { 
        return fabsf(x - v.x) < EPSILON && fabsf(y - v.y) < EPSILON && fabsf(z - v.z) < EPSILON; 
    }
    bool operator!=(const Vector3& v) const { return !(*this == v); }
    
    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }
    
    // Methods
    float length() const { return sqrtf(x*x + y*y + z*z); }
    float lengthSquared() const { return x*x + y*y + z*z; }
    
    Vector3 normalized() const {
        float len = length();
        if (len > EPSILON) return *this / len;
        return Vector3::zero();
    }
    
    void normalize() {
        float len = length();
        if (len > EPSILON) { x /= len; y /= len; z /= len; }
    }
    
    // Static methods
    static float dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    
    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
    
    static float distance(const Vector3& a, const Vector3& b) {
        return (b - a).length();
    }
    
    static float distanceSquared(const Vector3& a, const Vector3& b) {
        return (b - a).lengthSquared();
    }
    
    static Vector3 lerp(const Vector3& a, const Vector3& b, float t) {
        return a + (b - a) * t;
    }
    
    static Vector3 reflect(const Vector3& incident, const Vector3& normal) {
        return incident - normal * (2.0f * dot(incident, normal));
    }
    
    static Vector3 project(const Vector3& v, const Vector3& onto) {
        float d = dot(onto, onto);
        if (d > EPSILON) {
            return onto * (dot(v, onto) / d);
        }
        return Vector3::zero();
    }
    
    // Get horizontal component (XZ plane)
    Vector3 horizontal() const { return Vector3(x, 0, z); }
    float horizontalLength() const { return sqrtf(x*x + z*z); }
    
    // Member functions for dot and cross
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
};

// External operator for scalar * vector
inline Vector3 operator*(float s, const Vector3& v) { return v * s; }

// ============================================================================
// Free Functions - Math Utilities
// ============================================================================
inline float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline Vector3 lerp(const Vector3& a, const Vector3& b, float t) {
    return a + (b - a) * t;
}

inline float smoothstep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline float randomRange(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

inline Vector3 randomInSphere() {
    float u = (float)rand() / RAND_MAX;
    float v = (float)rand() / RAND_MAX;
    float theta = u * TWO_PI;
    float phi = acosf(2.0f * v - 1.0f);
    float r = cbrtf((float)rand() / RAND_MAX);
    float sinPhi = sinf(phi);
    return Vector3(
        r * sinPhi * cosf(theta),
        r * sinPhi * sinf(theta),
        r * cosf(phi)
    );
}

// ============================================================================
// Vector4 - 4D Vector (for homogeneous coordinates and colors with alpha)
// ============================================================================
struct Vector4 {
    float x, y, z, w;
    
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(const Vector3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
    
    Vector3 xyz() const { return Vector3(x, y, z); }
    
    Vector4 operator+(const Vector4& v) const { return Vector4(x + v.x, y + v.y, z + v.z, w + v.w); }
    Vector4 operator-(const Vector4& v) const { return Vector4(x - v.x, y - v.y, z - v.z, w - v.w); }
    Vector4 operator*(float s) const { return Vector4(x * s, y * s, z * s, w * s); }
    
    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }
};

// ============================================================================
// Color - RGBA Color
// ============================================================================
struct Color {
    float r, g, b, a;
    
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    
    static Color white() { return Color(1, 1, 1, 1); }
    static Color black() { return Color(0, 0, 0, 1); }
    static Color red() { return Color(1, 0, 0, 1); }
    static Color green() { return Color(0, 1, 0, 1); }
    static Color blue() { return Color(0, 0, 1, 1); }
    static Color yellow() { return Color(1, 1, 0, 1); }
    static Color cyan() { return Color(0, 1, 1, 1); }
    static Color magenta() { return Color(1, 0, 1, 1); }
    static Color orange() { return Color(1, 0.5f, 0, 1); }
    static Color gray() { return Color(0.5f, 0.5f, 0.5f, 1); }
    static Color darkGray() { return Color(0.2f, 0.2f, 0.2f, 1); }
    static Color transparent() { return Color(0, 0, 0, 0); }
    
    // Blend colors
    static Color lerp(const Color& a, const Color& b, float t) {
        return Color(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        );
    }
    
    // Apply to OpenGL
    void apply() const {
        glColor4f(r, g, b, a);
    }
    
    void applyAsMaterial() const {
        GLfloat ambient[] = { r * 0.2f, g * 0.2f, b * 0.2f, a };
        GLfloat diffuse[] = { r, g, b, a };
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    }
};

// ============================================================================
// Matrix4 - 4x4 Matrix (Column-major for OpenGL)
// ============================================================================
struct Matrix4 {
    float m[16]; // Column-major order for OpenGL
    
    Matrix4() {
        identity();
    }
    
    Matrix4(const float* data) {
        memcpy(m, data, 16 * sizeof(float));
    }
    
    void identity() {
        memset(m, 0, 16 * sizeof(float));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }
    
    // Access element at row r, column c (0-indexed)
    float& at(int r, int c) { return m[c * 4 + r]; }
    const float& at(int r, int c) const { return m[c * 4 + r]; }
    
    // Get raw pointer for OpenGL
    const float* ptr() const { return m; }
    float* ptr() { return m; }
    
    // Matrix multiplication
    Matrix4 operator*(const Matrix4& other) const {
        Matrix4 result;
        for (int c = 0; c < 4; ++c) {
            for (int r = 0; r < 4; ++r) {
                result.at(r, c) = 
                    at(r, 0) * other.at(0, c) +
                    at(r, 1) * other.at(1, c) +
                    at(r, 2) * other.at(2, c) +
                    at(r, 3) * other.at(3, c);
            }
        }
        return result;
    }
    
    // Transform a point (w=1)
    Vector3 transformPoint(const Vector3& v) const {
        float x = m[0]*v.x + m[4]*v.y + m[8]*v.z + m[12];
        float y = m[1]*v.x + m[5]*v.y + m[9]*v.z + m[13];
        float z = m[2]*v.x + m[6]*v.y + m[10]*v.z + m[14];
        float w = m[3]*v.x + m[7]*v.y + m[11]*v.z + m[15];
        if (fabsf(w) > EPSILON) {
            return Vector3(x/w, y/w, z/w);
        }
        return Vector3(x, y, z);
    }
    
    // Transform a direction (w=0)
    Vector3 transformDirection(const Vector3& v) const {
        return Vector3(
            m[0]*v.x + m[4]*v.y + m[8]*v.z,
            m[1]*v.x + m[5]*v.y + m[9]*v.z,
            m[2]*v.x + m[6]*v.y + m[10]*v.z
        );
    }
    
    // Transform a Vector4
    Vector4 transform(const Vector4& v) const {
        return Vector4(
            m[0]*v.x + m[4]*v.y + m[8]*v.z + m[12]*v.w,
            m[1]*v.x + m[5]*v.y + m[9]*v.z + m[13]*v.w,
            m[2]*v.x + m[6]*v.y + m[10]*v.z + m[14]*v.w,
            m[3]*v.x + m[7]*v.y + m[11]*v.z + m[15]*v.w
        );
    }
    
    // Static factory methods
    static Matrix4 translation(const Vector3& t) {
        Matrix4 result;
        result.m[12] = t.x;
        result.m[13] = t.y;
        result.m[14] = t.z;
        return result;
    }
    
    static Matrix4 Identity() {
        Matrix4 result;
        result.m[0] = result.m[5] = result.m[10] = result.m[15] = 1.0f;
        result.m[1] = result.m[2] = result.m[3] = 0;
        result.m[4] = result.m[6] = result.m[7] = 0;
        result.m[8] = result.m[9] = result.m[11] = 0;
        result.m[12] = result.m[13] = result.m[14] = 0;
        return result;
    }
    
    static Matrix4 translation(float x, float y, float z) {
        return translation(Vector3(x, y, z));
    }
    
    static Matrix4 scale(const Vector3& s) {
        Matrix4 result;
        result.m[0] = s.x;
        result.m[5] = s.y;
        result.m[10] = s.z;
        return result;
    }
    
    static Matrix4 scale(float s) {
        return scale(Vector3(s, s, s));
    }
    
    static Matrix4 scale(float x, float y, float z) {
        return scale(Vector3(x, y, z));
    }
    
    static Matrix4 rotationX(float angleRad) {
        Matrix4 result;
        float c = cosf(angleRad);
        float s = sinf(angleRad);
        result.m[5] = c;
        result.m[6] = s;
        result.m[9] = -s;
        result.m[10] = c;
        return result;
    }
    
    static Matrix4 rotationY(float angleRad) {
        Matrix4 result;
        float c = cosf(angleRad);
        float s = sinf(angleRad);
        result.m[0] = c;
        result.m[2] = -s;
        result.m[8] = s;
        result.m[10] = c;
        return result;
    }
    
    static Matrix4 rotationZ(float angleRad) {
        Matrix4 result;
        float c = cosf(angleRad);
        float s = sinf(angleRad);
        result.m[0] = c;
        result.m[1] = s;
        result.m[4] = -s;
        result.m[5] = c;
        return result;
    }
    
    static Matrix4 rotation(float angleRad, const Vector3& axis) {
        Vector3 a = axis.normalized();
        float c = cosf(angleRad);
        float s = sinf(angleRad);
        float t = 1.0f - c;
        
        Matrix4 result;
        result.m[0] = t*a.x*a.x + c;
        result.m[1] = t*a.x*a.y + s*a.z;
        result.m[2] = t*a.x*a.z - s*a.y;
        
        result.m[4] = t*a.x*a.y - s*a.z;
        result.m[5] = t*a.y*a.y + c;
        result.m[6] = t*a.y*a.z + s*a.x;
        
        result.m[8] = t*a.x*a.z + s*a.y;
        result.m[9] = t*a.y*a.z - s*a.x;
        result.m[10] = t*a.z*a.z + c;
        
        return result;
    }
    
    // View matrix (camera)
    static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        Vector3 f = (target - eye).normalized();
        Vector3 r = Vector3::cross(f, up).normalized();
        Vector3 u = Vector3::cross(r, f);
        
        Matrix4 result;
        result.m[0] = r.x;
        result.m[4] = r.y;
        result.m[8] = r.z;
        
        result.m[1] = u.x;
        result.m[5] = u.y;
        result.m[9] = u.z;
        
        result.m[2] = -f.x;
        result.m[6] = -f.y;
        result.m[10] = -f.z;
        
        result.m[12] = -Vector3::dot(r, eye);
        result.m[13] = -Vector3::dot(u, eye);
        result.m[14] = Vector3::dot(f, eye);
        
        return result;
    }
    
    // Perspective projection matrix
    static Matrix4 perspective(float fovYRad, float aspect, float nearZ, float farZ) {
        float tanHalfFov = tanf(fovYRad / 2.0f);
        
        Matrix4 result;
        memset(result.m, 0, 16 * sizeof(float));
        
        result.m[0] = 1.0f / (aspect * tanHalfFov);
        result.m[5] = 1.0f / tanHalfFov;
        result.m[10] = -(farZ + nearZ) / (farZ - nearZ);
        result.m[11] = -1.0f;
        result.m[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
        
        return result;
    }
    
    // Orthographic projection matrix
    static Matrix4 ortho(float left, float right, float bottom, float top, float nearZ, float farZ) {
        Matrix4 result;
        result.m[0] = 2.0f / (right - left);
        result.m[5] = 2.0f / (top - bottom);
        result.m[10] = -2.0f / (farZ - nearZ);
        result.m[12] = -(right + left) / (right - left);
        result.m[13] = -(top + bottom) / (top - bottom);
        result.m[14] = -(farZ + nearZ) / (farZ - nearZ);
        return result;
    }
    
    // Apply to OpenGL (legacy)
    void applyToGL() const {
        glMultMatrixf(m);
    }
    
    void loadToGL() const {
        glLoadMatrixf(m);
    }
    
    // Instance methods for setting translation/scale (for compatibility)
    void setTranslation(const Vector3& t) {
        m[12] = t.x;
        m[13] = t.y;
        m[14] = t.z;
    }
    
    void setScale(const Vector3& s) {
        m[0] = s.x;
        m[5] = s.y;
        m[10] = s.z;
    }
    
    Vector3 getTranslation() const {
        return Vector3(m[12], m[13], m[14]);
    }
    
    Vector3 getScale() const {
        return Vector3(m[0], m[5], m[10]);
    }
    
    // Simple invert (for orthogonal matrices - rotation/translation only)
    Matrix4 inverted() const {
        Matrix4 result;
        // Transpose the 3x3 rotation part
        result.m[0] = m[0];  result.m[4] = m[1];  result.m[8] = m[2];
        result.m[1] = m[4];  result.m[5] = m[5];  result.m[9] = m[6];
        result.m[2] = m[8];  result.m[6] = m[9];  result.m[10] = m[10];
        // Negate translation
        result.m[12] = -(m[0]*m[12] + m[1]*m[13] + m[2]*m[14]);
        result.m[13] = -(m[4]*m[12] + m[5]*m[13] + m[6]*m[14]);
        result.m[14] = -(m[8]*m[12] + m[9]*m[13] + m[10]*m[14]);
        result.m[3] = result.m[7] = result.m[11] = 0;
        result.m[15] = 1;
        return result;
    }
    
    void invert() {
        *this = inverted();
    }
    
    // Get raw data pointer (alias for m)
    const float* data() const { return m; }
    float* data() { return m; }
};

// ============================================================================
// Quaternion - For smooth rotations and animations
// ============================================================================
struct Quaternion {
    float x, y, z, w;
    
    Quaternion() : x(0), y(0), z(0), w(1) {}
    Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    
    static Quaternion identity() { return Quaternion(0, 0, 0, 1); }
    
    static Quaternion fromAxisAngle(const Vector3& axis, float angleRad) {
        float halfAngle = angleRad * 0.5f;
        float s = sinf(halfAngle);
        Vector3 a = axis.normalized();
        return Quaternion(a.x * s, a.y * s, a.z * s, cosf(halfAngle));
    }
    
    static Quaternion fromEuler(float pitch, float yaw, float roll) {
        float cy = cosf(yaw * 0.5f);
        float sy = sinf(yaw * 0.5f);
        float cp = cosf(pitch * 0.5f);
        float sp = sinf(pitch * 0.5f);
        float cr = cosf(roll * 0.5f);
        float sr = sinf(roll * 0.5f);
        
        return Quaternion(
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        );
    }
    
    Quaternion operator*(const Quaternion& q) const {
        return Quaternion(
            w*q.x + x*q.w + y*q.z - z*q.y,
            w*q.y - x*q.z + y*q.w + z*q.x,
            w*q.z + x*q.y - y*q.x + z*q.w,
            w*q.w - x*q.x - y*q.y - z*q.z
        );
    }
    
    float length() const {
        return sqrtf(x*x + y*y + z*z + w*w);
    }
    
    Quaternion normalized() const {
        float len = length();
        if (len > EPSILON) {
            return Quaternion(x/len, y/len, z/len, w/len);
        }
        return identity();
    }
    
    Quaternion conjugate() const {
        return Quaternion(-x, -y, -z, w);
    }
    
    Vector3 rotate(const Vector3& v) const {
        Quaternion p(v.x, v.y, v.z, 0);
        Quaternion result = (*this) * p * conjugate();
        return Vector3(result.x, result.y, result.z);
    }
    
    Matrix4 toMatrix() const {
        Matrix4 result;
        float xx = x * x;
        float xy = x * y;
        float xz = x * z;
        float xw = x * w;
        float yy = y * y;
        float yz = y * z;
        float yw = y * w;
        float zz = z * z;
        float zw = z * w;
        
        result.m[0] = 1 - 2*(yy + zz);
        result.m[1] = 2*(xy + zw);
        result.m[2] = 2*(xz - yw);
        
        result.m[4] = 2*(xy - zw);
        result.m[5] = 1 - 2*(xx + zz);
        result.m[6] = 2*(yz + xw);
        
        result.m[8] = 2*(xz + yw);
        result.m[9] = 2*(yz - xw);
        result.m[10] = 1 - 2*(xx + yy);
        
        return result;
    }
    
    static Quaternion slerp(const Quaternion& a, const Quaternion& b, float t) {
        float dot = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
        
        Quaternion b2 = b;
        if (dot < 0) {
            b2 = Quaternion(-b.x, -b.y, -b.z, -b.w);
            dot = -dot;
        }
        
        if (dot > 0.9995f) {
            return Quaternion(
                a.x + t*(b2.x - a.x),
                a.y + t*(b2.y - a.y),
                a.z + t*(b2.z - a.z),
                a.w + t*(b2.w - a.w)
            ).normalized();
        }
        
        float theta0 = acosf(dot);
        float theta = theta0 * t;
        float sinTheta = sinf(theta);
        float sinTheta0 = sinf(theta0);
        
        float s0 = cosf(theta) - dot * sinTheta / sinTheta0;
        float s1 = sinTheta / sinTheta0;
        
        return Quaternion(
            s0*a.x + s1*b2.x,
            s0*a.y + s1*b2.y,
            s0*a.z + s1*b2.z,
            s0*a.w + s1*b2.w
        );
    }
};

// ============================================================================
// Transform - Position, Rotation, Scale combined
// ============================================================================
struct Transform {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    
    Transform() : position(Vector3::zero()), rotation(Quaternion::identity()), scale(Vector3::one()) {}
    
    Matrix4 getMatrix() const {
        Matrix4 t = Matrix4::translation(position);
        Matrix4 r = rotation.toMatrix();
        Matrix4 s = Matrix4::scale(scale);
        return t * r * s;
    }
    
    Vector3 forward() const {
        return rotation.rotate(Vector3::forward());
    }
    
    Vector3 right() const {
        return rotation.rotate(Vector3::right());
    }
    
    Vector3 up() const {
        return rotation.rotate(Vector3::up());
    }
    
    void setRotationEuler(float pitch, float yaw, float roll) {
        rotation = Quaternion::fromEuler(pitch * DEG_TO_RAD, yaw * DEG_TO_RAD, roll * DEG_TO_RAD);
    }
    
    // Apply to OpenGL (legacy pipeline)
    void applyToGL() const {
        glTranslatef(position.x, position.y, position.z);
        Matrix4 r = rotation.toMatrix();
        glMultMatrixf(r.ptr());
        glScalef(scale.x, scale.y, scale.z);
    }
};

// ============================================================================
// Ray - For raycasting
// ============================================================================
struct Ray {
    Vector3 origin;
    Vector3 direction;
    
    Ray() : origin(Vector3::zero()), direction(Vector3::forward()) {}
    Ray(const Vector3& origin, const Vector3& direction) 
        : origin(origin), direction(direction.normalized()) {}
    
    Vector3 getPoint(float distance) const {
        return origin + direction * distance;
    }
};

// ============================================================================
// AABB - Axis-Aligned Bounding Box
// ============================================================================
struct AABB {
    Vector3 min;
    Vector3 max;
    
    AABB() : min(Vector3::zero()), max(Vector3::zero()) {}
    AABB(const Vector3& min, const Vector3& max) : min(min), max(max) {}
    
    static AABB fromCenterSize(const Vector3& center, const Vector3& size) {
        Vector3 halfSize = size * 0.5f;
        return AABB(center - halfSize, center + halfSize);
    }
    
    Vector3 center() const { return (min + max) * 0.5f; }
    Vector3 size() const { return max - min; }
    Vector3 extents() const { return (max - min) * 0.5f; }
    
    bool contains(const Vector3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
    
    bool intersects(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }
    
    // Ray-AABB intersection
    bool intersectsRay(const Ray& ray, float& tMin, float& tMax) const {
        tMin = 0.0f;
        tMax = 1e30f;
        
        for (int i = 0; i < 3; ++i) {
            if (fabsf(ray.direction[i]) < EPSILON) {
                if (ray.origin[i] < min[i] || ray.origin[i] > max[i])
                    return false;
            } else {
                float invD = 1.0f / ray.direction[i];
                float t1 = (min[i] - ray.origin[i]) * invD;
                float t2 = (max[i] - ray.origin[i]) * invD;
                
                if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
                
                if (t1 > tMin) tMin = t1;
                if (t2 < tMax) tMax = t2;
                
                if (tMin > tMax) return false;
            }
        }
        return true;
    }
    
    void expand(const Vector3& point) {
        if (point.x < min.x) min.x = point.x;
        if (point.y < min.y) min.y = point.y;
        if (point.z < min.z) min.z = point.z;
        if (point.x > max.x) max.x = point.x;
        if (point.y > max.y) max.y = point.y;
        if (point.z > max.z) max.z = point.z;
    }
    
    void expand(const AABB& other) {
        expand(other.min);
        expand(other.max);
    }
};

// ============================================================================
// Sphere - Bounding sphere
// ============================================================================
struct Sphere {
    Vector3 center;
    float radius;
    
    Sphere() : center(Vector3::zero()), radius(0) {}
    Sphere(const Vector3& center, float radius) : center(center), radius(radius) {}
    
    bool contains(const Vector3& point) const {
        return Vector3::distanceSquared(center, point) <= radius * radius;
    }
    
    bool intersects(const Sphere& other) const {
        float combinedRadius = radius + other.radius;
        return Vector3::distanceSquared(center, other.center) <= combinedRadius * combinedRadius;
    }
    
    bool intersectsRay(const Ray& ray, float& t) const {
        Vector3 oc = ray.origin - center;
        float a = Vector3::dot(ray.direction, ray.direction);
        float b = 2.0f * Vector3::dot(oc, ray.direction);
        float c = Vector3::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) return false;
        
        t = (-b - sqrtf(discriminant)) / (2.0f * a);
        if (t < 0) {
            t = (-b + sqrtf(discriminant)) / (2.0f * a);
        }
        return t >= 0;
    }
};

// ============================================================================
// Plane - Geometric plane
// ============================================================================
struct Plane {
    Vector3 normal;
    float distance; // distance from origin along normal
    
    Plane() : normal(Vector3::up()), distance(0) {}
    Plane(const Vector3& normal, float distance) : normal(normal.normalized()), distance(distance) {}
    Plane(const Vector3& normal, const Vector3& point) : normal(normal.normalized()) {
        distance = Vector3::dot(this->normal, point);
    }
    Plane(const Vector3& p1, const Vector3& p2, const Vector3& p3) {
        normal = Vector3::cross(p2 - p1, p3 - p1).normalized();
        distance = Vector3::dot(normal, p1);
    }
    
    float signedDistance(const Vector3& point) const {
        return Vector3::dot(normal, point) - distance;
    }
    
    bool intersectsRay(const Ray& ray, float& t) const {
        float denom = Vector3::dot(normal, ray.direction);
        if (fabsf(denom) < EPSILON) return false;
        
        t = (distance - Vector3::dot(normal, ray.origin)) / denom;
        return t >= 0;
    }
};

} // namespace Math

// Aliases for convenience
using Vec2 = Math::Vector2;
using Vec3 = Math::Vector3;
using Vec4 = Math::Vector4;
using Mat4 = Math::Matrix4;
using Quat = Math::Quaternion;

} // namespace Doomers
