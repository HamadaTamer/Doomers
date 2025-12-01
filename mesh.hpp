// Mesh.hpp
#pragma once
#include <vector>
#include <string>


struct Mesh {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;

    // bounding box
    float minX = 0, maxX = 0;
    float minY = 0, maxY = 0;
    float minZ = 0, maxZ = 0;
    bool  hasBounds = false;

    int vertexCount() const {
        return static_cast<int>(vertices.size() / 3);
    }

    void computeBounds();
    void draw(bool useTexcoords = true) const;
};


Mesh loadOBJ(const std::string& path);
