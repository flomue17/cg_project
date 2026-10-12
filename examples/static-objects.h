#pragma once

#include <cppgl.h>
#include "dataLoader.h"
#include "texture.h"
#include <memory>

class Skysphere {
public:
    Skysphere();

    void init(const std::string &scene);

    void update();

    void update_forwards();

    void update_backwards();

    void update_forwards_mask();

    void update_backwards_mask();

    void update_backwards_mask_weighting();

    void update_backwards_mask_confidence(int indexPicture);

    void update_buckets();

    void update_k_means();

    void filterConfidence();

    void resetTextures();

    void draw();

    glm::mat4 model;


    bool useMasks = false;
    //
    // float confidenceThreshold = 0.05f;
    // float confidenceIncreaseFactor = 0.5f;
    // float confidenceDecreaseFactor = 0.25f;
    // float initialConfidence = 0;
    // float filterConfidenceThreshold = 0.f;

    float confidenceThreshold = 0.345f;
    float confidenceIncreaseFactor = 0.187f;
    float confidenceDecreaseFactor = 0.187f;
    float initialConfidence = 0.2;
    float filterConfidenceThreshold = 0.f;

    static dataLoader loader;
    static cppgl::Texture2D envMap;
    static cppgl::Texture2D filteredEnvMap;

    static cppgl::Texture2D groundTruth;
    static cppgl::Texture2D counterWritten;
    static cppgl::Texture2D Deviation;
    static cppgl::Texture2D mean;
    static cppgl::Texture2D confidence;
    static cppgl::Texture2D k_means;


    static cppgl::Texture3D buckets;
    static cppgl::Texture3D bucketsCounter;
};

extern std::shared_ptr<Skysphere> skysphere;
