#ifndef MENU_DOMAINSCALESLIDER_HPP
#define MENU_DOMAINSCALESLIDER_HPP

#include "menu_Element.hpp"

#include <map>

#include "pass_Prism.hpp"

namespace menu{
class DomainScaleSlider : public Element {
public:
  inline DomainScaleSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    if(ImGui::DragFloat("##domain space scale", &mVis.refPrism().refDomainScale(), 0.01f, 0.0f, FLT_MAX, "domain space scale = %.2f", mFlags))
      mVis.refBookmarks().setDomainScale(mVis.getPrism().getDomainScale());
    ImGui::PopItemWidth();
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = ImGuiSliderFlags_None;
};
} // namespace menu

#endif // MENU_DOMAINSCALESLIDER_HPP
