//
// Created by it98eqyc on 12/10/24.
//

#include "scene.h"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>

#include "gui.h"
#include "../submodules/assimp/code/AssetLib/MD3/MD3FileData.h"

using namespace cppgl;

void Scene::importScene(const std::string &path) {
    std::vector<Mesh> meshes = load_meshes_gpu(path, false);
    Shader shader = Shader("mesh_shader", "shader/draw.vs", "shader/draw.fs");
    Shader pointShader = Shader("point_shader", "shader/point_cloud_draw.vs", "shader/point_cloud_draw.fs");

    for (int i = 0; i < meshes.size(); i++) {
        if (cloud) {
            draw_elements.emplace_back("mesh" + std::to_string(i), pointShader, meshes[i]);
        } else {
            draw_elements.emplace_back("mesh" + std::to_string(i), shader, meshes[i]);
        }
    }
}

void Scene::draw() {
    // static float x_angle = 0;
    // static float y_angle = 0;
    // static float z_angle = 0;
    // ImGui::Begin("rot scene");
    // ImGui::DragFloat("x angle", &x_angle, 0.1, -180., 180.);
    // ImGui::DragFloat("y angle", &y_angle, 0.1, -180., 180.);
    // ImGui::DragFloat("z angle", &z_angle, 0.1, -180., 180.);
    // ImGui::End();

    glm::mat4 rot_x = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1.f, 0.f, 0.f));
    // glm::mat4 rot_y = glm::rotate(glm::mat4(1), glm::radians(y_angle), glm::vec3(0.f, 1.f, 0.f));
    // glm::mat4 rot_z = glm::rotate(glm::mat4(1), glm::radians(z_angle), glm::vec3(0.f, 0.f, 1.f));

    for (auto &draw_element: draw_elements) {
        if (cloud) {
            draw_element->bind();
            Mesh m = draw_element->mesh;
            glDrawArrays(GL_POINTS, 0, m->geometry->positions.size());
            draw_element->unbind();
        } else {
            // draw_element->model = glm::rotate(glm::mat4(1), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::mat4(1);
            draw_element->model = rot_x;
            draw_element->bind();
            draw_element->draw();
            draw_element->unbind();
        }
    }
}
