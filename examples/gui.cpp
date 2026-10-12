//
// Created by it98eqyc on 12/3/24.
//
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include "gui.h"
#include <cppgl.h>

#include "static-objects.h"

using namespace cppgl;
using namespace glm;

//
// // setup draw shader
// Shader (
// "draw"
// ,
// "shader/draw.vs"
// ,
// "shader/draw.fs"
// );
//
// Shader fallbackShader = Shader("fallback", "shader/quad.vs", "shader/fallback.fs");
//
// // setup compute shader
// Shader computeShaderExample = Shader("computeShaderExample", "shader/computeShaderExample.glcs");
// Texture2D computeShaderOutputTex("computeExampleOutputTex", Context::resolution().x, Context::resolution().y,
//                                  GL_RGBA32F, GL_RGBA, GL_FLOAT);


int content_fullscreen = -1;
int indexPicture = 0;
// static bool camera_capture_mouse = false;

// ImGui docking setup
ImGuiID id_dock_main, id_dock_gui_rendering, id_dock_gui_rendering0, id_dock_gui_rendering1, id_dock_gui_rendering2,
        id_dock_gui_rendering3, id_dock_gui_right, id_dock_gui_bottom, id_dock_gui_bottom_right;

static bool imgui_dock_initialized = false;
bool Gui::camera_capture_mouse = false;
static bool doGreyscaleComputeShaderExample = false;
static bool testing = false;
static bool backward = false;
static int pos = 1;
int Gui::pointSize = 2;


// texture_to_imgui render a texture to imgui
/// \param tex the texture to render
/// \param size size in pixel of the imgui image to be rendered, 0 implies the original texture size
/// \param uv0 uv origin coordinate. the top left corner points to that coordinate of the texture
/// \param uv1 uv end coordinate. the bottom right corner points to that coordinate of the texture
/// \param tint_col
/// \param border_col
inline void texture_to_imgui(const cppgl::Texture2D &tex, const ivec2 &size = vec2(0, 0), const vec2 &uv0 = vec2(0, 0),
                             const vec2 &uv1 = vec2(1, 1), const vec4 &tint_col = vec4(1, 1, 1, 1),
                             const vec4 &border_col = vec4(0, 0, 0, 0)) {
    int w = (size[0] == 0) ? tex->w : size[0];
    int h = (size[1] == 0) ? tex->h : size[1];

    // Hier kehren wir die Y-Koordinate der uv1 um (von 1 zu 0)
    ImGui::Image(size_t(tex->id),
                 ImVec2((float) w, (float) h),
                 ImVec2(uv0.x, 1.0f - uv0.y),
                 ImVec2(uv1.x, 1.0f - uv1.y),
                 ImVec4(tint_col.x, tint_col.y, tint_col.z, tint_col.w),
                 ImVec4(border_col.x, border_col.y, border_col.z, border_col.w));
}


/// compute centered position while keeping the original aspect ratio
/// \param win_size size of the window / available content dimensions
/// \param tex_size original dimensions of the texture
/// \param out_img_size return argument for the image size
/// \param out_img_pos return argument for the image position
inline void centeredPosSize(const vec2 &win_size, const vec2 &tex_size, vec2 &out_img_size, vec2 &out_img_pos) {
    float ar_win = win_size.x / win_size.y;
    float ar_tex = tex_size.x / tex_size.y;
    if (ar_tex > ar_win) {
        // width is fitting
        out_img_size = vec2(win_size.x, win_size.x * (1 / ar_tex));
    } else {
        // height is fitting
        out_img_size = vec2((win_size.y) * ar_tex, (win_size.y)); // -2 to prevent an unwanted scrollbar
    }
    out_img_pos = vec2((win_size.x - out_img_size.x) / 2, (win_size.y - out_img_size.y) / 2);
}

static float x_angle = 0;
static float y_angle = 0;
static float z_angle = 0;

void Gui::drawGUI(Framebuffer fbo) {
    // ---------------------------------------
    // ImGui

    glm::vec2 res = Context::resolution();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(res.x, res.y));

    ImGui::Begin("Main", (bool *) nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoBackground |
                 ImGuiWindowFlags_NoBringToFrontOnFocus |
                 ImGuiWindowFlags_NoInputs |
                 ImGuiWindowFlags_NoScrollWithMouse);

    id_dock_main = ImGui::GetID("Dock_Main");
    ImGui::DockSpace(
        id_dock_main);
    if (!imgui_dock_initialized) {
        ImGui::DockBuilderRemoveNode(id_dock_main);
        ImGui::DockBuilderAddNode(id_dock_main);

        ImGuiID id_dock_main_copy = id_dock_main;
        ImGui::DockBuilderSplitNode(id_dock_main_copy, ImGuiDir_Left, 0.85f, &id_dock_gui_rendering,
                                    &id_dock_gui_right); // split horizontal
        ImGui::DockBuilderSplitNode(id_dock_gui_right, ImGuiDir_Up, 0.85f, &id_dock_gui_right,
                                    &id_dock_gui_bottom_right); // split right gui part vertical
        ImGuiID id_dock_gui_rendering_copy = id_dock_gui_rendering;
        ImGui::DockBuilderSplitNode(id_dock_gui_rendering_copy, ImGuiDir_Up, 0.85f, &id_dock_gui_rendering,
                                    &id_dock_gui_bottom); // split left gui part vertical
        id_dock_gui_rendering_copy = id_dock_gui_rendering;
        ImGui::DockBuilderSplitNode(id_dock_gui_rendering_copy, ImGuiDir_Up, 0.5f, &id_dock_gui_rendering0,
                                    &id_dock_gui_rendering2); // split rendering vertical
        ImGui::DockBuilderSplitNode(id_dock_gui_rendering0, ImGuiDir_Left, 0.5f, &id_dock_gui_rendering0,
                                    &id_dock_gui_rendering1); // split rendering0 vertical
        ImGui::DockBuilderSplitNode(id_dock_gui_rendering2, ImGuiDir_Left, 0.5f, &id_dock_gui_rendering2,
                                    &id_dock_gui_rendering3); // split rendering1 vertical


        ImGui::DockBuilderDockWindow("Application", id_dock_gui_right);
        ImGui::DockBuilderDockWindow("Params", id_dock_gui_right);
        ImGui::DockBuilderDockWindow("Random", id_dock_gui_bottom_right);
        ImGui::DockBuilderDockWindow("Performance", id_dock_gui_bottom);
        ImGui::DockBuilderDockWindow("###Inovis0", id_dock_gui_rendering0);
        ImGui::DockBuilderDockWindow("###Inovis1", id_dock_gui_rendering1);
        ImGui::DockBuilderDockWindow("###Inovis2", id_dock_gui_rendering2);
        ImGui::DockBuilderDockWindow("###Inovis3", id_dock_gui_rendering3);

        ImGui::DockBuilderFinish(id_dock_main);

        imgui_dock_initialized = true;
    }
    ImGui::End();

    if (ImGui::Begin("Application")) {
        // ImGui::SliderInt("Image", &pos, 1, 11);
        if (ImGui::Button("backwards mask")) {
            float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // Farbe (RGBA) zum Clearen
            glBindTexture(GL_TEXTURE_2D, skysphere->envMap->id);

            // Texturinhalt "clearen" (alle Pixel auf clearColor setzen)
            glClearTexImage(skysphere->envMap->id, 0, GL_RGBA, GL_FLOAT, clearColor);
            // Textur entbinden
            glBindTexture(GL_TEXTURE_2D, 0);


            skysphere->update_backwards_mask();
        }

        if (ImGui::Button("forward")) {
            skysphere->update_forwards();
        }

        if (ImGui::Button("backwards")) {
            skysphere->update_backwards();
        }

        if (ImGui::Button("forward_mask")) {
            skysphere->update_forwards_mask();
        }
        if (ImGui::Button("buckets")) {
            skysphere->update_buckets();
        }
        if (ImGui::Button("k-means")) {
            skysphere->update_k_means();
        }
        if (ImGui::Button("weighting sceme")) {
            skysphere->update_backwards_mask_weighting();
        }

        ImGui::Checkbox("mask", &skysphere->useMasks);


        ImGui::SliderFloat("Confidence Threshold", &skysphere->confidenceThreshold, 0.0f, 1.0f, "%.3f");

        // Regler für "Confidence Increase Factor"
        ImGui::SliderFloat("Confidence Increase Factor", &skysphere->confidenceIncreaseFactor, 0.0f, 1.0f, "%.3f");

        // Regler für "Confidence Decrease Factor"
        ImGui::SliderFloat("Confidence Decrease Factor", &skysphere->confidenceDecreaseFactor, 0.0f, 1.0f, "%.3f");

        // Regler für "Initial Confidence"
        ImGui::SliderFloat("Initial Confidence", &skysphere->initialConfidence, 0.0f, 1.0f, "%.3f");

        if (ImGui::Button(("All by 1"))) {
            skysphere->resetTextures();
            for (int i = 0; i < skysphere->loader.size(); i++) {
                skysphere->update_backwards_mask_confidence(i);
            }
        }


        ImGui::DragFloat("x angle", &x_angle, 0.1, -180., 180.);
        ImGui::DragFloat("y angle", &y_angle, 0.1, -180., 180.);
        ImGui::DragFloat("z angle", &z_angle, 0.1, -180., 180.);

        if (ImGui::SliderInt("Position", &pos, 0, 255, "%d")) {
            glm::mat4 view = skysphere->loader.views[pos].pose.view_mat;
            // mat4 inverse_view = view;
            // current_camera()->pos = glm::vec3(view[3].x, view[3].y, view[3].z);
            current_camera()->view = view;
            // current_camera()->proj = skysphere->loader.cams[pos].proj;
        }

        if (ImGui::SliderInt("Confidence by Picture", &indexPicture, 0, 255, "%d")) {
            skysphere->update_backwards_mask_confidence(indexPicture);
        };

        ImGui::Text(std::to_string(indexPicture).c_str());
        if (ImGui::Button(("reset"))) {
            skysphere->resetTextures();
            indexPicture = 0;
        }

        if (ImGui::SliderFloat("Threshold EnvMap Confidence", &skysphere->filterConfidenceThreshold, 0, 100.0f,
                               "%.0f")) {
            skysphere->filterConfidence();
        }

        ImGui::SliderInt("glPointSize", &pointSize, 1, 20);

        // static float x_angle = 0;
        // static float y_angle = 0;
        // ImGui::Begin("rot scene");
        // ImGui::DragFloat("x angle", &x_angle, 1, -25, 25);
        // ImGui::DragFloat("y angle", &y_angle, 1, -25, 25);
        // ImGui::InputFloat("View x", &x_angle);
        // ImGui::End();

        // current_camera()->pos = glm::vec3(x_angle, y_angle, 3.5);
    }
    ImGui::End();

    if (ImGui::Begin("Random")) {
        ImGui::Checkbox("Test position", &testing);

        //ImGui::Checkbox("compute shader example: convert to greyscale", &doGreyscaleComputeShaderExample);
    }
    ImGui::End();
    if (ImGui::Begin("Performance")) {
        // ImGui::ShowMetricsWindow();
    }
    ImGui::End();

    camera_capture_mouse = false;

    // fbo
    if (content_fullscreen == -1 || content_fullscreen == 0) {
        if (ImGui::Begin("Window0###Inovis0", (bool *) nullptr), ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoScrollbar) {
            auto windowsize = ImGui::GetContentRegionMax();
            vec2 img_size, img_pos;
            centeredPosSize(vec2(windowsize[0], windowsize[1]),
                            vec2((float) skysphere->groundTruth->w, (float) skysphere->groundTruth->h),
                            img_size,
                            img_pos);
            ImGui::SetCursorPosX(img_pos.x);
            ImGui::SetCursorPosY(img_pos.y);


            // if (testing) {
            //     // TODO fix the position
            //     // current_camera()->view = Skysphere::loader.views[pos - 1].pose.view_mat;
            //     // current_camera()->proj = Skysphere::loader.cams[0].proj;
            //     // Berechne die Inverse der View-Matrix
            //     // mat4 inverse_view = inverse(Skysphere::loader.views[pos - 1].pose.view_mat);
            //     // std::cout << " x " << inverse_view[3].x << " y " << inverse_view[3].y << " z " << inverse_view[3].z <<
            //             // std::endl;
            //
            //     // current_camera()->pos = glm::vec3(inverse_view[3].x, inverse_view[3].y, inverse_view[3].z);
            //     // current_camera()->update();
            // } else {
            //     current_camera()->update();
            // }


            // glm::mat4 rot_x = glm::rotate(glm::mat4(1), glm::radians(x_angle), glm::vec3(1.f, 0.f, 0.f));
            // glm::mat4 rot_y = glm::rotate(glm::mat4(1), glm::radians(y_angle), glm::vec3(0.f, 1.f, 0.f));
            // glm::mat4 rot_z = glm::rotate(glm::mat4(1), glm::radians(z_angle), glm::vec3(0.f, 0.f, 1.f));
            //
            // glm::mat4 rotMat = rot_z * rot_y * rot_x * mat4(1);
            //
            // inverse_view = rotMat * inverse_view;
            //
            // // std::cout << " x " << inverse_view[3].x << " y " << inverse_view[3].y << " z " << inverse_view[3].z <<
            // //         std::endl;


            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            texture_to_imgui(fbo->color_textures[0], img_size);
            // cppgl::Texture2D texture = cppgl::Texture2D("backwards_image", "outputs/envMap_mask_backwards.png", true,
            //                                             false);
            // texture_to_imgui(texture, img_size);

            camera_capture_mouse = camera_capture_mouse | ImGui::IsWindowHovered() | ImGui::IsWindowFocused();

            ImGui::SetCursorPos(ImVec2(10, 50));
            if (content_fullscreen == -1) {
                if (ImGui::Button("M", ImVec2(45, 45))) {
                    content_fullscreen = 0;
                }
            } else {
                if (ImGui::Button("M", ImVec2(45, 45))) {
                    content_fullscreen = -1;
                }
            }
        }

        ImGui::End();
    }

    // compare camera picture
    if (content_fullscreen == -1 || content_fullscreen == 1) {
        if (ImGui::Begin("Window1###Inovis1", (bool *) nullptr), ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoScrollbar) {
            auto windowsize = ImGui::GetContentRegionMax();
            vec2 img_size, img_pos;
            centeredPosSize(vec2(windowsize[0], windowsize[1]),
                            vec2((float) Skysphere::envMap->w, (float) Skysphere::envMap->h),
                            img_size,
                            img_pos);
            ImGui::SetCursorPosX(img_pos.x);
            ImGui::SetCursorPosY(img_pos.y);

            // texture_to_imgui(Skysphere::loader.views[pos - 1].image, img_size);
            texture_to_imgui(Skysphere::loader.views[indexPicture].image, img_size);

            //            camera_capture_mouse = camera_capture_mouse | ImGui::IsWindowHovered() | ImGui::IsWindowFocused();

            ImGui::SetCursorPos(ImVec2(10, 50));
            if (content_fullscreen == -1) {
                if (ImGui::Button("M", ImVec2(45, 45))) {
                    content_fullscreen = 1;
                }
            } else {
                if (ImGui::Button("M", ImVec2(45, 45))) {
                    content_fullscreen = -1;
                }
            }
        }

        ImGui::End();
    }

    // envmap
    if (content_fullscreen == -1 || content_fullscreen == 2) {
        if (ImGui::Begin("Window2###Inovis2", (bool *) nullptr), ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoScrollbar) {
            auto windowsize = ImGui::GetContentRegionMax();
            vec2 img_size, img_pos;
            centeredPosSize(vec2(windowsize[0], windowsize[1]),
                            vec2((float) Skysphere::envMap->w, (float) Skysphere::envMap->h),
                            img_size,
                            img_pos);
            ImGui::SetCursorPosX(img_pos.x);
            ImGui::SetCursorPosY(img_pos.y);

            // texture_to_imgui(Skysphere::envMap, img_size);
            texture_to_imgui(Skysphere::envMap, img_size);

            //camera_capture_mouse = camera_capture_mouse | ImGui::IsWindowHovered() | ImGui::IsWindowFocused();

            ImGui::SetCursorPos(ImVec2(10, 50));
            if (content_fullscreen == -1) {
                if (ImGui::Button("M", ImVec2(45, 45))) {
                    content_fullscreen = 2;
                }
            } else {
                if (ImGui::Button("M", ImVec2(45, 45))) {
                    content_fullscreen = -1;
                }
            }
        }

        ImGui::End();
    }

    //groundTruth
    if (content_fullscreen == -1 || content_fullscreen == 3) {
        if (ImGui::Begin("Window3###Inovis3", (bool *) nullptr), ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoScrollbar) {
            auto windowsize = ImGui::GetContentRegionMax();
            vec2 img_size, img_pos;
            centeredPosSize(vec2(windowsize[0], windowsize[1]),
                            vec2((float) Skysphere::groundTruth->w, (float) Skysphere::groundTruth->h),
                            img_size,
                            img_pos);
            ImGui::SetCursorPosX(img_pos.x);
            ImGui::SetCursorPosY(img_pos.y);

            // texture_to_imgui(Skysphere::groundTruth, img_size);
            texture_to_imgui(Skysphere::confidence, img_size);
            //camera_capture_mouse = camera_capture_mouse | ImGui::IsWindowHovered() | ImGui::IsWindowFocused();

            ImGui::SetCursorPos(ImVec2(10, 50));
            if (content_fullscreen == -1) {
                if (ImGui::Button("M", ImVec2(45, 45))) {
                    content_fullscreen = 3;
                }
            } else {
                if (ImGui::Button("M", ImVec2(45, 45))) {
                    content_fullscreen = -1;
                }
            }
        }

        ImGui::End();
    }
}
