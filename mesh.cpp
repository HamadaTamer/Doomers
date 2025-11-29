// Mesh.cpp
#include "Mesh.hpp"

// include glut first
#include <glut.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

struct Vec3 { float x, y, z; };
struct Vec2 { float u, v; };

Mesh loadOBJ(const std::string& path) {
    Mesh mesh;

    FILE* f = std::fopen(path.c_str(), "r");
    if (!f) {
        std::cerr << "Could not open OBJ file: " << path << "\n";
        return mesh;
    }

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
            // format: v/vt/vn v/vt/vn v/vt/vn
            int vIdx[3], vtIdx[3], vnIdx[3];

            int count = std::sscanf(
                line + 2,
                "%d/%d/%d %d/%d/%d %d/%d/%d",
                &vIdx[0], &vtIdx[0], &vnIdx[0],
                &vIdx[1], &vtIdx[1], &vnIdx[1],
                &vIdx[2], &vtIdx[2], &vnIdx[2]
            );

            if (count < 9) {
                // for faces that do not match this format we skip for now
                continue;
            }

            for (int i = 0; i < 3; ++i) {
                int vi = vIdx[i] - 1;   // OBJ is 1-based
                int ni = vnIdx[i] - 1;
                int ti = vtIdx[i] - 1;

                const Vec3& v = tempV[vi];
                mesh.vertices.push_back(v.x);
                mesh.vertices.push_back(v.y);
                mesh.vertices.push_back(v.z);

                // normals
                if (!tempVN.empty() && ni >= 0 && ni < (int)tempVN.size()) {
                    const Vec3& n = tempVN[ni];
                    mesh.normals.push_back(n.x);
                    mesh.normals.push_back(n.y);
                    mesh.normals.push_back(n.z);
                }
                else {
                    mesh.normals.push_back(0.0f);
                    mesh.normals.push_back(1.0f);
                    mesh.normals.push_back(0.0f);
                }

                // texcoords
                if (!tempVT.empty() && ti >= 0 && ti < (int)tempVT.size()) {
                    const Vec2& t = tempVT[ti];
                    mesh.texcoords.push_back(t.u);
                    mesh.texcoords.push_back(t.v);
                }
                else {
                    mesh.texcoords.push_back(0.0f);
                    mesh.texcoords.push_back(0.0f);
                }
            }
        }
    }

    std::fclose(f);

    std::cout << "Loaded " << path << " with "
        << mesh.vertexCount() << " vertices\n";

    return mesh;
}

void Mesh::draw(bool useTexcoords) const {
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
