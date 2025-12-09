// ============================================================================
// DOOMERS - ModelLoader.h
// 3D Model loading and rendering using Assimp library
// ============================================================================
#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <windows.h>
#include <glut.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "../Dependencies/assimp/include/assimp/Importer.hpp"
#include "../Dependencies/assimp/include/assimp/scene.h"
#include "../Dependencies/assimp/include/assimp/postprocess.h"
#include "TextureManager.h"

// Maximum models we can load
#define MAX_MODELS 32

// Model IDs
enum ModelID {
    MODEL_CRATE = 0,
    MODEL_HEALTHPACK,
    MODEL_AR_GUN,
    MODEL_AMMO_MAGAZINE,
    MODEL_COUNT
};

// Simple mesh structure for rendering
struct SimpleMesh {
    std::vector<float> vertices;    // x, y, z
    std::vector<float> normals;     // nx, ny, nz
    std::vector<float> texCoords;   // u, v
    std::vector<unsigned int> indices;
    GLuint textureID;
    bool hasTexture;
    float diffuseColor[3];
    
    SimpleMesh() : textureID(0), hasTexture(false) {
        diffuseColor[0] = 0.7f;
        diffuseColor[1] = 0.7f;
        diffuseColor[2] = 0.7f;
    }
    
    void draw() const {
        if (vertices.empty()) return;
        
        if (hasTexture && textureID > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glColor3f(1.0f, 1.0f, 1.0f);
        } else {
            glDisable(GL_TEXTURE_2D);
            glColor3f(diffuseColor[0], diffuseColor[1], diffuseColor[2]);
        }
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices.data());
        
        if (!normals.empty()) {
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, 0, normals.data());
        }
        
        if (!texCoords.empty() && hasTexture) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, texCoords.data());
        }
        
        if (!indices.empty()) {
            glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, indices.data());
        } else {
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vertices.size() / 3));
        }
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        if (hasTexture) {
            glDisable(GL_TEXTURE_2D);
        }
    }
};

// Model data structure
struct Model3D {
    std::vector<SimpleMesh> meshes;
    bool loaded;
    float minX, maxX, minY, maxY, minZ, maxZ;  // Bounding box
    float centerX, centerY, centerZ;
    float scale;  // Default scale to normalize model size
    
    Model3D() : loaded(false), minX(0), maxX(0), minY(0), maxY(0), minZ(0), maxZ(0),
                centerX(0), centerY(0), centerZ(0), scale(1.0f) {}
    
    void calculateBounds() {
        if (meshes.empty()) return;
        
        minX = minY = minZ = 1e10f;
        maxX = maxY = maxZ = -1e10f;
        
        for (const auto& mesh : meshes) {
            for (size_t i = 0; i < mesh.vertices.size(); i += 3) {
                float x = mesh.vertices[i];
                float y = mesh.vertices[i + 1];
                float z = mesh.vertices[i + 2];
                
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
                if (z < minZ) minZ = z;
                if (z > maxZ) maxZ = z;
            }
        }
        
        centerX = (minX + maxX) / 2.0f;
        centerY = (minY + maxY) / 2.0f;
        centerZ = (minZ + maxZ) / 2.0f;
        
        // Calculate scale to fit in 1 unit box
        float sizeX = maxX - minX;
        float sizeY = maxY - minY;
        float sizeZ = maxZ - minZ;
        float maxSize = sizeX;
        if (sizeY > maxSize) maxSize = sizeY;
        if (sizeZ > maxSize) maxSize = sizeZ;
        
        if (maxSize > 0.0001f) {
            scale = 1.0f / maxSize;
        }
    }
    
    void draw(float customScale = 1.0f, bool centerModel = true) const {
        if (!loaded) return;
        
        glPushMatrix();
        
        // Apply scaling
        float finalScale = scale * customScale;
        glScalef(finalScale, finalScale, finalScale);
        
        // Center the model
        if (centerModel) {
            glTranslatef(-centerX, -centerY, -centerZ);
        }
        
        // Draw all meshes
        for (const auto& mesh : meshes) {
            mesh.draw();
        }
        
        glPopMatrix();
    }
};

class ModelLoader {
private:
    static Model3D models[MODEL_COUNT];
    static bool initialized;
    
    // Process a mesh from Assimp
    static SimpleMesh processMesh(const aiMesh* mesh, const aiScene* scene, const char* baseDir) {
        SimpleMesh result;
        
        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            result.vertices.push_back(mesh->mVertices[i].x);
            result.vertices.push_back(mesh->mVertices[i].y);
            result.vertices.push_back(mesh->mVertices[i].z);
            
            if (mesh->HasNormals()) {
                result.normals.push_back(mesh->mNormals[i].x);
                result.normals.push_back(mesh->mNormals[i].y);
                result.normals.push_back(mesh->mNormals[i].z);
            }
            
            if (mesh->mTextureCoords[0]) {
                result.texCoords.push_back(mesh->mTextureCoords[0][i].x);
                result.texCoords.push_back(mesh->mTextureCoords[0][i].y);
            }
        }
        
        // Process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                result.indices.push_back(face.mIndices[j]);
            }
        }
        
        // Process material
        if (mesh->mMaterialIndex >= 0 && scene->mMaterials) {
            const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            
            // Get diffuse color
            aiColor3D color(0.7f, 0.7f, 0.7f);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            result.diffuseColor[0] = color.r;
            result.diffuseColor[1] = color.g;
            result.diffuseColor[2] = color.b;
            
            // Try to load texture
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString texPath;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
                    // Build full texture path
                    char fullPath[512];
                    sprintf(fullPath, "%s/%s", baseDir, texPath.C_Str());
                    
                    // Try to load texture with power-of-2 resizing for compatibility
                    result.textureID = SOIL_load_OGL_texture(
                        fullPath,
                        SOIL_LOAD_AUTO,
                        SOIL_CREATE_NEW_ID,
                        SOIL_FLAG_INVERT_Y | SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS
                    );
                    
                    if (result.textureID > 0) {
                        result.hasTexture = true;
                        glBindTexture(GL_TEXTURE_2D, result.textureID);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        printf("  Loaded texture: %s\n", fullPath);
                    } else {
                        printf("  Failed to load texture: %s\n", fullPath);
                    }
                }
            }
        }
        
        return result;
    }
    
    // Recursively process nodes
    static void processNode(const aiNode* node, const aiScene* scene, Model3D& model, const char* baseDir) {
        // Process all meshes in this node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model.meshes.push_back(processMesh(mesh, scene, baseDir));
        }
        
        // Process children
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, model, baseDir);
        }
    }
    
    static bool loadModel(const char* filepath, Model3D& model, const char* textureOverride = nullptr, bool invertTextureY = true) {
        printf("Loading model: %s\n", filepath);
        
        // Check if this is an FBX file - FBX files may need different UV handling
        bool isFBX = strstr(filepath, ".fbx") != nullptr || strstr(filepath, ".FBX") != nullptr;
        
        Assimp::Importer importer;
        
        // FBX files from Mixamo typically don't need UV flipping
        unsigned int importFlags = aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_JoinIdenticalVertices |
            aiProcess_OptimizeMeshes;
        
        // Only flip UVs for non-FBX files (OBJ, etc.)
        if (!isFBX) {
            importFlags |= aiProcess_FlipUVs;
        }
        
        const aiScene* scene = importer.ReadFile(filepath, importFlags);
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            printf("ERROR: Assimp failed to load model: %s\n", importer.GetErrorString());
            return false;
        }
        
        // Get base directory for textures
        char baseDir[512];
        strcpy(baseDir, filepath);
        char* lastSlash = strrchr(baseDir, '/');
        if (!lastSlash) lastSlash = strrchr(baseDir, '\\');
        if (lastSlash) *lastSlash = '\0';
        else strcpy(baseDir, ".");
        
        // Process the scene
        model.meshes.clear();
        processNode(scene->mRootNode, scene, model, baseDir);
        
        // If texture override specified, apply it to all meshes
        if (textureOverride) {
            // Use SOIL_FLAG_POWER_OF_TWO to handle non-power-of-2 textures
            // Also use SOIL_FLAG_MIPMAPS for better quality
            unsigned int soilFlags = SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS;
            if (invertTextureY) {
                soilFlags |= SOIL_FLAG_INVERT_Y;
            }
            
            GLuint overrideTex = SOIL_load_OGL_texture(
                textureOverride,
                SOIL_LOAD_AUTO,
                SOIL_CREATE_NEW_ID,
                soilFlags
            );
            
            if (overrideTex > 0) {
                glBindTexture(GL_TEXTURE_2D, overrideTex);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                
                for (auto& mesh : model.meshes) {
                    mesh.textureID = overrideTex;
                    mesh.hasTexture = true;
                }
                printf("  Applied override texture: %s\n", textureOverride);
            } else {
                printf("  WARNING: Failed to load override texture: %s\n", textureOverride);
                printf("  SOIL error: %s\n", SOIL_last_result());
            }
        }
        
        model.calculateBounds();
        model.loaded = true;
        
        printf("SUCCESS: Loaded model with %zu meshes\n", model.meshes.size());
        printf("  Bounds: (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f)\n", 
               model.minX, model.minY, model.minZ, model.maxX, model.maxY, model.maxZ);
        printf("  Scale factor: %.4f\n", model.scale);
        
        return true;
    }
    
public:
    static void init() {
        if (initialized) return;
        
        printf("=== ModelLoader: Initializing 3D models ===\n");
        
        // Get absolute path based on exe location
        char exePath[512];
        GetModuleFileNameA(NULL, exePath, 512);
        // Remove exe filename to get directory
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash) *lastSlash = '\0';
        
        char modelPath[512], texturePath[512];
        printf("Model base path: %s\\..\\res\\Models3D\\\n", exePath);
        
        // Load crate model
        sprintf(modelPath, "%s\\..\\res\\Models3D\\gart130-crate\\source\\L_Crate_2fbx.obj", exePath);
        sprintf(texturePath, "%s\\..\\res\\Models3D\\gart130-crate\\textures\\L_Crate.2fbx_lambert5_BaseColor.png", exePath);
        loadModel(modelPath, models[MODEL_CRATE], texturePath);
        
        // Load health pack model
        sprintf(modelPath, "%s\\..\\res\\Models3D\\health-pack\\source\\HealthPack\\healthpack.obj", exePath);
        sprintf(texturePath, "%s\\..\\res\\Models3D\\health-pack\\textures\\Healthpack_Textured_Albedo.png", exePath);
        loadModel(modelPath, models[MODEL_HEALTHPACK], texturePath);
        
        // Load AR gun model
        sprintf(modelPath, "%s\\..\\res\\Models3D\\AR\\source\\Gun.obj", exePath);
        sprintf(texturePath, "%s\\..\\res\\Models3D\\AR\\textures\\GAP_Examen_Gun_albedo_DriesDeryckere.tga.png", exePath);
        loadModel(modelPath, models[MODEL_AR_GUN], texturePath);
        
        // Load ammo magazine model (MTL file was missing, now created)
        sprintf(modelPath, "%s\\..\\res\\Models3D\\ak-47-magazine\\source\\ak_47_round.obj", exePath);
        sprintf(texturePath, "%s\\..\\res\\Models3D\\ak-47-magazine\\textures\\ak_47_round_BaseColor.jpeg", exePath);
        loadModel(modelPath, models[MODEL_AMMO_MAGAZINE], texturePath);
        
        initialized = true;
        
        // Print summary
        int loaded = 0;
        for (int i = 0; i < MODEL_COUNT; i++) {
            if (models[i].loaded) loaded++;
        }
        printf("=== ModelLoader: Initialized %d/%d models ===\n", loaded, MODEL_COUNT);
    }
    
    static bool isLoaded(ModelID id) {
        if (!initialized) init();
        if (id < 0 || id >= MODEL_COUNT) return false;
        return models[id].loaded;
    }
    
    static const Model3D* get(ModelID id) {
        if (!initialized) init();
        if (id < 0 || id >= MODEL_COUNT) return nullptr;
        if (!models[id].loaded) return nullptr;
        return &models[id];
    }
    
    // Draw a model with custom scale
    static void draw(ModelID id, float scale = 1.0f, bool centerModel = true) {
        if (!initialized) init();
        if (id < 0 || id >= MODEL_COUNT) return;
        if (!models[id].loaded) return;
        
        models[id].draw(scale, centerModel);
    }
    
    // Draw model grounded (feet on Y=0) instead of centered
    // This is for character models where we want feet touching the ground
    static void drawGrounded(ModelID id, float scale = 1.0f) {
        if (!initialized) init();
        if (id < 0 || id >= MODEL_COUNT) return;
        if (!models[id].loaded) return;
        
        glPushMatrix();
        
        // Apply scaling
        float finalScale = models[id].scale * scale;
        glScalef(finalScale, finalScale, finalScale);
        
        // Center X and Z, but use minY for Y (so model stands on ground)
        // This places the model's feet at Y=0
        glTranslatef(-models[id].centerX, -models[id].minY, -models[id].centerZ);
        
        // Draw all meshes
        for (const auto& mesh : models[id].meshes) {
            mesh.draw();
        }
        
        glPopMatrix();
    }
    
    // Draw model at specific position with rotation
    static void drawAt(ModelID id, float x, float y, float z, 
                       float scale = 1.0f, float rotY = 0.0f, float rotX = 0.0f, float rotZ = 0.0f) {
        if (!initialized) init();
        if (id < 0 || id >= MODEL_COUNT) return;
        if (!models[id].loaded) return;
        
        glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(rotY, 0, 1, 0);
        glRotatef(rotX, 1, 0, 0);
        glRotatef(rotZ, 0, 0, 1);
        models[id].draw(scale, true);
        glPopMatrix();
    }
    
    static void cleanup() {
        if (!initialized) return;
        
        for (int i = 0; i < MODEL_COUNT; i++) {
            for (auto& mesh : models[i].meshes) {
                if (mesh.textureID > 0) {
                    glDeleteTextures(1, &mesh.textureID);
                }
            }
            models[i].meshes.clear();
            models[i].loaded = false;
        }
        
        initialized = false;
    }
    
    // Public method to load model into external Model3D (for animation frames)
    static bool loadModelDirect(const char* filepath, Model3D& model, 
                                 const char* textureOverride = nullptr, 
                                 bool invertTextureY = true) {
        return loadModel(filepath, model, textureOverride, invertTextureY);
    }
};

// Static member definitions
Model3D ModelLoader::models[MODEL_COUNT];
bool ModelLoader::initialized = false;

// ============================================================================
// FRAME-BASED ANIMATION SYSTEM
// Loads baked animation frames from separate FBX files
// ============================================================================

#define MAX_ANIM_FRAMES 32

enum AnimationType {
    ANIM_IDLE = 0,
    ANIM_WALK,
    ANIM_KICK,
    ANIM_COUNT
};

struct FrameAnimation {
    Model3D frames[MAX_ANIM_FRAMES];
    int frameCount;
    float fps;
    bool loaded;
    bool loop;
    
    // Reference values from first frame - used for consistent positioning
    float refScale;
    float refCenterX, refCenterZ;
    float refMinY;
    
    FrameAnimation() : frameCount(0), fps(24.0f), loaded(false), loop(true),
                       refScale(1.0f), refCenterX(0), refCenterZ(0), refMinY(0) {}
};

class AnimationLoader {
private:
    static FrameAnimation animations[ANIM_COUNT];
    static bool initialized;
    
    static bool loadAnimationFrames(AnimationType anim, const char* basePath, 
                                     const char* prefix, const char* texturePath,
                                     int frameNumbers[], int count) {
        if (count > MAX_ANIM_FRAMES) count = MAX_ANIM_FRAMES;
        
        animations[anim].frameCount = 0;
        
        char framePath[512];
        for (int i = 0; i < count; i++) {
            sprintf(framePath, "%s\\%s_%03d.fbx", basePath, prefix, frameNumbers[i]);
            
            if (ModelLoader::loadModelDirect(framePath, animations[anim].frames[i], texturePath, true)) {
                // Store reference values from first successfully loaded frame
                if (animations[anim].frameCount == 0) {
                    Model3D& ref = animations[anim].frames[i];
                    animations[anim].refScale = ref.scale;
                    animations[anim].refCenterX = ref.centerX;
                    animations[anim].refCenterZ = ref.centerZ;
                    animations[anim].refMinY = ref.minY;
                }
                animations[anim].frameCount++;
            } else {
                printf("  Failed to load frame: %s\n", framePath);
            }
        }
        
        animations[anim].loaded = (animations[anim].frameCount > 0);
        printf("  Loaded %d/%d frames for animation %d\n", 
               animations[anim].frameCount, count, anim);
        return animations[anim].loaded;
    }
    
public:
    static void init() {
        if (initialized) return;
        
        printf("=== AnimationLoader: Loading baked animation frames ===\n");
        
        char exePath[512];
        GetModuleFileNameA(NULL, exePath, 512);
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash) *lastSlash = '\0';
        
        char bakedPath[512], texturePath[512];
        sprintf(bakedPath, "%s\\..\\res\\Models3D\\devil\\baked", exePath);
        sprintf(texturePath, "%s\\..\\res\\Models3D\\devil\\devil.png", exePath);
        
        // Check if baked folder exists
        DWORD attrib = GetFileAttributesA(bakedPath);
        if (attrib == INVALID_FILE_ATTRIBUTES || !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
            printf("  Baked animations folder not found: %s\n", bakedPath);
            printf("  Falling back to static models\n");
            initialized = true;
            return;
        }
        
        // Load WALK animation frames - sequential frames for smooth animation
        // We have walk_001 to walk_250, load 30 frames (every 8th) for performance
        printf("  Loading WALK animation (30 frames)...\n");
        int walkFrames[] = {
            1, 9, 17, 25, 33, 41, 49, 57, 65, 73,
            81, 89, 97, 105, 113, 121, 129, 137, 145, 153,
            161, 169, 177, 185, 193, 201, 209, 217, 225, 233
        };
        loadAnimationFrames(ANIM_WALK, bakedPath, "walk", texturePath, walkFrames, 30);
        animations[ANIM_WALK].fps = 30.0f;  // 30fps animation playback
        
        // Load KICK animation frames - for attack
        // Frames: 1, 5, 9, 13... up to 249 (every 4th frame = 63 frames total)
        // We'll load 20 frames for a snappy kick
        printf("  Loading KICK animation (20 frames)...\n");
        int kickFrames[] = {
            1, 13, 25, 37, 49, 61, 73, 85, 97, 109,
            121, 133, 145, 157, 169, 181, 193, 205, 217, 229
        };
        loadAnimationFrames(ANIM_KICK, bakedPath, "kick", texturePath, kickFrames, 20);
        animations[ANIM_KICK].fps = 24.0f;  // Faster for snappy attack
        animations[ANIM_KICK].loop = false;  // Don't loop - play once
        
        // No idle animation
        animations[ANIM_IDLE].loaded = false;
        
        initialized = true;
        printf("=== AnimationLoader: Initialization complete ===\n");
    }
    
    static bool isLoaded(AnimationType anim) {
        if (!initialized) return false;  // Don't lazy init - must call init() at startup
        if (anim < 0 || anim >= ANIM_COUNT) return false;
        return animations[anim].loaded && animations[anim].frameCount > 0;
    }
    
    static int getFrameCount(AnimationType anim) {
        if (!initialized) return 0;
        if (anim < 0 || anim >= ANIM_COUNT) return 0;
        return animations[anim].frameCount;
    }
    
    // Get frame index based on time
    static int getFrameIndex(AnimationType anim, float time) {
        if (!initialized) return 0;
        if (anim < 0 || anim >= ANIM_COUNT) return 0;
        if (!animations[anim].loaded || animations[anim].frameCount == 0) return 0;
        
        float fps = animations[anim].fps;
        int frameCount = animations[anim].frameCount;
        int frame = (int)(time * fps) % frameCount;
        
        if (!animations[anim].loop && frame >= frameCount - 1) {
            frame = frameCount - 1;  // Hold on last frame
        }
        
        return frame;
    }
    
    // Draw animation frame grounded
    static void drawGrounded(AnimationType anim, int frameIndex, float scale = 1.0f) {
        if (!initialized) return;
        if (anim < 0 || anim >= ANIM_COUNT) return;
        if (!animations[anim].loaded || animations[anim].frameCount == 0) return;
        
        // Clamp frame index
        if (frameIndex < 0) frameIndex = 0;
        if (frameIndex >= animations[anim].frameCount) {
            frameIndex = animations[anim].frameCount - 1;
        }
        
        Model3D& model = animations[anim].frames[frameIndex];
        if (!model.loaded) return;
        
        glPushMatrix();
        
        // Use REFERENCE scale and center from first frame to prevent shaking
        FrameAnimation& animData = animations[anim];
        float finalScale = animData.refScale * scale;
        glScalef(finalScale, finalScale, finalScale);
        
        // Ground the model using REFERENCE values (feet at Y=0)
        glTranslatef(-animData.refCenterX, -animData.refMinY, -animData.refCenterZ);
        
        for (const auto& mesh : model.meshes) {
            mesh.draw();
        }
        
        glPopMatrix();
    }
    
    // Draw animation based on time (automatically calculates frame)
    static void drawAnimated(AnimationType anim, float time, float scale = 1.0f) {
        int frameIndex = getFrameIndex(anim, time);
        drawGrounded(anim, frameIndex, scale);
    }
};

// Static member definitions for AnimationLoader
FrameAnimation AnimationLoader::animations[ANIM_COUNT];
bool AnimationLoader::initialized = false;

#endif // MODEL_LOADER_H
