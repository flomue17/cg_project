//
// Created by flo on 11/12/24.
//

#ifndef CPPGL_DATALOADER_H
#define CPPGL_DATALOADER_H
#include <filesystem>
#include <texture.h>
#include <vector>
#include "cameraData.h"
#include "dataset_params.h"

class View {
public:
    CameraExtrinsics pose;
    int cam_id = -1;
    std::filesystem::path image_file;
    cppgl::Texture2D image;
};

class dataLoader {
public:
    std::vector<View> views;
    std::vector<CameraIntrinsics> cams;

    dataLoader() = default;

    ~dataLoader() = default;

    void load(std::shared_ptr<DatasetParams> Dparams);

    [[nodiscard]] inline size_t size() const { return views.size(); }
    inline View &get(int i) { return views[i]; }
    inline View &operator()(int i) { return views[i]; }

private:
    inline void parseIntrinsics(const std::filesystem::path &dataset_dir, const std::vector<std::string> &camera_files,
                                std::shared_ptr<DatasetParams> Dparams);

    inline void parseExtrinsics(const std::filesystem::path &poses_path, const std::filesystem::path &images_path,
                                const std::filesystem::path &camera_indices_path,
                                const std::filesystem::path &image_dir);

    inline glm::mat4 CVCamera2GLProjectionMatrix(float fx, float fy, float cx, float cy, int width, int height);
};


#endif //CPPGL_DATALOADER_H
