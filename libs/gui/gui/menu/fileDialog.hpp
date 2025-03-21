#ifndef GUI_MENU_FILEDIALOG_HPP
#define GUI_MENU_FILEDIALOG_HPP

#include "gui/menu/Element.hpp"

#include "ImGuiFileDialog.h"

namespace gui{
namespace menu{
class FileDialog : public Element {
private:
  bool mIsOpen = false;
public:
  inline FileDialog(vis::Window& vis) : Element(vis) {}
  inline bool showIf(const bool& show, const bool& enable = true) override {
    if(not show) return false;
    if(not enable) ImGui::BeginDisabled();

    ImGui::PushItemWidth(-FLT_MIN);
    int pos = mVis.getContext()->getDir().find_last_of("/");
    pos = pos == std::string::npos ? 0 : pos+1;
    std::string dataSetName = mVis.getContext()->getDir().substr(pos);

    if(ImGui::Button(("Data Set: " + dataSetName).c_str())) {
      IGFD::FileDialogConfig config;
      config.path = (mVis.getContext()->getDir() + "/..").c_str();
      config.countSelectionMax = 1;
      config.flags = ImGuiFileDialogFlags_Modal;
      ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose a Directory", nullptr, config);
      mIsOpen = true;
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseDirDlgKey")) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        mVis.loadContext(filePath);
      }
      ImGuiFileDialog::Instance()->Close();
      mIsOpen = false;
    }

    ImGui::PopItemWidth();

    if(not enable) ImGui::EndDisabled();
    return true;
  }
  constexpr const bool& isOpen() const {return mIsOpen;}
};
} // namespace menu
} // namespace gui

#endif // GUI_MENU_FILEDIALOG_HPP
