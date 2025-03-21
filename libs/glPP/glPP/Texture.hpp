#ifndef GLPP_TEXTURE_HPP
#define GLPP_TEXTURE_HPP

#include <array>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glPP/utils.hpp"
#include <iostream>
namespace glPP {
  template<GLenum mGlTexture, class value_type = float>
  class Texture {
  friend class Framebuffer;
  public:
    static constexpr const GLsizei dimensions() {
      switch(mGlTexture) {
        case GL_TEXTURE_1D: return 1;
        case GL_TEXTURE_2D: return 2;
        case GL_TEXTURE_2D_MULTISAMPLE: return 2;
        case GL_TEXTURE_3D: return 3;
        default: return 0;
      }
    }
    static constexpr const GLsizei numBorders() {
      switch(mGlTexture) {
        case GL_TEXTURE_1D: return 1;
        case GL_TEXTURE_2D: return 2;
        case GL_TEXTURE_2D_MULTISAMPLE: return 2;
        case GL_TEXTURE_3D: return 3;
        default: return 0;
      }
    }

    inline Texture();

    template<int ns, int nf, int nb, int nc = 4>
    inline Texture(const GLenum& internalformat,
                      const GLsizei (&sizes) [ns],
                      const GLenum (&filterRules) [nf],
                      const GLenum (&borderBehaveiors) [nb],
                      const GLint& levels = 1,
                      const bool& generateMipmap = false,
                      const float (&borderColor) [nc] = {0.0f, 0.0f, 0.0f, 0.0f});

    template<int ns, int nb, int nc = 4>
    inline Texture(const GLenum& internalformat,
                      const GLsizei (&sizes) [ns],
                      const GLenum& filterRules,
                      const GLenum (&borderBehaveiors) [nb],
                      const GLint& levels = 1,
                      const bool& generateMipmap = false,
                      const float (&borderColor) [nc] = {0.0f, 0.0f, 0.0f, 0.0f});

    template<int ns, int nf, int nc = 4>
    inline Texture(const GLenum& internalformat,
                      const GLsizei (&sizes) [ns],
                      const GLenum (&filterRules) [nf],
                      const GLenum& borderBehaveiors,
                      const GLint& levels = 1,
                      const bool& generateMipmap = false,
                      const float (&borderColor) [nc] = {0.0f, 0.0f, 0.0f, 0.0f});

    template<int ns, int nc = 4>
    inline Texture(const GLenum& internalformat,
                      const GLsizei (&sizes) [ns],
                      const GLenum& filterRules,
                      const GLenum& borderBehaveiors,
                      const GLint& levels = 1,
                      const bool& generateMipmap = false,
                      const float (&borderColor) [nc] = {0.0f, 0.0f, 0.0f, 0.0f});

    template<class A, int ns, int nf, int nb, int nc = 4>
    inline Texture(const A& image, const GLenum& format,
                      const GLenum& internalformat,
                      const GLsizei (&sizes) [ns],
                      const GLenum (&filterRules) [nf],
                      const GLenum (&borderBehaveiors) [nb],
                      const GLint& levels = 1,
                      const bool& generateMipmap = false,
                      const float (&borderColor) [nc] = {0.0f, 0.0f, 0.0f, 0.0f});

    template<class A, int ns, int nb, int nc = 4>
    inline Texture(const A& image, const GLenum& format,
                      const GLenum& internalformat,
                      const GLsizei (&sizes) [ns],
                      const GLenum& filterRules,
                      const GLenum (&borderBehaveiors) [nb],
                      const GLint& levels = 1,
                      const bool& generateMipmap = false,
                      const float (&borderColor) [nc] = {0.0f, 0.0f, 0.0f, 0.0f});

    template<class A, int ns, int nf, int nc = 4>
    inline Texture(const A& image, const GLenum& format,
                      const GLenum& internalformat,
                      const GLsizei (&sizes) [ns],
                      const GLenum (&filterRules) [nf],
                      const GLenum& borderBehaveiors,
                      const GLint& levels = 1,
                      const bool& generateMipmap = false,
                      const float (&borderColor) [nc] = {0.0f, 0.0f, 0.0f, 0.0f});

    template<class A, int ns, int nc = 4>
    inline Texture(const A& image, const GLenum& format,
                      const GLenum& internalformat,
                      const GLsizei (&sizes) [ns],
                      const GLenum& filterRules,
                      const GLenum& borderBehaveiors,
                      const GLint& levels = 1,
                      const bool& generateMipmap = false,
                      const float (&borderColor) [nc] = {0.0f, 0.0f, 0.0f, 0.0f});


    constexpr Texture<mGlTexture, value_type>& operator=(Texture&& other);

    template<int n>
    inline void resizeAndClear(const GLsizei (&sizes) [n], const GLint& levels, const GLenum& internalformat);
    template<int n>
    inline void resizeAndClear(const GLsizei (&sizes) [n], const GLint& levels);
    template<int n>
    inline void resizeAndClear(const GLsizei (&sizes) [n]);

    template<int n>
    constexpr void setFilterRules(const GLenum (&filterRules) [n]);
    constexpr void setFilterRules(const GLenum& filterRules);
    constexpr void generateMipmap(const bool& generateMipmap = true);

    template<int n>
    constexpr void setBorderBehaveiors(const GLenum (&borderBehaveiors) [n]);
    constexpr void setBorderBehaveiors(const GLenum& borderBehaveiors);
    template<int n>
    constexpr void setBorderColor(const float (&borderColor) [n]);

    inline void bind(GLenum textureUnit) const;
    inline void bindImage(const GLuint& textureUnit, const GLenum& access, const GLint& level = 0) const;
    template<class A, int ns>
    constexpr void setSubImage(const A& imageArray, const GLint (&offsets) [ns], const GLsizei (&sizes) [ns], const GLint& level, const GLenum& format) const;
    template<class A, int ns>
    constexpr void setSubImage(const A& imageArray, const GLint (&offsets) [ns], const GLsizei (&sizes) [ns], const GLint& level = 0) const;
    template<class A>
    constexpr void setImage(const A& imageArray, const GLint& level, const GLenum& format) const;
    template<class A>
    constexpr void setImage(const A& imageArray, const GLint& level = 0) const;
    template<class A>
    constexpr A getImage(const GLint& level, const GLenum& format) const;
    template<class A>
    constexpr A getImage(const GLint& level = 0) const;
    template<class A, int ns>
    constexpr A getSubImage(const GLint (&offsets) [ns], const GLsizei (&sizes) [ns], const GLint& level, const GLenum& format) const;
    template<class A, int ns>
    constexpr A getSubImage(const GLint (&offsets) [ns], const GLsizei (&sizes) [ns], const GLint& level = 0) const;

    constexpr void clearImage(const GLint& level = 0) const;
    template<class A>
    constexpr void clearImage(const A& data, const GLint& level = 0) const;

    constexpr const GLsizei size() const;
    constexpr const GLsizei& size(const int& dimension) const;

    inline ~Texture();

  protected:
    GLuint mId;
    GLenum mInternalformat;
    std::array<GLsizei, dimensions()> mSizes;
    GLint mLevels;

    std::array<GLenum, 2> mFilterRules;
    bool mGenerateMipmap;

    std::array<GLenum, numBorders()> mBorderBehaveiors;
    std::array<float, 4> mBorderColor;

    inline void create();
    inline void discard();
  };

  template<GLenum mGlTexture, class value_type>
  inline void Texture<mGlTexture, value_type>::create() {
    glCreateTextures(mGlTexture, 1, &mId);

    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, mFilterRules[0]);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, mFilterRules[1]);
    if(mGenerateMipmap) glGenerateTextureMipmap(mId);

    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, mBorderBehaveiors[0]);
    if(mBorderBehaveiors.size() > 1) glTextureParameteri(mId, GL_TEXTURE_WRAP_T, mBorderBehaveiors[1]);
    if(mBorderBehaveiors.size() > 2) glTextureParameteri(mId, GL_TEXTURE_WRAP_R, mBorderBehaveiors[2]);
    glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, mBorderColor.data());

    switch(mGlTexture) {
      case GL_TEXTURE_1D:
        glTextureStorage1D(mId, mLevels, mInternalformat, mSizes[0]);
        break;
      case GL_TEXTURE_2D:
        glTextureStorage2D(mId, mLevels, mInternalformat, mSizes[0], mSizes[1]);
        break;
      case GL_TEXTURE_2D_MULTISAMPLE:
        glTextureStorage2DMultisample(mId, mLevels, mInternalformat, mSizes[0], mSizes[1], true);
        break;
      case GL_TEXTURE_3D:
        glTextureStorage3D(mId, mLevels, mInternalformat, mSizes[0], mSizes[1], mSizes[2]);
        break;
    }
  }

  template<GLenum mGlTexture, class value_type>
  inline void Texture<mGlTexture, value_type>::discard() {glDeleteTextures(1, &mId);}

  template<GLenum mGlTexture, class value_type>
  inline Texture<mGlTexture, value_type>::Texture()
  : mInternalformat(GL_R8),
    mFilterRules({GL_NEAREST, GL_NEAREST}),
    mLevels(1),
    mGenerateMipmap(false),
    mBorderColor({0.0f, 0.0f, 0.0f, 0.0f})
  {
    mSizes.fill(1);
    mBorderBehaveiors.fill(GL_CLAMP_TO_EDGE);
  }

  template<GLenum mGlTexture, class value_type>
  template<int ns, int nf, int nb, int nc>
  inline Texture<mGlTexture, value_type>::Texture(const GLenum& internalformat,
                             const GLsizei (&sizes) [ns],
                             const GLenum (&filterRules) [nf],
                             const GLenum (&borderBehaveiors) [nb],
                             const GLint& levels,
                             const bool& generateMipmap,
                             const float (&borderColor) [nc])
  : mInternalformat(internalformat),
    mSizes(std::to_array(sizes)),
    mFilterRules(std::to_array(filterRules)),
    mBorderBehaveiors(std::to_array(borderBehaveiors)),
    mLevels(levels),
    mGenerateMipmap(generateMipmap),
    mBorderColor(std::to_array(borderColor))
  {
    create();
  }

  template<GLenum mGlTexture, class value_type>
  template<int ns, int nb, int nc>
  inline Texture<mGlTexture, value_type>::Texture(const GLenum& internalformat,
                             const GLsizei (&sizes) [ns],
                             const GLenum& filterRules,
                             const GLenum (&borderBehaveiors) [nb],
                             const GLint& levels,
                             const bool& generateMipmap,
                             const float (&borderColor) [nc])
  : mInternalformat(internalformat),
    mSizes(std::to_array(sizes)),
    mFilterRules(std::to_array({filterRules, filterRules})),
    mBorderBehaveiors(std::to_array(borderBehaveiors)),
    mLevels(levels),
    mGenerateMipmap(generateMipmap),
    mBorderColor(std::to_array(borderColor))
  {
    create();
  }

  template<GLenum mGlTexture, class value_type>
  template<int ns, int nf, int nc>
  inline Texture<mGlTexture, value_type>::Texture(const GLenum& internalformat,
                             const GLsizei (&sizes) [ns],
                             const GLenum (&filterRules) [nf],
                             const GLenum& borderBehaveiors,
                             const GLint& levels,
                             const bool& generateMipmap,
                             const float (&borderColor) [nc])
  : mInternalformat(internalformat),
    mSizes(std::to_array(sizes)),
    mFilterRules(std::to_array(filterRules)),
    mLevels(levels),
    mGenerateMipmap(generateMipmap),
    mBorderColor(std::to_array(borderColor))
  {
    mBorderBehaveiors.fill(borderBehaveiors);
    create();
  }

  template<GLenum mGlTexture, class value_type>
  template<int ns, int nc>
  inline Texture<mGlTexture, value_type>::Texture(const GLenum& internalformat,
                             const GLsizei (&sizes) [ns],
                             const GLenum& filterRules,
                             const GLenum& borderBehaveiors,
                             const GLint& levels,
                             const bool& generateMipmap,
                             const float (&borderColor) [nc])
  : mInternalformat(internalformat),
    mSizes(std::to_array(sizes)),
    mFilterRules(std::to_array({filterRules, filterRules})),
    mLevels(levels),
    mGenerateMipmap(generateMipmap),
    mBorderColor(std::to_array(borderColor))
  {
    mBorderBehaveiors.fill(borderBehaveiors);
    create();
  }

  template<GLenum mGlTexture, class value_type>
  template<class A, int ns, int nf, int nb, int nc>
  inline Texture<mGlTexture, value_type>::Texture(const A& image, const GLenum& format,
                                      const GLenum& internalformat,
                                      const GLsizei (&sizes) [ns],
                                      const GLenum (&filterRules) [nf],
                                      const GLenum (&borderBehaveiors) [nb],
                                      const GLint& levels,
                                      const bool& generateMipmap,
                                      const float (&borderColor) [nc])
  : mInternalformat(internalformat),
    mSizes(std::to_array(sizes)),
    mFilterRules(std::to_array(filterRules)),
    mBorderBehaveiors(std::to_array(borderBehaveiors)),
    mLevels(levels),
    mGenerateMipmap(generateMipmap),
    mBorderColor(std::to_array(borderColor))
  {
    create();
    setImage(image, 0, format);
  }

  template<GLenum mGlTexture, class value_type>
  template<class A, int ns, int nb, int nc>
  inline Texture<mGlTexture, value_type>::Texture(const A& image, const GLenum& format,
                                      const GLenum& internalformat,
                                      const GLsizei (&sizes) [ns],
                                      const GLenum& filterRules,
                                      const GLenum (&borderBehaveiors) [nb],
                                      const GLint& levels,
                                      const bool& generateMipmap,
                                      const float (&borderColor) [nc])
  : mInternalformat(internalformat),
    mSizes(std::to_array(sizes)),
    mFilterRules(std::to_array({filterRules, filterRules})),
    mBorderBehaveiors(std::to_array(borderBehaveiors)),
    mLevels(levels),
    mGenerateMipmap(generateMipmap),
    mBorderColor(std::to_array(borderColor))
  {
    create();
    setImage(image, 0, format);
  }

  template<GLenum mGlTexture, class value_type>
  template<class A, int ns, int nf, int nc>
  inline Texture<mGlTexture, value_type>::Texture(const A& image, const GLenum& format,
                                      const GLenum& internalformat,
                                      const GLsizei (&sizes) [ns],
                                      const GLenum (&filterRules) [nf],
                                      const GLenum& borderBehaveiors,
                                      const GLint& levels,
                                      const bool& generateMipmap,
                                      const float (&borderColor) [nc])
  : mInternalformat(internalformat),
    mSizes(std::to_array(sizes)),
    mFilterRules(std::to_array(filterRules)),
    mLevels(levels),
    mGenerateMipmap(generateMipmap),
    mBorderColor(std::to_array(borderColor))
  {
    mBorderBehaveiors.fill(borderBehaveiors);
    create();
    setImage(image, 0, format);
  }

  template<GLenum mGlTexture, class value_type>
  template<class A, int ns, int nc>
  inline Texture<mGlTexture, value_type>::Texture(const A& image, const GLenum& format,
                                      const GLenum& internalformat,
                                      const GLsizei (&sizes) [ns],
                                      const GLenum& filterRules,
                                      const GLenum& borderBehaveiors,
                                      const GLint& levels,
                                      const bool& generateMipmap,
                                      const float (&borderColor) [nc])
  : mInternalformat(internalformat),
    mSizes(std::to_array(sizes)),
    mFilterRules(std::to_array({filterRules, filterRules})),
    mLevels(levels),
    mGenerateMipmap(generateMipmap),
    mBorderColor(std::to_array(borderColor))
  {
    mBorderBehaveiors.fill(borderBehaveiors);
    create();
    setImage(image, 0, format);
  }


  template<GLenum mGlTexture, class value_type>
  constexpr Texture<mGlTexture, value_type>& Texture<mGlTexture, value_type>::operator=(Texture&& other) {
    // to call rigth destructor
    auto other_mId = other.mId;
    other.mId = mId;
    mId = other_mId;

    mInternalformat = std::move(other.mInternalformat);
    mSizes = std::move(other.mSizes);

    mFilterRules = std::move(other.mFilterRules);
    mLevels = std::move(other.mLevels);
    mGenerateMipmap = std::move(other.mGenerateMipmap);

    mBorderBehaveiors = std::move(other.mBorderBehaveiors);
    mBorderColor = std::move(other.mBorderColor);

    return *this;
  }

  template<GLenum mGlTexture, class value_type>
  template<int n>
  inline void Texture<mGlTexture, value_type>::resizeAndClear(const GLsizei (&sizes) [n], const GLint& levels, const GLenum& internalformat) {
    mSizes = std::to_array(sizes);
    mLevels = levels;
    mInternalformat = internalformat;
    create();
  }

  template<GLenum mGlTexture, class value_type>
  template<int n>
  inline void Texture<mGlTexture, value_type>::resizeAndClear(const GLsizei (&sizes) [n], const GLint& levels) {
    mSizes = std::to_array(sizes);
    mLevels = levels;
    create();
  }

  template<GLenum mGlTexture, class value_type>
  template<int n>
  inline void Texture<mGlTexture, value_type>::resizeAndClear(const GLsizei (&sizes) [n]) {
    mSizes = std::to_array(sizes);
    create();
  }

  template<GLenum mGlTexture, class value_type>
  template<int n>
  constexpr void Texture<mGlTexture, value_type>::setFilterRules(const GLenum (&filterRules) [n]) {
    mFilterRules = std::to_array(filterRules);
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, mFilterRules[0]);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, mFilterRules[1]);
    if(mGenerateMipmap) glGenerateTextureMipmap(mId);
  }

  template<GLenum mGlTexture, class value_type>
  constexpr void Texture<mGlTexture, value_type>::setFilterRules(const GLenum& filterRules) {
    setFilterRules({filterRules, filterRules});
  }

  template<GLenum mGlTexture, class value_type>
  constexpr void Texture<mGlTexture, value_type>::generateMipmap(const bool& generateMipmap) {
    mGenerateMipmap = generateMipmap;
    if(mGenerateMipmap) glGenerateTextureMipmap(mId);
  }

  template<GLenum mGlTexture, class value_type>
  template<int n>
  constexpr void Texture<mGlTexture, value_type>::setBorderBehaveiors(const GLenum (&borderBehaveiors) [n]) {
    mBorderBehaveiors = std::to_array(borderBehaveiors);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, mBorderBehaveiors[0]);
    if(mBorderBehaveiors.size() > 1) glTextureParameteri(mId, GL_TEXTURE_WRAP_T, mBorderBehaveiors[1]);
    if(mBorderBehaveiors.size() > 2) glTextureParameteri(mId, GL_TEXTURE_WRAP_R, mBorderBehaveiors[2]);
    glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, mBorderColor.data());
  }

  template<GLenum mGlTexture, class value_type>
  constexpr void Texture<mGlTexture, value_type>::setBorderBehaveiors(const GLenum& borderBehaveiors) {
    switch(numBorders()) {
      case 1:
        setBorderBehaveiors({borderBehaveiors});
        return;
      case 2:
        setBorderBehaveiors({borderBehaveiors, borderBehaveiors});
        return;
      case 3:
        setBorderBehaveiors({borderBehaveiors, borderBehaveiors, borderBehaveiors});
        return;
    }
  }

  template<GLenum mGlTexture, class value_type>
  template<int n>
  constexpr void Texture<mGlTexture, value_type>::setBorderColor(const float (&borderColor) [n]) {
    mBorderColor = std::to_array(borderColor);
    glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, mBorderColor.data());
  }

  template<GLenum mGlTexture, class value_type>
  inline void Texture<mGlTexture, value_type>::bind(unsigned int textureUnit) const {
    glBindTextureUnit(textureUnit, mId);
  }

  template<GLenum mGlTexture, class value_type>
  inline void Texture<mGlTexture, value_type>::bindImage(const GLuint& textureUnit, const GLenum& access, const GLint& level) const {
    glBindImageTexture(textureUnit, mId, level, GL_FALSE, 0, access, mInternalformat);
  }

  template<GLenum mGlTexture, class value_type>
  constexpr const GLsizei Texture<mGlTexture, value_type>::size() const {
    int prod = 1;
    for(auto& elem : mSizes) prod*= elem;
    return prod;
  }

  template<GLenum mGlTexture, class value_type>
  constexpr const GLsizei& Texture<mGlTexture, value_type>::size(const int& dimension) const {return mSizes[dimension];}

  template<GLenum mGlTexture, class value_type>
  template<class A>
  constexpr A Texture<mGlTexture, value_type>::getImage(const GLint& level, const GLenum& format) const {
    A imageArray;
    imageArray.resize(this->size()*glPP::numChannels(format));
    GLenum type = glPP::enumType<typename A::value_type>();
    glGetTextureImage(mId, level, format, type,
      imageArray.size()*sizeof(typename A::value_type), imageArray.data());
    return imageArray;
  }

  template<GLenum mGlTexture, class value_type>
  template<class A>
  constexpr A Texture<mGlTexture, value_type>::getImage(const GLint& level) const {
    return getImage<A>(level, glPP::baseFormat(mInternalformat));
  }

  template<GLenum mGlTexture, class value_type>
  template<class A, int ns>
  constexpr A Texture<mGlTexture, value_type>::getSubImage(const GLint (&offsets) [ns], const GLsizei (&sizes) [ns], const GLint& level, const GLenum& format) const {
    if(mLevels <= level) throw std::string("[Error] 0");
    if(ns != mSizes.size()) throw std::string("[Error] 1");
    std::array<GLsizei, dimensions()> sizes_ = mSizes;
    for(auto& size : sizes_) for(int i = 0; i < level; ++i) size/= 2;
    for(int i = 0; i < sizes_.size(); ++i)
      if(sizes_[i] < offsets[i] + sizes[i]) throw std::string("[Error] 2");
    GLenum type = glPP::enumType<typename A::value_type>();
    int prod = 1;
    for(int i = 0; i < ns; ++i) prod*= sizes[i];
    A imageArray;
    imageArray.resize(prod*glPP::numChannels(format));
    switch(dimensions()) {
      case 1:
        glGetTextureSubImage(mId, level, offsets[0], 0, 0, sizes[0], 1, 1, format, type, imageArray.size()*sizeof(typename A::value_type), imageArray.data());
        break;
      case 2:
        glGetTextureSubImage(mId, level, offsets[0], offsets[1], 0, sizes[0], sizes[1], 1, format, type, imageArray.size()*sizeof(typename A::value_type), imageArray.data());
        break;
      case 3:
        glGetTextureSubImage(mId, level, offsets[0], offsets[1], offsets[2], sizes[0], sizes[1], sizes[2], format, type, imageArray.size()*sizeof(typename A::value_type), imageArray.data());
        break;
    }
    return imageArray;
  }
  template<GLenum mGlTexture, class value_type>
  template<class A, int ns>
  constexpr A Texture<mGlTexture, value_type>::getSubImage(const GLint (&offsets) [ns], const GLsizei (&sizes) [ns], const GLint& level) const {
    return getSubImage<A>(offsets, sizes, level, glPP::baseFormat(mInternalformat));
  }

  template<GLenum mGlTexture, class value_type>
  template<class A, int ns>
  constexpr void Texture<mGlTexture, value_type>::setSubImage(const A& imageArray, const GLint (&offsets) [ns], const GLsizei (&sizes) [ns], const GLint& level, const GLenum& format) const {
    if(mLevels <= level) throw std::string("[Error]");
    if(ns != mSizes.size()) throw std::string("[Error]");
    std::array<GLsizei, dimensions()> sizes_ = mSizes;
    for(auto& size : sizes_) for(int i = 0; i < level; ++i) size/= 2;
    for(int i = 0; i < sizes_.size(); ++i)
      if(sizes_[i] < offsets[i] + sizes[i]) throw std::string("[Error]");
    GLenum type = glPP::enumType<typename A::value_type>();
    switch(dimensions()) {
      case 1:
        glTextureSubImage1D(mId, level, offsets[0], sizes[0], format, type, imageArray.data());
        break;
      case 2:
        glTextureSubImage2D(mId, level, offsets[0], offsets[1], sizes[0], sizes[1], format, type, imageArray.data());
        break;
      case 3:
        glTextureSubImage3D(mId, level, offsets[0], offsets[1], offsets[2], sizes[0], sizes[1], sizes[2], format, type, imageArray.data());
        break;
    }
  }
  template<GLenum mGlTexture, class value_type>
  template<class A, int ns>
  constexpr void Texture<mGlTexture, value_type>::setSubImage(const A& imageArray, const GLint (&offsets) [ns], const GLsizei (&sizes) [ns], const GLint& level) const {
    setSubImage<A>(imageArray, offsets, sizes, level, glPP::baseFormat(mInternalformat));
  }

  template<GLenum mGlTexture, class value_type>
  template<class A>
  constexpr void Texture<mGlTexture, value_type>::setImage(const A& imageArray, const GLint& level, const GLenum& format) const {
    if(mLevels <= level) throw std::string("[Error]");
    std::array<GLsizei, dimensions()> sizes = mSizes;
    for(auto& size : sizes) for(int i = 0; i < level; ++i) size/= 2;
    GLenum type = glPP::enumType<typename A::value_type>();
    switch(dimensions()) {
      case 1:
        glTextureSubImage1D(mId, level, 0, sizes[0], format, type, imageArray.data());
        break;
      case 2:
        glTextureSubImage2D(mId, level, 0, 0, sizes[0], sizes[1], format, type, imageArray.data());
        break;
      case 3:
        glTextureSubImage3D(mId, level, 0, 0, 0, sizes[0], sizes[1], sizes[2], format, type, imageArray.data());
        break;
    }
  }

  template<GLenum mGlTexture, class value_type>
  template<class A>
  constexpr void Texture<mGlTexture, value_type>::setImage(const A& imageArray, const GLint& level) const {
    setImage<A>(imageArray, level, glPP::baseFormat(mInternalformat));
  }

  template<GLenum mGlTexture, class value_type>
  constexpr void Texture<mGlTexture, value_type>::clearImage(const GLint& level) const {
    GLsizei width = mSizes.size() > 0 ? mSizes[0] : 1;
    GLsizei height = mSizes.size() > 1 ? mSizes[1] : 1;
    GLsizei depth = mSizes.size() > 2 ? mSizes[2] : 1;
    GLenum format = baseFormat(mInternalformat);
// WARNING: does only work for float Textures
    GLenum type = enumType<float>();
    std::vector<float> data(numChannels(format), 0.0f);
    glClearTexSubImage(mId, level, 0, 0, 0, width, height, depth,
  	                   format, type, data.data());
  }

  template<GLenum mGlTexture, class value_type>
  template<class A>
  constexpr void Texture<mGlTexture, value_type>::clearImage(const A& data, const GLint& level) const {
    GLsizei width = mSizes.size() > 0 ? mSizes[0] : 1;
    GLsizei height = mSizes.size() > 1 ? mSizes[1] : 1;
    GLsizei depth = mSizes.size() > 2 ? mSizes[2] : 1;
    GLenum format = baseFormat(mInternalformat);
    GLenum type = glPP::enumType<typename A::value_type>();
    glClearTexSubImage(mId, level, 0, 0, 0, width, height, depth,
                       format, type, data.data());
  }

  template<GLenum mGlTexture, class value_type>
  inline Texture<mGlTexture, value_type>::~Texture() {discard();}
} // namespace glPP


#endif // GLPP_TEXTURE_HPP
