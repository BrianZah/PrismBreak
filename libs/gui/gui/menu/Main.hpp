#ifndef GUI_MENU_MAIN_HPP
#define GUI_MENU_MAIN_HPP

#include "vis/Window.hpp"
//#include "gui/menu/steepestDescent.hpp"
#include "gui/menu/fileDialog.hpp"
#include "gui/menu/Coloring.hpp"
#include "gui/menu/combo/MetricsSortCombo.hpp"
#include "gui/menu/combo/RenderingCombo.hpp"
#include "gui/menu/combo/ModelCombo.hpp"
#include "gui/menu/combo/ProjectionMethodCombo.hpp"
#include "gui/menu/combo/PointEncodingCombo.hpp"
#include "gui/menu/slider/PointRangeSlider.hpp"
#include "gui/menu/slider/DomainScaleSlider.hpp"
#include "gui/menu/slider/CodomainScaleSlider.hpp"
#include "gui/menu/slider/ThresholdSlider.hpp"
#include "gui/menu/slider/DistributionDepthSlider.hpp"
#include "gui/menu/checkbox/ScaleByWeight.hpp"
#include "gui/menu/checkbox/CalculateVisibility.hpp"

namespace gui{
namespace menu{
  class Main{
  public:
    inline Main(vis::Window& vis, const int& posX, const int& posY, const int& width, const int& height);
    inline void show();
    constexpr const bool& fileDialogIsOpen() const;
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
    //menu::SteepestDescent mSteepestDescent;
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
    menu::CalculateVisibility mCalculateVisibility;
  };

  inline Main::Main(vis::Window& vis, const int& posX, const int& posY, const int& width, const int& height)
  : mVis(vis),
    mPos(posX, posY), mSize(width, height), mSizeNew(width, height), mStyle(ImGui::GetStyle()),
    //mSteepestDescent(vis),
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
    mScaleByWeight(vis),
    mCalculateVisibility(vis)
  {}

  inline void Main::show() {
    ImGui::SetNextWindowPos(mPos);
    ImGui::SetNextWindowSize(mSize);
    ImGui::Begin("menu::Main", &mShow, mWindowFlags);

    mSizeNew.x = ImGui::GetWindowSize().x;
    mSize.x = mSizeNew.x;

    //if(mWidthHasChanged) {
    //  mSteepestDescent.setWidth(mSize.x);
    //}
    //mSteepestDescent.show();
    showFramerate();

    mFileDialog.showIf(true, true);
    mModelCombo.showIf(true, true);
    mScaleByWeight.showIf(true, true);
    mMetricsSortCombo.showIf(true, vis::Window::prism == mVis.getRenderPass());
    mCalculateVisibility.showIf(true, true);
    mRenderingCombo.showIf(true, mVis.getPrism().getStage() >= 2);
    mProjectionCombo.showIf(true, vis::Window::rayCast == mVis.getRenderPass());
    mPointEncodingCombo.showIf(true, vis::Window::rayCast == mVis.getRenderPass());
    mPointRangeSlider.showIf(true, vis::Window::rayCast == mVis.getRenderPass());
    mDomainScaleSlider.showIf(true, vis::Window::prism == mVis.getRenderPass());
    mCodomainScaleSlider.showIf(true, mVis.getPrism().getStage() == 0 && vis::Window::prism == mVis.getRenderPass());
    mThresholdSlider.showIf(true, mVis.getPrism().getStage() >= 1);
    mDistributionDepthSlider.showIf(true, mVis.getPrism().getStage() == 2 && vis::Window::prism == mVis.getRenderPass());
    mColoring.showIf(true, true);
    if(mVis.refPerformanceTest().isRunning()) ImGui::BeginDisabled();
      if(ImGui::Button("start perfomance test")) mVis.refPerformanceTest().start();
    if(mVis.refPerformanceTest().isRunning()) ImGui::EndDisabled();
    if(mVis.refLoadingTest().isRunning()) ImGui::BeginDisabled();
      if(ImGui::Button("start loading test")) mVis.refLoadingTest().start();
    if(mVis.refLoadingTest().isRunning()) ImGui::EndDisabled();

    ImGui::End();
  }

  constexpr const bool& Main::fileDialogIsOpen() const {return mFileDialog.isOpen();}

  constexpr const ImVec2& Main::getSize() const {return mSize;}
  constexpr void Main::setHeight(const int& height) {mSize.y = height;}
  constexpr void Main::setWidth(const int& width) {mSize.x = width;}

  inline void Main::showFramerate(const bool& show) {
    if(not show) return;
    ImGui::Text("Application average:\n%.1f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
  }

} // namespace menu
} // namespace gui

#endif // GUI_MENU_MAIN_HPP
