#ifndef GUI_MENU_POINTRANGESLIDER_HPP
#define GUI_MENU_POINTRANGESLIDER_HPP

#include "gui/menu/Element.hpp"

#include <map>
#include <cmath>

#include "pass/Points.hpp"

namespace gui{
namespace menu{
class PointRangeSlider : public Element {
public:
  inline PointRangeSlider(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    std::string titleMin = pass::Points::std == mVis.getPoints().getEncoding()
                           ? "%.2f < Point Std" : "%.0f %% < Point Probability";
    std::string titleMax = pass::Points::std == mVis.getPoints().getEncoding()
                           ? "Point Std < %.2f" : "Point Probability < %.0f %%";
    ImGui::DragFloatRange2("##Hide Points", &mVis.refPoints().refRange()[0], &mVis.refPoints().refRange()[1], 0.25f, 0.0f, +FLT_MAX, titleMin.c_str(), titleMax.c_str(), mFlags);
    ImGui::PopItemWidth();
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = ImGuiSliderFlags_AlwaysClamp;
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_POINTRANGESLIDER_HPP
