#include "../al_GLAD.hpp"
#include "glad.c"

static bool AL_GLAD_INITIALIZED = false;

bool alInitGLAD() {
  if (AL_GLAD_INITIALIZED) return true;
  if (gladLoadGL()) {
    AL_GLAD_INITIALIZED = true;
    return true;
  }
  else return false;
}

bool alIsGLLoaded() {
    return AL_GLAD_INITIALIZED;
}

