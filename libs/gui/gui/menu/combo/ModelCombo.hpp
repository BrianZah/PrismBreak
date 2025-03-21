#ifndef GUI_MENU_MODELCOMBO_HPP
#define GUI_MENU_MODELCOMBO_HPP

#include "gui/menu/Element.hpp"

#include <map>

#include "context/Context.hpp"
#include "pass/Prism.hpp"

namespace gui{
namespace menu{
class ModelCombo : public Element {
public:
  inline ModelCombo(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    if(ImGui::BeginCombo("Mixture Model", mModel.at(mVis.getPrism().getModel()).c_str(), mFlags)) {
      for(const auto& [model, name] : mModel) {
        const bool is_selected = (model == mVis.getPrism().getModel());
        if(ImGui::Selectable((name + "##" + std::to_string(model)).c_str())) {
          mVis.prepareContext(model);
          mVis.refPrism().setModel(model, vis::Window::prism == mVis.getRenderPass());
          mVis.refBookmarks().setModel(model, vis::Window::prism == mVis.getRenderPass());
          mVis.refRayCast().setModel(model, vis::Window::rayCast == mVis.getRenderPass());
          mVis.refPoints().setModel(model, vis::Window::rayCast == mVis.getRenderPass());
        }
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
  const std::map<context::Context::Model, std::string> mModel{
    {context::Context::tmm, "t-Distribution"},
    {context::Context::gmm, "Gaussian"}
  };

};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_MODELCOMBO_HPP
