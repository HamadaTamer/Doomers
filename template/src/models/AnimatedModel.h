// ============================================================================
// DOOMERS - AnimatedModel.h
// Assimp-based animated model loader for FBX models
// ============================================================================
#ifndef ANIMATED_MODEL_H
#define ANIMATED_MODEL_H

#include <string>
#include <vector>
#include <map>
#include <direct.h>  // For _getcwd
#include <glut.h>

// SOIL for loading textures
#include "../../Dependencies/soil/include/SOIL.h"

// Assimp headers
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace AnimatedModel {

    // ==================== BONE STRUCTURE ====================
    struct BoneInfo {
        aiMatrix4x4 offsetMatrix;
        aiMatrix4x4 finalTransform;
        int parentIndex;
    };

    // ==================== VERTEX WITH BONE WEIGHTS ====================
    struct AnimVertex {
        float position[3];
        float normal[3];
        float texCoords[2];
        int boneIDs[4];
        float boneWeights[4];
    };

    // ==================== MESH DATA ====================
    struct MeshData {
        std::vector<AnimVertex> vertices;
        std::vector<unsigned int> indices;
        GLuint VAO, VBO, EBO;
        GLuint textureID;           // OpenGL texture ID
        bool hasTexture;            // True if texture was loaded
        bool initialized;
        aiMatrix4x4 nodeTransform;  // Transform from node hierarchy
        bool hasBoneWeights;        // True if vertices have valid bone weights
        
        MeshData() : VAO(0), VBO(0), EBO(0), textureID(0), hasTexture(false), 
                     initialized(false), hasBoneWeights(false) {
            // Initialize to identity
            nodeTransform = aiMatrix4x4();
        }
    };

    // ==================== ANIMATION KEYFRAMES ====================
    struct VectorKey {
        double time;
        aiVector3D value;
    };

    struct QuatKey {
        double time;
        aiQuaternion value;
    };

    struct BoneAnimation {
        std::string boneName;
        std::vector<VectorKey> positionKeys;
        std::vector<QuatKey> rotationKeys;
        std::vector<VectorKey> scaleKeys;
    };

    struct Animation {
        std::string name;
        double duration;
        double ticksPerSecond;
        std::vector<BoneAnimation> channels;
    };

    // ==================== ANIMATED MODEL CLASS ====================
    class Model {
    public:
        std::vector<MeshData> meshes;
        std::vector<BoneInfo> bones;
        std::map<std::string, int> boneMapping;
        std::vector<Animation> animations;
        aiMatrix4x4 globalInverseTransform;
        
        const aiScene* scene;
        Assimp::Importer importer;
        
        bool loaded;
        std::string directory;
        
        // Current animation state
        int currentAnimation;
        float animationTime;
        float animationSpeed;
        bool looping;
        
        // Bone transforms for current frame
        std::vector<aiMatrix4x4> boneTransforms;
        
        Model() : scene(nullptr), loaded(false), currentAnimation(0), 
                  animationTime(0.0f), animationSpeed(1.0f), looping(true) {}
        
        ~Model() {
            cleanup();
        }
        
        // ==================== LOAD MODEL ====================
        bool load(const std::string& path) {
            printf("Calling Assimp ReadFile for: %s\n", path.c_str());
            fflush(stdout);
            
            scene = importer.ReadFile(path,
                aiProcess_Triangulate |
                aiProcess_GenNormals |
                aiProcess_FlipUVs |
                aiProcess_CalcTangentSpace |
                aiProcess_LimitBoneWeights);
            
            printf("Assimp ReadFile completed\n");
            fflush(stdout);
            
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                printf("Assimp Error: %s\n", importer.GetErrorString());
                printf("  scene=%p, flags=%d, rootNode=%p\n", 
                       (void*)scene, 
                       scene ? (int)scene->mFlags : -1, 
                       scene ? (void*)scene->mRootNode : nullptr);
                fflush(stdout);
                return false;
            }
            
            printf("Scene loaded - processing nodes...\n");
            fflush(stdout);
            
            directory = path.substr(0, path.find_last_of("/\\"));
            
            // Get global inverse transform
            printf("Getting global inverse transform...\n"); fflush(stdout);
            globalInverseTransform = scene->mRootNode->mTransformation;
            globalInverseTransform.Inverse();
            
            // Process all meshes - pass identity as initial parent transform
            printf("Processing nodes (meshes count in scene: %d)...\n", scene->mNumMeshes); fflush(stdout);
            aiMatrix4x4 identity;
            processNode(scene->mRootNode, scene, identity);
            printf("Nodes processed. Meshes loaded: %d\n", (int)meshes.size()); fflush(stdout);
            
            // Load animations
            printf("Loading animations...\n"); fflush(stdout);
            loadAnimations();
            printf("Animations loaded: %d\n", (int)animations.size()); fflush(stdout);
            
            // Initialize bone transforms to bind pose
            printf("Initializing bone transforms (%d bones)...\n", (int)bones.size()); fflush(stdout);
            boneTransforms.resize(bones.size());
            
            // Calculate initial bind pose by traversing the skeleton
            // This ensures bones are in correct positions even before animation plays
            if (scene && scene->mRootNode) {
                calculateBoneTransforms(scene->mRootNode, aiMatrix4x4());
            }
            printf("Bone transforms initialized to bind pose.\n"); fflush(stdout);
            
            loaded = true;
            printf("======================================\n");
            printf("MODEL LOADED SUCCESSFULLY!\n");
            printf("======================================\n");
            printf("  Path: %s\n", path.c_str());
            printf("  Meshes: %d\n", (int)meshes.size());
            printf("  Bones: %d\n", (int)bones.size());
            printf("  Animations: %d\n", (int)animations.size());
            
            // Log all animation names
            if (animations.size() > 0) {
                printf("\n  Animation Names Found:\n");
                for (size_t i = 0; i < animations.size(); i++) {
                    printf("    [%d] \"%s\" (duration: %.2f, ticks/sec: %.2f)\n", 
                           (int)i, animations[i].name.c_str(), 
                           animations[i].duration, animations[i].ticksPerSecond);
                }
            } else {
                printf("  WARNING: No animations found in model!\n");
            }
            printf("======================================\n");
            
            return true;
        }
        
        // ==================== PROCESS NODE RECURSIVELY ====================
        void processNode(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform) {
            if (!node) {
                printf("WARNING: null node encountered!\n"); fflush(stdout);
                return;
            }
            
            // For skinned meshes, we DON'T want to apply node transforms
            // The bone system handles positioning. Node transforms in FBX often
            // include unwanted scale factors (like 100x from Blender)
            
            printf("Processing node: %s (meshes: %d, children: %d)\n", 
                   node->mName.C_Str(), node->mNumMeshes, node->mNumChildren); 
            fflush(stdout);
            
            // Process all meshes in this node - use IDENTITY transform
            for (unsigned int i = 0; i < node->mNumMeshes; i++) {
                printf("  Processing mesh %d...\n", i); fflush(stdout);
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                MeshData meshData = processMesh(mesh, scene);
                // Keep identity transform - vertices are already in bind pose
                meshes.push_back(meshData);
                printf("  Mesh %d done.\n", i); fflush(stdout);
            }
            
            // Process children
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                processNode(node->mChildren[i], scene, parentTransform);
            }
        }
        
        // ==================== PROCESS MESH ====================
        MeshData processMesh(aiMesh* mesh, const aiScene* scene) {
            MeshData data;
            
            printf("    - Mesh has %d vertices, %d faces\n", mesh->mNumVertices, mesh->mNumFaces); fflush(stdout);
            
            // Process vertices
            printf("    - Processing vertices...\n"); fflush(stdout);
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                AnimVertex vertex;
                
                vertex.position[0] = mesh->mVertices[i].x;
                vertex.position[1] = mesh->mVertices[i].y;
                vertex.position[2] = mesh->mVertices[i].z;
                
                if (mesh->HasNormals()) {
                    vertex.normal[0] = mesh->mNormals[i].x;
                    vertex.normal[1] = mesh->mNormals[i].y;
                    vertex.normal[2] = mesh->mNormals[i].z;
                }
                
                if (mesh->mTextureCoords[0]) {
                    vertex.texCoords[0] = mesh->mTextureCoords[0][i].x;
                    vertex.texCoords[1] = mesh->mTextureCoords[0][i].y;
                }
                
                // Initialize bone data
                for (int j = 0; j < 4; j++) {
                    vertex.boneIDs[j] = 0;
                    vertex.boneWeights[j] = 0.0f;
                }
                
                data.vertices.push_back(vertex);
            }
            printf("    - Vertices done: %d\n", (int)data.vertices.size()); fflush(stdout);
            
            // Process indices
            printf("    - Processing faces...\n"); fflush(stdout);
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    data.indices.push_back(face.mIndices[j]);
                }
            }
            printf("    - Indices done: %d\n", (int)data.indices.size()); fflush(stdout);
            
            // Process bones with safety checks
            if (mesh->HasBones()) {
                printf("    - Processing %d bones...\n", mesh->mNumBones); fflush(stdout);
                int totalAssignedWeights = 0;
                for (unsigned int i = 0; i < mesh->mNumBones; i++) {
                    try {
                        aiBone* bone = mesh->mBones[i];
                        if (!bone) {
                            printf("      Bone %d: NULL bone, skipping\n", i); fflush(stdout);
                            continue;
                        }
                        
                        if (!bone->mName.data || bone->mName.length == 0) {
                            printf("      Bone %d: Invalid name, skipping\n", i); fflush(stdout);
                            continue;
                        }
                        
                        std::string boneName = bone->mName.C_Str();
                        printf("      Bone %d: %s (%d weights)\n", i, boneName.c_str(), bone->mNumWeights); fflush(stdout);
                        
                        int boneIndex;
                        if (boneMapping.find(boneName) == boneMapping.end()) {
                            boneIndex = (int)bones.size();
                            BoneInfo bi;
                            // Safely copy offset matrix element by element
                            for (int r = 0; r < 4; r++) {
                                for (int c = 0; c < 4; c++) {
                                    bi.offsetMatrix[r][c] = bone->mOffsetMatrix[r][c];
                                }
                            }
                            bi.parentIndex = -1;
                            bones.push_back(bi);
                            boneMapping[boneName] = boneIndex;
                        } else {
                            boneIndex = boneMapping[boneName];
                        }
                        
                        // Assign bone weights to vertices with safety checks
                        if (bone->mNumWeights > 0 && bone->mWeights != nullptr) {
                            int assignedCount = 0;
                            for (unsigned int j = 0; j < bone->mNumWeights; j++) {
                                unsigned int vertexID = bone->mWeights[j].mVertexId;
                                float weight = bone->mWeights[j].mWeight;
                                
                                if (vertexID >= data.vertices.size()) {
                                    continue;
                                }
                                
                                // Skip very small weights but not zero (some exporters use small values)
                                if (weight < 0.0001f) {
                                    continue;
                                }
                                
                                // Find empty slot for this bone weight
                                for (int k = 0; k < 4; k++) {
                                    if (data.vertices[vertexID].boneWeights[k] < 0.0001f) {
                                        data.vertices[vertexID].boneIDs[k] = boneIndex;
                                        data.vertices[vertexID].boneWeights[k] = weight;
                                        assignedCount++;
                                        break;
                                    }
                                }
                            }
                            if (assignedCount == 0 && bone->mNumWeights > 0) {
                                // Debug: print first weight value
                                printf("        (first weight value: %.6f)\n", bone->mWeights[0].mWeight);
                            }
                            totalAssignedWeights += assignedCount;
                        } else {
                            printf("        Warning: No weights array for this bone\n"); fflush(stdout);
                        }
                    } catch (...) {
                        printf("      Bone %d: Exception occurred, skipping\n", i); fflush(stdout);
                        continue;
                    }
                }
                printf("    - Bones processed: %d total bones, %d vertex weights assigned\n", (int)bones.size(), totalAssignedWeights); fflush(stdout);
                data.hasBoneWeights = (totalAssignedWeights > 0);
            } else {
                printf("    - No bones in this mesh\n"); fflush(stdout);
                data.hasBoneWeights = false;
            }
            
            // Load texture from material
            data.hasTexture = false;
            data.textureID = 0;
            if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < scene->mNumMaterials) {
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                printf("    - Material index: %d\n", mesh->mMaterialIndex); fflush(stdout);
                
                // Check texture counts for different types
                int diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
                int ambientCount = material->GetTextureCount(aiTextureType_AMBIENT);
                int unknownCount = material->GetTextureCount(aiTextureType_UNKNOWN);
                printf("    - Texture counts: diffuse=%d, ambient=%d, unknown=%d\n", 
                       diffuseCount, ambientCount, unknownCount); fflush(stdout);
                
                // Also check for embedded textures directly
                printf("    - Scene has %d embedded textures\n", scene->mNumTextures); fflush(stdout);
                
                if (diffuseCount > 0) {
                    aiString texPath;
                    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
                        printf("    - Texture path: %s\n", texPath.C_Str()); fflush(stdout);
                        
                        // Check if it's an embedded texture (starts with *)
                        if (texPath.data[0] == '*') {
                            int texIndex = atoi(texPath.C_Str() + 1);
                            if (texIndex >= 0 && texIndex < (int)scene->mNumTextures) {
                                aiTexture* embeddedTex = scene->mTextures[texIndex];
                                printf("    - Loading embedded texture %d (%dx%d)\n", 
                                       texIndex, embeddedTex->mWidth, embeddedTex->mHeight); fflush(stdout);
                                data.textureID = loadEmbeddedTexture(embeddedTex);
                                data.hasTexture = (data.textureID != 0);
                            }
                        } else {
                            // Try to load from file using SOIL
                            std::string fullPath = directory + "/" + texPath.C_Str();
                            printf("    - Trying to load texture from: %s\n", fullPath.c_str()); fflush(stdout);
                            
                            GLuint texID = SOIL_load_OGL_texture(
                                fullPath.c_str(),
                                SOIL_LOAD_RGBA,
                                SOIL_CREATE_NEW_ID,
                                SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
                            );
                            
                            if (texID) {
                                data.textureID = texID;
                                data.hasTexture = true;
                                printf("    - External texture loaded with SOIL: ID=%d\n", texID); fflush(stdout);
                            } else {
                                printf("    - Failed to load external texture: %s\n", SOIL_last_result()); fflush(stdout);
                            }
                        }
                    }
                }
            }
            
            return data;
        }
        
        // ==================== LOAD EMBEDDED TEXTURE ====================
        GLuint loadEmbeddedTexture(aiTexture* tex) {
            if (!tex) return 0;
            
            // If mHeight is 0, texture is compressed (PNG, JPG, etc.)
            if (tex->mHeight == 0) {
                printf("      Compressed texture format: %s, size: %d bytes\n", 
                       tex->achFormatHint, tex->mWidth); fflush(stdout);
                
                // Use SOIL to decode compressed texture from memory
                GLuint textureID = SOIL_load_OGL_texture_from_memory(
                    (unsigned char*)tex->pcData,
                    tex->mWidth,  // This is the size in bytes for compressed textures
                    SOIL_LOAD_RGBA,
                    SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
                );
                
                if (textureID) {
                    printf("      Texture loaded with SOIL: ID=%d\n", textureID); 
                    fflush(stdout);
                    return textureID;
                } else {
                    printf("      Failed to decode texture with SOIL: %s\n", SOIL_last_result());
                    fflush(stdout);
                    return 0;
                }
            } else {
                // Uncompressed ARGB8888 texture
                printf("      Uncompressed texture: %dx%d\n", tex->mWidth, tex->mHeight); fflush(stdout);
                
                // Convert ARGB to RGBA
                unsigned char* rgbaData = new unsigned char[tex->mWidth * tex->mHeight * 4];
                for (unsigned int i = 0; i < tex->mWidth * tex->mHeight; i++) {
                    aiTexel& t = tex->pcData[i];
                    rgbaData[i * 4 + 0] = t.r;
                    rgbaData[i * 4 + 1] = t.g;
                    rgbaData[i * 4 + 2] = t.b;
                    rgbaData[i * 4 + 3] = t.a;
                }
                
                GLuint textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->mWidth, tex->mHeight, 
                             0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                
                delete[] rgbaData;
                printf("      Texture loaded: ID=%d\n", textureID); fflush(stdout);
                return textureID;
            }
        }
        
        // ==================== LOAD ANIMATIONS ====================
        void loadAnimations() {
            if (!scene->HasAnimations()) return;
            
            for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
                aiAnimation* anim = scene->mAnimations[i];
                Animation animation;
                animation.name = anim->mName.C_Str();
                animation.duration = anim->mDuration;
                animation.ticksPerSecond = anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0;
                
                // Process animation channels (bone animations)
                for (unsigned int j = 0; j < anim->mNumChannels; j++) {
                    aiNodeAnim* channel = anim->mChannels[j];
                    BoneAnimation boneAnim;
                    boneAnim.boneName = channel->mNodeName.C_Str();
                    
                    // Position keys
                    for (unsigned int k = 0; k < channel->mNumPositionKeys; k++) {
                        VectorKey key;
                        key.time = channel->mPositionKeys[k].mTime;
                        key.value = channel->mPositionKeys[k].mValue;
                        boneAnim.positionKeys.push_back(key);
                    }
                    
                    // Rotation keys
                    for (unsigned int k = 0; k < channel->mNumRotationKeys; k++) {
                        QuatKey key;
                        key.time = channel->mRotationKeys[k].mTime;
                        key.value = channel->mRotationKeys[k].mValue;
                        boneAnim.rotationKeys.push_back(key);
                    }
                    
                    // Scale keys
                    for (unsigned int k = 0; k < channel->mNumScalingKeys; k++) {
                        VectorKey key;
                        key.time = channel->mScalingKeys[k].mTime;
                        key.value = channel->mScalingKeys[k].mValue;
                        boneAnim.scaleKeys.push_back(key);
                    }
                    
                    animation.channels.push_back(boneAnim);
                }
                
                animations.push_back(animation);
                printf("  Animation: %s (%.1f frames)\n", animation.name.c_str(), animation.duration);
            }
        }
        
        // ==================== SET ANIMATION ====================
        void setAnimation(int index) {
            if (index >= 0 && index < (int)animations.size()) {
                currentAnimation = index;
                animationTime = 0.0f;
            }
        }
        
        bool setAnimationByName(const std::string& name) {
            for (size_t i = 0; i < animations.size(); i++) {
                if (animations[i].name.find(name) != std::string::npos) {
                    setAnimation((int)i);
                    return true;
                }
            }
            return false;
        }
        
        // ==================== UPDATE ANIMATION ====================
        void update(float deltaTime) {
            if (!loaded || animations.empty()) return;
            
            Animation& anim = animations[currentAnimation];
            
            animationTime += deltaTime * animationSpeed * (float)anim.ticksPerSecond;
            
            if (looping) {
                animationTime = fmod(animationTime, (float)anim.duration);
            } else if (animationTime > anim.duration) {
                animationTime = (float)anim.duration;
            }
            
            // Calculate bone transforms
            calculateBoneTransforms(scene->mRootNode, aiMatrix4x4());
        }
        
        // ==================== CALCULATE BONE TRANSFORMS ====================
        void calculateBoneTransforms(aiNode* node, const aiMatrix4x4& parentTransform) {
            std::string nodeName = node->mName.C_Str();
            aiMatrix4x4 nodeTransform = node->mTransformation;
            
            // Check if this node has animation
            if (!animations.empty()) {
                Animation& anim = animations[currentAnimation];
                for (const auto& channel : anim.channels) {
                    if (channel.boneName == nodeName) {
                        // Interpolate position
                        aiVector3D position = interpolatePosition(channel);
                        aiQuaternion rotation = interpolateRotation(channel);
                        aiVector3D scale = interpolateScale(channel);
                        
                        // Build transform matrix
                        aiMatrix4x4 translationM, rotationM, scaleM;
                        aiMatrix4x4::Translation(position, translationM);
                        rotationM = aiMatrix4x4(rotation.GetMatrix());
                        aiMatrix4x4::Scaling(scale, scaleM);
                        
                        nodeTransform = translationM * rotationM * scaleM;
                        break;
                    }
                }
            }
            
            aiMatrix4x4 globalTransform = parentTransform * nodeTransform;
            
            // Update bone transform if this is a bone
            if (boneMapping.find(nodeName) != boneMapping.end()) {
                int boneIndex = boneMapping[nodeName];
                boneTransforms[boneIndex] = globalInverseTransform * globalTransform * bones[boneIndex].offsetMatrix;
            }
            
            // Process children
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                calculateBoneTransforms(node->mChildren[i], globalTransform);
            }
        }
        
        // ==================== INTERPOLATION HELPERS ====================
        aiVector3D interpolatePosition(const BoneAnimation& channel) {
            if (channel.positionKeys.size() == 1) {
                return channel.positionKeys[0].value;
            }
            
            for (size_t i = 0; i < channel.positionKeys.size() - 1; i++) {
                if (animationTime < channel.positionKeys[i + 1].time) {
                    float t = (float)(animationTime - channel.positionKeys[i].time) /
                              (float)(channel.positionKeys[i + 1].time - channel.positionKeys[i].time);
                    aiVector3D start = channel.positionKeys[i].value;
                    aiVector3D end = channel.positionKeys[i + 1].value;
                    return start + t * (end - start);
                }
            }
            return channel.positionKeys.back().value;
        }
        
        aiQuaternion interpolateRotation(const BoneAnimation& channel) {
            if (channel.rotationKeys.size() == 1) {
                return channel.rotationKeys[0].value;
            }
            
            for (size_t i = 0; i < channel.rotationKeys.size() - 1; i++) {
                if (animationTime < channel.rotationKeys[i + 1].time) {
                    float t = (float)(animationTime - channel.rotationKeys[i].time) /
                              (float)(channel.rotationKeys[i + 1].time - channel.rotationKeys[i].time);
                    aiQuaternion result;
                    aiQuaternion::Interpolate(result, channel.rotationKeys[i].value, 
                                              channel.rotationKeys[i + 1].value, t);
                    return result.Normalize();
                }
            }
            return channel.rotationKeys.back().value;
        }
        
        aiVector3D interpolateScale(const BoneAnimation& channel) {
            if (channel.scaleKeys.size() == 1) {
                return channel.scaleKeys[0].value;
            }
            
            for (size_t i = 0; i < channel.scaleKeys.size() - 1; i++) {
                if (animationTime < channel.scaleKeys[i + 1].time) {
                    float t = (float)(animationTime - channel.scaleKeys[i].time) /
                              (float)(channel.scaleKeys[i + 1].time - channel.scaleKeys[i].time);
                    aiVector3D start = channel.scaleKeys[i].value;
                    aiVector3D end = channel.scaleKeys[i + 1].value;
                    return start + t * (end - start);
                }
            }
            return channel.scaleKeys.back().value;
        }
        
        // ==================== DRAW WITH IMMEDIATE MODE ====================
        void draw(float scale = 1.0f) {
            if (!loaded) {
                printf("Model::draw called but not loaded!\n");
                return;
            }
            
            static int drawCount = 0;
            static bool printedAllMeshBounds = false;
            drawCount++;
            
            // Print bounds for ALL meshes once
            if (!printedAllMeshBounds && !meshes.empty()) {
                printf("\n=== ALL MESH BOUNDS (before scale) ===\n");
                for (size_t mi = 0; mi < meshes.size(); mi++) {
                    const auto& mesh = meshes[mi];
                    if (mesh.vertices.empty()) {
                        printf("  Mesh[%d]: EMPTY (0 vertices)\n", (int)mi);
                        continue;
                    }
                    float minX=99999, maxX=-99999, minY=99999, maxY=-99999, minZ=99999, maxZ=-99999;
                    for (const auto& vert : mesh.vertices) {
                        if (vert.position[0] < minX) minX = vert.position[0];
                        if (vert.position[0] > maxX) maxX = vert.position[0];
                        if (vert.position[1] < minY) minY = vert.position[1];
                        if (vert.position[1] > maxY) maxY = vert.position[1];
                        if (vert.position[2] < minZ) minZ = vert.position[2];
                        if (vert.position[2] > maxZ) maxZ = vert.position[2];
                    }
                    printf("  Mesh[%d]: %d verts, %d tris | X[%.1f,%.1f] Y[%.1f,%.1f] Z[%.1f,%.1f]\n",
                           (int)mi, (int)mesh.vertices.size(), (int)(mesh.indices.size()/3), minX, maxX, minY, maxY, minZ, maxZ);
                }
                printf("======================================\n\n");
                fflush(stdout);
                printedAllMeshBounds = true;
            }
            
            if (drawCount % 300 == 1) {
                printf("Model::draw: %d meshes, scale=%.3f\n", (int)meshes.size(), scale);
                fflush(stdout);
            }
            
            glPushMatrix();
            glScalef(scale, scale, scale);
            
            for (const auto& mesh : meshes) {
                drawMesh(mesh);
            }
            
            glPopMatrix();
        }
        
        void drawMesh(const MeshData& mesh) {
            // Enable texture if available
            if (mesh.hasTexture && mesh.textureID != 0) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, mesh.textureID);
            }
            
            // Draw mesh with skeletal animation (bone transforms applied to vertices)
            glBegin(GL_TRIANGLES);
            
            for (size_t i = 0; i < mesh.indices.size(); i++) {
                unsigned int index = mesh.indices[i];
                const AnimVertex& v = mesh.vertices[index];
                
                // Apply bone transforms (skeletal animation)
                float skinnedPos[3] = {0, 0, 0};
                float skinnedNorm[3] = {0, 0, 0};
                float totalWeight = 0.0f;
                
                // Sum contributions from all bones affecting this vertex
                for (int b = 0; b < 4; b++) {
                    float weight = v.boneWeights[b];
                    if (weight > 0.0f) {
                        int boneID = v.boneIDs[b];
                        if (boneID >= 0 && boneID < (int)boneTransforms.size()) {
                            const aiMatrix4x4& m = boneTransforms[boneID];
                            
                            // Transform position
                            float px = m.a1 * v.position[0] + m.a2 * v.position[1] + m.a3 * v.position[2] + m.a4;
                            float py = m.b1 * v.position[0] + m.b2 * v.position[1] + m.b3 * v.position[2] + m.b4;
                            float pz = m.c1 * v.position[0] + m.c2 * v.position[1] + m.c3 * v.position[2] + m.c4;
                            
                            skinnedPos[0] += px * weight;
                            skinnedPos[1] += py * weight;
                            skinnedPos[2] += pz * weight;
                            
                            // Transform normal (no translation, just rotation)
                            float nx = m.a1 * v.normal[0] + m.a2 * v.normal[1] + m.a3 * v.normal[2];
                            float ny = m.b1 * v.normal[0] + m.b2 * v.normal[1] + m.b3 * v.normal[2];
                            float nz = m.c1 * v.normal[0] + m.c2 * v.normal[1] + m.c3 * v.normal[2];
                            
                            skinnedNorm[0] += nx * weight;
                            skinnedNorm[1] += ny * weight;
                            skinnedNorm[2] += nz * weight;
                            
                            totalWeight += weight;
                        }
                    }
                }
                
                // If no bones affect this vertex, use original position
                float pos[3], norm[3];
                if (totalWeight < 0.001f) {
                    pos[0] = v.position[0];
                    pos[1] = v.position[1];
                    pos[2] = v.position[2];
                    norm[0] = v.normal[0];
                    norm[1] = v.normal[1];
                    norm[2] = v.normal[2];
                } else {
                    pos[0] = skinnedPos[0];
                    pos[1] = skinnedPos[1];
                    pos[2] = skinnedPos[2];
                    norm[0] = skinnedNorm[0];
                    norm[1] = skinnedNorm[1];
                    norm[2] = skinnedNorm[2];
                }
                
                // Normalize the normal vector for proper lighting
                float len = sqrt(norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2]);
                if (len > 0.0001f) {
                    norm[0] /= len;
                    norm[1] /= len;
                    norm[2] /= len;
                }
                
                glTexCoord2f(v.texCoords[0], v.texCoords[1]);
                glNormal3fv(norm);
                glVertex3fv(pos);
            }
            
            glEnd();
            
            // Disable texture after drawing
            if (mesh.hasTexture && mesh.textureID != 0) {
                glDisable(GL_TEXTURE_2D);
            }
        }
        
        // ==================== CLEANUP ====================
        void cleanup() {
            meshes.clear();
            bones.clear();
            boneMapping.clear();
            animations.clear();
            boneTransforms.clear();
            loaded = false;
        }
    };

    // ==================== GLOBAL SOLDIER MODEL ====================
    static Model soldierModel;
    static bool soldierLoaded = false;
    
    // Animation states
    enum SoldierAnimation {
        ANIM_IDLE = 0,
        ANIM_WALK,
        ANIM_RUN,
        ANIM_SHOOT,
        ANIM_RELOAD,
        ANIM_DEATH
    };
    
    inline bool loadSoldierModel() {
        if (soldierLoaded) return true;
        
        printf("\n=== ATTEMPTING TO LOAD SOLDIER MODEL ===\n");
        
        char cwd[1024];
        if (_getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current Working Directory: %s\n", cwd);
        }
        
        const char* pathList[] = {
            "template/res/pro-rifle-pack/solider-textured.fbx",
            "../res/pro-rifle-pack/solider-textured.fbx",
            "template/res/pro-rifle-pack/idle.fbx",
            "../res/pro-rifle-pack/idle.fbx"
        };
        int numPaths = sizeof(pathList) / sizeof(pathList[0]);
        
        for (int i = 0; i < numPaths; i++) {
            printf("Trying path: %s\n", pathList[i]);
            fflush(stdout);
            if (soldierModel.load(pathList[i])) {
                soldierLoaded = true;
                printf("SUCCESS! Model loaded from: %s\n", pathList[i]);
                fflush(stdout);
                return true;
            }
        }
        
        printf("\n!!! FAILED TO LOAD SOLDIER MODEL FROM ANY PATH !!!\n");
        printf("Falling back to procedural player model.\n\n");
        fflush(stdout);
        return false;
    }
    
    inline void updateSoldier(float deltaTime) {
        if (soldierLoaded) {
            soldierModel.update(deltaTime);
        }
    }
    
    inline void setSoldierAnimation(SoldierAnimation anim) {
        if (!soldierLoaded) return;
        
        // Mixamo animation names vary - try common patterns
        // The model will log available animations on load
        
        switch (anim) {
            case ANIM_IDLE:
                // Try common idle animation names
                if (!soldierModel.setAnimationByName("idle")) {
                    if (!soldierModel.setAnimationByName("Idle")) {
                        if (!soldierModel.setAnimationByName("idle aiming")) {
                            soldierModel.setAnimation(0); // Fallback to first animation
                        }
                    }
                }
                soldierModel.looping = true;
                soldierModel.animationSpeed = 1.0f;
                break;
            case ANIM_WALK:
                if (!soldierModel.setAnimationByName("walk_forward")) {
                    if (!soldierModel.setAnimationByName("Walking")) {
                        soldierModel.setAnimation(0);
                    }
                }
                soldierModel.looping = true;
                soldierModel.animationSpeed = 1.0f;
                break;
            case ANIM_RUN:
                if (!soldierModel.setAnimationByName("run_forward")) {
                    if (!soldierModel.setAnimationByName("Running")) {
                        soldierModel.setAnimation(0);
                    }
                }
                soldierModel.animationSpeed = 1.0f;
                soldierModel.looping = true;
                break;
            case ANIM_SHOOT:
                if (!soldierModel.setAnimationByName("idle aiming")) {
                    if (!soldierModel.setAnimationByName("Aiming")) {
                        soldierModel.setAnimation(0);
                    }
                }
                soldierModel.looping = true;
                soldierModel.animationSpeed = 1.0f;
                break;
            case ANIM_RELOAD:
                soldierModel.setAnimation(0);
                soldierModel.looping = false;
                soldierModel.animationSpeed = 1.0f;
                break;
            case ANIM_DEATH:
                if (!soldierModel.setAnimationByName("death_front")) {
                    if (!soldierModel.setAnimationByName("Death")) {
                        soldierModel.setAnimation(0);
                    }
                }
                soldierModel.looping = false;
                soldierModel.animationSpeed = 1.0f;
                break;
        }
    }
    
    inline void drawSoldier(float rotationY, float scale = 1.0f) {
        if (!soldierLoaded) {
            // Don't spam console when model isn't loaded
            static bool warned = false;
            if (!warned) {
                printf("drawSoldier: Model not loaded, using fallback\n");
                warned = true;
            }
            return;
        }
        
        static int frameCount = 0;
        static bool printedBounds = false;
        static float modelMinY = 0.0f;
        frameCount++;
        
        // Print bounds once and calculate offset
        if (!printedBounds && !soldierModel.meshes.empty()) {
            float minX=99999, maxX=-99999, minY=99999, maxY=-99999, minZ=99999, maxZ=-99999;
            for (const auto& mesh : soldierModel.meshes) {
                for (const auto& v : mesh.vertices) {
                    if (v.position[0] < minX) minX = v.position[0];
                    if (v.position[0] > maxX) maxX = v.position[0];
                    if (v.position[1] < minY) minY = v.position[1];
                    if (v.position[1] > maxY) maxY = v.position[1];
                    if (v.position[2] < minZ) minZ = v.position[2];
                    if (v.position[2] > maxZ) maxZ = v.position[2];
                }
            }
            modelMinY = minY;
            printf("\n=== MODEL BOUNDS (before scale) ===\n");
            printf("X: [%.2f to %.2f] width=%.2f\n", minX, maxX, maxX-minX);
            printf("Y: [%.2f to %.2f] height=%.2f\n", minY, maxY, maxY-minY);
            printf("Z: [%.2f to %.2f] depth=%.2f\n", minZ, maxZ, maxZ-minZ);
            printf("===================================\n\n");
            fflush(stdout);
            printedBounds = true;
        }
        
        if (frameCount % 300 == 1) {
            printf("drawSoldier: Drawing %d meshes at scale %.3f\n", (int)soldierModel.meshes.size(), scale);
            fflush(stdout);
        }
        
        glPushMatrix();
        
        // Enable lighting for proper shading
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        
        // Enable color material so glColor affects lit surfaces
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        
        // Set material properties for soldier
        float specular[] = {0.3f, 0.3f, 0.3f, 1.0f};
        float shininess = 20.0f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        
        // White color so textures show through properly
        glColor3f(1.0f, 1.0f, 1.0f);
        
        // Mixamo model is ~180 units tall, we want ~1.8 units
        float modelHeight = 180.0f;
        float finalScale = (1.8f / modelHeight) * scale;
        
        // Apply player rotation for facing direction
        glRotatef(rotationY + 180.0f, 0, 1, 0);
        
        soldierModel.draw(finalScale);
        
        glPopMatrix();
    }
    
    inline void drawSoldierFirstPerson(float scale = 1.0f) {
        if (!soldierLoaded) return;
        
        glPushMatrix();
        // Position for first person view - only arms and weapon visible
        glTranslatef(0.2f, -0.3f, -0.4f);
        glRotatef(180, 0, 1, 0); // Face forward
        
        glColor3f(0.3f, 0.35f, 0.25f);
        soldierModel.draw(scale * 0.8f);
        glPopMatrix();
    }
    
    inline int getAnimationCount() {
        return soldierLoaded ? (int)soldierModel.animations.size() : 0;
    }
    
    inline std::string getAnimationName(int index) {
        if (soldierLoaded && index >= 0 && index < (int)soldierModel.animations.size()) {
            return soldierModel.animations[index].name;
        }
        return "";
    }
}

#endif // ANIMATED_MODEL_H
