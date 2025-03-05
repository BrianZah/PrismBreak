#ifndef GL_BUFFER_HPP
#define GL_BUFFER_HPP

#include <glad/glad.h>

namespace gl{
  class Buffer{
    friend class VertexArray;
  private:
    GLuint mId;
    GLbitfield mFlags = 0;

    inline void create();
    inline void discard();
  public:
    inline Buffer();
    inline Buffer(const GLsizei& size, const GLbitfield& flags = 0);
    template<class A>
    inline Buffer(const A& array, const GLbitfield& flags = 0);
    inline void resizeAndClear(const GLsizei& size);
    template<class A>
    inline void setData(const A& array, const GLbitfield& flags = 0);
    inline void bind(const GLenum& type, const GLuint& index) const;
    template<class A>
    inline void setSubData(const A& array, const GLint& offset, const GLsizei& count) const;
    template<class A>
    inline void setSubData(const A& array, const GLint& offset = 0) const;
    inline ~Buffer();
  };

  inline void Buffer::create() {glCreateBuffers(1, &mId);}
  inline void Buffer::discard() {glDeleteBuffers(1, &mId);}

  inline Buffer::Buffer() {create();}

  inline Buffer::Buffer(const GLsizei& size, const GLbitfield& flags)
  : mFlags(flags)
  {
    create();
    glNamedBufferStorage(mId, size, nullptr, mFlags);
  }
  template<class A>
  inline Buffer::Buffer(const A& array, const GLbitfield& flags)
  : mFlags(flags)
  {
    create();
    glNamedBufferStorage(mId, array.size()*sizeof(typename A::value_type), array.data(), mFlags);
  }

  inline void Buffer::resizeAndClear(const GLsizei& size) {
    discard();
    create();
    glNamedBufferStorage(mId, size, nullptr, mFlags | GL_DYNAMIC_STORAGE_BIT);
  }
  template<class A>
  inline void Buffer::setData(const A& array, const GLbitfield& flags) {
    discard();
    create();
    mFlags = flags;
    glNamedBufferStorage(mId, array.size()*sizeof(typename A::value_type), array.data(), mFlags);
  }
  inline void Buffer::bind(const GLenum& type, const GLuint& index) const {
    glBindBufferBase(type, index, mId);
  }

  template<class A>
  inline void Buffer::setSubData(const A& array, const GLint& offset, const GLsizei& count) const {
    glNamedBufferSubData(mId, offset, count*sizeof(typename A::value_type), array.data());
  }
  template<class A>
  inline void Buffer::setSubData(const A& array, const GLint& offset) const {
    setSubData(array, offset, array.size());
  }

  Buffer::~Buffer() {glDeleteBuffers(1, &mId);}
} // namespace gl

#endif // GL_BUFFER_HPP
