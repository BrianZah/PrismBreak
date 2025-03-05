#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <list>
#include <vector>
#include <initializer_list>

#include <fstream>
#include <sstream>
#include <iostream>

namespace gl{
class Shader{
public:
  class Specs{
  friend class Shader;
  private:
    GLenum mType;
    std::string mPath;
    std::string mMacros;
    inline std::string toString(const std::list<std::string>& strs, const std::string& seperator = " ") const;

  public:
    template<std::convertible_to<std::string>... String>
    Specs(const GLenum& type, const std::string& path, const String& ...macros);
  };

public:
  inline Shader(const std::initializer_list<Specs>& specs = {});

  inline void setSpecs(const std::initializer_list<Specs>& specs);
  inline void refresh() const;
  inline void use() const;
  inline ~Shader();

//protected:
private:
  mutable GLuint mId;
  std::vector<Specs> mSpecs;
  //mutable std::vector<unsigned int> subroutineIndices;

  inline std::string shaderCode(const std::string& path, const std::string& macros = "", const bool& firstRecursion = true) const;
  inline void checkCompileErrors(GLuint shader, std::string type) const;
  inline void init() const;
  inline void initEmpty() const;
public:
  // utility uniform functions
  inline void setBool(const std::string& name, const bool& value) const;
  inline void setInt(const std::string& name, const int& value) const;

  inline void setIvec2(const std::string& name, const int x, const int y) const;
  inline void setIvec2(const std::string& name, const glm::ivec2& value) const;
  template<class Array>
  inline void setIvec2(const std::string& name, const Array& value, const GLsizei& count = 1) const;

  inline void setIvec3(const std::string& name, const int x, const int y, const int z) const;
  inline void setIvec3(const std::string& name, const glm::ivec3& value) const;
  template<class Array>
  inline void setIvec3(const std::string& name, const Array& value, const GLsizei& count = 1) const;

  inline void setIvec4(const std::string& name, const int x, const int y, const int z, const int w) const;
  inline void setIvec4(const std::string& name, const glm::ivec4& value) const;

  inline void setUint(const std::string& name, const unsigned int& value) const;

  inline void setFloat(const std::string& name, const float& value) const;
  template<class Array>
  inline void setFloat(const std::string& name, const Array& value, const GLsizei& count = 1) const;

  inline void setVec2(const std::string& name, const float x, const float y) const;
  inline void setVec2(const std::string& name, const glm::vec2& value) const;
  template<class Array>
  inline void setVec2(const std::string& name, const Array& value, const GLsizei& count = 1) const;

  inline void setVec3(const std::string& name, const float x, const float y, const float z) const;
  inline void setVec3(const std::string& name, const glm::vec3& value) const;
  template<class Array>
  inline void setVec3(const std::string& name, const Array& value, const GLsizei& count = 1) const;

  inline void setVec4(const std::string& name, const float x, const float y, const float z, const float w) const;
  inline void setVec4(const std::string& name, const glm::vec4& value) const;

  inline void setMat2(const std::string& name, const glm::mat2& mat) const;
  template<class Array>
  inline void setMat2(const std::string& name, const Array& mat, const GLboolean& transpose = GL_FALSE) const;
  inline void setMat3(const std::string& name, const glm::mat3& mat) const;
  template<class Array>
  inline void setMat3(const std::string& name, const Array& mat, const GLsizei& count = 1, const GLboolean& transpose = GL_FALSE) const;
  inline void setMat4(const std::string& name, const glm::mat4& mat) const;
  template<class Array>
  inline void setMat4(const std::string& name, const Array& mat, const GLboolean& transpose = GL_FALSE) const;

  inline void setBlock(const std::string& name, const int& uniformBlockBinding) const;
  inline void setBlock(const std::string& name, const int& uniformBlockBinding, const int& uniformBlockIndex) const;
  //void storeSubroutine(const std::string& uniformName, const std::string& subroutineName, const GLenum& shaderType) const;
  //void setSubroutines(const GLenum& shaderType) const;

  //virtual void storeSubroutine(const std::string& uniformName, const std::string& subroutineName) const;
  //virtual void setSubroutines() const;
};

template<std::convertible_to<std::string>... String>
Shader::Specs::Specs(const GLenum& type, const std::string& path, const String& ...macros)
: mType(type), mPath(path), mMacros(toString({macros...}, "\n"))
{}

inline std::string Shader::Specs::toString(const std::list<std::string>& strs, const std::string& seperator) const {
  std::string str;
  for(auto& s : strs) str+= s + seperator;
  return str;
}

inline void Shader::init() const {
  mId = glCreateProgram();
  for(const auto& specs : mSpecs) {
    std::cout << specs.mMacros;

    GLuint glShader = glCreateShader(specs.mType);
      const std::string code = shaderCode(specs.mPath, specs.mMacros);
      const char* cCode = code.c_str();

      glShaderSource(glShader, 1, &cCode, NULL);
      glCompileShader(glShader);
      checkCompileErrors(glShader, "Shader");
      glAttachShader(mId, glShader);
    glDeleteShader(glShader);
  }
  glLinkProgram(mId);
  checkCompileErrors(mId, "PROGRAM");
}

inline void Shader::initEmpty() const {
  mId = glCreateProgram();
    GLuint glShader = glCreateShader(GL_COMPUTE_SHADER);
    const std::string code = "#version 450\n layout(local_size_x = 1) in; void main() {}";
    const char* cCode = code.c_str();

    glShaderSource(glShader, 1, &cCode, NULL);
    glCompileShader(glShader);
    checkCompileErrors(glShader, "Shader");
    glAttachShader(mId, glShader);
    glDeleteShader(glShader);
  glLinkProgram(mId);
  checkCompileErrors(mId, "PROGRAM");
}

inline Shader::Shader(const std::initializer_list<Specs>& specs)
: mSpecs(specs)
{
  mSpecs.empty() ? initEmpty() : init();
}

inline void Shader::setSpecs(const std::initializer_list<Specs>& specs) {mSpecs = specs;}

inline void Shader::refresh() const {
  glDeleteProgram(mId);
  mSpecs.empty() ? initEmpty() : init();
}

inline std::string Shader::shaderCode(const std::string &path, const std::string& macros, const bool& firstRecursion) const {
  std::string includeIndentifier = "#include";
  std::string code = "";

  try{
    std::ifstream sourceFile(path);

    if(!sourceFile.is_open()) throw std::string(path + " not found");
    std::string line;

    bool macrosAreAdded = false;
    while(std::getline(sourceFile, line)) {
			// Look for the new shader include identifier
      if(line.find("#version ") != line.npos || line.find("#extension ") != line.npos) {
        code+= line + "\n";
        continue;
      } else if(not macrosAreAdded) {
        code+= macros;
        macrosAreAdded = true;
      } else if(line.find(includeIndentifier) != line.npos) {
				// Remove everything before include " and after ", this will cause the path to remain
				line.erase(0, line.find("\"")+1);
        line.erase(line.find("\""), line.npos);

				// The include path is relative to the current shader file path
        // so remove the sourceFile name and store the path to this folder
        size_t found = path.find_last_of("/");
				line.insert(0, path.substr(0, found+1));

				// By using recursion, the new include file can be extracted
				// and inserted at this location in the shader source code
				code+= shaderCode(line, {}, false);

				// Do not add this line to the shader source code, as the include
				// path would generate a compilation issue in the final source code
				continue;
			}
			code+= line + "\n";
		}
    sourceFile.close();
  }
  catch(std::string fail) {
    // handle exception here
    std::cerr << "[Error] " << fail << '\n';
  }
  return code;
}

inline void Shader::use() const {glUseProgram(mId);}
// utility uniform functions
inline void Shader::setBool(const std::string& name, const bool& value) const {
    glUniform1i(glGetUniformLocation(mId, name.c_str()), (int)value);
}
inline void Shader::setInt(const std::string& name, const int& value) const {
    glUniform1i(glGetUniformLocation(mId, name.c_str()), value);
}

inline void Shader::setIvec2(const std::string& name, const int x, const int y) const {
  glUniform2i(glGetUniformLocation(mId, name.c_str()), x, y);
}
inline void Shader::setIvec2(const std::string& name, const glm::ivec2& value) const {
  glUniform2iv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}
template<class Array>
inline void Shader::setIvec2(const std::string& name, const Array& value, const GLsizei& count) const {
  glUniform2iv(glGetUniformLocation(mId, name.c_str()), count, value.data());
}

inline void Shader::setIvec3(const std::string& name, const int x, const int y, const int z) const {
  glUniform3i(glGetUniformLocation(mId, name.c_str()), x, y, z);
}
inline void Shader::setIvec3(const std::string& name, const glm::ivec3& value) const {
  glUniform3iv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}
template<class Array>
inline void Shader::setIvec3(const std::string& name, const Array& value, const GLsizei& count) const {
  glUniform3iv(glGetUniformLocation(mId, name.c_str()), count, value.data());
}

inline void Shader::setIvec4(const std::string& name, const int x, const int y, const int z, const int w) const {
  glUniform4i(glGetUniformLocation(mId, name.c_str()), x, y, z, w);
}
inline void Shader::setIvec4(const std::string& name, const glm::ivec4& value) const {
  glUniform4iv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}

inline void Shader::setUint(const std::string& name, const unsigned int& value) const {
  glUniform1ui(glGetUniformLocation(mId, name.c_str()), value);
}

inline void Shader::setFloat(const std::string& name, const float& value) const {
    glUniform1f(glGetUniformLocation(mId, name.c_str()), value);
}
template<class Array>
inline void Shader::setFloat(const std::string& name, const Array& values, const GLsizei& count) const {
  glUniform1fv(glGetUniformLocation(mId, name.c_str()), count, values.data());
}

inline void Shader::setVec2(const std::string& name, float x, float y) const {
  glUniform2f(glGetUniformLocation(mId, name.c_str()), x, y);
}
inline void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}
template<class Array>
inline void Shader::setVec2(const std::string& name, const Array& value, const GLsizei& count) const {
  glUniform2fv(glGetUniformLocation(mId, name.c_str()), count, value.data());
}

inline void Shader::setVec3(const std::string& name, float x, float y, float z) const {
  glUniform3f(glGetUniformLocation(mId, name.c_str()), x, y, z);
}
inline void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}
template<class Array>
inline void Shader::setVec3(const std::string& name, const Array& value, const GLsizei& count) const {
  glUniform3fv(glGetUniformLocation(mId, name.c_str()), count, value.data());
}

inline void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(mId, name.c_str()), x, y, z, w);
}
inline void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
  glUniform4fv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}

inline void Shader::setMat2(const std::string& name, const glm::mat2& mat) const {
    glUniformMatrix2fv(glGetUniformLocation(mId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
template<class Array>
inline void Shader::setMat2(const std::string& name, const Array& mat, const GLboolean& transpose) const {
    glUniformMatrix2fv(glGetUniformLocation(mId, name.c_str()), 1, transpose, mat.data());
}
inline void Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
    glUniformMatrix3fv(glGetUniformLocation(mId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
template<class Array>
inline void Shader::setMat3(const std::string& name, const Array& mat, const GLsizei& count, const GLboolean& transpose) const {
    glUniformMatrix3fv(glGetUniformLocation(mId, name.c_str()), count, transpose, mat.data());
}
inline void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(mId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
template<class Array>
inline void Shader::setMat4(const std::string& name, const Array& mat, const GLboolean& transpose) const {
    glUniformMatrix4fv(glGetUniformLocation(mId, name.c_str()), 1, transpose, mat.data());
}

inline void Shader::setBlock(const std::string& name, const int& uniformBlockBinding) const {
    glUniformBlockBinding(mId, glGetUniformBlockIndex(mId, name.c_str()), uniformBlockBinding);
}
inline void Shader::setBlock(const std::string& name, const int& uniformBlockBinding, const int& uniformBlockIndex) const {
    glUniformBlockBinding(mId, uniformBlockIndex, uniformBlockBinding);
}
//void Shader::storeSubroutine(const std::string& uniformName, const std::string& subroutineName, const GLenum& shaderType) const {
//  int ul = glGetSubroutineUniformLocation(mId, shaderType, uniformName.c_str());
//  unsigned int si = glGetSubroutineIndex(mId, shaderType, subroutineName.c_str());}
//void Shader::setSubroutines(const GLenum& shaderType) const {
//  glUniformSubroutinesuiv(shaderType, subroutineIndices.size(), subroutineIndices.data());
//}
//void Shader::storeSubroutine(const std::string& uniformName, const std::string& subroutineName) const {}
//void Shader::setSubroutines() const {}
inline void Shader::checkCompileErrors(GLuint shader, std::string type) const {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
inline Shader::~Shader() {glDeleteProgram(mId);}
} // namespace gl


#endif//SHADER_HPP
