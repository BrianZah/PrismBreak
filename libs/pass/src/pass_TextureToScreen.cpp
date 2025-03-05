#include "pass_Base.hpp"
#include "pass_TextureToScreen.hpp"

#include <array>

#include "gl_Shader.hpp"
#include "gl_Texture.hpp"

namespace pass {
TextureToScreen::TextureToScreen(const gl::Texture<GL_TEXTURE_2D>* screenTexture)
: Base({
    gl::Shader::Specs(GL_VERTEX_SHADER, std::string(CMAKE_SHADERDIR) + "screenTexture.vs"),
    gl::Shader::Specs(GL_FRAGMENT_SHADER, std::string(CMAKE_SHADERDIR) + "screenTexture.fs")
  }),
  mQuad(Quad()), mScreenTexture(screenTexture)
{
  setFixUniforms();
}

TextureToScreen::TextureToScreen() : mScreenTexture(nullptr) {}

void TextureToScreen::init(const gl::Texture<GL_TEXTURE_2D>* screenTexture) {
  mShader.setSpecs({
    gl::Shader::Specs(GL_VERTEX_SHADER, std::string(CMAKE_SHADERDIR) + "screenTexture.vs"),
    gl::Shader::Specs(GL_FRAGMENT_SHADER, std::string(CMAKE_SHADERDIR) + "screenTexture.fs")
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

void TextureToScreen::setScreenTexture(const gl::Texture<GL_TEXTURE_2D>* screenTexture) {
  mScreenTexture = screenTexture;
}
} // namespace pass
