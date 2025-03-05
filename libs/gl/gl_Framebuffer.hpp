#ifndef GL_FRAMEBUFFER_HPP
#define GL_FRAMEBUFFER_HPP

#include <glad/glad.h>
#include "gl_Texture.hpp"

namespace gl{
  class Renderbuffer{
    friend class Framebuffer;
  private:
    GLuint mId;
    GLenum mInternalformat;
    GLsizei mSamples;
  public:
    inline Renderbuffer(const GLenum& internalformat, const GLsizei& width, const GLsizei& height, const GLsizei& samples = 1);
    inline Renderbuffer();
    inline Renderbuffer& operator=(Renderbuffer&& other);
    inline void resizeAndClear(const GLsizei& width, const GLsizei& height);
    inline ~Renderbuffer();
  };
  inline Renderbuffer::Renderbuffer(const GLenum& internalformat, const GLsizei& width, const GLsizei& height, const GLsizei& samples)
  : mInternalformat(internalformat), mSamples(samples)
  {
    glCreateRenderbuffers(1, &mId);
    if(samples == 1) {
      glNamedRenderbufferStorage(mId, mInternalformat, width, height);
    } else {
      glNamedRenderbufferStorageMultisample(mId, mInternalformat, width, height, samples);
    }
  }
  inline Renderbuffer::Renderbuffer() : Renderbuffer(GL_DEPTH24_STENCIL8, 1, 1) {}
  inline Renderbuffer& Renderbuffer::operator=(Renderbuffer&& other) {
    GLuint old_mId = mId;
    mId = other.mId;
    mInternalformat = other.mInternalformat;
    mSamples = other.mSamples;
    other.mId = old_mId;
    return *this;
  }
  inline void Renderbuffer::resizeAndClear(const GLsizei& width, const GLsizei& height) {
    *this = Renderbuffer(mInternalformat, width, height, mSamples);
  }
  inline Renderbuffer::~Renderbuffer() {glDeleteRenderbuffers(1, &mId);}

  class Framebuffer{
  private:
    GLuint mId;
    //std::vector<GLenum> mDrawBuffers;
  public:
    inline Framebuffer();
    inline void bind(const GLenum& target = GL_FRAMEBUFFER) const;
    inline void unbind() const;
    inline void attach(const gl::Texture<GL_TEXTURE_1D>& texture, const GLenum& attachment, const GLint& level = 0);
    inline void attach(const gl::Texture<GL_TEXTURE_2D>& texture, const GLenum& attachment, const GLint& level = 0);
    inline void attach(const gl::Texture<GL_TEXTURE_2D_MULTISAMPLE>& texture, const GLenum& attachment, const GLint& level = 0);
    template<std::convertible_to<GLenum>... glEnum>
    inline void drawBuffers(const glEnum& ...drawBuffers) const;
    inline void attach(const gl::Renderbuffer& renderbuffer, const GLenum& attachment);
    inline bool check() const;

    inline void clear(const GLenum& buffer, const GLfloat *value, const GLint& drawbuffer = 0) const;
    inline void clear(const GLenum& buffer, const GLint *value, const GLint& drawbuffer = 0) const;
    inline ~Framebuffer();
  };

  inline Framebuffer::Framebuffer() {glCreateFramebuffers(1, &mId);}

  inline void Framebuffer::bind(const GLenum& target) const {
    glBindFramebuffer(target, mId);
  }
  inline void Framebuffer::unbind() const {glBindFramebuffer(GL_FRAMEBUFFER, 0);}
  inline void Framebuffer::attach(const gl::Texture<GL_TEXTURE_1D>& texture, const GLenum& attachment, const GLint& level) {
    glNamedFramebufferTexture(mId, attachment, texture.mId, level);
    //mDrawBuffers.push_back(attachment);
    //glNamedFramebufferDrawBuffer(mId, attachment);
  }
  inline void Framebuffer::attach(const gl::Texture<GL_TEXTURE_2D>& texture, const GLenum& attachment, const GLint& level) {
    glNamedFramebufferTexture(mId, attachment, texture.mId, level);
    //mDrawBuffers.push_back(attachment);
    //glNamedFramebufferDrawBuffer(mId, attachment);
  }
  inline void Framebuffer::attach(const gl::Texture<GL_TEXTURE_2D_MULTISAMPLE>& texture, const GLenum& attachment, const GLint& level) {
    glNamedFramebufferTexture(mId, attachment, texture.mId, level);
    //mDrawBuffers.push_back(attachment);
    //glNamedFramebufferDrawBuffer(mId, attachment);
  }
  template<std::convertible_to<GLenum>... glEnum>
  inline void Framebuffer::drawBuffers(const glEnum& ...drawBuffers) const {
    std::vector<GLenum> drawBuffersA = {static_cast<GLenum>(drawBuffers)...};
    glNamedFramebufferDrawBuffers(mId, drawBuffersA.size(), drawBuffersA.data());
  }
  //template<class Array = std::vector<GLenum>>
  //inline void Framebuffer::drawBuffers(const Array& drawBuffers) const {
  //  glNamedFramebufferDrawBuffers(mId, drawBuffers.size(), drawBuffers.data());
    //mDrawBuffers.resize(0);
  //}
  inline void Framebuffer::attach(const gl::Renderbuffer& renderbuffer, const GLenum& attachment) {
    glNamedFramebufferRenderbuffer(mId, attachment,  GL_RENDERBUFFER, renderbuffer.mId);
    //mDrawBuffers.push_back(attachment);
  }
  inline bool Framebuffer::check() const {
    return glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE;
  }
  inline void Framebuffer::clear(const GLenum& buffer, const GLfloat *value, const GLint& drawbuffer) const {
    glClearNamedFramebufferfv(mId, buffer, drawbuffer, value);
  }
  inline void Framebuffer::clear(const GLenum& buffer, const GLint *value, const GLint& drawbuffer) const {
    glClearNamedFramebufferiv(mId, buffer, drawbuffer, value);
  }
  Framebuffer::~Framebuffer() {glDeleteFramebuffers(1, &mId);}
} // namespace gl

#endif // GL_FRAMEBUFFER_HPP
