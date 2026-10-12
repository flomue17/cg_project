#pragma once

#include <cppgl.h>

using namespace std;

#ifndef GUI_H
#define GUI_H

class Gui {
public:
     static void drawGUI(cppgl::Framebuffer fbo);

     static void drawElements(cppgl::Framebuffer fbo);

     static bool camera_capture_mouse;
     static int pointSize;

private:
};


#endif //GUI_H
