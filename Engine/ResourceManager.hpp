/**
 * DOOMERS - Resource Manager
 * 
 * Handles loading and caching of all game resources:
 * - OBJ/MTL models with materials
 * - Textures (PNG, TGA, JPG via stb_image)
 * - Future: Audio files, animation data
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"

// stb_image for texture loading (implementation in main.cpp)
#include "../stb_image.h"

namespace Doomers {

// ============================================================================
// Material - Surface properties for rendering
// ============================================================================
struct Material {
    std::string name;
    
    // Colors
    Math::Color ambient;
    Math::Color diffuse;
    Math::Color specular;
    Math::Color emission;
    float shininess;
    float alpha;
    
    // Textures
    std::string diffuseMapPath;
    std::string normalMapPath;
    std::string specularMapPath;
    unsigned int diffuseTextureId;
    unsigned int normalTextureId;
    unsigned int specularTextureId;
    
    Material() 
        : ambient(0.2f, 0.2f, 0.2f)
        , diffuse(0.8f, 0.8f, 0.8f)
        , specular(0.0f, 0.0f, 0.0f)
        , emission(0.0f, 0.0f, 0.0f, 0.0f)
        , shininess(32.0f)
        , alpha(1.0f)
        , diffuseTextureId(0)
        , normalTextureId(0)
        , specularTextureId(0)
    {}
    
    void apply() const {
        GLfloat amb[] = { ambient.r, ambient.g, ambient.b, alpha };
        GLfloat diff[] = { diffuse.r, diffuse.g, diffuse.b, alpha };
        GLfloat spec[] = { specular.r, specular.g, specular.b, alpha };
        GLfloat emit[] = { emission.r, emission.g, emission.b, alpha };
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emit);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        
        if (diffuseTextureId > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, diffuseTextureId);
        } else {
            glDisable(GL_TEXTURE_2D);
        }
    }
};

// ============================================================================
// Vertex - Complete vertex data
// ============================================================================
struct Vertex {
    Math::Vector3 position;
    Math::Vector3 normal;
    Math::Vector2 texCoord;
    
    Vertex() {}
    Vertex(const Math::Vector3& pos, const Math::Vector3& norm, const Math::Vector2& uv)
        : position(pos), normal(norm), texCoord(uv) {}
};

// ============================================================================
// SubMesh - Part of a mesh using one material
// ============================================================================
struct SubMesh {
    std::vector<Vertex> vertices;
    int materialIndex;
    
    SubMesh() : materialIndex(-1) {}
    
    void draw() const {
        if (vertices.empty()) return;
        
        glBegin(GL_TRIANGLES);
        for (const auto& v : vertices) {
            glNormal3f(v.normal.x, v.normal.y, v.normal.z);
            glTexCoord2f(v.texCoord.x, v.texCoord.y);
            glVertex3f(v.position.x, v.position.y, v.position.z);
        }
        glEnd();
    }
};

// ============================================================================
// Mesh - Collection of submeshes with materials
// ============================================================================
struct Mesh {
    std::string name;
    std::vector<SubMesh> submeshes;
    std::vector<Material> materials;
    Math::AABB bounds;
    bool boundsComputed;
    
    Mesh() : boundsComputed(false) {}
    
    void computeBounds() {
        bool first = true;
        for (const auto& sub : submeshes) {
            for (const auto& v : sub.vertices) {
                if (first) {
                    bounds.min = bounds.max = v.position;
                    first = false;
                } else {
                    bounds.expand(v.position);
                }
            }
        }
        boundsComputed = true;
    }
    
    Math::Vector3 getSize() const {
        return bounds.size();
    }
    
    Math::Vector3 getCenter() const {
        return bounds.center();
    }
    
    void draw() const {
        for (const auto& sub : submeshes) {
            if (sub.materialIndex >= 0 && sub.materialIndex < (int)materials.size()) {
                materials[sub.materialIndex].apply();
                glColor3f(1, 1, 1);
            } else {
                glDisable(GL_TEXTURE_2D);
                glColor3f(0.7f, 0.7f, 0.7f);
            }
            sub.draw();
        }
    }
    
    void drawWithTexture(unsigned int texId) const {
        if (texId > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texId);
            glColor3f(1, 1, 1);
        } else {
            glDisable(GL_TEXTURE_2D);
            glColor3f(0.7f, 0.7f, 0.7f);
        }
        
        for (const auto& sub : submeshes) {
            sub.draw();
        }
    }
    
    int totalVertices() const {
        int count = 0;
        for (const auto& sub : submeshes) {
            count += (int)sub.vertices.size();
        }
        return count;
    }
};

// ============================================================================
// Texture - Wrapper for OpenGL texture
// ============================================================================
struct Texture {
    unsigned int id;
    int width;
    int height;
    int channels;
    std::string path;
    
    Texture() : id(0), width(0), height(0), channels(0) {}
};

// ============================================================================
// ResourceManager - Singleton for managing all game resources
// ============================================================================
class ResourceManager {
public:
    static ResourceManager& instance() {
        static ResourceManager instance;
        return instance;
    }
    
    // ========================================================================
    // Texture Loading
    // ========================================================================
    unsigned int loadTexture(const std::string& path) {
        // Check cache
        auto it = textureCache.find(path);
        if (it != textureCache.end()) {
            return it->second.id;
        }
        
        LOG_INFO("Loading texture: " << path);
        
        Texture tex;
        tex.path = path;
        
        // Use stb_image to load
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(path.c_str(), &tex.width, &tex.height, &tex.channels, 0);
        
        if (!data) {
            LOG_ERROR("Failed to load texture: " << path);
            return 0;
        }
        
        GLenum format = GL_RGB;
        if (tex.channels == 1) format = GL_LUMINANCE;
        else if (tex.channels == 3) format = GL_RGB;
        else if (tex.channels == 4) format = GL_RGBA;
        
        glGenTextures(1, &tex.id);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        
        glTexImage2D(GL_TEXTURE_2D, 0, format, tex.width, tex.height, 0, format, GL_UNSIGNED_BYTE, data);
        
        // Filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        // Generate mipmaps (OpenGL 1.4+ with GLU)
        gluBuild2DMipmaps(GL_TEXTURE_2D, format, tex.width, tex.height, format, GL_UNSIGNED_BYTE, data);
        
        stbi_image_free(data);
        
        textureCache[path] = tex;
        LOG_INFO("Loaded texture: " << path << " (" << tex.width << "x" << tex.height << ", " << tex.channels << " channels)");
        
        return tex.id;
    }
    
    // ========================================================================
    // OBJ/MTL Loading
    // ========================================================================
    Mesh* loadMesh(const std::string& objPath, const std::string& baseDir = "") {
        std::string key = objPath;
        
        auto it = meshCache.find(key);
        if (it != meshCache.end()) {
            return &it->second;
        }
        
        LOG_INFO("Loading mesh: " << objPath);
        
        Mesh mesh;
        mesh.name = objPath;
        
        std::ifstream file(objPath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open OBJ file: " << objPath);
            return nullptr;
        }
        
        // Determine base directory
        std::string dir = baseDir;
        if (dir.empty()) {
            size_t lastSlash = objPath.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                dir = objPath.substr(0, lastSlash + 1);
            }
        } else if (dir.back() != '/' && dir.back() != '\\') {
            dir += "/";
        }
        
        // Temporary storage for parsed data
        std::vector<Math::Vector3> positions;
        std::vector<Math::Vector3> normals;
        std::vector<Math::Vector2> texCoords;
        
        int currentMaterialIndex = -1;
        std::map<int, SubMesh*> materialToSubmesh;
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;
            
            if (cmd == "mtllib") {
                std::string mtlFile;
                std::getline(iss >> std::ws, mtlFile);
                // Trim whitespace
                size_t start = mtlFile.find_first_not_of(" \t\r\n");
                size_t end = mtlFile.find_last_not_of(" \t\r\n");
                if (start != std::string::npos) {
                    mtlFile = mtlFile.substr(start, end - start + 1);
                }
                loadMTL(dir + mtlFile, dir, mesh.materials);
            }
            else if (cmd == "usemtl") {
                std::string matName;
                iss >> matName;
                currentMaterialIndex = findMaterialIndex(mesh.materials, matName);
                
                if (currentMaterialIndex == -1) {
                    // Create a placeholder material
                    Material mat;
                    mat.name = matName;
                    mesh.materials.push_back(mat);
                    currentMaterialIndex = (int)mesh.materials.size() - 1;
                }
            }
            else if (cmd == "v") {
                Math::Vector3 v;
                iss >> v.x >> v.y >> v.z;
                positions.push_back(v);
            }
            else if (cmd == "vn") {
                Math::Vector3 n;
                iss >> n.x >> n.y >> n.z;
                normals.push_back(n);
            }
            else if (cmd == "vt") {
                Math::Vector2 t;
                iss >> t.x >> t.y;
                texCoords.push_back(t);
            }
            else if (cmd == "f") {
                // Get or create submesh for current material
                SubMesh* submesh = nullptr;
                auto it = materialToSubmesh.find(currentMaterialIndex);
                if (it != materialToSubmesh.end()) {
                    submesh = it->second;
                } else {
                    mesh.submeshes.push_back(SubMesh());
                    submesh = &mesh.submeshes.back();
                    submesh->materialIndex = currentMaterialIndex;
                    materialToSubmesh[currentMaterialIndex] = submesh;
                }
                
                // Parse face vertices
                std::vector<int> vIndices, vtIndices, vnIndices;
                std::string token;
                while (iss >> token) {
                    int vi = 0, vti = 0, vni = 0;
                    parseFaceVertex(token, vi, vti, vni);
                    
                    // Convert to 0-based indices
                    if (vi > 0) vi--;
                    else if (vi < 0) vi = (int)positions.size() + vi;
                    
                    if (vti > 0) vti--;
                    else if (vti < 0) vti = (int)texCoords.size() + vti;
                    else vti = -1;
                    
                    if (vni > 0) vni--;
                    else if (vni < 0) vni = (int)normals.size() + vni;
                    else vni = -1;
                    
                    vIndices.push_back(vi);
                    vtIndices.push_back(vti);
                    vnIndices.push_back(vni);
                }
                
                // Triangulate (fan triangulation for convex polygons)
                for (size_t i = 1; i + 1 < vIndices.size(); ++i) {
                    for (int j : {0, (int)i, (int)(i + 1)}) {
                        Vertex vertex;
                        
                        if (vIndices[j] >= 0 && vIndices[j] < (int)positions.size())
                            vertex.position = positions[vIndices[j]];
                        
                        if (vnIndices[j] >= 0 && vnIndices[j] < (int)normals.size())
                            vertex.normal = normals[vnIndices[j]];
                        else
                            vertex.normal = Math::Vector3(0, 1, 0);
                        
                        if (vtIndices[j] >= 0 && vtIndices[j] < (int)texCoords.size())
                            vertex.texCoord = texCoords[vtIndices[j]];
                        
                        submesh->vertices.push_back(vertex);
                    }
                }
            }
        }
        
        mesh.computeBounds();
        meshCache[key] = mesh;
        
        LOG_INFO("Loaded mesh: " << objPath << " (" << mesh.totalVertices() << " vertices, " 
                 << mesh.submeshes.size() << " submeshes, " << mesh.materials.size() << " materials)");
        LOG_INFO("  Size: " << mesh.bounds.size().x << " x " << mesh.bounds.size().y << " x " << mesh.bounds.size().z);
        
        return &meshCache[key];
    }
    
    // Get a cached mesh
    Mesh* getMesh(const std::string& name) {
        auto it = meshCache.find(name);
        if (it != meshCache.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    // Get a cached texture
    unsigned int getTexture(const std::string& name) {
        auto it = textureCache.find(name);
        if (it != textureCache.end()) {
            return it->second.id;
        }
        return 0;
    }
    
    // Cleanup
    void unloadAll() {
        for (auto& pair : textureCache) {
            if (pair.second.id > 0) {
                glDeleteTextures(1, &pair.second.id);
            }
        }
        textureCache.clear();
        meshCache.clear();
    }
    
    void shutdown() {
        unloadAll();
        LOG_INFO("ResourceManager shutdown complete");
    }
    
private:
    ResourceManager() {}
    ~ResourceManager() { unloadAll(); }
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    std::map<std::string, Texture> textureCache;
    std::map<std::string, Mesh> meshCache;
    
    // ========================================================================
    // MTL Loading Helper
    // ========================================================================
    void loadMTL(const std::string& mtlPath, const std::string& baseDir, std::vector<Material>& materials) {
        std::ifstream file(mtlPath);
        if (!file.is_open()) {
            LOG_WARN("Failed to open MTL file: " << mtlPath);
            return;
        }
        
        LOG_INFO("Loading MTL: " << mtlPath);
        
        Material* currentMat = nullptr;
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;
            
            if (cmd == "newmtl") {
                std::string name;
                iss >> name;
                
                // Find existing or create new
                int idx = findMaterialIndex(materials, name);
                if (idx == -1) {
                    materials.push_back(Material());
                    idx = (int)materials.size() - 1;
                }
                currentMat = &materials[idx];
                currentMat->name = name;
            }
            else if (currentMat) {
                if (cmd == "Ka") {
                    iss >> currentMat->ambient.r >> currentMat->ambient.g >> currentMat->ambient.b;
                }
                else if (cmd == "Kd") {
                    iss >> currentMat->diffuse.r >> currentMat->diffuse.g >> currentMat->diffuse.b;
                }
                else if (cmd == "Ks") {
                    iss >> currentMat->specular.r >> currentMat->specular.g >> currentMat->specular.b;
                }
                else if (cmd == "Ke") {
                    iss >> currentMat->emission.r >> currentMat->emission.g >> currentMat->emission.b;
                }
                else if (cmd == "Ns") {
                    iss >> currentMat->shininess;
                }
                else if (cmd == "d" || cmd == "Tr") {
                    iss >> currentMat->alpha;
                }
                else if (cmd == "map_Kd") {
                    std::string texFile;
                    std::getline(iss >> std::ws, texFile);
                    
                    // Trim
                    size_t start = texFile.find_first_not_of(" \t\r\n\"");
                    size_t end = texFile.find_last_not_of(" \t\r\n\"");
                    if (start != std::string::npos) {
                        texFile = texFile.substr(start, end - start + 1);
                    }
                    
                    currentMat->diffuseMapPath = baseDir + texFile;
                    currentMat->diffuseTextureId = loadTexture(currentMat->diffuseMapPath);
                }
                else if (cmd == "map_Bump" || cmd == "bump") {
                    std::string texFile;
                    std::getline(iss >> std::ws, texFile);
                    
                    size_t start = texFile.find_first_not_of(" \t\r\n\"");
                    size_t end = texFile.find_last_not_of(" \t\r\n\"");
                    if (start != std::string::npos) {
                        texFile = texFile.substr(start, end - start + 1);
                    }
                    
                    currentMat->normalMapPath = baseDir + texFile;
                    // Note: Normal maps need special handling in shaders (not available in fixed-function)
                }
            }
        }
    }
    
    int findMaterialIndex(const std::vector<Material>& materials, const std::string& name) {
        for (size_t i = 0; i < materials.size(); ++i) {
            if (materials[i].name == name) return (int)i;
        }
        return -1;
    }
    
    void parseFaceVertex(const std::string& token, int& vi, int& vti, int& vni) {
        vi = vti = vni = 0;
        
        size_t slash1 = token.find('/');
        if (slash1 == std::string::npos) {
            vi = std::stoi(token);
            return;
        }
        
        vi = std::stoi(token.substr(0, slash1));
        
        size_t slash2 = token.find('/', slash1 + 1);
        if (slash2 == std::string::npos) {
            // v/vt format
            if (slash1 + 1 < token.length()) {
                vti = std::stoi(token.substr(slash1 + 1));
            }
            return;
        }
        
        // v/vt/vn or v//vn format
        if (slash2 > slash1 + 1) {
            vti = std::stoi(token.substr(slash1 + 1, slash2 - slash1 - 1));
        }
        if (slash2 + 1 < token.length()) {
            vni = std::stoi(token.substr(slash2 + 1));
        }
    }
};

// Convenience function
inline unsigned int LoadTexture(const std::string& path) {
    return ResourceManager::instance().loadTexture(path);
}

inline Mesh* LoadMesh(const std::string& objPath, const std::string& baseDir = "") {
    return ResourceManager::instance().loadMesh(objPath, baseDir);
}

} // namespace Doomers
