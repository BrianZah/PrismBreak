#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP

#include <string>
#include <memory>

#include "glfwPP/Window.hpp"
#include "imgui.h"

#include "vis/Window.hpp"
#include "gui/menu/Main.hpp"
#include "context/Context.hpp"

namespace gui{
  class Manager{
  public:
    Manager(glfwPP::Window& window, vis::Window& vis);
    void show();
    ~Manager();

  private:
    glfwPP::Window& mWindow;
    vis::Window& mVis;

    bool mInitialized = false;
    menu::Main mMenu;

    bool mShowDemoWindow;
    bool mWantCaptureGlyph = false;

    void showPointInfo();
    void showGlyphInfo();
  }; // class Manager
}//namespace gui

#endif // GUI_MANAGER_HPP
