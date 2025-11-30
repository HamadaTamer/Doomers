#include "Model.hpp"

#include <glut.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

struct Vec3 { float x, y, z; };
struct Vec2 { float u, v; };

// ---------- Helpers ----------

static std::string joinPath(const std::string& base,
    const std::string& file) {
    if (file.empty()) return "";
    if (!base.empty() && (base.back() == '/' || base.back() == '\\'))
        return base + file;
    return base + "/" + file;
}

// find material index by name; returns -1 if not found
static int findMaterialIndex(const std::vector<Material>& mats,
    const std::string& name) {
    for (size_t i = 0; i < mats.size(); ++i) {
        if (mats[i].name == name) return static_cast<int>(i);
    }
    return -1;
}

// find or create a SubMesh index for a given material index
static int findOrCreateSubMesh(std::vector<SubMesh>& subs,
    int materialIndex) {
    for (size_t i = 0; i < subs.size(); ++i) {
        if (subs[i].materialIndex == materialIndex) {
            return static_cast<int>(i);
        }
    }
    // not found, create
    SubMesh s;
    s.materialIndex = materialIndex;
    subs.push_back(s);
    return static_cast<int>(subs.size() - 1);
}

// ---------- .mtl loader ----------

static void loadMTL(const std::string& mtlPath,
    const std::string& baseDir,
    std::vector<Material>& materials) {
    std::ifstream in(mtlPath);
    if (!in) {
        std::cerr << "Could not open MTL file: " << mtlPath << "\n";
        return;
    }

    std::string line;
    Material* current = nullptr;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "newmtl") {
            std::string name;
            iss >> name;

            // find existing material with this name (created by usemtl)
            int idx = findMaterialIndex(materials, name);
            if (idx == -1) {
                // not found, create new
                materials.push_back(Material{});
                idx = (int)materials.size() - 1;
            }
            current = &materials[idx];
            current->name = name;
        }
        else if (cmd == "map_Kd") {
            if (!current) continue;

            // read the rest of the line as the filename (can contain spaces)
            std::string rest;
            std::getline(iss, rest);

            // trim leading/trailing spaces
            size_t start = rest.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) continue;
            size_t end = rest.find_last_not_of(" \t\r\n");
            std::string texFile = rest.substr(start, end - start + 1);

            // handle optional quotes: "Army man_Body_diffuse.png"
            if (!texFile.empty() && texFile.front() == '"' && texFile.back() == '"') {
                texFile = texFile.substr(1, texFile.size() - 2);
            }

            current->diffuseMap = joinPath(baseDir, texFile);
            current->textureId = loadTexture(current->diffuseMap.c_str());

            std::cout << "Material " << current->name
                << " map_Kd -> " << current->diffuseMap
                << " texId=" << current->textureId << "\n";
        }

    }
    std::cout << "Loading MTL: " << mtlPath << "\n";

}



// ---------- OBJ + MTL loader ----------

Model loadOBJWithMTL(const std::string& objPath,
    const std::string& baseDir) {
    Model model;

    std::ifstream in(objPath);
    if (!in) {
        std::cerr << "Could not open OBJ file: " << objPath << "\n";
        return model;
    }

    std::string line;
    std::string mtlFileName;

    std::vector<Vec3> tempV;
    std::vector<Vec3> tempVN;
    std::vector<Vec2> tempVT;

    int currentMaterial = -1;
    int currentSubMesh = -1;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "mtllib") {
            // read the rest of the line as one string (can contain spaces)
            std::string rest;
            std::getline(iss, rest);

            // trim leading spaces
            size_t start = rest.find_first_not_of(" \t\r\n");
            if (start != std::string::npos)
                rest = rest.substr(start);

            mtlFileName = rest;  // e.g. "Army man.mtl"
        }
        else if (cmd == "usemtl") {
            std::string matName;
            iss >> matName;

            // make sure the material exists in model.materials
            int matIndex = findMaterialIndex(model.materials, matName);
            if (matIndex == -1) {
                // create placeholder; will be filled when we parse MTL
                Material m;
                m.name = matName;
                model.materials.push_back(m);
                matIndex = static_cast<int>(model.materials.size() - 1);
            }

            currentMaterial = matIndex;
            currentSubMesh = findOrCreateSubMesh(model.submeshes,
                currentMaterial);
        }
        else if (cmd == "v") {
            Vec3 v;
            iss >> v.x >> v.y >> v.z;
            tempV.push_back(v);
        }
        else if (cmd == "vn") {
            Vec3 n;
            iss >> n.x >> n.y >> n.z;
            tempVN.push_back(n);
        }
        else if (cmd == "vt") {
            Vec2 t;
            iss >> t.u >> t.v;
            tempVT.push_back(t);
        }
        else if (cmd == "f") {
            if (currentSubMesh == -1) {
                // no material yet, create one "default"
                currentMaterial = -1;
                currentSubMesh = findOrCreateSubMesh(model.submeshes,
                    currentMaterial);
            }

            SubMesh& s = model.submeshes[currentSubMesh];

            // we support: v/vt/vn, v/vt, v//vn, v
            std::vector<std::string> tokens;
            std::string tok;
            while (iss >> tok) tokens.push_back(tok);
            if (tokens.size() < 3) continue;

            auto parseVertex = [&](const std::string& str,
                int& outV, int& outVT, int& outVN) {
                    outV = outVT = outVN = 0;

                    if (str.find("//") != std::string::npos) {
                        std::sscanf(str.c_str(), "%d//%d", &outV, &outVN);
                    }
                    else {
                        int slashCount =
                            static_cast<int>(std::count(str.begin(),
                                str.end(), '/'));
                        if (slashCount == 0) {
                            std::sscanf(str.c_str(), "%d", &outV);
                        }
                        else if (slashCount == 1) {
                            std::sscanf(str.c_str(), "%d/%d", &outV, &outVT);
                        }
                        else {
                            std::sscanf(str.c_str(), "%d/%d/%d",
                                &outV, &outVT, &outVN);
                        }
                    }
                };

            // triangulate as fan: (0, i, i+1)
            for (size_t i = 1; i + 1 < tokens.size(); ++i) {
                int idx[3] = { 0, (int)i, (int)i + 1 };

                for (int k = 0; k < 3; ++k) {
                    int vIdx, vtIdx, vnIdx;
                    parseVertex(tokens[idx[k]], vIdx, vtIdx, vnIdx);

                    int vi = vIdx - 1;
                    int ti = vtIdx - 1;
                    int ni = vnIdx - 1;

                    // position
                    const Vec3& v = tempV[vi];
                    s.vertices.push_back(v.x);
                    s.vertices.push_back(v.y);
                    s.vertices.push_back(v.z);

                    // normal
                    if (!tempVN.empty() && ni >= 0 &&
                        ni < (int)tempVN.size()) {
                        const Vec3& n = tempVN[ni];
                        s.normals.push_back(n.x);
                        s.normals.push_back(n.y);
                        s.normals.push_back(n.z);
                    }
                    else {
                        s.normals.push_back(0.0f);
                        s.normals.push_back(1.0f);
                        s.normals.push_back(0.0f);
                    }

                    // texcoord
                    if (!tempVT.empty() && ti >= 0 &&
                        ti < (int)tempVT.size()) {
                        const Vec2& t = tempVT[ti];
                        s.texcoords.push_back(t.u);
                        s.texcoords.push_back(t.v);
                    }
                    else {
                        s.texcoords.push_back(0.0f);
                        s.texcoords.push_back(0.0f);
                    }
                }
            }
        }
    }

    // now load the .mtl (if present) and hook textures
    if (!mtlFileName.empty()) {
        std::string mtlPath = joinPath(baseDir, mtlFileName);
        loadMTL(mtlPath, baseDir, model.materials);
    }


    std::cout << "Loaded model from " << objPath
        << " with " << model.submeshes.size()
        << " submeshes and " << model.materials.size()
        << " materials.\n";

    return model;
}

// ---------- draw ----------

void SubMesh::draw(const std::vector<Material>& materials) const {
    int matIndex = materialIndex;
    unsigned int texId = 0;

    if (matIndex >= 0 && matIndex < (int)materials.size()) {
        texId = materials[matIndex].textureId;
    }

    if (texId) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texId);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.7f, 0.7f, 0.7f);
    }

    glBegin(GL_TRIANGLES);
    int nVerts = vertexCount();
    for (int i = 0; i < nVerts; ++i) {
        if (!normals.empty()) {
            glNormal3f(normals[3 * i + 0],
                normals[3 * i + 1],
                normals[3 * i + 2]);
        }
        if (!texcoords.empty()) {
            glTexCoord2f(texcoords[2 * i + 0],
                texcoords[2 * i + 1]);
        }
        glVertex3f(vertices[3 * i + 0],
            vertices[3 * i + 1],
            vertices[3 * i + 2]);
    }
    glEnd();

    if (texId)
        glDisable(GL_TEXTURE_2D);
}

void Model::draw() const {
    for (const auto& s : submeshes) {
        s.draw(materials);
    }
}
