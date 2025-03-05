#ifndef MENU_DISTRIBUTIONDEPTHSLIDER_HPP
#define MENU_DISTRIBUTIONDEPTHSLIDER_HPP

#include "menu_Element.hpp"

#include <map>

#include "pass_Prism.hpp"

namespace menu{
class DistributionDepthSlider : public Element {
public:
  inline DistributionDepthSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    if(ImGui::DragFloat("##distribution depth", &mVis.refPrism().refDistributionDepth(), 0.05f, 0.0f, FLT_MAX, "distribution depth = %.2f", mFlags))
      mVis.refBookmarks().setDistributionDepth(mVis.getPrism().getDistributionDepth());
    ImGui::PopItemWidth();
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = ImGuiSliderFlags_None;
};
} // namespace menu

#endif // MENU_DISTRIBUTIONDEPTHSLIDER_HPP
