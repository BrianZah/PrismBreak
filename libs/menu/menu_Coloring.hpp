#ifndef MENU_COLORING_HPP
#define MENU_COLORING_HPP

#include "menu_Element.hpp"

#include "pass_Prism.hpp"

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
      std::string title = (Context::tmm == mVis.getPrism().getModel() ? "T" : "G") + std::to_string((iDis+1)/10) + std::to_string((iDis+1)%10) + ":";
      ImGui::Text(title.c_str());
      ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
      bool open_popup = ImGui::ColorButton(("##Color" + std::to_string(iDis)).c_str(), ImVec4(color[0], color[1], color[2], 1.0f), misc_flags);

      if ((iDis % 5) != 4) ImGui::SameLine(0, 10.0f);
      //open_popup |= ImGui::Button("Palette");
      if (open_popup)
      {
          ImGui::OpenPopup(("mypicker" + std::to_string(iDis)).c_str());
          backup_color = color;
      }
      if (ImGui::BeginPopup(("mypicker" + std::to_string(iDis)).c_str()))
      {
          //ImGui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
          //ImGui::Separator();
          if(ImGui::ColorPicker3(("##picker" + std::to_string(iDis)).c_str(), color.data(), misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_PickerHueWheel)) {
            mVis.refPrism().setColor(iDis, color, vis::Window::prism == mVis.getRenderPass());
            mVis.refBookmarks().setColor(iDis, color, vis::Window::prism == mVis.getRenderPass());
            mVis.refRayCast().setColor(iDis, color, vis::Window::rayCast == mVis.getRenderPass());
          }
          ImGui::SameLine();

          ImGui::BeginGroup(); // Lock X position
          ImGui::Text("Current");
          ImGui::ColorButton("##current", ImVec4(color[0], color[1], color[2], 1.0f), ImGuiColorEditFlags_NoPicker, ImVec2(60, 40));
          ImGui::Text("Previous");
          if (ImGui::ColorButton("##previous", ImVec4(backup_color[0], backup_color[1], backup_color[2], 1.0f), ImGuiColorEditFlags_NoPicker, ImVec2(60, 40))) {
            mVis.refPrism().setColor(iDis, backup_color, vis::Window::prism == mVis.getRenderPass());
            mVis.refBookmarks().setColor(iDis, backup_color, vis::Window::prism == mVis.getRenderPass());
            mVis.refRayCast().setColor(iDis, backup_color, vis::Window::rayCast == mVis.getRenderPass());
              //color = backup_color;
          }
          ImGui::Separator();
          ImGui::Text("Palette");
          for (int i = 0; i < pass::colors.size(); i+= 3)
          {
              ImGui::PushID(i);
              if ((i % 9) != 0)
                  ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

              ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
              if (ImGui::ColorButton("##palette", ImVec4(pass::colors[i], pass::colors[i+1], pass::colors[i+2], 1.0f), palette_button_flags, ImVec2(20, 20))) {
                  //color = ImVec4(pass::colors[i], pass::colors[i+1], pass::colors[i+2], color.w); // Preserve alpha!
                  mVis.refPrism().setColor(iDis, {pass::colors[i], pass::colors[i+1], pass::colors[i+2]}, vis::Window::prism == mVis.getRenderPass());
                  mVis.refBookmarks().setColor(iDis, {pass::colors[i], pass::colors[i+1], pass::colors[i+2]}, vis::Window::prism == mVis.getRenderPass());
                  mVis.refRayCast().setColor(iDis, {pass::colors[i], pass::colors[i+1], pass::colors[i+2]}, vis::Window::rayCast == mVis.getRenderPass());
              }

              ImGui::PopID();
          }
          if(ImGui::Button("Hide", ImVec2(68, 20))){
            //color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
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

#endif // MENU_COLORING_HPP
