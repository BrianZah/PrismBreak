#ifndef MENU_ELEMNENT_HPP
#define MENU_ELEMNENT_HPP

#include "vis_Window.hpp"

namespace menu{
class Element{
public:
  virtual inline bool showIf(const bool& show, const bool& enable = true) = 0;
protected:
  constexpr Element(vis::Window& vis) : mVis(vis) {}
  vis::Window& mVis;
};
} // namespace menu

#endif // MENU_ELEMNENT_HPP
