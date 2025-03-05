#include "gui_Manager.hpp"

#include "glfw_Window.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <string>
#include <memory>

#include "glfw_Window.hpp"
#include "vis_Window.hpp"
#include "menu_Main.hpp"
#include "Context.hpp"
//#include "CLooper.hpp"

namespace gui{
  Manager::Manager(glfw::Window& window, vis::Window& vis)
  : mWindow(window),
    mVis(vis),
    //mImGui(),
    mInitialized(false),
    mMenu(0, 0, window.getHeight()/5, window.getHeight(), vis),
    mShowDemoWindow(true)
  {
    //mImGui.init(mWindow, "#version 450");
    std::string glVersion = "#version 450";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window.getGLFWwindowPtr(), true);
    ImGui_ImplOpenGL3_Init(glVersion.c_str());
    ImGui::StyleColorsLight();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("/mnt/local/Documents/probability-vis/libs_3rd_party/imgui-1.89.9/misc/fonts/DroidSans.ttf", 25.0f);
    ImGui::GetStyle().ScaleAllSizes(3.0f);
    //ImGui::PushFont(font1);
    //ImGui::PopFont();
    mInitialized = true;

    mVis.setExternalGuiWantCaptureMouse([](){return ImGui::GetIO().WantCaptureMouse;});
    mVis.setExternalGuiWantCaptureKeyboard([](){return ImGui::GetIO().WantCaptureKeyboard;});
    //mVis.setExternalGuiWantCaptureKeyboard(std::bind(&menu::ImGuiContext::wantCaptureKeyboard, &mImGui));

    //mLooper->run();
    //std::this_thread::sleep_for(std::chrono::milliseconds(5));
    //mDispatcher = mLooper->getDispatcher();
    std::cout << "[Info](Manager::Manager) complete" << std::endl;
  }

  void Manager::newFrame() const {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void Manager::render() const {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  void Manager::showPointInfo() {
    auto [newDis, newPoint] = mVis.refRayCast().getSelectedDistributionAndPoint();
    static int iDis = -1;
    static int iPoint = -1;
    static bool mouseBlock = false;
    if(not ImGui::IsMouseDown(ImGuiMouseButton_Left)) mouseBlock = false;
    if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && not mouseBlock) {
      iDis = -1;
      iPoint = -1;
    }
    if(-1 < newDis) {
      iDis = newDis;
      iPoint = newPoint;
      mouseBlock = true;
    }
    if(-1 < iDis) {
      ImGui::OpenPopup("my_select_popup");
      ImGui::BeginPopup("my_select_popup");
      if(-1 < iPoint) {
        ImGui::SeparatorText((std::to_string(iPoint+1) + " " + mVis.getContext()->getPointNames()[iPoint]).c_str());
        auto unscaledValues = mVis.getContext()->getUnscaledPointValues(iPoint);
        //auto values = mVis.getContext()->getPointValues(iPoint);
        for(int i = 0; i < unscaledValues.size(); ++i) {
          ImGui::Text((mVis.getContext()->getAttributes()[i] + " = %.3f\n").c_str(), unscaledValues[i]);
        }
        float std = mVis.getContext()->getPointStandardDerivation(mVis.getRayCast().getModel(), iPoint);
        ImGui::Text("Standard Derivation = %.3f\n", std);
      } else {
        ImGui::SeparatorText(("Cluster " + std::to_string(iDis+1) + " (Means)").c_str());
        auto unscaledValues = mVis.getContext()->getUnscaledMean(mVis.getRayCast().getModel(), iDis);
        //auto values = mVis.getContext()->getMean(mVis.getRayCast().getModel(), iDis);
        for(int i = 0; i < unscaledValues.size(); ++i) {
          ImGui::Text((mVis.getContext()->getAttributes()[i] + " = %.3f\n").c_str(), unscaledValues[i]);
        }
      }
    }
  }

  void Manager::showGlyphInfo() {
    auto [set, comp, index] = mVis.refPrism().getUIElementSetAndCompAndIndex();
    int stage = mVis.refPrism().getStage();
    if(-1 < index) {
      ImGui::OpenPopup("my_select_popup");
      ImGui::BeginPopup("my_select_popup");
        for(int i = 0; i < mVis.getContext()->getAttributes().size(); ++i) {
          if(i == index)
            ImGui::Text(("> " + mVis.getContext()->getAttributes()[i] + " = " + std::to_string(std::abs(mVis.getContext()->getComponentValue(stage, set, comp, i))) + " <\n").c_str());
          else
            ImGui::Text(("   " + mVis.getContext()->getAttributes()[i] + " = " + std::to_string(std::abs(mVis.getContext()->getComponentValue(stage, set, comp, i))) + "\n").c_str());
        }
    }
  }

  void Manager::show() {
    if(mWindow.sizeHasChanged()) {
      mMenu.setHeight(mWindow.getHeight());
      mMenu.setWidth(mWindow.getHeight()/5);
    }

    newFrame();

    mMenu.show();
    if(mShowDemoWindow) ImGui::ShowDemoWindow(&mShowDemoWindow);
    showPointInfo();
    showGlyphInfo();

    render();

    //mFilterWindow.show();
    //if(mParentWindow.sizeHasChanged())
    //{
    //  mVis->setHeight(mParentWindow.getHeight());
    //  mVis->setWidth(mParentWindow.getWidth() - mFilterWindow.mSize.x);
    //  mVis->initiateSizeAdaptation();
    //}
    //if(mFilterWindow.mWidthHasChanged)
    //{
    //  mVis->setPosX(mFilterWindow.mSize.x);
    //  mVis->setWidth(mParentWindow.getWidth() - mFilterWindow.mSize.x);
    //  mVis->initiateSizeAdaptation();
    //}

  }

  Manager::~Manager() {
    if(not mInitialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    /*mLooper->stop();*/
  }

}//namespace gui
