#ifndef GUI_MENU_POINTENCODINGCOMBO_HPP
#define GUI_MENU_POINTENCODINGCOMBO_HPP

#include "gui/menu/Element.hpp"

#include <map>

#include "pass/Points.hpp"

namespace gui{
namespace menu{
class PointEncodingCombo : public Element {
public:
  inline PointEncodingCombo(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    if(ImGui::BeginCombo("Point Encoding", mEncoding.at(mVis.getPoints().getEncoding()).c_str(), mFlags)) {
      for(const auto& [encoding, name] : mEncoding) {
        const bool is_selected = (encoding == mVis.getPoints().getEncoding());
        if(ImGui::Selectable((name + "##" + std::to_string(encoding)).c_str())) {
          mVis.refPoints().setEncoding(encoding);//, vis::Window::rayCast == mVis.getRenderPass());
          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if(is_selected) ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    if(not enable) ImGui::EndDisabled();
    return true;
  }
private:
  ImGuiComboFlags mFlags = 0;
  const std::map<pass::Points::Encoding, std::string> mEncoding = {
    {pass::Points::std, "Standard Derivation"},
    {pass::Points::probabilities, "Cluster Probabilities"}
  };
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_POINTENCODINGCOMBO_HPP
