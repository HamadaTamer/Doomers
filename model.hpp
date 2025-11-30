#pragma once
#include <vector>
#include <string>

// Forward declaration: you already have this in main.cpp
// (stb_image + OpenGL texture creation).
// We will call it from the loader.
unsigned int loadTexture(const char* filename);

// ---------- Materials ----------

struct Material {
    std::string name;
    std::string diffuseMap;   // path to texture
    unsigned int textureId = 0;
};

// ---------- Sub-mesh (a set of triangles using one material) ----------

struct SubMesh {
    std::vector<float> vertices;   // x,y,z
    std::vector<float> normals;    // nx,ny,nz
    std::vector<float> texcoords;  // u,v
    int materialIndex = -1;        // index into Model::materials

    int vertexCount() const {
        return static_cast<int>(vertices.size() / 3);
    }

    void draw(const std::vector<Material>& materials) const;
};

// ---------- Whole model (OBJ with materials) ----------

struct Model {
    std::vector<Material> materials;
    std::vector<SubMesh>  submeshes;

    void draw() const;
};

// Loads .obj and its .mtl, given:
//   objPath: full path to Soldier.obj
//   baseDir: directory where textures & mtl live (for resolving paths)
Model loadOBJWithMTL(const std::string& objPath,
    const std::string& baseDir);
