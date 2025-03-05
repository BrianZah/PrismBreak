#ifndef MENU_STEEPESTDESCENT_HPP
#define MENU_STEEPESTDESCENT_HPP

#include "vis_Window.hpp"

namespace menu{
  class SteepestDescent{
  private:
    bool mShow;
    vis::Window& mVis;
    //float mWidth;
  public:
    constexpr SteepestDescent(vis::Window& vis);
    inline void show(const bool& show = true);
    //inline void setWidth(const float& width);
  };

  constexpr SteepestDescent::SteepestDescent(vis::Window& vis)
  : mShow(true), mVis(vis)
  {}
  inline void SteepestDescent::show(const bool& show) {
    auto& optimizeCamera = mVis.refCameraOptimizer();
    ImGui::Text("optimize Camera Position");
    static int i = 200;
    if(optimizeCamera.running()) ImGui::BeginDisabled();
      ImGui::Text("max Steps =");
      ImGui::SameLine();
      ImGui::PushItemWidth(90.0f);
      ImGui::InputInt("##max Steps", &i);
      ImGui::SameLine();
      ImGui::SetItemTooltip("max Steps");
      ImGui::SameLine();
    if(optimizeCamera.running()) ImGui::EndDisabled();
    if(not optimizeCamera.running()) {
      if(ImGui::Button("start")) optimizeCamera.start(i);
    } else {
      if(ImGui::Button("stop")) optimizeCamera.stop();
    }
    ImGui::PopItemWidth();
  }
  //inline void SteepestDescent::setWidth(const float& width) {mWidth = width;}
} // namespace menu

#endif // MENU_STEEPESTDESCENT_HPP
