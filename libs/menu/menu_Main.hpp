#ifndef MENU_MAIN_HPP
#define MENU_MAIN_HPP

#include "vis_Window.hpp"
#include "menu_steepestDescent.hpp"
#include "menu_fileDialog.hpp"
#include "menu_Coloring.hpp"
#include "combo/menu_MetricsSortCombo.hpp"
#include "combo/menu_RenderingCombo.hpp"
#include "combo/menu_ModelCombo.hpp"
#include "combo/menu_ProjectionMethodCombo.hpp"
#include "combo/menu_PointEncodingCombo.hpp"
#include "slider/menu_PointRangeSlider.hpp"
#include "slider/menu_DomainScaleSlider.hpp"
#include "slider/menu_CodomainScaleSlider.hpp"
#include "slider/menu_ThresholdSlider.hpp"
#include "slider/menu_DistributionDepthSlider.hpp"
#include "checkbox/menu_ScaleByWeight.hpp"

namespace menu{
  class Main{
  public:
    inline Main(const int& posX, const int& posY, const int& width, const int& height, vis::Window& vis);
    inline void show();
    constexpr const ImVec2& getSize() const;
    constexpr void setHeight(const int& height);
    constexpr void setWidth(const int& width);
  private:
    inline void showFramerate(const bool& show = true);
    inline void drawGui();

    ImGuiWindowFlags mWindowFlags = ImGuiWindowFlags_NoTitleBar;
    bool mShow = true;
    ImVec2 mPos = ImVec2(0, 0);
    ImVec2 mSize = ImVec2(0, 0);
    ImVec2 mSizeNew = ImVec2(0, 0);
    bool mWidthHasChanged = true;

    ImGuiStyle& mStyle;

    vis::Window& mVis;
    menu::SteepestDescent mSteepestDescent;
    menu::FileDialog mFileDialog;
    menu::Coloring mColoring;

    menu::MetricsSortCombo mMetricsSortCombo;
    menu::RenderingCombo mRenderingCombo;
    menu::ModelCombo mModelCombo;
    menu::ProjectionMethodCombo mProjectionCombo;
    menu::PointEncodingCombo mPointEncodingCombo;

    menu::PointRangeSlider mPointRangeSlider;
    menu::DomainScaleSlider mDomainScaleSlider;
    menu::CodomainScaleSlider mCodomainScaleSlider;
    menu::ThresholdSlider mThresholdSlider;
    menu::DistributionDepthSlider mDistributionDepthSlider;

    menu::ScaleByWeight mScaleByWeight;
  };

  inline Main::Main(const int& posX, const int& posY, const int& width, const int& height, vis::Window& vis)
  : mPos(posX, posY), mSize(width, height), mSizeNew(width, height), mStyle(ImGui::GetStyle()),
    mVis(vis),
    mSteepestDescent(vis),
    mFileDialog(vis),
    mColoring(vis),
    mMetricsSortCombo(vis),
    mRenderingCombo(vis),
    mModelCombo(vis),
    mProjectionCombo(vis),
    mPointEncodingCombo(vis),
    mPointRangeSlider(vis),
    mDomainScaleSlider(vis),
    mCodomainScaleSlider(vis),
    mThresholdSlider(vis),
    mDistributionDepthSlider(vis),
    mScaleByWeight(vis)
  {}

  inline void Main::show() {
    ImGui::SetNextWindowPos(mPos);
    ImGui::SetNextWindowSize(mSize);
    //mStyle.WindowRounding = 0.0f;
    ImGui::Begin("menu::Main", &mShow, mWindowFlags);
    //ImGui::PushItemWidth(mSize.x-20.0f);

    mSizeNew.x = ImGui::GetWindowSize().x;
    //mWidthHasChanged = (mSize.x != mSizeNew.x);
    mSize.x = mSizeNew.x;

    //if(mWidthHasChanged) {
    //  mSteepestDescent.setWidth(mSize.x);
    //}
    //mSteepestDescent.show();
    showFramerate();

    mFileDialog.showIf(true, true);
    //std::cout << "[Info] (menu::Main::show) mFileDialog complete" << std::endl;
    mModelCombo.showIf(true, true);
    //std::cout << "[Info] (menu::Main::show) mModelCombo complete" << std::endl;
    mScaleByWeight.showIf(true, true);
    //std::cout << "[Info] (menu::Main::show) mScaleByWeight complete" << std::endl;
    mMetricsSortCombo.showIf(true, vis::Window::prism == mVis.getRenderPass());
    //std::cout << "[Info] (menu::Main::show) mMetricsSortCombo complete" << std::endl;
    mRenderingCombo.showIf(true, mVis.getPrism().getStage() >= 2);
    //std::cout << "[Info] (menu::Main::show) mRenderingCombo complete" << std::endl;
    mProjectionCombo.showIf(true, vis::Window::rayCast == mVis.getRenderPass());
    //std::cout << "[Info] (menu::Main::show) mProjectionCombo complete" << std::endl;
    mPointEncodingCombo.showIf(true, vis::Window::rayCast == mVis.getRenderPass());
    //std::cout << "[Info] (menu::Main::show) mPointEncodingCombo complete" << std::endl;
    mPointRangeSlider.showIf(true, vis::Window::rayCast == mVis.getRenderPass());
    //std::cout << "[Info] (menu::Main::show) mPointRangeSlider complete" << std::endl;
    mDomainScaleSlider.showIf(true, vis::Window::prism == mVis.getRenderPass());
    //std::cout << "[Info] (menu::Main::show) mDomainScaleSlider complete" << std::endl;
    mCodomainScaleSlider.showIf(true, mVis.getPrism().getStage() == 0 && vis::Window::prism == mVis.getRenderPass());
    //std::cout << "[Info] (menu::Main::show) mCodomainScaleSlider complete" << std::endl;
    mThresholdSlider.showIf(true, mVis.getPrism().getStage() >= 1);
    //std::cout << "[Info] (menu::Main::show) mThresholdSlider complete" << std::endl;
    mDistributionDepthSlider.showIf(true, mVis.getPrism().getStage() == 2 && vis::Window::prism == mVis.getRenderPass());
    //std::cout << "[Info] (menu::Main::show) mDistributionDepthSlider complete" << std::endl;
    mColoring.showIf(true, true);
    if(mVis.refPerformanceTest().isRunning()) ImGui::BeginDisabled();
      if(ImGui::Button("start perfomance test")) mVis.refPerformanceTest().start();
    if(mVis.refPerformanceTest().isRunning()) ImGui::EndDisabled();
    if(mVis.refLoadingTest().isRunning()) ImGui::BeginDisabled();
      if(ImGui::Button("start loading test")) mVis.refLoadingTest().start();
    if(mVis.refLoadingTest().isRunning()) ImGui::EndDisabled();

    ImGui::End();
  }

  constexpr const ImVec2& Main::getSize() const {return mSize;}
  constexpr void Main::setHeight(const int& height) {mSize.y = height;}
  constexpr void Main::setWidth(const int& width) {mSize.x = width;}

  inline void Main::showFramerate(const bool& show) {
    if(not show) return;
    ImGui::Text("Application average:\n%.1f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    //static char fontsize = 20;
    //char one = 1;
    //if(ImGui::InputScalar("Fontsize",      ImGuiDataType_S8,     &fontsize,  &one, NULL, "%d")) {
    //  ImGuiIO& io = ImGui::GetIO();
    //  ImFont* font2 = io.Fonts->AddFontFromFileTTF("/mnt/local/Documents/probability-vis/libs_3rd_party/imgui-1.89.9/misc/fonts/Cousine-Regular.ttf", fontsize);
    //  ImGui::PopFont();
    //  ImGui::PushFont(font2);
    //}
  }

} // namespace menu

#endif // MENU_MAIN_HPP
