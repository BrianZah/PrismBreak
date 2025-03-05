#ifndef MENU_CODOMAINSCALESLIDER_HPP
#define MENU_CODOMAINSCALESLIDER_HPP

#include "menu_Element.hpp"

#include <map>
#include <cmath>

#include "pass_Prism.hpp"

namespace menu{
class CodomainScaleSlider : public Element {
public:
  inline CodomainScaleSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    std::string title = "Codomain Space Scale = %.2f";
    if(ImGui::DragFloat("##Codomain Space Scale", &mVis.refPrism().refCodomainScale(), 0.01f, 0.0f, FLT_MAX, title.c_str(), mFlags)) {
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

#endif // MENU_CODOMAINSCALESLIDER_HPP
