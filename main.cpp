#include <memory>

#include "glfwPP/Window.hpp"
#include "vis/Window.hpp"
#include "gui/Manager.hpp"

int main() {
  glfwPP::Window window(2560, 1440, 4, 5, true);
  if(window.initializationFailed()) return 0;
  vis::Window vis(std::string(CMAKE_DEFAULTDATASET), window, 0, 0, window.getWidth(), window.getHeight());
  gui::Manager gui(window, vis);

  while(not window.shouldClose()) {
    window.update();
    vis.update();
    gui.show();
  }

  return 1;
}
