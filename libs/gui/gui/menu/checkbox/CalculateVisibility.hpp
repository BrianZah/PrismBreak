#ifndef GUI_MENU_CALCULATEVISIBILITY_HPP
#define GUI_MENU_CALCULATEVISIBILITY_HPP

#include "gui/menu/Element.hpp"

#include <map>

#include "pass/Prism.hpp"

namespace gui{
namespace menu{
class CalculateVisibility : public Element {
public:
  inline CalculateVisibility(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    bool check = mVis.getPrism().getCalculateVisibility();
    if(ImGui::Checkbox("Calculate Visibility", &check)) {
      mVis.refPrism().setCalculateVisibility(check);
    }
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = 0;
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_CALCULATEVISIBILITY_HPP
