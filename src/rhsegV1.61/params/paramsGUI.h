#ifndef PARAMSGUI_H
#define PARAMSGUI_H

#include <defines.h>
#include "expParamsGUI.h"
#include <gui/fileObject.h>
#include <gui/numberObject.h>
#include <gui/numberListObject.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <gtkmm.h>

using namespace std;
using namespace CommonTilton;

namespace HSEGTilton
{
  class ParamsGUI : public Gtk::Window
  {
    public:
        ParamsGUI();
        virtual ~ParamsGUI();

    protected:
      //Signal handlers:
        void on_help_requested();
        void on_exit_requested();
        void on_regionSumButton_clicked();
        void on_regionStdDevButton_clicked();
        void on_regionBoundaryNpixButton_clicked();
        void on_regionThresholdButton_clicked();
//        void on_regionNghbrsListButton_clicked();
        void on_regionNbObjectsButton_clicked();
        void on_regionObjectsListButton_clicked();
        void on_chkNregionsObject_activated();
        void on_hsegOutNregionsObject_activated();
        void on_hsegOutThresholdsObject_activated();
        void on_gDissimButton_clicked();
        void on_debugObject_activate();
        void on_expParamsButton_clicked();
        void on_run_program();

      //Member widgets:
        Gtk::VBox vBox, initialBox;
        Gtk::Label inputImageFile;
        Gtk::Label inputDescription;
        NumberListObject scaleFactorsObject;
        NumberListObject offsetFactorsObject;
        Gtk::Label maskImageFile;
        Gtk::Label stdDevInImageFile;
        Gtk::Label stdDevMaskImageFile;
        Gtk::Label edgeInImageFile;
        Gtk::Label edgeMaskImageFile;
        Gtk::Label regionMapInImageFile;
        Gtk::VBox outputFilesBox;
        Gtk::Label classLabelsMapFile;
        Gtk::Label boundaryMapFile;
        Gtk::Label regionClassesFile;
        Gtk::Label objectLabelsMapFile;
        Gtk::Label regionObjectsFile;
        Gtk::Label oparamFile;
        Gtk::Label logFile;
        Gtk::Label spClustWght;
        Gtk::Label dissimCrit;
        Gtk::Frame paramsFrame;
        Gtk::VBox  paramsBox;
        Gtk::CheckButton regionSumButton;
        Gtk::CheckButton regionStdDevButton;
        Gtk::CheckButton regionBoundaryNpixButton;
        Gtk::CheckButton regionThresholdButton;
//        Gtk::CheckButton regionNghbrsListButton;
        Gtk::CheckButton regionNbObjectsButton;
        Gtk::CheckButton regionObjectsListButton;
        NumberObject stdDevWghtObject;
        NumberObject edgeThresholdObject;
        NumberObject edgePowerObject;
        NumberObject edgeWghtObject;
        NumberObject seamEdgeThresholdObject;
        Gtk::HBox comboHBox;
        Gtk::Label connTypeLabel;
        Gtk::ComboBoxText connTypeComboBox;
        Gtk::Frame chooseFrame;
        Gtk::VBox chooseBox;
        NumberObject chkNregionsObject;
        NumberListObject hsegOutNregionsObject;
        NumberListObject hsegOutThresholdsObject;
        NumberObject convNregionsObject;
        Gtk::HBox lastHBox;
        Gtk::CheckButton gDissimButton;
        NumberObject debugObject;
        Gtk::Button expParamsButton;

        Glib::RefPtr<Gtk::UIManager> m_refUIManager;
        Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

    private:
        bool regionSumFlag;
        bool regionStdDevFlag;
        bool regionBoundaryNpixFlag;
        bool regionThresholdFlag;
        bool regionNghbrsListFlag;
        bool regionNbObjectsFlag;
        bool regionObjectsListFlag;
        bool chkNregionsFlag;
        bool hsegOutNregionsFlag;
        bool hsegOutThresholdsFlag;
        bool gDissimFlag;
        ExpParamsGUI expParamsGUI;
        bool on_delete_event(GdkEventAny* event);
  };

} // HSEGTilton

#endif /* PARAMSGUI_H */


