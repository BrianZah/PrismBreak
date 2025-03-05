#ifndef MENU_THRESHOLDSLIDER_HPP
#define MENU_THRESHOLDSLIDER_HPP

#include "menu_Element.hpp"

#include <map>
#include <cmath>

#include "pass_Prism.hpp"

namespace menu{
class ThresholdSlider : public Element {
public:
  inline ThresholdSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    std::string title = mAdoptToStd ? "Standard Derivation = %.1f" : "threshold = %.7f";
    float stepSize = mVis.getPrism().getThreshold()*0.01f+0.0000001f;
    if(ImGui::DragFloat("##threshold", &mVis.refPrism().refThreshold(), stepSize, 0.0f, FLT_MAX, title.c_str(), mFlags)) {
      mVis.refBookmarks().setThreshold(mVis.getPrism().getThreshold());
      mVis.refRayCast().setThreshold(mVis.getPrism().getThreshold());
    }
    if(ImGui::Checkbox("Adopt threshold to Standard Derivation", &mAdoptToStd)) {
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

#endif // MENU_THRESHOLDSLIDER_HPP
