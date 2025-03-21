#ifndef PASS_BASE_HPP
#define PASS_BASE_HPP

#include "glPP/Shader.hpp"

namespace pass {
  class Base{
  protected:
    glPP::Shader mShader;
  public:
    virtual void resizeTextures(const int& width, const int& height) = 0;
    virtual void setFixUniforms() const = 0;
    virtual void execute() const = 0;
    virtual void refresh() const {
      mShader.refresh();
      setFixUniforms();
    }
    inline Base(const std::initializer_list<glPP::Shader::Specs>& specs = {}) : mShader(specs) {}
  }; // class Base
} // namespace pass

#endif // PASS_BASE_HPP
