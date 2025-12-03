/**
 * DOOMERS - Assimp Model Loader
 * 
 * Loads FBX, GLTF, and other formats via Assimp library
 * Supports:
 * - Static meshes
 * - Skeletal animation
 * - Multiple animations per model
 * - Animation blending
 * 
 * For OpenGL 2.x fixed-function pipeline (CPU skinning)
 */

#pragma once

#include "Core.hpp"
#include "Math.hpp"
#include "ResourceManager.hpp"

// Assimp headers
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <GL/gl.h>

namespace Doomers {

// Helper clamp function
template<typename T>
inline T clamp(T val, T minVal, T maxVal) {
    return std::max(minVal, std::min(val, maxVal));
}

// ============================================================================
// Bone Data
// ============================================================================
struct BoneInfo {
    int id;                      // Index in bone array
    std::string name;
    Math::Matrix4 offsetMatrix;  // Transforms from mesh space to bone space
};

// ============================================================================
// Vertex with Bone Weights (for skinning)
// ============================================================================
struct SkinnedVertex {
    Math::Vector3 position;
    Math::Vector3 normal;
    Math::Vector2 texCoord;
    
    // Bone influences (up to 4 bones per vertex)
    int boneIds[4] = {-1, -1, -1, -1};
    float boneWeights[4] = {0, 0, 0, 0};
    
    void addBoneInfluence(int boneId, float weight) {
        for (int i = 0; i < 4; ++i) {
            if (boneIds[i] < 0) {
                boneIds[i] = boneId;
                boneWeights[i] = weight;
                return;
            }
        }
        // Replace smallest weight if all slots full
        int minIdx = 0;
        for (int i = 1; i < 4; ++i) {
            if (boneWeights[i] < boneWeights[minIdx]) {
                minIdx = i;
            }
        }
        if (weight > boneWeights[minIdx]) {
            boneIds[minIdx] = boneId;
            boneWeights[minIdx] = weight;
        }
    }
    
    void normalizeWeights() {
        float total = 0;
        for (int i = 0; i < 4; ++i) {
            if (boneIds[i] >= 0) total += boneWeights[i];
        }
        if (total > 0.001f) {
            for (int i = 0; i < 4; ++i) {
                boneWeights[i] /= total;
            }
        }
    }
};

// ============================================================================
// Animation Keyframes
// ============================================================================
struct VectorKey {
    float time;
    Math::Vector3 value;
};

struct QuatKey {
    float time;
    float x, y, z, w;  // Quaternion
};

struct BoneAnimation {
    std::string boneName;
    std::vector<VectorKey> positionKeys;
    std::vector<QuatKey> rotationKeys;
    std::vector<VectorKey> scaleKeys;
    
    Math::Vector3 interpolatePosition(float time) const {
        if (positionKeys.empty()) return Math::Vector3(0, 0, 0);
        if (positionKeys.size() == 1) return positionKeys[0].value;
        
        // Find keyframes
        size_t idx = 0;
        for (size_t i = 0; i < positionKeys.size() - 1; ++i) {
            if (time < positionKeys[i + 1].time) {
                idx = i;
                break;
            }
            idx = i;
        }
        
        size_t nextIdx = (idx + 1) % positionKeys.size();
        float dt = positionKeys[nextIdx].time - positionKeys[idx].time;
        float t = (dt > 0.0001f) ? (time - positionKeys[idx].time) / dt : 0;
        t = clamp(t, 0.0f, 1.0f);
        
        return positionKeys[idx].value * (1.0f - t) + positionKeys[nextIdx].value * t;
    }
    
    void interpolateRotation(float time, float& qx, float& qy, float& qz, float& qw) const {
        if (rotationKeys.empty()) {
            qx = qy = qz = 0; qw = 1;
            return;
        }
        if (rotationKeys.size() == 1) {
            qx = rotationKeys[0].x;
            qy = rotationKeys[0].y;
            qz = rotationKeys[0].z;
            qw = rotationKeys[0].w;
            return;
        }
        
        size_t idx = 0;
        for (size_t i = 0; i < rotationKeys.size() - 1; ++i) {
            if (time < rotationKeys[i + 1].time) {
                idx = i;
                break;
            }
            idx = i;
        }
        
        size_t nextIdx = (idx + 1) % rotationKeys.size();
        float dt = rotationKeys[nextIdx].time - rotationKeys[idx].time;
        float t = (dt > 0.0001f) ? (time - rotationKeys[idx].time) / dt : 0;
        t = clamp(t, 0.0f, 1.0f);
        
        // Simple linear interpolation (SLERP would be better)
        qx = rotationKeys[idx].x * (1 - t) + rotationKeys[nextIdx].x * t;
        qy = rotationKeys[idx].y * (1 - t) + rotationKeys[nextIdx].y * t;
        qz = rotationKeys[idx].z * (1 - t) + rotationKeys[nextIdx].z * t;
        qw = rotationKeys[idx].w * (1 - t) + rotationKeys[nextIdx].w * t;
        
        // Normalize quaternion
        float len = sqrtf(qx*qx + qy*qy + qz*qz + qw*qw);
        if (len > 0.0001f) {
            qx /= len; qy /= len; qz /= len; qw /= len;
        }
    }
    
    Math::Vector3 interpolateScale(float time) const {
        if (scaleKeys.empty()) return Math::Vector3(1, 1, 1);
        if (scaleKeys.size() == 1) return scaleKeys[0].value;
        
        size_t idx = 0;
        for (size_t i = 0; i < scaleKeys.size() - 1; ++i) {
            if (time < scaleKeys[i + 1].time) {
                idx = i;
                break;
            }
            idx = i;
        }
        
        size_t nextIdx = (idx + 1) % scaleKeys.size();
        float dt = scaleKeys[nextIdx].time - scaleKeys[idx].time;
        float t = (dt > 0.0001f) ? (time - scaleKeys[idx].time) / dt : 0;
        t = clamp(t, 0.0f, 1.0f);
        
        return scaleKeys[idx].value * (1.0f - t) + scaleKeys[nextIdx].value * t;
    }
};

// ============================================================================
// Animation Clip
// ============================================================================
struct AnimationClip {
    std::string name;
    float duration;         // In ticks
    float ticksPerSecond;
    std::vector<BoneAnimation> channels;
    
    float getDurationSeconds() const {
        return (ticksPerSecond > 0) ? duration / ticksPerSecond : duration;
    }
    
    BoneAnimation* findChannel(const std::string& boneName) {
        for (auto& ch : channels) {
            if (ch.boneName == boneName) return &ch;
        }
        return nullptr;
    }
};

// ============================================================================
// Skeleton Node (Bone Hierarchy)
// ============================================================================
struct SkeletonNode {
    std::string name;
    Math::Matrix4 transform;        // Local transform
    int boneIndex = -1;             // -1 if not a bone
    std::vector<SkeletonNode> children;
};

// ============================================================================
// Animated Model
// ============================================================================
class AnimatedModel {
public:
    // Mesh data
    std::vector<SkinnedVertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int textureId = 0;
    
    // Skeleton
    SkeletonNode rootNode;
    std::vector<BoneInfo> bones;
    std::map<std::string, int> boneNameToIndex;
    Math::Matrix4 globalInverseTransform;
    
    // Animations
    std::vector<AnimationClip> animations;
    int currentAnimation = 0;
    float animationTime = 0;
    bool looping = true;
    
    // Final bone transforms (updated each frame)
    std::vector<Math::Matrix4> boneTransforms;
    
    // Transformed vertices (for CPU skinning in OpenGL 2.x)
    std::vector<Math::Vector3> transformedPositions;
    std::vector<Math::Vector3> transformedNormals;
    
    AnimatedModel() {}
    
    bool hasAnimations() const { return !animations.empty(); }
    int getAnimationCount() const { return (int)animations.size(); }
    
    std::string getAnimationName(int index) const {
        if (index >= 0 && index < (int)animations.size()) {
            return animations[index].name;
        }
        return "";
    }
    
    void setAnimation(int index) {
        if (index >= 0 && index < (int)animations.size()) {
            currentAnimation = index;
            animationTime = 0;
        }
    }
    
    void setAnimationByName(const std::string& name) {
        for (int i = 0; i < (int)animations.size(); ++i) {
            if (animations[i].name == name) {
                setAnimation(i);
                return;
            }
        }
    }
    
    void update(float deltaTime) {
        if (animations.empty() || currentAnimation < 0) return;
        
        AnimationClip& anim = animations[currentAnimation];
        
        // Advance time
        float tps = (anim.ticksPerSecond > 0) ? anim.ticksPerSecond : 25.0f;
        animationTime += deltaTime * tps;
        
        if (looping) {
            animationTime = fmodf(animationTime, anim.duration);
        } else if (animationTime > anim.duration) {
            animationTime = anim.duration;
        }
        
        // Calculate bone transforms
        calculateBoneTransforms(rootNode, Math::Matrix4::Identity());
        
        // Apply skinning (CPU)
        applySkinning();
    }
    
    void calculateBoneTransforms(SkeletonNode& node, const Math::Matrix4& parentTransform) {
        Math::Matrix4 nodeTransform = node.transform;
        
        // If this node has animation, use animated transform
        if (currentAnimation >= 0 && currentAnimation < (int)animations.size()) {
            AnimationClip& anim = animations[currentAnimation];
            BoneAnimation* channel = anim.findChannel(node.name);
            
            if (channel) {
                Math::Vector3 pos = channel->interpolatePosition(animationTime);
                Math::Vector3 scale = channel->interpolateScale(animationTime);
                float qx, qy, qz, qw;
                channel->interpolateRotation(animationTime, qx, qy, qz, qw);
                
                // Build transform from position, rotation, scale
                nodeTransform = buildTransformMatrix(pos, qx, qy, qz, qw, scale);
            }
        }
        
        Math::Matrix4 globalTransform = parentTransform * nodeTransform;
        
        // If this is a bone, store final transform
        if (node.boneIndex >= 0 && node.boneIndex < (int)bones.size()) {
            boneTransforms[node.boneIndex] = 
                globalInverseTransform * globalTransform * bones[node.boneIndex].offsetMatrix;
        }
        
        // Process children
        for (auto& child : node.children) {
            calculateBoneTransforms(child, globalTransform);
        }
    }
    
    Math::Matrix4 buildTransformMatrix(const Math::Vector3& pos, 
                                        float qx, float qy, float qz, float qw,
                                        const Math::Vector3& scale) {
        // Quaternion to rotation matrix
        float xx = qx * qx, yy = qy * qy, zz = qz * qz;
        float xy = qx * qy, xz = qx * qz, yz = qy * qz;
        float wx = qw * qx, wy = qw * qy, wz = qw * qz;
        
        Math::Matrix4 m;
        m.m[0] = (1 - 2*(yy + zz)) * scale.x;
        m.m[1] = 2*(xy + wz) * scale.x;
        m.m[2] = 2*(xz - wy) * scale.x;
        m.m[3] = 0;
        
        m.m[4] = 2*(xy - wz) * scale.y;
        m.m[5] = (1 - 2*(xx + zz)) * scale.y;
        m.m[6] = 2*(yz + wx) * scale.y;
        m.m[7] = 0;
        
        m.m[8] = 2*(xz + wy) * scale.z;
        m.m[9] = 2*(yz - wx) * scale.z;
        m.m[10] = (1 - 2*(xx + yy)) * scale.z;
        m.m[11] = 0;
        
        m.m[12] = pos.x;
        m.m[13] = pos.y;
        m.m[14] = pos.z;
        m.m[15] = 1;
        
        return m;
    }
    
    void applySkinning() {
        transformedPositions.resize(vertices.size());
        transformedNormals.resize(vertices.size());
        
        for (size_t i = 0; i < vertices.size(); ++i) {
            const SkinnedVertex& v = vertices[i];
            Math::Vector3 pos(0, 0, 0);
            Math::Vector3 norm(0, 0, 0);
            
            for (int j = 0; j < 4; ++j) {
                if (v.boneIds[j] >= 0 && v.boneIds[j] < (int)boneTransforms.size()) {
                    float weight = v.boneWeights[j];
                    if (weight > 0.0001f) {
                        Math::Matrix4& bt = boneTransforms[v.boneIds[j]];
                        pos = pos + bt.transformPoint(v.position) * weight;
                        norm = norm + bt.transformDirection(v.normal) * weight;
                    }
                }
            }
            
            // If no bones affected this vertex, use original position
            if (v.boneIds[0] < 0) {
                pos = v.position;
                norm = v.normal;
            }
            
            transformedPositions[i] = pos;
            transformedNormals[i] = norm.normalized();
        }
    }
    
    void draw() {
        if (textureId > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textureId);
        } else {
            glDisable(GL_TEXTURE_2D);
        }
        
        glColor3f(1, 1, 1);
        
        if (!transformedPositions.empty()) {
            // Draw with skinned vertices
            glBegin(GL_TRIANGLES);
            for (size_t i = 0; i < indices.size(); ++i) {
                unsigned int idx = indices[i];
                if (idx < transformedPositions.size()) {
                    glNormal3f(transformedNormals[idx].x, 
                              transformedNormals[idx].y, 
                              transformedNormals[idx].z);
                    glTexCoord2f(vertices[idx].texCoord.x, vertices[idx].texCoord.y);
                    glVertex3f(transformedPositions[idx].x, 
                              transformedPositions[idx].y, 
                              transformedPositions[idx].z);
                }
            }
            glEnd();
        } else {
            // Draw static (no animation)
            glBegin(GL_TRIANGLES);
            for (size_t i = 0; i < indices.size(); ++i) {
                unsigned int idx = indices[i];
                if (idx < vertices.size()) {
                    const SkinnedVertex& v = vertices[idx];
                    glNormal3f(v.normal.x, v.normal.y, v.normal.z);
                    glTexCoord2f(v.texCoord.x, v.texCoord.y);
                    glVertex3f(v.position.x, v.position.y, v.position.z);
                }
            }
            glEnd();
        }
    }
    
    void drawStatic() {
        // Draw without animation (bind pose)
        if (textureId > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textureId);
        } else {
            glDisable(GL_TEXTURE_2D);
        }
        
        glColor3f(1, 1, 1);
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < indices.size(); ++i) {
            unsigned int idx = indices[i];
            if (idx < vertices.size()) {
                const SkinnedVertex& v = vertices[idx];
                glNormal3f(v.normal.x, v.normal.y, v.normal.z);
                glTexCoord2f(v.texCoord.x, v.texCoord.y);
                glVertex3f(v.position.x, v.position.y, v.position.z);
            }
        }
        glEnd();
    }
};

// ============================================================================
// Assimp Model Loader
// ============================================================================
class AssimpLoader {
public:
    static AnimatedModel* loadModel(const std::string& path) {
        Assimp::Importer importer;
        
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_LimitBoneWeights |
            aiProcess_JoinIdenticalVertices
        );
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
            return nullptr;
        }
        
        AnimatedModel* model = new AnimatedModel();
        
        // Get global inverse transform
        model->globalInverseTransform = convertMatrix(scene->mRootNode->mTransformation);
        model->globalInverseTransform = model->globalInverseTransform.inverted();
        
        // Process all meshes
        for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
            processMesh(scene->mMeshes[m], scene, model);
        }
        
        // Normalize bone weights
        for (auto& v : model->vertices) {
            v.normalizeWeights();
        }
        
        // Build skeleton hierarchy
        model->rootNode = processNode(scene->mRootNode, model);
        
        // Initialize bone transforms array
        model->boneTransforms.resize(model->bones.size(), Math::Matrix4::Identity());
        
        // Load animations
        for (unsigned int a = 0; a < scene->mNumAnimations; ++a) {
            aiAnimation* anim = scene->mAnimations[a];
            
            AnimationClip clip;
            clip.name = anim->mName.C_Str();
            clip.duration = (float)anim->mDuration;
            clip.ticksPerSecond = (float)anim->mTicksPerSecond;
            
            for (unsigned int c = 0; c < anim->mNumChannels; ++c) {
                aiNodeAnim* channel = anim->mChannels[c];
                
                BoneAnimation boneAnim;
                boneAnim.boneName = channel->mNodeName.C_Str();
                
                // Position keys
                for (unsigned int k = 0; k < channel->mNumPositionKeys; ++k) {
                    VectorKey key;
                    key.time = (float)channel->mPositionKeys[k].mTime;
                    key.value.x = channel->mPositionKeys[k].mValue.x;
                    key.value.y = channel->mPositionKeys[k].mValue.y;
                    key.value.z = channel->mPositionKeys[k].mValue.z;
                    boneAnim.positionKeys.push_back(key);
                }
                
                // Rotation keys
                for (unsigned int k = 0; k < channel->mNumRotationKeys; ++k) {
                    QuatKey key;
                    key.time = (float)channel->mRotationKeys[k].mTime;
                    key.x = channel->mRotationKeys[k].mValue.x;
                    key.y = channel->mRotationKeys[k].mValue.y;
                    key.z = channel->mRotationKeys[k].mValue.z;
                    key.w = channel->mRotationKeys[k].mValue.w;
                    boneAnim.rotationKeys.push_back(key);
                }
                
                // Scale keys
                for (unsigned int k = 0; k < channel->mNumScalingKeys; ++k) {
                    VectorKey key;
                    key.time = (float)channel->mScalingKeys[k].mTime;
                    key.value.x = channel->mScalingKeys[k].mValue.x;
                    key.value.y = channel->mScalingKeys[k].mValue.y;
                    key.value.z = channel->mScalingKeys[k].mValue.z;
                    boneAnim.scaleKeys.push_back(key);
                }
                
                clip.channels.push_back(boneAnim);
            }
            
            model->animations.push_back(clip);
        }
        
        // Load texture if material has one
        if (scene->mNumMaterials > 0) {
            aiMaterial* mat = scene->mMaterials[0];
            aiString texPath;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
                // Get directory from model path
                std::string dir = path.substr(0, path.find_last_of("/\\") + 1);
                std::string fullTexPath = dir + texPath.C_Str();
                
                // Load texture (using existing ResourceManager)
                model->textureId = ResourceManager::instance().loadTexture(fullTexPath);
            }
        }
        
        std::cout << "Loaded model: " << path << std::endl;
        std::cout << "  Vertices: " << model->vertices.size() << std::endl;
        std::cout << "  Bones: " << model->bones.size() << std::endl;
        std::cout << "  Animations: " << model->animations.size() << std::endl;
        
        return model;
    }
    
private:
    static void processMesh(aiMesh* mesh, const aiScene* scene, AnimatedModel* model) {
        unsigned int baseVertex = (unsigned int)model->vertices.size();
        
        // Extract vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            SkinnedVertex vertex;
            
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;
            
            if (mesh->HasNormals()) {
                vertex.normal.x = mesh->mNormals[i].x;
                vertex.normal.y = mesh->mNormals[i].y;
                vertex.normal.z = mesh->mNormals[i].z;
            }
            
            if (mesh->HasTextureCoords(0)) {
                vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
                vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
            }
            
            model->vertices.push_back(vertex);
        }
        
        // Extract indices
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                model->indices.push_back(baseVertex + face.mIndices[j]);
            }
        }
        
        // Extract bones
        for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
            aiBone* bone = mesh->mBones[b];
            std::string boneName = bone->mName.C_Str();
            
            int boneIndex;
            if (model->boneNameToIndex.find(boneName) == model->boneNameToIndex.end()) {
                // New bone
                boneIndex = (int)model->bones.size();
                BoneInfo boneInfo;
                boneInfo.id = boneIndex;
                boneInfo.name = boneName;
                boneInfo.offsetMatrix = convertMatrix(bone->mOffsetMatrix);
                model->bones.push_back(boneInfo);
                model->boneNameToIndex[boneName] = boneIndex;
            } else {
                boneIndex = model->boneNameToIndex[boneName];
            }
            
            // Add bone weights to vertices
            for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
                unsigned int vertexId = baseVertex + bone->mWeights[w].mVertexId;
                float weight = bone->mWeights[w].mWeight;
                model->vertices[vertexId].addBoneInfluence(boneIndex, weight);
            }
        }
    }
    
    static SkeletonNode processNode(aiNode* node, AnimatedModel* model) {
        SkeletonNode skelNode;
        skelNode.name = node->mName.C_Str();
        skelNode.transform = convertMatrix(node->mTransformation);
        
        // Check if this node is a bone
        auto it = model->boneNameToIndex.find(skelNode.name);
        if (it != model->boneNameToIndex.end()) {
            skelNode.boneIndex = it->second;
        }
        
        // Process children
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            skelNode.children.push_back(processNode(node->mChildren[i], model));
        }
        
        return skelNode;
    }
    
    static Math::Matrix4 convertMatrix(const aiMatrix4x4& m) {
        Math::Matrix4 result;
        // Assimp is row-major, OpenGL is column-major
        result.m[0] = m.a1; result.m[4] = m.a2; result.m[8]  = m.a3; result.m[12] = m.a4;
        result.m[1] = m.b1; result.m[5] = m.b2; result.m[9]  = m.b3; result.m[13] = m.b4;
        result.m[2] = m.c1; result.m[6] = m.c2; result.m[10] = m.c3; result.m[14] = m.c4;
        result.m[3] = m.d1; result.m[7] = m.d2; result.m[11] = m.d3; result.m[15] = m.d4;
        return result;
    }
};

// ============================================================================
// Convenience function
// ============================================================================
inline AnimatedModel* LoadAnimatedModel(const std::string& path) {
    return AssimpLoader::loadModel(path);
}

} // namespace Doomers
