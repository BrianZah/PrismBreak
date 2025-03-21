#ifndef GUI_MENU_THRESHOLDSLIDER_HPP
#define GUI_MENU_THRESHOLDSLIDER_HPP

#include "gui/menu/Element.hpp"

#include <map>
#include <cmath>

#include "pass/Prism.hpp"

namespace gui{
namespace menu{
class ThresholdSlider : public Element {
public:
  inline ThresholdSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    std::string title = mAdoptToStd ? "Standard Derivation = %.2f" : "Threshold = %.7f";
    float stepSize = mVis.getPrism().getThreshold()*0.01f+0.0000001f;
    if(ImGui::DragFloat("##Threshold", &mVis.refPrism().refThreshold(), stepSize, 0.0f, FLT_MAX, title.c_str(), mFlags)) {
      mVis.refBookmarks().setThreshold(mVis.getPrism().getThreshold());
      mVis.refRayCast().setThreshold(mVis.getPrism().getThreshold());
    }
    if(ImGui::Checkbox("Adopt Threshold To Standard Derivation", &mAdoptToStd)) {
      mVis.refPrism().adoptThresholdToStd(mAdoptToStd);
      mVis.refRayCast().adoptThresholdToStd(mAdoptToStd);
      mVis.refBookmarks().adoptThresholdToStd(mAdoptToStd);
    }
    ImGui::PopItemWidth();
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = ImGuiSliderFlags_None;
  bool mAdoptToStd = true;
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_THRESHOLDSLIDER_HPP
