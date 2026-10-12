//
// Created by flo on 11/17/24.
//

#ifndef DATASET_PARAMS_H
#define DATASET_PARAMS_H

#include <string>
#include <memory> // Für std::shared_ptr
#include "vector"

class DatasetParams {
public:
    // Konstruktor
    DatasetParams(const std::string& dataset_dir,
                  const std::vector<std::string>& camera_files,
                  const std::string& image_dir,
                  float render_scale);

    // Attribute
    std::string dataset_dir;
    std::vector<std::string> camera_files;
    std::string image_dir;
    float render_scale;
};

#endif // DATASET_PARAMS_H
