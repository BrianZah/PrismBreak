#ifndef MENU_RENDERINGCOMBO_HPP
#define MENU_RENDERINGCOMBO_HPP

#include "menu_Element.hpp"

#include <map>

#include "pass_RayCast.hpp"
#include "pass_Prism.hpp"
#include "pass_Rendering.hpp"

namespace menu{
class RenderingCombo : public Element {
public:
  inline RenderingCombo(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    if(ImGui::BeginCombo("Rendering", mRendering.at(mVis.getPrism().getRendering()).c_str(), mFlags)) {
      for(const auto& [rendering, name] : mRendering) {
        const bool is_selected = (rendering == mVis.getPrism().getRendering());
        if(ImGui::Selectable((name + "##" + std::to_string(rendering)).c_str())) {
          mVis.refPrism().setRendering(rendering, vis::Window::prism == mVis.getRenderPass());
          mVis.refBookmarks().setRendering(rendering, vis::Window::prism == mVis.getRenderPass());
          mVis.refRayCast().setRendering(rendering, vis::Window::rayCast == mVis.getRenderPass());
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
  const std::map<pass::Rendering, std::string> mRendering = {
    {pass::mip, "Maximum Intensity Projection"},
    {pass::hulls, "Hulls"},
    {pass::dvr, "Direct Volume Rendering"}
  };
};
} // namespace menu

#endif // MENU_RENDERINGCOMBO_HPP
