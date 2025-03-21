#ifndef GUI_MENU_COLORING_HPP
#define GUI_MENU_COLORING_HPP

#include "gui/menu/Element.hpp"

#include "pass/Prism.hpp"

namespace gui{
namespace menu{
class Coloring : public Element {
public:
  inline Coloring(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);


    ImGui::SeparatorText("Coloring");
    for(int iDis = 0; iDis < mVis.getContext()->numDistributions(mVis.getPrism().getModel()); ++iDis) {
      std::array<float, 3> color = mVis.getPrism().getColor(iDis);
      ImGuiColorEditFlags misc_flags = 0;

      static std::array<float, 3> backup_color;
      std::string title = (context::Context::tmm == mVis.getPrism().getModel() ? "T" : "G") + std::to_string((iDis+1)/10) + std::to_string((iDis+1)%10);

      bool open_popup = ImGui::ColorButton(("##Color" + std::to_string(iDis)).c_str(), ImVec4(color[0], color[1], color[2], 1.0f), misc_flags);
      ImGui::SameLine(0, 0.5f*ImGui::GetStyle().ItemInnerSpacing.x);
      ImGui::Text(title.c_str());

      if ((iDis % 5) != 4) ImGui::SameLine(0, 1.5f*ImGui::GetStyle().ItemInnerSpacing.x);
      if (open_popup)
      {
          ImGui::OpenPopup(("mypicker" + std::to_string(iDis)).c_str());
          backup_color = color;
      }
      if (ImGui::BeginPopup(("mypicker" + std::to_string(iDis)).c_str()))
      {
          if(ImGui::ColorPicker3(("##picker" + std::to_string(iDis)).c_str(), color.data(), misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_PickerHueWheel)) {
            mVis.refPrism().setColor(iDis, color, vis::Window::prism == mVis.getRenderPass());
            mVis.refBookmarks().setColor(iDis, color, vis::Window::prism == mVis.getRenderPass());
            mVis.refRayCast().setColor(iDis, color, vis::Window::rayCast == mVis.getRenderPass());
          }
          ImGui::SameLine();

          ImGui::BeginGroup(); // Lock X position
          ImGui::Text("Current");
          ImGui::ColorButton("##current", ImVec4(color[0], color[1], color[2], 1.0f), ImGuiColorEditFlags_NoPicker);
          ImGui::Text("Previous");
          if (ImGui::ColorButton("##previous", ImVec4(backup_color[0], backup_color[1], backup_color[2], 1.0f), ImGuiColorEditFlags_NoPicker)) {
            mVis.refPrism().setColor(iDis, backup_color, vis::Window::prism == mVis.getRenderPass());
            mVis.refBookmarks().setColor(iDis, backup_color, vis::Window::prism == mVis.getRenderPass());
            mVis.refRayCast().setColor(iDis, backup_color, vis::Window::rayCast == mVis.getRenderPass());
          }
          ImGui::Separator();
          ImGui::Text("Palette");
          for (int i = 0; i < pass::colors.size(); i+= 3)
          {
              ImGui::PushID(i);
              if ((i % 9) != 0)
                  ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

              ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
              if (ImGui::ColorButton("##Palette", ImVec4(pass::colors[i], pass::colors[i+1], pass::colors[i+2], 1.0f), palette_button_flags)) {
                  mVis.refPrism().setColor(iDis, {pass::colors[i], pass::colors[i+1], pass::colors[i+2]}, vis::Window::prism == mVis.getRenderPass());
                  mVis.refBookmarks().setColor(iDis, {pass::colors[i], pass::colors[i+1], pass::colors[i+2]}, vis::Window::prism == mVis.getRenderPass());
                  mVis.refRayCast().setColor(iDis, {pass::colors[i], pass::colors[i+1], pass::colors[i+2]}, vis::Window::rayCast == mVis.getRenderPass());
              }

              ImGui::PopID();
          }
          if(ImGui::Button("Hide")){
            mVis.refPrism().setColor(iDis, {1.0f, 1.0f, 1.0f}, vis::Window::prism == mVis.getRenderPass());
            mVis.refBookmarks().setColor(iDis, {1.0f, 1.0f, 1.0f}, vis::Window::prism == mVis.getRenderPass());
            mVis.refRayCast().setColor(iDis, {1.0f, 1.0f, 1.0f}, vis::Window::rayCast == mVis.getRenderPass());
          }

          ImGui::EndGroup();
          ImGui::EndPopup();
      }
    }
    ImGui::NewLine();
    ImGui::PopItemWidth();
    if(not enable) ImGui::EndDisabled();
    return true;
  }
}; // Coloring
} // menu
} // namespace gui

#endif // GUI_MENU_COLORING_HPP
