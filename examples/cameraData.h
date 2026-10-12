/**
 * Copyright (c) 2023 Mathias Harrer
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "glm/glm.hpp"
#include <glm/gtx/quaternion.hpp>

using namespace glm;

struct CameraExtrinsics {
    mat4 view_mat;
};


struct CameraIntrinsics {
    glm::mat4 proj = {};
    int width = 0;
    int height = 0;
};

struct CameraInfo {
    CameraIntrinsics intrinsics;
    CameraExtrinsics extrinsics;
};
