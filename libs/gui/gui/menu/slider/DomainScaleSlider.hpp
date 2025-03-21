#ifndef GUI_MENU_DOMAINSCALESLIDER_HPP
#define GUI_MENU_DOMAINSCALESLIDER_HPP

#include "gui/menu/Element.hpp"

#include <map>

#include "pass/Prism.hpp"

namespace gui{
namespace menu{
class DomainScaleSlider : public Element {
public:
  inline DomainScaleSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    if(ImGui::DragFloat("##Scale Domain Space", &mVis.refPrism().refDomainScale(), 0.01f, 0.0f, FLT_MAX, "Scale Domain Space = %.2f", mFlags))
      mVis.refBookmarks().setDomainScale(mVis.getPrism().getDomainScale());
    ImGui::PopItemWidth();
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = ImGuiSliderFlags_None;
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_DOMAINSCALESLIDER_HPP
