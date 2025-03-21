#ifndef GUI_MENU_PROJECTIONMETHODCOMBO_HPP
#define GUI_MENU_PROJECTIONMETHODCOMBO_HPP

#include "gui/menu/Element.hpp"

#include <map>

#include "pass/RayCast.hpp"
#include "pass/Rendering.hpp"

namespace gui{
namespace menu{
class ProjectionMethodCombo : public Element {
public:
  inline ProjectionMethodCombo(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    if(ImGui::BeginCombo("Projection", mProjection.at(mVis.getRayCast().getProjectionMethod()).c_str(), mFlags)) {
      for(const auto& [projection, name] : mProjection) {
        const bool is_selected = (projection == mVis.getRayCast().getProjectionMethod());
        if(ImGui::Selectable((name + "##" + std::to_string(projection)).c_str())) {
          mVis.refRayCast().setProjectionMethod(projection, vis::Window::rayCast == mVis.getRenderPass());
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
  const std::map<pass::ProjectionMethod, std::string> mProjection = {
    {pass::projectDown, "Project Down"},
    {pass::projectUp, "Project Up"}
  };
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_PROJECTIONMETHODCOMBO_HPP
