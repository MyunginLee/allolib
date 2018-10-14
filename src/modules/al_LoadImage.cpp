#include <cstring> // memcpy

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"

#include "al/modules/al_LoadImage.hpp"

using namespace std;

al::ImageData al::loadImage(const char* filename) {
    ImageData image_data;
    int w, h, n;
    // n will contain number of channels originally in image file
    // last parameter '4': force 4 channels
    unsigned char *data = stbi_load(filename, &w, &h, &n, 4);
    if (!data) return image_data; // empty object (size of data = 0)
    image_data.width = w;
    image_data.height = h;
    image_data.data.resize(image_data.width * image_data.height * 4);
    memcpy(image_data.data.data(), data, image_data.width * image_data.height * 4);
    stbi_image_free(data);
    return image_data;
}

al::ImageData al::loadImage(string &filename) {
    return al::loadImage(filename.c_str());
}
