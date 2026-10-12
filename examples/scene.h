//
// Created by it98eqyc on 12/10/24.
//

#ifndef SCENE_H
#define SCENE_H
#include <drawelement.h>
#include <string>

#include "assimp/scene.h"

using namespace cppgl;
using namespace std;


class Scene {
public:
    bool cloud;

    void importScene(const std::string &path);

    void draw();

private:
    vector<Drawelement> draw_elements;
};


#endif //SCENE_H
