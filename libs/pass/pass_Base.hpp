#ifndef PASS_BASE_HPP
#define PASS_BASE_HPP

#include "gl_Shader.hpp"

namespace pass {
  class Base{
  protected:
    gl::Shader mShader;
  public:
    virtual void resizeTextures(const int& width, const int& height) = 0;
    virtual void setFixUniforms() const = 0;
    virtual void execute() const = 0;
    virtual void refresh() const {
      mShader.refresh();
      setFixUniforms();
    }
    //virtual void enter() const = 0;
    inline Base(const std::initializer_list<gl::Shader::Specs>& specs = {}) : mShader(specs) {}
  }; // class Base
} // namespace pass

#endif // PASS_BASE_HPP
