#ifndef GUI_MENU_CODOMAINSCALESLIDER_HPP
#define GUI_MENU_CODOMAINSCALESLIDER_HPP

#include "gui/menu/Element.hpp"

#include <map>
#include <cmath>

#include "pass/Prism.hpp"

namespace gui{
namespace menu{
class CodomainScaleSlider : public Element {
public:
  inline CodomainScaleSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    std::string title = "Scale Codomain Space = %.2f";
    if(ImGui::DragFloat("##Scale Codomain Space", &mVis.refPrism().refCodomainScale(), 0.01f, 0.0f, FLT_MAX, title.c_str(), mFlags)) {
      mVis.refBookmarks().setCodomainScale(mVis.getPrism().getCodomainScale());
    }
    ImGui::PopItemWidth();
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = ImGuiSliderFlags_None;
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_CODOMAINSCALESLIDER_HPP
