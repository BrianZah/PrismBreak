#ifndef MENU_FILEDIALOG_HPP
#define MENU_FILEDIALOG_HPP

#include "menu_Element.hpp"

#include "ImGuiFileDialog.h"

namespace menu{
class FileDialog : public Element {
public:
  inline FileDialog(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();
    ImGui::PushItemWidth(-FLT_MIN);
    int pos = mVis.getPathToData().find_last_of("/");
    pos = pos == std::string::npos ? 0 : pos+1;
    std::string dataSetName = mVis.getPathToData().substr(pos);
    ImGui::Text("Data Set:");
    ImGui::SameLine();
    if(ImGui::Button((dataSetName).c_str())) {
      IGFD::FileDialogConfig config;
      config.path = (mVis.getPathToData() + "/..").c_str();
      config.countSelectionMax = 1;
      config.flags = ImGuiFileDialogFlags_Modal;
      ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose a Directory", nullptr, config);
    }
    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseDirDlgKey")) {
      if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
        std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        std::cout << "filePath = " << filePath << std::endl;
        mVis.loadContext(filePath);
        // action
      }

      // close
      ImGuiFileDialog::Instance()->Close();
    }
    ImGui::PopItemWidth();
    //mStyle.WindowRounding = 0.0f;
    if(not enable) ImGui::EndDisabled();
    return true;
  }
};
} // namespace menu

#endif // MENU_FILEDIALOG_HPP
