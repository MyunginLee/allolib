#include "al/core/graphics/al_Mesh.hpp"
#include "al/core/graphics/al_Texture.hpp"
#include "al/core/graphics/al_Graphics.hpp"
#include "al/modules/al_LoadFont.hpp"
#include <unordered_map>

namespace al {

enum struct TEXT_ALIGN {
    LEFT, CENTER, RIGHT
};

struct FontModule {
    Texture fontTex;
    al::FontData fontData;
    Mesh textMesh {Mesh::TRIANGLES};
    std::unordered_map<int, al::CharData> cachedCharData;
    float alignFactorX = 0;
    float alignFactorY = 0;

    // size: height of font in texture, values larger than 128 might result
    //       not all chars fitting in the font texture >> TODO
    bool load(const char* filename, float size = 128); 

    void render(Graphics& g, const std::string text, float height = 1);

    // height: height of text in OpenGL space units
    // TODO: separate rendering and mesh generation so the user can get mesh data without rendering
    void render(Graphics& g, const char* text, float height = 1);

    // TODO: vertical align? might need to change font module implementation
    //       to advanved interface of stbtt
    void align(TEXT_ALIGN horizontalAlign);
};

}

inline bool al::FontModule::load(const char* filename, float size) {
    fontData = al::loadFont(filename, size);
    if (fontData.charData.size() == 0) {
        return false;
    }
    fontTex.create2D(fontData.width, fontData.height, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
    fontTex.submit(fontData.bitmap.data());
    fontTex.filter(GL_LINEAR);
    fontTex.bind_temp();
    // make `texture` in glsl return (1, 1, 1, r)
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    fontTex.unbind_temp();
    return true;
}

inline void al::FontModule::render(Graphics& g, const std::string text, float height) {
    al::FontModule::render(g, text.c_str(), height);
}

// mod ver of al::Font::write
inline void al::FontModule::render(Graphics& g, const char* text, float height) {
    textMesh.reset();

    float scale = height / fontData.pixelHeight;
    float xpos = 0;

    auto cdata = [this](int c) -> const al::CharData& {
        auto search = cachedCharData.find(c);
        if (search != cachedCharData.end()) return search->second;
        else return (cachedCharData[c] = al::getCharData(fontData, c));
    };

    while (*text) {
        int c = *text;
        auto d = cdata(c);

        float x0 = xpos + d.x0 * scale;
        float x1 = xpos + d.x1 * scale;
        float y0 = - d.y0 * scale;
        float y1 = - d.y1 * scale;

		textMesh.vertex(x0, y0, 0);
		textMesh.vertex(x1, y0, 0);
		textMesh.vertex(x0, y1, 0);
		textMesh.vertex(x0, y1, 0);
		textMesh.vertex(x1, y0, 0);
		textMesh.vertex(x1, y1, 0);

		textMesh.texCoord(d.s0,	d.t0);
		textMesh.texCoord(d.s1,	d.t0);
		textMesh.texCoord(d.s0,	d.t1);
		textMesh.texCoord(d.s0,	d.t1);
		textMesh.texCoord(d.s1,	d.t0);
		textMesh.texCoord(d.s1,	d.t1);

        xpos += (d.xAdvance * scale);

        ++text;
    }

    float xOffset = xpos * alignFactorX;
    for (auto& v : textMesh.vertices()) {
        v.x = v.x + xOffset;
    }

    fontTex.bind(0);
    g.texture();
    g.draw(textMesh);
    fontTex.unbind(0);
}

inline void al::FontModule::align(TEXT_ALIGN horizontalAlign) {
    switch (horizontalAlign) {
        case TEXT_ALIGN::RIGHT: alignFactorX = -1; break;
        case TEXT_ALIGN::CENTER: alignFactorX = -0.5; break;
        case TEXT_ALIGN::LEFT: alignFactorX = 0; break;
    }
}

