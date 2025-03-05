#ifndef TEX_HPP
#define TEX_HPP

#include <string>

namespace gl{
  constexpr std::string errorToString(const GLenum& err) {
    switch(err) {
      case GL_INVALID_ENUM: {return "GL_INVALID_ENUM";}
      case GL_INVALID_VALUE: {return "GL_INVALID_VALUE";}
      case GL_INVALID_OPERATION: {return "GL_INVALID_OPERATION";}
      case GL_STACK_OVERFLOW: {return "GL_STACK_OVERFLOW";}
      case GL_STACK_UNDERFLOW: {return "GL_STACK_UNDERFLOW";}
      case GL_OUT_OF_MEMORY: {return "GL_OUT_OF_MEMORY";}
      case GL_INVALID_FRAMEBUFFER_OPERATION: {return "GL_INVALID_FRAMEBUFFER_OPERATION";}
      case GL_CONTEXT_LOST: {return "GL_CONTEXT_LOST";}
      //case GL_TABLE_TOO_LARGE: {return "GL_TABLE_TOO_LARGE";}
      default: {return "GL_NO_ERROR";}
    }
  }

  inline std::string getLastError() {
    std::string error;
    GLenum err, lastErr;
    while((err = glGetError()) != GL_NO_ERROR) {lastErr = err;}
    return errorToString(lastErr);
  }

  inline std::string getAllErrors() {
    std::string errors;
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
      errors += errorToString(err) + ", ";
    }
    return errors;
  }

  template<class T>
  constexpr GLenum enumType(){
    if(std::is_same<T, GLbyte>::value) return GL_BYTE;
    if(std::is_same<T, GLubyte>::value) return GL_UNSIGNED_BYTE;
    if(std::is_same<T, GLshort>::value) return GL_SHORT;
    if(std::is_same<T, GLushort>::value) return GL_UNSIGNED_SHORT;
    if(std::is_same<T, GLint>::value) return GL_INT;
    if(std::is_same<T, GLuint>::value) return GL_UNSIGNED_INT;
    if(std::is_same<T, GLfixed>::value) return GL_FIXED;
    if(std::is_same<T, GLhalf>::value) return GL_HALF_FLOAT;
    if(std::is_same<T, GLfloat>::value) return GL_FLOAT;
    if(std::is_same<T, GLdouble>::value) return GL_DOUBLE;
    throw std::string("[Error](gl::enumType) Type does not exist in OpenGl");
  }
  // not complete
  constexpr int numChannels(const GLenum& baseFormat) {
    switch(baseFormat) {
      case GL_RED: return 1;
      case GL_GREEN: return 1;
      case GL_BLUE: return 1;
      case GL_RG: return 2;
      case GL_RGB: return 3;
      case GL_RGBA: return 4;
      case GL_BGR: return 3;
      case GL_BGRA: return 4;
      case GL_RED_INTEGER: return 1;
      case GL_GREEN_INTEGER: return 1;
      case GL_BLUE_INTEGER: return 1;
      case GL_RG_INTEGER: return 2;
      case GL_RGB_INTEGER: return 3;
      case GL_RGBA_INTEGER: return 4;
      case GL_BGR_INTEGER: return 3;
      case GL_BGRA_INTEGER: return 4;
      default:
        throw std::string("[Error](tex::numChannels) Format not supported");
        return GL_INVALID_ENUM;
    }
  }

  // constexpr const GLenum baseInternalFormat(const GLenum& sizedInternalFormat) {
  //   switch(sizedInternalFormat) {
  //     case GL_R8:             return GL_RED;
  //     case GL_R8_SNORM:       return GL_RED;
  //     case GL_R16:            return GL_RED;
  //     case GL_R16_SNORM:      return GL_RED;
  //     case GL_RG8:            return GL_RG;
  //     case GL_RG8_SNORM:      return GL_RG;
  //     case GL_RG16:           return GL_RG;
  //     case GL_RG16_SNORM:     return GL_RG;
  //     case GL_R3_G3_B2:       return GL_RGB;
  //     case GL_RGB4:           return GL_RGB;
  //     case GL_RGB5:           return GL_RGB;
  //     case GL_RGB8:           return GL_RGB;
  //     case GL_RGB8_SNORM:     return GL_RGB;
  //     case GL_RGB10:          return GL_RGB;
  //     case GL_RGB12:          return GL_RGB;
  //     case GL_RGB16_SNORM:    return GL_RGB;
  //     case GL_RGBA2:          return GL_RGB;
  //     case GL_RGBA4:          return GL_RGB;
  //     case GL_RGB5_A1:        return GL_RGBA;
  //     case GL_RGBA8:          return GL_RGBA;
  //     case GL_RGBA8_SNORM:    return GL_RGBA;
  //     case GL_RGB10_A2:       return GL_RGBA;
  //     case GL_RGB10_A2UI:     return GL_RGBA;
  //     case GL_RGBA12:         return GL_RGBA;
  //     case GL_RGBA16:         return GL_RGBA;
  //     case GL_SRGB8:          return GL_RGB;
  //     case GL_SRGB8_ALPHA8:   return GL_RGBA;
  //     case GL_R16F:           return GL_RED;
  //     case GL_RG16F:          return GL_RG;
  //     case GL_RGB16F:         return GL_RGB;
  //     case GL_RGBA16F:        return GL_RGBA;
  //     case GL_R32F:           return GL_RED;
  //     case GL_RG32F:          return GL_RG;
  //     case GL_RGB32F:         return GL_RGB;
  //     case GL_RGBA32F:        return GL_RGBA;
  //     case GL_R11F_G11F_B10F: return GL_RGB;
  //     case GL_RGB9_E5:        return GL_RGB;
  //     case GL_R8I:            return GL_RED;
  //     case GL_R8UI:           return GL_RED;
  //     case GL_R16I:           return GL_RED;
  //     case GL_R16UI:          return GL_RED;
  //     case GL_R32I:           return GL_RED;
  //     case GL_R32UI:          return GL_RED;
  //     case GL_RG8I:           return GL_RG;
  //     case GL_RG8UI:          return GL_RG;
  //     case GL_RG16I:          return GL_RG;
  //     case GL_RG16UI:         return GL_RG;
  //     case GL_RG32I:          return GL_RG;
  //     case GL_RG32UI:         return GL_RG;
  //     case GL_RGB8I:          return GL_RGB;
  //     case GL_RGB8UI:         return GL_RGB;
  //     case GL_RGB16I:         return GL_RGB;
  //     case GL_RGB16UI:        return GL_RGB;
  //     case GL_RGB32I:         return GL_RGB;
  //     case GL_RGB32UI:        return GL_RGB;
  //     case GL_RGBA8I:         return GL_RGBA;
  //     case GL_RGBA8UI:        return GL_RGBA;
  //     case GL_RGBA16I:        return GL_RGBA;
  //     case GL_RGBA16UI:       return GL_RGBA;
  //     case GL_RGBA32I:        return GL_RGBA;
  //     case GL_RGBA32UI:       return GL_RGBA;
  //     default:
  //       throw std::string("[Error](tex::baseInternalFormat) Format not supported");
  //       return GL_INVALID_ENUM;
  //   }
  // }

  constexpr const GLenum baseFormat(const GLenum& sizedInternalFormat) {
    switch(sizedInternalFormat) {
      case GL_R8:             return GL_RED;
      case GL_R8_SNORM:       return GL_RED;
      case GL_R16:            return GL_RED;
      case GL_R16_SNORM:      return GL_RED;
      case GL_RG8:            return GL_RG;
      case GL_RG8_SNORM:      return GL_RG;
      case GL_RG16:           return GL_RG;
      case GL_RG16_SNORM:     return GL_RG;
      case GL_R3_G3_B2:       return GL_RGB;
      case GL_RGB4:           return GL_RGB;
      case GL_RGB5:           return GL_RGB;
      case GL_RGB8:           return GL_RGB;
      case GL_RGB8_SNORM:     return GL_RGB;
      case GL_RGB10:          return GL_RGB;
      case GL_RGB12:          return GL_RGB;
      case GL_RGB16_SNORM:    return GL_RGB;
      case GL_RGBA2:          return GL_RGB;
      case GL_RGBA4:          return GL_RGB;
      case GL_RGB5_A1:        return GL_RGBA;
      case GL_RGBA8:          return GL_RGBA;
      case GL_RGBA8_SNORM:    return GL_RGBA;
      case GL_RGB10_A2:       return GL_RGBA;
      case GL_RGB10_A2UI:     return GL_RGBA;
      case GL_RGBA12:         return GL_RGBA;
      case GL_RGBA16:         return GL_RGBA;
      case GL_SRGB8:          return GL_RGB;
      case GL_SRGB8_ALPHA8:   return GL_RGBA;
      case GL_R16F:           return GL_RED;
      case GL_RG16F:          return GL_RG;
      case GL_RGB16F:         return GL_RGB;
      case GL_RGBA16F:        return GL_RGBA;
      case GL_R32F:           return GL_RED;
      case GL_RG32F:          return GL_RG;
      case GL_RGB32F:         return GL_RGB;
      case GL_RGBA32F:        return GL_RGBA;
      case GL_R11F_G11F_B10F: return GL_RGB;
      case GL_RGB9_E5:        return GL_RGB;
      case GL_R8I:            return GL_RED_INTEGER;
      case GL_R8UI:           return GL_RED_INTEGER;
      case GL_R16I:           return GL_RED_INTEGER;
      case GL_R16UI:          return GL_RED_INTEGER;
      case GL_R32I:           return GL_RED_INTEGER;
      case GL_R32UI:          return GL_RED_INTEGER;
      case GL_RG8I:           return GL_RG_INTEGER;
      case GL_RG8UI:          return GL_RG_INTEGER;
      case GL_RG16I:          return GL_RG_INTEGER;
      case GL_RG16UI:         return GL_RG_INTEGER;
      case GL_RG32I:          return GL_RG_INTEGER;
      case GL_RG32UI:         return GL_RG_INTEGER;
      case GL_RGB8I:          return GL_RGB_INTEGER;
      case GL_RGB8UI:         return GL_RGB_INTEGER;
      case GL_RGB16I:         return GL_RGB_INTEGER;
      case GL_RGB16UI:        return GL_RGB_INTEGER;
      case GL_RGB32I:         return GL_RGB_INTEGER;
      case GL_RGB32UI:        return GL_RGB_INTEGER;
      case GL_RGBA8I:         return GL_RGBA_INTEGER;
      case GL_RGBA8UI:        return GL_RGBA_INTEGER;
      case GL_RGBA16I:        return GL_RGBA_INTEGER;
      case GL_RGBA16UI:       return GL_RGBA_INTEGER;
      case GL_RGBA32I:        return GL_RGBA_INTEGER;
      case GL_RGBA32UI:       return GL_RGBA_INTEGER;
      default:
        throw std::string("[Error](tex::baseFormat) Format not supported");
        return GL_INVALID_ENUM;
    }
  }
}

#endif // TEX_HPP
