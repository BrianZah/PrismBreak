#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP

#include <string>
#include <memory>

#include "glfw_Window.hpp"
#include "imgui.h"

#include "vis_Window.hpp"
#include "menu_Main.hpp"
#include "Context.hpp"
//#include "CLooper.hpp"

namespace gui{
  class Manager{
  public:
    Manager(glfw::Window& window, vis::Window& vis);
    void show();
    ~Manager();

  private:
    glfw::Window& mWindow;
    vis::Window& mVis;

    bool mInitialized = false;
    menu::Main mMenu;

    bool mShowDemoWindow;

    void showPointInfo();
    void showGlyphInfo();
    void newFrame() const;
    void render() const;

    //std::unique_ptr<CLooper<>> mLooper = std::make_unique<CLooper<>>();
    //std::shared_ptr<CLooper<>::CDispatcher> mDispatcher;
  }; // class Manager
}//namespace gui

#endif // GUI_MANAGER_HPP
