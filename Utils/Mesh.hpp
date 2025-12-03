/**
 * DOOMERS - Mesh Utilities
 * 
 * Simple OBJ mesh loading and rendering
 * For basic geometry without materials
 */

#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>

#include <glut.h>

namespace Doomers {
namespace Utils {

// ============================================================================
// Simple Mesh - vertices, normals, texcoords
// ============================================================================
struct SimpleMesh {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;

    // Bounding box
    float minX = 0, maxX = 0;
    float minY = 0, maxY = 0;
    float minZ = 0, maxZ = 0;
    bool hasBounds = false;

    int vertexCount() const {
        return static_cast<int>(vertices.size() / 3);
    }

    void computeBounds() {
        if (vertices.empty()) {
            hasBounds = false;
            return;
        }

        minX = maxX = vertices[0];
        minY = maxY = vertices[1];
        minZ = maxZ = vertices[2];

        int n = vertexCount();
        for (int i = 1; i < n; ++i) {
            float x = vertices[3 * i + 0];
            float y = vertices[3 * i + 1];
            float z = vertices[3 * i + 2];

            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
            if (z < minZ) minZ = z;
            if (z > maxZ) maxZ = z;
        }
        hasBounds = true;
    }

    void draw(bool useTexcoords = true) const {
        if (vertices.empty()) return;

        glBegin(GL_TRIANGLES);
        int nVerts = vertexCount();

        for (int i = 0; i < nVerts; ++i) {
            if (!normals.empty()) {
                glNormal3f(normals[3 * i + 0],
                    normals[3 * i + 1],
                    normals[3 * i + 2]);
            }
            if (useTexcoords && !texcoords.empty()) {
                glTexCoord2f(texcoords[2 * i + 0],
                    texcoords[2 * i + 1]);
            }
            glVertex3f(vertices[3 * i + 0],
                vertices[3 * i + 1],
                vertices[3 * i + 2]);
        }
        glEnd();
    }
    
    float getWidth() const { return maxX - minX; }
    float getHeight() const { return maxY - minY; }
    float getDepth() const { return maxZ - minZ; }
};

// ============================================================================
// Load simple OBJ file
// ============================================================================
inline SimpleMesh loadSimpleOBJ(const std::string& path) {
    SimpleMesh mesh;

    FILE* f = std::fopen(path.c_str(), "r");
    if (!f) {
        std::cerr << "[Utils] Could not open OBJ file: " << path << "\n";
        return mesh;
    }

    struct Vec3 { float x, y, z; };
    struct Vec2 { float u, v; };

    std::vector<Vec3> tempV;
    std::vector<Vec3> tempVN;
    std::vector<Vec2> tempVT;

    char line[512];

    while (std::fgets(line, sizeof(line), f)) {
        if (std::strncmp(line, "v ", 2) == 0) {
            Vec3 v;
            std::sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
            tempV.push_back(v);
        }
        else if (std::strncmp(line, "vn ", 3) == 0) {
            Vec3 n;
            std::sscanf(line + 3, "%f %f %f", &n.x, &n.y, &n.z);
            tempVN.push_back(n);
        }
        else if (std::strncmp(line, "vt ", 3) == 0) {
            Vec2 t;
            std::sscanf(line + 3, "%f %f", &t.u, &t.v);
            tempVT.push_back(t);
        }
        else if (std::strncmp(line, "f ", 2) == 0) {
            // Support multiple face formats
            int vIdx[4], vtIdx[4], vnIdx[4];
            int count = 0;
            
            // Try v/vt/vn format first
            count = std::sscanf(line + 2,
                "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                &vIdx[0], &vtIdx[0], &vnIdx[0],
                &vIdx[1], &vtIdx[1], &vnIdx[1],
                &vIdx[2], &vtIdx[2], &vnIdx[2],
                &vIdx[3], &vtIdx[3], &vnIdx[3]);

            int numVerts = count / 3;
            
            if (numVerts < 3) {
                // Try v//vn format
                count = std::sscanf(line + 2,
                    "%d//%d %d//%d %d//%d %d//%d",
                    &vIdx[0], &vnIdx[0],
                    &vIdx[1], &vnIdx[1],
                    &vIdx[2], &vnIdx[2],
                    &vIdx[3], &vnIdx[3]);
                numVerts = count / 2;
                vtIdx[0] = vtIdx[1] = vtIdx[2] = vtIdx[3] = 0;
            }
            
            if (numVerts < 3) continue;

            // Triangulate (fan triangulation for quads)
            for (int i = 0; i < numVerts - 2; ++i) {
                int indices[3] = { 0, i + 1, i + 2 };
                
                for (int k = 0; k < 3; ++k) {
                    int idx = indices[k];
                    int vi = vIdx[idx] - 1;
                    int ni = vnIdx[idx] - 1;
                    int ti = vtIdx[idx] - 1;

                    if (vi >= 0 && vi < (int)tempV.size()) {
                        const Vec3& v = tempV[vi];
                        mesh.vertices.push_back(v.x);
                        mesh.vertices.push_back(v.y);
                        mesh.vertices.push_back(v.z);
                    }

                    if (!tempVN.empty() && ni >= 0 && ni < (int)tempVN.size()) {
                        const Vec3& n = tempVN[ni];
                        mesh.normals.push_back(n.x);
                        mesh.normals.push_back(n.y);
                        mesh.normals.push_back(n.z);
                    } else {
                        mesh.normals.push_back(0.0f);
                        mesh.normals.push_back(1.0f);
                        mesh.normals.push_back(0.0f);
                    }

                    if (!tempVT.empty() && ti >= 0 && ti < (int)tempVT.size()) {
                        const Vec2& t = tempVT[ti];
                        mesh.texcoords.push_back(t.u);
                        mesh.texcoords.push_back(t.v);
                    } else {
                        mesh.texcoords.push_back(0.0f);
                        mesh.texcoords.push_back(0.0f);
                    }
                }
            }
        }
    }

    std::fclose(f);

    mesh.computeBounds();
    std::cout << "[Utils] Loaded " << path << " - "
        << mesh.vertexCount() << " vertices, Size: "
        << mesh.getWidth() << " x " << mesh.getHeight() << " x " << mesh.getDepth() << "\n";

    return mesh;
}

} // namespace Utils
} // namespace Doomers
