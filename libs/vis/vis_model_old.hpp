#ifndef VIS_MODEL_HPP
#define VIS_MODEL_HPP

namespace vis{
  class Prisma{
  private:
    std::vector<float> mVertices = {-std::sqrt(2.0f/3.0f), -1.0f/3.0f, -std::sqrt(2.0f/9.0f),
                                     std::sqrt(2.0f/3.0f), -1.0f/3.0f, -std::sqrt(2.0f/9.0f),
                                     0.0f,                 -1.0f/3.0f,  std::sqrt(8.0f/9.0f),
                                     0.0f,                  1.0f,       0.0f};
    std::vector<unsigned int> mIndices = {0, 1, 3};
    gl::Buffer mVertexBuffer;
    gl::Buffer mElementBuffer;
    gl::VertexArray mVertexArray;

    glm::vec3 mCenter;
    glm::vec3 mUp;
    glm::vec3 mFront;
    glm::vec3 mRight;
  public:
    Prisma();
    init();
    void draw();
    const glm::mat4 getModelMatirx() const;
    void rotate(const float& xoffset, const float& yoffset, const float& zoffset);
  };
} // namespace vis

#endif // VIS_MODEL_HPP
