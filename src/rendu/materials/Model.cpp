#include "Model.h"
#include <iostream>

Model::Model(const std::string& path) {
    loadModel(path);
    setupMesh();
}

void Model::loadModel(const std::string& path) {
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    bool ret = loader.LoadASCIIFromFile(&_model, &err, &warn, path);

    if (!warn.empty()) std::cout << "Warn: " << warn << std::endl;
    if (!err.empty()) std::cerr << "Err: " << err << std::endl;
    if (!ret) throw std::runtime_error("Échec du chargement glTF");
}

void Model::setupMesh() {
    const tinygltf::Primitive& primitive = _model.meshes[0].primitives[0];

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // --- 1. ATTRIBUT : POSITIONS  ---
    const tinygltf::Accessor& posAcc = _model.accessors[primitive.attributes.at("POSITION")];
    const tinygltf::BufferView& posView = _model.bufferViews[posAcc.bufferView];
    const tinygltf::Buffer& posBuffer = _model.buffers[posView.buffer];

    glGenBuffers(1, &VBO_pos);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glBufferData(GL_ARRAY_BUFFER, posAcc.count * 3 * sizeof(float), 
                 &posBuffer.data[posView.byteOffset + posAcc.byteOffset], GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --- 2. ATTRIBUT : NORMALES  ---
    const tinygltf::Accessor& normAcc = _model.accessors[primitive.attributes.at("NORMAL")];
    const tinygltf::BufferView& normView = _model.bufferViews[normAcc.bufferView];
    const tinygltf::Buffer& normBuffer = _model.buffers[normView.buffer];

    glGenBuffers(1, &VBO_norm);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_norm);
    glBufferData(GL_ARRAY_BUFFER, normAcc.count * 3 * sizeof(float), 
                 &normBuffer.data[normView.byteOffset + normAcc.byteOffset], GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // --- 3. INDICES (EBO) ---
    const tinygltf::Accessor& idxAcc = _model.accessors[primitive.indices];
    const tinygltf::BufferView& idxView = _model.bufferViews[idxAcc.bufferView];
    const tinygltf::Buffer& idxBuffer = _model.buffers[idxView.buffer];

    this->indexCount = idxAcc.count;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxAcc.count * sizeof(unsigned short), 
                 &idxBuffer.data[idxView.byteOffset + idxAcc.byteOffset], GL_STATIC_DRAW);

    glBindVertexArray(0); // On détache le VAO
}

void Model::Draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (int)indexCount, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}