#include <cppgl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>

#include "gui.h"
#include "static-objects.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include "texture.h"
#include "scene.h"

// ------------------------------------------
// helper funcs and callbacks

void blit(const cppgl::Texture2D &tex) {
    static cppgl::Shader blit_shader("blit", "shader/quad.vs", "shader/blit.fs");
    blit_shader->bind();
    blit_shader->uniform("tex", tex, 0);
    cppgl::Quad::draw();
    blit_shader->unbind();
}

void resize_callback(int w, int h) {
    cppgl::Framebuffer::find("example_fbo")->resize(w, h);
    cppgl::Texture2D::find("computeExampleOutputTex")->resize(w, h);
}

void keyboard_callback(int key, int scancode, int action, int mods) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_R && action == GLFW_PRESS)
        cppgl::reload_modified_shaders();
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
        cppgl::Context::screenshot("screenshot.png");
}

void mouse_button_callback(int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse) return;
}

// ------------------------------------------
// main

using namespace cppgl;

int main(int argc, char **argv) {
    // init GL
    ContextParameters params;
    params.width = 1920;
    params.height = 1080;
    params.title = "Environment Map";
    //params.floating = GLFW_TRUE;
    //params.resizable = GLFW_FALSE;
    params.swap_interval = 1;
    Context::init(params);

    // setup fbo
    const glm::ivec2 res = Context::resolution();
    Framebuffer fbo = Framebuffer("example_fbo", res.x, res.y);
    fbo->attach_depthbuffer(Texture2D("example_fbo/depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT,
                                      GL_FLOAT));
    fbo->attach_colorbuffer(Texture2D("example_fbo/col", res.x, res.y, GL_RGBA32F, GL_RGBA, GL_FLOAT));
    fbo->check();

    // setup draw shader
    Shader("draw", "shader/draw.vs", "shader/draw.fs");
    Shader fallbackShader = Shader("fallback", "shader/quad.vs", "shader/fallback.fs");

    // setup compute shader
    Shader computeShaderExample = Shader("computeShaderExample", "shader/computeShaderExample.glcs");
    Texture2D computeShaderOutputTex("computeExampleOutputTex", Context::resolution().x, Context::resolution().y,
                                     GL_RGBA32F, GL_RGBA, GL_FLOAT);

    // install callbacks
    Context::set_resize_callback(resize_callback);
    Context::set_keyboard_callback(keyboard_callback);
    Context::set_mouse_button_callback(mouse_button_callback);
    static bool doGreyscaleComputeShaderExample = false;
    gui_add_callback("example_gui_callback", [] {
        ImGui::ShowMetricsWindow();
        ImGui::Checkbox("compute shader example: convert to greyscale",
                        &doGreyscaleComputeShaderExample);
    });

    string model = "";
    string toLoadScene = "";
    bool pointCloud = false;
    // parse cmd line args
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "model") {
            model = argv[++i];
        } else if (arg == "scene") {
            toLoadScene = argv[++i];
        } else if (arg == "cloud") {
            pointCloud = true;
        }
    }

    if (model == "" && toLoadScene == "") {
        std::cerr << "You must specify a valid scene model!" << std::endl;
        return 1;
    }


    Scene scene;
    scene.cloud = pointCloud;
    scene.importScene(model);


    // std::ostringstream command;
    // command << "bash skripts/call_k-means.py"; // Ausgabe umleiten
    //
    // system(command.str().c_str()); // Python-Skript ausführen
    //
    // std::ifstream file("scripts/k-means-result.txt");
    // if (!file) {
    //     std::cerr << "Fehler beim Öffnen der Datei\n";
    //     return 1;
    // }
    //
    // std::stringstream buffer;
    // buffer << file.rdbuf();

    skysphere->init(toLoadScene);

    // skysphere->k_means = buffer.str();


    while (Context::running()) {
        glPointSize(Gui::pointSize);

        // handle input
        glfwPollEvents();
        CameraImpl::default_input_handler(Context::frame_time());

        // update and reload shaders
        current_camera()->update();
        static uint32_t frame_counter = 0;
        if (frame_counter++ % 100 == 0)
            reload_modified_shaders();

        // render all drawelements (or fallback) into fbo
        fbo->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skysphere->draw();
        scene.draw();

        fbo->unbind();

        Gui::camera_capture_mouse = false;
        Gui::drawGUI(fbo);
        // finish frame
        Context::swap_buffers();
    }
}
