#ifndef VIS_PRISMPIPELINE_HPP
#define VIS_PRISMPIPELINE_HPP

#include <array>

#include "glPP/Framebuffer.hpp"
#include "glPP/Texture.hpp"

namespace vis{
class PrismFramebuffer : public glPP::Framebuffer{
public:
  PrismFramebuffer();
  void init(const int& width, const int& height);
  void resizeTextures(const int& width, const int& height);
  void clearAll();
  std::array<int, 2> getFacetAndTile(const double& xPos, const double& yPos) const;
  const glPP::Texture<GL_TEXTURE_2D>& getColor() const;

private:
  glPP::Texture<GL_TEXTURE_2D> mColor;
  glPP::Texture<GL_TEXTURE_2D> mWindowId;
  glPP::Renderbuffer mDepth;
  std::array<float, 4> mBackground = {0.0f, 0.0f, 0.0f, 0.0f};
  std::array<int, 4> mBackground1 = {-1, -1, -1, -1};
  std::array<float, 4> mBackground2 = {1.0f, 1.0f, 1.0f, 1.0f};
}; // class PrismPipeline
} //namespace vis


#endif // VIS_PRISMPIPELINE_HPP
