//
// Created by flo on 11/12/24.
//
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>  // Für moderne OpenGL
#include <GLFW/glfw3.h>  // Für Fenstermanagement und OpenGL-Kontext
#include <GL/gl.h>  // Für OpenGL-Funktionen
#include <glm/gtc/matrix_transform.hpp>
#include "dataLoader.h"
#include <fstream>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

using namespace glm;

void dataLoader::load(std::shared_ptr<DatasetParams> Dparams) {
    // load camera data
    std::string poses_path = std::filesystem::path(Dparams->dataset_dir) / "poses.txt";
    // hier gerade nur eine weil die kamera nur an einer position ist
    std::string images_path = std::filesystem::path(Dparams->dataset_dir) / "images.txt";
    std::string camera_indices_path = std::filesystem::path(Dparams->dataset_dir) / "camera_indices.txt";

    // delete previously loaded dataset
    views.clear();
    cams.clear();

    // an Florian:
    // dataset_dir: ist das root directory des datensets / der scene
    // camera_files: eine liste an dateinamen die in der dataset.ini aufgelistet sind. Jedes file beschreibt die intrinsics, also die projection matrix einer kamera.
    // jedem bild wird eine bestimmte camera.ini zugewiesen (oft ist auch nur eine camera.ini vorhanden, dann verwenden alle bilder die selbe kamera)
    parseIntrinsics(Dparams->dataset_dir, Dparams->camera_files, Dparams);
    // image_dir: aus der dataset.ini
    // depth_dir und timestamps_path: das brauchst du nicht DONE
    std::cout << std::filesystem::path(Dparams->dataset_dir) << std::endl;
    std::cout << std::filesystem::path(Dparams->image_dir) << std::endl;
    std::cout << std::filesystem::path(Dparams->dataset_dir) / Dparams->image_dir << std::endl;
    parseExtrinsics(poses_path, images_path, camera_indices_path,
                    std::filesystem::path(Dparams->dataset_dir) / Dparams->image_dir);
}

std::vector<std::string> split(const std::string &s, char delim = ' ') {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

void
dataLoader::parseIntrinsics(const std::filesystem::path &dataset_dir, const std::vector<std::string> &camera_files,
                            std::shared_ptr<DatasetParams> Dparams) {
    // lese pro camera.ini die intrinsics der kamera ein
    // alle camera.ini sind gleich aufgebaut. Beispiel:
    // '''
    // [SceneCameraParams]
    // w = 1920
    // h = 1080
    // # fx fy cx cy s
    // K = 2.026787841796875e+03 2.102084716796875e+03 9.600000000000000e+02 5.400000000000000e+02 0.000000000000000e+00
    // # 8 paramter distortion model. see distortion.h
    // distortion = -2.158057093620300e-01 9.319607168436050e-02 0.000000000000000e+00 0.000000000000000e+00 0.000000000000000e+00 0.000000000000000e+00 -1.267490442842245e-03 4.837984160985798e-04
    // # c d e cx cy
    // op = 1.000000000000000e+00 0.000000000000000e+00 0.000000000000000e+00 0.000000000000000e+00 0.000000000000000e+00
    // poly_world2cam =
    // poly_cam2world =
    // ocam_cutoff = 1.0000000000
    // '''
    // alles unterhalb der zeile mit "K = ..." ist fuer dich erstmal irrelevant, da das fuer fischaugen kameras und distortion ist. Das brauchen wir bei dir nicht

    for (int i = 0; i < camera_files.size(); ++i) {
        std::filesystem::path camera_path = dataset_dir / camera_files[i];

        // open camera_file
        std::fstream camera_file;
        camera_file.open(camera_path, std::fstream::in);
        // Checking whether the file is open.
        if (!camera_file.is_open()) {
            //logError("CameraDataloader::parseExtrinsics()", "could not open poses file: " + poses_path.string());
            return;
        }
        // read lines from camera file (vector of splitted strings)
        // e.g. for width line: [0]: w, [1]: =, [2] 1920
        std::vector<std::vector<std::string> > file_lines;
        std::string cur_line;
        while (getline(camera_file, cur_line)) {
            file_lines.push_back(split(cur_line, ' '));
        }
        CameraIntrinsics newCam;
        // die datein beinhalten auch eine variante fischaugen kameras und verzerrungen zu beschreiben.
        for (auto &line: file_lines) {
            // ab hier ist wichtig
            // width and height
            if (line[0] == "w") {
                newCam.width = std::atoi(line[2].c_str()) * Dparams->render_scale;
            }
            if (line[0] == "h") {
                newCam.height = std::atoi(line[2].c_str()) * Dparams->render_scale;
            }
            // Pinhole intrinsics: K matrix
            if (line[0] == "K") {
                float fx, fy, cx, cy, s;
                fx = std::strtof(line[2].c_str(), nullptr) * Dparams->render_scale;
                fy = std::strtof(line[3].c_str(), nullptr) * Dparams->render_scale;
                cx = std::strtof(line[4].c_str(), nullptr) * Dparams->render_scale;
                cy = std::strtof(line[5].c_str(), nullptr) * Dparams->render_scale;
                s = std::strtof(line[6].c_str(), nullptr) * Dparams->render_scale;
                // hier werden die parameter in eine pinhole kamera geladen. An der stelle musst du aus diesen parametern eine projection matrix erstellen

                newCam.proj = CVCamera2GLProjectionMatrix(fx, fy, cx, cy, newCam.width, newCam.height);
            }
            // bis hier ist wichtig
        }
        cams.emplace_back(newCam);
    }
}


void dataLoader::parseExtrinsics(const std::filesystem::path &poses_path, const std::filesystem::path &images_path,
                                 const std::filesystem::path &camera_indices_path,
                                 const std::filesystem::path &image_dir) {
    // hier werden alle daten gelaten, welche spezifisch zu einem bild im datenset sind

    // hier werden die posen geladen. die befinden sich immer in der poses.txt
    // die datei wird zeile fuer zeile eingelesen und in einen vektor geschrieben
    std::fstream poses_file;
    poses_file.open(poses_path, std::fstream::in);
    // Checking whether the file is open.
    if (!poses_file.is_open()) {
        //logError("CameraDataloader::parseExtrinsics()", "could not open poses file: " + poses_path.string());
        return;
    }

    // parse config file to lines
    std::vector<std::string> poses_lines;
    std::string line;
    // Read data from the file object and put it into a string.
    while (getline(poses_file, line)) {
        poses_lines.push_back(line);
    }
    poses_file.close();


    // hier werden die bilder pfade geladen. die befinden sich immer in der images.txt
    // die datei wird zeile fuer zeile eingelesen und in einen vektor geschrieben
    std::fstream images_file;
    images_file.open(images_path, std::fstream::in);
    // Checking whether the file is open.
    if (!images_file.is_open()) {
        //logError("CameraDataloader::parseExtrinsics()", "could not open images file: " + images_path.string());
        return;
    }
    // parse config file to lines
    std::vector<std::string> images_lines;
    // Read data from the file object and put it into a string.
    while (getline(images_file, line)) {
        images_lines.push_back(line);
    }

    // hier werden die camera indices geladen. die befinden sich immer in der camera_indices.txt
    // die datei wird zeile fuer zeile eingelesen und in einen vektor geschrieben
    // jede zeile beinhaltet einen index der in die liste an camera_files zeigt die bei den intrinsics geladen werden
    // hier wird also jedem bild eine camera zugewiesen
    std::fstream camera_indices_file;
    camera_indices_file.open(camera_indices_path, std::fstream::in);
    // Checking whether the file is open.
    if (!camera_indices_file.is_open()) {
        //logError("CameraDataloader::parseExtrinsics()",                 "could not open camera_indices file: " + camera_indices_path.string());
        return;
    }
    // parse config file to lines
    std::vector<std::string> camera_indices_lines;
    // Read data from the file object and put it into a string.
    while (getline(camera_indices_file, line)) {
        camera_indices_lines.push_back(line);
    }

    // Close the file object.
    images_file.close();
    //logMessage("CameraDataloader::parseExtrinsics()","#poses: " + std::to_string(poses_lines.size()) + " #images: " + std::to_string(images_lines.size()), 1);

    // in all den eingelesenen dateien steht eine zeile fuer eine pose/bild im datenset.
    // hier werden also alle bilder durchgegangen und ein datastruct in einen vector gepackt
    for (int i = 0; i < poses_lines.size(); i++) {
        std::cout << "load image " << i << " of " << poses_lines.size() << std::endl;
        std::stringstream poses_line_stream(poses_lines[i]);

        // camera extrinsics:
        // cam to world transformation: quaternion Q (rotation) und translation T
        // projektion von camera zu world space waere fuer einen punkt P im camera space also:
        //      P_world= Q*P + T
        ////      P_cam= Q^T*(P_world-T)
        // vorsicht: in der view matrix brauchst du die andere richtung: world -> camera.
        // das heißt auch, das translation nicht die kamera position ist, sondern kamera position = -Q*T (das muesstest du nochmal ueberpruefen)
        float qx, qy, qz, qw, tx, ty, tz; // cam2world
        poses_line_stream >> qx >> qy >> qz >> qw >> tx >> ty >> tz; // order in pose_file: qx qy qz qw tx ty tz
        View view;

        quat Q = quat(qw, qx, qy, qz); // cam2world
        vec3 T = vec3(tx, ty, tz);

        mat4 rotation = toMat4(Q);
        mat4 translation = glm::translate(glm::mat4(1.0f), T);

        mat4 m = rotation * translation;

        mat4 cv2gl = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        m = m * cv2gl;

        view.pose.view_mat = inverse(m);

        view.image_file = image_dir / images_lines[i];
        cppgl::Texture2D texture = cppgl::Texture2D(images_lines[i], image_dir / images_lines[i],
                                                    image_dir.parent_path() / "masks" / images_lines[i], false,
                                                    true,
                                                    true);
        view.image = texture;
        view.cam_id = std::atoi(camera_indices_lines[i].c_str());

        views.push_back(view);
    }
}

glm::mat4 dataLoader::CVCamera2GLProjectionMatrix(float fx, float fy, float cx, float cy, int width, int height) {
    float znear = .01;
    float zfar = 1000.;
    glm::mat4 m = glm::mat4();
    m[0][0] = 2.0 * fx / width;
    m[0][1] = 0.0;
    m[0][2] = 1.0 - 2.0 * cx / width;
    m[0][3] = 0.0;

    m[1][0] = 0.0;
    m[1][1] = 2.0 * fy / height;
    m[1][2] = 2.0 * cy / height - 1.0;
    m[1][3] = 0.0;

    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = (zfar + znear) / (znear - zfar);
    m[2][3] = 2.0 * zfar * znear / (znear - zfar);

    m[3][0] = 0.0;
    m[3][1] = 0.0;
    m[3][2] = -1.0;
    m[3][3] = 0.0;
    return glm::transpose(m);
}



