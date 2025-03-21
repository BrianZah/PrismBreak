#ifndef GUI_MENU_ELEMNENT_HPP
#define GUI_MENU_ELEMNENT_HPP

#include "vis/Window.hpp"

namespace gui{
namespace menu{
class Element{
public:
  virtual inline bool showIf(const bool& show, const bool& enable = true) = 0;
protected:
  constexpr Element(vis::Window& vis) : mVis(vis) {}
  vis::Window& mVis;
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_ELEMNENT_HPP
