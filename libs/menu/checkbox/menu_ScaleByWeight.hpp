#ifndef MENU_SCALEBYWEIGHT_HPP
#define MENU_SCALEBYWEIGHT_HPP

#include "menu_Element.hpp"

#include <map>

#include "pass_Prism.hpp"

namespace menu{
class ScaleByWeight : public Element {
public:
  inline ScaleByWeight(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    if(ImGui::Checkbox("Scale by weight", &mCheck)) {
      mVis.refPrism().scaleByWeight(mCheck);
      mVis.refRayCast().scaleByWeight(mCheck);
      mVis.refBookmarks().scaleByWeight(mCheck);
    }
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = 0;
  bool mCheck  = true;
};
} // namespace menu

#endif // MENU_SCALEBYWEIGHT_HPP
