#ifndef PASS_TEXTURETOSCREEN_HPP
#define PASS_TEXTURETOSCREEN_HPP

#include "pass_Base.hpp"

#include <array>

#include "gl_Shader.hpp"
#include "gl_Texture.hpp"
#include "gl_Buffer.hpp"
#include "gl_VertexArray.hpp"

namespace pass {

class TextureToScreen : public Base{
public:
  TextureToScreen(const gl::Texture<GL_TEXTURE_2D>* screenTexture);
  TextureToScreen();
  void init(const gl::Texture<GL_TEXTURE_2D>* screenTexture);

  void resizeTextures(const int& width, const int& height) override;
  void setFixUniforms() const override;
  void execute() const override;

  void setScreenTexture(const gl::Texture<GL_TEXTURE_2D>* screenTexture);
private:
  class Quad {
  private:
    gl::Buffer buffer;
    gl::VertexArray va;
  public:
    Quad()
    : va(),
      buffer(std::array<float, 20>{// positions,       texture Coords
                                   -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                                   -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                                    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                                    1.0f, -1.0f, 0.0f, 1.0f, 0.0f})
    {
      va.setAttribute(0, 3, GL_FLOAT, 0);
      va.setAttribute(1, 2, GL_FLOAT, 3*sizeof(float));
      va.bindVertexBuffer(buffer, 0, 5*sizeof(float));
    }

    void render() const {
      va.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
  };

  const Quad mQuad;
  std::array<float, 4> mBackground = {1.0f, 1.0f, 1.0f, 1.0f};
  const gl::Texture<GL_TEXTURE_2D>* mScreenTexture;
};
} // namespace pass
#endif // PASS_TEXTURETOSCREEN_HPP
