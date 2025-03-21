#ifndef GUI_MENU_METRICSSORTCOMBO_HPP
#define GUI_MENU_METRICSSORTCOMBO_HPP

#include "gui/menu/Element.hpp"

#include <map>

#include "pass/Prism.hpp"

namespace gui{
namespace menu{
class MetricsSortCombo : public Element {
public:
  inline MetricsSortCombo(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    if(ImGui::BeginCombo("Sorting", mMetrics.at(mVis.getPrism().getMetric()).c_str(), mFlags)) {
      for(const auto& [metric, name] : mMetrics) {
        const bool is_selected = (metric == mVis.getPrism().getMetric());
        if(ImGui::Selectable((name + "##" + std::to_string(metric)).c_str()))
          mVis.refPrism().sortTiles(metric);
        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if(is_selected) ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = 0;
  const std::map<pass::Prism::Metric, std::string> mMetrics{
    {pass::Prism::variance, "Variance"},
    {pass::Prism::sparsity, "Sparsity"},
    {pass::Prism::visibility, "Visibility"}
  };

};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_METRICSSORTCOMBO_HPP
