#ifndef PASS_RENDERING_HPP
#define PASS_RENDERING_HPP

namespace pass{
  enum Rendering{mip = 0, hulls = 1, dvr = 2};
  enum ProjectionMethod{projectDown = 0, projectUp = 1};
  constexpr std::array<float, 9*3> colors = {
     71.0f/255.0f, 168.0f/255.0f, 186.0f/255.0f,
    255.0f/255.0f, 255.0f/255.0f, 179.0f/255.0f,
    182.0f/255.0f, 140.0f/255.0f, 185.0f/255.0f,
    190.0f/255.0f, 186.0f/255.0f, 218.0f/255.0f,
    253.0f/255.0f, 180.0f/255.0f,  98.0f/255.0f,
    251.0f/255.0f, 128.0f/255.0f, 114.0f/255.0f,
     77.0f/255.0f,  73.0f/255.0f, 146.0f/255.0f,
    179.0f/255.0f, 222.0f/255.0f, 105.0f/255.0f,
    217.0f/255.0f, 217.0f/255.0f, 217.0f/255.0f
  };
} // namespace pass
#endif // PASS_RENDERING_HPP
