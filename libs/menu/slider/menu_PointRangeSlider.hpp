#ifndef MENU_POINTRANGESLIDER_HPP
#define MENU_POINTRANGESLIDER_HPP

#include "menu_Element.hpp"

#include <map>
#include <cmath>

#include "pass_Points.hpp"

namespace menu{
class PointRangeSlider : public Element {
public:
  inline PointRangeSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    std::string titleMin = pass::Points::std == mVis.getPoints().getEncoding()
                           ? "%.1f < Point std" : "%.1f % < Point Probability";
    std::string titleMax = pass::Points::std == mVis.getPoints().getEncoding()
                           ? "Point std < %.1f" : "Point Probability < %.1f %";
    ImGui::DragFloatRange2("##Hide Points", &mVis.refPoints().refRange()[0], &mVis.refPoints().refRange()[1], 0.25f, 0.0f, +FLT_MAX, titleMin.c_str(), titleMax.c_str(), mFlags);
    ImGui::PopItemWidth();
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = ImGuiSliderFlags_AlwaysClamp;
};
} // namespace menu

#endif // MENU_POINTRANGESLIDER_HPP
