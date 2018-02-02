#ifndef INCLUDE_AL_GLFW_HPP
#define INCLUDE_AL_GLFW_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/graphics/al_GLEW.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace al {

namespace glfw {
    void init(bool is_verbose=false);
    void terminate(bool is_verbose=false);
    inline GLFWwindow* current_window() { return glfwGetCurrentContext(); }
}

}

#endif