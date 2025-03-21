#ifndef GUI_MENU_DISTRIBUTIONDEPTHSLIDER_HPP
#define GUI_MENU_DISTRIBUTIONDEPTHSLIDER_HPP

#include "gui/menu/Element.hpp"

#include <map>

#include "pass/Prism.hpp"

namespace gui{
namespace menu{
class DistributionDepthSlider : public Element {
public:
  inline DistributionDepthSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    if(ImGui::DragFloat("##Distribution Depth", &mVis.refPrism().refDistributionDepth(), 0.05f, 0.0f, FLT_MAX, "Distribution Depth = %.2f", mFlags))
      mVis.refBookmarks().setDistributionDepth(mVis.getPrism().getDistributionDepth());
    ImGui::PopItemWidth();
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = ImGuiSliderFlags_None;
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_DISTRIBUTIONDEPTHSLIDER_HPP
