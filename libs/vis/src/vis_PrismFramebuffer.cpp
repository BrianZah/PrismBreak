#include "vis_PrismFramebuffer.hpp"

#include <array>
#include <vector>
#include <algorithm>
#include <iostream>

#include "gl_Framebuffer.hpp"
#include "gl_Texture.hpp"

namespace vis{

PrismFramebuffer::PrismFramebuffer()
: Framebuffer(), mColor(), mWindowId()
{}

void PrismFramebuffer::init(const int& width, const int& height) {
  mColor = gl::Texture<GL_TEXTURE_2D>(GL_RGBA16F, {width, height}, GL_NEAREST, GL_CLAMP_TO_EDGE);
  mWindowId = gl::Texture<GL_TEXTURE_2D>(GL_RG8I, {width, height}, GL_NEAREST, GL_CLAMP_TO_EDGE);
  attach(mColor, GL_COLOR_ATTACHMENT0);
  attach(mWindowId, GL_COLOR_ATTACHMENT1);
  drawBuffers(GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1);

  mDepth = gl::Renderbuffer(GL_DEPTH_COMPONENT32F, width, height);
  attach(mDepth, GL_DEPTH_ATTACHMENT);
}

void PrismFramebuffer::resizeTextures(const int& width, const int& height) {
  mColor.resizeAndClear({width, height});
  mWindowId.resizeAndClear({width, height});
  mDepth.resizeAndClear(width, height);
  attach(mColor, GL_COLOR_ATTACHMENT0);
  attach(mWindowId, GL_COLOR_ATTACHMENT1);
  drawBuffers(GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1);
  attach(mDepth, GL_DEPTH_ATTACHMENT);
}

void PrismFramebuffer::clearAll() {
  //bind();
  clear(GL_COLOR, mBackground.data(), 0);
  clear(GL_COLOR, mBackground1.data(), 1);
  clear(GL_DEPTH, mBackground2.data(), 0);
  //unbind();
}

std::array<int, 2> PrismFramebuffer::getFacetAndTile(const double& xPos, const double& yPos) const {
  int facet = -1, tile = -1;
  try{
    bind();
    auto v = mWindowId.getSubImage<std::vector<int>>({xPos, ((mWindowId.size(1)-1)-yPos)}, {1, 1});
    unbind();
    facet = v[0];
    tile = v[1];
    std::cout << "[Info](PrismFramebuffer::getIndexDistributionAndIndexComponent) v.size() = " << v.size() << std::endl;
    std::cout << "[Info](PrismFramebuffer::getIndexDistributionAndIndexComponent) facet, tile = " << facet << ", " << tile << std::endl;
  } catch(std::string msg) {
    std::cout << msg << std::endl;
  }
  return {facet, tile};
}

const gl::Texture<GL_TEXTURE_2D>& PrismFramebuffer::getColor() const {
  return mColor;
}

} //namespace vis
