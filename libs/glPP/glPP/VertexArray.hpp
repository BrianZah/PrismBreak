#ifndef GLPP_VERTEXARRAY_HPP
#define GLPP_VERTEXARRAY_HPP

#include <glad/glad.h>
#include "glPP/Buffer.hpp"

namespace glPP{
  class VertexArray{
  public:
    inline VertexArray();
    inline void setAttribute(const GLuint& index, const GLint& size, const GLenum& type, const GLuint& relativeOffset);
    inline void bindVertexBuffer(const glPP::Buffer& buffer, const GLuint& bindingIndex, const GLsizei& stride, const GLint& offset = 0) const;
    inline void bindElementBuffer(const glPP::Buffer& buffer) const;
    inline void drawArrays(const GLenum& mode, const GLint& first, const GLsizei& count) const;
    template<class T, class A = std::vector<unsigned int>>
    inline void drawElements(const GLenum& mode, const GLsizei& count, const A& indices = {}) const;
    inline ~VertexArray();
  private:
    GLuint mId;
    std::vector<GLuint> mAttributeIndices;
  };
  inline VertexArray::VertexArray() {
    glCreateVertexArrays(1, &mId);
  }
  inline void VertexArray::setAttribute(const GLuint& index, const GLint& size, const GLenum& type, const GLuint& relativeOffset) {
    glEnableVertexArrayAttrib(mId, index);
    glVertexArrayAttribFormat(mId, index, size, type, GL_FALSE, relativeOffset);
    mAttributeIndices.push_back(index);
  }
  inline void VertexArray::bindVertexBuffer(const glPP::Buffer& buffer, const GLuint& bindingIndex, const GLsizei& stride, const GLint& offset) const {
    for(const auto& index : mAttributeIndices) glVertexArrayAttribBinding(mId, index, bindingIndex);
    glVertexArrayVertexBuffer(mId, bindingIndex, buffer.mId, offset, stride);
  }
  inline void VertexArray::bindElementBuffer(const glPP::Buffer& buffer) const {
    glVertexArrayElementBuffer(mId, buffer.mId);
  }
  inline void VertexArray::drawArrays(const GLenum& mode, const GLint& first, const GLsizei& count) const {
    glBindVertexArray(mId);
    glDrawArrays(mode, first, count);
    glBindVertexArray(0);
  }
  template<class T, class A>
  inline void VertexArray::drawElements(const GLenum& mode, const GLsizei& count, const A& indices) const {
    glBindVertexArray(mId);
    glDrawElements(mode, count, enumType<T>(), indices.empty() ? nullptr : indices.data());
    glBindVertexArray(0);
  }

  inline VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &mId);
  }
} // namespace glPP

#endif // GLPP_VERTEXARRAY_HPP
