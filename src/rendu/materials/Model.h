#ifndef MODEL_H
#define MODEL_H

#include <tiny_gltf.h>
#include <glad/glad.h>
#include <string>
#include <vector>

class Model {
public:
    unsigned int VAO;
    size_t indexCount;

    Model(const std::string& path);
    void Draw();

private:
    tinygltf::Model _model; 
    unsigned int VBO_pos, VBO_norm,EBO;

    void loadModel(const std::string& path);
    void setupMesh();
};

#endif