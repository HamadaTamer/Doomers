// Mesh.hpp
#pragma once
#include <vector>
#include <string>

struct Mesh {
    std::vector<float> vertices;   // x,y,z
    std::vector<float> normals;    // nx,ny,nz
    std::vector<float> texcoords;  // u,v

    int vertexCount() const {
        return static_cast<int>(vertices.size() / 3);
    }

    void draw(bool useTexcoords = true) const;
};

Mesh loadOBJ(const std::string& path);
