#include "static-objects.h"

#include <fstream>

#include "dataLoader.h"

using namespace cppgl;
// ------------------------------------------------
// globals

// ------------------------------------------------
// prototypes
std::shared_ptr<Skysphere> skysphere = std::make_shared<Skysphere>();
Drawelement prototype;
Shader map_forward, map_backward, map_backward_mask, map_forward_mask, map_weighting, map_divide, map_squaredDeviation,
        map_standardDeviation, map_removeOutliners, map_confidence, map_buckets, map_buckets_calc, filteredConfidence,
        map_k_means;
dataLoader Skysphere::loader;
Texture2D Skysphere::groundTruth, Skysphere::envMap, Skysphere::counterWritten, Skysphere::Deviation, Skysphere::mean,
        Skysphere::confidence, Skysphere::filteredEnvMap, Skysphere::k_means;

Texture3D Skysphere::buckets, Skysphere::bucketsCounter;

std::vector<glm::vec4> extractKMeansFromStream(std::stringstream &buffer) {
    std::vector<glm::vec4> k_means_values;
    float r, g, b, a;

    std::string line;
    while (std::getline(buffer, line)) {
        std::stringstream line_stream(line);

        // Werte als vec4 (r, g, b, a) lesen
        line_stream >> r >> g >> b >> a;

        // Hinzufügen der K-Means-Werte zur Liste
        k_means_values.push_back(glm::vec4(r, g, b, a));
    }

    return k_means_values;
}

// OpenGL-Textur erstellen und die K-Means-Werte laden
GLuint createKMeansTexture(const std::vector<glm::vec4> &k_means_values) {
    GLuint textureID;

    // Textur erstellen
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Texturparameter setzen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // K-Means-Daten vorbereiten
    std::vector<float> k_means_data;
    for (const auto &color: k_means_values) {
        k_means_data.push_back(color.r);
        k_means_data.push_back(color.g);
        k_means_data.push_back(color.b);
        k_means_data.push_back(color.a);
    }

    // Daten in die Textur laden
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 8, 1, 0, GL_RGBA, GL_FLOAT, k_means_data.data());

    return textureID;
}

// HINT: you might need this for A12
static glm::vec3 polar2cartesian(float theta, float phi) {
    float sinTheta = sinf(theta);
    float cosTheta = cosf(theta);
    float sinPhi = sinf(phi);
    float cosPhi = cosf(phi);
    glm::vec3 normal = glm::vec3(-cosPhi * sinTheta,
                                 sinPhi * sinTheta,
                                 -cosTheta); // z as north pole.
    return normal;
}

void Skysphere::init(const std::string &scene) {
    // init skysphere prototype
    const uint32_t stack_count = 32;
    const uint32_t sector_count = 32;
    const float stack_step = M_PI / stack_count;
    const float sector_step = 2 * M_PI / sector_count;

    std::vector<glm::vec3> vertices = {};
    std::vector<glm::vec2> texcoords = {};

    //*2
    //calc vertices and texcoords, construct poles seperately
    //remove dummy values
    float theta_max = stack_count * stack_step;
    float phi_max = sector_count * sector_step;

    for (float theta = 0; theta < stack_count * stack_step; theta += stack_step) {
        for (float phi = 0; phi < sector_count * sector_step; phi += sector_step) //north to equator
        {
            if (theta == 0) {
                //north pole
                vertices.push_back(polar2cartesian(theta, phi));
                vertices.push_back(polar2cartesian(theta + stack_step, phi));
                vertices.push_back(polar2cartesian(theta + stack_step, phi + sector_step));

                texcoords.push_back({phi / phi_max, theta / theta_max});
                texcoords.push_back({phi / phi_max, (theta + stack_step) / theta_max});
                texcoords.push_back({(phi + sector_step) / phi_max, (theta + stack_step) / theta_max});
            } else if (theta == (stack_count - 1) * stack_step) {
                //south pole
                vertices.push_back(polar2cartesian(theta, phi));
                vertices.push_back(polar2cartesian(theta + stack_step, phi));
                vertices.push_back(polar2cartesian(theta, phi + sector_step));

                texcoords.push_back({phi / phi_max, theta / theta_max});
                texcoords.push_back({phi / phi_max, (theta + stack_step) / theta_max});
                texcoords.push_back({(phi + sector_step) / phi_max, theta / theta_max});
            } else {
                //triangle 1
                vertices.push_back(polar2cartesian(theta, phi));
                vertices.push_back(polar2cartesian(theta + stack_step, phi));
                vertices.push_back(polar2cartesian(theta + stack_step, phi + sector_step));

                texcoords.push_back({phi / phi_max, theta / theta_max});
                texcoords.push_back({phi / phi_max, (theta + stack_step) / theta_max});
                texcoords.push_back({(phi + sector_step) / phi_max, (theta + stack_step) / theta_max});


                //triangle 2
                vertices.push_back(polar2cartesian(theta + stack_step, phi + sector_step));
                vertices.push_back(polar2cartesian(theta, phi + sector_step));
                vertices.push_back(polar2cartesian(theta, phi));

                texcoords.push_back({(phi + sector_step) / phi_max, (theta + stack_step) / theta_max});
                texcoords.push_back({(phi + sector_step) / phi_max, theta / theta_max});
                texcoords.push_back({phi / phi_max, theta / theta_max});
            }
        }
    }

    auto sky_mesh = Mesh("skysphere-mesh");

    //add vertices and texcoords as vertex buffers (mesh->add_vertex_buffer())
    sky_mesh->add_vertex_buffer(GL_FLOAT, 3, vertices.size(), vertices.data());
    sky_mesh->add_vertex_buffer(GL_FLOAT, 2, texcoords.size(), texcoords.data());

    //create Materialand skysphere Texture2D
    auto sky_mat = Material("skysphere-material");


    groundTruth = Texture2D("groundTruth",
                            "scenes/bistro/cloud.png",
                            true,
                            false
    );

    // Skysphere::envMap = Texture2D("envMap", Skysphere::groundTruth->w, Skysphere::groundTruth->h,
    //                               Skysphere::groundTruth->internal_format, Skysphere::groundTruth->format,
    //                               Skysphere::groundTruth->type);

    Skysphere::envMap = Texture2D("envMap", 2048, 1024, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    Skysphere::filteredEnvMap = Texture2D("filteredEnvMap", 2048, 1024, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    Skysphere::Deviation = Texture2D("deviation", 2048, 1024, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    Skysphere::mean = Texture2D("mean", 2048, 1024, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    Skysphere::confidence = Texture2D("confidence", 2048, 1024, GL_R32F, GL_RED, GL_FLOAT);
    k_means = Texture2D("k_means",
                        "scripts/k-means.png",
                        false,
                        false
    );

    buckets = Texture3D("bucketsGrid", 2048, 1024, 8, GL_RGBA32F, GL_RGBA, GL_FLOAT);

    bucketsCounter = Texture3D("bucketsCounterGrid", 2048, 1024, 8, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);

    // Texturen entbinden
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint counterWrittenID;
    glGenTextures(1, &counterWrittenID);
    glBindTexture(GL_TEXTURE_2D, counterWrittenID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 2048, 1024, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    // R32F für float
    glClearTexImage(counterWrittenID, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    Skysphere::counterWritten = Texture2D("counterWritten", 2048, 1024, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
    glBindTexture(GL_TEXTURE_2D, 0);

    std::vector<std::string> cam_files;
    cam_files.emplace_back("camera0.ini");

    std::shared_ptr<DatasetParams> params = std::make_shared<DatasetParams>(scene, cam_files, "images",
                                                                            1.f);
    Skysphere::loader.load(params);

    map_forward = Shader("map_forward_cs", "shader/map_forward.cs");
    map_backward = Shader("map_backward_cs", "shader/map_backward.cs");

    map_forward_mask = Shader("map_forward_cs", "shader/map_forward_mask.cs");
    map_backward_mask = Shader("map_backward_cs", "shader/map_backward_mask.cs");

    map_weighting = Shader("map_weighting_cs", "shader/map_backward_mask_weighting.cs");
    map_divide = Shader("map_divide_cs", "shader/map_divide.cs");
    map_squaredDeviation = Shader("map_squared_deviation_cs", "shader/map_square_deviation.cs");
    map_standardDeviation = Shader("map_standard_deviation_cs", "shader/map_standard_deviation.cs");
    map_removeOutliners = Shader("map_removeOutliners_cs", "shader/map_removeOutliners.cs");

    map_confidence = Shader("map_confidence_cs", "shader/map_confidence.cs");

    map_buckets = Shader("map_buckets_cs", "shader/map_buckets.cs");
    map_k_means = Shader("map_k_means_cs", "shader/map_k_means.cs");
    map_buckets_calc = Shader("map_buckets_calc", "shader/map_buckets_calc.cs");

    filteredConfidence = Shader("filter_confidence_cs", "shader/filter_confidence.cs");

    sky_mat->add_texture("diffuse", Skysphere::groundTruth);
    sky_mesh->material = sky_mat;

    auto sky_shader = Shader("sky-shader", "shader/sky.vs", "shader/sky.fs");

    prototype = Drawelement("skysphere", sky_shader, sky_mesh);
}

// ------------------------------------------------
// Skysphere

Skysphere::Skysphere() : model(1) {
    model = glm::mat4(1);
    // mat4 rot_y_180 = glm::rotate(mat4(1), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
    // model = rot_y_180 * model;
}

void Skysphere::update_forwards() {
    resetTextures();

    // TODO create a double loop for multiple cameras. Than the dataLoader.load should be in a loop as well
    // TODO to extend with Masks create uniform to transmit the mask
    //
    for (size_t i = 0; i < Skysphere::loader.views.size(); ++i) {
        // Setze Uniforms für das aktuelle Bild
        map_forward->bind();

        mat4 withoutPosView = Skysphere::loader.views[i].pose.view_mat;
        withoutPosView[3] = vec4(0, 0, 0, 1);
        map_forward->uniform("u_invViewMatrix", inverse(withoutPosView)); // Extrinsics
        map_forward->uniform("u_invProjectionMatrix", inverse(Skysphere::loader.cams[0].proj)); // Intrinsics
        map_forward->uniform("u_imageResolution",
                             glm::ivec2(Skysphere::loader.cams[0].width, Skysphere::loader.cams[0].height));
        map_forward->uniform("u_envMapResolution", glm::ivec2(2048, 1024));

        Skysphere::loader.views[i].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);
        Skysphere::envMap->bind_image(1, GL_WRITE_ONLY, GL_RGBA32F);

        map_forward->dispatch_compute(1024, 1024, 1);

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        Skysphere::envMap->unbind();
        Skysphere::loader.views[i].image->unbind();
        map_forward->unbind();
    }
    prototype->mesh->material->add_texture("diffuse", Skysphere::envMap);
}

void Skysphere::update_backwards() {
    resetTextures();

    for (size_t i = 0; i < Skysphere::loader.views.size(); ++i) {
        map_backward->bind();
        mat4 withoutPosView = Skysphere::loader.views[i].pose.view_mat;
        withoutPosView[3] = vec4(0, 0, 0, 1);
        map_backward->uniform("u_ViewMatrix", withoutPosView); // Extrinsics
        map_backward->uniform("u_ProjectionMatrix", Skysphere::loader.cams[0].proj); // Intrinsics
        map_backward->uniform("u_imageResolution",
                              glm::ivec2(Skysphere::loader.cams[0].width, Skysphere::loader.cams[0].height));
        map_backward->uniform("u_envMapResolution", glm::ivec2(2048, 1024));

        Skysphere::loader.views[i].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);
        Skysphere::envMap->bind_image(1, GL_WRITE_ONLY, GL_RGBA32F);

        map_backward->dispatch_compute(2048, 1024, 1);

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        Skysphere::envMap->unbind();
        Skysphere::loader.views[i].image->unbind();
        map_backward->unbind();
    }
    prototype->mesh->material->add_texture("diffuse", Skysphere::envMap);
}

void Skysphere::update_forwards_mask() {
    resetTextures();

    for (size_t i = 0; i < Skysphere::loader.views.size(); ++i) {
        // Setze Uniforms für das aktuelle Bild
        map_forward_mask->bind();

        mat4 withoutPosView = Skysphere::loader.views[i].pose.view_mat;
        withoutPosView[3] = vec4(0, 0, 0, 1);
        map_forward_mask->uniform("u_invViewMatrix", inverse(withoutPosView)); // Extrinsics
        map_forward_mask->uniform("u_invProjectionMatrix", inverse(Skysphere::loader.cams[0].proj)); // Intrinsics
        map_forward_mask->uniform("u_imageResolution",
                                  glm::ivec2(Skysphere::loader.cams[0].width, Skysphere::loader.cams[0].height));
        map_forward_mask->uniform("u_envMapResolution", glm::ivec2(2048, 1024));

        Skysphere::loader.views[i].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);
        Skysphere::envMap->bind_image(1, GL_WRITE_ONLY, GL_RGBA32F);

        map_forward_mask->dispatch_compute(1024, 1024, 1);

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        Skysphere::envMap->unbind();
        Skysphere::loader.views[i].image->unbind();
        map_forward_mask->unbind();
    }
    prototype->mesh->material->add_texture("diffuse", Skysphere::envMap);
}

void Skysphere::update_backwards_mask() {
    resetTextures();

    for (size_t i = 0; i < Skysphere::loader.views.size(); ++i) {
        map_backward_mask->bind();
        mat4 withoutPosView = Skysphere::loader.views[i].pose.view_mat;
        withoutPosView[3] = vec4(0, 0, 0, 1);
        map_backward_mask->uniform("u_ViewMatrix", withoutPosView); // Extrinsics
        map_backward_mask->uniform("u_ProjectionMatrix", Skysphere::loader.cams[0].proj); // Intrinsics
        map_backward_mask->uniform("u_imageResolution",
                                   glm::ivec2(Skysphere::loader.cams[0].width, Skysphere::loader.cams[0].height));
        map_backward_mask->uniform("u_envMapResolution", glm::ivec2(2048, 1024));

        Skysphere::loader.views[i].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);
        Skysphere::envMap->bind_image(1, GL_WRITE_ONLY, GL_RGBA32F);

        map_backward_mask->dispatch_compute(2048, 1024, 1);

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        Skysphere::envMap->unbind();
        Skysphere::loader.views[i].image->unbind();
        map_backward_mask->unbind();
    }
    prototype->mesh->material->add_texture("diffuse", Skysphere::envMap);
}

void Skysphere::update_backwards_mask_weighting() {
    resetTextures();


    //alle Farbwerte addieren und Counter erhöhen
    for (size_t i = 0; i < Skysphere::loader.views.size(); ++i) {
        map_weighting->bind();
        mat4 withoutPosView = Skysphere::loader.views[i].pose.view_mat;
        withoutPosView[3] = vec4(0, 0, 0, 1);
        map_weighting->uniform("u_ViewMatrix", withoutPosView); // Extrinsics
        map_weighting->uniform("u_ProjectionMatrix", Skysphere::loader.cams[0].proj); // Intrinsics
        map_weighting->uniform("u_imageResolution",
                               glm::ivec2(Skysphere::loader.cams[0].width, Skysphere::loader.cams[0].height));
        map_weighting->uniform("u_envMapResolution", glm::ivec2(2048, 1024));

        map_weighting->uniform("useMasks", (int) useMasks);

        Skysphere::loader.views[i].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);
        Skysphere::counterWritten->bind_image(1, GL_READ_WRITE, GL_R32UI);
        Skysphere::mean->bind_image(2, GL_READ_WRITE, GL_RGBA32F);

        map_weighting->dispatch_compute(2048, 1024, 1);

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        Skysphere::mean->unbind();
        Skysphere::counterWritten->unbind();
        Skysphere::loader.views[i].image->unbind();
        map_weighting->unbind();
    }

    //Mittelwert berechnen
    map_divide->bind();

    Skysphere::mean->bind_image(0, GL_READ_WRITE, GL_RGBA32F);
    Skysphere::counterWritten->bind_image(1, GL_READ_WRITE, GL_R32UI);

    map_divide->dispatch_compute(2048, 1024, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    Skysphere::counterWritten->unbind();
    Skysphere::mean->unbind();

    map_divide->unbind();

    //Quadratische Abweichungen berechen
    for (size_t i = 0; i < Skysphere::loader.views.size(); ++i) {
        map_squaredDeviation->bind();
        mat4 withoutPosView = Skysphere::loader.views[i].pose.view_mat;
        withoutPosView[3] = vec4(0, 0, 0, 1);
        map_squaredDeviation->uniform("u_ViewMatrix", withoutPosView); // Extrinsics
        map_squaredDeviation->uniform("u_ProjectionMatrix", Skysphere::loader.cams[0].proj); // Intrinsics
        map_squaredDeviation->uniform("u_imageResolution",
                                      glm::ivec2(Skysphere::loader.cams[0].width, Skysphere::loader.cams[0].height));
        map_squaredDeviation->uniform("u_envMapResolution", glm::ivec2(2048, 1024));

        map_squaredDeviation->uniform("useMasks", (int) useMasks);

        Skysphere::loader.views[i].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);
        Skysphere::mean->bind_image(1, GL_READ_WRITE, GL_RGBA32F);
        Skysphere::Deviation->bind_image(2, GL_READ_WRITE, GL_RGBA32F);

        map_squaredDeviation->dispatch_compute(2048, 1024, 1);

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        Skysphere::Deviation->unbind();
        Skysphere::mean->unbind();
        Skysphere::loader.views[i].image->unbind();
        map_squaredDeviation->unbind();
    }


    //Standardabweichung berechnen


    map_standardDeviation->bind();
    Skysphere::counterWritten->bind_image(0, GL_READ_WRITE, GL_R32UI);
    Skysphere::Deviation->bind_image(1, GL_READ_WRITE, GL_RGBA32F);

    map_standardDeviation->dispatch_compute(2048, 1024, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    Skysphere::Deviation->unbind();
    Skysphere::counterWritten->unbind();

    map_standardDeviation->unbind();

    //reset counter
    GLuint counterWrittenID = Skysphere::counterWritten->id;
    glBindTexture(GL_TEXTURE_2D, counterWrittenID);
    glClearTexImage(counterWrittenID, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    //Alle Pixel die abweichen entfernen
    for (size_t i = 0; i < Skysphere::loader.views.size(); ++i) {
        map_removeOutliners->bind();
        mat4 withoutPosView = Skysphere::loader.views[i].pose.view_mat;
        withoutPosView[3] = vec4(0, 0, 0, 1);
        map_removeOutliners->uniform("u_ViewMatrix", withoutPosView); // Extrinsics
        map_removeOutliners->uniform("u_ProjectionMatrix", Skysphere::loader.cams[0].proj); // Intrinsics
        map_removeOutliners->uniform("u_imageResolution",
                                     glm::ivec2(Skysphere::loader.cams[0].width, Skysphere::loader.cams[0].height));
        map_removeOutliners->uniform("u_envMapResolution", glm::ivec2(2048, 1024));

        map_removeOutliners->uniform("useMasks", (int) useMasks);

        Skysphere::loader.views[i].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);
        Skysphere::envMap->bind_image(1, GL_READ_WRITE, GL_RGBA32F);
        Skysphere::counterWritten->bind_image(2, GL_READ_WRITE, GL_R32UI);
        Skysphere::Deviation->bind_image(3, GL_READ_WRITE, GL_RGBA32F);
        Skysphere::mean->bind_image(4, GL_READ_WRITE, GL_RGBA32F);

        map_removeOutliners->dispatch_compute(2048, 1024, 1);

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        Skysphere::mean->unbind();
        Skysphere::Deviation->unbind();
        Skysphere::counterWritten->unbind();
        Skysphere::envMap->unbind();
        Skysphere::loader.views[i].image->unbind();
        map_removeOutliners->unbind();
    }

    // //Mittelwert berechnen
    map_divide->bind();

    Skysphere::envMap->bind_image(0, GL_READ_WRITE, GL_RGBA32F);
    Skysphere::counterWritten->bind_image(1, GL_READ_WRITE, GL_R32UI);

    map_divide->dispatch_compute(2048, 1024, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    Skysphere::counterWritten->unbind();
    Skysphere::envMap->unbind();

    map_divide->unbind();

    prototype->mesh->material->add_texture("diffuse", Skysphere::envMap);
}

void Skysphere::update_backwards_mask_confidence(const int indexPicture) {
    map_confidence->bind();
    mat4 withoutPosView = loader.views[indexPicture].pose.view_mat;
    withoutPosView[3] = vec4(0, 0, 0, 1);
    map_confidence->uniform("u_ViewMatrix", withoutPosView); // Extrinsics
    map_confidence->uniform("u_ProjectionMatrix", loader.cams[0].proj); // Intrinsics
    map_confidence->uniform("u_imageResolution",
                            glm::ivec2(loader.cams[0].width, loader.cams[0].height));
    map_confidence->uniform("u_envMapResolution", ivec2(2048, 1024));

    map_confidence->uniform("confidenceThreshold", confidenceThreshold);
    map_confidence->uniform("confidenceIncreaseFactor", confidenceIncreaseFactor);
    map_confidence->uniform("confidenceDecreaseFactor", confidenceDecreaseFactor);
    map_confidence->uniform("initialConfidence", initialConfidence);

    map_confidence->uniform("useMasks", (int) useMasks);

    loader.views[indexPicture].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);
    envMap->bind_image(1, GL_READ_WRITE, GL_RGBA32F);
    confidence->bind_image(2, GL_READ_WRITE, GL_R32F);

    map_confidence->dispatch_compute(2048, 1024, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    confidence->unbind();
    envMap->unbind();
    loader.views[indexPicture].image->unbind();
    map_confidence->unbind();
    //
    // envMap->save_ldr(
    //     fs::path("envMap_pictures") / (fs::path(envMap->name) // Originalname von envMap
    //                                    .replace_extension("") // Entferne die ursprüngliche Erweiterung
    //                                    .string() + std::to_string(indexPicture) + ".png")
    // );
    // confidence->save_ldr(
    //     fs::path("confidence_pictures") / (fs::path(confidence->name) // Originalname von envMap
    //                                        .replace_extension("") // Entferne die ursprüngliche Erweiterung
    //                                        .string() + std::to_string(indexPicture) + ".png")
    // );
}

void Skysphere::update_buckets() {
    resetTextures();
    for (size_t i = 0; i < loader.views.size(); i++) {
        map_buckets->bind();
        mat4 withoutPosView = loader.views[i].pose.view_mat;
        withoutPosView[3] = vec4(0, 0, 0, 1);
        map_buckets->uniform("u_ViewMatrix", withoutPosView); // Extrinsics
        map_buckets->uniform("u_ProjectionMatrix", loader.cams[0].proj); // Intrinsics
        map_buckets->uniform("u_imageResolution",
                             glm::ivec2(loader.cams[0].width, loader.cams[0].height));
        map_buckets->uniform("u_envMapResolution", ivec2(2048, 1024));

        loader.views[i].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);

        buckets->bind_image(1, GL_READ_WRITE, GL_RGBA32F);

        bucketsCounter->bind_image(2, GL_READ_WRITE, GL_R32UI);

        map_buckets->dispatch_compute(2048, 1024, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        bucketsCounter->unbind();

        buckets->unbind();

        loader.views[i].image->unbind();

        map_buckets->unbind();
    }

    map_buckets_calc->bind();

    buckets->bind_image(0, GL_READ_WRITE, GL_RGBA32F);

    bucketsCounter->bind_image(1, GL_READ_WRITE, GL_R32UI);

    envMap->bind_image(2, GL_READ_WRITE, GL_RGBA32F);

    map_buckets->dispatch_compute(2048, 1024, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    envMap->unbind();

    bucketsCounter->unbind();

    buckets->unbind();
    map_buckets_calc->unbind();
}

void Skysphere::update_k_means() {
    resetTextures();
    for (size_t i = 0; i < loader.views.size(); i++) {
        map_k_means->bind();
        mat4 withoutPosView = loader.views[i].pose.view_mat;
        withoutPosView[3] = vec4(0, 0, 0, 1);
        map_k_means->uniform("u_ViewMatrix", withoutPosView); // Extrinsics
        map_k_means->uniform("u_ProjectionMatrix", loader.cams[0].proj); // Intrinsics
        map_k_means->uniform("u_imageResolution",
                             glm::ivec2(loader.cams[0].width, loader.cams[0].height));
        map_k_means->uniform("u_envMapResolution", ivec2(2048, 1024));

        loader.views[i].image->bind_image(0, GL_READ_ONLY, GL_RGBA8);

        buckets->bind_image(1, GL_READ_WRITE, GL_RGBA32F);

        bucketsCounter->bind_image(2, GL_READ_WRITE, GL_R32UI);

        k_means->bind_image(3, GL_READ_WRITE, GL_RGBA8);

        map_k_means->dispatch_compute(2048, 1024, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        k_means->unbind();

        bucketsCounter->unbind();

        buckets->unbind();

        loader.views[i].image->unbind();

        map_k_means->unbind();
    }

    map_buckets_calc->bind();

    buckets->bind_image(0, GL_READ_WRITE, GL_RGBA32F);

    bucketsCounter->bind_image(1, GL_READ_WRITE, GL_R32UI);

    envMap->bind_image(2, GL_READ_WRITE, GL_RGBA32F);

    map_buckets->dispatch_compute(2048, 1024, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    envMap->unbind();

    bucketsCounter->unbind();

    buckets->unbind();
    map_buckets_calc->unbind();
}

void Skysphere::filterConfidence() {
    filteredConfidence->bind();

    envMap->bind_image(0, GL_READ_WRITE, GL_RGBA32F);
    filteredEnvMap->bind_image(1, GL_READ_WRITE, GL_RGBA32F);
    confidence->bind_image(2, GL_READ_WRITE, GL_R32F);

    filteredConfidence->uniform("threshold", filterConfidenceThreshold);

    filteredConfidence->dispatch_compute(2048, 1024, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);


    confidence->unbind();
    filteredEnvMap->unbind();
    envMap->unbind();

    filteredConfidence->unbind();
}

void Skysphere::resetTextures() {
    glBindTexture(GL_TEXTURE_2D, counterWritten->id);
    glClearTexImage(counterWritten->id, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // Farbe (RGBA) zum Clearen
    glBindTexture(GL_TEXTURE_2D, envMap->id);
    glClearTexImage(envMap->id, 0, GL_RGBA, GL_FLOAT, clearColor);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, Deviation->id);
    glClearTexImage(Deviation->id, 0, GL_RGBA, GL_FLOAT, clearColor);
    glBindTexture(GL_TEXTURE_2D, 0);

    // mean Textur clearen
    glBindTexture(GL_TEXTURE_2D, mean->id);
    glClearTexImage(mean->id, 0, GL_RGBA, GL_FLOAT, clearColor);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Reset der Textur, um sie mit 0 zu füllen
    glBindTexture(GL_TEXTURE_2D, skysphere->confidence->id); // Binde die Textur
    glClearTexImage(skysphere->confidence->id, 0, GL_RED, GL_FLOAT, clearColor);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_3D, buckets->id);
    glClearTexImage(buckets->id, 0, GL_RGBA, GL_FLOAT, clearColor);
    glBindTexture(GL_TEXTURE_3D, 0);

    glBindTexture(GL_TEXTURE_3D, bucketsCounter->id);
    glClearTexImage(bucketsCounter->id, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void Skysphere::draw() {
    glDepthFunc(GL_LEQUAL);
    // switch from counter clockwise convention to clockwise
    glFrontFace(GL_CW);

    // HINT: here we might have skipped to draw the drawelement
    prototype->model = model;
    prototype->bind();
    prototype->draw();
    prototype->unbind();

    // switch back
    glFrontFace(GL_CCW);
    glDepthFunc(GL_LESS);
}
