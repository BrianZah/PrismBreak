#ifndef MENU_METRICSSORTCOMBO_HPP
#define MENU_METRICSSORTCOMBO_HPP

#include "menu_Element.hpp"

#include <map>

#include "pass_Prism.hpp"

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
    {pass::Prism::variance, "variance"},
    {pass::Prism::sparsity, "sparsity"},
    {pass::Prism::visibility, "visibility"}
  };

};
} // namespace menu

#endif // MENU_METRICSSORTCOMBO_HPP
