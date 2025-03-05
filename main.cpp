#include <memory>

#include "gui_Manager.hpp"
#include "Context.hpp"
#include "csv.hpp"

namespace defaultData{
  //const std::string name = "../data/test(dim=5_dis=5).csv";
  const std::string dir = "../data/Country-data";
  //const std::string dir = "../data/StudentPerformanceFactors_processed";
}

int main() {
  glfw::Window window(2560, 1440, 4, 5, true);
  if(window.initializationFailed()) return 0;
  vis::Window vis(defaultData::dir, window, 0, 0, window.getWidth(), window.getHeight());
  gui::Manager gui(window, vis);

  while(not window.shouldClose()) {
    window.update();
    vis.update();
    gui.show();
  }

  return 1;
}
