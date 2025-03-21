#include "pass/Base.hpp"
#include "pass/TextureToScreen.hpp"

#include <array>

#include "glPP/Shader.hpp"
#include "glPP/Texture.hpp"

namespace pass {
TextureToScreen::TextureToScreen(const glPP::Texture<GL_TEXTURE_2D>* screenTexture)
: Base({
    glPP::Shader::Specs(GL_VERTEX_SHADER, std::string(CMAKE_SHADERDIR) + "screenTexture.vs"),
    glPP::Shader::Specs(GL_FRAGMENT_SHADER, std::string(CMAKE_SHADERDIR) + "screenTexture.fs")
  }),
  mQuad(Quad()), mScreenTexture(screenTexture)
{
  setFixUniforms();
}

TextureToScreen::TextureToScreen() : mScreenTexture(nullptr) {}

void TextureToScreen::init(const glPP::Texture<GL_TEXTURE_2D>* screenTexture) {
  mShader.setSpecs({
    glPP::Shader::Specs(GL_VERTEX_SHADER, std::string(CMAKE_SHADERDIR) + "screenTexture.vs"),
    glPP::Shader::Specs(GL_FRAGMENT_SHADER, std::string(CMAKE_SHADERDIR) + "screenTexture.fs")
  });
  mShader.refresh();
  mScreenTexture = screenTexture;
  setFixUniforms();
}

void TextureToScreen::resizeTextures(const int& width, const int& height) {}

void TextureToScreen::setFixUniforms() const {
  mShader.use();
  mShader.setInt("screenTexture", 0);
}

void TextureToScreen::execute() const {
  glClearNamedFramebufferfv(0, GL_COLOR, 0, mBackground.data());

  mShader.use();
  mScreenTexture->bind(0);
  mQuad.render();
}

void TextureToScreen::setScreenTexture(const glPP::Texture<GL_TEXTURE_2D>* screenTexture) {
  mScreenTexture = screenTexture;
}
} // namespace pass
