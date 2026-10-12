//
// Created by flo on 11/17/24.
//

#include "dataset_params.h"
#include <iostream>

// Konstruktor
DatasetParams::DatasetParams(const std::string& dataset_dir,
                             const std::vector<std::string>& camera_files,
                             const std::string& image_dir,
                             float render_scale)
        : dataset_dir(dataset_dir), camera_files(camera_files), image_dir(image_dir), render_scale(render_scale) {}
